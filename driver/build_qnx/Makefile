##############################################################################
#
#    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


LIST=COMPONENTS
ENABLE_VG ?= 0

EARLY_DIRS=halkernel halarchkernel halarchkernel_vg galcore halosuser halarchuser halarchuser_vg gal vsc egl vdk gles11 gles20sc gles30 llvm clc cl vg11 vg11_vg vx vxc
include recurse.mk

galcore: halkernel halarchkernel halarchkernel_vg
gal: halosuser halarchuser halarchuser_vg
vsc egl: gal
vdk: egl egl
gles11 gles20sc gles30 vg11: egl gal vsc
vg11_vg: egl gal
gles20sc: gal vsc
llvm: gal vsc
clc: gal llvm vsc
cl: egl gal gles30 vsc
vx: clc gal llvm vsc
vxu: clc gal llvm vsc vx