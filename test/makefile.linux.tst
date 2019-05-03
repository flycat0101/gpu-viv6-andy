##############################################################################
#
#    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
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


################################################################################
# Macros.

AQROOT              ?= $(PWD)
AQARCH              ?= $(AQROOT)/arch/XAQ2
AQVGARCH            ?= $(AQROOT)/arch/GC350

export AQROOT AQARCH AQVGARCH

################################################################################
# Include common definitions.

include $(AQROOT)/makefile.linux.def

################################################################################
# Components of the project.

# app
HAL_TEST        := $(HAL_TEST_DIR)
HAL_UNIT_TEST   := $(AQROOT)/test/hal/common/UnitTest
GFX_TEST        := $(GFX_TEST_DIR)
VVLAUNCHER      := $(VVLAUNCHER_DIR)
OVX_TEST         := $(AQROOT)/test/ovx
ifneq ($(VIVANTE_NO_VG),1)
TIGER           := $(AQROOT)/test/vg11/tiger
endif

ifeq ($(USE_VDK),0)
VDK_TEST        :=
else
VDK_TEST        += $(VDK_TEST_DIR)
endif
CL11_TEST       := $(CL11_TEST_DIR)
CL11_UNIT       := $(CL11_UNIT_DIR)


ifeq ($(STATIC_LINK),1)
APP_HAL_LIST    := $(HAL_TEST)
ifneq ($(VIVANTE_NO_VG),1)
APP_OVG11_LIST  := $(TIGER)
endif
APP_GFX_LIST    :=
else
APP_HAL_LIST    := $(HAL_TEST) $(HAL_UNIT_TEST)
APP_GFX_LIST    := $(GFX_TEST)
ifneq ($(VIVANTE_NO_VG),1)
APP_OVG11_LIST  := $(TIGER)
endif
endif

ifeq ($(USE_OPENCL),1)
APP_OCL11_LIST  := $(CL11_TEST)
else
APP_OCL11_LIST  :=
endif
APP_VDK_LIST    += $(VDK_TEST) $(TIGER) $(VVLAUNCHER)
APP_OVX_LIST    := $(OVX_TEST)

APP_LIST        :=

ifeq ($(VIVANTE_ENABLE_2D),1)
APP_LIST        += $(APP_HAL_LIST)
ifneq ($(STATIC_LINK),1)
APP_LIST        += $(APP_GFX_LIST)
endif
endif

ifeq ($(VIVANTE_ENABLE_3D),1)
ifeq ($(USE_VDK),1)
APP_LIST        += $(APP_VDK_LIST)
else
ifneq ($(VIVANTE_NO_VG),1)
APP_LIST        += $(APP_OVG11_LIST)
endif
endif
ifeq ($(USE_OPENCL),1)
APP_LIST        += $(APP_OCL11_LIST)
endif
ifeq ($(USE_OPENVX),1)
APP_LIST        += $(APP_OVX_LIST)
endif
ifeq ($(USE_OPENVX),1)
APP_LIST        += $(APP_OVX_LIST)
endif
endif

ifeq ($(VIVANTE_ENABLE_VG),1)
ifneq ($(VIVANTE_NO_VG), 1)
APP_LIST        += $(APP_OVG11_LIST)
endif
endif

################################################################################
# Define the macros used in the common makefile.

SUBDIRS     :=  $(APP_LIST)
MAIN_MODULE :=  $(APP_LIST)

OBJ_DIR     := $(SDK_DIR)

################################################################################
# Targets

.PHONY: all clean install $(SUBDIRS)

all:     $(MAIN_MODULE)

clean:   V_TARGET := clean
install: V_TARGET := install

clean:   all
	@rm -rf $(SDK_DIR)

install: all
	@mkdir -p $(SDK_DIR)
	@-cp -f $(AQROOT)/release/SW/ReadMe_Linux_SDK.txt $(SDK_DIR)/ReadMe.txt

$(SUBDIRS):
	@test ! -d $@ || $(MAKE) -f makefile.linux -C $@ $(V_TARGET) gcdSTATIC_LINK=$(gcdSTATIC_LINK)

.PHONY: $(GFX_TEST)
$(GFX_TEST):
ifeq (,$(DFB_DIR))
	@echo "DFB_DIR variable was not exported; skipped gfx test building"
endif

################################################################################
# Supported targets.


ifeq ($(VIVANTE_ENABLE_2D),1)
gfx_test:   $(GFX_TEST)
hal_test:   $(HAL_TEST)
hal_unit:   $(HAL_UNIT_TEST)
hal_test_all:   $(HAL_TEST) $(HAL_UNIT_TEST)
chipinfo:   $(CHIPINFO)
endif

ifeq ($(USE_VDK), 0)
tutorial:   $(TUTORIAL)
else
vdktest:    $(VDK_TEST)
vv_launcher: $(VVLAUNCHER)
endif

ifeq ($(USE_OPENCL), 1)
cl11_test:  $(CL11_TEST)
cl11_unit:  $(CL11_UNIT)
endif

ifeq ($(USE_OPENVX), 1)
ovx_test:  $(OVX_TEST)
endif

ifneq ($(VIVANTE_NO_VG), 1)
ovg11_tst:  $(APP_OVG11_LIST)

tiger:      $(TIGER)
endif
