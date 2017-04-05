##############################################################################
#
#    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


# Copyright (c) 2012-2013 The Khronos Group Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and/or associated documentation files (the
# "Materials"), to deal in the Materials without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Materials, and to
# permit persons to whom the Materials are furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Materials.
#
# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(OPENVX_DEFS) -DgcdUSE_VX=1
AQROOT := $(OPENVX_TOP)/../../../..

LOCAL_SRC_FILES :=  viv_c_absdiff.c \
					viv_c_accumulate.c \
					viv_c_addsub.c \
					viv_c_bitwise.c \
					viv_c_channel.c \
					viv_c_conv3x3.c \
					viv_c_convertcolor.c \
					viv_c_convertdepth.c \
					viv_c_convolve.c \
					viv_c_fast9.c \
					viv_c_filter.c \
					viv_c_histogram.c \
					viv_c_integralimage.c \
					viv_c_lut.c \
					viv_c_magnitude.c \
					viv_c_norm.c \
					viv_c_nonmax.c \
					viv_c_sobel3x3.c \
					viv_c_statistics.c	\
					viv_c_remap.c

LOCAL_SRC_FILES +=	\
					viv_c_morphology.c	\
					viv_c_multiply.c	\
					viv_c_optpyrlk.c	\
					viv_c_phase.c	\
					viv_c_scale.c	\
					viv_c_threshold.c	\
					viv_c_warp.c	\
					viv_c_model.c	\

LOCAL_C_INCLUDES := $(OPENVX_INC) $(OPENVX_TOP)/$(OPENVX_SRC)/include $(OPENVX_TOP)/debug $(OPENVX_TOP)/../inc
LOCAL_C_INCLUDES += $(AQROOT)/hal/inc $(AQROOT)/hal/user $(AQROOT)/hal/os/linux/user $(AQROOT)/compiler/libVSC/include
LOCAL_MODULE := libopenvx-viv_c_model-lib
include $(BUILD_STATIC_LIBRARY)
