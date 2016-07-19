ifeq ($(filter vivante,$(BOARD_GPU_DRIVERS)),)
include $(call my-dir)/source/Android.mk
endif
