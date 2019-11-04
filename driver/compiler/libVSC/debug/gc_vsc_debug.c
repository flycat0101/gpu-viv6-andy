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


#include "gc_vsc.h"

#define VSC_DI_INVALID_STRING  0xffffffff

#define VSC_DI_OUT_OF_MEMORY 0xffffffff

#define vsdDIPRINT  gcmPRINT

#define vsdTEST_API 0

#define VSC_DI_DIE_PTR(id) ((((id) >= 0) && ((id) <= context->dieTable.usedCount)) ? (&context->dieTable.die[(id)]) : gcvNULL)
#define VSC_DI_HW_LOC_PTR(id) (((id) == VSC_DI_INVALID_HW_LOC ? gcvNULL : (&context->locTable.loc[id])))
#define VSC_DI_SW_LOC_PTR(id) (((id) == VSC_DI_INVALID_SW_LOC ? gcvNULL : (&context->swLocTable.loc[id])))

#define VSC_DI_DIETYPE_IS_VECTOR(die) ((die->tag == VSC_DI_TAG_TYPE \
                                        || die->tag == VSC_DI_TAG_VARIABE \
                                        || die->tag == VSC_DI_TAG_PARAMETER) \
                                        && die->u.variable.type.isPrimitiveType \
                                        && die->u.variable.type.type > 17 \
                                        && die->u.variable.type.type < 90 )

static gctPOINTER _ReallocateBuffer(VSC_DIContext * context, gctPOINTER srcPtr, gctUINT size, gctUINT initSize, gctUINT * reallocSize)
{
    gctPOINTER ptr;
    gctUINT newSize;

    if (size == 0)
    {
        newSize = initSize;
    }
    else
    {
        newSize = 2 * size;
    }

    if (gcmIS_ERROR(context->pfnAllocate(gcvNULL, newSize, &ptr)))
    {
        vsdDIPRINT("out of memory when allocate strTable");
        return gcvNULL;
    }

    if (size > 0)
    {
        gcoOS_MemCopy(ptr, srcPtr, size);
    }

    if (reallocSize)
    {
        *reallocSize = newSize;
    }

    if (srcPtr)
    {
        context->pfnFree(gcvNULL, srcPtr);
    }

    return (gctPOINTER) ptr;
}

static gctUINT _LookUpInStrTable(VSC_DIContext * context, gctCONST_STRING src)
{
    gctCHAR* pos;
    gctSIZE_T len0;
    gctSIZE_T len1;
    gctBOOL found = gcvFALSE;

    pos = context->strTable.str;
    if (context->strTable.str != gcvNULL)
    {
        gcoOS_StrLen(src, &len0);

        if (len0 == 0)
            return VSC_DI_INVALID_STRING;

        while(pos < (context->strTable.str + context->strTable.usedSize))
        {
            gcoOS_StrLen(pos, &len1);

            if ((len0 == len1) &&  (gcoOS_StrCmp(pos, src) == gcvSTATUS_OK))
            {
                found = gcvTRUE;
                break;
            }

            pos += len1 + 1;
        }

        if (found)
        {
            return (gctUINT)(pos - context->strTable.str);
        }
    }

    return VSC_DI_INVALID_STRING;
}

gctUINT _vscGetNameID(VSC_DIContext * context, gctCONST_STRING src)
{
    gctUINT ret;
    gctSIZE_T bytes;
    gctSTRING ptr;
    gctUINT newSize;

    ret = _LookUpInStrTable(context, src);

    if (VSC_DI_INVALID_STRING == ret)
    {
        gcoOS_StrLen(src, &bytes);

        ret = context->strTable.usedSize;

        if ((gctINT)(context->strTable.usedSize + bytes + 1) > context->strTable.size)
        {
            ptr = (gctSTRING)_ReallocateBuffer(context, (gctPOINTER)context->strTable.str, context->strTable.size, VSC_DI_STRTABLE_INIT_SIZE, &newSize);

            context->strTable.usedSize = context->strTable.size;
            ret = context->strTable.size;
            context->strTable.size = newSize;
            context->strTable.str = ptr;
        }

        ptr = context->strTable.str + context->strTable.usedSize;

        if (bytes > 0) gcoOS_MemCopy(ptr, src, bytes);
        context->strTable.usedSize += bytes + 1;
        *(ptr + bytes) = '\0';
        /*if name is strct/union will add the declare name in other time ,so need reserved some space for it*/
        if(gcmIS_SUCCESS(gcoOS_StrCmp(src, "struct "))||gcmIS_SUCCESS(gcoOS_StrCmp(src, "union ")))
        {
            if ((gctINT)(context->strTable.usedSize + 50) > context->strTable.size)
            {
                ptr = (gctSTRING)_ReallocateBuffer(context, (gctPOINTER)context->strTable.str, context->strTable.size, VSC_DI_STRTABLE_INIT_SIZE, &newSize);

                context->strTable.size = newSize;
                context->strTable.str = ptr;
            }
            context->strTable.usedSize += 50;
        }
    }

    return ret;
}

gctSTRING _GetNameStr(VSC_DIContext * context, gctUINT id)
{
    if (id >= context->strTable.size)
        return gcvNULL;

    return (gctSTRING)(&context->strTable.str[id]);
}

gceSTATUS _SetNameStr(VSC_DIContext * context, gctUINT id, gctSTRING name)
{
    gctSIZE_T len;
    gcoOS_StrLen(name, &len);

    if (id >= context->strTable.size || name == gcvNULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    len = context->strTable.size - id < len ? context->strTable.size - id : len;
    gcoOS_StrCopySafe(&context->strTable.str[id], len + 1, name);
    return gcvSTATUS_OK;
}

gctCONST_STRING _GetTagNameStr(VSC_DIContext * context, VSC_DIE_TAG tag)
{
    switch (tag)
    {
        case VSC_DI_TAG_INVALID: return "invalid";
        case VSC_DI_TAG_COMPILE_UNIT: return "cu";
        case VSC_DI_TAG_VARIABE: return "variable";
        case VSC_DI_TAG_SUBPROGRAM: return "subProgram";
        case VSC_DI_TAG_LEXICALBLOCK: return "lex block";
        case VSC_DI_TAG_PARAMETER: return "parameter";
        case VSC_DI_TAG_CONSTANT: return "const";
        case VSC_DI_TAG_TYPE: return "type";
    }

    return gcvNULL;
}

static VSC_DIE * _nextDIE(VSC_DIContext * context)
{
    gctUINT newSize;
    VSC_DIE * die;

    if (context->dieTable.usedCount == context->dieTable.count)
    {
        context->dieTable.die = (VSC_DIE *)_ReallocateBuffer(context,(gctPOINTER)context->dieTable.die,
                                                  (gctINT)(gcmSIZEOF(VSC_DIE) * context->dieTable.count),
                                                  (gcmSIZEOF(VSC_DIE) * VSC_DI_DIETABLE_INIT_COUNT),
                                                  &newSize);

        context->dieTable.count = (gctUINT16) (newSize / gcmSIZEOF(VSC_DIE));

        if (context->dieTable.die == gcvNULL)
        {
            context->dieTable.count = 0;
            context->dieTable.usedCount = 0;
            return gcvNULL;
        }
    }

    die = VSC_DI_DIE_PTR(context->dieTable.usedCount);

    gcoOS_ZeroMemory(die, gcmSIZEOF(VSC_DIE));

    die->parent = VSC_DI_INVALIDE_DIE;
    die->child = VSC_DI_INVALIDE_DIE;
    die->sib = VSC_DI_INVALIDE_DIE;
    die->id = context->dieTable.usedCount;
    die->name = VSC_DI_INVALID_STRING;

    context->dieTable.usedCount++;

    return die;
}

static VSC_DIE* _getCurrentSubProgramDie(VSC_DIContext * context, unsigned int pc)
{
    gctUINT line;
    VSC_DIE * die = gcvNULL;
    VSC_DIE * innerDie = gcvNULL;

    if (vscDIGetSrcLineByPC((void *)context, pc, &line))
    {
        /* look into the die table to find the die match the pc */
        die = &context->dieTable.die[context->cu];
        die = VSC_DI_DIE_PTR(die->child);

        while (die)
        {
            if((die->tag == VSC_DI_TAG_SUBPROGRAM) ||
                (die->tag == VSC_DI_TAG_LEXICALBLOCK))
            {
                if((die->lineNo <= line) &&
                    (die->endLineNo >= line))
                {
                    innerDie = die;
                    die = VSC_DI_DIE_PTR(die->child);
                    continue;
                }
            }

            if ((die->lineNo <= line) &&
                (die->endLineNo >= line))
            {
                break;
            }

            innerDie = die;
            die = VSC_DI_DIE_PTR(die->sib);
        }

        /* look up subprogram from inner to outer */
        while (innerDie)
        {
            if (innerDie->tag == VSC_DI_TAG_SUBPROGRAM)
            {
                break;
            }

            innerDie = VSC_DI_DIE_PTR(innerDie->parent);
        }
    }

    return innerDie;
}

static void _addChildDIE(VSC_DIContext * context, VSC_DIE * parent, VSC_DIE * child)
{
    VSC_DIE * die;

    die = parent;

    if (die->child == VSC_DI_INVALIDE_DIE)
    {
        die->child = child->id;
        return;
    }

    die = VSC_DI_DIE_PTR(die->child);

    while(die->sib != VSC_DI_INVALIDE_DIE)
    {
        die = VSC_DI_DIE_PTR(die->sib);
    }

    die->sib = child->id;
}

static VSC_DIE * _newDIE(VSC_DIContext * context, VSC_DIE_TAG tag, gctUINT16 parentID)
{
    VSC_DIE * die;
    VSC_DIE * parent;

    die = _nextDIE(context);

    die->tag = tag;
    die->parent = parentID;

    parent = VSC_DI_DIE_PTR(parentID);

    if (parent != gcvNULL)
    {
        _addChildDIE(context, parent, die);
    }

    return die;
}

gctUINT16 vscDIGetDIEType(VSC_DIContext * context)
{
    VSC_DIE * die = gcvNULL;

    if (context)
    {
        die = _newDIE(context, VSC_DI_TAG_TYPE, context->cu);

        die->u.variable.type.type = VIR_TYPE_BOOLEAN;
        die->u.variable.type.isPrimitiveType = gcvTRUE;

        return die->id;
    }
    else
    {
        return VSC_DI_INVALIDE_DIE;
    }
}

VSC_DIE * vscDIGetDIE(VSC_DIContext * context, gctUINT16 id)
{
    if (context && (id != VSC_DI_INVALIDE_DIE))
    {
        return VSC_DI_DIE_PTR(id);
    }

    return gcvNULL;
}

gctUINT16 vscDIAddDIE(VSC_DIContext * context,
                      VSC_DIE_TAG tag,
                      gctUINT16 parentID,
                      gctCONST_STRING name,
                      gctUINT fileNo,
                      gctUINT lineNo,
                      gctUINT endLineNo,
                      gctUINT colNo
                      )
{
    VSC_DIE * die = gcvNULL;
    gctUINT nameID = VSC_DI_INVALID_STRING;

    if (context)
    {
        if (!context->collect &&
            tag != VSC_DI_TAG_COMPILE_UNIT)
        {
            return VSC_DI_INVALIDE_DIE;
        }

        if (name) nameID = _vscGetNameID(context, name);

        die = _newDIE(context, tag, parentID);

        if (die)
        {
            die->fileNo = (gctUINT8)fileNo;
            die->lineNo = (gctUINT16)lineNo;
            die->endLineNo = (gctUINT16)endLineNo;
            die->colNo = (gctUINT8)colNo;
            die->name = nameID;

            if (tag == VSC_DI_TAG_VARIABE ||
                tag == VSC_DI_TAG_PARAMETER)
            {
                die->u.variable.swLoc = VSC_DI_INVALID_SW_LOC;
            }
        }
        return die->id;
    }
    else
    {
        return VSC_DI_INVALIDE_DIE;
    }
}

void _DIDumpDIETree(VSC_DIContext * context, gctUINT16 id, gctUINT shift, gctUINT tag)
{
    VSC_DIE * die;
    gctUINT16 child;
    gctUINT16 sibling;

    if (id != VSC_DI_INVALIDE_DIE)
    {
        vscDIDumpDIE(context, id,shift, tag);

        die = VSC_DI_DIE_PTR(id);
        child = die->child;

        shift++;

        _DIDumpDIETree(context, child, shift, tag);

        shift--;

        sibling = die->sib;

        _DIDumpDIETree(context, sibling, shift, tag);
    }
}


VSC_DI_SW_LOC *
vscDIGetSWLoc(
    VSC_DIContext * context,
    gctUINT16 loc
    )
{
    if (context == gcvNULL || loc >= context->swLocTable.usedCount)
    {
        return gcvNULL;
    }

    return &context->swLocTable.loc[loc];
}

VSC_DI_SW_LOC *
vscDIFindSWLoc(
    VSC_DIContext * context,
    gctUINT32 regId
    )
{
    VSC_DI_SW_LOC *ret = gcvNULL;
    VSC_DI_SW_LOC *sl;
    gctUINT i;

    if (context == gcvNULL )
    {
        return gcvNULL;
    }

    for (i = 0 ; i < context->swLocTable.usedCount; i ++)
    {
        sl = &context->swLocTable.loc[i];

        if (sl->reg)
        {
            if (sl->u.reg.start <= regId &&
                sl->u.reg.end >= regId)
            {
                ret = sl;
                break;
            }
        }
    }

    return ret;
}


gctUINT16 vscDIAddSWLoc(VSC_DIContext * context)
{
    gctUINT newSize;
    VSC_DI_SW_LOC * loc;

    if (context == gcvNULL)
        return VSC_DI_INVALID_SW_LOC;

    if (context->swLocTable.usedCount == context->swLocTable.count)
    {
        context->swLocTable.loc = (VSC_DI_SW_LOC *)_ReallocateBuffer(context,(gctPOINTER)context->swLocTable.loc,
                                                                  (gctINT)(gcmSIZEOF(VSC_DI_SW_LOC) * context->swLocTable.count),
                                                                  (gcmSIZEOF(VSC_DI_SW_LOC) * VSC_DI_LOCTABLE_INIT_COUNT),
                                                                  &newSize);

        context->swLocTable.count = (gctUINT16) (newSize / gcmSIZEOF(VSC_DI_SW_LOC));

        if (context->swLocTable.loc == gcvNULL)
        {
            context->swLocTable.count = 0;
            context->swLocTable.usedCount = 0;
            return VSC_DI_INVALID_SW_LOC;
        }
    }

    loc = &context->swLocTable.loc[context->swLocTable.usedCount];
    loc->id = context->swLocTable.usedCount;
    loc->next = VSC_DI_INVALID_SW_LOC;
    loc->hwLoc = VSC_DI_INVALID_HW_LOC;

    context->swLocTable.usedCount++;

    return loc->id;
}

VSC_DI_HW_LOC *
vscDIGetHWLoc(
    VSC_DIContext * context,
    gctUINT16 loc
    )
{
    if (context == gcvNULL || loc >= context->locTable.usedCount)
    {
        return gcvNULL;
    }

    return &context->locTable.loc[loc];
}

gctUINT16 vscDIAddHWLoc(VSC_DIContext * context)
{
    gctUINT newSize;
    VSC_DI_HW_LOC * loc;

    if (context == gcvNULL)
        return VSC_DI_INVALID_HW_LOC;

    if (context->locTable.usedCount == context->locTable.count)
    {
        context->locTable.loc = (VSC_DI_HW_LOC *)_ReallocateBuffer(context,(gctPOINTER)context->locTable.loc,
                                                                  (gctINT)(gcmSIZEOF(VSC_DI_HW_LOC) * context->locTable.count),
                                                                  (gcmSIZEOF(VSC_DI_HW_LOC) * VSC_DI_LOCTABLE_INIT_COUNT),
                                                                  &newSize);

        context->locTable.count = (gctUINT16) (newSize / gcmSIZEOF(VSC_DI_HW_LOC));

        if (context->locTable.loc == gcvNULL)
        {
            context->locTable.count = 0;
            context->locTable.usedCount = 0;
            return VSC_DI_INVALID_HW_LOC;
        }
    }

    loc = &context->locTable.loc[context->locTable.usedCount];
    loc->id = context->locTable.usedCount;
    loc->next = VSC_DI_INVALID_HW_LOC;

    context->locTable.usedCount++;

    return loc->id;
}


/* BE RA call this function to set hwLoc to swLoc, as the swLoc table is from FE, the reg/mem is not excatly match the BE
   reg/mem, so, we use this swLoc to check inside swLoc table, insert the hwLoc under swLoc, this may involve split the SW loc
   to small one.
*/
void vscDISetHwLocToSWLoc(VSC_DIContext * context, VSC_DI_SW_LOC * swLoc, VSC_DI_HW_LOC * hwLoc)
{
    VSC_DI_SW_LOC * sl = gcvNULL, * sl_tmp = gcvNULL, * sl_tmp1 = gcvNULL;
    VSC_DI_HW_LOC * hl = gcvNULL;
    gctUINT     i, j, k;
    gctUINT16   id;
    gctBOOL     found = gcvFALSE, breakFlag = gcvFALSE;

    if (!swLoc || !hwLoc)
        return;

    if (swLoc->reg)
    {
        for (i = 0 ; i < context->swLocTable.usedCount; i ++)
        {
            sl = &context->swLocTable.loc[i];

            if (sl->reg)
            {
                if (swLoc->u.reg.type == VSC_DIE_REG_TYPE_CONST)
                {
                    if (sl->u.reg.type == VSC_DIE_REG_TYPE_TMP)
                    {
                        continue;
                    }

                    if (sl->u.reg.start == swLoc->u.reg.start &&
                        sl->u.reg.end == swLoc->u.reg.end)
                    {
                        found = gcvTRUE;
                    }
                }
                else
                {
                    if (sl->u.reg.type == VSC_DIE_REG_TYPE_CONST ||
                        sl->u.reg.start > swLoc->u.reg.end ||
                        sl->u.reg.end < swLoc->u.reg.start)
                    {
                        breakFlag = gcvFALSE;
                        for (j = 0; j < i; j ++)
                        {
                            sl_tmp = &context->swLocTable.loc[j];
                            if (sl_tmp->hwLoc != VSC_DI_INVALID_HW_LOC &&
                                sl->u.reg.start == sl_tmp->u.reg.start)
                            {
                                for (k = j; k < i; k ++)
                                {
                                    sl_tmp1 = &context->swLocTable.loc[k];
                                    if (sl_tmp1->hwLoc != VSC_DI_INVALID_HW_LOC &&
                                        sl->u.reg.end == sl_tmp1->u.reg.end)
                                    {
                                        sl->hwLoc = sl_tmp1->hwLoc;
                                        breakFlag = gcvTRUE;
                                        break;
                                    }
                                }
                            }
                            if (breakFlag == gcvTRUE)
                            {
                                break;
                            }
                        }
                        continue;
                    }

                    if (sl->u.reg.start == swLoc->u.reg.start &&
                        sl->u.reg.end == swLoc->u.reg.end)
                    {
                        found = gcvTRUE;
                    }
                    else
                    {
                        /* split the sw loc here */
                        /* create a sw loc and add to next */
                        VSC_DI_SW_LOC * newSWLoc = gcvNULL;
                        gctUINT16 newSWLocId = VSC_DI_INVALID_SW_LOC;

                        newSWLocId = vscDIAddSWLoc(context);

                        /* re-get the sl in case we flush out the loc table in addSwLoc */
                        sl = &context->swLocTable.loc[i];
                        sl->next = newSWLocId;

                        /* copy the information of the base sw loc */
                        newSWLoc = vscDIGetSWLoc(context, newSWLocId);
                        *newSWLoc = *sl;
                        newSWLoc->id = newSWLocId;
                        newSWLoc->next = VSC_DI_INVALID_SW_LOC;

                        /* set the reg to new start and new end */
                        sl->u.reg.start = swLoc->u.reg.start;
                        sl->u.reg.end = swLoc->u.reg.end;

                        found = gcvTRUE;
                    }
                }

                if (!found)
                {
                    continue;
                }

                hl = vscDIGetHWLoc(context, vscDIAddHWLoc(context));

                if (hl)
                {
                    id = hl->id;
                    *hl = *hwLoc;
                    hl->id = id;
                    hl->next = sl->hwLoc;
                    sl->hwLoc = hl->id;

                    if (gcmOPT_EnableDebugDump())
                    {
                        if (hl->reg)
                        {
                            if (hl->u.reg.type == VSC_DIE_HW_REG_CONST)
                            {
                                gcmPRINT("| uniform[%d,%d]    -> [pc%d, pc%d] c[%d,%d] |",
                                    swLoc->u.reg.start, swLoc->u.reg.end, hl->beginPC, hl->endPC, hl->u.reg.start, hl->u.reg.end);
                            }
                            else
                            {
                                gcmPRINT("| reg[%d,%d]    -> [pc%d, pc%d] r[%d,%d] |",
                                swLoc->u.reg.start, swLoc->u.reg.end, hl->beginPC, hl->endPC, hl->u.reg.start, hl->u.reg.end);
                            }
                        }
                        else
                        {
                            gcmPRINT("| reg[%d,%d]    -> [pc%d, pc%d] r%d[0x%x,0x%x] |",
                                swLoc->u.reg.start, swLoc->u.reg.end,
                                hl->beginPC, hl->endPC,
                                hl->u.offset.baseAddr.start,
                                hl->u.offset.offset, hl->u.offset.endOffset);
                        }
                    }
                    if (hwLoc->u.reg.type == VSC_DIE_HW_REG_CONST)
                    {
                        /* when setting the hw location for uniforms,
                        need to find all the uniforms used in multiple kernels */
                        found = gcvFALSE;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    gcmASSERT(0);
                    return;
                }
            }
        }

        if (gcmOPT_EnableDebugDump() && (i == context->swLocTable.usedCount))
        {
            gcmPRINT("| reg[%d,%d]    -> none                       |", swLoc->u.reg.start, swLoc->u.reg.end);
        }
    }
    else
    {
        for (i = 0 ; i < context->swLocTable.usedCount; i ++)
        {
            sl = &context->swLocTable.loc[i];

            if (!sl->reg)
            {
                if (sl->u.offset.baseAddr.type != swLoc->u.offset.baseAddr.type ||
                    sl->u.offset.baseAddr.start != swLoc->u.offset.baseAddr.start)
                {
                    continue;
                }

                if (sl->u.offset.offset > swLoc->u.offset.endOffset||
                    sl->u.offset.endOffset < swLoc->u.offset.offset)
                {
                    continue;
                }

                if (sl->u.offset.offset == swLoc->u.offset.offset&&
                    sl->u.offset.endOffset == swLoc->u.offset.endOffset)
                {
                    hl = vscDIGetHWLoc(context, vscDIAddHWLoc(context));

                    if (hl)
                    {
                        id = hl->id;
                        *hl = *hwLoc;
                        hl->id = id;
                        hl->next = sl->hwLoc;
                        sl->hwLoc = hl->id;

                        if (gcmOPT_EnableDebugDump())
                        {
                            if (hl->reg)
                            {
                                if (hl->u.reg.type == VSC_DIE_HW_REG_CONST)
                                {
                                    gcmPRINT("| 0x%x[0x%x,0x%x]    -> [pc%d, pc%d] c[%d,%d] |",
                                        swLoc->u.reg.start, swLoc->u.reg.end, hl->beginPC, hl->endPC, hl->u.reg.start, hl->u.reg.end);
                                }
                                else
                                {
                                    gcmPRINT("| 0x%x[0x%x,0x%x]    -> [pc%d, pc%d] r[%d,%d] |",
                                        swLoc->u.reg.start, swLoc->u.reg.end, hl->beginPC, hl->endPC, hl->u.reg.start, hl->u.reg.end);
                                }
                            }
                            else
                            {
                                gcmPRINT("| 0x%x[0x%x,0x%x]    -> [pc%d, pc%d] r%d[0x%x,0x%x] |",
                                    swLoc->u.offset.baseAddr, swLoc->u.offset.offset, swLoc->u.offset.endOffset,
                                    hl->beginPC, hl->endPC,
                                    hl->u.offset.baseAddr.start,
                                    hl->u.offset.offset, hl->u.offset.endOffset);
                            }
                        }
                        break;
                    }
                    else
                    {
                        gcmASSERT(0);
                        return;
                    }
                }
                else
                {
                    /* TO_DO */
                    gcmASSERT(0);
                }
            }
        }

        if (gcmOPT_EnableDebugDump()&& (i == context->swLocTable.usedCount))
        {
            gcmPRINT("| 0x%x[0x%x,0x%x]    -> none                       |",
                swLoc->u.offset.baseAddr, swLoc->u.offset.offset, swLoc->u.offset.endOffset);
        }
    }
}

void vscDIChangeUniformSWLoc(VSC_DIContext * context, gctUINT tmpStart, gctUINT tmpEnd, gctUINT uniformIdx)
{
    VSC_DI_SW_LOC * sl = gcvNULL;
    gctUINT i;

    if (context == gcvNULL)  return;

    for (i = 0 ; i < context->swLocTable.usedCount; i ++)
    {
        sl = &context->swLocTable.loc[i];

        if (sl->reg)
        {
            if (sl->u.reg.start == tmpStart &&
                sl->u.reg.end == tmpEnd)
            {
                sl->u.reg.type = VSC_DIE_REG_TYPE_CONST;
                sl->u.reg.start = (gctUINT16) uniformIdx;
                sl->u.reg.end = (gctUINT16) uniformIdx;
                break;
            }
        }
    }
}


gceSTATUS vscDIAddLineTable(VSC_DIContext * context, gctUINT count)
{
    if (context == gcvNULL)
        return gcvSTATUS_OK;

    if (context->lineTable.count != 0)
        return gcvSTATUS_OK;

    if (context->pfnAllocate(gcvNULL, count *sizeof(VSC_DI_LINE_TABLE_MAP),
        (gctPOINTER*)&context->lineTable.map) != gcvSTATUS_OK)
    {
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    context->lineTable.count = count;

    return gcvSTATUS_OK;
}

gceSTATUS vscDIAddLineMap(VSC_DIContext * context, gctUINT id, VIR_SourceFileLoc sourceLoc, gctUINT start, gctUINT end)
{
    if (context == gcvNULL)
        return gcvSTATUS_OK;

    if (context->lineTable.count <= id)
        return gcvSTATUS_INVALID_INDEX;

    context->lineTable.map[id].sourcLoc = sourceLoc;
    context->lineTable.map[id].mcRange = VSC_DI_MC_RANGE(start, end);

    return gcvSTATUS_OK;
}

void vscDIDumpLineTable(VSC_DIContext * context)
{
    gctUINT i;
    gctUINT file;
    gctUINT line;
    gctUINT col;
    gctUINT start;
    gctUINT end;

    if (context == gcvNULL || context->lineTable.map == gcvNULL)
        return;

    gcmPRINT("--------------line table----------------");

    for (i = 0; i < context->lineTable.count; i++)
    {
        file = context->lineTable.map[i].sourcLoc.fileId;
        line = context->lineTable.map[i].sourcLoc.lineNo;
        col  = context->lineTable.map[i].sourcLoc.colNo;
        start = VSC_DI_MC_RANGE_START(context->lineTable.map[i].mcRange);
        end   = VSC_DI_MC_RANGE_END(context->lineTable.map[i].mcRange);

        gcmPRINT("|   source(%d,%d,%d)         pc(%d,%d)      |", file, line, col, start, end);
    }
    gcmPRINT("---------------------------------------------");
}

void vscDIDumpDIETree(VSC_DIContext * context, gctUINT16 id, gctUINT tag)
{
    if (context)
    {
        gcmPRINT("------------------------------------------DIE TREE id = %d---------------------------------------", id);
        _DIDumpDIETree(context,id, 0, tag);
        gcmPRINT("-------------------------------------------------------------------------------------------------");
    }
#if vsdTEST_API
    {
        char funcName[1024];
        char varName[1024];
        char typeName[1024];
        unsigned int lowPC;
        unsigned int highPC;
        unsigned int hwLocCount;
        unsigned int childrenCount, childrenCount1, childrenCount2, childrenCount3, childrenCount4;
        unsigned int data0, data1, data2, data3;
        int variableCount, argCount, i, j, k, l, m;
        gctBOOL isReg;
        gctBOOL isConst;
        int funcId = 1; /* please change this function id when changing the test case */
        char funIdStr[1024];
        char varIdStr[1024];
        char varIdStr2[1024];
        char varIdStr3[1024];
        char varIdStr4[1024];
        char varIdStr5[1024];
        vscDIGetFunctionInfo((void *)context, funcId, funcName, 1024, &lowPC, &highPC);
        argCount      = vscDIGetVariableCount((void *)context, funcId, gcvTRUE);
        variableCount = vscDIGetVariableCount((void *)context, funcId, gcvFALSE);
        gcoOS_PrintStrSafe(funIdStr, 10, 0, "%d",funcId);
        for (i = 0; i < argCount; i++)
        {
            vscDIGetVariableInfo((void *)context, funIdStr, i, gcvTRUE, varName, typeName, 1024, varIdStr, &lowPC, &highPC, &hwLocCount, &childrenCount);
            for (j = 0; j < (int)childrenCount; j++)
            {
                vscDIGetVariableInfo((void *)context, varIdStr, j, gcvFALSE, varName, typeName, 1024, varIdStr2, &lowPC, &highPC, &hwLocCount, &childrenCount1);
                for (k = 0; k < (int)childrenCount1; k++)
                {
                    vscDIGetVariableInfo((void *)context, varIdStr2, k, gcvFALSE, varName, typeName, 1024, varIdStr3, &lowPC, &highPC, &hwLocCount, &childrenCount2);
                    for (l = 0; l < (int)childrenCount2; l++)
                    {
                        vscDIGetVariableInfo((void *)context, varIdStr3, l, gcvFALSE, varName, typeName, 1024, varIdStr4, &lowPC, &highPC, &hwLocCount, &childrenCount3);
                        for (m = 0; m < (int)childrenCount3; m++)
                        {
                            vscDIGetVariableInfo((void *)context, varIdStr4, m, gcvFALSE, varName, typeName, 1024, varIdStr5, &lowPC, &highPC, &hwLocCount, &childrenCount4);
                        }
                    }
                }
            }
            for (j = 0; j < (int) hwLocCount; j++)
            {
                vscDIGetVariableHWLoc((void *)context, varIdStr, j, &isReg, &isConst, &lowPC, &highPC, &data0, &data1, &data2, &data3);
            }
        }
        for (i = 0; i < variableCount; i++)
        {
            vscDIGetVariableInfo((void *)context, funIdStr, i, gcvFALSE, varName, typeName, 1024, varIdStr, &lowPC, &highPC, &hwLocCount, &childrenCount);
            for (j = 0; j < (int)childrenCount; j++)
            {
                vscDIGetVariableInfo((void *)context, varIdStr, j, gcvFALSE, varName, typeName, 1024, varIdStr2, &lowPC, &highPC, &hwLocCount, &childrenCount1);
                for (k = 0; k < (int)childrenCount1; k++)
                {
                    vscDIGetVariableInfo((void *)context, varIdStr2, k, gcvFALSE, varName, typeName, 1024, varIdStr3, &lowPC, &highPC, &hwLocCount, &childrenCount2);
                    for (l = 0; l < (int)childrenCount2; l++)
                    {
                        vscDIGetVariableInfo((void *)context, varIdStr3, l, gcvFALSE, varName, typeName, 1024, varIdStr4, &lowPC, &highPC, &hwLocCount, &childrenCount3);
                        for (m = 0; m < (int)childrenCount3; m++)
                        {
                            vscDIGetVariableInfo((void *)context, varIdStr4, m, gcvFALSE, varName, typeName, 1024, varIdStr5, &lowPC, &highPC, &hwLocCount, &childrenCount4);
                        }
                    }
                }
            }
            for (j = 0; j < (int) hwLocCount; j++)
            {
                vscDIGetVariableHWLoc((void *)context, varIdStr, j, &isReg, &isConst, &lowPC, &highPC, &data0, &data1, &data2, &data3);
            }
        }
    }
#endif
}

#define VSC_DI_DUMP_DIE_LINE_HEADER()   \
            offset = 0; \
            context->tmpLog[offset] = '\0'; \
            gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,"|"); \
            while (tmp--) \
            { \
               gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,"    "); \
            }

void vscDIDumpDIE(VSC_DIContext * context, gctUINT16 id, gctUINT shift, gctUINT tag)
{
    gctUINT offset = 0;
    VSC_DIE * die;
    gctUINT tmp = shift;
    static const char * _strSwizzle[] =
    {
        ".x", ".yx", ".zx", ".wx", ".xyx", ".yyx", ".zyx", ".wyx", ".xzx", ".yzx", ".zzx", ".wzx", ".xwx", ".ywx", ".zwx", ".wwx",
        ".xxyx", ".yxyx", ".zxyx", ".wxyx", ".xyyx", ".yyyx", ".zyyx", ".wyyx", ".xzyx", ".yzyx", ".zzyx", ".wzyx", ".xwyx", ".ywyx", ".zwyx", ".wwyx",
        ".xxzx", ".yxzx", ".zxzx", ".wxzx", ".xyzx", ".yyzx", ".zyzx", ".wyzx", ".xzzx", ".yzzx", ".zzzx", ".wzzx", ".xwzx", ".ywzx", ".zwzx", ".wwzx",
        ".xxwx", ".yxwx", ".zxwx", ".wxwx", ".xywx", ".yywx", ".zywx", ".wywx", ".xzwx", ".yzwx", ".zzwx", ".wzwx", ".xwwx", ".ywwx", ".zwwx", ".wwwx",
        ".xxxy", ".yxxy", ".zxxy", ".wxxy", ".xyxy", ".yyxy", ".zyxy", ".wyxy", ".xzxy", ".yzxy", ".zzxy", ".wzxy", ".xwxy", ".ywxy", ".zwxy", ".wwxy",
        ".xxy", ".yxy", ".zxy", ".wxy", ".xy", ".y", ".zy", ".wy", ".xzy", ".yzy", ".zzy", ".wzy", ".xwy", ".ywy", ".zwy", ".wwy",
        ".xxzy", ".yxzy", ".zxzy", ".wxzy", ".xyzy", ".yyzy", ".zyzy", ".wyzy", ".xzzy", ".yzzy", ".zzzy", ".wzzy", ".xwzy", ".ywzy", ".zwzy", ".wwzy",
        ".xxwy", ".yxwy", ".zxwy", ".wxwy", ".xywy", ".yywy", ".zywy", ".wywy", ".xzwy", ".yzwy", ".zzwy", ".wzwy", ".xwwy", ".ywwy", ".zwwy", ".wwwy",
        ".xxxz", ".yxxz", ".zxxz", ".wxxz", ".xyxz", ".yyxz", ".zyxz", ".wyxz", ".xzxz", ".yzxz", ".zzxz", ".wzxz", ".xwxz", ".ywxz", ".zwxz", ".wwxz",
        ".xxyz", ".yxyz", ".zxyz", ".wxyz", ".xyyz", ".yyyz", ".zyyz", ".wyyz", ".xzyz", ".yzyz", ".zzyz", ".wzyz", ".xwyz", ".ywyz", ".zwyz", ".wwyz",
        ".xxz", ".yxz", ".zxz", ".wxz", ".xyz", ".yyz", ".zyz", ".wyz", ".xz", ".yz", ".z", ".wz", ".xwz", ".ywz", ".zwz", ".wwz",
        ".xxwz", ".yxwz", ".zxwz", ".wxwz", ".xywz", ".yywz", ".zywz", ".wywz", ".xzwz", ".yzwz", ".zzwz", ".wzwz", ".xwwz", ".ywwz", ".zwwz", ".wwwz",
        ".xxxw", ".yxxw", ".zxxw", ".wxxw", ".xyxw", ".yyxw", ".zyxw", ".wyxw", ".xzxw", ".yzxw", ".zzxw", ".wzxw", ".xwxw", ".ywxw", ".zwxw", ".wwxw",
        ".xxyw", ".yxyw", ".zxyw", ".wxyw", ".xyyw", ".yyyw", ".zyyw", ".wyyw", ".xzyw", ".yzyw", ".zzyw", ".wzyw", ".xwyw", ".ywyw", ".zwyw", ".wwyw",
        ".xxzw", ".yxzw", ".zxzw", ".wxzw", "", ".yyzw", ".zyzw", ".wyzw", ".xzzw", ".yzzw", ".zzzw", ".wzzw", ".xwzw", ".ywzw", ".zwzw", ".wwzw",
        ".xxw", ".yxw", ".zxw", ".wxw", ".xyw", ".yyw", ".zyw", ".wyw", ".xzw", ".yzw", ".zzw", ".wzw", ".xw", ".yw", ".zw", ".w",
    };

    if (context)
    {
        die = VSC_DI_DIE_PTR(id);
        if (die != gcvNULL)
        {
            if (!(tag & (1 << die->tag)))
                return;

            VSC_DI_DUMP_DIE_LINE_HEADER();

            if (die->tag == VSC_DI_TAG_VARIABE ||
                die->tag == VSC_DI_TAG_PARAMETER)
            {
                gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                   "id = %d tag = %s parent = %d line (%d,%d,%d) name = \"%s\", type(%d, %d, %d, %d, %d), pc(%d,%d)\n",
                   die->id, _GetTagNameStr(context, die->tag),die->parent, die->fileNo, die->lineNo, die->colNo, _GetNameStr(context,die->name),
                   die->u.variable.type.isPrimitiveType, die->u.variable.type.isPointer, die->u.variable.type.type, die->u.variable.type.array.numDim, die->u.variable.type.array.length[0],
                   die->u.variable.pcLine[0], die->u.variable.pcLine[1]);

                gcmPRINT(context->tmpLog);

                /* Dump loc */
                {
                    VSC_DI_SW_LOC * sl;
                    VSC_DI_HW_LOC * hl;

                    sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);

                    while (sl)
                    {
                        tmp = shift + 1;
                        VSC_DI_DUMP_DIE_LINE_HEADER();

                        gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                            "---------------------------");
                        gcmPRINT(context->tmpLog);

                        tmp = shift + 1;
                        VSC_DI_DUMP_DIE_LINE_HEADER();

                        if (sl->reg)
                        {
                            if (sl->u.reg.type == VSC_DIE_REG_TYPE_CONST)
                            {
                                gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                                   "|SWLoc[%d] uniform[%d,%d]        |", sl->id, sl->u.reg.start, sl->u.reg.end);
                            }
                            else
                            {
                                gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                                   "|SWLoc[%d] reg[%d,%d]        |", sl->id, sl->u.reg.start, sl->u.reg.end);
                            }
                        }
                        else
                        {
                            gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                               "|SWLoc[%d] %d[0x%x,0x%x]        |",sl->id, sl->u.offset.baseAddr.start,
                                               sl->u.offset.offset, sl->u.offset.endOffset);
                        }
                        gcmPRINT(context->tmpLog);

                        hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, sl->hwLoc);

                        while(hl)
                        {
                            tmp = shift + 1;
                            VSC_DI_DUMP_DIE_LINE_HEADER();

                            if (hl->reg)
                            {
                                if (hl->u.reg.type == VSC_DIE_HW_REG_CONST)
                                {
                                    gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                                   "|[pc%d, pc%d]  |  c(%d,%d) %s |",
                                                   hl->beginPC, hl->endPC, hl->u.reg.start, hl->u.reg.end, _strSwizzle[hl->u1.swizzle]);
                                }
                                else
                                {
                                    gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                                   "|[pc%d, pc%d]  |  r(%d,%d) < %d |",
                                                   hl->beginPC, hl->endPC, hl->u.reg.start, hl->u.reg.end, hl->u1.hwShift);
                                }
                            }
                            else
                            {
                                if (hl->u.offset.baseAddr.type == VSC_DIE_HW_REG_CONST)
                                {
                                    gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                                    "|[pc%d, pc%d]  |  c%d(0x%x,0x%x)  |",
                                                    hl->beginPC, hl->endPC,
                                                    hl->u.offset.baseAddr.start,
                                                    hl->u.offset.offset, hl->u.offset.endOffset);
                                }
                                else
                                {
                                    gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                                    "|[pc%d, pc%d]  |  r%d(0x%x,0x%x)  |",
                                                    hl->beginPC, hl->endPC,
                                                    hl->u.offset.baseAddr.start,
                                                    hl->u.offset.offset, hl->u.offset.endOffset);
                                }
                            }

                            hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, hl->next);
                            gcmPRINT(context->tmpLog);
                        }

                        tmp = shift + 1;
                        VSC_DI_DUMP_DIE_LINE_HEADER();

                        gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                            "---------------------------");
                        gcmPRINT(context->tmpLog);

                        sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
                    }
                }
                if(die->useMemory == gcvTRUE)
                {
                    tmp = shift;
                    VSC_DI_DUMP_DIE_LINE_HEADER();
                    gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                "| useMemory = %d size = %d Offset = %d |",
                                die->useMemory, die->size, die->alignmenOffset);
                    gcmPRINT(context->tmpLog);
                }
            }
            else if (die->tag == VSC_DI_TAG_TYPE)
            {
                if (!die->u.type.isPrimitiveType)
                {
                    VSC_DIE * typeDie = gcvNULL;
                    gctSTRING typeName = gcvNULL;

                    gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                        "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, defined type id = %d",
                        die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo,
                        die->colNo, _GetNameStr(context,die->name),
                        die->u.type.type);

                    if (die->u.type.type != 0)
                        typeDie = VSC_DI_DIE_PTR(die->u.type.type);
                    if (typeDie)
                    {
                        typeName = _GetNameStr(context, typeDie->name);
                    }
                    if (typeDie && typeName && typeName[0] != '\0' )
                    {
                        if (!typeDie->u.type.isPrimitiveType &&
                            (gcmIS_SUCCESS(gcoOS_StrCmp(typeName, "struct "))||
                             gcmIS_SUCCESS(gcoOS_StrCmp(typeName, "union "))))
                        {
                            gctSTRING dieName = _GetNameStr(context,die->name);
                            char * tempstr;
                            gctPOINTER memory;
                            gcoOS_AllocateMemory(NULL, 1024*sizeof(char), &memory);
                            tempstr = (char*)memory;
                            gcoOS_StrCopySafe(tempstr, 1024, typeName);
                            gcoOS_StrCatSafe(tempstr, 1024, dieName);
                            _SetNameStr(context, typeDie->name, tempstr);
                        }
                    }
                }
                else
                {
                    if (die->u.type.array.numDim > 0)
                    {
                        gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                            "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, VIR primitive type ID = %d, %d {%d, %d, %d, %d}",
                            die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo, die->lineNo, die->colNo, _GetNameStr(context,die->name),
                            die->u.type.isPrimitiveType, die->u.type.array.numDim, die->u.type.array.length[0],die->u.type.array.length[1],
                            die->u.type.array.length[2],die->u.type.array.length[3]);
                    }
                    else
                    {
                        gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                            "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, VIR primitive type ID = %d",
                            die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name),
                            die->u.type.isPrimitiveType);
                    }
                }
                gcmPRINT(context->tmpLog);
                if(die->size > 0)
                {
                    tmp = shift;
                    VSC_DI_DUMP_DIE_LINE_HEADER();
                    if(die->u.type.type == 0)
                    {
                        gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                            "| size = %d alignment = %d |",
                             die->size, die->alignmentSize);
                    }
                    else
                    {
                        gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                            "| size = %d alignment = %d alignmentOffset = %d |",
                             die->size, die->alignmentSize, die->alignmenOffset);
                    }

                    gcmPRINT(context->tmpLog);
                }
            }
            else if (die->tag == VSC_DI_TAG_SUBPROGRAM)
            {
                gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                    "id = %d tag = %s parent = %d line(%d,%d,%d) name = %s, pc(%d,%d)",
                    die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name),
                    die->u.func.pcLine[0], die->u.func.pcLine[1]);
                gcmPRINT(context->tmpLog);
            }
            else if (die->tag == VSC_DI_TAG_LEXICALBLOCK)
            {
                gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                    "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s",
                    die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name));
                gcmPRINT(context->tmpLog);
            }
            else
            {
                gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset, "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s",
                    die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name));
                gcmPRINT(context->tmpLog);
            }
        }
    }
}

gceSTATUS vscDISaveDebugInfo(VSC_DIContext * context, gctPOINTER * buffer, gctUINT32 * bufferSize)
{
    gctUINT8 * ptr;
    VSC_DIContext * ctx;

    if (context == gcvNULL)
    {
        if (bufferSize)
            *bufferSize += gcmSIZEOF(gctUINT8);

        if (buffer && *buffer)
        {
            ptr = (gctUINT8 *)(*buffer);
            *ptr = 0;
            ptr += gcmSIZEOF(gctUINT8);
            *buffer = (gctPOINTER)ptr;
        }

        return gcvSTATUS_OK;
    }

    if (bufferSize)
    {
        *bufferSize += gcmSIZEOF(gctUINT8)
                     + gcmSIZEOF(VSC_DIContext)
                     + context->dieTable.usedCount* gcmSIZEOF(VSC_DIE)
                     + context->strTable.usedSize
                     + context->lineTable.count * gcmSIZEOF(VSC_DI_LINE_TABLE_MAP)
                     + context->swLocTable.usedCount* gcmSIZEOF(VSC_DI_SW_LOC)
                     + context->locTable.usedCount* gcmSIZEOF(VSC_DI_HW_LOC);
    }

    if (buffer && *buffer)
    {
        ptr = (gctUINT8 *)(*buffer);

        *ptr = 1;
        ptr += gcmSIZEOF(gctUINT8);

        *(VSC_DIContext *)ptr = *context;
        ctx = (VSC_DIContext *)ptr;
        ptr += gcmSIZEOF(VSC_DIContext);

        if (context->dieTable.usedCount > 0)
        {
            gcoOS_MemCopy(ptr, (gctPOINTER)context->dieTable.die, context->dieTable.usedCount* gcmSIZEOF(VSC_DIE) );
            ptr += context->dieTable.usedCount * gcmSIZEOF(VSC_DIE);
        }

        ctx->dieTable.count = ctx->dieTable.usedCount = context->dieTable.usedCount;

        if (context->strTable.usedSize > 0)
        {
            gcoOS_MemCopy(ptr, context->strTable.str, context->strTable.usedSize);
            ptr += context->strTable.usedSize;
        }

        ctx->strTable.size = ctx->strTable.usedSize = context->strTable.usedSize;

        if (context->lineTable.count > 0)
        {
            gcoOS_MemCopy(ptr, context->lineTable.map, context->lineTable.count * gcmSIZEOF(VSC_DI_LINE_TABLE_MAP));
            ptr +=context->lineTable.count * gcmSIZEOF(VSC_DI_LINE_TABLE_MAP);
        }

        ctx->lineTable.count = context->lineTable.count;

        if (context->swLocTable.usedCount> 0)
        {
            gcoOS_MemCopy(ptr, context->swLocTable.loc, context->swLocTable.usedCount* gcmSIZEOF(VSC_DI_SW_LOC));
            ptr += context->swLocTable.usedCount* gcmSIZEOF(VSC_DI_SW_LOC);
        }

        ctx->swLocTable.count = ctx->swLocTable.usedCount = context->swLocTable.usedCount;

        if (context->locTable.usedCount> 0)
        {
            gcoOS_MemCopy(ptr, context->locTable.loc, context->locTable.usedCount* gcmSIZEOF(VSC_DI_HW_LOC));
            ptr += context->locTable.usedCount* gcmSIZEOF(VSC_DI_HW_LOC);
        }

        ctx->locTable.count = ctx->locTable.usedCount = context->locTable.usedCount;

        ctx->tmpLog = gcvNULL;

        *buffer = (gctPOINTER)ptr;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
vscDILoadDebugInfo(VSC_DIContext ** context, gctPOINTER* buffer, gctUINT32 * bufferSize)
{
    VSC_DIContext * ctx;
    VSC_DIContext * ptr;
    gctUINT8 * pos;
    gctUINT size;
    PFN_Allocate pfnAllocate;
    PFN_Free pfnFree;

    if (context == gcvNULL || buffer == gcvNULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    ptr = *(VSC_DIContext **)buffer;

    pfnAllocate = gcoOS_Allocate;
    pfnFree = gcoOS_Free;

    if (gcmIS_ERROR(pfnAllocate(gcvNULL,gcmSIZEOF(VSC_DIContext), (gctPOINTER *)&ctx)))
    {
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    *context = ctx;

    *ctx = *ptr;

    pos = (gctUINT8 *)(ptr + 1);
    *bufferSize -= gcmSIZEOF(VSC_DIContext);

    ctx->pfnAllocate = pfnAllocate;
    ctx->pfnFree = pfnFree;

    if (ctx->dieTable.count > 0)
    {
        size = ctx->dieTable.count * gcmSIZEOF(VSC_DIE);
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->dieTable.die)))
        {
            vsdDIPRINT("out of memory when allocate dieTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->dieTable.die, pos,size);

        pos += size;
        *bufferSize -= size;
    }

    if (ctx->strTable.size > 0)
    {
        size = ctx->strTable.size;
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->strTable.str)))
        {
            vsdDIPRINT("out of memory when allocate strTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->strTable.str, pos, size);

        pos += size;
        *bufferSize -= size;
    }

    if (ctx->lineTable.count > 0)
    {
        size = ctx->lineTable.count * gcmSIZEOF(VSC_DI_LINE_TABLE_MAP);
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->lineTable.map)))
        {
            vsdDIPRINT("out of memory when allocate dieTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->lineTable.map, pos, size);

        pos +=size;
        *bufferSize -= size;
    }

    if (ctx->swLocTable.count > 0)
    {
        size = ctx->swLocTable.count * gcmSIZEOF(VSC_DI_SW_LOC);
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->swLocTable.loc)))
        {
            vsdDIPRINT("out of memory when allocate dieTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->swLocTable.loc, pos,size);

        pos += size;
        *bufferSize -= size;
    }

    if (ctx->locTable.count > 0)
    {
        size = ctx->locTable.count * gcmSIZEOF(VSC_DI_HW_LOC);
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->locTable.loc)))
        {
            vsdDIPRINT("out of memory when allocate dieTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->locTable.loc, pos,size);

        pos += size;
        *bufferSize -= size;
    }

    if (gcmIS_ERROR(pfnAllocate(gcvNULL,VSC_DI_TEMP_LOG_SIZE,(gctPOINTER *)&ctx->tmpLog)))
    {
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    *buffer = (gctPOINTER)pos;

    return gcvSTATUS_OK;
}

gceSTATUS
vscDICopyDebugInfo(VSC_DIContext* Context, gctPOINTER* Buffer)
{
    VSC_DIContext * ctx;
    gctUINT size;
    PFN_Allocate pfnAllocate;
    PFN_Free pfnFree;

    if (Context == gcvNULL || Buffer == gcvNULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    pfnAllocate = gcoOS_Allocate;
    pfnFree = gcoOS_Free;

    if (gcmIS_ERROR(pfnAllocate(gcvNULL,gcmSIZEOF(VSC_DIContext), (gctPOINTER *)&ctx)))
    {
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    ctx->pfnAllocate = pfnAllocate;
    ctx->pfnFree = pfnFree;
    gcoOS_MemCopy(ctx, Context, gcmSIZEOF(VSC_DIContext));

    if (Context->dieTable.count > 0)
    {
        size = Context->dieTable.count * gcmSIZEOF(VSC_DIE);
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->dieTable.die)))
        {
            vsdDIPRINT("out of memory when allocate dieTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->dieTable.die, Context->dieTable.die, size);
    }

    if (Context->strTable.size > 0)
    {
        size = Context->strTable.size;
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->strTable.str)))
        {
            vsdDIPRINT("out of memory when allocate strTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->strTable.str, Context->strTable.str, size);
    }

    if (Context->lineTable.count > 0)
    {
        size = Context->lineTable.count * gcmSIZEOF(VSC_DI_LINE_TABLE_MAP);
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->lineTable.map)))
        {
            vsdDIPRINT("out of memory when allocate dieTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->lineTable.map, Context->lineTable.map, size);
    }

    if (Context->swLocTable.count > 0)
    {
        size = Context->swLocTable.count * gcmSIZEOF(VSC_DI_SW_LOC);
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->swLocTable.loc)))
        {
            vsdDIPRINT("out of memory when allocate dieTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->swLocTable.loc, Context->swLocTable.loc, size);
    }

    if (Context->locTable.count > 0)
    {
        size = Context->locTable.count * gcmSIZEOF(VSC_DI_HW_LOC);
        if (gcmIS_ERROR(ctx->pfnAllocate(gcvNULL, size, (gctPOINTER *)&ctx->locTable.loc)))
        {
            vsdDIPRINT("out of memory when allocate dieTable");
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_MemCopy(ctx->locTable.loc, Context->locTable.loc, size);
    }

    if (gcmIS_ERROR(pfnAllocate(gcvNULL,VSC_DI_TEMP_LOG_SIZE,(gctPOINTER *)&ctx->tmpLog)))
    {
        vsdDIPRINT("out of memory when allocate dieTable");
        return gcvSTATUS_OUT_OF_MEMORY;
    }
    gcoOS_MemCopy(ctx->tmpLog, Context->tmpLog, VSC_DI_TEMP_LOG_SIZE);

    *Buffer = ctx;
    return gcvSTATUS_OK;
}

void _vscDIInitCallStack(VSC_DIContext * context)
{
    gctUINT32 i;

    for (i = 0; i < VSC_DI_CALL_DEPTH; i++)
    {
        context->callStack[i].die = gcvNULL;
        context->callStack[i].sourceLoc.lineNo = 0;
        context->callStack[i].nextSourceLoc.lineNo = 0;
        context->callStack[i].nextPC = 0;
    }
}

gceSTATUS vscDIConstructContext(PFN_Allocate allocPfn, PFN_Free freePfn, VSC_DIContext ** context)
{
    VSC_DIContext * ptr;
    PFN_Allocate pfnAllocate;
    PFN_Free pfnFree;

    if (gcmOPT_EnableDebug())
    {
        pfnAllocate = (allocPfn == gcvNULL) ? gcoOS_Allocate : allocPfn;
        pfnFree = (freePfn == gcvNULL) ? gcoOS_Free : freePfn;

        if (gcmIS_ERROR(pfnAllocate(gcvNULL,gcmSIZEOF(VSC_DIContext), (gctPOINTER *)&ptr)))
        {
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        gcoOS_ZeroMemory(ptr, gcmSIZEOF(VSC_DIContext));

        ptr->pfnAllocate = pfnAllocate;
        ptr->pfnFree = pfnFree;

        ptr->cu = vscDIAddDIE(ptr, VSC_DI_TAG_COMPILE_UNIT, VSC_DI_INVALIDE_DIE, "CU_DIE", 0, 0, 0, 0);

        _vscDIInitCallStack(ptr);
        ptr->callDepth = -1;
        ptr->stepState = VSC_STEP_STATE_NONE;

        if (gcmIS_ERROR(pfnAllocate(gcvNULL,VSC_DI_TEMP_LOG_SIZE,(gctPOINTER *) &ptr->tmpLog)))
        {
            return gcvSTATUS_OUT_OF_MEMORY;
        }

        *context = ptr;

        return gcvSTATUS_OK;
    }
    else
    {
        return gcvSTATUS_INVALID_CONFIG;
    }
}

void vscDIDestroyContext(VSC_DIContext * context)
{
    if (context)
    {
        if (context->dieTable.die)
        {
            context->pfnFree(gcvNULL, context->dieTable.die);
        }

        if (context->strTable.str)
        {
            context->pfnFree(gcvNULL, context->strTable.str);
        }

        if (context->lineTable.map)
        {
            context->pfnFree(gcvNULL, context->lineTable.map);
        }

        if (context->locTable.loc)
        {
            /* make stupid compiler happy, will delete soon */
            vscDIGetHWLoc(context, vscDIAddHWLoc(context));
            vscDISetHwLocToSWLoc(context, gcvNULL, gcvNULL);
            context->pfnFree(gcvNULL, context->locTable.loc);
        }

        if (context->swLocTable.loc)
        {
            /* make stupid compiler happy, will delete soon */
            vscDIGetSWLoc(context, vscDIAddSWLoc(context));
            context->pfnFree(gcvNULL, context->swLocTable.loc);
        }

        if (context->tmpLog)
        {
            context->pfnFree(gcvNULL, context->tmpLog);
        }

        context->pfnFree(gcvNULL, context);
    }
}

void vscDIStep(void * ptr, unsigned int pc, unsigned int stepFlag)
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;

    if (context == gcvNULL || (int)pc < 0)
        return;

    context->stepState = (VSC_STEP_STATE)stepFlag;
}

void vscDIPushCallStack(
    void * ptr,
    unsigned int currentPC,
    unsigned int intoPC
    )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;

    if (context == gcvNULL)
        return;

    if (currentPC == 0)
    {
        if (context->callDepth == -1)
        {
            /* first initialize */
            VSC_DIE * intoDie = _getCurrentSubProgramDie(context, intoPC);
            context->callDepth++;
            context->callStack[context->callDepth].die = intoDie;
        }
        return;
    }

    if (context->callDepth + 1 < VSC_DI_CALL_DEPTH)
    {
        VSC_DIE * intoDie = _getCurrentSubProgramDie(context, intoPC);

        context->callStack[context->callDepth].nextPC = currentPC + 1;
        context->callDepth++;
        context->callStack[context->callDepth].die = intoDie;
    }
}

void vscDIPopCallStack(
    void * ptr,
    unsigned int currentPC
    )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;

    if (context == gcvNULL)
        return;

    if (context->callDepth > 0)
    {
        context->callDepth--;
    }
}

int vscDIGetCallStackDepth(
    void * ptr
    )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;

    if (context == gcvNULL)
        return 0;
    return context->callDepth;
}

void vscDIGetStackFrameInfo(
    void * ptr,
    int frameId,
    unsigned int * functionId,
    unsigned int * callerPc,
    char * functionName,
    unsigned int nameLength,
    char * fileName, /* not used currently */
    char * fullName, /* not used currently */
    unsigned int fileNameLength /* not used currently */
    )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;
    VSC_DIE * die;

    if (context == gcvNULL)
        return;

    /* Per communication with IDE team, this is to skip frame0 which is main function */
    frameId++;

    if (frameId > context->callDepth)
        return;

    die = context->callStack[frameId].die;

    if (!die || die->tag != VSC_DI_TAG_SUBPROGRAM)
        return;

    if (functionId)
        *functionId = (unsigned int) die->id;

    if (callerPc)
        *callerPc = context->callStack[frameId].nextPC - 1;

    if (functionName)
        gcoOS_StrCopySafe(functionName, nameLength, _GetNameStr(context, die->name));
}

void vscDIGetFunctionInfo(
    void * ptr,
    int functionId,
    char * functionName,
    unsigned int nameLength,
    unsigned int * lowPC,
    unsigned int * highPC
    )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;
    VSC_DIE * die;

    if (context == gcvNULL)
        return;

    die = VSC_DI_DIE_PTR(functionId);

    if (!die || die->tag != VSC_DI_TAG_SUBPROGRAM)
        return;

    if (functionName)
        gcoOS_StrCopySafe(functionName, nameLength, _GetNameStr(context, die->name));

    if (lowPC)
        *lowPC = die->u.func.pcLine[0];

    if (highPC)
        *highPC = die->u.func.pcLine[1];

#if vsdTEST_API
    gcmPRINT("id: %d, function name: %s, [%d %d]\n", functionId, functionName, *lowPC, *highPC);
#endif
}

int vscDIGetVariableCount(
    void * ptr,
    int parentId,
    gctBOOL bArgument
    )
{
    int ret = 0;
    VSC_DIContext * context = (VSC_DIContext *)ptr;
    VSC_DIE * die;
    VSC_DIE * child;
    gctSIZE_T i;

    if (context == gcvNULL)
        return ret;

    die = VSC_DI_DIE_PTR(parentId);

    if (!die)
        return ret;

    child = VSC_DI_DIE_PTR(die->child);

    if (!child)
        return ret;

    if (bArgument && die->tag == VSC_DI_TAG_SUBPROGRAM)
    {
        while (child)
        {
            if (child->tag == VSC_DI_TAG_PARAMETER)
                ret++;
            child = VSC_DI_DIE_PTR(child->sib);
        }
        return ret;
    }

    if (die->tag == VSC_DI_TAG_SUBPROGRAM)
    {
        for (i = parentId + 1; i < context->dieTable.usedCount; i++)
        {
            VSC_DIE tempDie = context->dieTable.die[i];
            VSC_DIE parent;

            if (tempDie.tag != VSC_DI_TAG_VARIABE)
                continue;

            /* if parent is block, find the function parent */
            parent = context->dieTable.die[tempDie.parent];
            while (parent.tag == VSC_DI_TAG_LEXICALBLOCK)
            {
                if (parent.id == VSC_DI_INVALIDE_DIE)
                    break;
                parent = context->dieTable.die[parent.parent];
            }

            if (parent.id == parentId)
                ret++;
        }
    }

    if (die->tag == VSC_DI_TAG_VARIABE)
    {
        for (i = parentId + 1; i < context->dieTable.usedCount; i++)
        {
            VSC_DIE tempDie = context->dieTable.die[i];
            VSC_DIE parent;

            if (tempDie.tag != VSC_DI_TAG_VARIABE)
                continue;

            parent = context->dieTable.die[tempDie.parent];
            if (parent.id == parentId)
                ret++;
        }
    }

#if vsdTEST_API
    gcmPRINT("id: %d, bArgument: %d, Count: %d\n", parentId, bArgument, ret);
#endif

    return ret;
}

static void _GetTypeStr(
    VSC_DIContext * context,
    VSC_DIE * die,
    char *typeStr,
    unsigned int strLength,
    unsigned int dimDepth)
{
    if (!die)
        return;

    if (!die->u.variable.type.isPrimitiveType)
    {
        gctINT type = die->u.variable.type.type;
        VSC_DIE *typeDie = VSC_DI_DIE_PTR(type);
        gctUINT offset = 0;
        gctSTRING typeName = gcvNULL;
        if (typeDie)
            typeName = _GetNameStr(context, typeDie->name);
        if (typeDie && typeName && typeName[0] != '\0' )
            gcoOS_PrintStrSafe(typeStr, strLength, &offset, "%s", _GetNameStr(context, typeDie->name));
        else
            gcoOS_StrCopySafe(typeStr, strLength, "struct or uninon"); /* Some struct or union may not have name. TODO: can provide more info if requied. */

        if (die->u.variable.type.isPointer)
            gcoOS_StrCatSafe(typeStr, strLength, " *");
        return;
    }

    gcoOS_StrCopySafe(typeStr, strLength, VIR_GetOCLTypeName(die->u.variable.type.type));

    if (die->u.variable.type.isPointer)
        gcoOS_StrCatSafe(typeStr, strLength, " *");

    if (die->u.variable.type.array.numDim > 0)
    {
        gctUINT offset = 0;
        gctINT i = dimDepth;
        for (; i < die->u.variable.type.array.numDim; i++)
        {
            offset = 0;
            gcoOS_PrintStrSafe(typeStr, strLength, &offset, "%s%s[%d]", typeStr, (unsigned int)i == dimDepth ? " " : "", die->u.variable.type.array.length[i]);
        }
    }
}

gctBOOL vscDIGetDieisPrimitiveType(
    VSC_DIE * die
    )
{
    if(die->tag == VSC_DI_TAG_TYPE)
    {
        return die->u.type.isPrimitiveType;
    }
    else if(die->tag == VSC_DI_TAG_PARAMETER || die->tag == VSC_DI_TAG_VARIABE)
    {
        return die->u.variable.type.isPrimitiveType;
    }

    return gcvFALSE;
}

gctUINT16 vscDIGetDieIdByStrInfo(
    void * ptr,
    const char * varIdStr
    )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;
    VSC_DIE * die;
    VSC_DIE * childDie = gcvNULL;
    gctSIZE_T Strlen = 0;
    gctUINT index = 0;
    gctINT curIdx = 0;
    gctINT  varId;
    gctINT  childId ;
    varId = VSC_DI_INVALIDE_DIE;
    childId = VSC_DI_INVALIDE_DIE;
    gcoOS_StrToInt(varIdStr, &varId);
    die = VSC_DI_DIE_PTR(varId);
    gcoOS_StrLen(varIdStr, &Strlen);
    for(; index < Strlen; index++)
    {
        if(varIdStr[index] == '-')
        {
            gcoOS_StrToInt(varIdStr + index + 1, &childId);
            if(die->child != VSC_DI_INVALIDE_DIE)
            {
                childDie = VSC_DI_DIE_PTR(die->child);
            }
            else if(!vscDIGetDieisPrimitiveType(die))
            {
                childDie = VSC_DI_DIE_PTR(die->u.type.type);
                childDie = VSC_DI_DIE_PTR(childDie->child);
            }

            curIdx = 0;
            while (childDie)
            {
                if (curIdx == childId)
                    break;
                curIdx++;
                childDie = VSC_DI_DIE_PTR(childDie->sib);
            }
            die = childDie;
        }
    }
    if(die)
        return die->id;
    else
        return VSC_DI_INVALIDE_DIE;
}

gctUINT vscGetArrayRelativeAddr(
    VSC_DI_ARRAY_DESC arrDesc,
    gctINT length[VSC_DI_MAX_ARRAY_DIM],
    gctINT dimDepth
    )
{
    gctUINT RelativeAddr = 0;
    gctINT index = 0;
    gctINT arrayLength = 1;
    gctINT eachDepthLength[VSC_DI_MAX_ARRAY_DIM];
    for(index = arrDesc.numDim - 1; index >= 0; index--)
    {
        eachDepthLength[index] = arrayLength;
        arrayLength = arrayLength * arrDesc.length[index];
    }
    for(index = 0; index < arrDesc.numDim && index < dimDepth; index++)
    {
        RelativeAddr = RelativeAddr + eachDepthLength[index] * length[index];
    }
    return RelativeAddr;
}

gctUINT vscDIGetDieOffset(
    void * ptr,
    const char * varIdStr
    )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;
    VSC_DIE * die;
    VSC_DIE * childDie = gcvNULL;
    gctUINT index = 0;
    gctINT varId;
    gctINT childId;
    gctINT curIdx;
    gctINT dimDepth = 0;
    gctINT arrNum = 0;
    gctINT usedNum;
    gctSIZE_T Strlen = 0;
    gctUINT offset = 0;
    VSC_DI_ARRAY_DESC arrDesc = {0};
    gctINT arrlength[VSC_DI_MAX_ARRAY_DIM];
    gcoOS_StrToInt(varIdStr, &varId);
    gcoOS_StrLen(varIdStr, &Strlen);
    die = VSC_DI_DIE_PTR((gctUINT)varId);
    if(!die)
        return 0;
    offset = die->alignmenOffset;
    for(; index < Strlen && die != gcvNULL; index++)
    {
        if(varIdStr[index] == '-')
        {
            gcoOS_StrToInt(varIdStr + index + 1, &childId);
            if(die->child != VSC_DI_INVALIDE_DIE)
            {
                childDie = VSC_DI_DIE_PTR(die->child);
            }
            else if(!vscDIGetDieisPrimitiveType(die))
            {
                childDie = VSC_DI_DIE_PTR(die->u.type.type);
                childDie = VSC_DI_DIE_PTR(childDie->child);
            }
            curIdx = 0;
            while (childDie)
            {
                if (curIdx == childId)
                    break;
                curIdx++;
                childDie = VSC_DI_DIE_PTR(childDie->sib);
            }
            if(childDie)
                offset += childDie->alignmenOffset;

            die = childDie;
            dimDepth = 0;
        }
        else if(varIdStr[index] == '.')
        {
            if(die->u.type.array.numDim > 0)
            {
                if(dimDepth == 0)
                {
                    arrDesc = die->u.type.array;
                    arrNum = arrDesc.numDim;
                }
                gcoOS_StrToInt(varIdStr + index + 1, &usedNum);
                if(dimDepth < arrNum)
                    arrlength[dimDepth] = usedNum;
                dimDepth++;
                if(arrNum == dimDepth)
                {
                    size_t size;
                    if(die->u.type.isPrimitiveType)
                        size = VIR_GetTypeAlignment(die->u.type.type);
                    else/*need more judge to judge struct or union*/
                        size = die->size;
                    offset += size * vscGetArrayRelativeAddr(arrDesc, arrlength, arrNum);
                }
                else if(dimDepth > arrNum)/*vector array*/
                {
                    offset += usedNum * VIR_GetTypeSize(VIR_GetTypeComponentType(die->u.type.type));
                }
            }
            else if(die->u.type.array.numDim == 0 || dimDepth > arrNum)/*vector*/
            {
                gcoOS_StrToInt(varIdStr + index + 1, &usedNum);
                offset += usedNum * VIR_GetTypeSize(VIR_GetTypeComponentType(die->u.type.type));
            }
        }
        else if(varIdStr[index] == '+')
        {
            offset = 0;
        }
    }
    return offset;
}

void vscDIGetIdStrInfo(
    const char * varIdStr,
    int * varId,
    gctUINT * PdimDepth,
    gctUINT * pointerDepth,
    gctUINT * structDepth,
    gctUINT * dotIndex,
    gctINT * dimNum
    )
{
    gctUINT index = 0;
    gctSIZE_T Strlen = 0;
    *PdimDepth = 0;
    *pointerDepth = 0;
    *structDepth = 0;
    gcoOS_StrToInt(varIdStr, varId);
    gcoOS_StrLen(varIdStr, &Strlen);
    for(; index < Strlen; index++)
    {
        if(varIdStr[index] == '+')
        {
            (*pointerDepth)++;
        }
        if(varIdStr[index] == '.')
        {
            dotIndex[*PdimDepth] = index;
            gcoOS_StrToInt(varIdStr+index+1, &dimNum[*PdimDepth]);
            (*PdimDepth)++;
        }
        if(varIdStr[index] == '-')
        {
            (*pointerDepth) = 0;
            (*PdimDepth) = 0;
            (*structDepth)++;
        }
    }
}

void vscDIGetArrayTempReg(
     void * ptr,
     VSC_DIE * die,
     int idx,
     gctUINT dimDepth,
     const gctINT * dimNum,
     gctINT * PtempReg
     )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;
    VSC_DI_SW_LOC * sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
    gctINT index = 0;
    gctUINT16 regStart = 0;
    gctINT tempReg = 0;
    gctINT eachDepthLength[4] = {1,1,1,1};
    gctINT arrayLength = 1;
    gctINT elemCount = 4;
    if(sl && dimDepth + 1 >= (gctUINT)die->u.variable.type.array.numDim)
    {
        /* calculate the array width*/
        for(index = (gctUINT)die->u.variable.type.array.numDim - 1; index >= 0; index--)
        {
            eachDepthLength[index] = arrayLength;
            arrayLength = arrayLength * die->u.variable.type.array.length[index];
        }
        while (sl->next < context->swLocTable.usedCount)
        {
            sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
        }

        /*calculate vector element count*/
        if(VSC_DI_DIETYPE_IS_VECTOR(die))
        {
            elemCount = VIR_GetTypeLogicalComponents(die->u.variable.type.type);
        }
        /*find the array start position in sw reg*/
        regStart = sl->u.reg.start;

        /*calculate the relative location*/
        for(index = 0; index < (gctINT)dimDepth; index++)
        {
            tempReg = tempReg + eachDepthLength[index] * dimNum[index];
        }
        /*the absolute sw location*/
        /*if  elementCount < 4, occupy one reg*/
        tempReg = regStart + (tempReg + idx) * ((elemCount / 4) == 0? 1 : (elemCount / 4));
    }
    *PtempReg = tempReg;
}

void vscDIGetVariableInfo(
    void * ptr,
    const char * parentIdStr,
    int idx,
    gctBOOL bArgument,
    char * varName,
    char * typeStr,
    unsigned int nameLength,
    char * varIdStr,
    unsigned int * lowPC,
    unsigned int * highPC,
    unsigned int * hwLocCount,
    unsigned int * childrenCount
    )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;
    VSC_DIE * die;
    VSC_DIE * child;
    VSC_DIE * Vdie;
    int curIdx = 0;
    gctSIZE_T i;
    int parentId;

    /*variables for idStr has '.' or '+'*/
    gctBOOL isArrayChild;
    gctBOOL isVectorChild;
    gctBOOL isPointerChild;
    gctBOOL isVStructChild;
    gctUINT index = 0;
    gctUINT offset = 0;
    gctUINT dimDepth = 0;
    gctUINT pointerDepth = 0;
    gctUINT structDepth = 0;
    gctUINT dotIndex[10];
    gctINT dimNum[10];

    vscDIGetIdStrInfo(parentIdStr,
                      &parentId,
                      &dimDepth,
                      &pointerDepth,
                      &structDepth,
                      dotIndex,
                      dimNum);

    if (context == gcvNULL)
        return;

    die = VSC_DI_DIE_PTR(parentId);
    Vdie = VSC_DI_DIE_PTR(vscDIGetDieIdByStrInfo(ptr, parentIdStr));

    if (!die || (die->tag != VSC_DI_TAG_SUBPROGRAM && die->tag != VSC_DI_TAG_VARIABE && die->tag != VSC_DI_TAG_PARAMETER))
        return;

    isArrayChild = (Vdie->tag == VSC_DI_TAG_TYPE || Vdie->tag == VSC_DI_TAG_VARIABE || Vdie->tag == VSC_DI_TAG_PARAMETER)
                   && Vdie->u.variable.type.array.numDim > 0
                   && (gctINT)dimDepth < Vdie->u.variable.type.array.numDim;

    isVectorChild = VSC_DI_DIETYPE_IS_VECTOR(Vdie);

    isPointerChild = (Vdie->tag == VSC_DI_TAG_VARIABE || Vdie->tag == VSC_DI_TAG_PARAMETER || Vdie->tag == VSC_DI_TAG_TYPE)
                     && Vdie->u.variable.type.isPointer
                     && pointerDepth < 1;/*need to judge (void **)*/

    isVStructChild = /*structDepth > 0 ||*/ /*use Vdie and no need structDepth to judge*/
                     (Vdie->child == VSC_DI_INVALIDE_DIE && !vscDIGetDieisPrimitiveType(Vdie));

    if(!isArrayChild && !isVectorChild && !isPointerChild && !isVStructChild)
    {
        child = VSC_DI_DIE_PTR(die->child);

        if (!child)
            return;

        if (die->tag == VSC_DI_TAG_SUBPROGRAM)
        {
            /* function */
            if (bArgument)
            {
                while (child)
                {
                    if (child->tag == VSC_DI_TAG_PARAMETER)
                    {
                        if (curIdx == idx)
                            break;
                        curIdx++;
                    }
                    child = VSC_DI_DIE_PTR(child->sib);
                }
            }
            else
            {
                child = gcvNULL;
                for (i = parentId + 1; i < context->dieTable.usedCount; i++)
                {
                    VSC_DIE curDie = context->dieTable.die[i];
                    VSC_DIE parent;

                    if (curDie.tag != VSC_DI_TAG_VARIABE)
                        continue;

                    /* if parent is block, find the function parent */
                    parent = context->dieTable.die[curDie.parent];
                    while (parent.tag == VSC_DI_TAG_LEXICALBLOCK)
                    {
                        if (parent.id == VSC_DI_INVALIDE_DIE)
                            break;
                        parent = context->dieTable.die[parent.parent];
                    }

                    if (parent.id == parentId)
                    {
                        if (curIdx == idx)
                        {
                            child = &curDie;
                            break;
                        }
                        curIdx++;
                    }
                }
            }
        }
        else
        {
            /* variable */
            curIdx = 0;
            while (child)
            {
                if (curIdx == idx)
                    break;
                curIdx++;
                child = VSC_DI_DIE_PTR(child->sib);
            }
        }

        if (!child)
            return;

        if (varName)
            gcoOS_StrCopySafe(varName, nameLength, _GetNameStr(context, child->name));

        _GetTypeStr(context, child, typeStr, nameLength,0);

        if (varIdStr)
        {
            offset = 0;
            gcoOS_PrintStrSafe(varIdStr, 50, &offset, "%d", child->id);
        }

        if (hwLocCount)
        {
            VSC_DI_SW_LOC * sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, child->u.variable.swLoc);
            VSC_DI_HW_LOC * hl;
            *hwLocCount = 0;
            while (sl)
            {
                hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, sl->hwLoc);
                while(hl)
                {
                    *hwLocCount = *hwLocCount + 1;
                    hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, hl->next);
                }
                sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
            }
        }

        if (lowPC)
            *lowPC = child->u.variable.pcLine[0];

        if (highPC)
            *highPC = child->u.variable.pcLine[1];

        if (childrenCount)
        {
            VSC_DIE *curChild = VSC_DI_DIE_PTR(child->child);
            *childrenCount = 0;
            while(curChild)
            {
                (*childrenCount)++;
                curChild = VSC_DI_DIE_PTR(curChild->sib);
            }
            /*first judge the array, then the vector*/
            if((child->tag == VSC_DI_TAG_VARIABE || child->tag == VSC_DI_TAG_PARAMETER) && child->u.variable.type.array.numDim > 0)
                *childrenCount = child->u.variable.type.array.length[0];
            else if((child->tag == VSC_DI_TAG_VARIABE || child->tag == VSC_DI_TAG_PARAMETER) && child->u.variable.type.isPointer)
            {
                *childrenCount = 1;
            }
            else if(VSC_DI_DIETYPE_IS_VECTOR(child))
            {
                *childrenCount = VIR_GetTypeLogicalComponents(child->u.variable.type.type);
            }
            else if(child->child == VSC_DI_INVALIDE_DIE && !vscDIGetDieisPrimitiveType(child))/*struct*/
            {
                VSC_DIE *varDie = VSC_DI_DIE_PTR(child->u.type.type);
                curChild = VSC_DI_DIE_PTR(varDie->child);
                *childrenCount = 0;
                while(curChild)
                {
                    (*childrenCount)++;
                    curChild = VSC_DI_DIE_PTR(curChild->sib);
                }
            }
        }
    }
    else if(isArrayChild)
    {

        if (varName)
        {
            gcoOS_StrCopySafe(varName, nameLength, _GetNameStr(context, Vdie->name));
            for (index = 0; index < dimDepth; index++)
            {
                offset = 0;
                gcoOS_PrintStrSafe(varName, nameLength, &offset, "%s[%d]", varName, dimNum[index]);
            }
            offset = 0;
            gcoOS_PrintStrSafe(varName, nameLength, &offset, "%s[%d]", varName, idx);
        }

         _GetTypeStr(context, Vdie, typeStr, nameLength, dimDepth + 1);

        if (varIdStr)
        {
            offset = 0;
            gcoOS_PrintStrSafe(varIdStr, nameLength, &offset, "%s.%d", parentIdStr, idx);
        }

        if (hwLocCount)
        {
            VSC_DI_HW_LOC * hl;
            VSC_DI_SW_LOC * sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
            gctINT tempReg = 0;

            vscDIGetArrayTempReg(ptr,
                                 Vdie,
                                 idx,
                                 dimDepth,
                                 dimNum,
                                 &tempReg);

            if(tempReg > 0 && !die->useMemory)
            {
                /*calculate the hwLocCount*/
                *hwLocCount = 0;
                sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
                while (sl)
                {
                    if (sl->reg)
                    {
                        if (sl->u.reg.start <= tempReg &&
                            sl->u.reg.end >= tempReg)
                        {
                            hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, sl->hwLoc);
                            while(hl)
                            {
                                if(hl->reg)
                                {
                                    if(hl->u.reg.end - hl->u.reg.start == sl->u.reg.end - sl->u.reg.start)
                                        *hwLocCount = *hwLocCount + 1;
                                }
                                else
                                    *hwLocCount = *hwLocCount + 1;
                                hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, hl->next);
                            }
                        }
                    }
                    sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
                }
            }
            else if(die->useMemory)
            {
                if((gctINT)dimDepth +1 < Vdie->u.type.array.numDim)
                    *hwLocCount = 0;
                else
                    *hwLocCount = 1;
            }
            else
            {
                *hwLocCount = 0;
            }
        }

        if (lowPC)
            *lowPC = die->u.variable.pcLine[0];

        if (highPC)
            *highPC = die->u.variable.pcLine[1];

        if (childrenCount)
        {
            if(dimDepth + 1 < (gctUINT)Vdie->u.variable.type.array.numDim)
                *childrenCount = Vdie->u.variable.type.array.length[dimDepth + 1];
            else if(isPointerChild)
            {
                *childrenCount = 1;
            }
            else if(isVectorChild)
            {
                *childrenCount = VIR_GetTypeLogicalComponents(Vdie->u.variable.type.type);
            }
            else
                *childrenCount = 0;
        }

    }
    else if(isPointerChild)
    {
         if (varName)
        {
            gctPOINTER memory;
            char * tempstr;
            gcoOS_AllocateMemory(NULL, nameLength*sizeof(char), &memory);
            tempstr = (char*)memory;

            gcoOS_StrCopySafe(tempstr, nameLength, _GetNameStr(context, Vdie->name));
            varName[0]='*';
            for(index = 0; tempstr[index]!=0; index++)
                varName[index + 1] = tempstr[index];
            varName[index + 1] = 0;
        }

        if(typeStr)
        {
            _GetTypeStr(context, Vdie, typeStr, nameLength,0);
            for(index = 0; typeStr[index]!=0; index++)
            {
                if(index > 0 && typeStr[index] == '*')
                    typeStr[index - 1] = 0;
            }
        }

        if (varIdStr)
        {
            offset = 0;
            gcoOS_PrintStrSafe(varIdStr, nameLength, &offset, "%s+", parentIdStr);
        }

        if (hwLocCount)
        {
            VSC_DI_SW_LOC * sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
            VSC_DI_HW_LOC * hl;
            *hwLocCount = 0;
            while (sl)
            {
                hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, sl->hwLoc);
                while(hl)
                {
                    *hwLocCount = *hwLocCount + 1;
                    hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, hl->next);
                }
                sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
            }
        }

        if (lowPC)
            *lowPC = die->u.variable.pcLine[0];

        if (highPC)
            *highPC = die->u.variable.pcLine[1];

        if (childrenCount)
        {
            if(isVectorChild)
            {
                *childrenCount = VIR_GetTypeLogicalComponents(die->u.variable.type.type);
            }
            else if(Vdie->child == VSC_DI_INVALIDE_DIE && !vscDIGetDieisPrimitiveType(Vdie))/*struct*/
            {
                VSC_DIE *varDie = VSC_DI_DIE_PTR(Vdie->u.type.type);
                VSC_DIE *curChild = VSC_DI_DIE_PTR(varDie->child);
                *childrenCount = 0;
                while(curChild)
                {
                    (*childrenCount)++;
                    curChild = VSC_DI_DIE_PTR(curChild->sib);
                }
            }
            else
            {
                *childrenCount = 0;
            }
        }
    }
    else if(isVectorChild)
    {
        if (varName)
        {
            offset = 0;
            gcoOS_PrintStrSafe(varName, nameLength, &offset, "s%x", idx);
        }

        if(typeStr)
        {
            gcoOS_StrCopySafe(typeStr, nameLength, VIR_GetOCLTypeName(VIR_GetTypeComponentType(Vdie->u.variable.type.type)));
        }

        if (varIdStr)
        {
            if(pointerDepth == 0)
            {
                offset = 0;
                gcoOS_PrintStrSafe(varIdStr, nameLength, &offset, "%s.%d", parentIdStr, idx);
            }
            else/*need change*/
            {
                offset = 0;
                gcoOS_PrintStrSafe(varIdStr, nameLength, &offset, "%s.%d", parentIdStr, idx);
            }
        }

        if (hwLocCount)
        {
            VSC_DI_HW_LOC * hl;
            VSC_DI_SW_LOC * sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
            gctINT tempReg = 0;
            gctINT arrayTempReg = 0;

            if(dimDepth > 0 && die->u.variable.type.array.numDim > 0)
            {
                vscDIGetArrayTempReg(ptr,
                                     die,
                                     dimNum[dimDepth-1],
                                     dimDepth-1,
                                     dimNum,
                                     &arrayTempReg);
                tempReg = arrayTempReg + idx/4;
            }
            else if(pointerDepth > 0)
            {
                if(sl && sl->reg)
                    tempReg = sl->u.reg.start;
            }
            else if(sl)
            {
                while (sl->next < context->swLocTable.usedCount)
                {
                    sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
                }
                if(sl->reg)
                {
                    tempReg = sl->u.reg.start + idx/4;
                }
            }

            if(die->useMemory || die->u.type.isPointer)
            {
                sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
                if(sl && sl->hwLoc != VSC_DI_INVALID_HW_LOC)
                    *hwLocCount = 1;
            }
            else if(pointerDepth > 0 || tempReg > 0)
            {
                /*calculate the hwLocCount*/
                *hwLocCount = 0;
                sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
                while (sl)
                {
                    if (sl->reg)
                    {
                        if (sl->u.reg.start <= tempReg &&
                            sl->u.reg.end >= tempReg)
                        {
                            hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, sl->hwLoc);
                            /* when r(0,0) < 3 ,can't get s1,s2,s3*/
                            if(hl && pointerDepth > 0)
                            {
                                *hwLocCount = 1;
                            }
                            else if(hl && hl->u1.hwShift + idx % 4 < 4)
                            {
                                *hwLocCount = 1;
                                break;
                            }
                        }
                    }
                    sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
                }
            }
            else
            {
                *hwLocCount = 0;
            }

            if(die->useMemory)
            {
                sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
                while (sl)
                {
                    hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, sl->hwLoc);
                    if(hl)
                    {
                        *hwLocCount = 1;
                    }
                    sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
                }
            }
        }

        if (lowPC)
            *lowPC = die->u.variable.pcLine[0];

        if (highPC)
            *highPC = die->u.variable.pcLine[1];

        if (childrenCount)
        {
            *childrenCount = 0;
        }
    }
    else if(isVStructChild)
    {
        if(Vdie->child == VSC_DI_INVALIDE_DIE && !vscDIGetDieisPrimitiveType(Vdie))
        {
            Vdie = VSC_DI_DIE_PTR(Vdie->u.type.type);
        }
        child = VSC_DI_DIE_PTR(Vdie->child);

        if (!child)
            return;

        /* variable */
        curIdx = 0;
        while (child)
        {
            if (curIdx == idx)
                break;
            curIdx++;
            child = VSC_DI_DIE_PTR(child->sib);
        }

        if (!child)
            return;

        if (varName)
            gcoOS_StrCopySafe(varName, nameLength, _GetNameStr(context, child->name));

        _GetTypeStr(context, child, typeStr, nameLength,0);

        if (varIdStr)
        {
            offset = 0;
            gcoOS_PrintStrSafe(varIdStr, nameLength, &offset, "%s-%d", parentIdStr, idx);
        }

        if (hwLocCount)
        {
            VSC_DI_SW_LOC * sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
            VSC_DI_HW_LOC * hl;
            *hwLocCount = 0;
            while (sl)
            {
                hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, sl->hwLoc);
                while(hl)
                {
                    *hwLocCount = *hwLocCount + 1;
                    hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, hl->next);
                }
                sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
            }
        }

        if (lowPC)
            *lowPC = die->u.variable.pcLine[0];

        if (highPC)
            *highPC = die->u.variable.pcLine[1];

        if (childrenCount)
        {
            VSC_DIE *curChild = VSC_DI_DIE_PTR(child->child);
            *childrenCount = 0;
            while(curChild)
            {
                (*childrenCount)++;
                curChild = VSC_DI_DIE_PTR(curChild->sib);
            }
            /*first judge the array, then the vector, need different judge in struct type*/
            if((child->tag == VSC_DI_TAG_TYPE || child->tag == VSC_DI_TAG_VARIABE || child->tag == VSC_DI_TAG_PARAMETER) && child->u.variable.type.array.numDim > 0)
                *childrenCount = child->u.variable.type.array.length[0];
            else if((child->tag == VSC_DI_TAG_TYPE || child->tag == VSC_DI_TAG_VARIABE || child->tag == VSC_DI_TAG_PARAMETER) && child->u.variable.type.isPointer)
            {
                *childrenCount = 1;
            }
            else if(VSC_DI_DIETYPE_IS_VECTOR(child))
            {
                *childrenCount = VIR_GetTypeLogicalComponents(child->u.variable.type.type);
            }
            else if(!vscDIGetDieisPrimitiveType(child))/*struct*/
            {
                VSC_DIE *varDie = VSC_DI_DIE_PTR(child->u.type.type);
                curChild = VSC_DI_DIE_PTR(varDie->child);
                *childrenCount = 0;
                while(curChild)
                {
                    (*childrenCount)++;
                    curChild = VSC_DI_DIE_PTR(curChild->sib);
                }
            }
        }
    }

#if vsdTEST_API
    gcmPRINT("id: %s, bArgument: %d, idx: %d, varName: %s, typeStr: %s, varId: %s, hwLocCount: %d, pc(%d %d), childrenCount: %d\n",
        parentIdStr, bArgument, idx, varName, typeStr, varIdStr, *hwLocCount, *lowPC, *highPC, *childrenCount);
    if(*hwLocCount != 0)
    {
        int j = 0;
        gctBOOL bIsReg;
        gctBOOL bIsConst;
        unsigned int lowPC;
        unsigned int highPC;
        unsigned int data0; /* regStart or baseAdd*/
        unsigned int data1; /* regEnd or offset */
        unsigned int data2;  /*swizzle, HwShift or endOffset */
        unsigned int data3;
        for (j = 0; j < (int)*hwLocCount; j++)
        {
            vscDIGetVariableHWLoc(ptr, varIdStr, j, &bIsReg, &bIsConst, &lowPC, &highPC, &data0, &data1, &data2, &data3);
        }
    }

#endif
}

void vscDIGetVariableHWLoc(
    void * ptr,
    const char * varIdStr,
    int idx,
    gctBOOL * bIsReg,
    gctBOOL * bIsConst,
    unsigned int * lowPC,
    unsigned int * highPC,
    unsigned int * data0, /* regStart or baseAdd*/
    unsigned int * data1, /* regEnd or offset */
    unsigned int * data2, /*swizzle, HwShift or endOffset */
    unsigned int * data3  /*indicate chanel when store in memory*/
    )
{
    VSC_DIContext * context = (VSC_DIContext *)ptr;
    VSC_DIE * die;
    VSC_DIE * Vdie;
    int varId = 0;
    int i = 0;
    VSC_DI_SW_LOC * sl = gcvNULL;
    VSC_DI_HW_LOC * hl = gcvNULL;
    gctBOOL found = gcvFALSE;

    gctBOOL isVector = gcvFALSE;
    gctBOOL isPointer = gcvFALSE;
    gctBOOL storeInMemory = gcvFALSE;
    gctUINT dimDepth = 0;
    gctUINT pointerDepth = 0;
    gctUINT VstructDepth = 0;
    gctUINT dotIndex[10];
    gctINT dimNum[10];
    gctINT tempReg = 0;
    gctINT regSOffset = 0;
    gctINT regEOffset = 0;
    vscDIGetIdStrInfo(varIdStr,
                      &varId,
                      &dimDepth,
                      &pointerDepth,
                      &VstructDepth,
                      dotIndex,
                      dimNum);

    if (context == gcvNULL)
        return;

    die = VSC_DI_DIE_PTR(varId);
    Vdie = VSC_DI_DIE_PTR(vscDIGetDieIdByStrInfo(ptr, varIdStr));

    if (!die || (die->tag != VSC_DI_TAG_VARIABE && die->tag != VSC_DI_TAG_PARAMETER))
        return;
    isPointer = (die->tag == VSC_DI_TAG_VARIABE || die->tag == VSC_DI_TAG_PARAMETER) && die->u.variable.type.isPointer && pointerDepth > 0;

    storeInMemory = VstructDepth > 0 ||
                    (die->useMemory && (die->tag == VSC_DI_TAG_VARIABE || die->tag == VSC_DI_TAG_PARAMETER));

    if(dimDepth > 0)
    {
        isVector = VSC_DI_DIETYPE_IS_VECTOR(Vdie) ;

        if(isVector)
        {
            gctINT arrayTempReg = 0;
            sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);

            if(die->u.variable.type.array.numDim > 0)
            {
                vscDIGetArrayTempReg(ptr,
                                     die,
                                     idx,
                                     dimDepth-1,
                                     dimNum,
                                     &arrayTempReg);
                tempReg = arrayTempReg + dimNum[dimDepth-1]/4;
            }
            else if (pointerDepth > 0)
            {
                if(sl && sl->reg)
                    tempReg = sl->u.reg.start;
            }
            else
            {
                 while (sl->next < context->swLocTable.usedCount)
                {
                    sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
                }
                if(sl->reg)
                {
                    tempReg = sl->u.reg.start  + dimNum[dimDepth-1]/4;
                }
            }
        }
        else
        {
            vscDIGetArrayTempReg(ptr,
                                 die,
                                 idx,
                                 dimDepth,
                                 dimNum,
                                 &tempReg);
        }

        if(pointerDepth == 0 && tempReg == 0)
            return;

        sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
        while (sl)
        {
            if (sl->reg)
            {
                if (sl->u.reg.start <= tempReg &&
                    sl->u.reg.end >= tempReg)
                {
                    regSOffset = tempReg - sl->u.reg.start;
                    regEOffset = sl->u.reg.end - tempReg;
                    break;
                }
                if(die->useMemory || isPointer)
                    break;
            }
            sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
        }
    }
    /*else if(isPointer)
    {

    }*/
    else/*normal case*/
    {
        sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, die->u.variable.swLoc);
    }

    while (sl)
    {
        hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, sl->hwLoc);
        while(hl)
        {
            if (i == idx)
            {
                found = gcvTRUE;
                break;
            }
            hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, hl->next);
            i++;
        }
        if (found)
            break;
        sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
    }

    if (!found || !hl)
        return;
    if(isPointer)
    {
        if (bIsReg)
            *bIsReg = 0;

        if (bIsConst)
            *bIsConst = (hl->u.reg.type == VSC_DIE_HW_REG_CONST);

        if (lowPC)
            *lowPC = hl->beginPC;

        if (highPC)
            *highPC = hl->endPC;

        if (data0)
        {
            if (hl->reg)
                *data0 = hl->u.reg.start + regSOffset;
            else
                *data0 = hl->u.offset.baseAddr.start + regSOffset;
        }

        if (data1)
        {
            if(dimDepth == 0)
                *data1 = 0;
            else
            {
                *data1 = vscDIGetDieOffset(ptr, varIdStr);
            }
        }
        if (data2)
        {
            if(dimDepth == 0 && !vscDIGetDieisPrimitiveType(Vdie))
                *data2 = *data1 + Vdie->size;
            else if(dimDepth == 0 || (gctINT)dimDepth == Vdie->u.type.array.numDim)/*only vector and array need find the type*/
                *data2 = *data1 + VIR_GetTypeRows(Vdie->u.variable.type.type) * VIR_GetTypeAlignment(Vdie->u.variable.type.type);
            else
                *data2 = *data1 + VIR_GetTypeSize(VIR_GetTypeComponentType(Vdie->u.variable.type.type));
        }
        if (data3)
        {
            if (hl->u.reg.type == VSC_DIE_HW_REG_CONST)
                *data3 = hl->u1.swizzle;
            else
                *data3 = hl->u1.hwShift;
        }
    }
    else if(storeInMemory)
    {
        if (bIsReg)
            *bIsReg = 0;

        if (bIsConst)
            *bIsConst = (hl->u.reg.type == VSC_DIE_HW_REG_CONST);

        if (lowPC)
            *lowPC = hl->beginPC;

        if (highPC)
            *highPC = hl->endPC;

        if (data0)
        {
            *data0 = hl->u.reg.start;
        }

        if (data1)
        {
            if(VstructDepth > 0)
            {
                *data1 = vscDIGetDieOffset(ptr, varIdStr);
            }
            else if(dimDepth == 0)
            {
                *data1 = Vdie->alignmenOffset;
            }
            else
            {
                /*vector and array*/
                *data1 = vscDIGetDieOffset(ptr, varIdStr);
            }
        }
        if (data2)
        {
            if(dimDepth == 0 || !vscDIGetDieisPrimitiveType(Vdie))
                *data2 = *data1 + Vdie->size;
            else if((gctINT)dimDepth == Vdie->u.type.array.numDim)/*only vector and array need find the type*/
                *data2 = *data1 + VIR_GetTypeRows(Vdie->u.variable.type.type) * VIR_GetTypeAlignment(Vdie->u.variable.type.type);
            else
                *data2 = *data1 + VIR_GetTypeSize(VIR_GetTypeComponentType(Vdie->u.variable.type.type));
        }
        if (data3)
        {
            if (hl->u.reg.type == VSC_DIE_HW_REG_CONST)
                *data3 = hl->u1.swizzle;
            else
                *data3 = hl->u1.hwShift;
        }
    }
    else
    {
        if (bIsReg)
            *bIsReg = hl->reg;

        if (bIsConst)
            *bIsConst = (hl->u.reg.type == VSC_DIE_HW_REG_CONST);

        if (lowPC)
            *lowPC = hl->beginPC;

        if (highPC)
            *highPC = hl->endPC;

        if (data0)
        {
            if (hl->reg)
                *data0 = hl->u.reg.start + regSOffset;
            else
                *data0 = hl->u.offset.baseAddr.start + regSOffset;
        }

        if (data1)
        {
            if (hl->reg)
                *data1 = hl->u.reg.end - regEOffset;
            else
                *data1 = hl->u.offset.offset - regEOffset;
        }

        if (data2)
        {
            if (hl->reg)
            {
                if (hl->u.reg.type == VSC_DIE_HW_REG_CONST)
                    *data2 = hl->u1.swizzle;
                else
                    *data2 = hl->u1.hwShift;

                if(isVector)
                {
                    *data2 += dimNum[dimDepth-1]%4;
                }
            }
            else
            {
                *data2 = hl->u.offset.endOffset;
            }
        }
        if (data3)
        {
            *data3 = 0;
        }

        if(!(*bIsReg) && isVector && *data2 - *data1 > 4)
        {
            int componentMemory = 0;
            int componentNum = 0;
            componentMemory = VIR_GetTypeSize(VIR_GetTypeComponentType(die->u.variable.type.type));
            componentNum = (*data2 - *data1) / componentMemory;
            *data1 = *data1 + (dimNum[dimDepth-1] % componentNum) * componentMemory;
            *data2 = *data1 + componentMemory;
        }
    }


#if vsdTEST_API
    gcmPRINT("id: %s, idx: %d, pc(%d %d), bIsReg: %d, bIsConst: %d, loc info: %d, %d, %d, %d\n",
        varIdStr, idx, *lowPC, *highPC, *bIsReg, *bIsConst, *data0, *data1, *data2, *data3);
#endif
}

int vscDIGetSrcLineByPC(void * ptr, unsigned int pc, unsigned int * line)
{
    gctUINT i;
    gctUINT pcStart;
    gctUINT pcEnd;
    VSC_DIContext * context = (VSC_DIContext *) ptr;

    if ((context->stepState == VSC_STEP_STATE_OUT) &&
        (context->callDepth > 0))
    {
        pc = context->callStack[context->callDepth - 1].nextPC;
    }

    for (i = 0 ; i < context->lineTable.count; i++)
    {
        pcStart = VSC_DI_MC_RANGE_START(context->lineTable.map[i].mcRange);
        pcEnd = VSC_DI_MC_RANGE_END(context->lineTable.map[i].mcRange);

        if (pcStart <= pc && pcEnd >= pc)
        {
            *line = context->lineTable.map[i].sourcLoc.lineNo;
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

/* very slow loop, will refine */
void vscDIGetPCBySrcLine(void * ptr, unsigned int src, unsigned int refPC, unsigned int * start, unsigned int * end)
{

    gctUINT i;
    VSC_DIContext * context = (VSC_DIContext *) ptr;
    gctUINT j = VSC_DI_INVALID_PC;
    gctBOOL srcNoBreak = gcvTRUE; /* the source line must be continuouse */

    *start = VSC_DI_INVALID_PC;
    for (i = 0 ; i < context->lineTable.count; i++)
    {
        if (/*(context->lineTable.map[i].sourcLoc.fileId == src.fileId) && */
            (context->lineTable.map[i].sourcLoc.lineNo == src))
        {
            if (*start == VSC_DI_INVALID_PC)
            {
                *start = VSC_DI_MC_RANGE_START(context->lineTable.map[i].mcRange);
            }

            j = i;
        }

        if (srcNoBreak &&
            (j != VSC_DI_INVALID_PC) &&
            (j != i))
        {
            /* The same source line may occurs in many places, this condition is used to extract the current one line */
            break;
        }
    }

    if (j != VSC_DI_INVALID_PC)
    {
        *end = VSC_DI_MC_RANGE_END(context->lineTable.map[j].mcRange);
    }
    else
    {
        *end = VSC_DI_INVALID_PC;
    }
}

void vscDIGetNearPCBySrcLine(void * ptr, unsigned int src,unsigned int * newSrc, unsigned int * start, unsigned int * end)
{
    gctUINT i;
    VSC_DIContext * context = (VSC_DIContext *) ptr;
    gctUINT j = VSC_DI_INVALID_PC;
    gctUINT chosenLine = src;
    gctUINT s = 0;

    *start = VSC_DI_INVALID_PC;
    do
    {
        chosenLine = src + s++;
        for (i = 0 ; i < context->lineTable.count; i++)
        {
            if (/*(context->lineTable.map[i].sourcLoc.fileId == src.fileId) && */
                (context->lineTable.map[i].sourcLoc.lineNo == chosenLine))
            {
                if (*start == VSC_DI_INVALID_PC)
                {
                    *start = i;
                }

                j = i;
            }
        }
    }
    while (*start == VSC_DI_INVALIDE_DIE);

    *newSrc = chosenLine;
    *end = j;
}

unsigned int vscDIGetPCByFuncName(void * ptr, char * name)
{
    gctUINT i;
    gctCHAR * str;
    VSC_DIContext * context = (VSC_DIContext *) ptr;

    for (i = 0; i < context->dieTable.usedCount; i++)
    {
        if (context->dieTable.die[i].tag == VSC_DI_TAG_SUBPROGRAM)
        {
            str = _GetNameStr(context,context->dieTable.die[i].name);

            if (gcmIS_SUCCESS(gcoOS_StrCmp(str, name)))
            {
                return (gctUINT)context->dieTable.die[i].u.func.pcLine[0];
            }
        }
    }

    return (gctUINT)VSC_DI_INVALID_PC;
}

VSC_DIE * _lookDieInScope(VSC_DIContext * context, VSC_DIE * scope, char * name)
{
    VSC_DIE * die;
    gctCHAR * str;

    die = VSC_DI_DIE_PTR(scope->child);

    while (die)
    {
        if(die->tag == VSC_DI_TAG_VARIABE ||
            die->tag == VSC_DI_TAG_PARAMETER)
        {
            str = _GetNameStr(context,die->name);

            if (gcmIS_SUCCESS(gcoOS_StrCmp(str, name)))
            {
                return die;
            }
        }

        if (die->child != VSC_DI_INVALIDE_DIE)
        {
            VSC_DIE * orgDie = die;
            die = _lookDieInScope(context, die, name);
            if (die)
            {
                return die;
            }

            die = orgDie;
        }

        die = VSC_DI_DIE_PTR(die->sib);
    }

    return gcvNULL;
}

gctBOOL _vscDIGetVariableLocByPC(VSC_DIContext * context, gctUINT pc, VSC_DIE * die, VSC_DI_EXTERN_LOC * loc, gctUINT * locCount)
{
    VSC_DI_HW_LOC * hwLoc;
    VSC_DI_SW_LOC * swLoc;
    gctUINT j = 0;

    swLoc = vscDIGetSWLoc(context,die->u.variable.swLoc);

    if (swLoc == gcvNULL)
        return gcvFALSE;

    while (swLoc)
    {
        hwLoc = VSC_DI_HW_LOC_PTR(swLoc->hwLoc);

        while (hwLoc)
        {
            if (loc && hwLoc->beginPC <= pc && hwLoc->endPC >= pc)
            {
                gctUINT16 mask;
                gctUINT i = 0;

                loc[j].reg = (unsigned int)hwLoc->reg;
                loc[j].u.reg = hwLoc->u.reg;
                loc[j].u.offset = hwLoc->u.offset;

                /* Get mask from SWLoc, this is just tmp code, slLoc mask not same to HW loc, and we can't do this */
                if (swLoc->reg)
                {
                    if (loc[j].reg)
                    {
                        loc[j].u.reg.mask = swLoc->u.reg.mask;
                    }
                    else
                    {
                        mask = swLoc->u.reg.mask;

                        while (mask)
                        {
                            i++;
                            loc[j].u.offset.endOffset = (unsigned short)(loc[j].u.offset.offset + 4 * i);
                            mask = mask >> 1;
                        }
                    }
                }

                break;
            }

            hwLoc = VSC_DI_HW_LOC_PTR(hwLoc->next);
        }

        swLoc = vscDIGetSWLoc(context, swLoc->next);
        j++;
    }

    if (locCount)
    {
        *locCount = j;
    }

    return gcvTRUE;
}

gctBOOL _vscDIGetStructVariableLocByPC(VSC_DIContext * context, gctUINT pc, VSC_DIE * die, VSC_DI_EXTERN_LOC * loc, gctUINT * locCount)
{
    VSC_DI_EXTERN_LOC * hwLoc;
    gctUINT i, j = 0;
    VSC_DIE * childDie = gcvNULL;

    if (die->child == VSC_DI_INVALIDE_DIE)
        return gcvFALSE;

    childDie = vscDIGetDIE(context, die->child);

    while (childDie)
    {
        gcmASSERT(childDie->u.variable.type.isPrimitiveType);

        _vscDIGetVariableLocByPC(context, pc, childDie, gcvNULL, &i);

        j += i;

        childDie = VSC_DI_DIE_PTR(childDie->sib);
    }

    if (j <= 0)
    {
        return gcvFALSE;
    }

    if (loc)
    {
        if (gcoOS_Allocate(gcvNULL, gcmSIZEOF(VSC_DI_EXTERN_LOC) * j, (gctPOINTER *)&hwLoc) == gcvSTATUS_OK)
        {
            j = 0;
            childDie = vscDIGetDIE(context, die->child);
        }

        while (childDie)
        {
            _vscDIGetVariableLocByPC(context, pc, childDie, (VSC_DI_EXTERN_LOC *)(&hwLoc[j]), &i);

            j += i;

            childDie = VSC_DI_DIE_PTR(childDie->sib);
        }

        gcoOS_MemCopy(loc, hwLoc, gcmSIZEOF(VSC_DI_EXTERN_LOC) * j);

        if (hwLoc)
        {
            gcoOS_Free(gcvNULL, hwLoc);
        }
    }

    if (locCount)
    {
        *locCount = j;
    }

    return gcvTRUE;
}

unsigned int
vscDIGetVaribleLocByNameAndPC(void * ptr, unsigned int pc, char * name, void * loc, unsigned int * locCount)
{
    /*VIR_SourceFileLoc line;*/
    gctUINT line;
    VSC_DIE * innerDie;
    VSC_DIE * die;
    VSC_DIContext * context = (VSC_DIContext *) ptr;
    VSC_DI_EXTERN_LOC * location = (VSC_DI_EXTERN_LOC *)loc;

    /* base on pc to located name space */
    if(vscDIGetSrcLineByPC(context, pc, &line))
    {
        /* look up name space for name */
        die = &context->dieTable.die[context->cu];
        innerDie = VSC_DI_DIE_PTR(context->cu);

        /* Goto the inner scope */
        die =  VSC_DI_DIE_PTR(die->child);

        while (die)
        {

            if (die->tag == VSC_DI_TAG_SUBPROGRAM )
            {
                if (die->lineNo <= line &&
                    die->endLineNo >= line)
                {
                    innerDie = die;
                    die =  VSC_DI_DIE_PTR(die->child);
                }
                else
                {
                    die = VSC_DI_DIE_PTR(die->sib);
                }
            }
            else if (die->tag == VSC_DI_TAG_LEXICALBLOCK)
            {
                if (die->lineNo <= line &&
                    die->endLineNo >= line)
                {
                    innerDie = die;
                    die =  VSC_DI_DIE_PTR(die->child);
                }
                else
                {
                    die = VSC_DI_DIE_PTR(die->sib);
                }
            }
            else
            {
                die = VSC_DI_DIE_PTR(die->sib);
            }
        }

        /* look up name from inner to outer */
        die = gcvNULL;
        while(innerDie)
        {
            die = _lookDieInScope(context, innerDie, name);
            if (die == gcvNULL)
            {
                innerDie = VSC_DI_DIE_PTR(innerDie->parent);
            }
            else
            {
                break;
            }
        }

        /* look up the variable location by PC */
        if (die)
        {
            if (die->u.variable.type.isPrimitiveType &&
                _vscDIGetVariableLocByPC(context, pc, die, location, locCount))
            {
                return gcvTRUE;
            }
            else if (_vscDIGetStructVariableLocByPC(context, pc, die, location, locCount))
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

void
vscDISetAlignment(VSC_DIContext * context, gctUINT16 DieId, gctUINT alignmentSize, gctUINT alignmenOffset, gctUINT size, VIR_TypeId typeId)
{
    VSC_DIE * die;
    VSC_DIE * die2;
    die = vscDIGetDIE(context, DieId);
    if(die && (die->tag == VSC_DI_TAG_VARIABE || die->tag == VSC_DI_TAG_PARAMETER || die->tag == VSC_DI_TAG_TYPE))
    {
        die->alignmentSize=alignmentSize;
        die->alignmenOffset=alignmenOffset;
        die->size=size;
        if(typeId > VIR_TYPE_UNKNOWN && typeId < VIR_TYPE_LAST_PRIMITIVETYPE)
            die->u.type.type = typeId;
        if(die->tag != VSC_DI_TAG_TYPE && !die->u.variable.type.isPrimitiveType)
        {
            die2 = vscDIGetDIE(context, (gctUINT16)(die->u.variable.type.type));
            die2->alignmentSize=alignmentSize;
            die2->alignmenOffset=alignmenOffset;
            die2->size=size;
            if(typeId > VIR_TYPE_UNKNOWN && typeId < VIR_TYPE_LAST_PRIMITIVETYPE)
                die2->u.type.type = typeId;
        }
    }
}

void
vscDISetUseMemory(
    VSC_DIContext * context,
    gctUINT16 DieId
    )
{
    VSC_DIE * die;
    die = vscDIGetDIE(context, DieId);
    die->useMemory = gcvTRUE;
}


