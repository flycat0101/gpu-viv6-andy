##############################################################################
#
#    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
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


##########################################################
# Set top variables

AQROOT=$(_WINCEROOT)\public\GCHAL
AQARCH=$(AQROOT)\arch\XAQ2
AQVGARCH=$(AQROOT)\arch\GC350

##########################################################
# Set global variables for OS

_COMMONPUBROOT=$(_PUBLICROOT)\common
__PROJROOT=$(AQROOT)


##########################################################
# Set OEM and ISV include path

_OEMINCPATH=$(_WINCEROOT)\public\common\sdk\inc
_OEMINCPATH=$(_OEMINCPATH);$(_WINCEROOT)\public\common\oak\inc
_OEMINCPATH=$(_OEMINCPATH);$(_WINCEROOT)\public\common\ddk\inc

_ISVINCPATH=$(_WINCEROOT)\public\common\sdk\inc


##########################################################
# Set project global variables

WINCEOEM=1
RELEASETYPE=PLATFORM
WARNISERROR=1

!IF "$(VIVANTE_SDK_DIR)" == ""
VIVANTE_SDK_DIR=$(_FLATRELEASEDIR)\vivante_sdk
!ENDIF

VIVANTE_SDK_INC=$(VIVANTE_SDK_DIR)\inc
VIVANTE_SDK_LIB=$(VIVANTE_SDK_DIR)\lib
VIVANTE_SDK_BIN=$(VIVANTE_SDK_DIR)\bin

!IF "$(INCLUDES)" != ""
INCLUDES=$(INCLUDES);$(VIVANTE_SDK_INC);$(VIVANTE_SDK_INC)\HAL;
!ELSE
INCLUDES=$(VIVANTE_SDK_INC);$(VIVANTE_SDK_INC)\HAL;
!ENDIF

!IF "$(_WINCEOSVER)" >= "700"
GCLIB_PATH=$(_COMMONPUBROOT)\platform\$(_TGTPLAT)\lib\$(_CPUINDPATH)
!ELSE
GCLIB_PATH=$(_TARGETPLATROOT)\lib\$(_CPUINDPATH)
!ENDIF


##########################################################
# LOCAL COMPILING COMMAND

GCHAL_C_COMPILING_COMMAND= \
	@echo BUILD_MARKER:C_COMPILE_START Compiling $< \
	&& @$(CCOMPILER) $(CONLY_FLAGS) -Fo$@ $(C_COMMAND_LINE_OPTIONS) $< \
	&& @echo BUILD_MARKER:C_COMPILE_END

GCHAL_CPP_COMPILING_COMMAND= \
	@echo BUILD_MARKER:CPP_COMPILE_START Compiling $< \
	&& @$(CXXCOMPILER) $(CONLY_FLAGS) -Fo$@ $(C_COMMAND_LINE_OPTIONS) $< \
	&& @echo BUILD_MARKER:CPP_COMPILE_END


##########################################################
# Include board specific setting of macros if existing.

!IF EXIST ($(BUILDROOT)\board_specific_$(_TGTPLAT).inc)
!  INCLUDE $(BUILDROOT)\board_specific_$(_TGTPLAT).inc
!ENDIF

##########################################################
# symbol name for library path.

LIBSYS=$(_COMMONSDKROOT)\lib\$(_CPUINDPATH)\coredll.lib
LIBGALUSER=$(GCLIB_PATH)\libGAL.lib
LIBEGL=$(GCLIB_PATH)\libEGL.lib
LIBVDK=$(GCLIB_PATH)\libVDK.lib
LIBGLES_CL=$(GCLIB_PATH)\libGLES_CL.lib
LIBGLES_CM=$(GCLIB_PATH)\libGLES_CM.lib
LIBGLESV1_CL=$(GCLIB_PATH)\libGLESv1_CL.lib
LIBGLESV1_CM=$(GCLIB_PATH)\libGLESv1_CL.lib
LIBGLESV2=$(GCLIB_PATH)\libGLESv2.lib
LIBGLESV3=$(GCLIB_PATH)\libGLESv2.lib
LIBOVG=$(GCLIB_PATH)\libOpenVG.lib
LIBGLSLC=$(GCLIB_PATH)\libGLSLC.lib

CDEFINES=$(CDEFINES) -DLIBEGL=libEGL.dll

##########################################################
# Default setting of macros

!IFNDEF VIVANTE_ENABLE_3D
VIVANTE_ENABLE_3D=1
!ENDIF

!IFNDEF VIVANTE_ENABLE_2D
VIVANTE_ENABLE_2D=1
!ENDIF

!IFNDEF VIVANTE_ENABLE_VG
VIVANTE_ENABLE_VG=0
!ENDIF

CDEFINES=$(CDEFINES) -DgcdENABLE_VG=$(VIVANTE_ENABLE_VG)
CDEFINES=$(CDEFINES) -DgcdENABLE_2D=$(VIVANTE_ENABLE_2D)
CDEFINES=$(CDEFINES) -DgcdENABLE_3D=$(VIVANTE_ENABLE_3D)

!IFNDEF VDK
USE_VDK=1
!ENDIF

!IFNDEF USE_BANK_ALIGNMENT
USE_BANK_ALIGNMENT=0
!ENDIF

!IFNDEF BANK_BIT_START
BANK_BIT_START=0
!ENDIF

!IFNDEF BANK_BIT_END
BANK_BIT_END=0
!ENDIF

!IFNDEF BANK_CHANNEL_BIT
BANK_CHANNEL_BIT=0
!ENDIF

!IFNDEF USE_LOADTIME_OPT
USE_LOADTIME_OPT=1
!ENDIF

!IF "$(VIVANTE_ENABLE_3D)" == "1"

BUILD_HALTI=1
!ENDIF

##########################################################
# The global definitions of C/C++ macros

!IF "$(_TGTPLAT)" == "Emulator" || "$(_TGTPLAT)" == "DeviceEmulator"
CDEFINES=$(CDEFINES) -DEMULATOR
!ENDIF

CDEFINES=$(CDEFINES) -DUSE_VDK=$(USE_VDK)

!IF "$(USE_BANK_ALIGNMENT)" == "1"
CDEFINES=$(CDEFINES) -DgcdENABLE_BANK_ALIGNMENT=1

!   IF "$(BANK_BIT_START)" != "0"
!       IF "$(BANK_BIT_END)" != "0"
CDEFINES=$(CDEFINES) -DgcdBANK_BIT_START=$(BANK_BIT_START)
CDEFINES=$(CDEFINES) -DgcdBANK_BIT_END=$(BANK_BIT_END)
!       ENDIF
!   ENDIF

!   IF "$(BANK_CHANNEL_BIT)" != "0"
CDEFINES=$(CDEFINES) -DgcdBANK_CHANNEL_BIT=$(BANK_CHANNEL_BIT)
!   ENDIF
!ENDIF

!IF "$(USE_LOADTIME_OPT)" != "1"
CDEFINES=$(CDEFINES) -DGC_ENABLE_LOADTIME_OPT=$(USE_LOADTIME_OPT)
!ENDIF

CDEFINES=$(CDEFINES) -DgcdPOWER_SUSPEND_WHEN_IDLE=0

LDEFSTACK=/STACK:131072


##########################################################
# Validate dependent path

!IF !EXIST ($(AQROOT))
!   ERROR $(AQROOT) dose not exist!
!ENDIF

!IF !EXIST ($(AQARCH))
!ENDIF

!IF "$(VIVANTE_ENABLE_VG)" == "1"
!   IF !EXIST ($(AQVGARCH))
!   ENDIF
!ENDIF

