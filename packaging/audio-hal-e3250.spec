Name:       audio-hal-e3250
Summary:    TIZEN Audio HAL for Exynos3250
Version:    0.2.16
Release:    0
Group:      System/Libraries
License:    Apache-2.0
URL:        http://tizen.org
Source0:    audio-hal-e3250-%{version}.tar.gz
BuildRequires: pkgconfig(alsa)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(iniparser)
BuildRequires: pkgconfig(dlog)
Provides: libtizen-audio.so

%description
TIZEN Audio HAL for Exynos3250

%prep
%setup -q -n %{name}-%{version}

%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS ?DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"

%autogen
%configure

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/etc/smack/accesses.d
cp -arf audio_hal_sys_file_access.efl %{buildroot}/etc/smack/accesses.d
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}

%make_install

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%manifest audio-hal-e3250.manifest
%defattr(-,root,root,-)
/etc/smack/accesses.d/audio_hal_sys_file_access.efl
/usr/lib/libtizen-audio.so
%{_datadir}/license/%{name}
