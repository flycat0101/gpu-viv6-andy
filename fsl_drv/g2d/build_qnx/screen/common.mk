# Find the driver's root directory, which is 3 levels below the current make file
driver_root:=$(abspath ../../$(dir $(lastword $(MAKEFILE_LIST))))
qnx_build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="Vivante Screen Blit Module"
endef

NAME=libScreenBlit

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(VIVANTE_SDK_DIR)/hal/inc
EXTRA_INCVPATH += $(VIVANTE_SDK_DIR)/hal/os/qnx/user
EXTRA_INCVPATH += $(VIVANTE_SDK_DIR)/hal/user
EXTRA_INCVPATH += $(VIVANTE_SDK_DIR)/sdk/inc
EXTRA_INCVPATH += $(VIVANTE_SDK_DIR)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/screen
EXTRA_INCVPATH += $(driver_root)/include

SOURCE_OBJECTS += $(driver_root)/screen/vivante_g2d.o

EXTRA_SRCVPATH += $(driver_root)/screen

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LDOPTS += -lGAL -lg2d
#LIBPREF_g2d = -Bstatic
#LIBPOST_g2d = -Bdynamic

ifeq ($(USE_FAST_MEM_COPY), 1)
LIBS += fastmemcpyS
endif
LIBS += $(SCREEN_LIBS)

LIBS += m

ifeq ($(filter so, $(VARIANT_LIST)),so)
POST_BUILD=$(CP_HOST) lib$(NAME).so $(install_dir)/lib$(NAME).so
endif

ifndef QNX_INTERNAL
QNX_INTERNAL=$(qnx_build_dir)/.qnx_internal.mk
endif

include $(QNX_INTERNAL)

include $(MKFILES_ROOT)/qtargets.mk
