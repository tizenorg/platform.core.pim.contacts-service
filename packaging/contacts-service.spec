Name:       contacts-service
Summary:    Contacts Service
Version:    0.13.0
Release:    1
Group:      Social & Content/Pim
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    contacts-service.service
Source2:    contacts-service.socket
Source3:    contacts-service2.manifest
BuildRequires:  cmake
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
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libsmack)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-session)
BuildRequires:  pkgconfig(cynara-creds-socket)
BuildRequires:  pkgconfig(capi-system-info)
Requires(post): /usr/bin/sqlite3, /bin/chmod, /bin/chown
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Contacts Service Library

%package -n contacts-service2
Summary:	       New Contacts service library
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
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DTZ_SYS_ETC=%TZ_SYS_ETC \
-DENABLE_SIM_FEATURE:BOOL=ON \
-DENABLE_LOG_FEATURE:BOOL=ON \
-DMAJORVER=${MAJORVER} \
-DFULLVER=%{version}
%__make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_unitdir_user}/default.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_unitdir_user}/contacts-service.service
ln -s ../contacts-service.service %{buildroot}%{_unitdir_user}/default.target.wants/contacts-service.service

mkdir -p %{buildroot}%{_unitdir_user}/sockets.target.wants
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir_user}/contacts-service.socket
ln -s ../contacts-service.socket %{buildroot}%{_unitdir_user}/sockets.target.wants/contacts-service.socket

mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/contacts-service2
cp LICENSE.APLv2 %{buildroot}/usr/share/license/contacts-service2-devel

%post -n contacts-service2
/sbin/ldconfig

vconftool set -t int file/private/contacts-service/default_lang 0 -g 5000 -s contacts-service::vconf-private
vconftool set -t int db/contacts-svc/name_sorting_order 0 -g 5000 -s contacts-service::vconf
vconftool set -t int db/contacts-svc/name_display_order 0 -g 5000 -s contacts-service::vconf
vconftool set -t int db/contacts-svc/phonenumber_min_match_digit 8 -g 5000 -s contacts-service::vconf

%postun -p /sbin/ldconfig

%files -n contacts-service2
%manifest contacts-service2.manifest
%defattr(-,root,root,-)
%{_libdir}/libcontacts-service2.so.*
%{_bindir}/contacts-service-ipcd*
%{_unitdir_user}/contacts-service.service
%{_unitdir_user}/default.target.wants/contacts-service.service
%{_unitdir_user}/sockets.target.wants/contacts-service.socket
%{_unitdir_user}/contacts-service.socket
/usr/share/license/%{name}2

%files -n contacts-service2-devel
%defattr(-,root,root,-)
%{_libdir}/libcontacts-service2.so
%{_libdir}/pkgconfig/contacts-service2.pc
%{_includedir}/contacts-svc/contacts.h
%{_includedir}/contacts-svc/contacts_*.h
/usr/share/license/contacts-service2-devel
