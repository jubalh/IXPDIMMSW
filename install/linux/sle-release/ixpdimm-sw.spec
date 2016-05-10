#
# spec file for package nvdimm-mgmt
#
# Copyright (c) 2016, Intel Corporation.
# Copyright (c) 2016 SUSE LINUX GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

%define product_name ixpdimm-sw 
%define upstream_name IXPDIMMSW
%define product_base_name ixpdimm
%define apiname lib%{product_base_name}-api01
%define devel_apiname lib%{product_base_name}-api-devel
%define corename lib%{product_base_name}-core01
%define devel_corename lib%{product_base_name}-core-devel
%define cliname lib%{product_base_name}-cli01
%define devel_cliname lib%{product_base_name}-cli-devel
%define monitorname %{product_base_name}-monitor
%define cimlibs lib%{product_base_name}-cim
%define devel_cimlibs lib%{product_base_name}-cim-devel
Name:           %{product_name}
Version:        01.00.00.2033
Release:        0
Summary:        API for development of %{product_name} management utilities
License:        BSD-3-Clause
Group:          System/Monitor
Url:            https://01.org/ixpdimm-sw
Source:         https://github.com/01org/%{upstream_name}/archive/v01.00.00.2033.tar.gz
Patch:          cppflags.patch
BuildRequires:  gcc-c++
BuildRequires:  intelnvmcimlibrary-devel
BuildRequires:  intelnvmclilibrary-devel
BuildRequires:  intelnvmi18nlibrary-devel
BuildRequires:  libctemplate-devel
BuildRequires:  libkmod-devel
BuildRequires:  libndctl-devel
BuildRequires:  libnuma-devel
BuildRequires:  openssl-devel
BuildRequires:  sblim-cmpi-devel
BuildRequires:  sqlite3-devel
Requires:       libndctl6 >= 51

%description
An application program interface (API) for configuring and managing
%{product_name}. Including basic inventory, capacity provisioning,
health monitoring, and troubleshooting.

%package -n %{apiname}
Summary:        API for development of %{product_name} management utilities
Group:          System/Monitor
Requires:       %{name} 

%description -n %{apiname}
An application program interface (API) for configuring and managing
%{product_name}. Including basic inventory, capacity provisioning,
health monitoring, and troubleshooting.

%package -n %{devel_apiname}
Summary:        Development files for %{name}
Group:          Development/Libraries/C and C++
Requires:       %{apiname} = %{version}

%description -n %{devel_apiname}
The %{name}-devel package contains header files for
developing applications that use %{name}.

%package -n %{corename}
Summary:        Development files for %{name}
Group:          System/Monitor
Requires:       %{name}

%description -n %{corename}
The %{corename} package contains libraries that support
other %{product_name} products.

%package -n %{devel_corename}
Summary:        Development files for %{corename}
Group:          Development/Libraries/C and C++
Requires:       %{corename} = %{version}

%description -n %{devel_corename}
Development files for %{corename} package.

%package -n %{cimlibs}
Summary:        CIM provider for %{name}
Group:          System/Monitor
Requires:       %{corename} = %{version}
Requires:       pywbem
Requires(post): pywbem
Requires(pre):  pywbem

%description -n %{cimlibs}
%{cimlibs} is a common information model (CIM) provider that exposes
%{product_name} as standard CIM objects in order to plug-in to various
common information model object managers (CIMOMS).

%package -n %{devel_cimlibs}
Summary:        Development files for %{cimlibs}
Group:          Development/Libraries/C and C++
Requires:       %{cimlibs} = %{version}

%description -n %{devel_cimlibs}
Development files for %{cimlibs} package.

%package -n %{monitorname}
Summary:        Daemon for monitoring the status of %{product_name}
Group:          System/Monitor
BuildRequires:  systemd-rpm-macros
Requires:       %{cimlibs} = %{version}
%{?systemd_requires}

%description -n %{monitorname}
A daemon for monitoring the health and status of %{product_name}

%package -n %{cliname}
Summary:        CLI for managment of %{product_name}
Group:          System/Monitor

%description -n %{cliname}
A command line interface (CLI) application for configuring and
managing %{product_name}. Including commands for basic inventory,
capacity provisioning, health monitoring, and troubleshooting.

%package -n %{devel_cliname}
Summary:        Development files for %{cliname}
Group:          Development/Libraries/C and C++
Requires:       %{cliname} = %{version}

%description -n %{devel_cliname}
Development files for %{cliname} package.

%prep
%setup -q -n %{upstream_name}-%{version}
%patch -p1

%build
#make #{?_smp_mflags} BUILDNUM=#{version} RELEASE=1 DATADIR=#{_datadir} LINUX_PRODUCT_NAME=#{product_name} CFLAGS_EXTERNAL="#{?cflag}"
make %{?_smp_mflags} BUILDNUM=%{version} RELEASE=1 DATADIR=%{_datadir} LINUX_PRODUCT_NAME=%{product_name} CFLAGS_EXTERNAL="%{?cflag}" CPPFLAGS_EXTERNAL="%{?cppflag}"

%install
make install RELEASE=1 RPM_ROOT=%{buildroot} LIB_DIR=%{_libdir} INCLUDE_DIR=%{_includedir} BIN_DIR=%{_bindir} DATADIR=%{_datadir} UNIT_DIR=%{_unitdir} LINUX_PRODUCT_NAME=%{product_name} SYSCONF_DIR=%{_sysconfdir} MANPAGE_DIR=%{_mandir}

%post -n %{corename} -p /sbin/ldconfig
%post -n %{cliname} -p /sbin/ldconfig
%post -n %{apiname} -p /sbin/ldconfig
%post -n %{cimlibs}
/sbin/ldconfig
if [ -x %{_sbindir}/cimserver ]
then
	cimserver --status &> /dev/null
	if [ $? -eq 0 ]
	then
	CIMMOF=cimmof
	else
    for repo in %{_localstatedir}/lib/Pegasus %{_localstatedir}/lib/pegasus %{_prefix}/local%{_localstatedir}/lib/pegasus %{_localstatedir}/local/lib/pegasus %{_localstatedir}/opt/tog-pegasus /opt/ibm/icc/cimom
    do
      if [ -d $repo/repository ]
      then
	  CIMMOF="cimmofl -R $repo"
      fi
    done
	fi
	for ns in interop root/interop root/PG_Interop;
	do
	   $CIMMOF -E -n$ns %{_datadir}/%{product_name}/Pegasus/mof/pegasus_register.mof &> /dev/null
	   if [ $? -eq 0 ]
	   then
			$CIMMOF -uc -n$ns %{_datadir}/%{product_name}/Pegasus/mof/pegasus_register.mof &> /dev/null
			$CIMMOF -uc -n$ns %{_datadir}/%{product_name}/Pegasus/mof/profile_registration.mof &> /dev/null
			break
	   fi
	done
	$CIMMOF -aE -uc -n root/intelwbem %{_datadir}/%{product_name}/Pegasus/mof/intelwbem.mof &> /dev/null
fi
if [ -x %{_sbindir}/sfcbd ]
then
	RESTART=0
	systemctl is-active sblim-sfcb.service &> /dev/null
	if [ $? -eq 0 ]
	then
		RESTART=1
		systemctl stop sblim-sfcb.service &> /dev/null
	fi

	sfcbstage -n root/intelwbem -r %{_datadir}/%{product_name}/sfcb/INTEL_NVDIMM.reg %{_datadir}/%{product_name}/sfcb/sfcb_intelwbem.mof
	sfcbrepos -f

	if [[ $RESTART -gt 0 ]]
	then
		systemctl start sblim-sfcb.service &> /dev/null
	fi
fi

%pre -n %{monitorname}
%service_add_pre ixpdimm-monitor.service

%post -n %{monitorname}
%service_add_post ixpdimm-monitor.service
/bin/systemctl --no-reload enable ixpdimm-monitor.service &> /dev/null || :
/bin/systemctl start ixpdimm-monitor.service &> /dev/null || :
exit 0

%postun -n %{corename} -p /sbin/ldconfig
%postun -n %{cimlibs} -p /sbin/ldconfig

%pre -n %{cimlibs}
# If upgrading, deregister old version
if [ "$1" -gt 1 ]; then
	RESTART=0
	if [ -x %{_sbindir}/cimserver ]
	then
		cimserver --status &> /dev/null
		if [ $? -gt 0 ]
		then
			RESTART=1
			cimserver enableHttpConnection=false enableHttpsConnection=false enableRemotePrivilegedUserAccess=false slp=false &> /dev/null
		fi
		cimprovider -d -m intelwbemprovider &> /dev/null
		cimprovider -r -m intelwbemprovider &> /dev/null
		mofcomp -v -r -n root/intelwbem %{_datadir}/%{product_name}/Pegasus/mof/intelwbem.mof &> /dev/null
		mofcomp -v -r -n root/intelwbem %{_datadir}/%{product_name}/Pegasus/mof/profile_registration.mof &> /dev/null
		if [[ $RESTART -gt 0 ]]
		then
			cimserver -s &> /dev/null
		fi
	fi
fi

%preun -n %{cimlibs}
RESTART=0
if [ -x %{_sbindir}/cimserver ]
then
	cimserver --status &> /dev/null
	if [ $? -gt 0 ]
	then
		RESTART=1
		cimserver enableHttpConnection=false enableHttpsConnection=false enableRemotePrivilegedUserAccess=false slp=false &> /dev/null
	fi
	cimprovider -d -m intelwbemprovider &> /dev/null
	cimprovider -r -m intelwbemprovider &> /dev/null
	mofcomp -r -n root/intelwbem %{_datadir}/%{product_name}/Pegasus/mof/intelwbem.mof &> /dev/null
	mofcomp -v -r -n root/intelwbem %{_datadir}/%{product_name}/Pegasus/mof/profile_registration.mof &> /dev/null
	if [[ $RESTART -gt 0 ]]
	then
		cimserver -s &> /dev/null
	fi
fi

if [ -x %{_sbindir}/sfcbd ]
then
	RESTART=0
	systemctl is-active sblim-sfcb.service &> /dev/null
	if [ $? -eq 0 ]
	then
		RESTART=1
		systemctl stop sblim-sfcb.service &> /dev/null
	fi

	sfcbunstage -n root/intelwbem -r INTEL_NVDIMM.reg sfcb_intelwbem.mof
	sfcbrepos -f

	if [[ $RESTART -gt 0 ]]
	then
		systemctl start sblim-sfcb.service &> /dev/null
	fi
fi

%preun -n %{monitorname}
%service_del_preun ixpdimm-monitor.service

%postun -n %{monitorname}
%service_del_postun ixpdimm-monitor.service

%postun -n %{cliname} -p /sbin/ldconfig

%postun -n %{apiname} -p /sbin/ldconfig

%files -n %{apiname}
%defattr(755,root,root,755)
%{_libdir}/libixpdimm-api.so.*
%dir %{_datadir}/%{product_name}
%attr(640,root,root) %{_datadir}/%{product_name}/*.pem
%attr(640,root,root) %config(noreplace) %{_datadir}/%{product_name}/*.dat*
%license LICENSE

%files -n %{devel_apiname}
%defattr(755,root,root,755)
%{_libdir}/libixpdimm-api.so
%attr(644,root,root) %{_includedir}/nvm_types.h
%attr(644,root,root) %{_includedir}/nvm_management.h
%license LICENSE

%files -n %{corename}
%defattr(755,root,root,755)
%{_libdir}/libixpdimm-core.so.*
%license LICENSE

%files -n %{devel_corename}
%{_libdir}/libixpdimm-core.so

%files -n %{cimlibs}
%defattr(755,root,root,755)
%dir %{_libdir}/cmpi/
%{_libdir}/cmpi/libixpdimm-cim.so.*
%dir %{_datadir}/%{product_name}/Pegasus
%dir %{_datadir}/%{product_name}/Pegasus/mof
%dir %{_datadir}/%{product_name}/sfcb
%attr(644,root,root) %{_datadir}/%{product_name}/sfcb/*.reg
%attr(644,root,root) %{_datadir}/%{product_name}/sfcb/*.mof
%attr(644,root,root) %{_datadir}/%{product_name}/Pegasus/mof/*.mof
%attr(644,root,root) %{_sysconfdir}/ld.so.conf.d/%{product_name}-%{_arch}.conf
%license LICENSE

%files -n %{devel_cimlibs}
%{_libdir}/cmpi/libixpdimm-cim.so

%files -n %{monitorname}
%defattr(755,root,root,755)
%{_bindir}/ixpdimm-monitor
%{_unitdir}/ixpdimm-monitor.service
%license LICENSE
%attr(644,root,root) %{_mandir}/man8/ixpdimm-monitor*

%files -n %{cliname}
%defattr(755,root,root,755)
%{_bindir}/ixpdimm-cli
%{_libdir}/libixpdimm-cli.so.*
%license LICENSE
%attr(644,root,root) %{_mandir}/man8/ixpdimm-cli*

%files -n %{devel_cliname}
%{_libdir}/libixpdimm-cli.so

%changelog
