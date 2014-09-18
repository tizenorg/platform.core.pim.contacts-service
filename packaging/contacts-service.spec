Name:       contacts-service
Summary:    Contacts Service
Version:    0.10.3
Release:    1
Group:      Social & Contents/Pim
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    contacts-service.service
Source2:    contacts-service.socket
Source3:    contacts-service2.manifest
BuildRequires:  cmake
BuildRequires:  vconf-keys-devel
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(tapi)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-image-util)
BuildRequires:  pkgconfig(pims-ipc)
BuildRequires:  pkgconfig(accounts-svc)
BuildRequires:  pkgconfig(badge)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libsmack)
BuildRequires:  pkgconfig(security-server)
BuildRequires:  pkgconfig(libtzplatform-config)
Requires(post): /usr/bin/sqlite3, /bin/chmod, /bin/chown
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Contacts Service Library

%package -n contacts-service2
Summary:        New Contacts service library
Requires(post): libprivilege-control-conf

%description -n contacts-service2
New Contact Serivce Library files

%package -n contacts-service2-devel
Summary:    New Contacts Service  (devel)
Requires:   %{name}2 = %{version}-%{release}

%description -n contacts-service2-devel
New Contacts Service Library (devel) files

%prep
%setup -q
cp %{SOURCE3} .


%build
%cmake . -DTZ_SYS_ETC=%TZ_SYS_ETC
%__make %{?_smp_mflags}


%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_unitdir_user}/tizen-middleware.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_unitdir_user}/contacts-service.service
ln -s ../contacts-service.service %{buildroot}%{_unitdir_user}/tizen-middleware.target.wants/contacts-service.service

mkdir -p %{buildroot}%{_unitdir_user}/sockets.target.wants
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir_user}/contacts-service.socket
ln -s ../contacts-service.socket %{buildroot}%{_unitdir_user}/sockets.target.wants/contacts-service.socket


%post -n contacts-service2
/sbin/ldconfig

vconftool set -t int file/private/contacts-service/default_lang 0 -g 6005 -s contacts-service::vconf-private
vconftool set -t int db/contacts-svc/name_sorting_order 0 -g 6005 -s contacts-service::vconf
vconftool set -t int db/contacts-svc/name_display_order 0 -g 6005 -s contacts-service::vconf
vconftool set -t int db/contacts-svc/phonenumber_min_match_digit 8 -g 6005 -s contacts-service::vconf


%postun -n contacts-service2 -p /sbin/ldconfig


%postun -p /sbin/ldconfig

%files -n contacts-service2
%manifest contacts-service2.manifest
%defattr(-,root,root,-)
%{_libdir}/libcontacts-service2.so.*
%{_bindir}/contacts-service-ipcd*
%{_unitdir_user}/contacts-service.service
%{_unitdir_user}/tizen-middleware.target.wants/contacts-service.service
%{_unitdir_user}/sockets.target.wants/contacts-service.socket
%{_unitdir_user}/contacts-service.socket

%files -n contacts-service2-devel
%defattr(-,root,root,-)
%{_libdir}/libcontacts-service2.so
%{_libdir}/pkgconfig/contacts-service2.pc
%{_includedir}/contacts-svc/contacts.h
%{_includedir}/contacts-svc/contacts_*.h
