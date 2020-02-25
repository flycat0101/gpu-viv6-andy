/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
' version 0_213
' - fix cache_line_mode conditions (bug#2046)
' - fix bug#1928 conditions
' version 0_212
' - fixed internal write cycle calculation
' version 0_207
' - added TP_VIPSRAM_OT1_BUG2050 and VIPSRAM_ASYNC_FIFO
' - fixed InternalKernelReadCycleCount in bottleneck comparison.
' version 0_194
' - adjusted v7 pipe line latency bubble issue modeling
' version 0_193
' - v7 pipe line latency bubble issue
' version 0_188
' - model Bug#2045 NEIGHBOR_IMG_TRAN_NOT_EFFICIENT_BUG2045
' version 0_186
' - fixed Cycles_Multiply() AXISRAMTotal
' version 0_182
' - Tensor Add Merge
* - version 0_181
* - fixed Cycles_Create()
* ...
* version 0_156
* - separate 1x1 conv bandwidth modeling
* version 0_154
* - Updated Tile3DImageSingleReadBW() regarding 1x1 conv layers
*       Port CL#207746

* version 0_153
* - Add Internal KernelReadBW Limit (Bug#1998)

*  version 0_152
* - initialize NumOfKernel for v7 configs
* version 0_151
* - fixed bug1992 modeling
* version 0_150
* - Removed INT16 accum_buf_depth adjustment from NumOfKernel() as the adjustment has been done by the calling program
* version 0_149
* - Included 2x2 pooling V8 limitation in NumOfKernel() calculation
* version 0_148
* - Modeled KERNEL_PER_CORE_LT_ONE_THIRD_COEF_BUF_DEPTH_BUG2000
* version 0_147
* - Updated the ImageRepeatedRead Calculation for ReadBandwidth()
* version 0_146
* - Corrected V8 1x1 NumOfKernel() HW limitation for INT16
* version 0_145
* - added V8 1x1 NumOfKernel() HW limitation
* version 0_144
* - updated V8 NumOfKernel() HW limitation
* version 0_143
* - add Manual Kernels Per Core
* version 0_142
* - updated ImageRepeatedRead calculation
* ----- port CL#207737
*
* version 0_141
*  - add NN_SLOW_OUTPUT_HW_FEATURE
*    - port CL#207717
*
* version 0_139
*  - model bug#1992
*    - port CL#207715
*
**/
