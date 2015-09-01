/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __chip_draw_h_
#define __chip_draw_h_
#include "gc_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

extern GLvoid __glChipFlush(__GLcontext * gc);
extern GLvoid __glChipFinish(__GLcontext * gc);
extern GLvoid __glChipDrawNothing(__GLcontext* gc);
extern GLvoid __glChipDrawIndexedPrimitive(__GLcontext* gc);
extern GLvoid __glChipDrawPrimitive(__GLcontext* gc);
extern GLvoid __glChipDrawQuadListPrimitive(__GLcontext* gc);
extern GLvoid __glChipDrawQuadStripPrimitive(__GLcontext* gc);
extern GLvoid __glChipDrawPolygonPrimitive(__GLcontext* gc);
extern GLvoid __glChipDrawLineLoopPrimitive(__GLcontext* gc);
extern GLvoid __glChipDrawIndexedQuadListPrimitive(__GLcontext* gc);
extern GLvoid __glChipDrawIndexedQuadStripPrimitive(__GLcontext* gc);
extern GLvoid __glChipDrawIndexedPolygonPrimitive(__GLcontext* gc);
extern GLvoid __glChipBegin(__GLcontext* gc, GLenum mode);
extern GLvoid __glChipEnd(__GLcontext* gc);

#ifdef __cplusplus
}
#endif

#endif /* __chip_draw_h_ */
