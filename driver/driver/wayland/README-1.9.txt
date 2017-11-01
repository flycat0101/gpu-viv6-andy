Please reference [weston build](https://wayland.freedesktop.org/building.html)
for basic information.

This document mainly tell how to cross compile wayland/weston.


Build wayland-scanner
=====================

wayland-scanner tool is required to parse wayland protocol xml files.
Here we build and install tool at first before setup cross-compile environment.

`
  git clone git://anongit.freedesktop.org/wayland/wayland
  cd wayland
  git checkout 1.9.0 -b v1.9

  ./autogen.sh --disable-libraries --disable-documentation
  make -j4

  sudo make install
`

Setting up the environment
==========================

*$DESTDIR* is where the target system is installed.
The $DESTDIR environment variable will impact the target path of 'make install'
command below.

`
export DESTDIR=/path/to/target/sys-root

export WLD=$DESTDIR
export ACLOCAL_PATH=$WLD/usr/share/aclocal
export ACLOCAL="aclocal -I $ACLOCAL_PATH"

mkdir -p $WLD/usr/share/aclocal # needed by autotools
`

Following environment variables are required for cross-compile
HOST is the cross-compiler prefix, such as arm-poky-linux-gnueabi.

`
export PATH=$PATH:/path/to/cross-compile

export PKG_CONFIG_PATH=$DESTDIR/usr/lib/pkgconfig/:$DESTDIR/usr/share/pkgconfig
export PKG_CONFIG_ALLOW_SYSTEM_LIBS=1
export PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
export PKG_CONFIG_SYSROOT_DIR=$DESTDIR
export PKG_CONFIG_LIBDIR=$DESTDIR/usr/lib

export HOST=host
export CFLAGS=cflags
export CXXFLAGS=cxxflags
export LDFLAGS=ldflags
`


Build Wayland
=============

Cross comiple libraries for target platform

These commands are used when build host wayland-scanner tool.
`
  git clone git://anongit.freedesktop.org/wayland/wayland
  cd wayland
git checkout 1.9.0 -b v1.9
`

* '--with-host-scanner' option is required this time.

Again, *$DESTDIR* enironment impacts the install directory!
Same for all 'make install' commands!

`
  ./autogen.sh --host=$HOST --prefix=/usr --with-sysroot=$DESTDIR \
      --disable-documentation --with-host-scanner

  make -j4
  make install
`


Build Vivante Drivers
=====================

This topic is out of this document.
Can now build vivante drivers, depending on wayland header and libraries.

*NOTICE*: Please build in a different environment. CFLAGS, LDFLAGS may impact
the build.

And they copy required .pc files to target system.

`
  cp driver/wayland/pkgconfig/*.pc /path/to/target/sys-root/usr/lib/pkgconfig/
`


Build weston
============

Weston depends on some other libraries, build those libraries first.

libevdev
--------

* Required by libinput.

`
  git clone git://anongit.freedesktop.org/libevdev
  cd libevdev

  ./autogen.sh --host=$HOST --prefix=/usr --with-sysroot=$DESTDIR

  make -j4
  make install
`

libinput
--------

Here we use 1.8.1 verision here. If you want to use newer version, plesae follow
[libinput build]
(https://wayland.freedesktop.org/libinput/doc/latest/building_libinput.html)

* libinput requires libmtdev, skip here.
* libinput requires libwacom for tablets, skip here.

`
  git clone git://anongit.freedesktop.org/git/wayland/libinput
  cd libinput

  git checkout 1.8.1 -b v1.8.1

  ./autogen.sh --host=$HOST --prefix=/usr --with-sysroot=$DESTDIR \
      --disable-libwacom --disable-documentation --disable-tests \
      --disable-test-run --disable-debug-gui

  make -j4
make install
`

weston
------

`
  git clone git://anongit.freedesktop.org/wayland/weston
  cd weston
  git checkout 1.9.0 -b v1.9

  patch -p1 < /path/to/enable-viante-fbdev.patch

  ./autogen.sh --host=$HOST --prefix=/usr --with-sysroot=$DESTDIR \
      --enable-setuid-install \
      --enable-clients \
      --enable-simple-clients \
      --enable-simple-egl-clients \
      --enable-fbdev-compositor \
      --enable-demo-clients-install \
      --disable-simple-intel-dmabuf-client \
      --disable-xwayland \
      --disable-libunwind \
      --disable-rpi-compositor \
      --disable-rdp-compositor \
      --disable-headless-compositor \
      --disable-wayland-compositor \
      --disable-drm-compositor \
      --disable-x11-compositor \
      --disable-xwayland-test \
      WESTON_NATIVE_BACKEND=fbdev-backend.so \
      --disable-weston-launch \
      --disable-vaapi-recorder \
      --disable-dbus --disable-wcap-tools --disable-xkbcommon

  make -j4
  make install
`

Run weston
==========

Weston need XDG_RUNTIME_DIR environment to run. You may put following to
/etc/profile.d/weston.sh
See [weston build](https://wayland.freedesktop.org/building.html).

`
if test -z "$XDG_RUNTIME_DIR"; then
    export XDG_RUNTIME_DIR=/run/user/$USER
    if ! test -d "${XDG_RUNTIME_DIR}"; then
        mkdir --parents "${XDG_RUNTIME_DIR}"
        chmod 0700 "${XDG_RUNTIME_DIR}"
    fi
fi
`

Issues
======

1. Sometimes libtool will hardcoded runtime library path (-Wl,-rpath,xxx) into
   libraries or executables. That could be an issue, remove hardcoded path by:

`
  sed -i 's/hardcode_libdir_flag_spec=\(.*\)-rpath /hardcode_libdir_flag_spec=\1-rpath-link /' libtool
`

2. Some auto-tool verision may use '--with-lt-sysroot=' instead of
   '--with-sysroot' option. Please be aware of the warning after autoreconf.
