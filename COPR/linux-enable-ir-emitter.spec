%global   debug_package %{nil}

Name:     linux-enable-ir-emitter
Version:  2.0.1
Release:  1%{?dist}
Summary:  Enables infrared cameras that are not directly enabled out-of-the box

URL:      https://github.com/EmixamPP/%{name}
License:  MIT
Source0:  https://github.com/EmixamPP/%{name}/archive/refs/tags/%{version}.tar.gz

BuildRequires: make
BuildRequires: gcc
Requires: python3-opencv
Requires: python3-pyyaml
Requires: nano

%description
Enables infrared cameras that are not directly enabled out-of-the box.

%prep
%autosetup

%build
make -C sources

%install
# software
mkdir -p %{buildroot}%{_libdir}/%{name}
install -Dm 755 sources/enable-ir-emitter -t %{buildroot}%{_libdir}/%{name}
install -Dm 644 sources/config.yaml -t %{buildroot}%{_libdir}/%{name}
install -Dm 644 sources/*.py -t %{buildroot}%{_libdir}/%{name}

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
%{_libdir}/%{name}/
%{_prefix}/lib/systemd/system/%{name}.service
%{_bindir}/%{name}

%changelog
* Thu Aug 12 2021 Maxime Dirksen <emixampp@fedoraproject.org> - 2.0.1-1
- Initial package
