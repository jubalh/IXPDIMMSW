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
 * This file contains the base class for indication factories as well as
 * providing facade functions for getting all indications for a given event.
 */

#ifndef    _WBEM_FRAMEWORK_NVMINDICATIONFACTORY_H_
#define    _WBEM_FRAMEWORK_NVMINDICATIONFACTORY_H_


#include <intel_cim_framework/Instance.h>
#include <intel_cim_framework/Exception.h>
#include <nvm_management.h>

namespace wbem
{
namespace indication
{
class NvmIndicationFactory
{
public:

	virtual ~NvmIndicationFactory()	{	}

	static void createIndications(struct event *pEvent, std::vector<wbem::framework::Instance *> &indications);

	virtual framework::Instance *createIndication(struct event *pEvent) throw (framework::Exception) = 0;

private:
	static void getFactories(std::vector<NvmIndicationFactory *> &factories);
};
}
}

#endif // _WBEM_FRAMEWORK_NVMINDICATIONFACTORY_H_