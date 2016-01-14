/*
 * Copyright (c) 2015, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Intel Corporation nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Lay out the volatile region in memory.
 */

#include "LayoutStepVolatile.h"
#include <utility.h>
#include <LogEnterExit.h>
#include <exception/NvmExceptionBadRequest.h>

wbem::logic::LayoutStepVolatile::LayoutStepVolatile()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

wbem::logic::LayoutStepVolatile::~LayoutStepVolatile()
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
}

bool wbem::logic::LayoutStepVolatile::isRemainingStep(const MemoryAllocationRequest &request)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);
	return request.volatileCapacity == REQUEST_REMAINING_CAPACITY;
}

void wbem::logic::LayoutStepVolatile::execute(const MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytesToAllocate = getRequestedCapacityBytes(request, layout);
	if (bytesToAllocate)
	{
		std::vector<Dimm> dimmsToLayout;
		for (std::vector<Dimm>::const_iterator dimmIter = request.dimms.begin();
				dimmIter != request.dimms.end(); dimmIter++)
		{
			if (layout.reserveDimmGuid != dimmIter->guid)
			{
				dimmsToLayout.push_back(*dimmIter);
			}
		}
		NVM_UINT64 bytesAllocated = 0;
		NVM_UINT64 alignedBytesAllocated = 0;
		while (bytesAllocated < bytesToAllocate)
		{
			NVM_UINT64 bytesRemaining = bytesToAllocate - bytesAllocated;
			try
			{
				std::vector<Dimm> dimmsIncluded;
				NVM_UINT64 bytesPerDimm = getLargestPerDimmSymmetricalBytes(
						dimmsToLayout, layout.goals, bytesRemaining, dimmsIncluded);
				for (std::vector<Dimm>::const_iterator dimmIter = dimmsIncluded.begin();
								dimmIter != dimmsIncluded.end(); dimmIter++)
				{
					NVM_UINT64 alignedBytes = getAlignedDimmBytes(request, *dimmIter, layout, bytesPerDimm);
					layout.goals[dimmIter->guid].volatile_size += bytesToConfigGoalSize(alignedBytes);
					alignedBytesAllocated += alignedBytes;
					bytesAllocated += bytesPerDimm;
				}
			}
			catch (exception::NvmExceptionBadRequestSize &e)
			{
				// out of capacity, clean up and pass along the exception
				layout.volatileCapacity = bytesToConfigGoalSize(alignedBytesAllocated);
				throw exception::NvmExceptionBadRequestVolatileSize();
			}
		}
		layout.volatileCapacity = bytesToConfigGoalSize(alignedBytesAllocated);
	}
}

NVM_UINT64 wbem::logic::LayoutStepVolatile::getRequestedCapacityBytes(
		const struct MemoryAllocationRequest& request,
		MemoryAllocationLayout& layout)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	if (request.volatileCapacity != REQUEST_REMAINING_CAPACITY)
	{
		bytes = configGoalSizeToBytes(request.volatileCapacity);
	}
	else
	{
		bytes = getRemainingBytesFromRequestedDimms(request, layout);
	}
	return bytes;
}

NVM_UINT64 wbem::logic::LayoutStepVolatile::getAlignedDimmBytes(
		const MemoryAllocationRequest& request,
		const Dimm &dimm,
		MemoryAllocationLayout& layout,
		const NVM_UINT64 &requestedBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 existingVolatileBytes = bytesToConfigGoalSize(layout.goals[dimm.guid].volatile_size);
	NVM_UINT64 totalVolatileBytes = getTotalVolatileBytes(requestedBytes, existingVolatileBytes);
	NVM_UINT64 dimmBytes = round_down(dimm.capacity, BYTES_PER_GB);

	NVM_UINT64 alignedTotalVolatileBytes = totalVolatileBytes;
	// volatile layout is last step
	if (request.volatileCapacity == REQUEST_REMAINING_CAPACITY)
	{
		if (request.persistentExtents.size() > 0)
		{
			alignedTotalVolatileBytes = roundDownVolatileToPMAlignment(
					dimm, layout, totalVolatileBytes, dimmBytes);
		}
	}
	// volatile is first step
	else
	{
		// round up volatile by consuming storage
		alignedTotalVolatileBytes = roundVolatileToNearestPMAlignment(
				dimm, layout, totalVolatileBytes, dimmBytes);
	}

	if (alignedTotalVolatileBytes <= existingVolatileBytes)
	{
		throw exception::NvmExceptionBadRequestSize();
	}
	NVM_UINT64 alignedBytes = alignedTotalVolatileBytes - existingVolatileBytes;
	if (alignedBytes < BYTES_PER_GB)
	{
		throw exception::NvmExceptionBadRequestSize();
	}
	return alignedBytes;
}

NVM_UINT64 wbem::logic::LayoutStepVolatile::getTotalVolatileBytes(
		const NVM_UINT64 &requestedBytes, const NVM_UINT64 &existingBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 bytes = 0;
	if (requestedBytes < BYTES_PER_GB)
	{
		throw exception::NvmExceptionBadRequestSize();
	}
	NVM_UINT64 totalVolatileBytes = existingBytes + requestedBytes;
	bytes = round_down(totalVolatileBytes, BYTES_PER_GB); // always 1 GiB aligned
	return bytes;
}

NVM_UINT64 wbem::logic::LayoutStepVolatile::roundDownVolatileToPMAlignment(
		const Dimm &dimm, MemoryAllocationLayout& layout,
		const NVM_UINT64 &volatileBytes, const NVM_UINT64 dimmBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 pmBytes = dimmBytes - volatileBytes;
	NVM_UINT64 pmAlignedBytes = pmBytes;
	if (pmBytes > 0)
	{
		pmAlignedBytes = round_up(pmBytes, PM_ALIGNMENT_GIB * BYTES_PER_GB);
		if (pmAlignedBytes > dimmBytes)
		{
			throw exception::NvmExceptionBadRequestSize();
		}
	}
	NVM_UINT64 alignedVolatileBytes = dimmBytes - pmAlignedBytes;
	if (alignedVolatileBytes < BYTES_PER_GB)
	{
		throw exception::NvmExceptionBadRequestSize();
	}
	return alignedVolatileBytes;
}

NVM_UINT64 wbem::logic::LayoutStepVolatile::roundUpVolatileToPMAlignment(
		const Dimm &dimm, MemoryAllocationLayout& layout,
		const NVM_UINT64 &volatileBytes, const NVM_UINT64 dimmBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	NVM_UINT64 pmBytes = dimmBytes - volatileBytes;
	NVM_UINT64 pmAlignedBytes = pmBytes;
	if (pmBytes > 0)
	{
		pmAlignedBytes = round_down(pmBytes, PM_ALIGNMENT_GIB * BYTES_PER_GB);
		if (pmAlignedBytes == 0)
		{
			throw exception::NvmExceptionBadRequestSize();
		}
	}
	NVM_UINT64 alignedVolatileBytes = dimmBytes - pmAlignedBytes;
	return alignedVolatileBytes;
}

NVM_UINT64 wbem::logic::LayoutStepVolatile::roundVolatileToNearestPMAlignment(
		const Dimm &dimm, MemoryAllocationLayout& layout,
		const NVM_UINT64 &volatileBytes, const NVM_UINT64 dimmBytes)
{
	LogEnterExit logging(__FUNCTION__, __FILE__, __LINE__);

	bool canRoundUp = true;
	bool canRoundDown = true;
	NVM_UINT64 roundedDownBytes = 0;
	NVM_UINT64 roundedUpBytes = 0;
	NVM_UINT64 roundedUpDiff = 0;
	NVM_UINT64 roundedDownDiff = 0;
	try
	{
		roundedUpBytes = roundUpVolatileToPMAlignment(dimm, layout, volatileBytes, dimmBytes);
		roundedUpDiff = roundedUpBytes - volatileBytes;
		if (roundedUpDiff < BYTES_PER_GB || roundedUpBytes > dimmBytes)
		{
			canRoundUp = false;
		}
	}
	catch (exception::NvmExceptionBadRequestSize &)
	{
		canRoundUp = false;
	}

	try
	{
		roundedDownBytes = roundDownVolatileToPMAlignment(dimm, layout, volatileBytes, dimmBytes);
		roundedDownDiff = volatileBytes - roundedDownBytes;
		if (roundedDownDiff < BYTES_PER_GB || roundedDownBytes > dimmBytes)
		{
			canRoundUp = false;
		}
	}
	catch (exception::NvmExceptionBadRequestSize &)
	{
		canRoundDown = false;
	}


	NVM_UINT64 alignedBytes = 0;
	if (canRoundUp && canRoundDown)
	{
		alignedBytes = roundedUpDiff < roundedDownDiff ? roundedUpBytes : roundedDownBytes;
	}
	else if (canRoundUp)
	{
		alignedBytes = roundedUpBytes;
	}
	else if (canRoundDown)
	{
		alignedBytes = roundedDownBytes;
	}
	else
	{
		throw exception::NvmExceptionBadRequestSize();
	}

	return alignedBytes;
}