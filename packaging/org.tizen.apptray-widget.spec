%define PREFIX /usr/apps/org.tizen.apptray-widget
Name:	org.tizen.apptray-widget
Version:	0.1.1
Release:	1
Summary: Tizen W apptray widget application
Source: %{name}-%{version}.tar.gz
License: Flora-1.1
Group: Applications/System

BuildRequires: cmake
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(badge)
BuildRequires: pkgconfig(embryo)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-widget-application)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(ecore-evas)
BuildRequires: pkgconfig(ecore-file)
BuildRequires: pkgconfig(ecore-imf)
BuildRequires: pkgconfig(ecore-input)
BuildRequires: pkgconfig(eet)
BuildRequires: pkgconfig(efl-assist)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(widget_viewer)
BuildRequires: pkgconfig(widget_service)
BuildRequires: pkgconfig(widget_viewer_evas)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(feedback)
BuildRequires: efl-assist
BuildRequires: efl-assist-devel
BuildRequires: gettext-tools
BuildRequires: edje-bin, embryo-bin

%ifarch %{arm}
%define ARCH arm
%else
%define ARCH emulator
%endif

%description
W Apptray-Widget application

%prep
%setup -q

%build

%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS+=" -DTIZEN_ENGINEER_MODE"
export CXXFLAGS+=" -DTIZEN_ENGINEER_MODE"
export FFLAGS+=" -DTIZEN_ENGINEER_MODE"
%endif

CFLAGS+=" -fPIC -fpie -O2 "
LDFLAGS+="-Wl,--rpath=%{PREFIX}/lib -Wl,--as-needed -Wl,--hash-style=both"; export LDFLAGS
cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX} -DARCH=%{ARCH}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

%make_install
mkdir -p %{buildroot}/opt/usr/apps/org.tizen.apptray-widget
mkdir -p %{buildroot}%{_libdir}/systemd/system/tizen-system.target.wants

%post
/sbin/ldconfig
/usr/bin/signing-client/hash-signer-client.sh -a -d -p platform /usr/apps/org.tizen.apptray-widget

%files
%manifest apptray-widget-app/%{name}.manifest
%defattr(-,root,root,-)
%attr(-,inhouse,inhouse)
#%dir %{PREFIX}/res
%{PREFIX}/bin/*
%{PREFIX}/res/*
%{PREFIX}/res/edje/*
%{PREFIX}/shared/res/*
/usr/share/packages/*
#/usr/share/packages/%{name}.xml
/usr/share/icons/org.tizen.apptray-widget-app.png
/usr/share/icons/org.tizen.apptray-widget-app-small.png
/usr/share/icons/org.tizen.apptray-widget.png
/usr/share/icons/org.tizen.apptray-widget-small.png
/usr/share/icons/apps_widget_preview.png

