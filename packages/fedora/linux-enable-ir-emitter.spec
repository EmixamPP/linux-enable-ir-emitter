%global   debug_package %{nil}

Name:     linux-enable-ir-emitter
Version:  4.4.0
Release:  1%{?dist}
Summary:  Enables infrared cameras that are not directly enabled out-of-the box
URL:      https://github.com/EmixamPP/%{name}
License:  MIT

Source0: linux-enable-ir-emitter
Source1: lib

BuildRequires: meson >= 0.61.0
BuildRequires: gcc-c++ >= 5.5.0
BuildRequires: opencv-devel >= 3.4.4
BuildRequires: libgcc
BuildRequires: libstdc++
BuildRequires: glibc

Requires: python3 >= 3.6.2

%description
Enables infrared cameras that are not directly enabled out-of-the box.

%build
cd %{SOURCE0}
meson setup build

%install
DESTDIR=%{buildroot} meson install -C %{SOURCE0}/build
install -Dm 644 %{SOURCE1}/* -t %{buildroot}%{_libdir}/%{name}/lib

%files
%{_libdir}/%{name}
%{_sysconfdir}/%{name}
%{_bindir}/%{name}
%{_datadir}/bash-completion/completions/%{name}
/usr/lib/systemd/system/linux-enable-ir-emitter.service

%post
# if SELinux is installed, fix denied access to /dev/video
which semanage &> /dev/null
if [ "$?" -eq 0 ]; then
    semanage fcontext -a -t bin_t %{_exec_prefix}/lib/%{name}/bin/execute-driver &> /dev/null
    semanage fcontext -a -t bin_t %{_exec_prefix}/lib/%{name}/bin/driver-generator &> /dev/null
    restorecon -v %{_libdir}/%{name}/bin/* &> /dev/null
fi

%preun
# remove SELinux permission
which semanage &> /dev/null
if [ "$?" -eq 0 ]; then
    semanage fcontext -d %{_exec_prefix}/lib/%{name}/bin/execute-driver &> /dev/null
    semanage fcontext -d %{_exec_prefix}/lib/%{name}/bin/driver-generator &> /dev/null
fi

systemctl disable linux-enable-ir-emitter &> /dev/null

%postun
if [ "$1" -eq 0 ]; then
    # delete python cache
    rm -rf %{_libdir}/%{name}/
    # udev rule
    rm -f /etc/udev/rules.d/99-linux-enable-ir-emitter.rules
fi

%changelog
* Fri Feb 17 2023 Maxime Dirksen <dev@emixam.be> - 4.4.0-1
- Total rework of the implementation
- Support multiple emitters camera
- Memorize broken instructions to skip them 
- Usage of /dev/v4l/by-path for persistence
- Drop distribution repositories support
* Thu Oct 6 2022 Maxime Dirksen <dev@emixam.be> - 4.1.6-1
- Fix bad_alloc on some distributions
* Tue Sep 13 2022 Maxime Dirksen <dev@emixam.be> - 4.1.5-1
- Fix boot service for custom device 
* Thu Aug 11 2022 Maxime Dirksen <dev@emixam.be> - 4.1.4-1
- Force V4l2 backend in opencv
- Improvement of driver generation
* Mon Jul 4 2022 Maxime Dirksen <dev@emixam.be> - 4.1.2-1
- Asynchronous camera triggering
- Fix camera triggering issue
- Fix device symlink boot service side effect
* Sun Jun 19 2022 Maxime Dirksen <dev@emixam.be> - 4.0.0-1
- Rework, optimization and improvement of driver generation 
- Remove manual configuration commands
- Remove option for integration into Howdy
* Thu Dec 9 2021 Maxime Dirksen <dev@emixam.be> - 3.2.5-1
- Tweak for integration into Howdy(https://github.com/boltgolt/howdy)  
- Bash auto completion
- Better systemd support
* Thu Nov 4 2021 Maxime Dirksen <dev@emixam.be> - 3.2.2-1
- Support any device path format
- Improve systemd service
* Sat Oct 23 2021 Maxime Dirksen <dev@emixam.be> - 3.2.0-1
- Multiple device support
* Thu Sep 23 2021 Maxime Dirksen <dev@emixam.be> - 3.1.1-1
- Limit in negative answers for a same pattern
* Wed Sep 22 2021 Maxime Dirksen <dev@emixam.be> - 3.1.0-1
- New configuration system
- Exit codes
- Change configuration file location
* Sun Aug 29 2021 Maxime Dirksen <dev@emixam.be> - 2.1.0-1
- New fix command, to resolve well know problems
- Systemd service file modified to prevent /dev/video file descriptor error
* Thu Aug 12 2021 Maxime Dirksen <dev@emixam.be> - 2.0.1-1
- Initial package
