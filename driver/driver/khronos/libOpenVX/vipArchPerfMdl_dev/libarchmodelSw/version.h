/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
' version 0_165
' - fix typo in MergeSubgraph()
' version (vip_arch_performance 187)
' - update DataRequesterInCacheLineMode condition (fixing bug#22846)
' version 0_147
' - fix a bug when doing Alignment with Nx1-shape layers. inimage_stride should be set equal to aligned_inimage_slice
*/