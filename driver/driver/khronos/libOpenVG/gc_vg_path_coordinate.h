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


#ifndef __gc_vg_path_coordinate_h__
#define __gc_vg_path_coordinate_h__

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/*-------------------- Coordinate access function types. ---------------------*/

typedef VGfloat (* vgtGETCOORDINATE) (
    vgsCONTEXT_PTR Context,
    vgsPATHWALKER_PTR Walker
    );

typedef void (* vgtSETCOORDINATE) (
    vgsCONTEXT_PTR Context,
    vgsPATHWALKER_PTR Walker,
    gctFLOAT Coordinate
    );

typedef VGfloat (* vgtGETCOPYCOORDINATE) (
    vgsCONTEXT_PTR Context,
    vgsPATHWALKER_PTR Source,
    vgsPATHWALKER_PTR Destination
    );

/*----------------------------------------------------------------------------*/
/*-------------------------- Coordinate access API. --------------------------*/

void vgfGetCoordinateAccessArrays(
    vgsCONTEXT_PTR Context,
    gctFLOAT Scale,
    gctFLOAT Bias,
    vgtGETCOORDINATE ** GetArray,
    vgtSETCOORDINATE ** SetArray,
    vgtGETCOPYCOORDINATE ** GetCopyArray
    );

#ifdef __cplusplus
}
#endif

#endif
