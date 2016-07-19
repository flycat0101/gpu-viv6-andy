ifneq ($(PREBUILT_FSL_IMX_GPU),true)

GPU_LOCAL_PATH := $(call my-dir)
include $(GPU_LOCAL_PATH)/driver/Android.mk
include $(GPU_LOCAL_PATH)/fsl_drv/g2d/Android.mk
include $(GPU_LOCAL_PATH)/fsl_drv/helper/Android.mk
include $(GPU_LOCAL_PATH)/fsl_drv/tool/Android.mk
endif
