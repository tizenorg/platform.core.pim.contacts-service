Name:       contacts-service
Summary:    Contacts Service
Version: 0.9.84.2
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    contacts-service.service
Source1001: 	contacts-service.manifest
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
Requires(post): /usr/bin/sqlite3, /bin/chmod, /bin/chown
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(post): telephony-daemon
Requires(postun): /sbin/ldconfig

%description
Contacts Service Library

%package -n contacts-service2
Summary:	New Contacts service library
Group:	Development/Libraries
Requires(post): telephony-daemon
Requires(post): libprivilege-control-conf

%description -n contacts-service2
New Contact Serivce Library


%package -n contacts-service2-devel
Summary:    New Contacts Service  (devel)
Group:      Development/Libraries
Requires:   %{name}2 = %{version}-%{release}

%description -n contacts-service2-devel
New Contacts Service Library (devel)

%prep
%setup -q
cp %{SOURCE1001} .


%build
%cmake .
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_sysconfdir}/rc.d/rc3.d/
mkdir -p %{buildroot}%{_sysconfdir}/rc.d/rc5.d/
ln -s ../init.d/contacts-service-ipcd.sh %{buildroot}%{_sysconfdir}/rc.d/rc3.d/S50contacts-svc-helper
ln -s ../init.d/contacts-service-ipcd.sh %{buildroot}%{_sysconfdir}/rc.d/rc5.d/S50contacts-svc-helper

mkdir -p %{buildroot}/usr/lib/systemd/user/tizen-middleware.target.wants
install -m 0644 %SOURCE1 %{buildroot}/usr/lib/systemd/user/contacts-service.service
ln -s ../contacts-service.service %{buildroot}/usr/lib/systemd/user/tizen-middleware.target.wants/contacts-service.service


%post -n contacts-service2
/sbin/ldconfig

chown :6005 /opt/usr/data/contacts-svc
chown :6005 /opt/usr/dbspace/.contacts-svc.db
chown :6005 /opt/usr/dbspace/.contacts-svc.db-journal
chown :6005 -R /opt/usr/data/contacts-svc/img
chown :6005 /opt/usr/data/contacts-svc/.CONTACTS_SVC_*_CHANGED

chmod 660 /opt/usr/dbspace/.contacts-svc.db
chmod 660 /opt/usr/dbspace/.contacts-svc.db-journal
chmod 775 /opt/usr/data/contacts-svc
chmod 770 -R /opt/usr/data/contacts-svc/img/
chmod 660 /opt/usr/data/contacts-svc/.CONTACTS_SVC_*

vconftool set -t int file/private/contacts-service/default_lang 0 -g 6005 -s contacts-service::vconf-private
vconftool set -t int db/contacts-svc/name_sorting_order 0 -g 6005 -s contacts-service::vconf
vconftool set -t int db/contacts-svc/name_display_order 0 -g 6005 -s contacts-service::vconf
vconftool set -t int db/contacts-svc/phonenumber_min_match_digit 8 -g 6005 -s contacts-service::vconf


%postun -n contacts-service2 -p /sbin/ldconfig


%postun -p /sbin/ldconfig

%files -n contacts-service2
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libcontacts-service2.so.*
%{_bindir}/contacts-service-ipcd*
/etc/rc.d/rc*.d/S50contacts-svc-helper
/opt/usr/data/contacts-svc/.CONTACTS_SVC_*
/opt/usr/data/contacts-svc/img/*
%attr(0755,root,root) /etc/rc.d/init.d/contacts-service-ipcd.sh
/usr/lib/systemd/user/contacts-service.service
/usr/lib/systemd/user/tizen-middleware.target.wants/contacts-service.service
%config(noreplace) /opt/usr/dbspace/.contacts-svc.db*
/opt/etc/smack/accesses.d/%{name}2.rule

%files -n contacts-service2-devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libcontacts-service2.so
%{_libdir}/pkgconfig/contacts-service2.pc
%{_includedir}/contacts-svc/contacts.h
%{_includedir}/contacts-svc/contacts_*.h