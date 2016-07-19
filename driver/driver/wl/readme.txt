WAYLAND SETUP AND BUILD INSTRUCTIONS

0. CONTENTS

	1. INTRODUCTION
	2. FEATURES
	3. LIMITATIONS
	4. PREREQUISITIES
	5. BUILDING
	6. RUNNING

1. INTRODUCTION

These instructions can be used to set up and build Wayland/Weston compositor
with Vivante 3D acceleration support. General information about Wayland can
be obtained at http://wayland.freedesktop.org/.

2. FEATURES

Currently, Vivante driver provides 3D acceleration for the Weston compositor.
Both CPU based client applications as well as 3D client applications are
supported.

3. LIMITATIONS

Note that Wayland/Weston provides support for a number of features, not just
for rendering graphics. These include touchscreen handling, tty events, pointer
device handling, to name a few. As Vivante's efforts are mainly concentrated
on enabling graphics, some of these features are disabled or commented out for
the development to proceed. It is assumed that this (non-graphics)
functionality should be properly supported by the BSP provider.

This also means that some of the Wayland sample applications that require
support for these subsystems will not work, fully or at all, with the
instructions provided herein.

On the flip side, Wayland lacks basic framebuffer backend support that modern
graphics stacks for embedded devices typically use. Thus Vivante has added this
support to the Wayland stack.

Only Wayland 1.9.0 / Weston 1.9.0 version has been validated to work.

4. PREREQUISITIES

The following is the software environment required:

- Wayland1.9.0/Weston1.9.0: Wayland dependencies are listed in
  http://wayland.freedesktop.org/building.html.
- Vivante patch for Weston.

5. BUILDING

Note that Wayland depends on Vivante stack for rendering and Vivante stack
itself depends on Wayland for window system interaction. Therefore there is
an inherent circular dependency. To work around this, the build is performed
in phases enabling enough functionality in the current phase to be able to
build the next phase.

The build procedure involves the following steps in the given order :

- Building Wayland server and client libraries.
- Building Vivante stack with Wayland support.
- Building Weston and sample applications.

In order to prevent potential build complications, please rename the
existing Vivante headers/libraries/driver on the target so the build system
will not pick up the incorrect libraries.

5.1. BUILDING WAYLAND SERVER AND CLIENT LIBRARIES

This build is performed on the target device.

Initialize a terminal window with the following settings.

export WLD=<Set this to wayland install directory>
export LD_LIBRARY_PATH=$WLD/lib
export PKG_CONFIG_PATH=$WLD/lib/pkgconfig/:$WLD/share/pkgconfig/
export ACLOCAL="aclocal -I $WLD/share/aclocal"

Also, create the 'share/aclocal' directory.

mkdir -p $WLD/share/aclocal

To build, download the wayland library v1.9.0 sources from
http://wayland.freedesktop.org/releases/wayland-1.9.0.tar.xz and issue the
following commands:

    $ cd wayland
    $ ./autogen.sh --prefix=$WLD
    $ make
    $ make install

This will create and install libwayland-server.so and libwayland-client.so.

5.2. BUILDING VIVANTE STACK WITH WAYLAND SUPPORT

This build is performed on the host PC.

Please build vivante driver with the next:
(No wayland option)
export ROOTFS=<Set this to rootfs directory>
export ROOTFS_USR=$ROOTFS/usr
export ROOTFS_DIR=$ROOTFS/usr
export EGL_API_FB=1
export EGL_API_DRI=0
export USE_VDK=0
export VIVANTE_NO_VG=1

After finishing the building, do the next:
The step above is only to generate libgal for building the next.
Please go to <project_directory>/driver/wl
please do the next at the same TERM as the one for building the driver above, otherwise there will lack environment variables
make -f gcmakefile.linux
make -f gcmakefile.linux install


Please build a directory named wayland-viv under $WLD/include
and copy <project_directory>/driver/wl/gc_wayland_protocol.h and wayland-viv-client-protocol.h and wayland-viv-server-protocol.h   $WLD/include/wayland-viv
and copy <project_directory>/build/sdk/drivers/libgc_wayland_protocol**   $WLD/lib


Then rebuild vivante driver with the next:
(The user should use this built driver to run weston and Apps)

export ROOTFS=<Set this to rootfs directo>
export ROOTFS_USR=$ROOTFS/usr
export ROOTFS_DIR=$ROOTFS/usr
export EGL_API_FB=1
export EGL_API_DRI=0
export USE_VDK=0
export EGL_API_WL=1
export WL_EGL_PLATFORM=1
export WAYLAND_DIR=$WLD
export VIVANTE_NO_VG=1

With these settings run the build script to create driver binaries for Wayland-based driver.

After the building is done, please copy <project_directory>/build/sdk/drivers/libwayland-viv**  $WLD/lib

5.3. BUILDING WESTON AND SAMPLE APPLICATIONS

This build is performed on the target device.

Copy the sdk directory from the previous step to the target as was previously
done. Also, copy the .../sdk/drivers to /usr/lib on the target.
Please mount ROOTFS/WLD from host to target.


Before proceeding with Weston, we will need Cairo, libxkbcommon, and Pixman as
Weston and/or its clients depend on them. Please build these as explained in
http://wayland.freedesktop.org/building.html. But note that, for now, we do not
enable gl backend for Cairo, so the '--enable-gl --enable-xcb' flags must not be
used when building Cairo. Also note that, when building libxkbcommon, the
for-weston-1.9.0 branch must not be used as mentioned in the link above. Please
use the master branch.
Now add the following environment settings in the terminal window.
export WLD=<Set it according to your mount>
export ROOTFS=<Set it according to your mount>
export PKG_CONFIG_PATH=$WLD/lib/pkgconfig/:$WLD/share/pkgconfig/
export ACLOCAL="aclocal -I $WLD/share/aclocal"
export LD_LIBRARY_PATH=$WLD/lib:$LD_LIBRARY_PATH
export COMPOSITOR_LIBS="-L${WLD}/lib -lGLESv2 -lEGL -lGAL -lwayland-server -lxkbcommon -lpixman-1"
export COMPOSITOR_CFLAGS="-I ${ROOTFS}/usr/include/pixman-1 -DLINUX=1 -DEGL_API_FB -DEGL_API_WL"
export FB_COMPOSITOR_CFLAGS="-DLINUX=1 -DEGL_API_FB -DEGL_API_WL -I $WLD/include"
export FB_COMPOSITOR_LIBS="-L${WLD}/lib -lGLESv2 -lEGL -lwayland-server -lxkbcommon"
export SIMPLE_EGL_CLIENT_CFLAGS="-DLINUX -DEGL_API_FB -DEGL_API_WL"


Download Weston 1.9.0 version from http://wayland.freedesktop.org/releases/weston-1.9.0.tar.xz
untar and apply the patch provided by Vivante to weston 1.9.

Weston and client applications can now be built using the following commands.
    $ cd weston
    ./autogen.sh --prefix=/usr --enable-setuid-install --disable-tablet-shell --disable-xwayland --enable-simple-clients --enable-clients --enable-demo-clients --disable-libunwind --disable-rpi-compositor --disable-rdp-compositor --enable-simple-egl-clients  --disable-libunwind --disable-xwayland-test WESTON_NATIVE_BACKEND=fbdev-backend.so
--disable-weston-launch --disable-headless-compositor --disable-wayland-compositor --disable-drm-compositor --disable-x11-compositor --enable-fbdev-compositor --disable-vaapi-recorder

    $ make
    $ make install

This will create and install the weston executable and some sample applications.

6. RUNNING

Make sure the kernel Vivante driver is insmod'ed properly. Also, Weston must be run
as root. In addition to the environment settings mentioned previously, export the
following environment variable in the terminal :

# Assuming we are in 'weston' directory.
export XDG_RUNTIME_DIR=`pwd`

Now to run the compositor, execute 'src/weston --tty=2 --use-gl=1 &'. You should see a blue screen fading
in. You should see a simple desktop. You can then enter
'clients/simple-egl &' to see a 3D client in action. There are other client
applications that can be run.

Also, set 'export FB_MULTI_BUFFER=2' to use the flip presentation method for
the compositor. This variable can also be set to '3' assuming FB driver has
enough memory.
