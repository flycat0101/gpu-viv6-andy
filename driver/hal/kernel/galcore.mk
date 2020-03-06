#the device/fsl/common/build/kernel.mk should be included before this file
ifeq ($(GPU_VIV6_PATH),)
$(error GPU_VIV6_PATH not defined but galcore.mk included)
endif

KERNEL_DIR := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ
TARGET_ARCH := $(TARGET_KERNEL_ARCH)
GALCORE_CROSS_COMPILE := $(strip $(KERNEL_CROSS_COMPILE_WRAPPER))
TARGET_OUT_SHARED_LIBRARIES := $(PRODUCT_OUT)/vendor/lib

GALCORE_SRC_PATH := $(GPU_VIV6_PATH)/gpu-viv6/driver
GALCORE_ARCH_PATH := $(GPU_VIV6_PATH)/gpu-viv6/driver/arch/XAQ2
GALCORE_VGARCH_PATH := $(GPU_VIV6_PATH)/gpu-viv6/driver/arch/GC350

#AQROOT := $(GALCORE_SRC_PATH)
VIVANTE_ANDROID_MK_DEF :=
include $(GALCORE_SRC_PATH)/Android.mk.def

#
# galcore.ko
#

GALCORE_OUT := $(TARGET_OUT_INTERMEDIATES)/GALCORE_OBJ
GALCORE := \
	$(GALCORE_OUT)/galcore.ko

KERNEL_CFLAGS ?= KCFLAGS=-mno-android

KERNELENVSH := $(GALCORE_OUT)/kernelenv.sh
$(KERNELENVSH):
	mkdir -p $(GALCORE_OUT)
	echo 'export KERNEL_DIR=$(KERNEL_DIR)' > $(KERNELENVSH)
	echo 'export CROSS_COMPILE=$(GALCORE_CROSS_COMPILE)' >> $(KERNELENVSH)
	echo 'export ARCH_TYPE=$(ARCH_TYPE)' >> $(KERNELENVSH)

galcore: $(KERNELENVSH) $(GALCORE_SRC_PATH)
	$(hide) if [ ${clean_build} = 1 ]; then \
		PATH=$$PATH $(MAKE) -C $(GALCORE_SRC_PATH) clean \
		AQROOT=$(abspath $(GALCORE_SRC_PATH)); \
		AQARCH=$(abspath $(GALCORE_ARCH_PATH)); \
		AQVGARCH=$(abspath $(GALCORE_VGARCH_PATH)); \
	fi
	@ . $(KERNELENVSH); $(kernel_build_shell_env) \
	$(MAKE) -f Kbuild -C $(GALCORE_SRC_PATH) \
		$(CLANG_TO_COMPILE) \
		$(KERNEL_CFLAGS) \
		AQROOT=$(abspath $(GALCORE_SRC_PATH)) \
		AQARCH=$(abspath $(GALCORE_ARCH_PATH)) \
		AQVGARCH=$(abspath $(GALCORE_VGARCH_PATH)) \
		ARCH_TYPE=$(ARCH_TYPE) \
		DEBUG=$(DEBUG) \
		VIVANTE_ENABLE_DRM=$(DRM_GRALLOC) \
		VIVANTE_ENABLE_2D=$(VIVANTE_ENABLE_2D) \
		VIVANTE_ENABLE_3D=$(VIVANTE_ENABLE_3D) \
		VIVANTE_ENABLE_VG=$(VIVANTE_ENABLE_VG); \
	cp $(GALCORE_SRC_PATH)/galcore.ko $(GALCORE);

