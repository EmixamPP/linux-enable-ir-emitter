%global   debug_package %{nil}

Name:     linux-enable-ir-emitter
Version:  3.2.5
Release:  1%{?dist}
Summary:  Enables infrared cameras that are not directly enabled out-of-the box
URL:      https://github.com/EmixamPP/%{name}
License:  MIT

Source0:  https://github.com/EmixamPP/%{name}/archive/refs/tags/%{version}.tar.gz

BuildRequires: make
BuildRequires: gcc
Requires: python3-opencv
Requires: python3-pyyaml
Requires: usbutils

%description
Enables infrared cameras that are not directly enabled out-of-the box.

%prep
%autosetup

%build
make -C sources/driver/uvc

%install
# software
install -Dm 644 sources/*.py -t %{buildroot}%{_libdir}/%{name}/
install -Dm 644 sources/command/*.py -t %{buildroot}%{_libdir}/%{name}/command/
install -Dm 644 sources/driver/*.py -t %{buildroot}%{_libdir}/%{name}/driver/

install -Dm 755 sources/driver/uvc/*query -t %{buildroot}%{_libdir}/%{name}/driver/uvc/
install -Dm 755 sources/driver/uvc/*query.o -t %{buildroot}%{_libdir}/%{name}/driver/uvc/

# executable
chmod +x %{buildroot}%{_libdir}/%{name}/%{name}.py
mkdir -p %{buildroot}%{_bindir}/
ln -fs %{_libdir}/%{name}/%{name}.py %{buildroot}%{_bindir}/%{name}

# auto complete for bash
install -Dm 644 sources/autocomplete/%{name} -t %{buildroot}%{_datadir}/bash-completion/completions/

%files
%license LICENSE
%doc README.md
%ghost /usr/lib/systemd/system/linux-enable-ir-emitter.service
%{_libdir}/%{name}/
%{_bindir}/%{name}
%{_datadir}/bash-completion/completions/%{name}

%postun
# if last uninstallation (not after update)
if [ "$1" -eq 0 ]; then
    # delete python cache
    rm -rf %{_libdir}/%{name}/

    # delete driver
    rm -f /etc/%{name}.yaml

    # delete systemd service
    systemctl disable linux-enable-ir-emitter.service
    rm -f /usr/lib/systemd/system/linux-enable-ir-emitter.service
    rm -f /etc/udev/rules.d/99-linux-enable-ir-emitter-rules
fi

%changelog
* Sun Dec 9 2021 Maxime dirksen <emixampp@fedoraproject.org> - 3.2.5-1
- Tweak for integration into Howdy(https://github.com/boltgolt/howdy)  
- Bash auto completition
- Better systemd support
* Thu Nov 4 2021 Maxime dirksen <emixampp@fedoraproject.org> - 3.2.2-1
- Support any device path format
- Improve systemd service
* Sat Oct 23 2021 Maxime dirksen <emixampp@fedoraproject.org> - 3.2.0-1
- Multiple device support
* Thu Sep 23 2021 Maxime dirksen <emixampp@fedoraproject.org> - 3.1.1-1
- Limit in negative answers for a same pattern
* Wed Sep 22 2021 Maxime dirksen <emixampp@fedoraproject.org> - 3.1.0-1
- New configuration system
- Exit codes
- Change configuration file location
* Sun Aug 29 2021 Maxime Dirksen <emixampp@fedoraproject.org> - 2.1.0-1
- New fix command, to resolve well know problems
- Systemd service file modified to prevent /dev/video file descriptor error
* Thu Aug 12 2021 Maxime Dirksen <emixampp@fedoraproject.org> - 2.0.1-1
- Initial package
