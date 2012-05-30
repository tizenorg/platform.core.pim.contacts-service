Name:       contacts-service
Summary:    Contacts Service
Version: 0.6.1
Release:    10
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: packaging/contacts-service.manifest 
Requires(post): /sbin/ldconfig
Requires(post): /usr/bin/sqlite3
Requires(post): /usr/bin/vconftool
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  vconf-keys-devel
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(tapi)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(icu-i18n)

%description
Contacts Service Library

%package devel
Summary:    Contacts Service  (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Contacts Service Library (devel)

%prep
%setup -q


%build
cp %{SOURCE1001} .
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/etc/rc.d/rc3.d/
mkdir -p %{buildroot}/etc/rc.d/rc5.d/
ln -s ../init.d/contacts-svc-helper.sh %{buildroot}/etc/rc.d/rc3.d/S50contacts-svc-helper
ln -s ../init.d/contacts-svc-helper.sh %{buildroot}/etc/rc.d/rc5.d/S50contacts-svc-helper

%post
/sbin/ldconfig

# from contacts-service-bin.postinst
contacts-svc-helper schema
chown :6005 /opt/dbspace/.contacts-svc.db
chown :6005 /opt/dbspace/.contacts-svc.db-journal
chown :6005 -R /opt/data/contacts-svc/img
chown :6005 /opt/data/contacts-svc/.CONTACTS_SVC_*

chmod 660 /opt/dbspace/.contacts-svc.db
chmod 660 /opt/dbspace/.contacts-svc.db-journal
chmod 770 -R /opt/data/contacts-svc/img/
chmod 660 /opt/data/contacts-svc/.CONTACTS_SVC_*
vconftool set -t int db/service/contacts/default_lang 1

# from libcontacts-service.postinst
vconftool set -t int db/service/contacts/name_sorting_order 0 -g 6005
vconftool set -t int db/service/contacts/name_display_order 0 -g 6005


%postun -p /sbin/ldconfig


%files
%manifest contacts-service.manifest
%defattr(-,root,root,-)
%{_libdir}/libcontacts-service.so*
%{_bindir}/contacts-svc-helper*
%attr(0755,root,root) /etc/rc.d/init.d/contacts-svc-helper.sh
/etc/rc.d/rc*.d/S50contacts-svc-helper
/opt/data/contacts-svc/.CONTACTS_SVC_*
/opt/data/contacts-svc/img/*

%files devel
%manifest contacts-service.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_libdir}/pkgconfig/contacts-service.pc
%{_includedir}/contacts-svc/*.h
