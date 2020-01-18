Summary: Visualization tools for the Performance Co-Pilot toolkit
Name: pcp-gui
Version: 1.5.11
%define buildversion 1

Release: %{buildversion}%{?dist}
License: GPLv2+ and LGPLv2+ and LGPLv2+ with exceptions
URL: http://oss.sgi.com/projects/pcp
Group: Applications/System
Source0: pcp-gui-%{version}.src.tar.gz

BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: autoconf, bison, flex, gawk
BuildRequires: pcp >= 3.5.0, pcp-libs-devel >= 3.5.0
BuildRequires: desktop-file-utils
BuildRequires: qt4-devel >= 4.2

Requires: pcp

%description
Visualization tools for the Performance Co-Pilot toolkit.
Performance Co-Pilot (PCP) provides a framework and services to support
system-level performance monitoring and performance management.

The PCP GUI package primarily includes visualization tools for
monitoring systems using live and archived Performance Co-Pilot
(PCP) sources.

These tools have dependencies on graphics libraries which may or
may not be installed on server machines, so PCP GUI is delivered,
managed and maintained as a separate (source and binary) package
to the core PCP infrastructure.

#
# pcp-doc package
#
%package -n pcp-doc
Group: Documentation
Summary: Documentation and tutorial for the Performance Co-Pilot

%description -n pcp-doc
Documentation and tutorial for the Performance Co-Pilot
Performance Co-Pilot (PCP) provides a framework and services to support
system-level performance monitoring and performance management.

The pcp-doc package provides useful information on using and
configuring the Performance Co-Pilot (PCP) toolkit for system
level performance management.  It includes tutorials, HOWTOs,
and other detailed documentation about the internals of core
PCP utilities and daemons, and the PCP graphical tools.

#
# pcp-gui-testsuite
#
%package testsuite
License: GPLv2
Group: Development/Libraries
Summary: Performance Co-Pilot (PCP) GUI test suite
URL: http://oss.sgi.com/projects/pcp/

Requires: pcp-gui = %{version}-%{release}
Requires: pcp-testsuite

%description testsuite
Quality assurance test suite for Performance Co-Pilot (PCP) GUI.

%prep
%setup -q

%build
autoconf
%configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
export DIST_ROOT=$RPM_BUILD_ROOT
export PCP_PMSNAPCONTROL_PATH=/etc/pcp/pmsnap/control
make install DESTDIR=$RPM_BUILD_ROOT PCP_PMSNAPCONTROL_PATH=$PCP_PMSNAPCONTROL_PATH
rm -rf $RPM_BUILD_ROOT/usr/share/doc/pcp-gui
desktop-file-validate $RPM_BUILD_ROOT/%{_datadir}/applications/pmchart.desktop

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc README doc/CHANGES doc/COPYING
%{_bindir}/*
%{_libexecdir}/*
%{_localstatedir}/lib/pcp
%{_mandir}/man1/*
%{_sysconfdir}/pcp/pmsnap
%config(noreplace) %{_sysconfdir}/pcp/pmsnap
%{_datadir}/pcp
%{_datadir}/pixmaps/*
%{_datadir}/applications/pmchart.desktop

%files -n pcp-doc
%defattr(-,root,root,-)
%{_datadir}/doc/pcp-doc

%files testsuite
%defattr(-,pcpqa,pcpqa)
%{_localstatedir}/lib/pcp-gui/testsuite

%changelog
* Fri Nov 01 2013 Nathan Scott <nathans@redhat.com> - 1.5.11-1
- Updates to the PCP Programmers Guide.
- Fix pmchart value/units reporting issue on chart selection.

* Mon Sep 09 2013 Nathan Scott <nathans@redhat.com> - 1.5.10-1
- Updates to the PCP Users and Administrators Guide.
- Updates to the PCP Programmers Guide.
- Install a known-good pdf version of each book (BZ 990303).
- Improve pmchart hostname reporting via pmGetContextHostName.

* Sun Jul 28 2013 Nathan Scott <nathans@redhat.com> - 1.5.9-1
- Fix problems with pmchart Samples/Visible History (BZ 968825)
- Fix pmchart startup handling lack of metrics source (BZ 957007)
- Fix missing metric handling for archive View files (BZ 981140)
- UX improvements for the New/Edit Chart Apply button (BZ 957669)
- Resolve packaging issue where /usr/bin was installed (BZ 988176)
- Improve pmchart show-/hide-time-control mechanism (BZ 957002)
- Add a close button to pmchart Preferences dialog (BZ 922198)
- Fix pmchart Preferences dialog color scheme sigsegv (BZ 963505)

* Sat Apr 20 2013 Nathan Scott <nathans@redhat.com> - 1.5.8-2
- Update to latest PCP GUI sources.
- Build fix when building against older PCP headers.
- Force PCP_PMSNAPCONTROL_PATH to the preferred location.

* Fri Apr 19 2013 Nathan Scott <nathans@redhat.com> - 1.5.7-1
- Update to latest PCP GUI sources.
- Fix Save View memory corruption in pmchart (BZ 951173)

* Fri Oct 26 2012 Nathan Scott <nathans@redhat.com> - 1.5.6-1
- Update to latest PCP GUI sources.
- Introduces new pcp-gui-testsuite sub-package.

* Fri Jul 20 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.5.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Fri Mar 23 2012 Mark Goodwin <mgoodwin@redhat.com> - 1.5.5-1
- Update to latest sources (rolls in desktop patch too)

* Fri Jan 13 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.5.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Fri Dec 02 2011 Mark Goodwin <mgoodwin@redhat.com> - 1.5.2-2
- Patched pmchart.desktop, needed for RHEL builds

* Thu Dec 01 2011 Mark Goodwin <mgoodwin@redhat.com> - 1.5.2-1
- Fixed License and assorted minor rpmlint issues following review

* Mon Sep 19 2011 Harshula Jayasuriya <harshula@redhat.com> - 1.5.1-1
- Initial Fedora release
