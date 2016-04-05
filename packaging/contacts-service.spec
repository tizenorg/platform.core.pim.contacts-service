Name:       contacts-service
Summary:    Contacts Service
Version:    0.13.49
Release:    0
Group:      Social & Content/Service
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    %{name}.service
Source2:    %{name}.socket
Source1001: %{name}.manifest
BuildRequires: cmake
BuildRequires: pkgconfig(db-util)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(sqlite3)
BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(capi-base-common)
BuildRequires: pkgconfig(capi-media-image-util)
BuildRequires: pkgconfig(pims-ipc)
BuildRequires: pkgconfig(accounts-svc)
BuildRequires: pkgconfig(libexif)
BuildRequires: pkgconfig(libsmack)
BuildRequires: pkgconfig(libtzplatform-config)
BuildRequires: pkgconfig(cynara-client)
BuildRequires: pkgconfig(cynara-session)
BuildRequires: pkgconfig(cynara-creds-socket)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(icu-uc)
BuildRequires: pkgconfig(phonenumber-utils)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Contacts Service Library

%package -n contacts-service2
Summary:	New Contacts service library

%description -n contacts-service2
New Contact Serivce 2 Client Library


%package -n contacts-service2-devel
Summary:  New Contacts Service(devel)
Group:    Social & Content/Development
Requires: %{name}2 = %{version}-%{release}

%description -n contacts-service2-devel
New Contacts Service Library (devel) files

%package -n contacts-service2-test
Summary:  New Contacts Service(test)
Group:    Social & Content/Testing
Requires: %{name}2 = %{version}-%{release}

%description -n contacts-service2-test
New Contacts Service Test Program


%prep
%setup -q
chmod g-w %_sourcedir/*
cp %{SOURCE1001} ./%{name}.manifest
cp %{SOURCE1001} ./%{name}-test.manifest


%build
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DCMAKE_VERBOSE_MAKEFILE=OFF -DMAJORVER=${MAJORVER} -DFULLVER=%{version} -DBIN_INSTALL_DIR:PATH=%{_bindir} \
		-DTZ_SYS_ETC=%TZ_SYS_ETC -DENABLE_LOG_FEATURE:BOOL=ON

%__make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_unitdir_user}/default.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_unitdir_user}/contacts-service.service

mkdir -p %{buildroot}%{_unitdir_user}/sockets.target.wants
install -m 0644 %SOURCE2 %{buildroot}%{_unitdir_user}/contacts-service.socket
ln -s ../contacts-service.socket %{buildroot}%{_unitdir_user}/sockets.target.wants/contacts-service.socket

%post -n contacts-service2 -p /sbin/ldconfig
%post -n contacts-service2-test
chsmack -e "User" /usr/bin/contacts-service-test

%postun -n contacts-service2 -p /sbin/ldconfig

%files -n contacts-service2
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libcontacts-service2.so.*
%{_bindir}/contacts-service-ipcd*
%{_unitdir_user}/contacts-service.service
%{_unitdir_user}/sockets.target.wants/contacts-service.socket
%{_unitdir_user}/contacts-service.socket
%license LICENSE.APLv2


%files -n contacts-service2-devel
%defattr(-,root,root,-)
%{_libdir}/libcontacts-service2.so
%{_libdir}/pkgconfig/contacts-service2.pc
%{_includedir}/contacts-svc/contacts.h
%{_includedir}/contacts-svc/contacts_*.h
%license LICENSE.APLv2


%files -n contacts-service2-test
%manifest %{name}-test.manifest
%defattr(-,root,root,-)
%{_bindir}/contacts-service-test
%license LICENSE.APLv2

