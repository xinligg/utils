Summary: Remote Desktop Protocol functionality
Name: freerdp
Version: 2.0
Release: 1%{?dist}
License: Apache License 2.0
Group: Applications/Communications
URL: http://freerdp.sourceforge.net/
Source: http://downloads.sourceforge.net/%{name}/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  openssl-devel, libX11-devel, libXcursor-devel, cups-devel
BuildRequires:	alsa-lib-devel, pulseaudio-libs-devel, gstreamer1-devel, gstreamer1-plugins-base-devel, ffmpeg-devel, libjpeg-turbo-devel, systemd-devel, libXrandr-devel

%description
freerdp implements Remote Desktop Protocol (RDP), used in a number of Microsoft
products.

%package -n xfreerdp
Summary: Remote Desktop Protocol client
Requires: libXrandr
Requires: pulseaudio-libs, libX11, gstreamer1, ffmpeg-libs, libjpeg-turbo, systemd-libs
%description -n xfreerdp
xfreerdp is a client for Remote Desktop Protocol (RDP), used in a number of
Microsoft products.

%package devel
Summary: Libraries and header files for embedding and extending freerdp
Requires: %{name} = %{version}-%{release}
Requires: pkgconfig
%description devel
Header files and unversioned libraries for libfreerdp, libfreerdpchanman and
libfreerdpkbd.

%prep
%setup -q -n freerdp2-2.0.0~git20170725.1.1648deb+dfsg1

%build
rm -f CMakeCache.txt
cmake -DSTATIC_CHANNELS=OFF -DWITH_PULSE=ON -DWITH_ALSA=ON -DCHANNEL_URBDRC=ON -DCHANNEL_URBDRC_CLIENT=ON -DWITH_CUPS=OFF -DWITH_PCSC=OFF -DWITH_SMARTCARD_INSPECT=OFF -DWITH_MBEDTLS=ON -DWITH_X11=ON -DWITH_XRANDR=ON -DWITH_FFMPEG=ON -DWITH_GSTREAMER_1_0=ON -DWITH_JPEG=ON -DWITH_X264=OFF -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release

make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm -f $RPM_BUILD_ROOT%{_libdir}/{freerdp/,lib}*.{a,la} # FIXME: They shouldn't be installed in the first place
rm -rf $RPM_BUILD_ROOT%{_libdir}/cmake

%clean
rm -rf $RPM_BUILD_ROOT

%files -n xfreerdp
%defattr(-,root,root)
%{_bindir}/winpr-makecert
%{_bindir}/winpr-hash
%{_bindir}/xfreerdp
%{_libdir}/lib*.so.*
%{_libdir}/freerdp2/
%{_mandir}/*/*

%files devel
%defattr(-,root,root)
%{_includedir}/freerdp2/
%{_includedir}/winpr2/
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/*

%changelog

* Tue Mar 12 2019 Xin Li <xinli@gxos.com.cn> - 2.0
- Initial freerdp spec - made and tested for GXOS7
