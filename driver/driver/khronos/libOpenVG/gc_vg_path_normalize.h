/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vg_path_normalize_h__
#define __gc_vg_path_normalize_h__

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/*---------------------- Normalization function type. ------------------------*/

/* Segment normalization function parameters. */
#define vgmNORMALIZEPARAMETERS \
    vgsCONTEXT_PTR Context, \
    vgsPATHWALKER_PTR Source, \
    gctFLOAT_PTR Coordinates, \
    gceVGCMD_PTR Group, \
    gceVGCMD_PTR Command

typedef void (* vgtNORMALIZEFUNCTION) (
    vgmNORMALIZEPARAMETERS
    );

/*----------------------------------------------------------------------------*/
/*---------------------------- Normalization API. ----------------------------*/

void vgfGetNormalizationArray(
    vgsCONTEXT_PTR Context,
    vgtNORMALIZEFUNCTION ** Array,
    gctUINT_PTR Count
    );

#ifdef __cplusplus
}
#endif

#endif
