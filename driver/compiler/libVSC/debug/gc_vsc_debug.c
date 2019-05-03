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

#define VSC_DI_DIE_PTR(id) (((id) == VSC_DI_INVALIDE_DIE) ? gcvNULL :(&context->dieTable.die[(id)]))
#define VSC_DI_HW_LOC_PTR(id) (((id) == VSC_DI_INVALID_HW_LOC ? gcvNULL : (&context->locTable.loc[id])))
#define VSC_DI_SW_LOC_PTR(id) (((id) == VSC_DI_INVALID_SW_LOC ? gcvNULL : (&context->swLocTable.loc[id])))

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
    }

    return ret;
}

gctSTRING _GetNameStr(VSC_DIContext * context, gctUINT id)
{
    if (id >= context->strTable.size)
        return gcvNULL;

    return (gctSTRING)(&context->strTable.str[id]);
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
        die->u.variable.type.primitiveType = gcvTRUE;

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
    VSC_DI_SW_LOC * sl = gcvNULL;
    VSC_DI_HW_LOC * hl = gcvNULL;
    gctUINT     i;
    gctUINT16   id;
    gctBOOL     found = gcvFALSE;

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
                        continue;
                    }

                    if (sl->u.reg.start == swLoc->u.reg.start &&
                        sl->u.reg.end == swLoc->u.reg.end)
                    {
                        found = gcvTRUE;
                    }
                    else
                    {
                        /* we need split the sw loc here */
                        /* the split is not finished yet, we create a sw loc and add to next */
                        if (sl->u.reg.end != swLoc->u.reg.end)
                        {
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
                        }

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
                            gcmPRINT("| reg[%d,%d]    -> [pc%d, pc%d] 0x%x[0x%x,0x%x] |",
                                swLoc->u.reg.start, swLoc->u.reg.end,
                                hl->beginPC, hl->endPC,
                                hl->u.offset.baseAddr, hl->u.offset.offset, hl->u.offset.endOffset);
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
                                gcmPRINT("| 0x%x[0x%x,0x%x]    -> [pc%d, pc%d] 0x%x[0x%x,0x%x] |",
                                    swLoc->u.offset.baseAddr, swLoc->u.offset.offset, swLoc->u.offset.endOffset,
                                    hl->beginPC, hl->endPC,
                                    hl->u.offset.baseAddr, hl->u.offset.offset, hl->u.offset.endOffset);
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
                   "id = %d tag = %s parent = %d line (%d,%d,%d) name = \"%s\", type(%d, %d)\n",
                   die->id, _GetTagNameStr(context, die->tag),die->parent, die->fileNo, die->lineNo, die->colNo, _GetNameStr(context,die->name),
                   die->u.variable.type.primitiveType, die->u.variable.type.type);

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
                                                   "|[pc%d, pc%d]  |  c(%d,%d)  |",
                                                   hl->beginPC, hl->endPC, hl->u.reg.start, hl->u.reg.end);
                                }
                                else
                                {
                                    gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                                                   "|[pc%d, pc%d]  |  r(%d,%d)  |",
                                                   hl->beginPC, hl->endPC, hl->u.reg.start, hl->u.reg.end);
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
            }
            else if (die->tag == VSC_DI_TAG_TYPE)
            {
                if (!die->u.type.primitiveType)
                {
                    gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                        "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, defined type id = %d",
                        die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo,
                        die->colNo, _GetNameStr(context,die->name),
                        die->u.type.type);
                }
                else
                {
                    if (die->u.type.array.numDim > 0)
                    {
                        gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                            "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, VIR primitive type ID = %d, %d {%d, %d, %d, %d}",
                            die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo, die->lineNo, die->colNo, _GetNameStr(context,die->name),
                            die->u.type.primitiveType, die->u.type.array.numDim, die->u.type.array.length[0],die->u.type.array.length[1],
                            die->u.type.array.length[2],die->u.type.array.length[3]);
                    }
                    else
                    {
                        gcoOS_PrintStrSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, &offset,
                            "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, VIR primitive type ID = %d",
                            die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name),
                            die->u.type.primitiveType);
                    }
                }
                gcmPRINT(context->tmpLog);
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

#if (!VSC_LITE_BUILD)
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
#endif

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
            context->lineTable.map[i].sourcLoc.lineNo == src)
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
                context->lineTable.map[i].sourcLoc.lineNo == chosenLine)
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
        gcmASSERT(childDie->u.variable.type.primitiveType);

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
            if (die->u.variable.type.primitiveType &&
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


