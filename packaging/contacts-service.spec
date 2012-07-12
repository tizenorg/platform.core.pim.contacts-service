Name:           contacts-service
Version:        0.6.1
Release:        10
License:        Apache-2.0
Summary:        Contacts Service
Group:          PIM/Contacts
Source0:        %{name}-%{version}.tar.gz
Source1:        contacts-server.service
Source1001:     contacts-service.manifest
BuildRequires:  cmake
BuildRequires:  vconf-keys-devel
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(tapi)
BuildRequires:  pkgconfig(vconf)
Requires(post): /usr/bin/sqlite3, /bin/chmod, /bin/chown
Requires(post): /usr/bin/vconftool
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Contacts Service Library

%package devel
Summary:        Contacts Service Development files
Group:          Development/Libraries
Requires:       %{name} = %{version}

%description devel
Development files and headers for the Contacts Service Library

%prep
%setup -q


%build
cp %{SOURCE1001} .
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}


make %{?_smp_mflags}

%install
%make_install

mkdir -p %{buildroot}%{_sysconfdir}/rc.d/rc3.d/
mkdir -p %{buildroot}%{_sysconfdir}/rc.d/rc5.d/
ln -s ../init.d/contacts-svc-helper.sh %{buildroot}%{_sysconfdir}/rc.d/rc3.d/S50contacts-svc-helper
ln -s ../init.d/contacts-svc-helper.sh %{buildroot}%{_sysconfdir}/rc.d/rc5.d/S50contacts-svc-helper


mkdir -p %{buildroot}%{_libdir}/systemd/user/tizen-middleware.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_libdir}/systemd/user/contacts-server.service
ln -s ../contacts-server.service %{buildroot}%{_libdir}/systemd/user/tizen-middleware.target.wants/contacts-server.service


%post
/sbin/ldconfig

# from contacts-service-bin.postinst
contacts-svc-helper schema
chown :6005 /opt/data/contacts-svc
chown :6005 /opt/dbspace/.contacts-svc.db
chown :6005 /opt/dbspace/.contacts-svc.db-journal
chown :6005 -R /opt/data/contacts-svc/img
chown :6005 /opt/data/contacts-svc/.CONTACTS_SVC_*

chmod 660 /opt/dbspace/.contacts-svc.db
chmod 660 /opt/dbspace/.contacts-svc.db-journal
chmod 775 /opt/data/contacts-svc
chmod 770 -R /opt/data/contacts-svc/img/
chmod 660 /opt/data/contacts-svc/.CONTACTS_SVC_*
vconftool set -t int db/service/contacts/default_lang 1
vconftool set -t int db/service/contacts/name_sorting_order 0 -g 6005
vconftool set -t int db/service/contacts/name_display_order 0 -g 6005


%postun -p /sbin/ldconfig


%files
%manifest contacts-service.manifest
%{_libdir}/libcontacts-service.so*
%{_bindir}/contacts-svc-helper*
%attr(0755,root,root) /etc/rc.d/init.d/contacts-svc-helper.sh
/etc/rc.d/rc*.d/S50contacts-svc-helper
%attr(660,root,db_contacts) /opt/data/contacts-svc/.CONTACTS_SVC_*
%attr(770,root,db_contacts) /opt/data/contacts-svc/img/*
%{_libdir}/systemd/user/contacts-server.service
%{_libdir}/systemd/user/tizen-middleware.target.wants/contacts-server.service

%files devel
%manifest contacts-service.manifest
%{_libdir}/pkgconfig/contacts-service.pc
%{_includedir}/contacts-svc/*.h
