#!/bin/bash
  export PATH=/opt/fsl-imx-xwayland-4.19-thud/4.19-thud/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/:$PATH
#  export ROOTFS_USR=/nfsroot/nfs_L3.0.35_1.1.0_121218/usr
  export ROOTFS=/opt/fsl-imx-xwayland-4.19-thud/4.19-thud/sysroots/aarch64-poky-linux
  export ROOTFS_USR=$ROOTFS/usr
  export CROSS_COMPILE=aarch64-poky-linux-
  export CFLAGS=--sysroot=$ROOTFS
  export LFLAGS=--sysroot=$ROOTFS
  export ARCH=arm64-yocto
  export SDK_BUILD=1
  make clean
  make
