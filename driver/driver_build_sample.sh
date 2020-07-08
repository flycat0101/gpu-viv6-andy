#!/bin/bash

set -e

########################################################
# establish build environment and build options value
# Please modify the following items according your build environment

if [ -z $ARCH ]; then
ARCH=arm-yocto
fi

export AQROOT=`pwd`
export AQARCH=$AQROOT/arch/XAQ2

export SDK_DIR=$AQROOT/build/sdk

case "$ARCH" in

arm)
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=arm920
    #export FIXED_ARCH_TYPE=arm
    export KERNEL_DIR=/home/software/Linux/linux-2.6.21-arm1
    export CROSS_COMPILE=arm-none-linux-gnueabi-
    export TOOLCHAIN=/home/software/Linux/toolchain
    export LIB_DIR=$TOOLCHAIN/arm-none-linux-gnueabi/libc/usr/lib
;;

arm-yocto)
    ARCH=arm
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=cortex-a9
    export CPU_ARCH=armv7-a
    BUILD_YOCTO_DRI_BUILD=1
    BUILD_OPTION_USE_OPENCL=1
    BUILD_OPTION_VIVANTE_ENABLE_2D=1
    BUILD_OPTION_MXC_FBDEV=1
;;

arm64-yocto)
    export ARCH=arm64
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=cortex-a53
    export CPU_ARCH=armv8-a
    export USE_KMS=1
    BUILD_YOCTO_DRI_BUILD=1
    BUILD_OPTION_USE_OPENCL=1
    BUILD_OPTION_USE_VULKAN=1
    BUILD_OPTION_USE_OPENVX=1
    BUILD_OPTION_MXC_FBDEV=1
    BUILD_OPTION_VIVANTE_ENABLE_2D=0
    BUILD_OPTION_VIVANTE_ENABLE_VG=0
;;
IMX8_Alpha)
    ARCH=arm64
    export BOARD=IMX8_Alpha
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=cortex-a53
    export CPU_ARCH=armv8-a

    export KERNEL_DIR=/home/software/Linux/freescale/L4.9.11_imx8_alpha_kernel_code/linux-imx
    export TOOLCHAIN=/home/software/Linux/freescale/fsl-imx-internal-xwayland/4.9.11-8mq_alpha/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux
    export PATH=$TOOLCHAIN:$PATH
    export CROSS_COMPILE=aarch64-poky-linux-
    export ROOTFS=/home/software/Linux/freescale/fsl-imx-internal-xwayland/4.9.11-8mq_alpha/sysroots/aarch64-poky-linux
    export ROOTFS_USR=$ROOTFS/usr
    export CFLAGS="--sysroot=$ROOTFS  -D__arm64"
    export PFLAGS="--sysroot=$ROOTFS"
    export LDFLAGS="--sysroot=$ROOTFS"
    export X11_ARM_DIR=$ROOTFS/usr
    export HAVE_G2D=1
    BUILD_OPTION_EGL_API_DRI=0
    BUILD_OPTION_EGL_API_FB=0
    BUILD_OPTION_X11_DRI3=1
    BUILD_OPTION_VIVANTE_ENABLE_DRM=1
;;

X86_PCIE)
    #export FIXED_ARCH_TYPE=x86_64
    export KERNEL_DIR=/home/software/Linux/x86_pcie/linux-headers-4.8.0-41-generic
    export ROOTFS=/
    export TOOLCHAIN=/usr
    export CROSS_COMPILE=""
    export CPU_TYPE=0
    export CPU_ARCH=0
    export ARCH_TYPE=x86_64
    export USE_LINUX_PCIE=1
;;

ppc-be)
    export ARCH_TYPE=powerpc
    export CPU_TYPE=440

    # '-be' mens big-endian
    #export FIXED_ARCH_TYPE=ppc-be

    # set ENDIANNESS to build driver with little-endian
    #export ENDIANNESS=-mlittle-endian

    export KERNEL_DIR=/home/software/Linux/linux-2.6.27
    export CROSS_COMPILE=ppc_4xx-
    export TOOLCHAIN=/home/software/eldk/usr
    export DEPMOD=$TOOLCHAIN/bin/depmod.pl
    export LIB_DIR=/home/software/eldk/ppc_4xx/lib

;;

mips-le)
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=0
    export ARCH_TYPE=mips
    export CPU_ARCH=34kf

    #
    # to select the right ES20 pre-built files
    #
    #export FIXED_ARCH_TYPE=mips-le

    #
    # to build driver with little endin
    #
    export ENDIANNESS=-mel

    export KERNEL_DIR=/home/software/Linux/linux-2.6.19-mips.le
    export CROSS_COMPILE=mips-linux-gnu-
    export TOOLCHAIN=/home/software/Linux/mips-4.4-5
    export LIB_DIR=$TOOLCHAIN/mips-linux-gnu/libc/el/usr/lib
;;

mips-be)
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=0
    export ARCH_TYPE=mips
    export CPU_ARCH=34kf

    #
    # to select the right ES20 pre-built files
    #
    #export FIXED_ARCH_TYPE=mips-be

    #
    # to build driver with little endin
    #
    export ENDIANNESS=-meb

    export KERNEL_DIR=/home/software/Linux/linux-2.6.19-mips.be
    export CROSS_COMPILE=mips-linux-gnu-
    export TOOLCHAIN=/home/software/Linux/mips-4.4-5
    export LIB_DIR=$TOOLCHAIN/lib
;;

*)
   echo "ERROR: Unknown $ARCH, or not support so far"
   exit 1
;;

esac;


########################################################
# Driver Build Options
#
BUILD_OPTION_DEBUG=0
BUILD_OPTION_ABI=0
BUILD_OPTION_LINUX_OABI=0
BUILD_OPTION_NO_DMA_COHERENT=0
BUILD_OPTION_USE_VDK=1
BUILD_OPTION_gcdSTATIC_LINK=0
BUILD_OPTION_CUSTOM_PIXMAP=0
BUILD_OPTION_USE_3D_VG=1
BUILD_OPTION_USE_FB_DOUBLE_BUFFER=0
BUILD_OPTION_USE_PLATFORM_DRIVER=1
if [ -z $BUILD_OPTION_MAXCPUS ]; then
    BUILD_OPTION_MAXCPUS=4
fi

if [ -z $BUILD_OPTION_VIVANTE_ENABLE_VG ]; then
    BUILD_OPTION_VIVANTE_ENABLE_VG=1
fi

if [ -z $USE_KMS ]; then
    USE_KMS=0
fi
BUILD_OPTION_FPGA_BUILD=0
if [ -z $BUILD_OPTION_MXC_FBDEV ]; then
    BUILD_OPTION_MXC_FBDEV=0
fi
if [ -z $BUILD_OPTION_EGL_API_FB ]; then
    BUILD_OPTION_EGL_API_FB=1
fi
if [ -z $BUILD_OPTION_EGL_API_DFB ]; then
    BUILD_OPTION_EGL_API_DFB=0
fi
if [ -z $BUILD_OPTION_EGL_API_DRI ]; then
    BUILD_OPTION_EGL_API_DRI=0
fi
if [ -z $BUILD_OPTION_X11_DRI3 ]; then
    BUILD_OPTION_X11_DRI3=0
fi
if [ -z $BUILD_OPTION_EGL_API_WL ]; then
    BUILD_OPTION_EGL_API_WL=0
fi
if [ -z $BUILD_OPTION_EGL_API_X ]; then
    BUILD_OPTION_EGL_API_X=0
fi
if [ -z $BUILD_OPTION_EGL_API_GBM ]; then
    BUILD_OPTION_EGL_API_GBM=0
fi
if [ -z $BUILD_OPTION_EGL_API_NULLWS ]; then
    BUILD_OPTION_EGL_API_NULLWS=0
fi
if [ -z $BUILD_OPTION_EGL_API_GBM ]; then
    BUILD_OPTION_EGL_API_GBM=1
fi
if [ "$BOARD" = "IMX8_Alpha" ];then
    BUILD_OPTION_EGL_API_GBM=0
fi

if [ -z $BUILD_OPTION_VIVANTE_ENABLE_DRM ];then
    BUILD_OPTION_VIVANTE_ENABLE_DRM=0
fi

if [ -z $BUILD_OPTION_X11_DRI3 ];then
    BUILD_OPTION_X11_DRI3=0
fi
if [ -z $BUILD_OPTION_USE_OPENCL ]; then
    BUILD_OPTION_USE_OPENCL=0
fi
if [ -z $BUILD_OPTION_USE_OPENVX ]; then
    BUILD_OPTION_USE_OPENVX=0
fi
if [ -z $BUILD_OPTION_USE_VULKAN ]; then
    BUILD_OPTION_USE_VULKAN=0
fi
if [ -z $BUILD_OPTION_ENABLE_GPU_CLOCK_BY_DRIVER ]; then
    BUILD_OPTION_ENABLE_GPU_CLOCK_BY_DRIVER=0
fi
if [ -z $BUILD_GL4_DRI_BUILD ]; then
    BUILD_GL4_DRI_BUILD=0
fi
if [ -z $BUILD_OPTION_VIVANTE_NO_GL4 ]; then
    BUILD_OPTION_VIVANTE_NO_GL4=0
fi
if [ -z $BUILD_OPTION_VIVANTE_NO_VG ]; then
    BUILD_OPTION_VIVANTE_NO_VG=0
fi
if [ -z $BUILD_OPTION_VIVANTE_ENABLE_DRM ]; then
    BUILD_OPTION_VIVANTE_ENABLE_DRM=0
fi
if [ -z $BUILD_OPTION_VIVANTE_ENABLE_3D ]; then
    BUILD_OPTION_VIVANTE_ENABLE_3D=1
fi
if [ -z $BUILD_OPTION_VIVANTE_ENABLE_2D ]; then
    BUILD_OPTION_VIVANTE_ENABLE_2D=0
fi

BUILD_OPTIONS="NO_DMA_COHERENT=$BUILD_OPTION_NO_DMA_COHERENT"
BUILD_OPTIONS="$BUILD_OPTIONS USE_VDK=$BUILD_OPTION_USE_VDK"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_WL=$BUILD_OPTION_EGL_API_WL"
BUILD_OPTIONS="$BUILD_OPTIONS MXC_FBDEV=$BUILD_OPTION_MXC_FBDEV"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_FB=$BUILD_OPTION_EGL_API_FB"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_DFB=$BUILD_OPTION_EGL_API_DFB"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_DRI=$BUILD_OPTION_EGL_API_DRI"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_X=$BUILD_OPTION_EGL_API_X"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_GBM=$BUILD_OPTION_EGL_API_GBM"
BUILD_OPTIONS="$BUILD_OPTIONS X11_DRI3=$BUILD_OPTION_X11_DRI3"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_GBM=$BUILD_OPTION_EGL_API_GBM"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_NULLWS=$BUILD_OPTION_EGL_API_NULLWS"
BUILD_OPTIONS="$BUILD_OPTIONS gcdSTATIC_LINK=$BUILD_OPTION_gcdSTATIC_LINK"
BUILD_OPTIONS="$BUILD_OPTIONS ABI=$BUILD_OPTION_ABI"
BUILD_OPTIONS="$BUILD_OPTIONS LINUX_OABI=$BUILD_OPTION_LINUX_OABI"
BUILD_OPTIONS="$BUILD_OPTIONS DEBUG=$BUILD_OPTION_DEBUG"
BUILD_OPTIONS="$BUILD_OPTIONS CUSTOM_PIXMAP=$BUILD_OPTION_CUSTOM_PIXMAP"
BUILD_OPTIONS="$BUILD_OPTIONS USE_3D_VG=$BUILD_OPTION_USE_3D_VG"
BUILD_OPTIONS="$BUILD_OPTIONS USE_OPENCL=$BUILD_OPTION_USE_OPENCL"
BUILD_OPTIONS="$BUILD_OPTIONS USE_OPENVX=$BUILD_OPTION_USE_OPENVX"
BUILD_OPTIONS="$BUILD_OPTIONS USE_VULKAN=$BUILD_OPTION_USE_VULKAN"
BUILD_OPTIONS="$BUILD_OPTIONS USE_FB_DOUBLE_BUFFER=$BUILD_OPTION_USE_FB_DOUBLE_BUFFER"
BUILD_OPTIONS="$BUILD_OPTIONS USE_PLATFORM_DRIVER=$BUILD_OPTION_USE_PLATFORM_DRIVER"
BUILD_OPTIONS="$BUILD_OPTIONS ENABLE_GPU_CLOCK_BY_DRIVER=$BUILD_OPTION_ENABLE_GPU_CLOCK_BY_DRIVER"
BUILD_OPTIONS="$BUILD_OPTIONS FPGA_BUILD=$BUILD_OPTION_FPGA_BUILD"
BUILD_OPTIONS="$BUILD_OPTIONS GL4_DRI_BUILD=$BUILD_GL4_DRI_BUILD"
BUILD_OPTIONS="$BUILD_OPTIONS VIVANTE_NO_GL4=$BUILD_OPTION_VIVANTE_NO_GL4"
BUILD_OPTIONS="$BUILD_OPTIONS VIVANTE_NO_VG=$BUILD_OPTION_VIVANTE_NO_VG"
BUILD_OPTIONS="$BUILD_OPTIONS VIVANTE_ENABLE_DRM=$BUILD_OPTION_VIVANTE_ENABLE_DRM"
BUILD_OPTIONS="$BUILD_OPTIONS VIVANTE_ENABLE_VG=$BUILD_OPTION_VIVANTE_ENABLE_VG"
BUILD_OPTIONS="$BUILD_OPTIONS VIVANTE_ENABLE_3D=$BUILD_OPTION_VIVANTE_ENABLE_3D"
BUILD_OPTIONS="$BUILD_OPTIONS VIVANTE_ENABLE_2D=$BUILD_OPTION_VIVANTE_ENABLE_2D"
BUILD_OPTIONS="$BUILD_OPTIONS X11_DRI3=$BUILD_OPTION_X11_DRI3"
BUILD_OPTIONS="$BUILD_OPTIONS VIVANTE_ENABLE_DRM=$BUILD_OPTION_VIVANTE_ENABLE_DRM"
BUILD_OPTIONS="$BUILD_OPTIONS -j$BUILD_OPTION_MAXCPUS"
BUILD_OPTIONS="$BUILD_OPTIONS USE_KMS=$USE_KMS"

#export PATH=$TOOLCHAIN/bin:$PATH
COMMITNR=`git log -n 1 --format=%h`
if [ "x$COMMITNR" != "x" ]; then
  DIRTY=`git diff --quiet HEAD || echo '-dirty'`
  export CFLAGS="$CFLAGS -DGIT_STRING='$COMMITNR$DIRTY'"
fi

########################################################
# clean/build driver and samples
# build results will save to $SDK_DIR/
#
(
set -o pipefail
if [ "clean" == "$1" ]; then
cd $AQROOT; make -f makefile.linux $BUILD_OPTIONS clean
else
cd $AQROOT; make -f makefile.linux $BUILD_OPTIONS install 2>&1 | tee $AQROOT/linux_build.log
fi
)
########################################################
# other build/clean commands to build/clean specified items, eg.
#
# cd $AQROOT; make -f makefile.linux $BUILD_OPTIONS gal_core V_TARGET=clean || exit 1
# cd $AQROOT; make -f makefile.linux $BUILD_OPTIONS gal_core V_TARGET=install || exit 1

