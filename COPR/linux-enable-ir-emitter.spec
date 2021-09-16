%global   debug_package %{nil}

Name:     linux-enable-ir-emitter
Version:  3.0.0
Release:  1%{?dist}
Summary:  Enables infrared cameras that are not directly enabled out-of-the box

URL:      https://github.com/EmixamPP/%{name}
License:  MIT
Source0:  https://github.com/EmixamPP/%{name}/archive/refs/tags/%{version}.tar.gz

BuildRequires: make
BuildRequires: gcc
Requires: python3-opencv
Requires: python3-pyyaml

%description
Enables infrared cameras that are not directly enabled out-of-the box.

%prep
%autosetup

%build
make -C sources/uvc

%install
# software
mkdir -p %{buildroot}%{_libdir}/%{name}
install -Dm 644 sources/*.py -t %{buildroot}%{_libdir}/%{name}/

mkdir -p %{buildroot}%{_libdir}/%{name}/uvc
install -Dm 755 sources/uvc/*query -t %{buildroot}%{_libdir}/%{name}/uvc/
install -Dm 755 sources/uvc/*query.o -t %{buildroot}%{_libdir}/%{name}/uvc/

mkdir -p %{buildroot}%{_libdir}/%{name}/command
install -Dm 644 sources/command/*.py -t %{buildroot}%{_libdir}/%{name}/command/

# boot service
mkdir -p %{buildroot}%{_prefix}/lib/systemd/system/
install -Dm 644 sources/%{name}.service -t %{buildroot}%{_prefix}/lib/systemd/system/

# executable
mkdir -p %{buildroot}%{_bindir}
chmod +x %{buildroot}%{_libdir}/%{name}/%{name}.py
ln -fs %{_libdir}/%{name}/%{name}.py %{buildroot}%{_bindir}/%{name}

%files
%license LICENSE
%doc README.md
%{_libdir}/%{name}
%{_prefix}/lib/systemd/system/%{name}.service
%{_bindir}/%{name}

%postun
# delete files added after installation
rm -rf %{_libdir}/%{name}/

%changelog
* Thu Sep 16 2021 Maxime dirksen <emixampp@fedoraproject.org> - 3.0.0-1
- New configuration system
- Custom exit code
* Sun Aug 29 2021 Maxime Dirksen <emixampp@fedoraproject.org> - 2.1.0-1
- New fix command, to resolve well know problems
- Systemd service file modified to prevent /dev/video file descriptor  error
* Thu Aug 12 2021 Maxime Dirksen <emixampp@fedoraproject.org> - 2.0.1-1
- Initial package
