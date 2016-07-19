#!/bin/bash

########################################################
# establish build environment and build options value
# Please modify the following items according your build environment
ARCH=arm
if [ ! -z $1 ]; then
    ARCH=$1
fi


export AQROOT=`pwd`
if [ -z $VIVANTE_SDK_DIR ]; then
    if [ ! -d "$AQROOT/sdk/drivers" ] || [ ! -d "$AQROOT/sdk/include" ]; then
        echo
        echo "ERROR: not found Vivante driver SDK."
        echo "Vivante driver SDK includes driver libraries and driver header files exported."
        echo
        echo "By default, VIVANTE_SDK_DIR=$AQROOT/sdk"
        echo "Your can just copy Vivante driver SDK to $AQROOT/sdk to make sure the tool-chain can find it, "
        echo "or export VIVANTE_SDK_DIR enironment varaible which directed to Vivante dirver SDK."
        echo "for example,"
        echo "export VIVANTE_SDK_DIR=$AQROOT/../DRIVER/build/sdk"
        echo
        echo
        exit
    fi
fi

export SDK_DIR=$AQROOT/build/sdk

case "$ARCH" in

arm)
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=arm920

    export DFB_DIR=/home/software/Linux/dfb
    export CROSS_COMPILE=arm-none-linux-gnueabi-
    export TOOLCHAIN=/home/software/Linux/toolchain
    export LIB_DIR=$TOOLCHAIN/arm-none-linux-gnueabi/libc/usr/lib
;;

arm-fsl)
    ARCH=arm
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=cortex-a9
    export CPU_ARCH=armv7-a

    export KERNEL_DIR=/home/software/Linux/freescale/linux-3.0.35-c27cb385-20130116
    export CROSS_COMPILE=arm-fsl-linux-gnueabi-
    export TOOLCHAIN=/home/software/Linux/freescale/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain
#    export LIB_DIR=$TOOLCHAIN/arm-fsl-linux-gnueabi/libc/usr/lib

;;

arm-yocto)
    ARCH=arm
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=cortex-a9
    export CPU_ARCH=armv7-a

    export KERNEL_DIR=/home/software/Linux/YOCTO/L3.10.9_1.0.0_alpha_20131009
    export TOOLCHAIN=/home/software/Linux/YOCTO/poky/sysroots/x86_64-pokysdk-linux/usr
    export PATH=$TOOLCHAIN/bin:$TOOLCHAIN/bin/cortexa9hf-vfp-neon-poky-linux-gnueabi:$PATH
    export CROSS_COMPILE=arm-poky-linux-gnueabi-
    export ROOTFS=/home/software/Linux/YOCTO/x11-20130912221643
    export ROOTFS_USR=$ROOTFS/usr
    export CFLAGS="-D__ARM_PCS_VFP --sysroot=$ROOTFS"
    export LFLAGS="--sysroot=$ROOTFS"
    export PFLAGS="--sysroot=$ROOTFS"
    export FPU=vfp
    export FLOAT_ABI=hard
    BUILD_YOCTO_DRI_BUILD=1
;;

arm-his-eabi)
    ARCH=arm
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=arm920
    export CROSS_COMPILE=arm-hisi_vfpv3d16-linux-
    export TOOLCHAIN=/opt/hisi-linux/x86-arm/gcc-4.4.0-uClibc-0.9.30.2-softvfpv3
    export LIB_DIR=/opt/hisi-linux/x86-arm/gcc-4.4.0-uClibc-0.9.30.2-softvfpv3/usr/lib
;;

arm-his-oabi)
    ARCH=arm
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=arm920
    export CROSS_COMPILE=arm-hismall-linux-
    export TOOLCHAIN=/opt/hisi-linux/x86-arm/gcc-3.4.3-uClibc-0.9.28
    export LIB_DIR=/opt/hisi-linux/x86-arm/gcc-3.4.3-uClibc-0.9.28/usr/lib
;;

unicore)
    export ARCH_TYPE=unicore
    export CPU_TYPE=0
    export CPU_ARCH=0

    export CROSS_COMPILE=unicore32-linux-
    export TOOLCHAIN=/home/software/Linux/uc4-1.0-beta-hard-RHELAS5
    export LIB_DIR=$TOOLCHAIN/unicore32-linux/lib
;;

tensilica)
    export ARCH_TYPE=$ARCH
    export CPU_TYPE=0
    export CPU_ARCH=0

    CROSS_COMPILE=xtensa_venus-linux-
    TOOLCHAIN=/home/software/Linux/xtensa/staging_dir/usr
    LIB_DIR=$TOOLCHAIN/lib
;;

ppc-be)
    export ARCH_TYPE=powerpc
    export CPU_TYPE=440

    # set ENNDIANNESS to build application with little-endian
    #export ENDIANNESS=-mlittle-endian

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
    # to build driver with little endin
    #
    export ENDIANNESS=-mel

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
    # to build driver with little endin
    #
    export ENDIANNESS=-meb

    export CROSS_COMPILE=mips-linux-gnu-
    export TOOLCHAIN=/home/software/Linux/mips-4.4-5
    export LIB_DIR=$TOOLCHAIN/lib
;;

mips-le-24kc)
    export ARCH_TYPE=mips
    export CPU_ARCH=24kc
    export CPU_TYPE=0

    #
    #  set build optons: little-endian
    #
    export ENDIANNESS=-mel

    export CROSS_COMPILE=mipsel-linux-gnu-
    export TOOLCHAIN=/home/software/Linux/tools-2.6.27
    export LIB_DIR=$TOOLCHAIN/lib
;;

*)
   echo "ERROR: Unknown $ARCH, or not support so far"
   exit 1
;;

esac;

########################################################
# set special build options valule
# You can modify the build options for different results according your requirement
#
#    option                  value         description                            default value
#    -------------------------------------------------------------------------------------
#    DEBUG                   1        Enable debugging.                              0
#                            0        Disable debugging.
#
#    NO_DMA_COHERENT         1        Disable coherent DMA function.                 0
#                            0        Enable coherent DMA function.
#
#                                     Please set this to 1 if you are not sure what
#                                     it should be.
#
#    ABI                     0        Change application binary interface, default   0
#                                     is 0 which means no setting
#                                     aapcs-linux For example, build driver for Aspenite board
#
#    LINUX_OABI              1        Enable this if build environment is ARM OABI.  0
#                            0        Normally disable it for ARM EABI or other machines.
#
#    USE_VDK                 1        Eanble this one when the applications          0
#                                     are using the VDK programming interface.
#                            0        Disable this one when the applications
#                                     are NOT using the VDK programming interface.
#
#                                     Don't eanble gcdSTATIC_LINK (see below)
#                                     at the same time since VDK will load some
#                                     libraries dynamically.
#
#    EGL_API_FB              1        Use the FBDEV as the GUI system for the EGL.    0
#                            0        Use X11 system as the GUI system for the EGL.
#
#    EGL_API_DRI             1        Use DRI to support X accelerator.               0
#                                     EGL_API_FB must be 0.
#                            0        Do not use DRI to support X accelerator.
#
#    EGL_API_DFB             1        Use directFB accelerator.                       0
#                                     EGL_API_FB and EGL_API_DRI must be 0.
#                            0        Do not use DRI to support X accelerator.
#
#    gcdSTATIC_LINK          1        Enable static linking.                          0
#                            0        Disable static linking;
#
#                                     Don't enable this one when you are building
#                                     GFX driver and HAL unit tests since both of
#                                     them need dynamic linking mechanisim.
#                                     And it must NOT be enabled when USE_VDK=1.
#
#    USE_PLATFORM_DRIVER      1       Use platform driver model to build kernel      1
#                                     module on linux while kernel version is 2.6.
#                             0       Use legecy kernel driver model.
#
#    DIRECTFB_MAJOR_VERSION   1
#    DIRECTFB_MINOR_VERSION   4
#    DIRECTFB_MICRO_VERSION   0       DirectFB version supported by GFX driver.
#                                     Currentlly we support DirectFB-1.4.0.
#

BUILD_OPTION_DEBUG=0
BUILD_OPTION_ABI=0
BUILD_OPTION_LINUX_OABI=0
BUILD_OPTION_NO_DMA_COHERENT=0
BUILD_OPTION_gcdSTATIC_LINK=0
BUILD_OPTION_CUSTOM_PIXMAP=0
BUILD_OPTION_USE_VDK=1
if [ -z $BUILD_OPTION_EGL_API_FB ]; then
    BUILD_OPTION_EGL_API_FB=1
fi
if [ -z $BUILD_OPTION_EGL_API_DFB ]; then
    BUILD_OPTION_EGL_API_DFB=0
fi
if [ -z $BUILD_OPTION_EGL_API_DRI ]; then
    BUILD_OPTION_EGL_API_DRI=0
fi
if [ -z $BUILD_OPTION_USE_OPENCL ]; then
    BUILD_OPTION_USE_OPENCL=0
fi
if [ -z $BUILD_OPTION_USE_OPENVX ]; then
    BUILD_OPTION_USE_OPENVX=0
fi

if [ -z $BUILD_YOCTO_DRI_BUILD ]; then
    BUILD_YOCTO_DRI_BUILD=0
fi

BUILD_OPTION_DIRECTFB_MAJOR_VERSION=1
BUILD_OPTION_DIRECTFB_MINOR_VERSION=4
BUILD_OPTION_DIRECTFB_MICRO_VERSION=0

BUILD_OPTIONS="NO_DMA_COHERENT=$BUILD_OPTION_NO_DMA_COHERENT"
BUILD_OPTIONS="$BUILD_OPTIONS USE_VDK=$BUILD_OPTION_USE_VDK"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_FB=$BUILD_OPTION_EGL_API_FB"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_DFB=$BUILD_OPTION_EGL_API_DFB"
BUILD_OPTIONS="$BUILD_OPTIONS EGL_API_DRI=$BUILD_OPTION_EGL_API_DRI"
BUILD_OPTIONS="$BUILD_OPTIONS gcdSTATIC_LINK=$BUILD_OPTION_gcdSTATIC_LINK"
BUILD_OPTIONS="$BUILD_OPTIONS ABI=$BUILD_OPTION_ABI"
BUILD_OPTIONS="$BUILD_OPTIONS LINUX_OABI=$BUILD_OPTION_LINUX_OABI"
BUILD_OPTIONS="$BUILD_OPTIONS DEBUG=$BUILD_OPTION_DEBUG"
BUILD_OPTIONS="$BUILD_OPTIONS CUSTOM_PIXMAP=$BUILD_OPTION_CUSTOM_PIXMAP"
BUILD_OPTIONS="$BUILD_OPTIONS USE_OPENCL=$BUILD_OPTION_USE_OPENCL"
BUILD_OPTIONS="$BUILD_OPTIONS USE_OPENVX=$BUILD_OPTION_USE_OPENVX"
BUILD_OPTIONS="$BUILD_OPTIONS DIRECTFB_MAJOR_VERSION=$BUILD_OPTION_DIRECTFB_MAJOR_VERSION"
BUILD_OPTIONS="$BUILD_OPTIONS DIRECTFB_MINOR_VERSION=$BUILD_OPTION_DIRECTFB_MINOR_VERSION"
BUILD_OPTIONS="$BUILD_OPTIONS DIRECTFB_MICRO_VERSION=$BUILD_OPTION_DIRECTFB_MICRO_VERSION"
BUILD_OPTIONS="$BUILD_OPTIONS YOCTO_DRI_BUILD=$BUILD_YOCTO_DRI_BUILD"

export PATH=$TOOLCHAIN/bin:$PATH

########################################################
# clean/build driver and samples
# build results will save to $SDK_DIR/
#
cd $AQROOT; make -j1 -f makefile.linux $BUILD_OPTIONS clean
cd $AQROOT; make -j1 -f makefile.linux $BUILD_OPTIONS install 2>&1 | tee $AQROOT/linux_build.log

########################################################
# other build/clean commands to build/clean specified items, eg.
#
# cd $AQROOT; make -f makefile.linux $BUILD_OPTIONS hal_test V_TARGET=clean || exit 1
# cd $AQROOT; make -f makefile.linux $BUILD_OPTIONS hal_test V_TARGET=install || exit 1

