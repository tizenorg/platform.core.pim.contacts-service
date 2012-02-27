Name:       contacts-service
Summary:    Contacts Service
Version:    0.4.2
Release:    1
Group:      TO_BE/FILLED_IN
License:    Apache 2.0
Source0:    %{name}-%{version}.tar.gz
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
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post 
/sbin/ldconfig
contacts-svc-helper schema
chown :6005 /opt/dbspace/.contacts-svc.db
chown :6005 /opt/dbspace/.contacts-svc.db-journal

vconftool set -t int db/service/contacts/default_lang 1
vconftool set -t int db/service/contacts/name_sorting_order 0 -g 6005
vconftool set -t int db/service/contacts/name_display_order 0 -g 6005

#mkdir -p %{buildroot}%{_sysconfdir}/rc.d/
#mkdir -p %{buildroot}%{_sysconfdir}/rc.d/rc3.d/
#mkdir -p %{buildroot}%{_sysconfdir}/rc.d/rc5.d/
#ln -s %{buildroot}/%{_sysconfdir}/init.d/contacts-svc-helper.sh %{buildroot}%/{_sysconfdir}/rc.d/rc3.d/S50contacts-svc-helper
#ln -s %{buildroot}/%{_sysconfdir}/init.d/contacts-svc-helper.sh %{buildroot}%/{_sysconfdir}/rc.d/rc5.d/S50contacts-svc-helper

mkdir -p /etc/rc.d/
mkdir -p /etc/rc.d/rc3.d/
mkdir -p /etc/rc.d/rc5.d/
ln -s /etc/init.d/contacts-svc-helper.sh /etc/rc.d/rc3.d/S50contacts-svc-helper
ln -s /etc/init.d/contacts-svc-helper.sh /etc/rc.d/rc5.d/S50contacts-svc-helper


%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%attr(0660,root,db_contact)/opt/data/contacts-svc/.CONTACTS_SVC_*
%{_libdir}/libcontacts-service.so*
%{_bindir}/contacts-svc-helper
%attr(0755,root,root) /etc/rc.d/init.d/contacts-svc-helper.sh

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_libdir}/pkgconfig/contacts-service.pc
%{_includedir}/contacts-svc/*.h
