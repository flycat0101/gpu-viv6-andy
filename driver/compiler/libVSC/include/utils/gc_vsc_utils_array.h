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


#ifndef __gc_vsc_utils_array_h_
#define __gc_vsc_utils_array_h_

BEGIN_EXTERN_C()

#define VSC_INVALID_ARRAY_INDEX       0xFFFFFFFF

typedef gctBOOL (*PFN_VSC_ARRAY_ELE_CMP)(void* pElement1, void* pElement2);

/* A simple resizable array definition */
typedef struct _VSC_SIMPLE_RESIZABLE_ARRAY
{
    void*                     pElement;
    gctUINT                   elementSize;
    gctUINT                   allocatedCount;
    gctUINT                   elementCount;

    /* To check whether element is equal. If it is set to NULL, user must assure
       he won't call vscSRARR_RemoveElementByContent */
    PFN_VSC_ARRAY_ELE_CMP     pfnEleCmp;

    /* What type of MM are this SR-array built on? */
    VSC_MM*                   pMM;
}VSC_SIMPLE_RESIZABLE_ARRAY;

/* Creation and destroy */
VSC_SIMPLE_RESIZABLE_ARRAY* vscSRARR_Create(VSC_MM* pMM, gctUINT initAllocEleCount, gctUINT elementSize, PFN_VSC_ARRAY_ELE_CMP pfnEleCmp);
VSC_ErrCode vscSRARR_Initialize(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, VSC_MM* pMM, gctUINT initAllocEleCount, gctUINT elementSize, PFN_VSC_ARRAY_ELE_CMP pfnEleCmp);
void vscSRARR_Finalize(VSC_SIMPLE_RESIZABLE_ARRAY* pArray);
void vscSRARR_Clear(VSC_SIMPLE_RESIZABLE_ARRAY* pArray);
void vscSRARR_Destroy(VSC_SIMPLE_RESIZABLE_ARRAY* pArray);

/* Operations */

VSC_ErrCode vscSRARR_SetElementCount(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, gctUINT newEleCount); /* Directly mark how many elements are used in array */
gctUINT vscSRARR_GetElementCount(VSC_SIMPLE_RESIZABLE_ARRAY* pArray);
gctUINT vscSRARR_AddElement(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, void* pNewEle);
gctUINT vscSRARR_AddElementToSpecifiedIndex(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, void* pNewEle, gctINT index);
void* vscSRARR_GetNextEmpty(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, gctUINT *pIndex);
void* vscSRARR_GetElement(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, gctUINT index);
gctUINT vscSRARR_GetElementIndexByContent(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, void* pEle);

/* Note that following 2 functions will change the indices of elements after target element which is to be removed.
   For vscSRARR_RemoveElementByContent, user must assure all elements have different contents. */
void vscSRARR_RemoveElementByIndex(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, gctUINT index);
gctBOOL vscSRARR_RemoveElementByContent(VSC_SIMPLE_RESIZABLE_ARRAY* pArray, void* pEleToRemove);

END_EXTERN_C()

#endif /* __gc_vsc_utils_array_h_ */

