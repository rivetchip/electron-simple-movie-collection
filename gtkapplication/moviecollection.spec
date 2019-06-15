Name: moviecollection
Version: # FIXME
Release: 1%{?dist}
Summary: # FIXME
License: # FIXME

Source0: %{name}-%{version}.tar.xz # FIXME

BuildRequires: meson
BuildRequires: gcc
BuildRequires: pkgconfig(gtk+-3.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(libcurl)

%description

%prep
%autosetup

%build
%meson
%meson_build

%install
%meson_install

%check
%meson_test

%files


%changelog
* Wed May 29 2019 meson <meson@example.com> - 
- 

