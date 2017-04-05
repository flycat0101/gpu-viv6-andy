/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
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
        context->dieTable.die = _ReallocateBuffer(context,(gctPOINTER)context->dieTable.die,
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
            die->colNo = (gctUINT8)colNo;
            die->name = nameID;
        }
        return die->id;
    }
    else
    {
        return VSC_DI_INVALIDE_DIE;
    }
}

void _DIDumpDIETree(VSC_DIContext * context, gctUINT16 id, gctUINT shift)
{
    VSC_DIE * die;
    gctUINT16 child;
    gctUINT16 sibling;

    if (id != VSC_DI_INVALIDE_DIE)
    {
        vscDIDumpDIE(context, id,shift);

        die = VSC_DI_DIE_PTR(id);
        child = die->child;

        shift++;

        _DIDumpDIETree(context, child, shift);

        shift--;

        sibling = die->sib;

        _DIDumpDIETree(context, sibling, shift);
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

void vscDISetHwLocToSWLoc(VSC_DIContext * context, gctUINT16 swLoc, gctUINT16 hwLoc)
{
    VSC_DI_SW_LOC * sl;
    VSC_DI_HW_LOC * hl;
    gctUINT16 id;

    sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, swLoc);

    if (sl)
    {
        id = sl->hwLoc;

        if (id == VSC_DI_INVALID_HW_LOC)
        {
            sl->hwLoc = hwLoc;
        }
        else
        {
            hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, id);
            id = hl->next;

            while (id != VSC_DI_INVALID_HW_LOC)
            {
                hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, id);
                id = hl->next;
            }

            hl->next = hwLoc;
        }
    }
}

void _vscDIDumpVariableLoc(VSC_DIContext * context, gctUINT16 loc, gctBOOL * flagTable)
{
    VSC_DI_SW_LOC * sl;
    VSC_DI_HW_LOC * hl;

    sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, loc);

    if (sl && flagTable[sl->id])
    {
        return;
    }

    gcmPRINT("variable");

    while (sl)
    {
        if (flagTable)
        {
            flagTable[sl->id] = gcvTRUE;
        }

        if (sl->reg)
        {
             gcmPRINT("SWLoc %d logic reg(%d,%d)", sl->id, sl->u.reg.start, sl->u.reg.end);
        }
        else
        {
            gcmPRINT("SWLoc %d 0x%x[0x%x,0x%x]",sl->id, sl->u.offset.baseAddr, sl->u.offset.offset, sl->u.offset.endOffset);
        }

        hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, sl->hwLoc);

        while(hl)
        {
            if (hl->reg)
            {
                gcmPRINT("        [pc%d, pc%d] hw r(%d,%d)", hl->beginPC, hl->endPC, hl->u.reg.start, hl->u.reg.end);
            }
            else
            {
                gcmPRINT("        [pc%d, pc%d] hw 0x%x(0x%x,0x%x)", hl->beginPC, hl->endPC, hl->u.offset.baseAddr, hl->u.offset.offset, hl->u.offset.endOffset);
            }

            hl = (VSC_DI_HW_LOC *) vscDIGetHWLoc(context, hl->next);
        }
        sl = (VSC_DI_SW_LOC *)vscDIGetSWLoc(context, sl->next);
    }

    gcmPRINT("\n");
}

void vscDIDumpLocTable(VSC_DIContext * context)
{
    gctUINT i;
    gctBOOL * table = gcvNULL;

    if (context->swLocTable.usedCount > 0)
    {
        gcmPRINT("-------------------------Loc table--------------------");
        if (gcmIS_ERROR(context->pfnAllocate(gcvNULL,gcmSIZEOF(gctBOOL) * context->swLocTable.usedCount, (gctPOINTER *)&table)))
        {
            gcmPRINT("-----------------------------------------------------");
            return;
        }

        gcoOS_ZeroMemory(table, gcmSIZEOF(gctBOOL) * context->swLocTable.usedCount);

        for (i = 0 ; i < context->swLocTable.usedCount; i++)
        {
            _vscDIDumpVariableLoc(context, (gctUINT16)i, table);
        }

        context->pfnFree(gcvNULL, table);
        gcmPRINT("-----------------------------------------------------");
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

void vscDIDumpDIETree(VSC_DIContext * context, gctUINT16 id)
{
    if (context)
    {
        _DIDumpDIETree(context,id, 0);
    }
}

void vscDIDumpDIE(VSC_DIContext * context, gctUINT16 id, gctUINT shift)
{
    gctUINT offset = 0;
    VSC_DIE * die;
    gctCHAR str[128];

    if (context)
    {
        die = VSC_DI_DIE_PTR(id);
        if (die != gcvNULL)
        {
            context->tmpLog[offset] = '\0';
            while (shift--)
            {
                gcoOS_StrCatSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, "    ");
            }

            if (die->tag == VSC_DI_TAG_VARIABE ||
                die->tag == VSC_DI_TAG_PARAMETER)
            {
                if (die->u.variable.loc.reg)
                {
                    gcoOS_PrintStrSafe(str, 128, &offset,
                       "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, PrimType = %d, type = %d, r%d - r%d",
                       die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo, die->lineNo, die->colNo, _GetNameStr(context,die->name),
                        die->u.variable.type.primitiveType, die->u.variable.type.type, die->u.variable.loc.u.reg.start,
                        die->u.variable.loc.u.reg.end);
                }
                else
                {
                    gcoOS_PrintStrSafe(str, 128, &offset,
                       "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, PrimType = %d, type = %d, 0x%x(0x%x - 0x%x)",
                        die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name),
                        die->u.variable.type.primitiveType, die->u.variable.type.type, die->u.variable.loc.u.offset.baseAddr,
                        die->u.variable.loc.u.offset.offset,
                        die->u.variable.loc.u.offset.endOffset);
                }
            }
            else if (die->tag == VSC_DI_TAG_TYPE)
            {
                if (!die->u.type.primitiveType)
                {
                    gcoOS_PrintStrSafe(str, 128, &offset, "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, defined type id = %d",
                        die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name),
                        die->u.type.type);
                }
                else
                {
                    if (die->u.type.array.numDim > 0)
                    {
                        gcoOS_PrintStrSafe(str, 128, &offset, "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, VIR primitive type ID = %d, %d {%d, %d, %d, %d}",
                            die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo, die->lineNo, die->colNo, _GetNameStr(context,die->name),
                            die->u.type.primitiveType, die->u.type.array.numDim, die->u.type.array.length[0],die->u.type.array.length[1],
                            die->u.type.array.length[2],die->u.type.array.length[3]);
                    }
                    else
                    {
                        gcoOS_PrintStrSafe(str, 128, &offset, "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, VIR primitive type ID = %d",
                            die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name),
                            die->u.type.primitiveType);
                    }
                }
            }
            else if (die->tag == VSC_DI_TAG_SUBPROGRAM)
            {
                gcoOS_PrintStrSafe(str, 128, &offset, "id = %d tag = %s parent = %d line(%d,%d,%d) name = %s, range (%d,%d), pc(%d,%d)",
                    die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name),
                    die->u.func.startLineNo, die->u.func.endLineNo, die->u.func.pcLine[0], die->u.func.pcLine[1]);
            }
            else if (die->tag == VSC_DI_TAG_LEXICALBLOCK)
            {
                gcoOS_PrintStrSafe(str, 128, &offset, "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s, scope (%d,%d)",
                    die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name),
                    die->u.lexScope.startLineNo, die->u.lexScope.endLineNo);
            }
            else
            {
                gcoOS_PrintStrSafe(str, 128, &offset, "id = %d tag = %s parent = %d line (%d,%d,%d) name = %s",
                    die->id, _GetTagNameStr(context, die->tag),die->parent,die->fileNo,die->lineNo, die->colNo, _GetNameStr(context,die->name));
            }

            gcoOS_StrCatSafe(context->tmpLog, VSC_DI_TEMP_LOG_SIZE, str);
            gcmPRINT(context->tmpLog);
        }
    }
}

gceSTATUS vscDISaveDebugInfo(VSC_DIContext * context, gctPOINTER * buffer, gctUINT32 * bufferSize)
{
    gctUINT8 * ptr;

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
                     + context->dieTable.count * gcmSIZEOF(VSC_DIE)
                     + context->strTable.size
                     + context->lineTable.count * gcmSIZEOF(VSC_DI_LINE_TABLE_MAP)
                     + context->swLocTable.count * gcmSIZEOF(VSC_DI_SW_LOC)
                     + context->locTable.count * gcmSIZEOF(VSC_DI_HW_LOC);
    }

    if (buffer && *buffer)
    {
        ptr = (gctUINT8 *)(*buffer);

        *ptr = 1;
        ptr += gcmSIZEOF(gctUINT8);

        *(VSC_DIContext *)ptr = *context;
        ptr += gcmSIZEOF(VSC_DIContext);

        gcoOS_MemCopy(ptr, (gctPOINTER)context->dieTable.die, context->dieTable.count * gcmSIZEOF(VSC_DIE) );
        ptr += context->dieTable.count * gcmSIZEOF(VSC_DIE);

        if (context->strTable.size > 0)
        {
            gcoOS_MemCopy(ptr, context->strTable.str, context->strTable.size);
            ptr += context->strTable.size;
        }

        if (context->lineTable.count > 0)
        {
            gcoOS_MemCopy(ptr, context->lineTable.map, context->lineTable.count * gcmSIZEOF(VSC_DI_LINE_TABLE_MAP));
            ptr +=context->lineTable.count * gcmSIZEOF(VSC_DI_LINE_TABLE_MAP);
        }

        if (context->swLocTable.count > 0)
        {
            gcoOS_MemCopy(ptr, context->swLocTable.loc, context->swLocTable.count * gcmSIZEOF(VSC_DI_SW_LOC));
            ptr += context->swLocTable.count * gcmSIZEOF(VSC_DI_SW_LOC);
        }

        if (context->locTable.count > 0)
        {
            gcoOS_MemCopy(ptr, context->locTable.loc, context->locTable.count * gcmSIZEOF(VSC_DI_HW_LOC));
            ptr += context->locTable.count * gcmSIZEOF(VSC_DI_HW_LOC);
        }

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

    if (context == gcvNULL || buffer == gcvNULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    ptr = *(VSC_DIContext **)buffer;

    pfnAllocate = ptr->pfnAllocate;

    if (gcmIS_ERROR(pfnAllocate(gcvNULL,gcmSIZEOF(VSC_DIContext), (gctPOINTER *)&ctx)))
    {
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    *context = ctx;

    *ctx = *ptr;

    pos = (gctUINT8 *)(ptr + 1);
    *bufferSize -= gcmSIZEOF(VSC_DIContext);

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

    *buffer = (gctPOINTER)pos;

    return gcvSTATUS_OK;
}

gceSTATUS vscDIConstructContext(PFN_Allocate allocPfn, PFN_Free freePfn, VSC_DIContext ** context)
{
    VSC_DIContext * ptr;
    PFN_Allocate pfnAllocate;
    PFN_Free pfnFree;

    pfnAllocate = (allocPfn == gcvNULL) ? gcoOS_Allocate : allocPfn;
    pfnFree = (freePfn == gcvNULL) ? gcoOS_Free : freePfn;

    if (gcmIS_ERROR(pfnAllocate(gcvNULL,gcmSIZEOF(VSC_DIContext), (gctPOINTER *)&ptr)))
    {
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    gcoOS_ZeroMemory(ptr, gcmSIZEOF(VSC_DIContext));

    ptr->pfnAllocate = pfnAllocate;
    ptr->pfnFree = pfnFree;

    ptr->cu = vscDIAddDIE(ptr, VSC_DI_TAG_COMPILE_UNIT, VSC_DI_INVALIDE_DIE, "CU_DIE", 0, 0, 0);

    *context = ptr;

    return gcvSTATUS_OK;
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
            vscDISetHwLocToSWLoc(context, 0xffff, 0xffff);
            context->pfnFree(gcvNULL, context->locTable.loc);
        }

        if (context->swLocTable.loc)
        {
            /* make stupid compiler happy, will delete soon */
            vscDIGetSWLoc(context, vscDIAddSWLoc(context));
            context->pfnFree(gcvNULL, context->swLocTable.loc);
        }

        context->pfnFree(gcvNULL, context);
    }
}

int vscDIGetSrcLineByPC(void * ptr, unsigned int pc, unsigned int * line)
{
    gctUINT i;
    gctUINT pcStart;
    gctUINT pcEnd;
    VSC_DIContext * context = (VSC_DIContext *) ptr;

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
void vscDIGetPCBySrcLine(void * ptr, unsigned int src, unsigned int * start, unsigned int * end)
{
    gctUINT i;
    VSC_DIContext * context = (VSC_DIContext *) ptr;
    gctUINT j = VSC_DI_INVALID_PC;

    *start = VSC_DI_INVALID_PC;
    for (i = 0 ; i < context->lineTable.count; i++)
    {
        if (/*(context->lineTable.map[i].sourcLoc.fileId == src.fileId) && */
            (context->lineTable.map[i].sourcLoc.lineNo == src))
        {
            if (*start == VSC_DI_INVALID_PC)
            {
                *start = i;
            }

            j = i;
        }
    }

    *end = j;
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

    if (die &&
        (die->tag == VSC_DI_TAG_VARIABE ||
        die->tag == VSC_DI_TAG_PARAMETER)
       )
    {
        str = _GetNameStr(context,die->name);

        if (gcmIS_SUCCESS(gcoOS_StrCmp(str, name)))
        {
            return die;
        }
    }

    return gcvNULL;
}

gctBOOL _vscDIGetVariableLocByPC(VSC_DIContext * context, gctUINT pc, VSC_DIE * die, VSC_DI_HW_LOC ** loc)
{
    VSC_DI_HW_LOC * hwLoc;

    hwLoc = VSC_DI_HW_LOC_PTR(die->u.variable.loc.hwLoc);

    while (hwLoc)
    {
        if (hwLoc->beginPC <= pc && hwLoc->endPC >= pc)
        {
            *loc = hwLoc;
            return gcvTRUE;
        }
        hwLoc = VSC_DI_HW_LOC_PTR(hwLoc->next);
    }

    return gcvFALSE;
}

gctBOOL vscDIGetVaribleLocByNameAndPC(VSC_DIContext * context, gctUINT pc, char * name, VSC_DI_HW_LOC ** loc)
{
    /*VIR_SourceFileLoc line;*/
    gctUINT line;
    VSC_DIE * innerDie;
    VSC_DIE * die;

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
                if (die->u.func.startLineNo >= line &&
                    die->u.func.endLineNo <= line)
                {
                    innerDie = die;
                    die =  VSC_DI_DIE_PTR(die->child);
                }
            }
            else if (die->tag == VSC_DI_TAG_LEXICALBLOCK)
            {
                if (die->u.lexScope.startLineNo >= line &&
                    die->u.lexScope.endLineNo <= line)
                {
                    innerDie = die;
                    die =  VSC_DI_DIE_PTR(die->child);
                }
            }
            else
            {
                die = VSC_DI_DIE_PTR(die->sib);
                continue;
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
        }

        /* look up the variable location by PC */
        if (die != gcvNULL &&
            _vscDIGetVariableLocByPC(context, pc, die, loc))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}


