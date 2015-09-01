AC_PREREQ([2.64])
AC_INIT([Wayland Vivante Protocol Extension], [0.1])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 foreign dist-bzip2])

AM_SILENT_RULES([yes])

AC_PROG_CC

LT_PREREQ([2.2])
LT_INIT

PKG_PROG_PKG_CONFIG()
PKG_CHECK_MODULES(WAYLAND_SERVER, [wayland-server])
PKG_CHECK_MODULES(WAYLAND_CLIENT, [wayland-client])
AC_CHECK_HEADERS(['HAL/gc_hal.h'], [], [AC_MSG_ERROR([Cannot find Vivante headers])])

if test "x$GCC" = "xyes"; then
   GCC_CFLAGS="-Wall -g -Wstrict-prototypes -Wmissing-prototypes"
fi
AC_SUBST(GCC_CFLAGS)

WAYLAND_SCANNER_RULES(['$(top_srcdir)/protocol'])

AC_CONFIG_FILES([
   Makefile
   gc_wayland_protocol.pc
   wayland-viv.pc
   wayland-egl.pc
   ])
AC_OUTPUT
