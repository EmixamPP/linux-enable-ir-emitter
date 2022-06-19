%global   debug_package %{nil}

Name:     linux-enable-ir-emitter
Version:  4.0.0
Release:  1%{?dist}
Summary:  Enables infrared cameras that are not directly enabled out-of-the box
URL:      https://github.com/EmixamPP/%{name}
License:  MIT

Source0:  https://github.com/EmixamPP/%{name}/archive/refs/tags/%{version}.tar.gz

BuildRequires: make
BuildRequires: g++
BuildRequires: opencv-devel

Requires: usbutils
Requires: opencv
Requires: python3

%description
Enables infrared cameras that are not directly enabled out-of-the box.

%prep
%autosetup

%build
make -C sources/driver/

%install
# software
install -Dm 644 sources/*.py -t %{buildroot}%{_libdir}/%{name}/
install -Dm 644 sources/command/*.py -t %{buildroot}%{_libdir}/%{name}/command/
install -Dm 755 sources/driver/driver-generator -t %{buildroot}%{_libdir}/%{name}/driver/
install -Dm 755 sources/driver/execute-driver -t %{buildroot}%{_libdir}/%{name}/driver/

# executable
chmod 755 %{buildroot}%{_libdir}/%{name}/%{name}.py
mkdir -p %{buildroot}%{_bindir}/
ln -fs %{_libdir}/%{name}/%{name}.py %{buildroot}%{_bindir}/%{name}

# auto complete for bash
install -Dm 644 sources/autocomplete/%{name} -t %{buildroot}%{_datadir}/bash-completion/completions/

# drivers folder
mkdir -p %{buildroot}%{_sysconfdir}/%{name}/

%files
%license LICENSE
%doc README.md
%{_libdir}/%{name}/
%{_sysconfdir}/%{name}/
%{_bindir}/%{name}
%{_datadir}/bash-completion/completions/%{name}

%post
# if SELinux is installed, fix denied access to /dev/video
which semanage &> /dev/null
if [ "$?" -eq 0 ]; then
    semanage fcontext -a -t bin_t %{_exec_prefix}/lib/%{name}/driver/execute-driver
    semanage fcontext -a -t bin_t %{_exec_prefix}/lib/%{name}/driver/driver-generator
    restorecon -v %{_libdir}/%{name}/driver/* 1> /dev/null
fi

# support update v3 to v4 
if [ "$1" -eq 2 ] && [ -f %{_sysconfdir}/%{name}.yaml ]; then 
    python %{_libdir}/%{name}/migrate-v3.py
    rm -f %{_sysconfdir}/%{name}.yaml
fi

%postun
if [ "$1" -eq 0 ]; then
    # remove SeLinux permission
    which semanage &> /dev/null
    if [ "$?" -eq 0 ] && [ "$1" -eq 0 ]; then
        semanage fcontext -d %{_exec_prefix}/lib//%{name}/driver/execute-driver
        semanage fcontext -d %{_exec_prefix}/lib//%{name}/driver/driver-generator
    fi

    # delete python cache
    rm -rf %{_libdir}/%{name}/

    # delete drivers
    rm -rf %{_sysconfdir}/%{name}/ # v4 and higher 
    rm -f %{_sysconfdir}/%{name}.yaml # v3

    # delete systemd service
    systemctl disable linux-enable-ir-emitter &> /dev/null
    rm -f /usr/lib/systemd/system/linux-enable-ir-emitter.service
    rm -f /etc/udev/rules.d/99-linux-enable-ir-emitter.rules
fi

%changelog
* Sat Jun 19 2022 Maxime Dirksen <copr@emixam.be> - 4.0.0-1
- Rework, optimization and improvement of driver generation 
- Remove manual configuration commands
- Remove option for integration into Howdy
* Thu Dec 9 2021 Maxime Dirksen <copr@emixam.be> - 3.2.5-1
- Tweak for integration into Howdy(https://github.com/boltgolt/howdy)  
- Bash auto completion
- Better systemd support
* Thu Nov 4 2021 Maxime Dirksen <copr@emixam.be> - 3.2.2-1
- Support any device path format
- Improve systemd service
* Sat Oct 23 2021 Maxime Dirksen <copr@emixam.be> - 3.2.0-1
- Multiple device support
* Thu Sep 23 2021 Maxime Dirksen <copr@emixam.be> - 3.1.1-1
- Limit in negative answers for a same pattern
* Wed Sep 22 2021 Maxime Dirksen <copr@emixam.be> - 3.1.0-1
- New configuration system
- Exit codes
- Change configuration file location
* Sun Aug 29 2021 Maxime Dirksen <copr@emixam.be> - 2.1.0-1
- New fix command, to resolve well know problems
- Systemd service file modified to prevent /dev/video file descriptor error
* Thu Aug 12 2021 Maxime Dirksen <copr@emixam.be> - 2.0.1-1
- Initial package
