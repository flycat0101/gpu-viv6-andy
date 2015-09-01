##############################################################################
#
#    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
#    All Rights Reserved.
#
#    Permission is hereby granted, free of charge, to any person obtaining
#    a copy of this software and associated documentation files (the
#    'Software'), to deal in the Software without restriction, including
#    without limitation the rights to use, copy, modify, merge, publish,
#    distribute, sub license, and/or sell copies of the Software, and to
#    permit persons to whom the Software is furnished to do so, subject
#    to the following conditions:
#
#    The above copyright notice and this permission notice (including the
#    next paragraph) shall be included in all copies or substantial
#    portions of the Software.
#
#    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
#    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
#    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
#    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
##############################################################################


#
# Common inlude file for Linux build.
#

################################################################
# Select prebuilt binary types.
FIXED_ARCH_TYPE     ?=

################################################################
# Arch.

ARCH_TYPE           ?= arm
CPU_TYPE            ?= arm920
CPU_ARCH            ?=
ABI                 ?=
ENDIANNESS          ?=
FPU                 ?=
FLOAT_ABI           ?=

USE_ARMCC           ?= 0
LINUX_OABI          ?= 0

gcdSTATIC_LINK      ?= 0

VIVANTE_ENABLE_2D   ?= 1
VIVANTE_ENABLE_VG   ?= 0
VIVANTE_ENABLE_3D   ?= 1
VIVANTE_NO_VG       ?= 0

LINUX_EMULATOR      ?= 0
NO_KERNEL           ?= 0

USE_OPENCL          ?= 1
USE_OPENVX          ?= 0
USE_VDK             ?= 0

EGL_API_FB          ?= 0
EGL_API_WL          ?= 0
EGL_API_DRI         ?= 0
EGL_API_DFB         ?= 0
EGL_API_X           ?= 0
USE_SW_FB           ?= 0

USE_MOVG            ?= 0
GC355_MEM_PRINT     ?= 0
GC355_PROFILER      ?= 0

USE_BANK_ALIGNMENT  ?= 0
BANK_BIT_START      ?= 0
BANK_BIT_END        ?= 0
BANK_CHANNEL_BIT    ?= 0

USE_LOADTIME_OPT    ?= 1
YOCTO_DRI_BUILD     ?= 0
VSC_VIR_BUILD       ?= 1


################################################################
# Option dependency.

ifeq ($(VIVANTE_ENABLE_3D),1)
  ifeq ($(USE_OPENCL),1)
    BUILD_OPENCL_ICD:= 1
    BUILD_OPENCL_FP := 0
  else
    BUILD_OPENCL_ICD:= 0
  endif
else
  BUILD_OPENCL_ICD  := 1
  BUILD_OPENCL_FP   := 0
endif

ifeq ($(VIVANTE_NO_VG),1)
  VIVANTE_ENABLE_VG := 0
endif

ifeq ($(gcdSTATIC_LINK),1)
  GL_11_APPENDIX    ?= _es11
  GL_2_APPENDIX     ?= _es2
  GL_3_APPENDIX     ?= _es3
endif

ifneq ($(LINUX_OABI),1)
  ABI               ?= aapcs-linux
endif

################################################################
# Initialize build flags.

ifeq ($(MAKELEVEL),0)
  ORIG_CFLAGS       := $(CFLAGS)
  ORIG_CXXFLAGS     := $(CXXFLAGS)
  ORIG_LFLAGS       := $(LFLAGS)
  ORIG_PFLAGS       := $(PFLAGS)

  export ORIG_CFLAGS ORIG_CXXFLAGS ORIG_LFLAGS ORIG_PFLAGS
endif

CFLAGS              := $(ORIG_CFLAGS)
CXXFLAGS            := $(ORIG_CXXFLAGS)
LFLAGS              := $(ORIG_LFLAGS)
PFLAGS              := $(ORIG_PFLAGS)

################################################################
# Toolchain.

ifeq ($(USE_ARMCC),1)
  ARM_BASE          ?= /home/software/ARM
  ARM_VERSION       ?= 4.0/650
  CROSS_COMPILE     ?= $(ARM_BASE)/RVCT/Programs/$(ARM_VERSION)/linux-pentium/arm
  CFLAGS            += -I$(ARM_BASE)/RVCT/Data/$(ARM_VERSION)/include/unix
  CC                := $(CROSS_COMPILE)cc
  CXX               := $(CROSS_COMPILE)cc
  AR                := $(CROSS_COMPILE)ar
  AS                := $(CROSS_COMPILE)as
  LD                := $(CROSS_COMPILE)link
else
  CROSS_COMPILE     ?= arm-none-linux-gnueabi-
  CC                := $(CROSS_COMPILE)gcc
  CXX               := $(CROSS_COMPILE)g++
  AR                := $(CROSS_COMPILE)ar
  AS                := $(CROSS_COMPILE)as
  LD                := $(CROSS_COMPILE)ld
  RANLIB            := $(CROSS_COMPILE)ranlib
  STRIP             := $(CROSS_COMPILE)strip
endif

################################################################
# Resource.

KERNEL_DIR          ?=
TOOL_DIR            ?= /home/software/Linux
X11_ARM_DIR         ?= $(TOOL_DIR)/X11_ARM
ROOTFS_USR          ?= $(TOOL_DIR)/RootFS/usr
WAYLAND_DIR         ?= $(TOOL_DIR)/wayland

VIVANTE_SDK_DIR     ?= $(AQROOT)/build/sdk
VIVANTE_SDK_INC     ?= $(VIVANTE_SDK_DIR)/include
VIVANTE_SDK_LIB     ?= $(VIVANTE_SDK_DIR)/drivers

################################################################
# Target directory.

ifeq ($(DEBUG),1)
  OBJ_DIR           ?= bin_d
else
  OBJ_DIR           ?= bin_r
endif

################################################################
# switch to build halti driver instead of es2 driver

BUILD_HALTI         ?= 1

################################################################
# Release directory.

SDK_DIR             ?= $(AQROOT)/build/sdk

################################################################
# Force to use dma_coherent_* stuff.

NO_DMA_COHERENT     ?= 0

################################################################
# Set this value to 1 if you are using ARM L2 cache.

ENABLE_ARM_L2_CACHE := 0

################################################################
# Set this value to 1 if you are using DOVE board.
CONFIG_DOVE_GPU     := 0

###############################################################
# Common flags.

ifeq ($(USE_ARMCC),1)
  CFLAGS            += --c99 #--strict
else
  ifneq ($(ABI),)
    ifneq ($(ABI),0)
      CFLAGS        += -mabi=$(ABI)
    endif
  endif

  ifneq ($(ENDIANNESS),)
    CFLAGS          += $(ENDIANNESS)
    LFLAGS          += $(ENDIANNESS)
    PFLAGS          += $(ENDIANNESS)
  endif

  CFLAGS            += -DLINUX
  CFLAGS            += -Wall -D_REENTRANT -fno-strict-aliasing

  ifneq ($(CPU_TYPE),)
    ifneq ($(CPU_TYPE),0)
      CFLAGS        += -mtune=$(CPU_TYPE)
      LFLAGS        += -mtune=$(CPU_TYPE)
      PFLAGS        += -mtune=$(CPU_TYPE)
    endif
  endif

  ifneq ($(CPU_ARCH),)
    ifneq ($(CPU_ARCH),0)
      CFLAGS        += -march=$(CPU_ARCH)
      LFLAGS        += -march=$(CPU_ARCH)
      PFLAGS        += -march=$(CPU_ARCH)
    endif
  endif
endif

ifneq ($(FPU),)
  CFLAGS            += -mfpu=$(FPU)
  CXXFLAGS          += -mfloat-abi=$(FLOAT_ABI)
endif

ifneq ($(FLOAT_ABI),)
  CFLAGS            += -mfloat-abi=$(FLOAT_ABI)
  CXXFLAGS          += -mfloat-abi=$(FLOAT_ABI)
  LFLAGS            += -mfloat-abi=$(FLOAT_ABI)
  PFLAGS            += -mfloat-abi=$(FLOAT_ABI)
endif

ifeq ($(LINUX_OABI), 1)
  CFLAGS            += -DLINUX_OABI
endif

ifeq ($(DEBUG), 1)
# CFLAGS            += -g3 -ggdb3 -O0 -DDEBUG -D_DEBUG -DgcdDEBUG=1
  CFLAGS            += -g -O0 -DDEBUG -D_DEBUG -DgcdDEBUG=1
else
  ifeq ("$(DEBUG)", "valgrind")
    CFLAGS          += -g -O -DgcdBUILT_FOR_VALGRIND=1
  else
    CFLAGS          += -O2
  endif
endif

CFLAGS              += -DgcdENABLE_3D=$(VIVANTE_ENABLE_3D)
CFLAGS              += -DgcdENABLE_2D=$(VIVANTE_ENABLE_2D)
CFLAGS              += -DgcdENABLE_VG=$(VIVANTE_ENABLE_VG)

ifeq ($(VIVANTE_NO_VG),1)
  CFLAGS            += -DVIVANTE_NO_VG
endif

ifeq ($(VIVANTE_ENABLE_3D),1)
  CFLAGS            += -DgcdUSE_OPENCL=$(USE_OPENCL)
else
  CFLAGS            += -DgcdUSE_OPENCL=0
endif

ifeq ($(VIVANTE_ENABLE_3D),1)
  CFLAGS            += -DgcdUSE_VX=$(USE_OPENVX)
else
  CFLAGS            += -DgcdUSE_VX=0
endif

ifneq ($(VIVANTE_ENABLE_3D)_$(VIVANTE_ENABLE_VG),0_0)
  CFLAGS            += -DUSE_VDK=$(USE_VDK) -DUSE_SW_FB=$(USE_SW_FB)
else
  CFLAGS            += -DUSE_VDK=0
endif

CFLAGS              += -DgcdMOVG=$(USE_MOVG)

ifeq ($(USE_BANK_ALIGNMENT),1)
  CFLAGS            += -DgcdENABLE_BANK_ALIGNMENT=1

  ifneq ($(BANK_BIT_START),0)
    ifneq ($(BANK_BIT_END),0)
      CFLAGS        += -DgcdBANK_BIT_START=$(BANK_BIT_START)
      CFLAGS        += -DgcdBANK_BIT_END=$(BANK_BIT_END)
    endif
  endif

  ifneq ($(BANK_CHANNEL_BIT),0)
    CFLAGS          += -DgcdBANK_CHANNEL_BIT=$(BANK_CHANNEL_BIT)
  endif
endif

ifeq ($(EGL_API_FB), 1)
  CFLAGS            += -DEGL_API_FB
endif

ifeq ($(EGL_API_WL), 1)
  CFLAGS            += -DEGL_API_WL -Wno-deprecated-declarations
  LFLAGS            += -L$(ROOTFS_USR)/lib
endif

ifeq ($(EGL_API_DRI),1)
  ifeq ($(YOCTO_DRI_BUILD),1)
    CFLAGS          += -DDRI_PIXMAPRENDER_ASYNC
    CFLAGS          += -DDRI_PIXMAPRENDER_GL
  endif

  CFLAGS            += -DEGL_API_DRI
  CFLAGS            += -I$(X11_ARM_DIR)/include/arm-linux-gnueabi
endif

ifeq ($(EGL_API_DFB), 1)
  CFLAGS            += -DEGL_API_DFB
endif

ifeq ($(EGL_API_X), 1)
  CFLAGS            += -DEGL_API_X
  CFLAGS            += -I$(X11_ARM_DIR)/include/arm-linux-gnueabi
endif

ifeq ($(STATIC_LINK), 1)
  CFLAGS            += -DSTATIC_LINK
endif


ifeq ($(USE_NEW_LINUX_SIGNAL),1)
  CFLAGS            += -DUSE_NEW_LINUX_SIGNAL=1
endif

ifneq ($(USE_ARMCC),1)
  CXXFLAGS          += -fno-short-enums
endif

ifneq (,$(EGL_APPENDIX))
  CFLAGS            += -D_EGL_APPENDIX=$(EGL_APPENDIX)
endif

ifeq ($(gcdSTATIC_LINK), 1)
  CFLAGS            += -DgcdSTATIC_LINK=1
else
  CFLAGS            += -DgcdSTATIC_LINK=0
endif

ifneq (,$(GL_2_APPENDIX))
  CFLAGS            += -D_GL_2_APPENDIX=$(GL_2_APPENDIX)
endif

ifneq ($(GL_11_APPENDIX),)
  CFLAGS            += -D_GL_11_APPENDIX=$(GL_11_APPENDIX)
endif

ifneq ($(GL_3_APPENDIX),)
  CFLAGS            += -D_GL_3_APPENDIX=$(GL_3_APPENDIX)
endif

ifneq ($(VG_APPENDIX),)
  CFLAGS            += -D_VG_APPENDIX=$(VG_APPENDIX)
endif

CFLAGS              += -DgcdREGISTER_ACCESS_FROM_USER=1

ifeq ($(gcdFPGA_BUILD),1)
  CFLAGS            += -DgcdFPGA_BUILD=1
else
  CFLAGS            += -DgcdFPGA_BUILD=0
endif

ifeq ($(FORCE_ALL_VIDEO_MEMORY_CACHED),1)
  CFLAGS            += -DgcdPAGED_MEMORY_CACHEABLE=1
else
  CFLAGS            += -DgcdPAGED_MEMORY_CACHEABLE=0
endif

ifeq ($(USE_LOADTIME_OPT),1)
  CFLAGS            += -DGC_ENABLE_LOADTIME_OPT=1
else
  CFLAGS            += -DGC_ENABLE_LOADTIME_OPT=0
endif



CFLAGS              += -DgcdGC355_MEM_PRINT=$(GC355_MEM_PRINT)
CFLAGS              += -DgcdGC355_PROFILER=$(GC355_PROFILER)

################################################################################
# Build with profiler

CFLAGS              += -DVIVANTE_PROFILER=1
CFLAGS              += -DVIVANTE_PROFILER_CONTEXT=1


################################################################################
#
# Third party resources, DFB_DIR to specify DirectFB SDK path
# To build DirectFB Accelerator and gfx test, must set DFB_DIR
#
#DFB_DIR            ?= $(TOOL_DIR)/dfb

################################################################################
# Module directories.

# drivers
GAL_DIR             := $(AQROOT)/hal
EGL_DIR             := $(AQROOT)/driver/khronos/libEGL
GLES11_DIR          := $(AQROOT)/driver/khronos/libGLESv11

  GLES2X_DIR        := $(AQROOT)/driver/khronos/libGLESv3

GLSLC_DIR           := $(AQROOT)/compiler/libGLSLC
VSC_DIR             := $(AQROOT)/compiler/libVSC

VG113D_DIR          := $(AQROOT)/driver/khronos/libOpenVG_3D/vg11/driver
VG112D_DIR          := $(AQROOT)/driver/khronos/libOpenVG
GFX_DIR             := $(AQROOT)/driver/dfb
EXA_DIR             := $(AQROOT)/driver/X/EXA/src

GL21_DIR            := $(AQROOT)/driver/khronos/libGL2

CL11_DIR            := $(AQROOT)/driver/khronos/libCL
CL11_ICD_DIR        := $(AQROOT)/driver/khronos/libCL/icdloader12
CLC_DIR             := $(AQROOT)/compiler/libCLC

OPENVX_DIR          := $(AQROOT)/driver/khronos/libOpenVX

VDK_DIR             := $(AQROOT)/sdk/vdk
VIVANTE_LIB_DIR     := $(AQROOT)/sdk/vivante

# applications
EGL_TEST_DIR        := $(AQROOT)/test/egl
ES11_TEST_DIR       := $(AQROOT)/test/es11/Linux
ES2X_TEST_DIR       := $(AQROOT)/test/es20
ES2X_EXTERN_DIR     := $(AQROOT)/test/es20/extern
ES20_TEST_DIR       := $(AQROOT)/sdk/samples/es20
CL11_TEST_DIR       := $(AQROOT)/test/cl11

VDK_TEST_DIR        := $(AQROOT)/sdk/samples/vdk
HAL_TEST_DIR        := $(AQROOT)/sdk
HAL_CUSTOM_DIR      := $(AQROOT)/test/hal/Linux/custom
CHIPINFO_DIR        := $(AQROOT)/test/hal/common/chipinfo

GLBES11_DIR         := $(AQROOT)/test/es11/GLBenchmark_ES1.1v2
GLBES20_DIR         := $(AQROOT)/test/es20/GLBenchmark2_RC2
GLBES30_DIR         := $(AQROOT)/../../../TEST/SW/Benchmarks/Kishonti/GLBenchmark3.0/AA21A9
GLBESNAVI11_DIR     := $(AQROOT)/test/es11/GLBenchmark_Navi_Beta2/es11
GLBESNAVI20_DIR     := $(AQROOT)/test/es11/GLBenchmark_Navi_Beta2/es20
GLBESNAVI_DIR       := $(AQROOT)/test/es20/GLBenchNavi
#GLBESNAVI_DIR      := $(AQROOT)/test/es20/GLBenchmark_navi
GTF_DIR             := $(AQROOT)/test/es20/conform/GTF_ES/glsl/GTF
CTS_ES30_DIR        := $(AQROOT)/../../../TEST/SW/Khronos/Conformance/es30/conform/GTF_ES/glsl/GTF
VGMARK_DIR          := $(AQROOT)/test/vg/common/VGMark_10_src
VGMARK11_DIR        := $(AQROOT)/test/vg11/VGMark11/VGMark11_addendum
VGCTS_DIR           := $(AQROOT)/test/vg/ovg_1.0.1_cts_rc10
VGCTS11_DIR         := $(AQROOT)/test/vg/ovg_1.1_cts_rc10

ifeq ($(VIVANTE_ENABLE_3D),1)
  ifeq ($(USE_VDK),1)
    TUTORIAL_DIR    := $(ES11_TEST_DIR)
    MM06_DIR        := $(AQROOT)/test/es11/common/3DMarkMobile06/configuration/vdk
    CONFORM_DIR     := $(AQROOT)/test/es11/conform
  else
    MM06_DIR        := $(ES11_TEST_DIR)/3DMarkMobile06
  endif
endif

MM07_DIR            := $(ES2X_TEST_DIR)/3DMarkMobileES2_RC_src
MM07U_DIR           := $(ES2X_TEST_DIR)/3DMarkMobileES2_Update
SM20_DIR            := $(AQROOT)/test/es20/SimulationMark_ES_2.0
KANZI_DIR           := $(AQROOT)/test/es20/Kanzi_UI_demo/Kanzi_UI_src
AB20_DIR            := $(ES2X_TEST_DIR)/Automotive
JRC_DIR             := $(ES2X_TEST_DIR)/JRC
GFX_TEST_DIR        := $(AQROOT)/test/gfx
ES20_GEARS_DIR      := $(AQROOT)/test/es20/gles2-cm-gears
ES11_UNIT_DIR       := $(AQROOT)/test/es11/common/UnitTest
ES11_EXTERN_DIR     := $(AQROOT)/test/es11/extern
ES11_EXTENSION_DIR  := $(AQROOT)/test/es11/Extensions
ES11_GEARS_DIR      := $(AQROOT)/test/es11/gles1-cm-gears
VG11_UNIT_DIR       := $(AQROOT)/test/vg11/UnitTest
VG11_EXTERN_DIR     := $(AQROOT)/test/vg11/extern
CL11_UNIT_DIR       := $(AQROOT)/test/cl11/UnitTest
CL11_EXTERN_DIR     := $(AQROOT)/test/cl11/extern
CL11_CONFORM_DIR    := $(AQROOT)/test/cl11/conform
CL11_SAMPLE_DIR     := $(AQROOT)/sdk/samples/cl11


TEXTURE5_DIR        := $(ES2X_TEST_DIR)/texture5
TEXTURE7_DIR        := $(ES2X_TEST_DIR)/texture7

ACTINIUM_DIR        := $(ES2X_TEST_DIR)/actinium
VVLAUNCHER_DIR      := $(ES2X_TEST_DIR)/vv_launcher

VIDEOCUBE_DIR       := $(AQROOT)/test/es11/extern/VideoCube
VC_TESTCASE_DIR     := $(AQROOT)/test/es20/vCompiler_Testcase


################################################################################
# Platform specific configs.

THIRDPARTY_CUSTOMER ?=
THIRDPARTY_TYPE     ?=
