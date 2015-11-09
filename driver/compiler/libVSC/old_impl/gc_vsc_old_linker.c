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


#include "gc_vsc.h"

#if gcdENABLE_3D

#include "vir/transform/gc_vsc_vir_uniform.h"
#include "old_impl/gc_vsc_old_optimizer.h"
#define _GC_OBJ_ZONE    gcvZONE_COMPILER

#define _USE_NEW_VARIABLE_HANDLER_  0

extern void
gcSL_SetInst2NOP(
    IN OUT gcSL_INSTRUCTION      Code
    );

gceSTATUS
gcLINKTREE_MarkAllAsUsed(
    IN gcLINKTREE Tree
    );

gceSTATUS
_LinkBuiltinLibs(
    IN gcSHADER* Shaders
);

/* Utility functions */
gctSOURCE_t
_SetSwizzle(
    IN gctUINT16 Swizzle,
    IN gcSL_SWIZZLE Value,
    IN OUT gctSOURCE_t *Source
    )
{
    /* Select on swizzle. */
    switch ((gcSL_SWIZZLE) Swizzle)
    {
    case gcSL_SWIZZLE_X:
        /* Set swizzle x. */
        return gcmSL_SOURCE_SET(*Source, SwizzleX, Value);

    case gcSL_SWIZZLE_Y:
        /* Set swizzle y. */
        return gcmSL_SOURCE_SET(*Source, SwizzleY, Value);

    case gcSL_SWIZZLE_Z:
        /* Set swizzle z. */
        return gcmSL_SOURCE_SET(*Source, SwizzleZ, Value);

    case gcSL_SWIZZLE_W:
        /* Set swizzle w. */
        return gcmSL_SOURCE_SET(*Source, SwizzleW, Value);

    default:
        return (gctUINT16) -1;
    }
}

gctUINT16
_SelectSwizzle(
    IN gctUINT16 Swizzle,
    IN gctSOURCE_t Source
    )
{
    /* Select on swizzle. */
    switch ((gcSL_SWIZZLE) Swizzle)
    {
    case gcSL_SWIZZLE_X:
        /* Select swizzle x. */
        return gcmSL_SOURCE_GET(Source, SwizzleX);

    case gcSL_SWIZZLE_Y:
        /* Select swizzle y. */
        return gcmSL_SOURCE_GET(Source, SwizzleY);

    case gcSL_SWIZZLE_Z:
        /* Select swizzle z. */
        return gcmSL_SOURCE_GET(Source, SwizzleZ);

    case gcSL_SWIZZLE_W:
        /* Select swizzle w. */
        return gcmSL_SOURCE_GET(Source, SwizzleW);

    default:
        return (gctUINT16) -1;
    }
}

gctINT
_getEnableComponentCount(
    IN gctUINT32 Enable
    )
{
    gctINT count = 0;
    if (Enable & gcSL_ENABLE_X)
    {
        count++;
    }
    if (Enable & gcSL_ENABLE_Y)
    {
        count++;
    }
    if (Enable & gcSL_ENABLE_Z)
    {
        count++;
    }
    if (Enable & gcSL_ENABLE_W)
    {
        count++;
    }
    return count;
}

gctUINT8
_Enable2SwizzleWShift(
    IN gctUINT32 Enable
    )
{
    switch (Enable)
    {
    case gcSL_ENABLE_X:
        return gcSL_SWIZZLE_XXXX;

    case gcSL_ENABLE_Y:
        return gcSL_SWIZZLE_YYYY;

    case gcSL_ENABLE_Z:
        return gcSL_SWIZZLE_ZZZZ;

    case gcSL_ENABLE_W:
        return gcSL_SWIZZLE_WWWW;

    case gcSL_ENABLE_XY:
        return gcSL_SWIZZLE_XYYY;

    case gcSL_ENABLE_XZ:
        return gcSL_SWIZZLE_XZZZ;

    case gcSL_ENABLE_XW:
        return gcSL_SWIZZLE_XWWW;

    case gcSL_ENABLE_YZ:
        return gcSL_SWIZZLE_YYZZ;

    case gcSL_ENABLE_YW:
        return gcSL_SWIZZLE_YYWW;

    case gcSL_ENABLE_ZW:
        return gcSL_SWIZZLE_ZZZW;

    case gcSL_ENABLE_XYZ:
        return gcSL_SWIZZLE_XYZZ;

    case gcSL_ENABLE_XYW:
        return gcSL_SWIZZLE_XYWW;

    case gcSL_ENABLE_XZW:
        return gcSL_SWIZZLE_XZZW;

    case gcSL_ENABLE_YZW:
        return gcSL_SWIZZLE_YYZW;

    case gcSL_ENABLE_XYZW:
        return gcSL_SWIZZLE_XYZW;

    default:
        break;

    }

    gcmFATAL("ERROR: Invalid enable 0x%04X", Enable);
    return gcSL_SWIZZLE_XYZW;
}

gctUINT8
_Enable2Swizzle(
    IN gctUINT32 Enable
    )
{
    switch (Enable)
    {
    case gcSL_ENABLE_X:
        return gcSL_SWIZZLE_XXXX;

    case gcSL_ENABLE_Y:
        return gcSL_SWIZZLE_YYYY;

    case gcSL_ENABLE_Z:
        return gcSL_SWIZZLE_ZZZZ;

    case gcSL_ENABLE_W:
        return gcSL_SWIZZLE_WWWW;

    case gcSL_ENABLE_XY:
        return gcSL_SWIZZLE_XYYY;

    case gcSL_ENABLE_XZ:
        return gcSL_SWIZZLE_XZZZ;

    case gcSL_ENABLE_XW:
        return gcSL_SWIZZLE_XWWW;

    case gcSL_ENABLE_YZ:
        return gcSL_SWIZZLE_YZZZ;

    case gcSL_ENABLE_YW:
        return gcSL_SWIZZLE_YWWW;

    case gcSL_ENABLE_ZW:
        return gcSL_SWIZZLE_ZWWW;

    case gcSL_ENABLE_XYZ:
        return gcSL_SWIZZLE_XYZZ;

    case gcSL_ENABLE_XYW:
        return gcSL_SWIZZLE_XYWW;

    case gcSL_ENABLE_XZW:
        return gcSL_SWIZZLE_XZWW;

    case gcSL_ENABLE_YZW:
        return gcSL_SWIZZLE_YZWW;

    case gcSL_ENABLE_XYZW:
        return gcSL_SWIZZLE_XYZW;

    default:
        break;

    }

    gcmFATAL("ERROR: Invalid enable 0x%04X", Enable);
    return gcSL_SWIZZLE_XYZW;
}

/*******************************************************************************
*********************************************************** Shader Compacting **
*******************************************************************************/

typedef struct _gcsJUMP *       gcsJUMP_PTR;
typedef struct _gcsJUMP
{
    gcsJUMP_PTR                 next;
    gctINT                      from;
    gctBOOL                     alwaysJmp;
}
gcsJUMP;

gceSTATUS
CompactShader(
    IN gcoOS Os,
    IN gcSHADER Shader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsJUMP_PTR * jumpTable = gcvNULL;
    gctUINT32 i;
    gctPOINTER pointer = gcvNULL;

    do
    {
        if (Shader->codeCount == 0)
        {
            break;
        }

        /* Allocate array of pointers. */
        gcmERR_BREAK(gcoOS_Allocate(Os,
                                    gcmSIZEOF(gcsJUMP_PTR) * Shader->codeCount,
                                    &pointer));

        jumpTable = pointer;

        /* Initialize array of pointers to NULL. */
            gcoOS_ZeroMemory(jumpTable,
                         gcmSIZEOF(gcsJUMP_PTR) * Shader->codeCount);

        /* Walk all instructions. */
        for (i = 0; i < Shader->codeCount; ++i)
        {
            gcSL_INSTRUCTION code;
            gctINT target;
            gctBOOL alwaysJmp = gcvFALSE;

            /* Get pointer to instruction. */
            code = Shader->code + i;

            /* Find JMP opcodes. */
            switch (gcmSL_OPCODE_GET(code->opcode, Opcode))
            {
            case gcSL_CALL:
            case gcSL_JMP:
                target = code->tempIndex;
                /* Check for a JMP.ALWAYS. */
                if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_JMP &&
                    gcmSL_TARGET_GET(Shader->code[i].temp, Condition) == gcSL_ALWAYS)
                {
                    alwaysJmp = gcvTRUE;
                }
                break;
            default:
                target = -1;
                break;
            }

            if ((target >= 0) && (target < (gctINT) Shader->codeCount))
            {
                gcsJUMP_PTR jump;

                /* Allocate a new gcsJUMP structure. */
                gcmERR_BREAK(gcoOS_Allocate(Os,
                                            gcmSIZEOF(gcsJUMP),
                                            &pointer));

                jump = pointer;

                /* Set location being jumped from. */
                jump->from = i;
                jump->alwaysJmp = alwaysJmp;

                /* Link gcsJUMP structure to target instruction. */
                jump->next        = jumpTable[target];
                jumpTable[target] = jump;
            }
        }

        if (gcmIS_ERROR(status))
        {
            /* Break on error. */
            break;
        }

        /* Walk all entries in array of pointers. */
        for (i = 0; i < Shader->codeCount; ++i)
        {
            const gctSIZE_T size = gcmSIZEOF(struct _gcSL_INSTRUCTION);
            gcsJUMP_PTR jump, modify;

            for (jump = jumpTable[i]; jump != gcvNULL; jump = jump->next)
            {
                if (jump->from == -1 || !jump->alwaysJmp)
                {
                    continue;
                }

                for (modify = jump->next;
                     modify != gcvNULL;
                     modify = modify->next)
                {
                    gctINT source = jump->from;
                    gctINT target = modify->from;

                    if (target == -1 || !modify->alwaysJmp)
                    {
                        continue;
                    }

                    while (gcmIS_SUCCESS(gcoOS_MemCmp(Shader->code + source,
                                                      Shader->code + target,
                                                      size)))
                    {
                        if (jumpTable[target] != gcvNULL)
                        {
                            break;
                        }

                        --source;
                        --target;

                        if ((source < 0) || (target < 0))
                        {
                            break;
                        }
                    }

                    /* Check for combined instructions that cannot be separted. */
                    if (target >= 0)
                    {
                        gcSL_INSTRUCTION code   = Shader->code + target;
                        gcSL_OPCODE      opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
                        if (gcSL_isOpcodeTexldModifier(opcode) ||
                            opcode == gcSL_SET || opcode == gcSL_CMP)
                        {
                            target = modify->from;
                        }
                    }
                    if (source >= 0 && target < modify->from)
                    {
                        gcSL_INSTRUCTION code   = Shader->code + source;
                        gcSL_OPCODE      opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
                        if (gcSL_isOpcodeTexldModifier(opcode) ||
                            opcode == gcSL_SET || opcode == gcSL_CMP)
                        {
                            target = modify->from;
                        }
                    }

                    if (modify->from - ++target > 0)
                    {
                        gctUINT16 offset = (gctUINT16)(modify->from - target);
                        gcsJUMP_PTR jmp;
                        gcSL_INSTRUCTION fromCode;

                        gcoOS_MemCopy(Shader->code + target,
                                      Shader->code + modify->from,
                                      size);
                        /* Change the code to JMP. */
                        Shader->code[target].tempIndex = (gctUINT16) (source + 1);
                        /* Move the next code. */
                        target++;
                        while (target != modify->from)
                        {
                            jmp = jumpTable[target];
                            while(jmp && jmp->from != -1)
                            {
                                gcmASSERT(!jmp->alwaysJmp);
                                fromCode = Shader->code + jmp->from;
                                /* Update the temp index for JMP/CALL. */
                                if (fromCode->tempIndex == (gctUINT16)target)
                                {
                                    fromCode->tempIndex += offset;
                                }
                                jmp = jmp->next;
                            }
                            gcoOS_ZeroMemory(Shader->code + target,
                                             size);

                            #if defined(gcSL_NOP) && gcSL_NOP
                                gcmSL_OPCODE_UPDATE(Shader->code[target].opcode, Opcode, gcSL_NOP);
                            #endif
                            target++;
                        }


                        modify->from = -1;
                    }
                }
            }
        }
    }
    while (gcvFALSE);

    if (jumpTable != gcvNULL)
    {
        /* Free up the entire array of pointers. */
        for (i = 0; i < Shader->codeCount; ++i)
        {
            /* Free of the linked list of gcsJUMP structures. */
            while (jumpTable[i] != gcvNULL)
            {
                gcsJUMP_PTR jump = jumpTable[i];
                jumpTable[i] = jump->next;

                gcmVERIFY_OK(gcmOS_SAFE_FREE(Os, jump));
            }
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(Os, jumpTable));
    }

    /* Return the status. */
    return status;
}

/*******************************************************************************
**                                  _DumpRegister
********************************************************************************
**
**  Print the format, type, and addressing mode of a register.
**
**  INPUT:
**
**      gcSL_TYPE Type
**          Type of the register.
**
**      gcSL_FORMAT Format
**          Format of the register.
**
**      gcSL_INDEX Index
**          Index of the register.
**
**      gcSL_INDEXED Mode
**          Addressing mode for the register.
**
**      gctINT Indexed
**          Indexed register if addressing mode is indexed.
**
**      gctBOOL Physical
**          Use physical registers instead of logical ones.
**
**  OUTPUT:
**
**      char * Buffer
**          Pointer to the buffer to be used as output.
**
**  RETURN VALUE:
**
**      gctINT
**          The number of characters copied into the output buffer.
*/
static gctINT
_DumpRegister(
    IN gcSL_TYPE Type,
    IN gcSL_FORMAT Format,
    IN gctUINT16 Index,
    IN gcSL_INDEXED Mode,
    IN gctINT Indexed,
    IN gctBOOL Physical,
    IN char * Buffer,
    IN gctSIZE_T BufferSize
    )
{
    gctUINT offset = 0;

    static gctCONST_STRING type[] =
    {
        "instruction", /* Use gcSL_NONE as an instruction. */
        "temp",
        "attribute",
        "uniform",
        "sampler",
        "constant",
        "output",
    };

    static gctCONST_STRING physicalType[] =
    {
        "",
        "r",
        "v",
        "c",
        "s",
        "c",
        "o",
    };

    static gctCONST_STRING format[] =
    {
        "", /* Don't append anything for float registers. */
        ".int",
        ".bool",
        ".uint",
        ".int8",
        ".uint8",
        ".int16",
        ".uint16",
        ".int64",
        ".uint64",
        ".snorm8",
        ".unorm8",
        ".float16",
        ".float64",
        ".snorm16",
        ".unorm16",
        ".invalid"
    };

    static const char index[] =
    {
        '?', 'x', 'y', 'z', 'w',
    };

    gcmASSERT(Format <= gcSL_INVALID);

    /* Print type, format and index of register. */
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                           "%s",
                           Physical ? physicalType[Type] : type[Type]));
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                           "%s(%d",
                           format[Format],
                           gcmSL_INDEX_GET(Index, Index)));

    if (gcmSL_INDEX_GET(Index, ConstValue) > 0)
    {
        /* Append the addressing mode. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                               "+%d", gcmSL_INDEX_GET(Index, ConstValue)));
    }

    if (Mode != gcSL_NOT_INDEXED)
    {
        /* Append the addressing mode. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                               "+%s",
                               Physical ? physicalType[gcSL_TEMP]
                                        : type[gcSL_TEMP]));
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                               "%s(%d).%c",
                               format[gcSL_INTEGER],
                               Indexed,
                               index[Mode]));
    }
    else if (Indexed > 0)
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, "+%d", Indexed));
    }

    /* Append the final ')'. */
    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, ")"));

    /* Return the number of characters printed. */
    return offset;
}

/*******************************************************************************
**                                    _DumpList
********************************************************************************
**
**  Print all entries in a gcsLINKTREE_LIST list.
**
**  INPUT:
**
**      gctCONST_STRING Title
**          Pointer to the title for the list.
**
**      gcsLINKTREE_LIST_PTR List
**          Pointer to the first gcsLINKTREE_LIST_PTR structure to print.
**
**      gctBOOL Physical
**          Use physical registers instead of logical ones.
**
**  OUTPUT:
**
**      Nothing.
*/
static void
_DumpList(
    IN gctCONST_STRING Title,
    IN gcsLINKTREE_LIST_PTR List,
    IN gctBOOL Physical
    )
{
    char buffer[256];
    gctUINT offset = 0;
    gctUINT length;

    length = (gctUINT) gcoOS_StrLen(Title, gcvNULL);
    length = gcmMIN(length, gcmSIZEOF(buffer) - 1);

    if (List == gcvNULL)
    {
        /* Don't print anything on an empty list. */
        return;
    }

    /* Print the title. */
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, Title));

    /* Walk while there are entries in the list. */
    while (List != gcvNULL)
    {
        /* Check if we have reached the right margin. */
        if (offset > 70)
        {
            /* Dump the assembled line. */
            gcoOS_Print("%s,", buffer);

            /* Indent to the end of the title. */
            for (offset = 0; offset < length; offset++)
            {
                buffer[offset] = ' ';
            }
        }
        else if (offset > length)
        {
            /* Print comma and space. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, ", "));
        }

        /* Decode the register. */
        offset += _DumpRegister(List->type,
                            gcSL_FLOAT,
                            (gctUINT16) List->index,
                            gcSL_NOT_INDEXED,
                            -1,
                            Physical,
                            buffer + offset,
                            gcmSIZEOF(buffer) - offset);

        /* Move to the next entry in the list. */
        List = List->next;
    }

    /* Dump the final assembled line. */
    gcoOS_Print("%s", buffer);
}

/*******************************************************************************
**                                   _DumpLinkTree
********************************************************************************
**
**  Print the shader and dependency tree for debugging.
**
**  INPUT:
**
**      gctCONST_STRING Text
**          Pointer to text to print before dumping shader and tree.
**
**      gcLINKTREE Tree
**          Pointer to the gcLINKTREE structure to print.
**
**  OUTPUT:
**
**      gcLINKTREE * Tree
**          Pointer to a variable receiving the gcLINKTREE structure pointer.
*/
void
_DumpLinkTree(
    IN gctCONST_STRING Text,
    IN gcLINKTREE Tree,
    IN gctBOOL DumpShaderOnly
    )
{
    gctSIZE_T i;
    char buffer[256];
    gcSHADER shader = Tree->shader;

    /* Print header. */
    for (i = 0; i < 79; i++)
    {
        buffer[i] = '=';
    }
    buffer[i] = '\0';

    gcoOS_Print("%s\n%s", buffer, Text);
    gcDump_Shader(gcvNULL, "Linktree Shader", gcvNULL, shader, gcvFALSE);

    if (DumpShaderOnly)
    {
        return;
    }

    /***************************************************************************
    ** II: Dump dependency tree.
    */

    /* Print header. */
    for (i = 0; i < 79; i++)
    {
        buffer[i] = '*';
    }
    buffer[i] = '\0';

    gcoOS_Print("\n%s\n[TREE]\n", buffer);

    /* Walk all attributes. */
    for (i = 0; i < Tree->attributeCount; i++)
    {
        /* Get the gcLINKTREE_ATTRIBUTE pointer. */
        gcLINKTREE_ATTRIBUTE attribute = Tree->attributeArray + i;

        /* Only dump valid attributes. */
        if (attribute->lastUse < 0)
        {
            continue;
        }

        /* Dump the attribute life span. */
        gcoOS_Print("  Attribute %d:",
                      i);
        gcoOS_Print("    No longer referenced after instruction %d",
                      attribute->lastUse);

        /* Dump the users of the attribute. */
        _DumpList("    Users: ", attribute->users, gcvFALSE);
    }

    for (i = 0; i < Tree->tempCount; i++)
    {
        /* Get the gcLINKTREE_TEMP pointer. */
        gcLINKTREE_TEMP temp = Tree->tempArray + i;
        gctUINT offset = 0;

        /* Only process if temporary register is defined. */
        if (temp->defined == gcvNULL)
        {
            continue;
        }

        /* Header of the temporary register. */
        gcoOS_Print("  Temp %d:", i);

        /* Check if this temporay register is an attribute. */
        if (temp->owner != gcvNULL)
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "    %s attribute for function ",
                                   (temp->inputOrOutput == gcvFUNCTION_INPUT)
                                       ? "Input" :
                                   (temp->inputOrOutput == gcvFUNCTION_OUTPUT)
                                       ? "Output" : "Inout"));

            if (temp->isOwnerKernel) {
                offset += gcSL_GetName(((gcKERNEL_FUNCTION)temp->owner)->nameLength,
                                   ((gcKERNEL_FUNCTION)temp->owner)->name,
                                   buffer + offset,
                                   gcmSIZEOF(buffer) - offset);
            } else {
                offset += gcSL_GetName(((gcFUNCTION)temp->owner)->nameLength,
                                   ((gcFUNCTION)temp->owner)->name,
                                   buffer + offset,
                                   gcmSIZEOF(buffer) - offset);
            }

            /* Dump the buffer. */
            gcoOS_Print("%s", buffer);
        }
        else
        {
            /* Life span of the temporary register. */
            gcoOS_Print("    No longer referenced after instruction %d",
                          temp->lastUse);
        }

        offset = 0;

        /* Check if this temporay register is a variable. */
        if (temp->variable != gcvNULL)
        {
            if ((gctINT)temp->variable->nameLength >= 0)
            {
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "    Variable: %s",
                                       temp->variable->name));
            }
            else
            {
                gctCONST_STRING name = _PredefinedName(shader, temp->variable->nameLength, gcvTRUE);
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "    Variable: %s",
                                       name));
            }

            /* Dump the buffer. */
            gcoOS_Print("%s", buffer);

            offset = 0;
        }

        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "    Usage: (inUse = %d) .", temp->inUse ));

        if (temp->usage & gcSL_ENABLE_X)
        {
            /* X is used. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, "x"));
        }

        if (temp->usage & gcSL_ENABLE_Y)
        {
            /* Y is used. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, "y"));
        }

        if (temp->usage & gcSL_ENABLE_Z)
        {
            /* Z is used. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, "z"));
        }

        if (temp->usage & gcSL_ENABLE_W)
        {
            /* W is used. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, "w"));
        }

        /* Dump the temporary register. */
        gcoOS_Print("%s", buffer);

        /* Dump the definitions of the temporary register. */
        _DumpList("    Definitions: ", temp->defined, gcvFALSE);

        /* Dump the dependencies of the temporary register. */
        _DumpList("    Dependencies: ", temp->dependencies, gcvFALSE);

        /* Dump the constant values for the components. */
        if ((temp->constUsage[0] == 1)
        ||  (temp->constUsage[1] == 1)
        ||  (temp->constUsage[2] == 1)
        ||  (temp->constUsage[3] == 1)
        )
        {
            gctINT index;
            gctUINT8 usage = temp->usage;

            offset = 0;
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "    Constants: {"));

            for (index = 0; (index < 4) && (usage != 0); ++index, usage >>= 1)
            {
                if (offset > 16)
                {
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           ", "));
                }

                if (temp->constUsage[index] == 1)
                {
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           "%f (%u)", temp->constValue[index].f, temp->constValue[index].u));
                }
                else if (temp->constUsage[index] == -1)
                {
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           "xxx"));
                }
                else
                {
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           "---"));
                }
            }

            if (offset > 16)
            {
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "}"));

                gcoOS_Print("%s", buffer);
            }
        }

        /* Dump the users of the temporary register. */
        _DumpList("    Users: ", temp->users, gcvFALSE);

        gcoOS_Print("    Last Use: %d", temp->lastUse);
    }

    /* Walk all outputs. */
    for (i = 0; i < Tree->outputCount; i++)
    {
        /* Only process outputs that are not dead. */
        if (Tree->outputArray[i].tempHolding < 0)
        {
            continue;
        }

        /* Dump dependency. */
        gcoOS_Print("  Output %d:", i);
        gcoOS_Print("    Dependent on %s(%d)",
                      Tree->physical ? "r" : "temp",
                      Tree->outputArray[i].tempHolding);

        if (Tree->outputArray[i].fragmentAttribute >= 0)
        {
            /* Dump linkage. */
            gcoOS_Print("    Linked to fragment index %d (attribute %d)",
                          Tree->outputArray[i].fragmentIndex,
                          Tree->outputArray[i].fragmentAttribute);
        }
    }

    /* Trailer. */
    for (i = 0; i < 79; i++)
    {
        buffer[i] = '=';
    }
    buffer[i] = '\0';

    gcoOS_Print("%s", buffer);
}


/*******************************************************************************
**                               dump_LinkTree
********************************************************************************
**
**  Print the shader and dependency tree for debugging.
**  Allways callable from debugger.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to the gcLINKTREE structure to print.
**
**  OUTPUT:
**
**      none
*/
void
dump_LinkTree(
    IN gcLINKTREE Tree
    )
{
    _DumpLinkTree("", Tree, gcvFALSE);
}

/*******************************************************************************
**                            gcLINKTREE_Construct
********************************************************************************
**
**  Construct a new gcLINKTREE structure.
**
**  INPUT:
**
**      gcoOS Os
**          Pointer to an gcoOS object.
**
**  OUTPUT:
**
**      gcLINKTREE * Tree
**          Pointer to a variable receiving the gcLINKTREE structure pointer.
*/
gceSTATUS
gcLINKTREE_Construct(
    IN gcoOS Os,
    OUT gcLINKTREE * Tree
    )
{
    gceSTATUS status;
    gcLINKTREE tree = gcvNULL;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER_ARG("Os=0x%x", Os);

    /* Allocate the gcLINKTREE structure. */
    gcmONERROR(
        gcoOS_Allocate(Os, sizeof(struct _gcLINKTREE), &pointer));

    tree = pointer;

    /* Initialize the gcLINKTREE structure.  */
    tree->shader         = gcvNULL;
    tree->attributeCount = 0;
    tree->attributeArray = gcvNULL;
    tree->tempCount      = 0;
    tree->tempArray      = gcvNULL;
    tree->outputCount    = 0;
    tree->packedAwayOutputCount    = 0;
    tree->outputArray    = gcvNULL;
    tree->physical       = gcvFALSE;
    tree->branch         = gcvNULL;
    tree->hints          = gcvNULL;
    tree->useICache      = gcvFALSE;
#if gcdUSE_WCLIP_PATCH
    tree->strictWClipMatch = gcvFALSE;
    tree->MVPCount = 0;
    tree->WChannelEqualToZ = gcvFALSE;
#endif

    /* Return the gcLINKTREE structure pointer. */
    *Tree = tree;

    /* Success. */
    gcmFOOTER_ARG("*Tree=0x%x", *Tree);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                             gcLINKTREE_Destroy
********************************************************************************
**
**  Destroy a gcLINKTREE structure.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcLINKTREE_Destroy(
    IN gcLINKTREE Tree
    )
{
    gctSIZE_T i;
    gcsLINKTREE_LIST_PTR node;
    gcCROSS_FUNCTION_LIST functionList;

    gcmHEADER_ARG("Tree=0x%x", Tree);

    /* See if there are any attributes. */
    if (Tree->attributeArray != gcvNULL)
    {
        /* Walk all attributes. */
        for (i = 0; i < Tree->attributeCount; i++)
        {
            /* Loop while there are users. */
            while (Tree->attributeArray[i].users != gcvNULL)
            {
                /* Get the user. */
                node = Tree->attributeArray[i].users;

                /* Remove the user from the list. */
                Tree->attributeArray[i].users = node->next;

                /* Free the gcLINKTREE_LIST structure. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, node));
            }
        }

        /* Free the array of attributes. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Tree->attributeArray));
    }

    /* Verify if there are any temporary registers. */
    if (Tree->tempArray != gcvNULL)
    {
        /* Loop through all temporary registers. */
        for (i = 0; i < Tree->tempCount; i++)
        {
            /* Loop while there are definitions. */
            while (Tree->tempArray[i].defined != gcvNULL)
            {
                /* Get the definition. */
                node = Tree->tempArray[i].defined;

                /* Remove the definitions from the list. */
                Tree->tempArray[i].defined = node->next;

                /* Free the gcLINKTREE_LIST structure. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, node));
            }

            /* Loop while there are dependencies. */
            while (Tree->tempArray[i].dependencies != gcvNULL)
            {
                /* Get the dependency. */
                node = Tree->tempArray[i].dependencies;

                /* Remove the dependency from the list. */
                Tree->tempArray[i].dependencies = node->next;

                /* Free the gcLINKTREE_LIST structure. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, node));
            }

            /* Loop while we have users. */
            while (Tree->tempArray[i].users != gcvNULL)
            {
                /* Get the user. */
                node = Tree->tempArray[i].users;

                /* Remove the user from the list. */
                Tree->tempArray[i].users = node->next;

                /* Free the gcCROSS_FUNCTION_LIST structure. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, node));
            }

            /* Loop while we function list. */
            while (Tree->tempArray[i].crossFuncList != gcvNULL)
            {
                /* Get the function list. */
                functionList = Tree->tempArray[i].crossFuncList;

                /* Remove the user from the list. */
                Tree->tempArray[i].crossFuncList = functionList->next;

                /* Free the gcLINKTREE_LIST structure. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, functionList));
            }
        }

        /* Free the array of temporary registers. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Tree->tempArray));
    }

    /* Verify if there are any output registers. */
    if (Tree->outputArray != gcvNULL)
    {
        /* Free the array of output registers. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Tree->outputArray));
    }

    /* Destroy any branches. */
    while (Tree->branch != gcvNULL)
    {
        /* Unlink branch from linked list. */
        gcSL_BRANCH_LIST branch = Tree->branch;
        Tree->branch = branch->next;

        /* Free the branch. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, branch));
    }

    /* Destroy any hints. */
    if (Tree->hints != gcvNULL)
    {
        for (i = 0; i < Tree->shader->codeCount; ++i)
        {
            /* Free any linked caller. */
            while (Tree->hints[i].callers != gcvNULL)
            {
                gcsCODE_CALLER_PTR caller = Tree->hints[i].callers;
                Tree->hints[i].callers = caller->next;

                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, caller));
            }

            /* Free any linked tempReg. */
            while (Tree->hints[i].liveTemps != gcvNULL)
            {
                gcLINKTREE_TEMP_LIST tempList = Tree->hints[i].liveTemps;
                Tree->hints[i].liveTemps = tempList->next;

                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempList));
            }
        }

        /* Free the hints. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Tree->hints));
    }

    /* Free the gcLINKTREE structure. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Tree));

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                             gcLINKTREE_AddList
********************************************************************************
**
**  Insert a new node into a linke list.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**      gcsLINKTREE_LIST_PTR * Root
**          Pointer to a variable holding the gcLINKTREE_LIST structure pointer.
**
**      gcSL_TYPE Type
**          Type for the new node.
**
**      gctINT Index
**          Index for the new node.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcLINKTREE_AddList(
    IN gcLINKTREE Tree,
    IN gcsLINKTREE_LIST_PTR * Root,
    IN gcSL_TYPE Type,
    IN gctINT Index
    )
{
    gceSTATUS status;
    gcsLINKTREE_LIST_PTR list;
    gctPOINTER pointer = gcvNULL;

    gcmHEADER_ARG("Tree=0x%x Root=0x%x Type=%d Index=%d", Tree, Root, Type, Index);

    /* Walk all entries in the list. */
    for (list = *Root; list != gcvNULL; list = list->next)
    {
        /* Does the current list entry matches the new one? */
        if ((list->type == Type) && (list->index == Index) )
        {
            /* Success. */
            ++list->counter;
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }

    /* Allocate a new gcsLINKTREE_LIST structure. */
    gcmONERROR(
        gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(gcsLINKTREE_LIST),
                       &pointer));

    list = pointer;

    /* Initialize the gcLINKTREE_LIST structure. */
    list->next    = *Root;
    list->type    = Type;
    list->index   = Index;
    list->counter = 1;

    /* Link the new gcLINKTREE_LIST structure into the list. */
    *Root = list;

    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                              _AttributeSource
********************************************************************************
**
**  Add an attribute as a source operand to the dependency tree.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**      gcSL_INSTRUCTION Code
**          Code.
**
**      gctINT SourceIndex
**          Source Index.
**
**      gctUIN16 Index
**          Index for the attribute.
**
**      gctINT TempIndex
**          Temporary register using the attribute.  When 'TempIndex' is less
**          than zero, there is no temporary register and the user is a
**          conditional instruction.
**
**      gctINT InstructionCounter
**          Current instruction counter.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS
_AttributeSource(
    IN gcLINKTREE Tree,
    IN gcSL_INSTRUCTION Code,
    IN gctINT SourceIndex,
    IN gctUINT16 Index,
    IN gctINT TempIndex,
    IN gctINT InstructionCounter
    )
{
    gcLINKTREE_ATTRIBUTE attribute;
    gceSTATUS status;
    gctUINT16 index = gcmSL_INDEX_GET(Index, Index);

    /* Get root for attribute. */
    gcmASSERT(index < Tree->attributeCount);
    attribute = &Tree->attributeArray[index];

    /* Update last usage of attribute. */
    attribute->lastUse = InstructionCounter;

    /* Add instruction user for attribute. */
    status = gcLINKTREE_AddList(Tree,
                                &attribute->users,
                                gcSL_NONE,
                                InstructionCounter);

    if (gcmIS_ERROR(status))
    {
        /* Error. */
        return status;
    }

    if (TempIndex >= 0)
    {
        /* Add attribute as dependency for target. */
        status = gcLINKTREE_AddList(Tree,
                                    &Tree->tempArray[TempIndex].dependencies,
                                    gcSL_ATTRIBUTE,
                                    index);
    }

    do
    {
        gcATTRIBUTE         attribute = Tree->shader->attributes[index];
        gcATTRIBUTE         childAttri = gcvNULL;
        gctSOURCE_t         source;
        gctSTRING           name = gcvNULL;
        gctSTRING           nameAfterDot = gcvNULL;
        gctSIZE_T           nameLength = 0;
        gcsIO_BLOCK         ioBlock = gcvNULL;
        gctINT16            childIndex;

        if (Code == gcvNULL || !gcmATTRIBUTE_isIOBlockMember(attribute) ||
            attribute->nameLength < 0 || !attribute->name)
        {
            break;
        }

        source = (SourceIndex == 0) ? Code->source0 : Code->source1Index;

        if (gcmSL_SOURCE_GET(source, Indexed) == gcSL_NOT_INDEXED)
        {
            break;
        }

        /* Make sure that this attribute is a member of an array structure. */
        if (gcmATTRIBUTE_isInstanceMember(attribute))
        {
            gcoOS_StrStr(attribute->name, ".", &nameAfterDot);
            if (nameAfterDot == gcvNULL)
            {
                break;
            }
            name = &nameAfterDot[1];
        }

        gcoOS_StrStr(name, ".", &nameAfterDot);
        if (nameAfterDot == gcvNULL)
        {
            break;
        }
        if (*(nameAfterDot-1) != ']')
        {
            break;
        }
        name = nameAfterDot - 1;
        while (name[0] != '[')
        {
            name = name - 1;
        }
        /* Get the name length of "instanceName.structName[". */
        nameLength = name - attribute->name + 1;

        /* Mark all struct members as used.*/
        ioBlock = Tree->shader->ioBlocks[attribute->ioBlockIndex];
        childIndex = GetIBIFirstChild(GetSBInterfaceBlockInfo(ioBlock));

        while (childIndex != -1)
        {
            gcmONERROR(gcSHADER_GetAttribute(Tree->shader,
                                             (gctUINT)childIndex,
                                             &childAttri));

            if (childAttri->name &&
                gcmIS_SUCCESS(gcoOS_StrNCmp(attribute->name,
                                            childAttri->name,
                                            nameLength)))
            {
                _AttributeSource(Tree,
                                 gcvNULL,
                                 0,
                                 childAttri->index,
                                 TempIndex,
                                 InstructionCounter);
            }

            childIndex = childAttri->nextSibling;
        }
    } while (gcvFALSE);

OnError:
    /* Return the status. */
    return status;
}

/*******************************************************************************
**                                 _TempSource
********************************************************************************
**
**  Add an temporary register as a source operand to the dependency tree.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**      gctSIZE_T Index
**          Index for the temporary register.
**
**      gctINT TempIndex
**          Temporary register using the temporary register.  When 'TempIndex'
**          is less than zero, there is no temporary register and the user is a
**          conditional instruction.
**
**      gctINT InstructionCounter
**          Current instruction counter.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS
_TempSource(
    IN gcLINKTREE Tree,
    IN gctSIZE_T Index,
    IN gctINT TempIndex,
    IN gctINT InstructionCounter
    )
{
    gcLINKTREE_TEMP temp;
    gcLINKTREE_TEMP paired64BitUpperTemp = gcvNULL;

    gceSTATUS status;

    /* Get root for temporary register. */
    gcmASSERT(Index < Tree->tempCount);
    temp = &Tree->tempArray[Index];

#if _SUPPORT_LONG_ULONG_DATA_TYPE
    if (isFormat64bit(temp->format))
    {
        gcmASSERT(Index+1 < Tree->tempCount);
        paired64BitUpperTemp = &Tree->tempArray[Index + 1];
    }
#endif
    /* Update last usage of temporary register. */
    temp->lastUse = InstructionCounter;

    /* Mark the temporary register is used used as a normal source. */
    temp->usedAsNormalSrc = gcvTRUE;

    /* Add instruction user for temporary register. */
    status = gcLINKTREE_AddList(Tree,
                                &temp->users,
                                gcSL_NONE,
                                InstructionCounter);

    if (!gcmIS_ERROR(status) && paired64BitUpperTemp)
    {
        paired64BitUpperTemp->lastUse = InstructionCounter;
        status = gcLINKTREE_AddList(Tree,
                                    &paired64BitUpperTemp->users,
                                    gcSL_NONE,
                                    InstructionCounter);
    }
    if (gcmIS_ERROR(status))
    {
        /* Error. */
        return status;
    }

    if (TempIndex >= 0)
    {
        /* Add temporary register as dependency for target. */
        status = gcLINKTREE_AddList(Tree,
                                    &Tree->tempArray[TempIndex].dependencies,
                                    gcSL_TEMP,
                                    Index);
    }

    /* Return the status. */
    return status;
}

/*******************************************************************************
**                                 _IndexedSource
********************************************************************************
**
**  Add an temporary register as a source index to the dependency tree.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**      gctSIZE_T Index
**          Index for the temporary register.
**
**      gctINT InstructionCounter
**          Current instruction counter.
**
**      gctINT TempIndex
**          Temporary register using the temporary register.  When 'TempIndex'
**          is less than zero, there is no temporary register and the user is a
**          conditional instruction.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS
_IndexedSource(
    IN gcLINKTREE Tree,
    IN gctSIZE_T Index,
    IN gctINT InstructionCounter,
    IN gctINT TempIndex
    )
{
    gcLINKTREE_TEMP temp;
    gcLINKTREE_TEMP paired64BitUpperTemp = gcvNULL;
    gceSTATUS status;

    /* Get root for temporary register. */
    gcmASSERT(Index < Tree->tempCount);
    temp = &Tree->tempArray[Index];

#if _SUPPORT_LONG_ULONG_DATA_TYPE
    if (isFormat64bit(temp->format))
    {
        gcmASSERT(Index+1 < Tree->tempCount);
        paired64BitUpperTemp = &Tree->tempArray[Index + 1];
    }
#endif
    /* Update last usage of temporary register. */
    temp->lastUse = InstructionCounter;

    /* Mark the temporary register is used as an index. */
    temp->isIndex = gcvTRUE;

    /* Add instruction user for temporary register. */
    status = gcLINKTREE_AddList(Tree,
                                &temp->users,
                                gcSL_NONE,
                                InstructionCounter);

    if (!gcmIS_ERROR(status) && paired64BitUpperTemp)
    {
        paired64BitUpperTemp->lastUse = InstructionCounter;
        paired64BitUpperTemp->isIndex = gcvTRUE;
        status = gcLINKTREE_AddList(Tree,
                                    &paired64BitUpperTemp->users,
                                    gcSL_NONE,
                                    InstructionCounter);
    }
    if (gcmIS_ERROR(status))
    {
        /* Error. */
        return status;
    }

    if (TempIndex >= 0)
    {
        /* Add temporary register as dependency for target. */
        status = gcLINKTREE_AddList(Tree,
                                    &Tree->tempArray[TempIndex].dependencies,
                                    gcSL_TEMP,
                                    Index);
    }

    /* Return the status. */
    return status;
}

/*******************************************************************************
**                                   _Delete
********************************************************************************
**
**  Delete all nodes inside a linked list.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**      gcsLINKTREE_LIST_PTR * Root
**          Pointer to the variable holding the linked list pointer.
**
**  OUTPUT:
**
**      Nothing.
*/
static void
_Delete(
    IN gcLINKTREE Tree,
    IN gcsLINKTREE_LIST_PTR * Root
    )
{
    /* Loop while there are valid nodes. */
    while (*Root != gcvNULL)
    {
        /* Get the node pointer. */
        gcsLINKTREE_LIST_PTR node = *Root;

        /* Remove the node from the linked list. */
        *Root = node->next;

        /* Free the node. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, node));
    }
}

static void
_FindCallers(
    IN gcLINKTREE Tree,
    IN gctPOINTER Owner,
    IN gctBOOL IsOwnerKernel,
    IN gctINT Nesting,
    IN OUT gctINT_PTR LastUse
    )
{
    gcsCODE_CALLER_PTR caller;

    if (IsOwnerKernel) {
        caller = Tree->hints[((gcKERNEL_FUNCTION)Owner)->codeStart].callers;
    } else {
        caller = Tree->hints[((gcFUNCTION)Owner)->codeStart].callers;
    }

    for (;
         caller != gcvNULL;
         caller = caller->next)
    {
        if ((Tree->hints[caller->caller].owner != gcvNULL)
        &&  (Tree->hints[caller->caller].callNest > Nesting)
        )
        {
            _FindCallers(Tree,
                         Tree->hints[caller->caller].owner,
                         Tree->hints[caller->caller].isOwnerKernel,
                         Nesting,
                         LastUse);
        }
        else if (*LastUse < 0 ||
                 Tree->hints[caller->caller].callNest < Tree->hints[*LastUse].callNest)
        {
            *LastUse = caller->caller;
        }
        else if (*LastUse < caller->caller)
        {
            *LastUse = caller->caller;
        }
    }
}

static void
_addTempToLoopHeadLiveList(
    IN gcLINKTREE       Tree,
    IN gcLINKTREE_TEMP  TempReg,
    IN gctINT           JmpIndex
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER            pointer         = gcvNULL;
    gcoOS                 os              = gcvNULL;
    gcLINKTREE_TEMP_LIST  tempList;
    gcSL_INSTRUCTION      code            = &Tree->shader->code[JmpIndex];
    gctUINT               loopHeadInstIdx = code->tempIndex; /* head of loop */

    gcmASSERT(Tree->hints[JmpIndex].isBackJump);
    gcmASSERT(Tree->hints[loopHeadInstIdx].callers != gcvNULL);

    /* Allocate gcLINKTREE_TEMP_LIST node. */
    status = gcoOS_Allocate(os,
                            gcmSIZEOF(struct _gcLINKTREE_TEMP_LIST),
                            &pointer);

    gcmASSERT(pointer != gcvNULL);
    if (!gcmIS_ERROR(status))
    {
        tempList       = pointer;
        tempList->next = Tree->hints[loopHeadInstIdx].liveTemps;
        tempList->temp = TempReg;

        Tree->hints[loopHeadInstIdx].liveTemps = tempList;
    }
}

static void
_addTempToFunctionLiveList(
    IN gcLINKTREE           Tree,
    IN gcLINKTREE_TEMP      TempReg,
    IN gctINT               CallIndex
    )
{
    gceSTATUS               status          = gcvSTATUS_OK;
    gctPOINTER              pointer         = gcvNULL;
    gcoOS                   os              = gcvNULL;
    gcLINKTREE_TEMP_LIST    tempList;
    gcCROSS_FUNCTION_LIST   functionList;
    gcSL_INSTRUCTION        callCode        = &Tree->shader->code[CallIndex];
    gctUINT                 funcHeadInstIdx = callCode->tempIndex; /* head of function */
    gcFUNCTION              function        = gcvNULL;
    gctUINT                 i;

    gcmASSERT(Tree->hints[CallIndex].isCall);
    gcmASSERT(Tree->hints[funcHeadInstIdx].callers != gcvNULL);
    /* check if the temp is defined in called function */
    if (TempReg->owner != gcvNULL &&
        TempReg->owner == Tree->hints[funcHeadInstIdx].owner)
    {
        /* this can happen if the register is output */
        return;
    }

    /* check if the tempReg is already in the tempList */
    for (tempList = Tree->hints[funcHeadInstIdx].liveTemps;
        tempList != gcvNULL; tempList = tempList->next)
    {
        if (tempList->temp == TempReg)
        {
            /* found it in the list */
            return;
        }
    }
    /* Allocate gcLINKTREE_TEMP_LIST node. */
    status = gcoOS_Allocate(os,
                            gcmSIZEOF(struct _gcLINKTREE_TEMP_LIST),
                            &pointer);

    gcmASSERT(pointer != gcvNULL);
    if (!gcmIS_ERROR(status))
    {
        tempList       = pointer;
        tempList->next = Tree->hints[funcHeadInstIdx].liveTemps;
        tempList->temp = TempReg;

        Tree->hints[funcHeadInstIdx].liveTemps = tempList;
    }

    /* Add this function to the cross function list. */
    status = gcoOS_Allocate(os,
                            gcmSIZEOF(struct _gcsCROSS_FUNCTION_LIST),
                            &pointer);

    gcmASSERT(pointer != gcvNULL);
    if (!gcmIS_ERROR(status))
    {
        functionList = pointer;
        gcoOS_ZeroMemory(pointer, gcmSIZEOF(struct _gcsCROSS_FUNCTION_LIST));
        functionList->callIndex = CallIndex;
        functionList->next = TempReg->crossFuncList;
        TempReg->crossFuncList = functionList;
    }

    gcSHADER_GetFunctionByHeadIndex(Tree->shader, funcHeadInstIdx, &function);
    if (function)
    {
        for (i = function->codeStart; i < function->codeStart + function->codeCount; i++)
        {
            gcSL_INSTRUCTION code = &Tree->shader->code[i];

            if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_CALL)
            {
                _addTempToFunctionLiveList(Tree, TempReg, i);
            }
        }
    }
    else if (Tree->shader->type == gcSHADER_TYPE_CL)
    {
        gcKERNEL_FUNCTION kernelFunction = gcvNULL;
        gcSHADER_GetKernelFunctionByHeadIndex(Tree->shader, funcHeadInstIdx, &kernelFunction);
        if (kernelFunction)
        {
            for (i = kernelFunction->codeStart; i < kernelFunction->codeStart + kernelFunction->codeCount; i++)
            {
                gcSL_INSTRUCTION code = &Tree->shader->code[i];

                if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_CALL)
                {
                    _addTempToFunctionLiveList(Tree, TempReg, i);
                }
            }
        }
        else
        {
            gcmASSERT(kernelFunction != gcvNULL);
        }
        }
    else
    {
        gcmASSERT(function != gcvNULL);
    }
}

/* return true if the FristCode and SecondCode in the same Basic Block:
 *
 *   1. There is no control flow instruction in between
 *   2. There is no jump or call to the code in between
 *
 */
static gctBOOL
_isCodeInSameBB(
    IN gcLINKTREE      Tree,
    IN gctINT          FirstCodeIdx,
    IN gctINT          SecondCodeIdx,
    IN gcLINKTREE_TEMP TempReg,
    OUT gctINT *       CrossLoopIndex
    )
{
    gctINT curCodeIdx = SecondCodeIdx;
    gctBOOL inSameBB = gcvTRUE;

    gcmASSERT(Tree->hints != gcvNULL &&
              FirstCodeIdx >= 0 &&
              SecondCodeIdx > FirstCodeIdx &&
              ((SecondCodeIdx < (gctINT)Tree->shader->codeCount) ||
               (SecondCodeIdx >= (gctINT)Tree->shader->codeCount &&
               (TempReg->users->type == gcSL_OUTPUT || GetVariableIsOtput(TempReg->variable)))
              ));

    /* If the variable is an output, then the user code index may exceed the code count. */
    if (SecondCodeIdx >= (gctINT)Tree->shader->codeCount)
    {
        inSameBB = gcvTRUE;
        return inSameBB;
    }

    /* start from last statement */
    for (; curCodeIdx >= FirstCodeIdx; curCodeIdx--)
    {
        gcSL_INSTRUCTION curCode = &Tree->shader->code[curCodeIdx];
        gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(curCode->opcode, Opcode);
        if ((opcode == gcSL_JMP && curCodeIdx != SecondCodeIdx)
            || opcode == gcSL_CALL
            || opcode == gcSL_RET)
        {
            if (Tree->hints[curCodeIdx].isBackJump && CrossLoopIndex != gcvNULL)
            {
                if (*CrossLoopIndex == -1)
                {
                    *CrossLoopIndex = curCodeIdx;
                }
                else
                {
                    gctINT prevLoopHead = Tree->shader->code[*CrossLoopIndex].tempIndex;
                    gctINT curLoopHead  = curCode->tempIndex;

                    /* put the temp in the first loop it entered */
                    if (curLoopHead < prevLoopHead)
                    {
                        *CrossLoopIndex = curCodeIdx;
                    }
                }
            }
            if (opcode == gcSL_CALL && TempReg != gcvNULL)
            {
                /* temp register is alive across a function call, the callee
                   function cannot use the register assigned to the temp
                   register, add the temp reigster to the function's
                   pre-alloc temp register list
                 */
                _addTempToFunctionLiveList(Tree, TempReg, curCodeIdx);
            }
            /* branch is not last statement */
            inSameBB = gcvFALSE;
        }

        if (Tree->hints[curCodeIdx].callers != gcvNULL && curCodeIdx != FirstCodeIdx)
        {
            /* has other instruction jump to this code */
            inSameBB = gcvFALSE;
        }
    } /* for */

    /* found the second code can be reached without meet any control flow */
    return inSameBB;
} /* _isCodeInSameBB */

static gctBOOL
_defineAndUserInSameEBB(
    IN gcLINKTREE      Tree,
    IN gcLINKTREE_TEMP Temp)
{
    gcsLINKTREE_LIST_PTR        user;
    gcsLINKTREE_LIST_PTR        def;
    gctINT   defineCodeIdx;
    gctINT   firstUsedIdx = 0x7fffffff;
    gctINT   firstDefine  = 0x7fffffff;
    gctINT   lastUsedIdx  = 0;
    gctINT   crossLoopIdx = -1;
    gctBOOL  inSameEBB = gcvTRUE;
    /* return true if there is no user */
    if (Temp->users == gcvNULL)
        return gcvTRUE;

    /* find the first and last use code */
    for (user = Temp->users; user != gcvNULL; user = user->next)
    {
        if (user->index < firstUsedIdx)
            firstUsedIdx = user->index;
        if (user->index > lastUsedIdx)
            lastUsedIdx = user->index;
    }

    /* go through each definition */
    for (def = Temp->defined; def != gcvNULL; def = def->next)
    {
        defineCodeIdx = def->index;

        if (defineCodeIdx < firstDefine)
            firstDefine = defineCodeIdx;

        if (firstUsedIdx <= defineCodeIdx)
        {
            /* there are two cases:
                1.  the temp is undefined and use it self as initializer:
                       ADD  temp(1), temp(1), 1.0

                2.  the define and use is in a loop:
                     int temp2, temp1=1;
                     for (int i=0; i< 10; i++) {
                          if (i > 0)
                            temp1 = temp2;
                          else
                            temp2 = 0;
                     }
             */
            inSameEBB = gcvFALSE;
        }

        if (defineCodeIdx < lastUsedIdx &&
            !_isCodeInSameBB(Tree, defineCodeIdx, lastUsedIdx, Temp, &crossLoopIdx))
            inSameEBB = gcvFALSE;
    }

    /*
    ** If this temp register holds a global variable, all function calls between its first define code
    ** and last used code should add this temp into their live temp list.
    */
    if (Temp->variable != gcvNULL &&
        IsVariableGlobal(Temp->variable) &&
        !IsVariableOutput(Temp->variable))
    {
        gctINT startIdx = 0, endIdx = 0;
        gctINT i;

        startIdx = gcmMIN(firstDefine, firstUsedIdx);
        startIdx = gcmMIN(startIdx, lastUsedIdx);
        endIdx = gcmMAX(firstDefine, firstUsedIdx);
        endIdx = gcmMAX(endIdx, lastUsedIdx);

        gcmASSERT(startIdx < (gctINT)Tree->shader->codeCount &&
                  endIdx < (gctINT)Tree->shader->codeCount);

        for (i = startIdx; i <= endIdx; i++)
        {
            gcSL_INSTRUCTION code = &Tree->shader->code[i];

            if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_CALL)
            {
                continue;
            }

            _addTempToFunctionLiveList(Tree, Temp, i);
        }
    }

    /* If this temp register is a input argument of a function,
    ** we need to assume that there is a definition for this temp on the head of this function.
    */
    if (Temp->inputOrOutput == gcvFUNCTION_INPUT ||
        Temp->inputOrOutput == gcvFUNCTION_INOUT)
    {
        gcFUNCTION function = gcvNULL;

        function = (gcFUNCTION)Temp->owner;
        gcmASSERT(function != gcvNULL);

        defineCodeIdx = function->codeStart;
        gcmASSERT(defineCodeIdx >= 0);

        if (defineCodeIdx < firstDefine)
            firstDefine = defineCodeIdx;

        if (firstUsedIdx <= defineCodeIdx)
        {
            inSameEBB = gcvFALSE;
        }

        if (defineCodeIdx < lastUsedIdx &&
            !_isCodeInSameBB(Tree, defineCodeIdx, lastUsedIdx, Temp, &crossLoopIdx))
            inSameEBB = gcvFALSE;
    }

    if (crossLoopIdx != -1)
    {
        gctINT loopHead = Tree->shader->code[crossLoopIdx].tempIndex;

        if (loopHead < firstDefine)
        {
            gcmASSERT(inSameEBB == gcvFALSE);
            Temp->crossLoopIdx = crossLoopIdx;
            /* add temp to loop header */
            _addTempToLoopHeadLiveList(Tree, Temp, crossLoopIdx);
        }
    }
    /* no one is in different BB */
    return inSameEBB;
}

static void
_updateTempLastUse(
    IN gcLINKTREE       Tree,
    IN gcLINKTREE_TEMP  TempReg,
    IN gctINT           LastUse
    )
{
    gcmASSERT(LastUse < (gctINT)Tree->shader->codeCount);
    TempReg->lastUse = LastUse;

    /* if the temp's live range extended to the end of loop, it should
       also extend to the begin of the loop */
    if (LastUse >= 0 && Tree->hints[LastUse].isBackJump)
    {
        gcmASSERT(LastUse < (gctINT)Tree->shader->codeCount);
        _addTempToLoopHeadLiveList(Tree, TempReg, LastUse);
    }
}

static gctINT
_GetNumUsedComponents(
    IN gcSL_ENABLE Enable
    )
{
    gctINT  usedComponents = 0;
    gctINT  i;

    for (i=0; i < gcSL_COMPONENT_COUNT; i++)
    {
        if (gcmIsComponentEnabled(Enable, i)) {
            usedComponents++;
        }
    }
    return usedComponents;
}

static gcUNIFORM
_FindUniformBlockLeafMember(
    IN gcSHADER Shader,
    IN gcUNIFORM CurrentMember,
    IN gctINT Offset,
    OUT gctUINT16 *Index,
    OUT gctINT *StartChannel
    )
{
    gceSTATUS status;
    gcUNIFORM currentMember, nextMember;
    gcUNIFORM blockUniformMember = gcvNULL;
    gctUINT16 index = 0;
    gctINT startChannel = 0;

    nextMember = CurrentMember;
    while (nextMember) {
       currentMember = nextMember;
       if(currentMember->nextSibling == -1) {
           nextMember = gcvNULL;
       }
       else {
           status = gcSHADER_GetUniform(Shader,
                                        currentMember->nextSibling,
                                        &nextMember);
           if (gcmIS_ERROR(status)) return gcvNULL;
           if ((nextMember->offset != -1) && Offset >= nextMember->offset) {
              continue;
           }
       }

       if (gcmType_Kind(currentMember->u.type) == gceTK_SAMPLER)
       {
           continue;
       }
       else if (currentMember->offset != Offset)
       {
           /* The member needs to be either an array/matrix element or struct element */
           if (isUniformBlockMember(currentMember)) {
               gctINT32 remainder;

               remainder = Offset - currentMember->offset;

               if (isUniformArray(currentMember)) { /* is an array */
                   index = (gctUINT16)(remainder / currentMember->arrayStride);
                   if (index < currentMember->arraySize) {
                       remainder -= (index * currentMember->arrayStride);
                   }
                   else
                   {
                       /* If the index is out of range, then it is not in this array, we need to reset the index. */
                       index = 0;
                       continue;
                   }
               }

               if (gcmType_isMatrix(currentMember->u.type)) {
                   gctUINT32 rows;
                   gctUINT32 columns;
                   gcTYPE_GetTypeInfo(currentMember->u.type,
                                      &rows,
                                      &columns,
                                      gcvNULL);
                   index *= (gctUINT16)columns;

                   if (remainder)
                   { /* assumption is that offset should always be at vector boundary */
                       gctSIZE_T matrixIndex;

                       matrixIndex = remainder / currentMember->matrixStride;
                       if (currentMember->isRowMajor)
                       {
                           gctSIZE_T columnIndex = 0;

                           if (matrixIndex < rows)
                           {
                              remainder -= (matrixIndex * currentMember->matrixStride);
                              /* compute the column vector index */
                              columnIndex = remainder / 4;  /* assuming 4 bytes for float */
                              if(columnIndex >= columns) continue;

                              gcmASSERT(matrixIndex < 4);

                              startChannel = matrixIndex;
                           }
                           else continue;
                           index += (gctUINT16)columnIndex;
                       }
                       else
                       {
                           if (matrixIndex < columns)
                           {
                              remainder -= (matrixIndex * currentMember->matrixStride);
                              gcmASSERT(remainder == 0);
                              if (remainder) break;
                              index += (gctUINT16)matrixIndex;
                           }
                           else continue;
                       }
                   }
               }
               else {
                   /* Assume offset be always at vector boundary */
                   if (remainder) continue;
               }
           }
           else if (gcmType_Kind(currentMember->u.type) != gceTK_SAMPLER)
           { /* check the struct */
               gcmASSERT(isUniformStruct(currentMember));
               gcmASSERT(currentMember->firstChild != -1);
               status = gcSHADER_GetUniform(Shader,
                                            currentMember->firstChild,
                                            &currentMember);
               if (gcmIS_ERROR(status)) return gcvNULL;
               currentMember = _FindUniformBlockLeafMember(Shader,
                                                           currentMember,
                                                           Offset,
                                                           &index,
                                                           &startChannel);
               if(currentMember == gcvNULL) continue;
           }
       }
       else if (isUniformStruct(currentMember))
       {
           gcmASSERT(currentMember->firstChild != -1);
           status = gcSHADER_GetUniform(Shader,
                                        currentMember->firstChild,
                                        &currentMember);
           if (gcmIS_ERROR(status)) return gcvNULL;
           currentMember = _FindUniformBlockLeafMember(Shader,
                                                       currentMember,
                                                       Offset,
                                                       &index,
                                                       &startChannel);
           if(currentMember == gcvNULL) return gcvNULL;
       }
       blockUniformMember = currentMember;
       break;
    }
    *Index = index;
    *StartChannel = startChannel;
    return blockUniformMember;
}

gcUNIFORM
_FindUniformBlockMember(
    IN gcSHADER Shader,
    IN gcUNIFORM BlockUniform,
    IN gctUINT BlockIndex,
    IN gctINT Offset,
    OUT gctUINT16 *Index,
    OUT gctINT *StartChannel
    )
{
    gceSTATUS status;
    gcUNIFORM currentMember;
    gcsUNIFORM_BLOCK uniformBlock;

    gcmASSERT(Index);
    gcmASSERT(BlockUniform->blockIndex != -1);

    status = gcSHADER_GetUniformBlock(Shader,
                                      BlockUniform->blockIndex + BlockIndex,
                                      &uniformBlock);
    if (gcmIS_ERROR(status)) return gcvNULL;

    gcmASSERT(GetUBFirstChild(uniformBlock) != -1);
    status = gcSHADER_GetUniform(Shader,
                                 GetUBFirstChild(uniformBlock),
                                 &currentMember);
    if (gcmIS_ERROR(status)) return gcvNULL;

    return _FindUniformBlockLeafMember(Shader,
                                       currentMember,
                                       Offset,
                                       Index,
                                       StartChannel);
}

static void
_InitTreeTemp(
    IN OUT gcLINKTREE_TEMP     Temp,
    IN     gctINT              Index,
    IN     gcSL_FORMAT         Format
    )
{
    Temp->index                 = Index;
    Temp->inUse                 = gcvFALSE;
    Temp->usage                 = 0;
    Temp->isIndex               = gcvFALSE;
    Temp->usedAsNormalSrc       = gcvFALSE;
    Temp->isIndexing            = gcvFALSE;
    Temp->isPaired64BitUpper    = gcvFALSE;
    Temp->crossLoopIdx          = -1;
    Temp->defined               = gcvNULL;
    Temp->constUsage[0]         = 0;
    Temp->constUsage[1]         = 0;
    Temp->constUsage[2]         = 0;
    Temp->constUsage[3]         = 0;
    /* change the initial lastUse to be -2, instead of -1.
       -1 means the last use is the end of shader.
       If the destination of an instruction is not used (e.g., atomic operation),
       the initial value will be used to assign register. If -1, the register will be
       reserved until the end of shader. But this register is never used. It will
       causes extra register usage.
    */
    Temp->lastUse               = -2;
    Temp->dependencies          = gcvNULL;
    Temp->users                 = gcvNULL;
    Temp->assigned              = -1;
    Temp->owner                 = gcvNULL;
    Temp->isOwnerKernel         = gcvFALSE;
    Temp->variable              = gcvNULL;
    Temp->format                = Format;
    Temp->precision             = gcSL_PRECISION_MEDIUM;
    Temp->inputOrOutput         = (gceINPUT_OUTPUT)-1;
    Temp->crossFuncList         = gcvNULL;
}

static void
_InitTreeHints(
    IN OUT gcsCODE_HINT_PTR     Hints,
    IN     gctINT              Index
    )
{
    Hints[Index].owner          = gcvNULL;
    Hints[Index].isOwnerKernel  = gcvFALSE;
    Hints[Index].isBackJump     = gcvFALSE;
    Hints[Index].isCall         = gcvFALSE;
    Hints[Index].isLoopHeader   = gcvFALSE;
    Hints[Index].callers        = gcvNULL;
    Hints[Index].callNest       = -1;
    Hints[Index].liveTemps      = gcvNULL;
    Hints[Index].lastUseForTemp = Index;
    Hints[Index].lastLoadUser   = -1;
    Hints[Index].loadDestIndex  = -1;
}

static gceSTATUS
_GetFunctionByArgumentIndex(
    IN gcLINKTREE Tree,
    IN gctINT RegIndex,
    IN gceINPUT_OUTPUT argType,
    OUT gctINT *FunctionIndex,
    OUT gctUINT *CodeStart
    )
{
    gctSIZE_T i;
    gctINT funcIndex = -1;
    gctUINT codeStart = 0;

    for (i = 0; i < Tree->shader->functionCount; i++)
    {
        gcFUNCTION function = Tree->shader->functions[i];
        gctSIZE_T j;

        for (j  = 0; j < function->argumentCount; j++)
        {
            if (RegIndex == function->arguments[j].index && function->arguments[j].qualifier == argType)
            {
                codeStart = function->codeStart;
                funcIndex = i;
                break;
            }
        }

        if (funcIndex != -1)
            break;
    }

    if (funcIndex != -1)
    {
        *FunctionIndex = funcIndex;
        *CodeStart = codeStart;
        return gcvSTATUS_OK;
    }

    for (i = 0; i < Tree->shader->kernelFunctionCount; i++)
    {
        gcKERNEL_FUNCTION function = Tree->shader->kernelFunctions[i];
        gctSIZE_T j;

        for (j  = 0; j < function->argumentCount; j++)
        {
            if (RegIndex == function->arguments[j].index && function->arguments[j].qualifier == argType)
            {
                codeStart = function->codeStart;
                funcIndex = i;
                break;
            }
        }

        if (funcIndex != -1)
            break;
    }


    *FunctionIndex = funcIndex;
    *CodeStart = codeStart;
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                              gcLINKTREE_Build
********************************************************************************
**
**  Build a tree based on a gcSHADER object.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**      gcSHADER Shader
**          Pointer to a gcSHADER object.
**
**      gceSHADER_FLAGS Flags
**          Link flags.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcLINKTREE_Build(
    IN gcLINKTREE Tree,
    IN gcSHADER Shader,
    IN gceSHADER_FLAGS Flags
    )
{
    gcoOS os;
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T i, pc, f, curInstIdx;
    gctSIZE_T tempCount, texAttrCount;
    gcSL_INSTRUCTION code          = gcvNULL;
    gctSOURCE_t source             = 0;
    gcLINKTREE_TEMP temp           = gcvNULL;
    gcLINKTREE_TEMP paired64BitUpperTemp = gcvNULL;
    gctTARGET_t target               = 0;
    gctUINT16 index                = 0;
    gcsLINKTREE_LIST_PTR * texAttr = gcvNULL;
    gctINT nest                    = 0;
    gctBOOL modified               = gcvFALSE;
    gctPOINTER pointer             = gcvNULL;
    gctBOOL psHasDiscard           = gcvFALSE;
    gcSL_OPCODE opcode;

    /* Extract the gcoOS object pointer. */
    os = gcvNULL;

    /* Save the shader object. */
    Tree->shader = Shader;
    Tree->flags  = Flags;

    /***************************************************************************
    ** I - Count temporary registers.
    */

    /* Initialize number of temporary registers. */
    tempCount    = 0;
    texAttrCount = 0;

    /* Check symbol table to find the maximal index. */
    if (Shader->variableCount > 0)
    {
        gctUINT variableCount = Shader->variableCount;

        for (i = 0; i < variableCount; i++)
        {
            gcVARIABLE variable = Shader->variables[i];

            if (isVariableNormal(variable))
            {
                gctUINT size = GetVariableKnownArraySize(variable) * gcmType_Rows(variable->u.type);
                gctUINT end = variable->tempIndex + size;

                if (end > tempCount) tempCount = end;
            }
        }
    }

    if ((Shader->outputCount > 0))
    {
        gctUINT outputCount = Shader->outputCount;

        for (i = 0; i < outputCount; i++)
        {
            gctUINT size = 0;
            gctUINT end  = 0;
            gcOUTPUT output = Shader->outputs[i];

            if (!output || output->tempIndex < tempCount) continue;
            size = output->arraySize * gcmType_Rows(output->type);
            end = output->tempIndex + size;

            if (end > tempCount) tempCount = end;
        }
    }

    /******************** Check the argument temp index Function **********************/
    for (i = 0; i < Shader->functionCount; ++i)
    {
        gcFUNCTION function = Shader->functions[i];
        gctSIZE_T j;

        for (j = 0; j < function->argumentCount; ++j)
        {
            gctINT argIndex = function->arguments[j].index;

            if  (argIndex >= (gctINT) tempCount)
            {
                tempCount = argIndex + 1;
            }
        }
    }

    /*************** Check the argument temp index for Kernel Function *****************/
    for (i = 0; i < Shader->kernelFunctionCount; ++i)
    {
        gcKERNEL_FUNCTION kernelFunction = Shader->kernelFunctions[i];
        gctSIZE_T j;

        for (j = 0; j < kernelFunction->argumentCount; ++j)
        {
            gctINT argIndex = kernelFunction->arguments[j].index;
            if  (argIndex >= (gctINT) tempCount)
            {
                tempCount = argIndex + 1;
            }
        }
    }

    /* Walk all instructions. */
    for (curInstIdx = 0; curInstIdx < Shader->codeCount; curInstIdx++)
    {
        /* Get instruction. */
        code = &Shader->code[curInstIdx];
        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

        if (gcSL_isOpcodeHaveNoTarget(opcode))
        {
            /* Determine temporary register usage. */
            switch (opcode)
            {
            case gcSL_KILL:
                psHasDiscard = gcvTRUE;
                break;

            case gcSL_TEXBIAS:
                /* fall through */
            case gcSL_TEXFETCH_MS:
                /* fall through */
            case gcSL_TEXLOD:
            case gcSL_TEXU:
            case gcSL_TEXU_LOD:
                if ((gctUINT) code->source0Index >= texAttrCount)
                {
                    /* Adjust texture attribute count. */
                    texAttrCount = code->source0Index + 1;
                }
                break;

            case gcSL_TEXGATHER:
                /* fall through */
            case gcSL_TEXGRAD:
                /* Need to get sampler number from next texld instruction. */
                {
                    /* Get next instruction. */
                    gcSL_INSTRUCTION nextCode = &Shader->code[curInstIdx + 1];
                    if (gcSL_isOpcodeTexld(gcmSL_OPCODE_GET(nextCode->opcode, Opcode)))
                    {
                        if ((gctUINT) nextCode->source0Index >= texAttrCount)
                        {
                            /* Adjust texture attribute count. */
                            texAttrCount = nextCode->source0Index + 1;
                        }
                    }
                }
                break;

            default:
                break;
            }
        }
        else
        {
            if ((gctUINT) code->tempIndex >= tempCount)
            {
                /* Adjust temporary register count. */
                tempCount = code->tempIndex + 1;
            }

            if ((gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP)
            &&  (code->source0Index >= tempCount)
            )
            {
                /* Adjust temporary register count. */
                tempCount = code->source0Index + 1;
            }

            if ((gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP)
            &&  (code->source1Index >= tempCount)
            )
            {
                /* Adjust temporary register count. */
                tempCount = code->source1Index + 1;
            }
        }
    }

    /***************************************************************************
    ** II - Allocate register arrays.
    */

    do
    {
        if (Shader->attributeCount > 0)
        {
            /* Allocate attribute array. */
            gcmERR_BREAK(gcoOS_Allocate(os,
                                        gcmSIZEOF(struct _gcLINKTREE_ATTRIBUTE) * Shader->attributeCount,
                                        &pointer));

            Tree->attributeArray = pointer;

            /* Initialize all attributes as dead and having no users. */
            for (i = 0; i < Shader->attributeCount; i++)
            {
                Tree->attributeArray[i].inUse   = gcvFALSE;
                Tree->attributeArray[i].lastUse = -1;
                Tree->attributeArray[i].users   = gcvNULL;
            }

            /* Set attribute count. */
            Tree->attributeCount = Shader->attributeCount;
        }

        if (tempCount > 0)
        {
            gcSL_FORMAT format = gcSL_FLOAT;

            if (Shader->type == gcSHADER_TYPE_CL)
            {
                format = gcSL_UINT32;
            }
#if _SUPPORT_LONG_ULONG_DATA_TYPE
            tempCount++;    /* add one more temp in case of last temp has 64bit format */
#endif
            /* Allocate temporary register array. */
            gcmERR_BREAK(gcoOS_Allocate(os,
                                        gcmSIZEOF(struct _gcLINKTREE_TEMP) *
                                        tempCount,
                                        &pointer));

            Tree->tempArray = pointer;

            /* Initialize all temporary registers as not defined. */
            for (i = 0; i < tempCount; i++)
            {
                _InitTreeTemp(&Tree->tempArray[i], i, format);

            }

            /* Set temporary register count. */
            Tree->tempCount = tempCount;

            /* Initialize variable. */
            if (Shader->variableCount > 0)
            {
                gctUINT variableCount = Shader->variableCount;

                for (i = 0; i < variableCount; i++)
                {
                    gcVARIABLE variable = Shader->variables[i];

                    if (isVariableNormal(variable))
                    {
                        gctUINT startIndex, endIndex;
                        gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvTRUE,
                                                          &startIndex, &endIndex);

                        if ((GetVariableKnownArraySize(variable) > 1) || gcmType_isMatrix(variable->u.type) ||
                            (endIndex - startIndex > 1))
                        {
                            gctUINT size = GetVariableKnownArraySize(variable) * gcmType_Rows(variable->u.type);
                            gctUINT j;

                            gcmASSERT(variable->tempIndex + size <= Tree->tempCount);
                            temp = Tree->tempArray + variable->tempIndex;
                            for (j = 0; j < size; j++, temp++)
                            {
                                temp->variable = variable;

#if !_USE_NEW_VARIABLE_HANDLER_
                                /* Mark entire register as non optimizable. */
                                temp->constUsage[0] =
                                temp->constUsage[1] =
                                temp->constUsage[2] =
                                temp->constUsage[3] = -1;
#endif
                            }
                        }
                        else {
                            gcmASSERT((gctSIZE_T)(variable->tempIndex + 1) <= Tree->tempCount);
                            temp = Tree->tempArray + variable->tempIndex;
                            temp->variable = variable;
                        }
                    }
                }
            }
        }

        if (Shader->outputCount > 0)
        {
            /* Allocate output array. */
            gcmERR_BREAK(gcoOS_Allocate(os,
                                        gcmSIZEOF(struct _gcLINKTREE_OUTPUT) *
                                            Shader->outputCount,
                                        &pointer));

            Tree->outputArray = pointer;

            /* Initialize all outputs as not being defined. */
            for (i = 0; i < Shader->outputCount; i++)
            {
                Tree->outputArray[i].inUse             = gcvFALSE;
                Tree->outputArray[i].isArray           = gcvFALSE;
                Tree->outputArray[i].isPacked          = gcvFALSE;
                Tree->outputArray[i].isTransformFeedback = gcvFALSE;
                Tree->outputArray[i].tempHolding       = -1;
                Tree->outputArray[i].fragmentAttribute = -1;
                Tree->outputArray[i].vsOutputIndex     = 0;
                Tree->outputArray[i].fragmentIndex     = 0;
                Tree->outputArray[i].fragmentIndexEnd  = 0;
                Tree->outputArray[i].skippedFragmentAttributeRows  = 0;
            }

            /* Copy number of outputs. */
            Tree->outputCount = Shader->outputCount;
        }

        if (texAttrCount > 0)
        {
            /* Allocate texture attribute array. */
            gcmERR_BREAK(gcoOS_Allocate(os,
                                        gcmSIZEOF(gcsLINKTREE_LIST_PTR) *
                                            texAttrCount,
                                        &pointer));

            texAttr = pointer;

            for (i = 0; i < texAttrCount; ++i)
            {
                texAttr[i] = gcvNULL;
            }
        }

        if (Shader->codeCount > 0)
        {
            /* Allocate hints. */
            gcmERR_BREAK(gcoOS_Allocate(os,
                                        gcmSIZEOF(gcsCODE_HINT) *
                                        Shader->codeCount,
                                        &pointer));

            Tree->hints = pointer;
            gcoOS_ZeroMemory(Tree->hints,
                             gcmSIZEOF(gcsCODE_HINT));
            Tree->hints->psHasDiscard = psHasDiscard;

            /* Reset all hints. */
            for (curInstIdx = 0; curInstIdx < Tree->shader->codeCount; ++curInstIdx)
            {
                _InitTreeHints(Tree->hints, curInstIdx);
            }
        }
    }
    while (gcvFALSE);

    if (gcmIS_ERROR(status))
    {
        /* Error. */
        return status;
    }

    /***************************************************************************
    ** III - Determine dependency and users of all registers.
    */

    /* Walk all instructions. */
    for (curInstIdx = 0; curInstIdx < Shader->codeCount; curInstIdx++)
    {
        gctBOOL isTexAttr  = gcvFALSE;
        gctBOOL isTexAttr2 = gcvFALSE;
        gctUINT samplerNum = 0xFF;
        gcsCODE_CALLER_PTR caller;

        /* Determine ownership of the code for functions. */
        for (f = 0; f < Shader->functionCount; ++f)
        {
            gcFUNCTION function = Shader->functions[f];

            if ((curInstIdx >= function->codeStart)
            &&  (curInstIdx < function->codeStart + function->codeCount)
            )
            {
                Tree->hints[curInstIdx].owner = function;
                Tree->hints[curInstIdx].isOwnerKernel = gcvFALSE;
                break;
            }
        }

        /* Determine ownership of the code for kernel functions. */
        for (f = 0; f < Shader->kernelFunctionCount; ++f)
        {
            gcKERNEL_FUNCTION kernelFunction = Shader->kernelFunctions[f];

            if (kernelFunction->isMain)
            {
                continue;
            }

            if ((curInstIdx >= kernelFunction->codeStart)
            &&  (curInstIdx < kernelFunction->codeEnd)
            )
            {
                Tree->hints[curInstIdx].owner = kernelFunction;
                Tree->hints[curInstIdx].isOwnerKernel = gcvTRUE;
                break;
            }
        }

        if (Tree->hints[curInstIdx].owner == gcvNULL)
        {
            Tree->hints[curInstIdx].callNest = 0;
        }

        /* Set PC. */
        pc = (Flags & gcvSHADER_RESOURCE_USAGE) ? curInstIdx : (gctSIZE_T) -1;

        /* Get instruction. */
        code = &Shader->code[curInstIdx];
        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

        /* Dispatch on opcode. */
        switch (opcode)
        {
        case gcSL_CALL:
            /* fall through */
        case gcSL_JMP:
            if (code->tempIndex < Shader->codeCount)
            {
                gctUINT    targetInstIdx = code->tempIndex;
                /* Allocate a gcsCODE_CALLER structure. */
                gcmERR_BREAK(gcoOS_Allocate(os,
                                            gcmSIZEOF(gcsCODE_CALLER),
                                            &pointer));

                caller = pointer;

                /* Add the current caller to the list of callees. */
                caller->caller = curInstIdx;
                caller->next   = Tree->hints[targetInstIdx].callers;

                Tree->hints[targetInstIdx].callers = caller;

                /* Add last use for temp register used in code gen if it is back jump. */
                if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_JMP && targetInstIdx < curInstIdx)
                {
                    gctSIZE_T inst;

                    /* mark loop header and back jump */
                    Tree->hints[curInstIdx].isBackJump = gcvTRUE;
                    Tree->hints[targetInstIdx].isLoopHeader = gcvTRUE;

                    for (inst = targetInstIdx; inst < curInstIdx; inst++)
                    {
                        if (Tree->hints[inst].lastUseForTemp < (gctINT) curInstIdx)
                        {
                            Tree->hints[inst].lastUseForTemp = (gctINT) curInstIdx;
                        }
                        else
                        {
                            gcmASSERT(Tree->hints[inst].lastUseForTemp < (gctINT) curInstIdx);
                        }
                    }
                }
                if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_CALL)
                {
                    Tree->hints[curInstIdx].isCall = gcvTRUE;
                }
            }
            /* fall through */

        default:
            if (gcSL_isOpcodeHaveNoTarget(opcode))
            {
                temp = gcvNULL;
                if (gcSL_isOpcodeTexldModifier(opcode))
                {
                    if (opcode == gcSL_TEXGATHER || opcode == gcSL_TEXGRAD)
                    {
                        /* Get sample number. */
                        gcSL_INSTRUCTION nextCode = &Shader->code[curInstIdx + 1];
                        if (gcSL_isOpcodeTexld(gcmSL_OPCODE_GET(nextCode->opcode, Opcode)))
                        {
                            samplerNum = nextCode->source0Index;
                        }
                        else
                        {
                            gcmASSERT(0);
                        }
                        isTexAttr2 = gcvTRUE;
                    }
                    else
                    {
                        isTexAttr = gcvTRUE;
                    }
                }
                break;
            }

            /* Get gcSL_TARGET field. */
            target = code->temp;

            /* Get pointer to temporary register. */
            temp = &Tree->tempArray[code->tempIndex];
            paired64BitUpperTemp = gcvNULL;

            if (!gcSL_isOpcodeUseTargetAsSource(opcode))
            {
                gcSL_FORMAT format = gcmSL_TARGET_GET(target, Format);

#if _SUPPORT_LONG_ULONG_DATA_TYPE
                if (isFormat64bit(format))
                {
                    /* Not real target for STORE1 */
                    if(opcode == gcSL_STORE1) break;
                    gcmASSERT((gctUINT)(code->tempIndex+1) < Tree->tempCount);
                    paired64BitUpperTemp = &Tree->tempArray[code->tempIndex + 1];
                    paired64BitUpperTemp->isPaired64BitUpper = gcvTRUE;
                }
#endif
                /* Add instruction to definitions. */
                gcLINKTREE_AddList(Tree, &temp->defined, gcSL_NONE, curInstIdx);

                /* Set register format. */
                if (format == gcSL_FLOAT16 &&
                    gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_CONV)
                {
                    gcmASSERT(gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_LOAD ||
                              gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_STORE ||
                              gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_STORE1 ||
                              gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_CMP ||
                              gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MOV);

                    format = gcSL_FLOAT;
                }

                 /* Cannot enforce consistent format for each register
                  * due to math built-in functions and unions.
                  */
                temp->format = format;

                if (paired64BitUpperTemp)
                {
                    gcLINKTREE_AddList(Tree, &paired64BitUpperTemp->defined, gcSL_NONE, curInstIdx);
                    paired64BitUpperTemp->format = format;
                }
            }
            else
            {
                /* Target is actually a source. */
                gcmERR_BREAK(_TempSource(Tree, code->tempIndex, -1, curInstIdx));
            }

            /*
            ** If this is a IMAGE_RD, we need to mark all channel enabled,
            ** because the enabled is controlled by image info.
            */
            if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_IMAGE_RD ||
                gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_IMAGE_RD_3D ||
                gcSL_isOpcodeTexld(gcmSL_OPCODE_GET(code->opcode, Opcode)))
            {
                temp->usage = gcSL_ENABLE_XYZW;
                if (paired64BitUpperTemp)
                {
                    paired64BitUpperTemp->usage = gcSL_ENABLE_XYZW;
                }
            }
            else
            {
                /* Update register usage. */
                temp->usage |= gcmSL_TARGET_GET(target, Enable);
                if (paired64BitUpperTemp)
                {
                    paired64BitUpperTemp->usage |= gcmSL_TARGET_GET(target, Enable);
                }
            }

            /* If the target is indexed temp register, mark all array temp registers as target. */
            if (gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED)
            {
                index = code->tempIndexed;

                /* Add index usage. */
                gcmERR_BREAK(_IndexedSource(Tree,
                                            gcmSL_INDEX_GET(index, Index),
                                            pc,
                                            (temp == gcvNULL)
                                            ? -1
                                            : code->tempIndex));

                Tree->tempArray[code->tempIndex].isIndexing = gcvTRUE;

                if (Tree->tempArray[code->tempIndex].variable)
                {
                    gcVARIABLE variable = Tree->tempArray[code->tempIndex].variable;

                    gctUINT startIndex, endIndex, j;
                    gcmASSERT(isVariableNormal(variable));
                    gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvFALSE,
                                                      &startIndex, &endIndex);
                    gcmASSERT(startIndex == variable->tempIndex);

                    for (j = startIndex + 1; j < endIndex; j++)
                    {
                        /* Get pointer to temporary register. */
                        gcLINKTREE_TEMP tempA = &Tree->tempArray[j];

                        tempA->isIndexing = gcvTRUE;

                        /* Add instruction to definitions. */
                        gcLINKTREE_AddList(Tree, &tempA->defined, gcSL_NONE, curInstIdx);

                        /* Update register usage. */
                        tempA->usage |= gcmSL_TARGET_GET(target, Enable);

                        gcmERR_BREAK(_IndexedSource(Tree,
                                                    gcmSL_INDEX_GET(index, Index),
                                                    pc,
                                                    (tempA == gcvNULL)
                                                    ? -1
                                                    : (gctINT)j));
                    }
                }
                else
                {
                    /* This temp register should be an array function argument. */
                }
            }
            break;
        }

        /* Determine usage of source0. */
        source = code->source0;

        switch (gcmSL_SOURCE_GET(source, Type))
        {
        case gcSL_ATTRIBUTE:
            /* Source is an attribute. */
            if (!isTexAttr2)
            {
                gcmERR_BREAK(_AttributeSource(Tree,
                                              code,
                                              0,
                                              code->source0Index,
                                              (temp == gcvNULL)
                                                  ? -1
                                                  : code->tempIndex,
                                              pc));
            }
            else
            {
                gcmERR_BREAK(gcLINKTREE_AddList(Tree,
                                                &texAttr[samplerNum],
                                                gcSL_ATTRIBUTE,
                                                code->source0Index));
            }
            break;

        case gcSL_TEMP:
            /* Source is a temporary register. */
            if (!isTexAttr2)
            {
                gcmERR_BREAK(_TempSource(Tree,
                                         code->source0Index,
                                         (temp == gcvNULL) ? -1 : code->tempIndex,
                                         pc));
            }
            else
            {
                gcmERR_BREAK(gcLINKTREE_AddList(Tree,
                                                &texAttr[samplerNum],
                                                gcSL_TEMP,
                                                code->source0Index));
            }

            /* If the target is indexed temp register, mark all array temp registers as target. */
            if ((temp != gcvNULL)
            &&  (gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED)
            &&  (Tree->tempArray[code->tempIndex].variable)
            )
            {
                gcVARIABLE variable = Tree->tempArray[code->tempIndex].variable;

                gctUINT startIndex, endIndex, j;
                gcmASSERT(isVariableNormal(variable));
                gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvFALSE,
                                                  &startIndex, &endIndex);
                gcmASSERT(startIndex == variable->tempIndex);

                Tree->tempArray[code->tempIndex].isIndexing = gcvTRUE;

                /* Source is a temporary register array. */
                for (j = startIndex + 1; j < endIndex; j++)
                {
                    gcLINKTREE_TEMP tempA = &Tree->tempArray[j];
                    tempA->isIndexing = gcvTRUE;

                    gcmERR_BREAK(_TempSource(Tree,
                                             code->source0Index,
                                             j,
                                             pc));
                }
            }
            break;
        }

        /* Break on error. */
        if (gcmIS_ERROR(status))
        {
            break;
        }

        switch (gcmSL_SOURCE_GET(source, Indexed))
        {
        case gcSL_INDEXED_X:
            /* fall through */
        case gcSL_INDEXED_Y:
            /* fall through */
        case gcSL_INDEXED_Z:
            /* fall through */
        case gcSL_INDEXED_W:
            index = code->source0Indexed;

            /* Add index usage for source 0. */
            gcmERR_BREAK(_IndexedSource(Tree,
                                        gcmSL_INDEX_GET(index, Index),
                                        pc,
                                        (temp == gcvNULL)
                                            ? -1
                                            : code->tempIndex));

            /* If the source is temp register, mark all array temp registers as inputs. */
            if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
            {
                if (Tree->tempArray[code->source0Index].variable)
                {
                    gcVARIABLE variable = Tree->tempArray[code->source0Index].variable;

                    gctUINT startIndex, endIndex, j;
                    gcmASSERT(isVariableNormal(variable));
                    gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvFALSE,
                                                      &startIndex, &endIndex);
                    gcmASSERT(startIndex == variable->tempIndex);

                    /* Source is a temporary register array. */
                    for (j = startIndex; j < endIndex; j++)
                    {
                        gcLINKTREE_TEMP tempA = &Tree->tempArray[j];
                        tempA->isIndexing = gcvTRUE;

                        if (temp == gcvNULL)
                        {
                            gcmERR_BREAK(_TempSource(Tree,
                                                     j,
                                                     -1,
                                                     pc));
                        }
                        else if (gcmSL_TARGET_GET(target, Indexed) == gcSL_NOT_INDEXED ||
                                 Tree->tempArray[code->tempIndex].variable == gcvNULL)
                        {
                            gcmERR_BREAK(_TempSource(Tree,
                                                     j,
                                                     code->tempIndex,
                                                     pc));
                        }
                        else
                        {
                            gcVARIABLE variableT = Tree->tempArray[code->tempIndex].variable;

                            gctUINT startIndexT, endIndexT, k;
                            gcmASSERT(isVariableNormal(variable));
                            gcSHADER_GetVariableIndexingRange(Tree->shader, variableT, gcvFALSE,
                                                              &startIndexT, &endIndexT);
                            gcmASSERT(startIndexT == variableT->tempIndex);

                            /* Source is a temporary register array. */
                            for (k = startIndexT; k < endIndexT; k++)
                            {
                                gcLINKTREE_TEMP tempA = &Tree->tempArray[k];
                                tempA->isIndexing = gcvTRUE;

                                gcmERR_BREAK(_TempSource(Tree,
                                                         j,
                                                         k,
                                                         pc));
                            }
                        }
                    }
                }
                else
                {
                    gcmASSERT(Tree->tempArray[code->source0Index].variable == gcvNULL);
                }
            }

            if (gcmOPT_UseVIRCodeGen() != VIRCG_None)
            {
                if (Tree->tempArray[code->source0Indexed].defined == gcvNULL)
                {
                    if (Tree->tempArray[code->source0Indexed].variable &&
                        (Tree->tempArray[code->source0Indexed].variable->nameLength == gcSL_VERTEX_ID ||
                         Tree->tempArray[code->source0Indexed].variable->nameLength == gcSL_INSTANCE_ID))
                    {
                        Tree->tempArray[code->source0Indexed].format = gcSL_INTEGER;
                    }
                }
            }
        }

        /* Break on error. */
        if (gcmIS_ERROR(status))
        {
            break;
        }

        /* Special case for TEXLD. */
        if (gcSL_isOpcodeTexld(gcmSL_OPCODE_GET(code->opcode, Opcode)) &&
            (texAttrCount > 0) &&
            (code->source0Index < texAttrCount)
        )
        {
            gcsLINKTREE_LIST_PTR list;

            /* Add any texture attribute dependencies. */
            gcmASSERT(temp != gcvNULL);
            gcmASSERT(gcmSL_SOURCE_GET(source, Type) == gcSL_UNIFORM ||
                      gcmSL_SOURCE_GET(source, Type) == gcSL_SAMPLER);

            for (list = texAttr[code->source0Index]; list != gcvNULL; list = list->next)
            {
                if (list->type == gcSL_ATTRIBUTE)
                {
                    gcmERR_BREAK(_AttributeSource(Tree,
                                                  gcvNULL,
                                                  0,
                                                  (gctUINT16)list->index,
                                                  code->tempIndex,
                                                  pc));
                }
                else if (list->type == gcSL_TEMP)
                {
                    gcmERR_BREAK(_TempSource(Tree,
                                             list->index,
                                             code->tempIndex,
                                             pc));
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }
            }

            /* Clean up texAttr at current sampler# */
            _Delete(Tree, &texAttr[code->source0Index]);
            texAttr[code->source0Index] = gcvNULL;
        }

        /* Determine usage of source1. */
        source = code->source1;

        switch (gcmSL_SOURCE_GET(source, Type))
        {
        case gcSL_ATTRIBUTE:
            /* Source is an attribute. */

            if (!isTexAttr && !isTexAttr2)
            {
                gcmERR_BREAK(_AttributeSource(Tree,
                                              code,
                                              1,
                                              code->source1Index,
                                              (temp == gcvNULL)
                                                  ? -1
                                                  : code->tempIndex,
                                              pc));
            }
            else if (isTexAttr)
            {
                gcmERR_BREAK(gcLINKTREE_AddList(Tree,
                                                &texAttr[code->source0Index],
                                                gcSL_ATTRIBUTE,
                                                code->source1Index));
            }
            else
            {
                gcmERR_BREAK(gcLINKTREE_AddList(Tree,
                                                &texAttr[samplerNum],
                                                gcSL_ATTRIBUTE,
                                                code->source1Index));
            }

            break;

        case gcSL_TEMP:
            /* Source is a temporary register. */
            if (!isTexAttr && !isTexAttr2)
            {
                gcmERR_BREAK(_TempSource(Tree,
                                         code->source1Index,
                                         (temp == gcvNULL) ? -1 : code->tempIndex,
                                         pc));

                /* If the target is indexed temp register, mark all array temp registers as target. */
                if (temp != gcvNULL && gcmSL_TARGET_GET(target, Indexed) != gcSL_NOT_INDEXED &&
                    Tree->tempArray[code->tempIndex].variable)
                {
                    gcVARIABLE variable = Tree->tempArray[code->tempIndex].variable;

                    gctUINT startIndex, endIndex, j;
                    gcmASSERT(isVariableNormal(variable));
                    gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvFALSE,
                                                      &startIndex, &endIndex);
                    gcmASSERT(startIndex == variable->tempIndex);

                    Tree->tempArray[code->tempIndex].isIndexing = gcvTRUE;

                    /* Source is a temporary register array. */
                    for (j = startIndex + 1; j < endIndex; j++)
                    {
                        gcLINKTREE_TEMP tempA = &Tree->tempArray[j];
                        tempA->isIndexing = gcvTRUE;

                        gcmERR_BREAK(_TempSource(Tree,
                                                 code->source1Index,
                                                 j,
                                                 pc));
                    }
                }
            }
            else if (isTexAttr)
            {
                gcmERR_BREAK(gcLINKTREE_AddList(Tree,
                                                &texAttr[code->source0Index],
                                                gcSL_TEMP,
                                                code->source1Index));
            }
            else
            {
                gcmERR_BREAK(gcLINKTREE_AddList(Tree,
                                                &texAttr[samplerNum],
                                                gcSL_TEMP,
                                                code->source1Index));
            }
            break;
        default:
            break;
        }

        /* Break on error. */
        if (gcmIS_ERROR(status))
        {
            break;
        }

        switch (gcmSL_SOURCE_GET(source, Indexed))
        {
        case gcSL_INDEXED_X:
            /* fall through */
        case gcSL_INDEXED_Y:
            /* fall through */
        case gcSL_INDEXED_Z:
            /* fall through */
        case gcSL_INDEXED_W:
            index = code->source1Indexed;

            /* Add index usage for source 1. */
            gcmERR_BREAK(_IndexedSource(Tree,
                                        gcmSL_INDEX_GET(index, Index),
                                        pc,
                                        (temp == gcvNULL)
                                            ? -1
                                            : code->tempIndex));

            /* If the source is temp register, mark all array temp registers as inputs. */
            if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP)
            {
                if (Tree->tempArray[code->source1Index].variable)
                {
                    gcVARIABLE variable = Tree->tempArray[code->source1Index].variable;

                    gctUINT startIndex, endIndex, j;
                    gcmASSERT(isVariableNormal(variable));
                    gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvFALSE,
                                                      &startIndex, &endIndex);
                    gcmASSERT(startIndex == variable->tempIndex);

                    /* Source is a temporary register array. */
                    for (j = startIndex; j < endIndex; j++)
                    {
                        gcLINKTREE_TEMP tempA = &Tree->tempArray[j];
                        tempA->isIndexing = gcvTRUE;

                        if (temp == gcvNULL)
                        {
                            gcmERR_BREAK(_TempSource(Tree,
                                                     j,
                                                     -1,
                                                     pc));
                        }
                        else if (gcmSL_TARGET_GET(target, Indexed) == gcSL_NOT_INDEXED ||
                                 Tree->tempArray[code->tempIndex].variable == gcvNULL)
                        {
                            gcmERR_BREAK(_TempSource(Tree,
                                                     j,
                                                     code->tempIndex,
                                                     pc));
                        }
                        else
                        {
                            gcVARIABLE variableT = Tree->tempArray[code->tempIndex].variable;

                            gctUINT startIndexT, endIndexT, k;
                            gcmASSERT(isVariableNormal(variable));
                            gcSHADER_GetVariableIndexingRange(Tree->shader, variableT, gcvFALSE,
                                                              &startIndexT, &endIndexT);
                            gcmASSERT(startIndexT == variableT->tempIndex);

                            /* Source is a temporary register array. */
                            for (k = startIndexT; k < endIndexT; k++)
                            {
                                gcLINKTREE_TEMP tempA = &Tree->tempArray[k];
                                tempA->isIndexing = gcvTRUE;

                                gcmERR_BREAK(_TempSource(Tree,
                                                         j,
                                                         k,
                                                         pc));
                            }
                        }
                    }
                }
                else
                {
                    gcmASSERT(Tree->tempArray[code->source1Index].variable);
                }
            }

            if (gcmOPT_UseVIRCodeGen() != VIRCG_None)
            {
                if (Tree->tempArray[code->source1Indexed].defined == gcvNULL)
                {
                    if (Tree->tempArray[code->source1Indexed].variable &&
                        (Tree->tempArray[code->source1Indexed].variable->nameLength == gcSL_VERTEX_ID ||
                         Tree->tempArray[code->source1Indexed].variable->nameLength == gcSL_INSTANCE_ID))
                    {
                        Tree->tempArray[code->source1Indexed].format = gcSL_INTEGER;
                    }
                }
            }

            break;

        default:
            break;
        }

        /* Break on error. */
        if (gcmIS_ERROR(status))
        {
            break;
        }
    }

    /* Update last use for temp register used in code gen. */
    /* For each function, all its statements are set to the last call. */
    for (i = 0; i < Shader->functionCount + Shader->kernelFunctionCount; ++i)
    {
        gctUINT codeStart, codeEnd, ic;
        gctINT lastUse = -1;
        gcsCODE_CALLER_PTR caller;

        if (i < Shader->functionCount) {
                gcFUNCTION function = Shader->functions[i];
                codeStart = function->codeStart;
                codeEnd   = codeStart + function->codeCount;
        } else {
            gcKERNEL_FUNCTION kernelFunction = Shader->kernelFunctions[ i - Shader->functionCount ];
            codeStart = kernelFunction->codeStart;
            codeEnd   = kernelFunction->codeEnd;
        }

        for (caller = Tree->hints[codeStart].callers;
             caller != gcvNULL;
             caller = caller->next)
        {
            if (Tree->hints[caller->caller].lastUseForTemp > lastUse)
            {
                lastUse = Tree->hints[caller->caller].lastUseForTemp;
            }
        }

        if (lastUse >= 0)
        {
            for (ic = codeStart; ic < codeEnd; ic++)
            {
                Tree->hints[ic].lastUseForTemp = lastUse;
            }
        }
    }

    /* Determine call nesting level for functions and kernel functions. */
    for (nest = 1, modified = gcvTRUE; modified; ++nest)
    {
        modified = gcvFALSE;

        for (i = 0; i < Shader->functionCount + Shader->kernelFunctionCount; ++i)
        {
            gctUINT codeStart, codeCount;
            gcsCODE_CALLER_PTR caller;

            if (i < Shader->functionCount)
            {
                gcFUNCTION function = Shader->functions[i];
                codeStart = function->codeStart;
                codeCount = function->codeCount;
            }
            else
            {
                gcKERNEL_FUNCTION kernelFunction = Shader->kernelFunctions[ i - Shader->functionCount ];
                codeStart = kernelFunction->codeStart;
                codeCount = kernelFunction->codeEnd - kernelFunction->codeStart;
            }
            if (Tree->hints != gcvNULL)
            {
                if (Tree->hints[codeStart].callNest > 0)
                {
                    continue;
                }

                for (caller = Tree->hints[codeStart].callers;
                     (caller != gcvNULL);
                     caller = caller->next)
                {
                    if (Tree->hints[caller->caller].callNest == nest - 1)
                    {
                        gctSIZE_T j;

                        for (j = 0; j < codeCount; ++j)
                        {
                            Tree->hints[codeStart + j].callNest = nest;
                        }

                        modified = gcvTRUE;
                        break;
                    }
                }
            }
        }

        if (!modified)
        {
            break;
        }
    }

    /***************************************************************************
    ** IV - Add output dependencies.
    */

    if (gcmNO_ERROR(status))
    {
        /* Loop through all outputs. */
        for (i = 0; i < Shader->outputCount; i++)
        {
            gcOUTPUT output;
            gcLINKTREE_TEMP tempOutput;
            gctUINT size, regIdx, endRegIdx;

            /* Get gcOUTPUT pointer. */
            output = Shader->outputs[i];

            if (output != gcvNULL)
            {
                gctUINT16 tempIndex = output->tempIndex;

                if (output->nameLength == gcSL_POINT_COORD)
                {
                    /* The output is just a placeholder.  Use gl_Position's tempIndex. */
                    gctSIZE_T j;

                    for (j = 0; j < Shader->outputCount; j++)
                    {
                        if (Shader->outputs[j]->nameLength == gcSL_POSITION)
                        {
                            tempIndex = Shader->outputs[j]->tempIndex;
                            break;
                        }
                    }

                    gcmASSERT(j < Shader->outputCount);
                }

                /* If the temp reg index is larger than the max temp reg,
                ** then this output is no used.
                */
                if (tempIndex >= Tree->tempCount)
                    continue;

                /* Copy temporary register to output. */
                Tree->outputArray[i].tempHolding = tempIndex;

                /* we expand output array to elements of VS, but for fragment shader, */
                /* we consider it as entity. We should refine it to match at two sides later! */
                size = 1; /*output->arraySize;*/

                if (gcmType_isMatrix(output->type)) size *= gcmType_Rows(output->type);
                endRegIdx = tempIndex + size;

                for (regIdx = tempIndex; regIdx < endRegIdx; regIdx ++)
                {
                    /* Get temporary register defining this output. */
                    tempOutput = &Tree->tempArray[regIdx];

                    /* Make sure the temp is marked as an output. */
                    tempOutput->lastUse = -1;

                    /* Add output to users of temporary register. */
                    gcmERR_BREAK(gcLINKTREE_AddList(Tree,
                                                    &tempOutput->users,
                                                    gcSL_OUTPUT,
                                                    i));
                }
            }
        }
    }

    if (texAttr != gcvNULL)
    {
        /* Free each dependency list in the texture attribute array. */
        for (i = 0; i < texAttrCount; ++i)
        {
            if (texAttr[i] != gcvNULL)
            {
                _Delete(Tree, &texAttr[i]);
            }
        }

        /* Free the texture attribute array. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(os, texAttr));
    }

    /***************************************************************************
    ****************************************************** Function Arguments **
    ***************************************************************************/

    for (i = 0; i < Shader->functionCount; ++i)
    {
        gcFUNCTION function = Shader->functions[i];
        gctSIZE_T j;

        for (j = 0; j < function->argumentCount; ++j)
        {
            gctINT argIndex = function->arguments[j].index;

            gcmASSERT(Tree->tempArray[argIndex].owner == gcvNULL ||
                      gcShaderHwRegAllocated(Shader));
            Tree->tempArray[argIndex].owner = function;
            Tree->tempArray[argIndex].isOwnerKernel = gcvFALSE;

            Tree->tempArray[argIndex].inputOrOutput = function->arguments[j].qualifier;
        }
    }

    /***************************************************************************
    *********************************************** Kernel Function Arguments **
    ***************************************************************************/

    for (i = 0; i < Shader->kernelFunctionCount; ++i)
    {
        gcKERNEL_FUNCTION kernelFunction = Shader->kernelFunctions[i];
        gctSIZE_T j;

        for (j = 0; j < kernelFunction->argumentCount; ++j)
        {
            gctINT argIndex = kernelFunction->arguments[j].index;

            gcmASSERT(Tree->tempArray[argIndex].owner == gcvNULL ||
                      gcShaderHwRegAllocated(Shader));
            Tree->tempArray[argIndex].owner = kernelFunction;
            Tree->tempArray[argIndex].isOwnerKernel = gcvTRUE;

            Tree->tempArray[argIndex].inputOrOutput = kernelFunction->arguments[j].qualifier;
        }
    }

    /***************************************************************************
    *********************************** Temporaries that are used beyond CALL **
    ***************************************************************************/

    if (Flags & gcvSHADER_RESOURCE_USAGE)
    {
        /* For each attribute, if it is used inside a function, update the
           last usage to the last callee of that function.  Or, if the
           attribute is used inside a loop, update the last usage to the end
           of the loop. */
        for (i = 0; i < Tree->attributeCount; ++i)
        {
            gcsLINKTREE_LIST_PTR user;
            gctINT lastUse = -1;

            for (user = Tree->attributeArray[i].users;
                 user != gcvNULL;
                 user = user->next)
            {
                /* Does the user belongs to a function? */
                if(Tree->hints != gcvNULL){
                    if (Tree->hints[user->index].owner != gcvNULL)
                    {
                        /* Find the last callee of the function inside main(). */
                        _FindCallers(Tree,
                                     Tree->hints[user->index].owner,
                                     Tree->hints[user->index].isOwnerKernel,
                                     0,
                                     &lastUse);
                    }
                    else
                    {
                        gctINT userIndex;
                        gcsCODE_CALLER_PTR caller;

                        if (user->index > lastUse)
                        {
                            lastUse = user->index;
                        }

                        /* Walk the code backwards to find a JMP from beyond this
                        user. */
                        for (userIndex = user->index; userIndex >= 0; --userIndex)
                        {
                            if (Tree->hints[userIndex].owner != gcvNULL)
                            {
                                if (Tree->hints[userIndex].isOwnerKernel) {
                                    index = (gctUINT16) ((gcKERNEL_FUNCTION)Tree->hints[userIndex].owner)->codeStart;
                                } else {
                                    index = (gctUINT16) ((gcFUNCTION)Tree->hints[userIndex].owner)->codeStart;
                                }
                                continue;
                            }

                            for (caller = Tree->hints[userIndex].callers;
                                 caller != gcvNULL;
                                 caller = caller->next)
                            {
                                if ((Tree->hints[caller->caller].owner == gcvNULL)
                                &&  (caller->caller > user->index)
                                &&  (caller->caller > lastUse)
                                )
                                {
                                    lastUse = caller->caller;
                                }
                            }
                        }
                    }
                }
            }

            Tree->attributeArray[i].lastUse = lastUse;
        }

        /* For each temp, if it is used both inside and outside a function, update
           the last usage to the last callee of that function in the outermost
           usage.  Or, if the temp is used inside a loop, update the last usage to
           the end of the loop. */
        for (i = 0; i < Tree->tempCount; ++i)
        {
            gcLINKTREE_TEMP tempReg = &Tree->tempArray[i];
            gcsLINKTREE_LIST_PTR define, user;
            gctINT lastUse = -1, lastNest = 0;
            gctPOINTER lastOwner = gcvNULL;
            gctBOOL isLastOwnerKernel = gcvFALSE;
            gctINT firstDefined = 0x7fffffff;

            if (tempReg->defined == gcvNULL)
            {
                continue;
            }

            /* all definition and its uses of the temp are in the same basic block,
               do not change the last use even if they are inside a loop */
            if (gctOPT_hasFeature(FB_LIVERANGE_FIX1) &&
                _defineAndUserInSameEBB(Tree, tempReg))
            {
                continue;
            }

            for (define = tempReg->defined;
                 define != gcvNULL;
                 define = define->next)
            {
                /* Get the index of first define. */
                if (define->index < firstDefined)
                    firstDefined = define->index;

                if (Tree->hints != gcvNULL)
                {
                    if (((lastUse == -1)
                    ||  ((Tree->hints[define->index].owner == lastOwner)
                        && (define->index > lastUse)
                        )
                    ||  (Tree->hints[define->index].callNest < lastNest)
                    )
                    && (Tree->hints[define->index].callNest != -1)
                    )
                    {
                        lastUse   = define->index;
                        lastNest  = Tree->hints[lastUse].callNest;
                        lastOwner = Tree->hints[lastUse].owner;
                        isLastOwnerKernel = Tree->hints[lastUse].isOwnerKernel;
                    }
                }
            }

            if (lastNest > 0)
            {
                /* Check if lastOwner has multiple callers directly or indirectly. */
                gctINT upperUse       = lastUse;
                gctINT upperNest      = lastNest;
                gctPOINTER upperOwner = lastOwner;
                gctBOOL isUpperOwnerKernel = isLastOwnerKernel;
                gcsCODE_CALLER_PTR nextCaller;

                while (upperNest > 0)
                {
                    _FindCallers(Tree,
                                 upperOwner,
                                 isUpperOwnerKernel,
                                 upperNest - 1,
                                 &upperUse);

                    upperNest  = Tree->hints[upperUse].callNest;
                    if(Tree->hints != gcvNULL){
                        if (isUpperOwnerKernel) {
                            nextCaller = Tree->hints[((gcKERNEL_FUNCTION)upperOwner)->codeStart].callers->next;
                        } else {
                            nextCaller = Tree->hints[((gcFUNCTION)upperOwner)->codeStart].callers->next;
                        }
                        if (nextCaller)
                        {
                            /* Multiple callers. */
                            lastUse   = upperUse;
                            lastNest  = upperNest;
                            lastOwner = upperOwner = Tree->hints[upperUse].owner;
                            isLastOwnerKernel = isUpperOwnerKernel = Tree->hints[upperUse].isOwnerKernel;
                        }
                        else
                        {
                            upperOwner = Tree->hints[upperUse].owner;
                            isUpperOwnerKernel = Tree->hints[upperUse].isOwnerKernel;
                        }
                    }
                }
            }

            for (user = tempReg->users; user != gcvNULL; user = user->next)
            {
                if (user->type == gcSL_OUTPUT)
                {
                    lastUse = -1;
                    break;
                }

                if (user->type != gcSL_NONE)
                {
                    lastUse = -1;
                    break;
                }
                if(Tree->hints != gcvNULL)
                {
                    if (Tree->hints[user->index].callNest > lastNest)
                    {
                        _FindCallers(Tree,
                                     Tree->hints[user->index].owner,
                                     Tree->hints[user->index].isOwnerKernel,
                                     lastNest,
                                     &lastUse);
                    }
                    else if (Tree->hints[user->index].callNest < lastNest)
                    {
                        /* Caller of lastOwner or temp is global variable. */
                        lastUse = user->index;
                    }
                    else if (Tree->hints[user->index].owner == lastOwner)
                    {
                        gctINT userIndex;

                        if (user->index > lastUse)
                        {
                            lastUse = user->index;
                        }

                        for (userIndex = user->index; userIndex >= 0; --userIndex)
                        {
                            gcsCODE_CALLER_PTR caller;

                            if (Tree->hints[userIndex].owner != lastOwner)
                            {
                                if (lastOwner != gcvNULL)
                                {
                                    break;
                                }
                                if (Tree->hints[userIndex].isOwnerKernel)
                                {
                                    userIndex = (gctUINT16) ((gcKERNEL_FUNCTION)Tree->hints[userIndex].owner)->codeStart;
                                }
                                else
                                {
                                    userIndex = (gctUINT16) ((gcFUNCTION)Tree->hints[userIndex].owner)->codeStart;
                                }
                                continue;
                            }

                            for (caller = Tree->hints[userIndex].callers;
                                 caller != gcvNULL;
                                 caller = caller->next)
                            {
                                if ((Tree->hints[caller->caller].owner == lastOwner)
                                &&  (caller->caller > user->index)
                                &&  (caller->caller > lastUse)
                                )
                                {
                                    if (userIndex < firstDefined && (Tree->shader->code + caller->caller)->opcode == gcSL_JMP)
                                        continue;
                                    else
                                        lastUse = caller->caller;
                                }
                            }
                        }
                    }
                }
            }
            _updateTempLastUse(Tree, tempReg, lastUse);
        }

        /* If there is another function call between a function argument assignment and its caller,
        ** we need to add the argument register to the live list of another function.
        */
        for (i = 0; i < Tree->tempCount; i++)
        {
            gcLINKTREE_TEMP tempReg = &Tree->tempArray[i];
            gcsLINKTREE_LIST_PTR user;
            gctINT funcIndex;
            gctUINT funcHead;

            /* Skip non function input registers. */
            if (tempReg->inputOrOutput != gcvFUNCTION_INPUT)
                continue;

            _GetFunctionByArgumentIndex(Tree, tempReg->index, gcvFUNCTION_INPUT, &funcIndex, &funcHead);
            gcmASSERT(funcIndex != -1);

            user = tempReg->defined;

            for (; user != gcvNULL; user = user->next)
            {
                gctINT instIndex = user->index + 1;

                /* Get the nearest call instruction index. */
                while (instIndex < (gctINT)Tree->shader->codeCount &&
                    Tree->shader->code[instIndex].opcode != gcSL_CALL)
                {
                    instIndex++;
                }

                if (instIndex >= (gctINT)Tree->shader->codeCount ||
                    Tree->shader->code[instIndex].tempIndex == funcHead)
                    continue;

                _addTempToFunctionLiveList(Tree, tempReg, instIndex);
            }
        }

        /* If there is another function call between a function output argument and its user,
        ** we need to add the argument register to the live list of another function.
        */
        for (i = 0; i < Tree->tempCount; i++)
        {
            gcLINKTREE_TEMP tempReg = &Tree->tempArray[i];
            gcsLINKTREE_LIST_PTR user;
            gctINT funcIndex;
            gctUINT funcHead;

            /* Skip non function output registers. */
            if (tempReg->inputOrOutput != gcvFUNCTION_OUTPUT)
                continue;

            _GetFunctionByArgumentIndex(Tree, tempReg->index, gcvFUNCTION_OUTPUT, &funcIndex, &funcHead);
            gcmASSERT(funcIndex != -1);

            user = tempReg->users;

            for (; user != gcvNULL; user = user->next)
            {
                gctINT instIndex = user->index - 1;

                /* Get the nearest call instruction index. */
                while (instIndex > 0 && Tree->shader->code[instIndex].opcode != gcSL_CALL)
                {
                    instIndex --;
                }

                if (instIndex < 0 || Tree->shader->code[instIndex].tempIndex == funcHead)
                    continue;

                _addTempToFunctionLiveList(Tree, tempReg, instIndex);
            }
        }
    }

#if _USE_NEW_VARIABLE_HANDLER_
    /* Update lastUse for variables. */
    if (Shader->variableCount > 0)
    {
        gctUINT variableCount = Shader->variableCount;

        for (i = 0; i < variableCount; i++)
        {
            gcVARIABLE variable = Shader->variables[i];

            if ((GetVariableKnownArraySize(variable) > 1) || gcmType_isMatrix(variable->type))
            {
                gctUINT size = GetVariableKnownArraySize(variable) * gcmType_Rows(variable->type);
                gctINT lastUse = -1;
                gctUINT j;

                gcmASSERT(variable->tempIndex + size <= Tree->tempCount);
                temp = Tree->tempArray + variable->tempIndex;
                for (j = 0; j < size; j++, temp++)
                {
                    if (temp->lastUse > lastUse)
                    {
                        lastUse = temp->lastUse;
                    }
                }
                temp = Tree->tempArray + variable->tempIndex;
                for (j = 0; j < size; j++, temp++)
                {
                    _updateTempLastUse(Tree, tempReg, lastUse);
                }
            }
        }
    }
#endif

    if (Tree->shader->type == gcSHADER_TYPE_FRAGMENT)
    {
        for (i = 0; i < Tree->attributeCount; ++i)
        {
            if ((Tree->shader->attributes[i] != gcvNULL)
            &&  (Tree->shader->attributes[i]->nameLength == gcSL_POSITION_W)
            )
            {
                Tree->attributeArray[i].inUse   = gcvTRUE;
                Tree->attributeArray[i].lastUse = 0;
            }
        }
    }

    /* Set attributes inUse if the shader says it is always used */
    for (i = 0; i < Tree->attributeCount; ++i)
    {
        if (!Tree->attributeArray[i].inUse &&  (Tree->shader->attributes[i] != gcvNULL))
        {
            gcePATCH_ID patchId = gcvPATCH_INVALID;

            gcoHAL_GetPatchID(gcvNULL, &patchId);

            if (patchId == gcvPATCH_NENAMARK && Tree->shader->type == gcSHADER_TYPE_VERTEX)
            {
                gcmATTRIBUTE_SetAlwaysUsed(Tree->shader->attributes[i], gcvTRUE);
            }

            if (gcmATTRIBUTE_alwaysUsed(Tree->shader->attributes[i]))
            {
                Tree->attributeArray[i].inUse   = gcvTRUE;
                if (Tree->attributeArray[i].users == gcvNULL)
                    Tree->attributeArray[i].lastUse = 0;
            }
        }
    }
    if (Tree->shader->loadUsers)
    {
        gctINT * loadUsers = Tree->shader->loadUsers;
        for (curInstIdx = 0; curInstIdx < Tree->shader->codeCount; curInstIdx++)
        {
            if (loadUsers[curInstIdx] >= 0)
            {
                /* Get instruction. */
                code = &Shader->code[curInstIdx];
                gcmASSERT(gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_LOAD);
                Tree->hints[curInstIdx].lastLoadUser = loadUsers[curInstIdx];
                Tree->hints[curInstIdx].loadDestIndex = code->tempIndex;
            }
        }
    }

    /* Return the status. */
    return status;
}

/*******************************************************************************
**                          _AddDependencies
********************************************************************************
**
**  Mark the dependedncies of a register as used.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**      gcsLINKTREE_LIST_PTR List
**          Pointer to the linked list of dependencies.
**
**  OUTPUT:
**
**      Nothing.
*/
static void
_AddDependencies(
    IN gcLINKTREE Tree,
    IN gcsLINKTREE_LIST_PTR List
    )
{
    /* Loop while we have nodes. */
    while (List != gcvNULL)
    {
        /* Dispatch on the type of register. */
        switch (List->type)
        {
        case gcSL_TEMP:
            /* Only process temporary register if not yet used. */
            if (!Tree->tempArray[List->index].inUse)
            {
                /* Mark temporary register as used. */
                Tree->tempArray[List->index].inUse = gcvTRUE;

                /* Process dependencies of temporary register. */
                _AddDependencies(Tree,
                                 Tree->tempArray[List->index].dependencies);
            }
            break;

        case gcSL_ATTRIBUTE:
            /* Mark attribute as used. */
            Tree->attributeArray[List->index].inUse = gcvTRUE;
            break;

        default:
            return;
        }

        /* Move to next entry in the list. */
        List = List->next;
    }
}

/*******************************************************************************
**                                   _RemoveItem
********************************************************************************
**
**  Remove an item from a list.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**      gcsLINKTREE_LIST_PTR * Root
**          Pointer to the variable holding the linked list pointer.
**
**      gcSL_TYPE Type
**          Type of the item to remove.
**
**      gctINT Index
**          Index of the item to remove.
**
**  OUTPUT:
**
**      Nothing.
*/
static gceSTATUS
_RemoveItem(
    IN gcLINKTREE Tree,
    IN gcsLINKTREE_LIST_PTR * Root,
    IN gcSL_TYPE Type,
    IN gctINT Index
    )
{
    gcsLINKTREE_LIST_PTR list, last = gcvNULL;
    gceSTATUS status;

    for (list = *Root; list != gcvNULL; list = list->next)
    {
        if ((list->type == Type) && (list->index == Index))
        {
            if (list->counter > 1)
            {
                list->counter -= 1;
                break;
            }

            if (last == gcvNULL)
            {
                *Root = list->next;
            }
            else
            {
                last->next = list->next;
            }

            status = gcmOS_SAFE_FREE(gcvNULL, list);
            return status;
        }

        last = list;
    }

    return gcvSTATUS_OK;
}

/*******************************************************************************
**                                   _UnlinkNode
********************************************************************************
**
**  Unlink a node from a list.
**
**  INPUT:
**
**      gcsLINKTREE_LIST_PTR * Root
**          Pointer to the variable holding the linked list pointer.
**
**      gcsLINKTREE_LIST_PTR Node
**          Node to unlink.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
_UnlinkNode(
    IN gcsLINKTREE_LIST_PTR * Root,
    IN gcsLINKTREE_LIST_PTR   Node
    )
{
    gcsLINKTREE_LIST_PTR list, last = gcvNULL;

    for (list = *Root; list != gcvNULL; list = list->next)
    {
        if (list == Node)
        {
            if (list->counter > 1)
            {
                list->counter -= 1;
                return gcvSTATUS_OK;
            }

            if (last == gcvNULL)
            {
                *Root = list->next;
            }
            else
            {
                last->next = list->next;
            }
            return gcvSTATUS_OK;
        }

        last = list;
    }

    return gcvSTATUS_NOT_FOUND;
}

/*******************************************************************************
**                          gcLINKTREE_RemoveUnusedAttributes
********************************************************************************
**
**  Remove unused attributes from a shader.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcLINKTREE_RemoveUnusedAttributes(
    IN gcLINKTREE Tree
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T i;

    for (i = 0; i < Tree->attributeCount; i++)
    {
        /* Only process if attribute is dead. */
        if (Tree->shader->attributes[i] &&
            !Tree->attributeArray[i].inUse &&
            !gcmATTRIBUTE_alwaysUsed(Tree->shader->attributes[i]) &&
            !gcmATTRIBUTE_packedAway(Tree->shader->attributes[i]))
        {
            /* Only process if attribute is dead. */
            Tree->attributeArray[i].lastUse = -1;

            /* Loop while we have users. */
            _Delete(Tree, &Tree->attributeArray[i].users);

            if (ENABLE_FULL_NEW_LINKER)
            {
                if (Tree->shader->attributes[i] != gcvNULL)
                {
                    /* Only process if the shader defines this attribute. */
                    gcmATTRIBUTE_SetEnabled(Tree->shader->attributes[i], gcvFALSE);
                }
            }
            else
            {
                /* Only process if the shader defines this attribute. */
                if ((Tree->shader->type == gcSHADER_TYPE_FRAGMENT) &&
                     (Tree->shader->attributes[i] != gcvNULL) )
                {
                    /* Mark attribute as dead. */
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Tree->shader->attributes[i]));

                    /* Loop while we have users. */
                    Tree->shader->attributes[i] = gcvNULL;
                }
                else if (Tree->shader->attributes[i] != gcvNULL)
                {
                    /* Only process if the shader defines this attribute. */
                    gcmATTRIBUTE_SetEnabled(Tree->shader->attributes[i], gcvFALSE);
                }
            }
        }
    }

    return status;
}

gceSTATUS
gcLINKTREE_AddDependencyForCode(
    IN gcLINKTREE Tree,
    IN gcSL_INSTRUCTION code
    )
{
    gctINT index;
    gctINT regIndex;

    /* handle source 0 and its indexed */
    switch (gcmSL_SOURCE_GET(code->source0, Type))
    {
    case gcSL_TEMP:
        index = gcmSL_INDEX_GET(code->source0Index, Index);

        /* Only process if temporary register is not yet in use. */
        if (!Tree->tempArray[index].inUse)
        {
            /* Mark the temporary register as used. */
            Tree->tempArray[index].inUse = gcvTRUE;

            /* Process all dependencies. */
            _AddDependencies(Tree,
                                Tree->tempArray[index].dependencies);
        }
        break;

    case gcSL_ATTRIBUTE:
        index = gcmSL_INDEX_GET(code->source0Index, Index);

        /* Mark the attribute as used. */
        Tree->attributeArray[index].inUse = gcvTRUE;
        break;

    default:
        break;
    }

    if (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED)
    {
        regIndex = gcmSL_INDEX_GET(code->source0Index, Index);
        index = code->source0Indexed;
        if (!Tree->tempArray[index].inUse)
        {
            /* Mark the temporary register as used. */
            Tree->tempArray[index].inUse = gcvTRUE;

            /* Process all dependencies. */
            _AddDependencies(Tree,
                                Tree->tempArray[index].dependencies);
        }

        if (Tree->tempArray[regIndex].variable != gcvNULL)
        {
            gcVARIABLE variable = Tree->tempArray[regIndex].variable;

            gctUINT startIndex, endIndex, j;
            gcmASSERT(isVariableNormal(variable));
            gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvFALSE,
                                                &startIndex, &endIndex);
            gcmASSERT(startIndex == variable->tempIndex);

            /* Source is a temporary register array. */
            for (j = startIndex; j < endIndex; j++)
            {
                gcLINKTREE_TEMP tempA = &Tree->tempArray[j];
                tempA->inUse = gcvTRUE;

                /* Process all dependencies. */
                _AddDependencies(Tree, tempA->dependencies);
            }
        }
    }

    /* handle source 1 and its indexed */
    switch (gcmSL_SOURCE_GET(code->source1, Type))
    {
    case gcSL_TEMP:
        index = gcmSL_INDEX_GET(code->source1Index, Index);

        /* Only process if temporary register is not yet in use. */
        if (!Tree->tempArray[index].inUse)
        {
            /* Mark the temporary register as used. */
            Tree->tempArray[index].inUse = gcvTRUE;

            /* Process all dependencies. */
            _AddDependencies(Tree,
                                Tree->tempArray[index].dependencies);
        }
        break;

    case gcSL_ATTRIBUTE:
        index = gcmSL_INDEX_GET(code->source1Index, Index);

        /* Mark the attribute as used. */
        Tree->attributeArray[index].inUse = gcvTRUE;
        break;

    default:
        break;
    }

    if (gcmSL_SOURCE_GET(code->source1, Indexed) != gcSL_NOT_INDEXED)
    {
        regIndex = gcmSL_INDEX_GET(code->source1Index, Index);
        index = code->source1Indexed;
        if (!Tree->tempArray[index].inUse)
        {
            /* Mark the temporary register as used. */
            Tree->tempArray[index].inUse = gcvTRUE;

            /* Process all dependencies. */
            _AddDependencies(Tree,
                                Tree->tempArray[index].dependencies);
        }

        if (Tree->tempArray[regIndex].variable != gcvNULL)
        {
            gcVARIABLE variable = Tree->tempArray[regIndex].variable;

            gctUINT startIndex, endIndex, j;
            gcmASSERT(isVariableNormal(variable));
            gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvFALSE,
                                                &startIndex, &endIndex);
            gcmASSERT(startIndex == variable->tempIndex);

            /* Source is a temporary register array. */
            for (j = startIndex; j < endIndex; j++)
            {
                gcLINKTREE_TEMP tempA = &Tree->tempArray[j];
                tempA->inUse = gcvTRUE;

                /* Process all dependencies. */
                _AddDependencies(Tree, tempA->dependencies);
            }
        }
    }

    /* handle dest's indexed */
    if (gcmSL_TARGET_GET(code->temp, Indexed) != gcSL_NOT_INDEXED)
    {
        gcSL_INDEXED indexCompnent = gcmSL_TARGET_GET(code->temp, Indexed);
        if (!Tree->tempArray[indexCompnent].inUse)
        {
            /* Mark the temporary register as used. */
            Tree->tempArray[indexCompnent].inUse = gcvTRUE;

            /* Process all dependencies. */
            _AddDependencies(Tree,
                                Tree->tempArray[indexCompnent].dependencies);
        }
    }
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                          gcLINKTREE_RemoveDeadCode
********************************************************************************
**
**  Remove dead code from a shader.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcLINKTREE_RemoveDeadCode(
    IN gcLINKTREE Tree
    )
{
    gctSIZE_T i;

    if (gctOPT_hasFeature(FB_DISABLE_OLD_DCE))
    {
        gcLINKTREE_MarkAllAsUsed(Tree);
        return gcvSTATUS_OK;
    }

    /***************************************************************************
    ** I: Mark all defined outputs and their dependency tree as used.
    */

    /* Walk all outputs. */
    for (i = 0; i < Tree->outputCount; i++)
    {
        /* Get temporary register holding this output. */
        gctINT temp = Tree->outputArray[i].tempHolding;

        if (temp < 0)
        {
            /* Remove output from shader. */
            if (Tree->shader->outputs[i] != gcvNULL)
            {
                /* Delete the output. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Tree->shader->outputs[i]));
            }
        }
        else
        {
            gctUINT size, regIdx, endRegIdx;
            gcOUTPUT output = Tree->shader->outputs[i];

            size = 1;  /* do not go through each element of array,
                        * it is already flaten: each element has its
                        * own output variable */
            if (gcmType_isMatrix(output->type)) size *= gcmType_Rows(output->type);
            endRegIdx = temp + size;

            for (regIdx = temp; regIdx < endRegIdx; regIdx ++)
            {
                /* Only process if temporary register is not yet in use. */
                if (!Tree->tempArray[regIdx].inUse)
                {
                    /* Mark the temporary register as used. */
                    Tree->tempArray[regIdx].inUse = gcvTRUE;

                    /* Process all dependencies. */
                    _AddDependencies(Tree, Tree->tempArray[regIdx].dependencies);
                }
            }
        }
    }

    for (i = 0; i < Tree->tempCount; i++)
    {
        if (Tree->tempArray[i].inUse)
        {
            if (Tree->tempArray[i].variable != gcvNULL)
            {
                gcVARIABLE variable = Tree->tempArray[i].variable;

                gctUINT startIndex, endIndex, j;
                gcmASSERT(isVariableNormal(variable));
                gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvFALSE,
                                                    &startIndex, &endIndex);
                gcmASSERT(startIndex == variable->tempIndex);

                /* Source is a temporary register array. */
                for (j = startIndex; j < endIndex; j++)
                {
                    gcLINKTREE_TEMP tempA = &Tree->tempArray[j];
                    tempA->inUse = gcvTRUE;

                    /* Process all dependencies. */
                    _AddDependencies(Tree, tempA->dependencies);
                }
            }
        }
    }

    /***************************************************************************
    ** II: Mark all dependencies of JMP instructions as used.
    **     Mark all memory write instructions as used for CL shader.
    */

    for (i = 0; i < Tree->shader->codeCount; ++i)
    {
        gcSL_INSTRUCTION code = &Tree->shader->code[i];

        switch (gcmSL_OPCODE_GET(code->opcode, Opcode))
        {
        case gcSL_JMP:
            /* fall through */
        case gcSL_CALL:
            /* fall through */
        case gcSL_RET:
            /* fall through */
        case gcSL_KILL:
            if (gcmSL_TARGET_GET(code->temp, Condition) != gcSL_ALWAYS)
            {
                gcLINKTREE_AddDependencyForCode(Tree, code);
            }
            break;

        /* For a CL shader, it may has no output,
        ** but we need to check the temp registers that use memory write instructions.
        */
        /* store instructions. */
        case gcSL_STORE:
        case gcSL_STORE1:
        case gcSL_ATTR_ST:

        /* bit range instruction is paired with bitextract/bitinsert
           and should not be removed */
        case gcSL_BITRANGE:
        case gcSL_BITRANGE1:

        /* atom instructions. */
        case gcSL_ATOMADD:
        case gcSL_ATOMSUB:
        case gcSL_ATOMXCHG:
        case gcSL_ATOMCMPXCHG:
        case gcSL_ATOMMIN:
        case gcSL_ATOMMAX:
        case gcSL_ATOMOR:
        case gcSL_ATOMAND:
        case gcSL_ATOMXOR:

        /* image write instruction. */
        case gcSL_IMAGE_WR:
        case gcSL_IMAGE_WR_3D:
            if (/*Tree->shader->type == gcSHADER_TYPE_CL && */
                !Tree->tempArray[code->tempIndex].inUse)
            {
                /* Mark the temporary register as used. */
                Tree->tempArray[code->tempIndex].inUse = gcvTRUE;

                /* Process all dependencies. */
                _AddDependencies(Tree, Tree->tempArray[code->tempIndex].dependencies);
            }
            break;

        default:
            break;
        }
    }

    for (i = 0; i < Tree->shader->codeCount; ++i)
    {
        gcSL_INSTRUCTION code = &Tree->shader->code[i];
        gcSL_OPCODE opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
        gctINT index;

        /* If the attribute is used as the bias value on a texture load bias instructions,
        ** the real user is the code before user code.
        */
        if (gcSL_isOpcodeTexldModifier(opcode))
        {
            if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_ATTRIBUTE)
            {
                index = gcmSL_INDEX_GET(code->source1Index, Index);
                Tree->attributeArray[index].inUse = gcvTRUE;
            }
        }

        if (opcode == gcSL_JMP || opcode == gcSL_CALL ||
            opcode == gcSL_RET || opcode == gcSL_KILL)
        {
            continue;
        }

        /* if the target register is use, the all source are used too */
        if (!gcSL_isOpcodeHaveNoTarget(code->opcode) && gcmSL_TARGET_GET(code->temp, Enable) != gcSL_ENABLE_NONE)
        {
            if ((gctUINT32)code->tempIndex >= Tree->tempCount)
            {
                gcmASSERT(gcvFALSE);
                continue;
            }
            else if (Tree->tempArray[code->tempIndex].inUse)
            {
                gcLINKTREE_AddDependencyForCode(Tree, code);
            }
        }

    }

    /* Walk all temporary registers. */
    for (i = 0; i < Tree->tempCount; i++)
    {
        /* Only process if temporary register is dead. */
        if (!Tree->tempArray[i].inUse
        &&  (Tree->tempArray[i].defined != gcvNULL)
        )
        {
            gctINT minPC, maxPC;
            gcsLINKTREE_LIST_PTR node;

            /* Find the minimum instruction index. */
            minPC = Tree->tempArray[i].defined->index;

            for (node = Tree->tempArray[i].defined->next;
                 node != gcvNULL;
                 node = node->next)
            {
                if (node->index < minPC)
                {
                    minPC = node->index;
                }
            }

            /* Get the maximum instruction index. */
            maxPC = gcmMAX(Tree->tempArray[i].lastUse,
                          (gctINT) Tree->shader->codeCount - 1);

            /* Loop through the instructions where the temporary register is
               alive. */
            while (minPC <= maxPC)
            {
                /* Get the instruction pointer. */
                gcSL_INSTRUCTION code = Tree->shader->code + minPC;
                gcSL_OPCODE opcode    = gcmSL_OPCODE_GET(code->opcode, Opcode);
                if (code->tempIndex == i)
                {
                    switch (opcode)
                    {
                    case gcSL_JMP:
                        /* fall through */
                    case gcSL_CALL:
                        /* fall through */
                    case gcSL_RET:
                        /* fall through */
                    case gcSL_KILL:
                        /* fall through */
                    case gcSL_EMIT_VERTEX:
                        /* fall through */
                    case gcSL_END_PRIMITIVE:
                        break;

                    case gcSL_SET:
                    case gcSL_CMP:
                        {
                            gcSL_INSTRUCTION relatedCode;

                            if (gcmSL_TARGET_GET(code->temp, Condition) == gcSL_ZERO) {
                                if((minPC + 1) <= maxPC) {
                                   relatedCode = code + 1;
                                   if((gcmSL_OPCODE_GET(relatedCode->opcode, Opcode) == gcSL_SET ||
                                       gcmSL_OPCODE_GET(relatedCode->opcode, Opcode) == gcSL_CMP) &&
                                      gcmSL_TARGET_GET(relatedCode->temp, Condition) == gcSL_NOT_ZERO) {
                                      break;
                                   }
                                }
                            }
                            else if (gcmSL_TARGET_GET(code->temp, Condition) == gcSL_NOT_ZERO) {
                                relatedCode = code - 1;
                                if(relatedCode >= Tree->shader->code &&
                                   (gcmSL_OPCODE_GET(relatedCode->opcode, Opcode) == gcSL_SET ||
                                    gcmSL_OPCODE_GET(relatedCode->opcode, Opcode) == gcSL_CMP) &&
                                   gcmSL_TARGET_GET(relatedCode->temp, Condition) == gcSL_ZERO) {
                                   /* Replace the SET.Z instruction with a NOP. */
                                   gcoOS_ZeroMemory(relatedCode,
                                                    gcmSIZEOF(struct _gcSL_INSTRUCTION));
                                }
                            }
                        }
                        gcoOS_ZeroMemory(code,
                                         gcmSIZEOF(struct _gcSL_INSTRUCTION));
                        break;

                    default:
                        /* check if the previous texld modifier should be removed too */
                        if (gcSL_isOpcodeTexld(opcode))
                        {
                            gcSL_INSTRUCTION prevCode = code - 1;
                            if (prevCode >= Tree->shader->code)
                            {
                                gcSL_OPCODE opcodePrev = gcmSL_OPCODE_GET(prevCode->opcode, Opcode);

                                if (gcSL_isOpcodeTexldModifier(opcodePrev))
                                {
                                    /* Replace instruction with a NOP. */
                                    gcoOS_ZeroMemory(prevCode, gcmSIZEOF(struct _gcSL_INSTRUCTION));
                                }
                            }
                        }
                        /* Replace instruction with a NOP. */
                        gcoOS_ZeroMemory(code,
                                                     gcmSIZEOF(struct _gcSL_INSTRUCTION));
                    }
                }

                /* Next instruction. */
                minPC++;
            }

            /* Remove the temporary register usage. */
            Tree->tempArray[i].lastUse = -1;

            /* Remove all definitions. */
            _Delete(Tree, &Tree->tempArray[i].defined);

            /* Remove all dependencies. */
            _Delete(Tree, &Tree->tempArray[i].dependencies);

            /* Remove all users. */
            _Delete(Tree, &Tree->tempArray[i].users);
        }
    }

    /* Success. */
    return gcvSTATUS_OK;
}

static gceSTATUS
_vpAllocate(
    IN gctUINT32 Bytes,
    OUT gctPOINTER * Memory
    )
{
    return gcoOS_Allocate(gcvNULL, Bytes, Memory);
};

static gceSTATUS
_vpFree(
    IN gctPOINTER Memory
    )
{
    return gcoOS_Free(gcvNULL, Memory);
}

static const gcsAllocator _vpAllocator = {_vpAllocate, _vpFree };

static gctBOOL
_CompareTempKey (
     IN void *    Data,
     IN void *    Key
     )
{
    return  ((gcVaryingPackInfo*)Data)->treeOutput->tempHolding == (gctINT)(gctUINTPTR_T)Key ;
}

static gctBOOL
_CompareAttributeKey (
     IN void *    Data,
     IN void *    Key
     )
{
    return  ((gcVaryingPackInfo*)Data)->treeOutput->fragmentAttribute == (gctINT)(gctUINTPTR_T)Key ;
}


static void
_fixSourceSwizzle(
    IN OUT gctSOURCE_t *    Source,
    IN gcComponentsMapping  CompMap
    )
{
    gcSL_TYPE type = gcmSL_SOURCE_GET(*Source, Type);
    gctSOURCE_t src = *Source;

    if (type == gcSL_NONE ||
        type == gcSL_SAMPLER ||
        type == gcSL_CONSTANT)
    {
        return;
    }

    switch (CompMap) {
    case gcCMAP_XY2ZW:
        src = gcmSL_SOURCE_SET(src, SwizzleZ,
                               gcmSL_SOURCE_GET(src, SwizzleX));
        src = gcmSL_SOURCE_SET(src, SwizzleW,
                               gcmSL_SOURCE_GET(src, SwizzleY));
        break;
    case gcCMAP_X2Y:        /* map .x to .y */
        src = gcmSL_SOURCE_SET(src, SwizzleY,
                               gcmSL_SOURCE_GET(src, SwizzleX));
        break;
    case gcCMAP_X2Z:        /* map .x to .z */
        src = gcmSL_SOURCE_SET(src, SwizzleZ,
                               gcmSL_SOURCE_GET(src, SwizzleX));
        break;
    case gcCMAP_X2W:        /* map .x to .w */
        src = gcmSL_SOURCE_SET(src, SwizzleW,
                               gcmSL_SOURCE_GET(src, SwizzleX));
        break;
    case gcCMAP_Y2Z:        /* map .y to .z */
        src = gcmSL_SOURCE_SET(src, SwizzleZ,
                               gcmSL_SOURCE_GET(src, SwizzleY));
        break;
    case gcCMAP_Y2W:        /* map .y to .w */
        src = gcmSL_SOURCE_SET(src, SwizzleW,
                               gcmSL_SOURCE_GET(src, SwizzleY));
        break;
    case gcCMAP_Z2W:        /* map .z to .w */
        src = gcmSL_SOURCE_SET(src, SwizzleW,
                               gcmSL_SOURCE_GET(src, SwizzleZ));
        break;
    default:
        gcmASSERT(0);
        break;
    }
    *Source = src;
    return;
}

/* fix Code's swizzle according to CompMap */
static void
_fixSwizzle(
    IN OUT gcSL_INSTRUCTION Code,
    IN gcComponentsMapping  CompMap
    )
{
    gcSL_OPCODE opcode = gcmSL_OPCODE_GET(Code->opcode, Opcode);
    if (opcode == gcSL_DP3 || opcode == gcSL_DP4 || opcode == gcSL_DP2 ||
        opcode == gcSL_CROSS || opcode == gcSL_NORM)
    {
        /* don't change swizzle if the instuction uses components
           independent of its enable */
        return;
    }

    /* fix source0 */
    _fixSourceSwizzle(&Code->source0, CompMap);

    /* fix source1 */
    _fixSourceSwizzle(&Code->source1, CompMap);
}

/* fix Code's enable according to CompMap */
static void
_fixEnable(
    IN OUT gcSL_INSTRUCTION Code,
    IN gcComponentsMapping  CompMap
    )
{
    gcSL_ENABLE old_enable = gcmSL_TARGET_GET(Code->temp, Enable);
    gcSL_ENABLE new_enable = gcSL_ENABLE_NONE;
    switch (CompMap) {
    case gcCMAP_XY2ZW:
        if ((old_enable & gcSL_ENABLE_X) != gcSL_ENABLE_NONE)
        {
            new_enable |= gcSL_ENABLE_Z;
        }
        if ((old_enable & gcSL_ENABLE_Y) != gcSL_ENABLE_NONE)
        {
            new_enable |= gcSL_ENABLE_W;
        }
        Code->temp = gcmSL_TARGET_SET(Code->temp,
                                      Enable, new_enable);
        break;
    case gcCMAP_X2Y:
        if ((old_enable & gcSL_ENABLE_X) != 0)
            Code->temp = gcmSL_TARGET_SET(Code->temp,
                                          Enable, gcSL_ENABLE_Y);
        break;
    case gcCMAP_X2Z:
        if ((old_enable & gcSL_ENABLE_X) != 0)
            Code->temp = gcmSL_TARGET_SET(Code->temp,
                                          Enable, gcSL_ENABLE_Z);
        break;
    case gcCMAP_X2W:
        if ((old_enable & gcSL_ENABLE_X) != 0)
            Code->temp = gcmSL_TARGET_SET(Code->temp,
                                          Enable, gcSL_ENABLE_W);
        break;
    case gcCMAP_Y2Z:
        if ((old_enable & gcSL_ENABLE_Y) != 0)
            Code->temp = gcmSL_TARGET_SET(Code->temp,
                                          Enable, gcSL_ENABLE_Z);
        break;
    case gcCMAP_Y2W:
        if ((old_enable & gcSL_ENABLE_Y) != 0)
            Code->temp = gcmSL_TARGET_SET(Code->temp,
                                          Enable, gcSL_ENABLE_W);
        break;
    case gcCMAP_Z2W:
        if ((old_enable & gcSL_ENABLE_Z) != 0)
            Code->temp = gcmSL_TARGET_SET(Code->temp,
                                          Enable, gcSL_ENABLE_W);
        break;

    default:
        break;
    }
}

/* map the source component*/
static void
_mappingSourceComponent(
    IN OUT gctSOURCE_t *    Source,
    IN gcComponentsMapping  CompMap
    )
{
    gcSL_TYPE type = gcmSL_SOURCE_GET(*Source, Type);
    gcSL_SWIZZLE swizzle[4];
    gcSL_SWIZZLE new_swizzle;
    gctINT i;

    if (type == gcSL_NONE ||
        type == gcSL_SAMPLER ||
        type == gcSL_CONSTANT)
    {
        return;
    }

    swizzle[0] = gcmSL_SOURCE_GET(*Source, SwizzleX);
    swizzle[1] = gcmSL_SOURCE_GET(*Source, SwizzleY);
    swizzle[2] = gcmSL_SOURCE_GET(*Source, SwizzleZ);
    swizzle[3] = gcmSL_SOURCE_GET(*Source, SwizzleW);

    /* mapping the swizzle for each component */
    for (i = 0; i < 4; i++)
    {
        switch (CompMap) {
        case gcCMAP_XY2ZW:
            if (swizzle[i] == gcSL_SWIZZLE_X)
            {
                swizzle[i] = gcSL_SWIZZLE_Z;
            }
            else if (swizzle[i] == gcSL_SWIZZLE_Y)
            {
                swizzle[i] = gcSL_SWIZZLE_W;
            }
            else
            {
                gcmASSERT(0);
            }
            break;
        case gcCMAP_X2Y:        /* map .x to .y */
            gcmASSERT(swizzle[i] == gcSL_SWIZZLE_X);
            swizzle[i] = gcSL_SWIZZLE_Y;
            break;
        case gcCMAP_X2Z:        /* map .x to .z */
            gcmASSERT(swizzle[i] == gcSL_SWIZZLE_X);
            swizzle[i] = gcSL_SWIZZLE_Z;
            break;
        case gcCMAP_X2W:        /* map .x to .w */
            gcmASSERT(swizzle[i] == gcSL_SWIZZLE_X);
            swizzle[i] = gcSL_SWIZZLE_W;
            break;
        case gcCMAP_Y2Z:        /* map .y to .z */
            gcmASSERT(swizzle[i] == gcSL_SWIZZLE_Y);
            swizzle[i] = gcSL_SWIZZLE_Z;
            break;
        case gcCMAP_Y2W:        /* map .y to .w */
            gcmASSERT(swizzle[i] == gcSL_SWIZZLE_Y);
            swizzle[i] = gcSL_SWIZZLE_W;
            break;
        case gcCMAP_Z2W:        /* map .z to .w */
            gcmASSERT(swizzle[i] == gcSL_SWIZZLE_Z);
            swizzle[i] = gcSL_SWIZZLE_W;
            break;
        default:
            gcmASSERT(0);
            break;
        }
    }
    /* set the new swizzle */
    new_swizzle = gcmComposeSwizzle(swizzle[0],
                                    swizzle[1],
                                    swizzle[2],
                                    swizzle[3]);
    *Source = gcmSL_SOURCE_SET(*Source, Swizzle, new_swizzle);

    return;
}

/* change the sourceIndex/sourceIndexed from old temp index to NewIndex,
   if the old one is the same as the one in VpInfo
  */
static void
_fixSourceTemp(
    IN OUT gctSOURCE_t *    Source,
    IN OUT gctUINT16 *      SourceIndex,
    IN OUT gctUINT16 *      SourceIndexed,
    IN gcVaryingPackInfo *  VpInfo,
    IN gctINT               NewTempIndex
    )
{
    if (gcmSL_SOURCE_GET(*Source, Type) == gcSL_TEMP &&
        gcmSL_INDEX_GET(*SourceIndex, Index) ==
                                  VpInfo->treeOutput->tempHolding)
    {
        *SourceIndex = gcmSL_INDEX_SET(*SourceIndex, Index, NewTempIndex);
        _mappingSourceComponent(Source, VpInfo->compmap);
    }

    /* fix indexed */
    if (gcmSL_SOURCE_GET(*Source, Type) != gcSL_NONE &&
        gcmSL_SOURCE_GET(*Source, Indexed) != gcSL_NOT_INDEXED &&
        *SourceIndexed == VpInfo->treeOutput->tempHolding
        )
    {
        gcSL_INDEXED indexCompnent = gcmSL_SOURCE_GET(*Source, Indexed);

        /* change indexed value */
        *SourceIndexed = (gctUINT16)NewTempIndex;
        switch (VpInfo->compmap) {
        case gcCMAP_XY2ZW:
            if (indexCompnent == gcSL_INDEXED_X)
            {
                *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_Z);
            }
            else if (indexCompnent == gcSL_INDEXED_Y)
            {
                *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_W);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
            break;
        case gcCMAP_X2Y:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_Y);
            break;
        case gcCMAP_X2Z:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_Z);
            break;
        case gcCMAP_X2W:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_W);
            break;
        case gcCMAP_Y2Z:
            gcmASSERT(indexCompnent == gcSL_INDEXED_Y);
            *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_Z);
            break;
        case gcCMAP_Y2W:
            gcmASSERT(indexCompnent == gcSL_INDEXED_Y);
            *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_W);
            break;
        case gcCMAP_Z2W:
            gcmASSERT(indexCompnent == gcSL_INDEXED_Z);
            *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_W);
            break;
        default:
            break;
        }
    }
    return;
}

static void
_fixTargetTemp(
    IN OUT gcSL_INSTRUCTION Code,
    IN gcVaryingPackInfo *  VpInfo,
    IN gctINT               NewTempIndex
    )
{
    /* fix non-indexed target. */
    if (Code->tempIndex == (gctUINT16)VpInfo->treeOutput->tempHolding)
    {
        Code->tempIndex = (gctUINT16)NewTempIndex;
        _fixEnable(Code, VpInfo->compmap);
    }

    /* fix indexed target. */
    if (gcmSL_TARGET_GET(Code->temp, Indexed) != gcSL_NOT_INDEXED &&
        Code->tempIndexed == (gctUINT16)VpInfo->treeOutput->tempHolding)
    {
        gcSL_INDEXED indexCompnent = gcmSL_TARGET_GET(Code->temp, Indexed);

        /* change indexed value */
        Code->tempIndexed = (gctUINT16)NewTempIndex;
        switch (VpInfo->compmap)
        {
        case gcCMAP_XY2ZW:
            if (indexCompnent == gcSL_INDEXED_X)
            {
                Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_Z);
            }
            else if (indexCompnent == gcSL_INDEXED_Y)
            {
                Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_W);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
            break;

        case gcCMAP_X2Y:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_Y);
            break;

        case gcCMAP_X2Z:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_Z);
            break;

        case gcCMAP_X2W:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_W);
            break;

        case gcCMAP_Y2Z:
            gcmASSERT(indexCompnent == gcSL_INDEXED_Y);
            Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_Z);
            break;

        case gcCMAP_Y2W:
            gcmASSERT(indexCompnent == gcSL_INDEXED_Y);
            Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_W);
            break;

        case gcCMAP_Z2W:
            gcmASSERT(indexCompnent == gcSL_INDEXED_Z);
            Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_W);
            break;

        default:
            break;
        }
    }
    return;
}

static void
_fixSourcesTemp(
    IN OUT gcSL_INSTRUCTION Code,
    IN gcVaryingPackInfo *  VpInfo,
    IN gctINT               NewTempIndex
    )
{
    /* fix source0 */
    _fixSourceTemp(&Code->source0,
                   &Code->source0Index,
                   &Code->source0Indexed,
                   VpInfo,
                   NewTempIndex);

    /* fix source1 */
    _fixSourceTemp(&Code->source1,
                   &Code->source1Index,
                   &Code->source1Indexed,
                   VpInfo,
                   NewTempIndex);
}

/* patching vertex shader:

     for each packed output, mapping the temp register and components to new one
 */
static void
_packVertexOutput(
    IN gcLINKTREE VertexTree,
    IN gcsList *  VpMappingList
    )
{
    gctUINT       i;
    gcsListNode  *mappingNode;

    /* Walk all temporary registers:
         find if the temp register is in mapping list
         do the mapping if it is in the list:
            1. change the definition
            2. change the users
            3. cleanup the tree data for both temp and mapped temp
            4. change the output array
     */
    for (i = 0; i < VertexTree->tempCount; i++)
    {
        gcLINKTREE_TEMP temp = &VertexTree->tempArray[i];
        if (!temp->defined)
            continue;

        /* Only process if temporary register is in VpMappingList:

              temp(41).x => temp(29).w

              temp(45).xy => temp(23).zw
         */
        mappingNode = gcList_FindNode(VpMappingList, (gctPOINTER)(gctUINTPTR_T)i, _CompareTempKey);
        if (mappingNode != gcvNULL)
        {
            gcVaryingPackInfo * vpInfo = (gcVaryingPackInfo*)mappingNode->data;
            gctINT packedWith = vpInfo->treeOutput->packedWith;
            gctINT newTempIdx = VertexTree->outputArray[packedWith].tempHolding;
            gcLINKTREE_TEMP newTemp = &VertexTree->tempArray[newTempIdx];
            gcsLINKTREE_LIST_PTR node;

            /* for each defintion of the temp, change the

                  OP  temp.xy, s0, s1

               to:

                  new_temp = temp_mapping(temp),
                  new_enable = component_mapping(xy);
                  s0_new_swizzle = swizzle_mapping(s0.swizzle);
                  s1_new_swizzle = swizzle_mapping(s1.swizzle);

                  OP new_temp.new_enable, s0.s0_new_swizzle, s1.s1_new_swizzle
             */

            for (node = temp->defined; node != gcvNULL; node = node->next)
            {
                gcSL_INSTRUCTION code = VertexTree->shader->code + node->index;

                gcmASSERT(code->tempIndex == i);
                /* change temp index */
                code->tempIndex = (gctUINT16)newTempIdx;

                /* change enable */
                _fixEnable(code, vpInfo->compmap);
                /* change swizzle */
                _fixSwizzle(code, vpInfo->compmap);

               /* update new temp's usage */
                newTemp->usage |= gcmSL_TARGET_GET(code->temp, Enable);
               /* add the defined to new temp */
                gcLINKTREE_AddList(VertexTree,
                                   &newTemp->defined,
                                   node->type,
                                   node->index);
            }

            /* fix all users, including the use in indexed usage:

                   temp.x  ==>  new_temp.y  for X2Y mapping
                   temp.x  ==>  new_temp.z  for X2Z mapping
                   temp.x  ==>  new_temp.w  for X2W mapping
                   temp.y  ==>  new_temp.z  for Y2Z mapping
                   temp.y  ==>  new_temp.w  for Y2W mapping
                   temp.z  ==>  new_temp.w  for Z2W mapping
             */
            for (node = temp->users;
                 node != gcvNULL;
                 node = node->next)
            {
                gcSL_INSTRUCTION code;
                /* no need to handle if the user is output */
                if (node->type == gcSL_OUTPUT)
                    continue;
                gcmASSERT(node->type == gcSL_NONE);
                code = VertexTree->shader->code + node->index;

                /* change sources */
                _fixSourcesTemp(code, vpInfo, newTempIdx);

                /* If this temp reg is used as a target on a store instruction,
                ** we also need to change it.
                */
                if (code->opcode == gcSL_STORE ||
                    code->opcode == gcSL_IMAGE_WR ||
                    code->opcode == gcSL_IMAGE_WR_3D ||
                    code->opcode == gcSL_ATTR_ST)
                {
                    _fixTargetTemp(code,
                                vpInfo,
                                newTempIdx);
                   /* update new temp's usage */
                    newTemp->usage |= gcmSL_TARGET_GET(code->temp, Enable);
                }

                /* add the user to new temp */
                gcLINKTREE_AddList(VertexTree,
                                   &newTemp->users,
                                   node->type,
                                   node->index);
            }

            for (node = temp->dependencies;
                 node != gcvNULL;
                 node = node->next)
            {
                /* add the dependencies to new temp */
                gcLINKTREE_AddList(VertexTree,
                                   &newTemp->dependencies,
                                   node->type,
                                   node->index);
            }

            /* Remove the temporary register usage. */

            /* Remove all definitions. */
            _Delete(VertexTree, &temp->defined);

            /* Remove all dependencies. */
            _Delete(VertexTree, &temp->dependencies);

            /* Remove all users. */
            _Delete(VertexTree, &temp->users);

            /* Mark the target as not used. */
            temp->inUse   = gcvFALSE;
            temp->lastUse = -1;
            temp->usage   = 0;
        }
    }

    /* Success. */
} /* _packVertexOutput */

/* change the sourceIndex from old attribute index to NewTAttributeIndex,
   if the old one is the same as the one in VpInfo
  */
static void
_fixSourceAttribute(
    IN OUT gctSOURCE_t *    Source,
    IN OUT gctUINT16 *      SourceIndex,
    IN OUT gctUINT16 *      SourceIndexed,
    IN gcVaryingPackInfo *  VpInfo,
    IN gctINT               NewTAttributeIndex
    )
{
    if (gcmSL_SOURCE_GET(*Source, Type) == gcSL_ATTRIBUTE &&
         gcmSL_INDEX_GET(*SourceIndex, Index) ==
                                  VpInfo->treeOutput->fragmentAttribute)
    {
        *SourceIndex = gcmSL_INDEX_SET(*SourceIndex, Index, NewTAttributeIndex);
        _mappingSourceComponent(Source, VpInfo->compmap);
    }

    return;
}

static void
_fixSourcesAttribute(
    IN OUT gcSL_INSTRUCTION Code,
    IN gcVaryingPackInfo *  VpInfo,
    IN gctINT               NewAttributeIndex
    )
{
    /* fix source0 */
    _fixSourceAttribute(&Code->source0,
                        &Code->source0Index,
                        &Code->source0Indexed,
                        VpInfo,
                        NewAttributeIndex);

    /* fix source1 */
    _fixSourceAttribute(&Code->source1,
                        &Code->source1Index,
                        &Code->source1Indexed,
                        VpInfo,
                        NewAttributeIndex);
}

/* get the packed attribute index from the VpInfo, which only has
   packed output index */
static gctINT
_getPackedAttributeIndex(
    IN gcLINKTREE          VertexTree,
    IN gcVaryingPackInfo * VpInfo
    )
{
    gctINT packedWithOutput   = VpInfo->treeOutput->packedWith;
    gcLINKTREE_OUTPUT treeOutput = &VertexTree->outputArray[packedWithOutput];
    return treeOutput->fragmentAttribute;
}

static gceSTATUS
_UpdateShadeModeForEachComponent(
    IN gcLINKTREE Tree,
    IN gctINT PackedAttributeIndex,
    IN gctINT PackingAttributeIndex,
    IN gcComponentsMapping ComponentMap
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcATTRIBUTE packedAttribute = Tree->shader->attributes[PackedAttributeIndex];
    gcATTRIBUTE packingAttribute = Tree->shader->attributes[PackingAttributeIndex];

    switch (ComponentMap)
    {
    case gcCMAP_XY2ZW:
        packedAttribute->componentShadeMode[2] = packingAttribute->componentShadeMode[0];
        packedAttribute->componentShadeMode[3] = packingAttribute->componentShadeMode[1];
        break;
    case gcCMAP_X2Y:
        packedAttribute->componentShadeMode[1] = packingAttribute->componentShadeMode[0];
        break;
    case gcCMAP_X2Z:
        packedAttribute->componentShadeMode[2] = packingAttribute->componentShadeMode[0];
        break;
    case gcCMAP_X2W:
        packedAttribute->componentShadeMode[3] = packingAttribute->componentShadeMode[0];
        break;
    case gcCMAP_Y2Z:
        packedAttribute->componentShadeMode[2] = packingAttribute->componentShadeMode[1];
        break;
    case gcCMAP_Y2W:
        packedAttribute->componentShadeMode[3] = packingAttribute->componentShadeMode[1];
        break;
    case gcCMAP_Z2W:
        packedAttribute->componentShadeMode[3] = packingAttribute->componentShadeMode[2];
        break;
    case gcCMAP_UNCHANGED:
        break;
    default:
        gcmASSERT(0);
        break;
    }
    return status;
}

/* patching fragment shader */
static void
_packFragmentAttribute(
    IN OUT gcLINKTREE FragmentTree,
    IN gcLINKTREE     VertexTree,
    IN gcsList *      VpMappingList
    )
{
    gctUINT       i;
    gcsListNode  *mappingNode;

    /* Walk all attribute registers:
         find if the attribute register is in mapping list
         do the mapping if it is in the list:
            1. change the definition
            2. change the users
            3. cleanup the tree data for both temp and mapped temp
            4. change the attribute array
     */
    for (i = 0; i < FragmentTree->attributeCount; i++)
    {
        gcLINKTREE_ATTRIBUTE treeAttribute = &FragmentTree->attributeArray[i];
        if (!treeAttribute->inUse)
            continue;

        /* Only process if attribute register is in VpMappingList:

              attribute(5).x => attribute(2).w

              attribute(7).xy => attribute(6).zw
         */
        mappingNode = gcList_FindNode(VpMappingList, (gctPOINTER)(gctUINTPTR_T)i, _CompareAttributeKey);
        if (mappingNode != gcvNULL)
        {
            gcVaryingPackInfo * vpInfo = (gcVaryingPackInfo*)mappingNode->data;
            gctINT newAttributeIdx = _getPackedAttributeIndex(VertexTree, vpInfo);
            gcLINKTREE_ATTRIBUTE newTreeAttibute = &FragmentTree->attributeArray[newAttributeIdx];
            gcsLINKTREE_LIST_PTR node;

            /* Change the shade mode for each components. */
            _UpdateShadeModeForEachComponent(FragmentTree, newAttributeIdx, i, vpInfo->compmap);

            /* fix all users, including the use in indexed usage:

                   attribute.x  ==>  new_attribute.y  for X2Y mapping
                   attribute.x  ==>  new_attribute.z  for X2Z mapping
                   attribute.x  ==>  new_attribute.w  for X2W mapping
                   attribute.y  ==>  new_attribute.z  for Y2Z mapping
                   attribute.y  ==>  new_attribute.w  for Y2W mapping
                   attribute.z  ==>  new_attribute.w  for Z2W mapping
             */
            for (node = treeAttribute->users;
                 node != gcvNULL;
                 node = node->next)
            {
                gcSL_INSTRUCTION code, prevCode;
                gcSL_OPCODE  opcode, prevOpcode;
                /* no need to handle if the user is output */
                if (node->type == gcSL_OUTPUT)
                    continue;
                gcmASSERT(node->type == gcSL_NONE);
                code = FragmentTree->shader->code + node->index;

                /* change sources */
                _fixSourcesAttribute(code, vpInfo, newAttributeIdx);

                opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
                /* If the attribute is used as the bias value on a texture load bias instructions,
                ** the affect code is the code before user code.
                */
                if (gcSL_isOpcodeTexld(opcode) && node->index > 0)
                {
                    prevCode = FragmentTree->shader->code + node->index - 1;
                    prevOpcode = gcmSL_OPCODE_GET(prevCode->opcode, Opcode);

                    if (gcSL_isOpcodeTexldModifier(prevOpcode))
                    {
                        _fixSourcesAttribute(prevCode, vpInfo, newAttributeIdx);
                    }
                }

                /* add the user to new temp */
                gcLINKTREE_AddList(FragmentTree,
                                   &newTreeAttibute->users,
                                   node->type,
                                   node->index);
            }

            /* merge the last use */
            if (treeAttribute->lastUse > newTreeAttibute->lastUse)
                newTreeAttibute->lastUse = treeAttribute->lastUse;

            /* copy attribute's isTexure field to new texture */
            gcmATTRIBUTE_SetIsZWTexture(FragmentTree->shader->attributes[newAttributeIdx],
                gcmATTRIBUTE_isTexture(FragmentTree->shader->attributes[i]));
            gcmASSERT(gcmATTRIBUTE_isTexture(FragmentTree->shader->attributes[i]) == gcvFALSE ||
                      vpInfo->compmap == gcCMAP_XY2ZW);

            /* Remove all users. */
            _Delete(FragmentTree, &treeAttribute->users);

            /* Mark the target as not used. */
            treeAttribute->inUse   = gcvFALSE;
            treeAttribute->lastUse = -1;
        }
    }

    /* Success. */
} /* _packFragmentAttribute */

static void
_cleanupOutput(
    IN OUT gcVaryingPackInfo * VpInfo
    )
{
    gcLINKTREE_OUTPUT treeOutput  = VpInfo->treeOutput;
    treeOutput->inUse = gcvFALSE;
    /* Mark vertex output as dead. */
    treeOutput->tempHolding = -1;

}

static void
_cleanupAttribute(
    IN OUT gcLINKTREE      FragmentTree,
    IN gcVaryingPackInfo * VpInfo
    )
{
    gcLINKTREE_OUTPUT    treeOutput     = VpInfo->treeOutput;
    gctINT               attributeIndex = treeOutput->fragmentAttribute;
    gcLINKTREE_ATTRIBUTE treeAttribute  = &FragmentTree->attributeArray[attributeIndex];
    gcATTRIBUTE          attribute      = FragmentTree->shader->attributes[attributeIndex];

    gcmASSERT(attributeIndex < (gctINT)FragmentTree->shader->attributeCount);

    treeAttribute->inUse = gcvFALSE;
    /* Mark attribute as dead. */
    treeAttribute->lastUse = -1;

    /* Mark the attribute as packed away. */
    gcmATTRIBUTE_SetPackedAway(attribute, gcvTRUE);
}

static const gcSHADER_TYPE floatTypeTable[4][4] = {
    {gcSHADER_FLOAT_X1, gcSHADER_FLOAT_X2, gcSHADER_FLOAT_X3, gcSHADER_FLOAT_X4}, /* 1 row */
    {gcSHADER_UNKONWN_TYPE, gcSHADER_FLOAT_2X2, gcSHADER_FLOAT_2X3, gcSHADER_FLOAT_2X4}, /* 2 row */
    {gcSHADER_UNKONWN_TYPE, gcSHADER_FLOAT_3X2, gcSHADER_FLOAT_3X3, gcSHADER_FLOAT_3X4}, /* 3 row */
    {gcSHADER_UNKONWN_TYPE, gcSHADER_FLOAT_4X2, gcSHADER_FLOAT_4X3, gcSHADER_FLOAT_4X4}, /* 4 row */
};

static const gcSHADER_TYPE intTypeTable[4] = {
    gcSHADER_INTEGER_X1, gcSHADER_INTEGER_X2, gcSHADER_INTEGER_X3, gcSHADER_INTEGER_X4
};

static const gcSHADER_TYPE uintTypeTable[4] = {
    gcSHADER_UINT_X1, gcSHADER_UINT_X2, gcSHADER_UINT_X3, gcSHADER_UINT_X4
};


static const gcSHADER_TYPE fixedTypeTable[4] = {
    gcSHADER_FIXED_X1, gcSHADER_FIXED_X2, gcSHADER_FIXED_X3, gcSHADER_FIXED_X4
};

static gcSHADER_TYPE longTypeTable[4] = {
    gcSHADER_INT64_X1, gcSHADER_INT64_X2, gcSHADER_INT64_X3, gcSHADER_INT64_X4
};

static gcSHADER_TYPE ulongTypeTable[4] = {
    gcSHADER_UINT64_X1, gcSHADER_UINT64_X2, gcSHADER_UINT64_X3, gcSHADER_UINT64_X4
};

/* change Type to same row type which has Components */
static void
_changeTypeComponents(
    IN OUT gcSHADER_TYPE *Type,
    IN gctINT             Components)
{
    gctINT oldComponents  = gcmType_Comonents(*Type);
    gctINT rows           = gcmType_Rows(*Type);
    gcSHADER_TYPE_KIND tk = gcmType_Kind(*Type);

    if (oldComponents == Components)
        return;

    gcmASSERT(rows <= 4 && Components <= 4);
    switch (tk) {
    case gceTK_FLOAT:
        gcmASSERT(Components > 1 || rows == 1);
        *Type = floatTypeTable[rows-1][Components-1];
        break;
    case gceTK_INT:
        gcmASSERT(rows == 1);
        *Type = intTypeTable[Components-1];
        break;
    case gceTK_UINT:
        gcmASSERT(rows == 1);
        *Type = uintTypeTable[Components-1];
        break;
    case gceTK_FIXED:
        gcmASSERT(rows == 1);
        *Type = fixedTypeTable[Components-1];
        break;
    case gceTK_INT64:
        gcmASSERT(rows == 1);
        *Type = longTypeTable[Components-1];
        break;
    case gceTK_UINT64:
        gcmASSERT(rows == 1);
        *Type = ulongTypeTable[Components-1];
        break;
    default:
        gcmASSERT(0);
        break;
    }
}

static gctBOOL
_isBuiltinOutput(
    IN gcOUTPUT Output
    )
{
    return (gctINT)Output->nameLength < 0;
}

/*******************************************************************************
**                               gcLINKTREE_PackVarying
********************************************************************************
**
**  Pack the output of the vertex tree and unpack the attributes of the fragment tree.
**
**  INPUT:
**
**      gcLINKTREE VertexTree
**          Pointer to a gcLINKTREE structure representing the vertex shader.
**
**      gcLINKTREE FragmentTree
**          Pointer to a gcLINKTREE structure representing the fragment shader.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcLINKTREE_PackVarying(
    IN gcLINKTREE VertexTree,
    IN gcLINKTREE FragmentTree
    )
{
    gctINT j, vec3_i=0, vec2_i=0, vec1_i=0;
    gctINT i;
    /*gctINT vec4Count = 0;*/
    gctINT vec3Count = 0;
    gctINT vec2Count = 0;
    gctINT vec1Count = 0;

    /*gcLINKTREE_OUTPUT  vec4Outputs[_MAX_VARYINGS];*/
    gcLINKTREE_OUTPUT  vec3Outputs[_MAX_VARYINGS];
    gcLINKTREE_OUTPUT  vec2Outputs[_MAX_VARYINGS * 2];
    gcLINKTREE_OUTPUT  vec1Outputs[_MAX_VARYINGS * 4];
    gcLINKTREE_OUTPUT  treeOutput;

    gcATTRIBUTE attribute;

    gcVaryingPacking varyingPacking[_MAX_VARYINGS];
    gctINT           packCount = 0;
    gctINT           skippedAttributeRows = 0;

    gcsList          vpMappingList;

    if (!gcOPT_doVaryingPackingForShader(VertexTree->shader))
    {
        return gcvSTATUS_OK;
    }

    /* sort outputs by size */
    for (j = 0; j < (gctINT)VertexTree->outputCount; ++j)
    {
        /* Get the gcOUTPUT pointer. */
        treeOutput = &VertexTree->outputArray[j];
        if (!treeOutput->inUse ||
            treeOutput->rows > 1 || treeOutput->isArray ||
            _isBuiltinOutput(VertexTree->shader->outputs[j]))
        {
            /* TODO: handle matrix and array */
            continue;
        }

        gcmASSERT(treeOutput->fragmentAttribute < (gctINT)FragmentTree->shader->attributeCount);
        attribute = FragmentTree->shader->attributes[treeOutput->fragmentAttribute];
        if (attribute && gcmATTRIBUTE_alwaysUsed(attribute))
            continue;

        switch (treeOutput->components) {
        case 1:
            vec1Outputs[vec1Count++] = treeOutput;
            break;
        case 2:
            vec2Outputs[vec2Count++] = treeOutput;
            break;
        case 3:
            vec3Outputs[vec3Count++] = treeOutput;
            break;
        default:
            gcmASSERT(treeOutput->components == 4);
            /*vec4Outputs[vec4Count++] = treeOutput;*/
            break;
        }
    }

    /* pack/merge outputs:
          vec4, ..., vec4,
          (vec2, vec2), ..., (vec2, vec2),
          (vec3, vec1), ..., (vec3, vec1),
          (vec2, vec1, vec1), ...
          vec3, vec3
          vec2
     */

    gcoOS_ZeroMemory(varyingPacking, sizeof(varyingPacking));

    /* vec2_2 packing modes */
    for (; vec2_i<vec2Count-1; )
    {
        gcLINKTREE_OUTPUT firstOutput, secondOutput;
        gcVaryingPacking * vp = &varyingPacking[packCount++];
        vp->mode = PackingVec2_2;
        firstOutput  = vec2Outputs[vec2_i++];
        secondOutput = vec2Outputs[vec2_i++];
        gcmASSERT(vec2_i <= vec2Count);
        gcmASSERT(firstOutput->inUse && firstOutput->rows == 1 && !firstOutput->isArray);
        gcmASSERT(secondOutput->inUse && secondOutput->rows == 1 && !secondOutput->isArray);

        firstOutput->isPacked         = gcvTRUE;
        firstOutput->packingMode      = PackingVec2_2;
        firstOutput->packedWith       = firstOutput->vsOutputIndex;
        vp->u.pack2_2[0].treeOutput   = firstOutput;
        vp->u.pack2_2[0].enabled      = gcSL_ENABLE_XY;
        vp->u.pack2_2[0].compmap      = gcCMAP_UNCHANGED;

        secondOutput->isPacked        = gcvTRUE;
        secondOutput->packingMode     = PackingVec2_2;
        secondOutput->packedWith      = firstOutput->vsOutputIndex;
        vp->u.pack2_2[1].treeOutput   = secondOutput;
        vp->u.pack2_2[1].enabled      = gcSL_ENABLE_ZW;
        vp->u.pack2_2[1].compmap      = gcCMAP_XY2ZW;
    }

    /* vec_3_1 packing */
    for (; vec3_i<vec3Count; )
    {
        gcLINKTREE_OUTPUT firstOutput, secondOutput;
        gcVaryingPacking * vp;
        if (vec1_i >= vec1Count)
            break;  /* no vec1 is available, bail out */

        vp = &varyingPacking[packCount++];
        vp->mode = PackingVec3_1;
        firstOutput  = vec3Outputs[vec3_i++];
        secondOutput = vec1Outputs[vec1_i++];

        gcmASSERT(firstOutput->inUse && firstOutput->rows == 1 && !firstOutput->isArray);
        gcmASSERT(secondOutput->inUse && secondOutput->rows == 1 && !secondOutput->isArray);
        firstOutput->isPacked         = gcvTRUE;
        firstOutput->packingMode      = PackingVec3_1;
        firstOutput->packedWith       = firstOutput->vsOutputIndex;
        vp->u.pack3_1[0].treeOutput   = firstOutput;
        vp->u.pack3_1[0].enabled      = gcSL_ENABLE_XYZ;
        vp->u.pack3_1[0].compmap      = gcCMAP_UNCHANGED;

        secondOutput->isPacked        = gcvTRUE;
        secondOutput->packingMode     = PackingVec3_1;
        secondOutput->packedWith      = firstOutput->vsOutputIndex;
        vp->u.pack3_1[1].treeOutput   = secondOutput;
        vp->u.pack3_1[1].enabled      = gcSL_ENABLE_W;
        vp->u.pack3_1[1].compmap      = gcCMAP_X2W;
    }

    /* handle remaining vec2: vec2_1_1 packing */
    if (vec2_i == vec2Count-1 && vec1_i < vec1Count)
    {
        /* pack vec2 and vec1 */
        gcLINKTREE_OUTPUT firstOutput, secondOutput;
        gcVaryingPacking * vp = &varyingPacking[packCount++];
        vp->mode = PackingVec2_1_1;
        firstOutput  = vec2Outputs[vec2_i++];
        secondOutput = vec1Outputs[vec1_i++];
        gcmASSERT(firstOutput->inUse && firstOutput->rows == 1 && !firstOutput->isArray);
        gcmASSERT(secondOutput->inUse && secondOutput->rows == 1 && !secondOutput->isArray);

        firstOutput->isPacked           = gcvTRUE;
        firstOutput->packingMode        = PackingVec2_1_1;
        firstOutput->packedWith         = firstOutput->vsOutputIndex;
        vp->u.pack2_1_1[0].treeOutput   = firstOutput;
        vp->u.pack2_1_1[0].enabled      = gcSL_ENABLE_XY;
        vp->u.pack2_1_1[0].compmap      = gcCMAP_UNCHANGED;

        secondOutput->isPacked          = gcvTRUE;
        secondOutput->packingMode       = PackingVec2_1_1;
        secondOutput->packedWith        = firstOutput->vsOutputIndex;
        vp->u.pack2_1_1[1].treeOutput   = secondOutput;
        vp->u.pack2_1_1[1].enabled      = gcSL_ENABLE_Z;
        vp->u.pack2_1_1[1].compmap      = gcCMAP_X2Z;

        /* more vec1? */
        if (vec1_i < vec1Count)
        {
            gcLINKTREE_OUTPUT thirdOutput   = vec1Outputs[vec1_i++];

            thirdOutput->isPacked           = gcvTRUE;
            thirdOutput->packingMode        = PackingVec2_1_1;
            thirdOutput->packedWith         = firstOutput->vsOutputIndex;
            vp->u.pack2_1_1[2].treeOutput   = thirdOutput;
            vp->u.pack2_1_1[2].enabled      = gcSL_ENABLE_W;
            vp->u.pack2_1_1[2].compmap      = gcCMAP_X2W;
        }
        else
        {
            vp->u.pack2_1_1[2].treeOutput   = gcvNULL;
        }
    }

    /* vec1_1_1_1 packing */
    for (; vec1_i<vec1Count-1; )
    {
        gcLINKTREE_OUTPUT firstOutput, secondOutput;
        gcVaryingPacking * vp;

        vp = &varyingPacking[packCount++];
        vp->mode = PackingVec1_1_1_1;
        firstOutput  = vec1Outputs[vec1_i++];
        secondOutput = vec1Outputs[vec1_i++];

        gcmASSERT(firstOutput->inUse && firstOutput->rows == 1 && !firstOutput->isArray);
        gcmASSERT(secondOutput->inUse && secondOutput->rows == 1 && !secondOutput->isArray);
        firstOutput->isPacked             = gcvTRUE;
        firstOutput->packingMode          = PackingVec1_1_1_1;
        firstOutput->packedWith           = firstOutput->vsOutputIndex;
        vp->u.pack1_1_1_1[0].treeOutput   = firstOutput;
        vp->u.pack1_1_1_1[0].enabled      = gcSL_ENABLE_X;
        vp->u.pack1_1_1_1[0].compmap      = gcCMAP_UNCHANGED;

        secondOutput->isPacked            = gcvTRUE;
        secondOutput->packingMode         = PackingVec1_1_1_1;
        secondOutput->packedWith          = firstOutput->vsOutputIndex;
        vp->u.pack1_1_1_1[1].treeOutput   = secondOutput;
        vp->u.pack1_1_1_1[1].enabled      = gcSL_ENABLE_Y;
        vp->u.pack1_1_1_1[1].compmap      = gcCMAP_X2Y;

        /* more vec1? */
        if (vec1_i < vec1Count)
        {
            gcLINKTREE_OUTPUT thirdOutput     = vec1Outputs[vec1_i++];
            thirdOutput->isPacked             = gcvTRUE;
            thirdOutput->packingMode          = PackingVec1_1_1_1;
            thirdOutput->packedWith           = firstOutput->vsOutputIndex;
            vp->u.pack1_1_1_1[2].treeOutput   = thirdOutput;
            vp->u.pack1_1_1_1[2].enabled      = gcSL_ENABLE_Z;
            vp->u.pack1_1_1_1[2].compmap      = gcCMAP_X2Z;
            /* more vec1? */
            if (vec1_i < vec1Count)
            {
                gcLINKTREE_OUTPUT fourthOutput    = vec1Outputs[vec1_i++];
                fourthOutput->isPacked            = gcvTRUE;
                fourthOutput->packingMode         = PackingVec1_1_1_1;
                fourthOutput->packedWith          = firstOutput->vsOutputIndex;
                vp->u.pack1_1_1_1[3].treeOutput   = fourthOutput;
                vp->u.pack1_1_1_1[3].enabled      = gcSL_ENABLE_W;
                vp->u.pack1_1_1_1[3].compmap      = gcCMAP_X2W;
            }
            else
            {
                vp->u.pack1_1_1_1[3].treeOutput   = gcvNULL;
            }
        }
        else
        {
            vp->u.pack1_1_1_1[2].treeOutput   = gcvNULL;
            vp->u.pack1_1_1_1[3].treeOutput   = gcvNULL;
        }
    }

    if (packCount == 0)
        return gcvSTATUS_OK;

    gcList_Init(&vpMappingList, (gcsAllocator *)&_vpAllocator);

    /* gathering varying pack mapping info */
    for (i = 0; i < packCount; i++)
    {
        /* add all the packed info to vpMappingList */
        gcVaryingPacking * vp = &varyingPacking[i];
        gcVaryingPackInfo * pi;
        switch (vp->mode) {
        case PackingVec3_1:
            pi = &vp->u.pack3_1[1];
            gcList_AddNode(&vpMappingList, pi);
            break;
        case PackingVec2_2:
            pi = &vp->u.pack2_2[1];
            gcList_AddNode(&vpMappingList, pi);
            break;
        case PackingVec2_1_1:
            pi = &vp->u.pack2_1_1[1];
            gcList_AddNode(&vpMappingList, pi);
            pi = &vp->u.pack2_1_1[2];
            if (pi->treeOutput)
            {
                gcList_AddNode(&vpMappingList, pi);
            }
            break;
        case PackingVec1_1_1_1:
            pi = &vp->u.pack1_1_1_1[1];
            gcList_AddNode(&vpMappingList, pi);
            pi = &vp->u.pack1_1_1_1[2];
            if (pi->treeOutput)
            {
                gcList_AddNode(&vpMappingList, pi);
                pi = &vp->u.pack1_1_1_1[3];
                if (pi->treeOutput)
                {
                    gcList_AddNode(&vpMappingList, pi);
                }
            }
            break;
        default:
            break;
        }
    }

    /* patching vertex shader:

         for each packed output, mapping the temp register and components to new one
     */
    _packVertexOutput(VertexTree, &vpMappingList);

    /* patch output info */
    for (i = 0; i < packCount; i++)
    {
        /* add all the packed info to vpMappingList */
        gcVaryingPacking *  vp = &varyingPacking[i];
        gcOUTPUT            output;
        gcVaryingPackInfo * pi0 = 0;
        gcVaryingPackInfo * pi;
        gctINT              components = 4; /* components in packed varying */

        switch (vp->mode) {
        case PackingVec3_1:
            pi0 = &vp->u.pack3_1[0];
            pi = &vp->u.pack3_1[1];
            _cleanupOutput(pi);
            VertexTree->packedAwayOutputCount++;
            break;
        case PackingVec2_2:
            pi0 = &vp->u.pack2_2[0];
            pi = &vp->u.pack2_2[1];
            _cleanupOutput(pi);
            VertexTree->packedAwayOutputCount++;
            break;
        case PackingVec2_1_1:
            pi0 = &vp->u.pack2_1_1[0];
            pi = &vp->u.pack2_1_1[1];
            _cleanupOutput(pi);
            VertexTree->packedAwayOutputCount++;
            pi = &vp->u.pack2_1_1[2];
            if (pi->treeOutput)
            {
                _cleanupOutput(pi);
                VertexTree->packedAwayOutputCount++;
            }
            else
                components = 3;
            break;
        case PackingVec1_1_1_1:
            pi0 = &vp->u.pack1_1_1_1[0];
            pi = &vp->u.pack1_1_1_1[1];
            _cleanupOutput(pi);
            VertexTree->packedAwayOutputCount++;
            pi = &vp->u.pack1_1_1_1[2];
            if (pi->treeOutput)
            {
                _cleanupOutput(pi);
                VertexTree->packedAwayOutputCount++;
                pi = &vp->u.pack1_1_1_1[3];
                if (pi->treeOutput)
                {
                    _cleanupOutput(pi);
                    VertexTree->packedAwayOutputCount++;
                }
                else
                    components = 3;
            }
            else
                components = 2;
            break;
        default:
            gcmASSERT(0);
            break;
        }
        /* change packed varying type */
        output = VertexTree->shader->outputs[pi0->treeOutput->vsOutputIndex];
        _changeTypeComponents(&output->type, components);
    }


    /* patching fragment shader */
    _packFragmentAttribute(FragmentTree, VertexTree, &vpMappingList);

    /* patch attribute info */
    for (i = 0; i < packCount; i++)
    {
        /* add all the packed info to vpMappingList */
        gcVaryingPacking *  vp = &varyingPacking[i];
        gcATTRIBUTE         attribute;
        gcVaryingPackInfo * pi0 = 0;
        gcVaryingPackInfo * pi;
        gctINT              components = 4; /* components in packed varying */

        switch (vp->mode) {
        case PackingVec3_1:
            pi0 = &vp->u.pack3_1[0];
            pi = &vp->u.pack3_1[1];
            _cleanupAttribute(FragmentTree, pi);
            break;
        case PackingVec2_2:
            pi0 = &vp->u.pack2_2[0];
            pi = &vp->u.pack2_2[1];
            _cleanupAttribute(FragmentTree, pi);
            break;
        case PackingVec2_1_1:
            pi0 = &vp->u.pack2_1_1[0];
            pi = &vp->u.pack2_1_1[1];
            _cleanupAttribute(FragmentTree, pi);
            pi = &vp->u.pack2_1_1[2];
            if (pi->treeOutput)
                _cleanupAttribute(FragmentTree, pi);
            else
                components = 3;
            break;
        case PackingVec1_1_1_1:
            pi0 = &vp->u.pack1_1_1_1[0];
            pi = &vp->u.pack1_1_1_1[1];
            _cleanupAttribute(FragmentTree, pi);
            pi = &vp->u.pack1_1_1_1[2];
            if (pi->treeOutput)
            {
                _cleanupAttribute(FragmentTree, pi);
                pi = &vp->u.pack1_1_1_1[3];
                if (pi->treeOutput)
                    _cleanupAttribute(FragmentTree, pi);
                else
                    components = 3;
            }
            else
                components = 2;
            break;
        default:
            gcmASSERT(0);
            break;
        }
        /* change packed varying type */
        attribute = FragmentTree->shader->attributes[pi0->treeOutput->fragmentAttribute];
        _changeTypeComponents(&attribute->type, components);
    }

    /* compute skippedFragmentAttributeRows for each treeOutput */
    for (i=0; i < (gctINT)FragmentTree->attributeCount; i++)
    {
        if (!FragmentTree->attributeArray[i].inUse)
        {
            gcATTRIBUTE attribute = FragmentTree->shader->attributes[i];
            gctUINT32 components = 0, rows = 0;
            /* get the rows from its type */
            if (attribute == gcvNULL)
                continue;

            gcTYPE_GetTypeInfo(attribute->type, &components, &rows, 0);
            rows *= attribute->arraySize;

            skippedAttributeRows += rows;
        }
        else
        {
            if (skippedAttributeRows > 0)
            {
                /* propagate the skipped rows to the correspoding output */
                gctINT j;
                for (j = 0; j < (gctINT)VertexTree->outputCount; j++)
                {
                    if (VertexTree->outputArray[j].fragmentAttribute == i)
                    {
                        /* found the matching output */
                        break;
                    }
                }
                /* there maybe more than one output mapped to the same attribute */
                while (j < (gctINT)VertexTree->outputCount &&
                       VertexTree->outputArray[j].fragmentAttribute == i)
                {
                    VertexTree->outputArray[j].skippedFragmentAttributeRows =
                        skippedAttributeRows;
                    j++;
                }
            }
        }
    }

    gcList_Clean(&vpMappingList, gcvFALSE);

    return gcvSTATUS_OK;
}

gctBOOL
_CheckSingleUniformStruct(
    IN gcLINKTREE VertexTree,
    IN gcLINKTREE FragmentTree,
    gcUNIFORM VertUniform,
    gcUNIFORM FragUniform,
    gctINT *UniformArray
    )
{
    gcUNIFORM vertChild, fragChild;
    gctBOOL match = gcvTRUE;
    gctBOOL isHaltiCompiler;

    gcmASSERT(VertUniform->firstChild < (gctINT16)VertexTree->shader->uniformCount &&
        FragUniform->firstChild < (gctINT16)FragmentTree->shader->uniformCount);

    vertChild = VertexTree->shader->uniforms[VertUniform->firstChild];
    fragChild = FragmentTree->shader->uniforms[FragUniform->firstChild];

    isHaltiCompiler = gcSHADER_IsHaltiCompiler(VertexTree->shader);

    while(vertChild && fragChild)
    {
        gcSHADER_TYPE       type1, type2;
        gcSHADER_PRECISION  prec1, prec2;
        gctUINT32           length1, length2;

        gcUNIFORM_GetTypeEx(vertChild, &type1, gcvNULL, &prec1, &length1);
        gcUNIFORM_GetTypeEx(fragChild, &type2, gcvNULL, &prec2, &length2);

        if (vertChild->nameLength != fragChild->nameLength ||
            (gcoOS_MemCmp(vertChild->name, fragChild->name, vertChild->nameLength) != gcvSTATUS_OK) ||
            (type1 != type2) ||

/* Do precision checking only for HALTI and on float types only, as
   general checking will bring es30 conformance test
*/
            (length1 != length2) ||
            (GetUniformCategory(vertChild) != GetUniformCategory(fragChild)))
        {
            match = gcvFALSE;
        }
        else if (isHaltiCompiler)  /* check precision for float type when HALTI */
        {
            if(gcmType_Kind(type1) == gceTK_FLOAT &&
               prec1 != prec2)
            {
                match = gcvFALSE;
            }
        }

        if (!isUniformStruct(vertChild) &&
            GetUniformBinding(vertChild) != GetUniformBinding(fragChild))
        {
            match = gcvFALSE;
        }

        if (match && isUniformStruct(vertChild))
        {
            match = _CheckSingleUniformStruct(VertexTree, FragmentTree, vertChild, fragChild, UniformArray);
            UniformArray[fragChild->index] = match ? 1 : 0;
        }

        if (!match)
            return gcvFALSE;

        if (vertChild->nextSibling == -1 || fragChild->nextSibling == -1)
            break;

        vertChild->matchIndex = fragChild->index;
        fragChild->matchIndex = vertChild->index;

        vertChild = VertexTree->shader->uniforms[vertChild->nextSibling];
        fragChild = FragmentTree->shader->uniforms[fragChild->nextSibling];
    }

    if (vertChild->nextSibling != fragChild->nextSibling)
        return gcvFALSE;

    return gcvTRUE;
}

static gcSHADER_PRECISION
_fixUniformPrecision(
    IN gcSHADER Shader,
    IN gcSHADER_PRECISION precision,
    IN gcSHADER_TYPE dataType,
    IN gcSHADER_KIND shaderType
    )
{
    /* Get real precision if default.
    ** Expand "default" to one of "lowp", "mediump" or "highp".
    */
    if (gcSHADER_PRECISION_DEFAULT == precision)
    {
        switch (dataType)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
        case gcSHADER_FLOAT_2X2:
        case gcSHADER_FLOAT_3X3:
        case gcSHADER_FLOAT_4X4:
        case gcSHADER_FLOAT_2X3:
        case gcSHADER_FLOAT_2X4:
        case gcSHADER_FLOAT_3X2:
        case gcSHADER_FLOAT_3X4:
        case gcSHADER_FLOAT_4X2:
        case gcSHADER_FLOAT_4X3:
            /* change "default" to "highp" for VS float or PS float in ES20 */
            if (!gcSHADER_IsHaltiCompiler(Shader) || gcSHADER_TYPE_VERTEX == shaderType)
            {
                precision = gcSHADER_PRECISION_HIGH;
            }
            break;

        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:
            /* change "default" to "highp" for VS int */
            if (gcSHADER_TYPE_VERTEX == shaderType)
            {
                precision = gcSHADER_PRECISION_HIGH;
            }
            /* change "default" to "mediump" for PS int */
            else if (gcSHADER_TYPE_FRAGMENT == shaderType)
            {
                precision = gcSHADER_PRECISION_MEDIUM;
            }
            break;

        case gcSHADER_SAMPLER_2D:
        case gcSHADER_SAMPLER_CUBIC:
            /* "lowp" for sampler2D and samplercube as spec requires */
            precision = gcSHADER_PRECISION_LOW;
            break;

        default:
            break;
        }
    }

    return precision;
}

/*******************************************************************************
**                               gcLINKTREE_Link
********************************************************************************
**
**  Link the output of the vertex tree to the attributes of the fragment tree.
**
**  INPUT:
**
**      gcLINKTREE VertexTree
**          Pointer to a gcLINKTREE structure representing the vertex shader.
**
**      gcLINKTREE FragmentTree
**          Pointer to a gcLINKTREE structure representing the fragment shader.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcLINKTREE_Link(
    IN gcLINKTREE* ShaderTrees,
    IN gctBOOL     DoVaryingPacking
    )
{
    gctUINT i, j;
    gctINT k;
    gctINT index = 0;
    gctPOINTER pointer = gcvNULL;
    gctINT *uniformArray;
    gceSTATUS status = gcvSTATUS_OK;
    gcLINKTREE VertexTree = ShaderTrees[gceSGSK_VERTEX_SHADER],
               FragmentTree = ShaderTrees[gceSGSK_FRAGMENT_SHADER];

    if (VertexTree == gcvNULL || FragmentTree == gcvNULL)
    {
        /* no kinking needed if any tree is empty */
        return status;
    }
    /***************************************************************************
    ** I - Walk all attributes in the fragment tree and find the corresponding
    **     vertex output.
    */

    if (!ENABLE_FULL_NEW_LINKER)
    {
        /* Walk all attributes. */
        for (i = 0; i < FragmentTree->attributeCount; ++i)
        {
            /* Get the gcATTRIBUTE pointer. */
            gcATTRIBUTE attribute = FragmentTree->shader->attributes[i];
            /* Only process valid attributes. */
            if (attribute == gcvNULL || gcmATTRIBUTE_packedAway(attribute))
            {
                continue;
            }

            if (
                (ENABLE_FULL_NEW_LINKER && attribute->nameLength == gcSL_POINT_COORD) ||
                attribute->nameLength == gcSL_FRONT_FACING ||
                attribute->nameLength == gcSL_HELPER_INVOCATION)
            {
                continue;
            }

            /* Walk all vertex outputs. */
            for (j = 0; j < VertexTree->outputCount; ++j)
            {
                /* Get the gcOUTPUT pointer. */
                gcOUTPUT output = VertexTree->shader->outputs[j];

                /* Output already got killed before. */
                if (output == gcvNULL)
                {
                    continue;
                }

                /* Compare the names of the output and attribute. */
                if ((output->nameLength == attribute->nameLength
                &&  (((gctINT) output->nameLength < 0)
                    || (gcmIS_SUCCESS(gcoOS_MemCmp(output->name,
                                                  attribute->name,
                                                  output->nameLength)) && (attribute->location == -1 || output->location == -1))))
                    || (output->location == attribute->location &&
                        attribute->location != -1 && output->type == attribute->type &&
                        output->precision == attribute->precision)
                )
                {
                    gctUINT32 components = 0, rows = 0;

                    if (output->shaderMode != attribute->shaderMode)
                    {
                        return gcvSTATUS_VARYING_TYPE_MISMATCH;
                    }

                    /* Make sure the varying variables are of the same type. */
                    if ((output->type != attribute->type)
                    ||  (output->arraySize != attribute->arraySize)
                    )
                    {
                        /* Type mismatch. */
                        return gcvSTATUS_VARYING_TYPE_MISMATCH;
                    }

                    /* For a ES20 shader, invariant of varyings on VS and PS must match. */
                    if (!gcSHADER_IsHaltiCompiler(VertexTree->shader) &&
                        gcmOUTPUT_isInvariant(output) != gcmATTRIBUTE_isInvariant(attribute))
                    {
                        return gcvSTATUS_VARYING_TYPE_MISMATCH;
                    }

                    /* Determine rows and components. */
                    gcTYPE_GetTypeInfo(attribute->type,
                                       &components, &rows, 0);
                    rows *= attribute->arraySize;

                    /* Only mapping the used attributes. */
                    if (FragmentTree->attributeArray[i].inUse)
                    {
                        for (k = 0; k < attribute->arraySize; ++k)
                        {
                            /* Mark vertex output as in-use and link to fragment attribute. */
                            gcLINKTREE_OUTPUT outputInfo;
                            gcmASSERT(j + k < VertexTree->outputCount);
                            outputInfo = &VertexTree->outputArray[j + k];
                            outputInfo->inUse             = gcvTRUE;
                            outputInfo->isArray           = (attribute->arraySize > 1);
                            outputInfo->isPacked          = gcvFALSE;
                            outputInfo->packedWith        = j;  /* init value: packed with itself */
                            outputInfo->fragmentAttribute = i;
                            outputInfo->components        = components;
                            outputInfo->rows              = rows;
                            outputInfo->elementInArray    = k;
                            outputInfo->vsOutputIndex     = j;
                            outputInfo->fragmentIndex     = index;

                            if (output->nameLength != gcSL_POSITION)
                            {
                                index += gcmType_Rows(attribute->type);
                                outputInfo->fragmentIndexEnd = index - 1;
                            }
                            else
                            {
                                /* Actually fragmentIndex and fragmentIndexEnd are no meanful for POSITION since
                                 * it always be put at first attribute and assign it to R0 */
                                outputInfo->fragmentIndexEnd = index;
                            }
                        }
                    }

                    /* Stop looping. */
                    break;
                }
            }

            if (j == VertexTree->outputCount && FragmentTree->attributeArray[i].inUse)
            {
                gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: input %s in fragment shader is \"%s\" "
                         "undefined in vertex shader", attribute->name);
                /* No matching vertex output found. */
                return gcvSTATUS_UNDECLARED_VARYING;
            }
        }
    }

    /******************************************************************************
    ** II - Walk all uniform blocks in the fragment tree and find the corresponding
    **      uniform blocks in the vertex tree.
    */

    /* Check uniform block size. */
    for (i = 0; i < VertexTree->shader->uniformBlockCount; ++i)
    {
        gcsUNIFORM_BLOCK uniformBlock = VertexTree->shader->uniformBlocks[i];
        /* Only process uniform blocks. */
        if (uniformBlock == gcvNULL ||
            GetUBBlockIndex(uniformBlock) == VertexTree->shader->_defaultUniformBlockIndex || /* artificial UBO's */
            GetUBBlockIndex(uniformBlock) == VertexTree->shader->constUniformBlockIndex)
        {
            continue;
        }

        if (GetUBBlockSize(uniformBlock) > GetGLMaxUniformBLockSize())
        {
            return gcvSTATUS_TOO_MANY_UNIFORMS;
        }
    }

    for (i = 0; i < FragmentTree->shader->uniformBlockCount; ++i)
    {
        gcsUNIFORM_BLOCK uniformBlock = FragmentTree->shader->uniformBlocks[i];
        /* Only uniformBlock uniform blocks. */
        if (uniformBlock == gcvNULL ||
            GetUBBlockIndex(uniformBlock) == FragmentTree->shader->_defaultUniformBlockIndex || /* artificial UBO's */
            GetUBBlockIndex(uniformBlock) == FragmentTree->shader->constUniformBlockIndex)
        {
            continue;
        }

        if (GetUBBlockSize(uniformBlock) > GetGLMaxUniformBLockSize())
        {
            return gcvSTATUS_TOO_MANY_UNIFORMS;
        }
    }

    /* Walk all uniform block. */
    for (i = 0; i < VertexTree->shader->uniformBlockCount; ++i)
    {
        gcsUNIFORM_BLOCK vertUniformBlock = VertexTree->shader->uniformBlocks[i];
        gcUNIFORM vertUniform, fragUniform;

        /* Only process uniform blocks. */
        if (vertUniformBlock == gcvNULL ||
            GetUBBlockIndex(vertUniformBlock) == VertexTree->shader->_defaultUniformBlockIndex || /* artificial UBO's */
            GetUBBlockIndex(vertUniformBlock) == VertexTree->shader->constUniformBlockIndex)
        {
            continue;
        }

        /* Walk all fragment uniform blocks. */
        for (j = 0; j < FragmentTree->shader->uniformBlockCount; ++j)
        {
            gctINT16 vertBlockMember, fragBlockMember;
            gceSTATUS status;

            /* Get the gcOUTPUT pointer. */
            gcsUNIFORM_BLOCK fragUniformBlock = FragmentTree->shader->uniformBlocks[j];

            if (fragUniformBlock == gcvNULL) continue;

            /* Compare the names of the vertex uniform block and fragment uniform block. */
            if (vertUniformBlock->nameLength == fragUniformBlock->nameLength
                && gcmIS_SUCCESS(gcoOS_MemCmp(vertUniformBlock->name,
                                              fragUniformBlock->name,
                                              vertUniformBlock->nameLength)))
            {
                gctBOOL mismatch = gcvFALSE;

                /* check uniform block array size to be the same */
                if((GetUBPrevSibling(vertUniformBlock) != -1 && GetUBPrevSibling(fragUniformBlock) == -1) ||
                   (GetUBPrevSibling(vertUniformBlock) == -1 && GetUBPrevSibling(fragUniformBlock) != -1) ||
                   (GetUBNextSibling(vertUniformBlock) != -1 && GetUBNextSibling(fragUniformBlock) == -1) ||
                   (GetUBNextSibling(vertUniformBlock) == -1 && GetUBNextSibling(fragUniformBlock) != -1))
                {
                    mismatch = gcvTRUE;
                }
                else if(GetUBPrevSibling(vertUniformBlock) == -1 &&
                        GetUBNextSibling(vertUniformBlock) != -1)
                {

                    gcsUNIFORM_BLOCK vertArrayElementBlock, fragArrayElementBlock;

                    vertArrayElementBlock = vertUniformBlock;
                    fragArrayElementBlock = fragUniformBlock;
                    while (GetUBNextSibling(vertArrayElementBlock) != -1 &&
                           GetUBNextSibling(fragArrayElementBlock) != -1)
                    {
                        status =  gcSHADER_GetUniformBlock(VertexTree->shader,
                                                           GetUBNextSibling(vertArrayElementBlock),
                                                           &vertArrayElementBlock);
                        if (gcmIS_ERROR(status))
                        {
                            /* Return on error. */
                            return status;
                        }

                        status =  gcSHADER_GetUniformBlock(FragmentTree->shader,
                                                           GetUBNextSibling(fragArrayElementBlock),
                                                           &fragArrayElementBlock);
                        if (gcmIS_ERROR(status))
                        {
                            /* Return on error. */
                            return status;
                        }
                    }
                    if(GetUBNextSibling(vertArrayElementBlock) != -1 ||
                       GetUBNextSibling(fragArrayElementBlock) != -1)
                    {
                        mismatch = gcvTRUE;
                    }
                }

                if(mismatch)
                {
                     gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader uniform block \"%s\" array size "
                              "does not match with corresponding uniform block array size in fragment shader",
                              GetUBName(vertUniformBlock));

                     return gcvSTATUS_UNIFORM_MISMATCH;
                }

                /* Make sure the block memory layout are of the same. */
                if(GetUBMemoryLayout(vertUniformBlock) != GetUBMemoryLayout(fragUniformBlock))
                {
                    gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader uniform block \"%\" memory layout "
                             "does not match with corresponding uniform block in fragment shader",
                             GetUBName(vertUniformBlock));

                    return gcvSTATUS_UNIFORM_MISMATCH;

                }

                if (GetUBBinding(vertUniformBlock) != GetUBBinding(fragUniformBlock))
                {
                    gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader uniform block \"%s\" binding "
                                "does not match with corresponding uniform block binding in fragment shader",
                                GetUBName(vertUniformBlock));

                    return gcvSTATUS_UNIFORM_MISMATCH;
                }

                /* Make sure the block memory layout are of the same.
                ** Default UBO might not have same number of uniforms
                */
                if(gcoOS_StrCmp(GetUBName(vertUniformBlock), "#DefaultUBO") != gcvSTATUS_OK &&
                    gcoOS_StrCmp(GetUBName(fragUniformBlock), "#DefaultUBO") != gcvSTATUS_OK &&
                    GetUBNumBlockElement(vertUniformBlock) != GetUBNumBlockElement(fragUniformBlock))
                {
                    gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader uniform block \"%\" member count "
                             "does not match with corresponding uniform block in fragment shader",
                             GetUBName(vertUniformBlock));

                    return gcvSTATUS_UNIFORM_MISMATCH;

                }
                /* Make sure the block members are of the same type. */
                for(k = 0, vertBlockMember = GetUBFirstChild(vertUniformBlock),
                           fragBlockMember = GetUBFirstChild(fragUniformBlock);
                        k < GetUBNumBlockElement(vertUniformBlock); k++)
                {
                    gceUNIFORM_FLAGS vertUniformFlag, fragUniformFlag;

                    gcmASSERT(vertBlockMember != -1);
                    gcmASSERT(fragBlockMember != -1);

                    status =  gcSHADER_GetUniform(VertexTree->shader,
                                                  vertBlockMember,
                                                  &vertUniform);
                    if (gcmIS_ERROR(status))
                    {
                        /* Return on error. */
                        return status;
                    }

                    status =  gcSHADER_GetUniform(FragmentTree->shader,
                                                  fragBlockMember,
                                                  &fragUniform);
                    if (gcmIS_ERROR(status))
                    {
                        /* Return on error. */
                        return status;
                    }

                    /* We don't need to check a uniform block member is active or not. */
                    vertUniformFlag = GetUniformFlags(vertUniform) & (~gcvUNIFORM_FLAG_IS_INACTIVE);
                    fragUniformFlag = GetUniformFlags(fragUniform) & (~gcvUNIFORM_FLAG_IS_INACTIVE);

                    if (vertUniform->nameLength != fragUniform->nameLength
                        || !gcmIS_SUCCESS(gcoOS_MemCmp(vertUniform->name,
                                                      fragUniform->name,
                                                      vertUniform->nameLength)))
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader uniform block member name \"%s\" "
                                 "does not match with corresponding uniform block member name \"%s\" in fragment shader",
                                 vertUniform->name, fragUniform->name);

                        return gcvSTATUS_UNIFORM_MISMATCH;
                    }
                    if (vertUniform->isRowMajor != fragUniform->isRowMajor)
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader uniform block member \"%s\" layout "
                                 "does not match with corresponding uniform block member layout in fragment shader",
                                 vertUniform->name);

                        return gcvSTATUS_UNIFORM_MISMATCH;
                    }
                    if (GetUniformCategory(vertUniform) != GetUniformCategory(fragUniform) ||
                        vertUniform->u.type != fragUniform->u.type)
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader uniform block member \"%s\" type "
                                 "does not match with corresponding uniform block member type in fragment shader",
                                 vertUniform->name);

                        return gcvSTATUS_UNIFORM_MISMATCH;
                    }


                    if ((GetUniformFlagsSpecialKind(vertUniformFlag) != GetUniformFlagsSpecialKind(fragUniformFlag))
                        || (vertUniform->arraySize != fragUniform->arraySize))
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader uniform block member \"%s\" array size %d "
                                 "does not match with corresponding uniform block member array size %d in fragment shader",
                                 vertUniform->name, vertUniform->arraySize, fragUniform->arraySize);

                        return gcvSTATUS_UNIFORM_MISMATCH;
                    }

                    if (GetUniformBinding(vertUniform) != GetUniformBinding(fragUniform))
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader uniform block member \"%s\" binding "
                                 "does not match with corresponding uniform block member binding in fragment shader",
                                 vertUniform->name);

                        return gcvSTATUS_UNIFORM_MISMATCH;
                    }

                    vertUniform->matchIndex = fragUniform->index;
                    fragUniform->matchIndex = vertUniform->index;

                    vertBlockMember = vertUniform->nextSibling;
                    fragBlockMember = fragUniform->nextSibling;
                }

                /* This UBO is matched with a PS UBO, save the match index for it. */
                SetUBMatchIndex(vertUniformBlock, GetUBIndex(fragUniformBlock));
                SetUBMatchIndex(fragUniformBlock, GetUBIndex(vertUniformBlock));
            }
        }
    }


     /******************************************************************************
    ** III - Walk all strorage blocks in the fragment tree and find the corresponding
    **      strorage blocks in the vertex tree.
    */

    /* Walk all storage blocks. */
    for (i = 0; i < VertexTree->shader->storageBlockCount; ++i)
    {
        gcsSTORAGE_BLOCK vertStorageBlock = VertexTree->shader->storageBlocks[i];

        if (vertStorageBlock == gcvNULL)
        {
            continue;
        }

        /* Walk all fragment storage blocks. */
        for (j = 0; j < FragmentTree->shader->storageBlockCount; ++j)
        {
            gceSTATUS status;

            gcsSTORAGE_BLOCK fragStorageBlock = FragmentTree->shader->storageBlocks[j];

            if (fragStorageBlock == gcvNULL) continue;

            /* Compare the names of the vertex storage block and fragment storage block. */
            if (vertStorageBlock->nameLength == fragStorageBlock->nameLength
                && gcmIS_SUCCESS(gcoOS_MemCmp(vertStorageBlock->name,
                                              fragStorageBlock->name,
                                              vertStorageBlock->nameLength)))
            {
                gctBOOL mismatch = gcvFALSE;

                /* check storage block array size to be the same */
                if((GetSBPrevSibling(vertStorageBlock) != -1 && GetSBPrevSibling(fragStorageBlock) == -1) ||
                   (GetSBPrevSibling(vertStorageBlock) == -1 && GetSBPrevSibling(fragStorageBlock) != -1) ||
                   (GetSBNextSibling(vertStorageBlock) != -1 && GetSBNextSibling(fragStorageBlock) == -1) ||
                   (GetSBNextSibling(vertStorageBlock) == -1 && GetSBNextSibling(fragStorageBlock) != -1))
                {
                    mismatch = gcvTRUE;
                }
                else if(GetSBPrevSibling(vertStorageBlock) == -1 &&
                        GetSBNextSibling(vertStorageBlock) != -1)
                {

                    gcsSTORAGE_BLOCK vertArrayElementBlock, fragArrayElementBlock;

                    vertArrayElementBlock = vertStorageBlock;
                    fragArrayElementBlock = fragStorageBlock;
                    while (GetSBNextSibling(vertArrayElementBlock) != -1 &&
                           GetSBNextSibling(fragArrayElementBlock) != -1)
                    {
                        status =  gcSHADER_GetStorageBlock(VertexTree->shader,
                                                           GetSBNextSibling(vertArrayElementBlock),
                                                           &vertArrayElementBlock);
                        if (gcmIS_ERROR(status))
                        {
                            /* Return on error. */
                            return status;
                        }

                        status =  gcSHADER_GetStorageBlock(FragmentTree->shader,
                                                           GetSBNextSibling(fragArrayElementBlock),
                                                           &fragArrayElementBlock);
                        if (gcmIS_ERROR(status))
                        {
                            /* Return on error. */
                            return status;
                        }
                    }
                    if(GetSBNextSibling(vertArrayElementBlock) != -1 ||
                       GetSBNextSibling(fragArrayElementBlock) != -1)
                    {
                        mismatch = gcvTRUE;
                    }
                }

                if(mismatch)
                {
                     gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block \"%s\" array size "
                              "does not match with corresponding storage block array size in fragment shader",
                              GetSBName(vertStorageBlock));

                     return gcvSTATUS_SSBO_MISMATCH;
                }

                /* Make sure the block memory layout are of the same. */
                if(GetSBMemoryLayout(vertStorageBlock) != GetSBMemoryLayout(fragStorageBlock))
                {
                    gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block \"%\" memory layout "
                             "does not match with corresponding storage block in fragment shader",
                             GetSBName(vertStorageBlock));
                    return gcvSTATUS_SSBO_MISMATCH;

                }

                if (GetSBBinding(vertStorageBlock) != GetSBBinding(fragStorageBlock))
                {
                    gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block \"%s\" binding "
                                "does not match with corresponding storage block binding in fragment shader",
                                GetSBName(vertStorageBlock));

                    return gcvSTATUS_SSBO_MISMATCH;
                }

                /* This storage block is matched with a PS storage block, save the match index for it. */
                SetSBMatchIndex(vertStorageBlock, GetSBIndex(fragStorageBlock));
                SetSBMatchIndex(fragStorageBlock, GetSBIndex(vertStorageBlock));
            }
        }
    }

    for (i = 0; i < VertexTree->shader->variableCount; ++i)
    {
        gcsSTORAGE_BLOCK vertStorageBlock;
        gcVARIABLE vertVariable = VertexTree->shader->variables[i];
        gctINT16 vertBlockIndex;

        if (vertVariable == gcvNULL || !isVariableBlockMember(vertVariable))
        {
            continue;
        }

        vertBlockIndex = GetVariableBlockID(vertVariable);
        vertStorageBlock = VertexTree->shader->storageBlocks[vertBlockIndex];

        if (GetSBFlag(vertStorageBlock) & gceIB_FLAG_WITH_INSTANCE_NAME)
        {
            continue;
        }

        /* Walk all fragment storage blocks. */
        for (j = 0; j < FragmentTree->shader->variableCount; ++j)
        {
            gcsSTORAGE_BLOCK fragStorageBlock;
            gcVARIABLE fragVariable = FragmentTree->shader->variables[j];
            gctINT16 fragBlockIndex;

            if (fragVariable == gcvNULL || !isVariableBlockMember(fragVariable))
            {
                continue;
            }

            fragBlockIndex = GetVariableBlockID(fragVariable);
            fragStorageBlock = FragmentTree->shader->storageBlocks[fragBlockIndex];

            if (GetSBFlag(fragStorageBlock) & gceIB_FLAG_WITH_INSTANCE_NAME)
            {
                continue;
            }

            /* If variables name match, then IO block must match too. */
            if (vertVariable->nameLength == fragVariable->nameLength
                && gcmIS_SUCCESS(gcoOS_MemCmp(vertVariable->name,
                                              fragVariable->name,
                                              vertVariable->nameLength)))
            {
                /* Compare the names of the vertex storage block and fragment storage block. */
                if (vertStorageBlock->nameLength == fragStorageBlock->nameLength
                    && gcmIS_SUCCESS(gcoOS_MemCmp(vertStorageBlock->name,
                                                  fragStorageBlock->name,
                                                  vertStorageBlock->nameLength)))
                {
                    gctBOOL mismatch = gcvFALSE;

                    /* check storage block array size to be the same */
                    if((GetSBPrevSibling(vertStorageBlock) != -1 && GetSBPrevSibling(fragStorageBlock) == -1) ||
                       (GetSBPrevSibling(vertStorageBlock) == -1 && GetSBPrevSibling(fragStorageBlock) != -1) ||
                       (GetSBNextSibling(vertStorageBlock) != -1 && GetSBNextSibling(fragStorageBlock) == -1) ||
                       (GetSBNextSibling(vertStorageBlock) == -1 && GetSBNextSibling(fragStorageBlock) != -1))
                    {
                        mismatch = gcvTRUE;
                    }
                    else if(GetSBPrevSibling(vertStorageBlock) == -1 &&
                            GetSBNextSibling(vertStorageBlock) != -1)
                    {

                        gcsSTORAGE_BLOCK vertArrayElementBlock, fragArrayElementBlock;

                        vertArrayElementBlock = vertStorageBlock;
                        fragArrayElementBlock = fragStorageBlock;
                        while (GetSBNextSibling(vertArrayElementBlock) != -1 &&
                               GetSBNextSibling(fragArrayElementBlock) != -1)
                        {
                            status =  gcSHADER_GetStorageBlock(VertexTree->shader,
                                                               GetSBNextSibling(vertArrayElementBlock),
                                                               &vertArrayElementBlock);
                            if (gcmIS_ERROR(status))
                            {
                                /* Return on error. */
                                return status;
                            }

                            status =  gcSHADER_GetStorageBlock(FragmentTree->shader,
                                                               GetSBNextSibling(fragArrayElementBlock),
                                                               &fragArrayElementBlock);
                            if (gcmIS_ERROR(status))
                            {
                                /* Return on error. */
                                return status;
                            }
                        }
                        if(GetSBNextSibling(vertArrayElementBlock) != -1 ||
                           GetSBNextSibling(fragArrayElementBlock) != -1)
                        {
                            mismatch = gcvTRUE;
                        }
                    }

                    if(mismatch)
                    {
                         gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block \"%s\" array size "
                                  "does not match with corresponding storage block array size in fragment shader",
                                  GetSBName(vertStorageBlock));

                         return gcvSTATUS_SSBO_MISMATCH;
                    }

                    /* Make sure the block memory layout are of the same. */
                    if(GetSBMemoryLayout(vertStorageBlock) != GetSBMemoryLayout(fragStorageBlock))
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block \"%\" memory layout "
                                 "does not match with corresponding storage block in fragment shader",
                                 GetSBName(vertStorageBlock));
                        return gcvSTATUS_SSBO_MISMATCH;

                    }

                    if (GetSBBinding(vertStorageBlock) != GetSBBinding(fragStorageBlock))
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block \"%s\" binding "
                                    "does not match with corresponding storage block binding in fragment shader",
                                    GetSBName(vertStorageBlock));

                        return gcvSTATUS_SSBO_MISMATCH;
                    }

                    /* This storage block is matched with a PS storage block, save the match index for it. */
                    SetSBMatchIndex(vertStorageBlock, GetSBIndex(fragStorageBlock));
                    SetSBMatchIndex(fragStorageBlock, GetSBIndex(vertStorageBlock));
                }
                else
                {
                    return gcvSTATUS_SSBO_MISMATCH;
                }
                break;
            }
        }
    }

    if (!ENABLE_FULL_NEW_LINKER)
    {
        /***************************************************************************
        ** Walk all uniform struct in the fragment tree and find the corresponding
        **      uniform struct in the vertex tree.
        */
        if (FragmentTree->shader->uniformCount != 0)
            status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(gctINT) * FragmentTree->shader->uniformCount, &pointer);

        if (gcmIS_ERROR(status))
        {
            /* Return on error. */
            return status;
        }

        uniformArray = pointer;
        for (i = 0; i < FragmentTree->shader->uniformCount; i++)
        {
            uniformArray[i] = -1;
        }

        for (i = 0; i < FragmentTree->shader->uniformCount; i++)
        {
            gcUNIFORM vertUniform, fragUniform;
            gctBOOL match;
            gcSHADER_TYPE       type1, type2;
            gcSHADER_PRECISION  prec1, prec2;
            gctUINT32           length1, length2;

            fragUniform = FragmentTree->shader->uniforms[i];

            if (!isUniformStruct(fragUniform)) continue;

            /* If uniformArray is 1, it means this struct have been checked before,
            ** so we don't need to check again.
            */
            if (uniformArray[i] == 1)
                continue;

            for (j = 0; j < VertexTree->shader->uniformCount; j++)
            {
                match = gcvTRUE;
                vertUniform = VertexTree->shader->uniforms[j];

                if (!(vertUniform->nameLength == fragUniform->nameLength &&
                    gcmIS_SUCCESS(gcoOS_MemCmp(vertUniform->name,
                                                          fragUniform->name,
                                                          vertUniform->nameLength)))) continue;

                gcUNIFORM_GetTypeEx(vertUniform, &type1, gcvNULL, &prec1, &length1);
                gcUNIFORM_GetTypeEx(fragUniform, &type2, gcvNULL, &prec2, &length2);

                if (type1 != type2 || prec1 != prec2 || length1 != length2 ||
                    GetUniformCategory(vertUniform) != GetUniformCategory(fragUniform))
                {
                    match = gcvFALSE;
                }

                if (match && !_CheckSingleUniformStruct(VertexTree, FragmentTree, vertUniform, fragUniform, uniformArray))
                {
                    match = gcvFALSE;
                    uniformArray[i] = 0;
                }

                if (!match)
                {
                    gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: fragment shader uniform struct \"%s\" "
                             "does not match with corresponding uniform in vertex shader", vertUniform->name);
                    gcmOS_SAFE_FREE(gcvNULL, uniformArray);
                    return gcvSTATUS_UNIFORM_MISMATCH;
                }

                vertUniform->matchIndex = fragUniform->index;
                fragUniform->matchIndex = vertUniform->index;

                uniformArray[i] = 1;

                break;
            }
        }

        if (FragmentTree->shader->uniformCount != 0)
            gcmOS_SAFE_FREE(gcvNULL, uniformArray);

        /***************************************************************************
        ** Walk all normal uniforms in the fragment tree and
        **      find the corresponding uniforms in the vertex tree.
        */
        for (i = 0; i < FragmentTree->shader->uniformCount; i++)
        {
            gcUNIFORM vertUniform, fragUniform;
            gctBOOL match;
            gcSHADER_TYPE       type1, type2;
            gcSHADER_PRECISION  prec1, prec2;
            gctUINT32           length1, length2;

            fragUniform = FragmentTree->shader->uniforms[i];

            if (!isUniformNormal(fragUniform)) continue;

            if (fragUniform->parent != -1) continue;

            for (j = 0; j < VertexTree->shader->uniformCount; j++)
            {
                match = gcvTRUE;
                vertUniform = VertexTree->shader->uniforms[j];

                if (!(vertUniform->nameLength == fragUniform->nameLength &&
                    gcmIS_SUCCESS(gcoOS_MemCmp(vertUniform->name,
                                                          fragUniform->name,
                                                          vertUniform->nameLength)))) continue;

                gcUNIFORM_GetTypeEx(vertUniform, &type1, gcvNULL, &prec1, &length1);
                gcUNIFORM_GetTypeEx(fragUniform, &type2, gcvNULL, &prec2, &length2);

                if (type1 != type2 || length1 != length2 ||
                    GetUniformCategory(vertUniform) != GetUniformCategory(fragUniform))
                {
                    match = gcvFALSE;
                }

                /* We need to fix precision for es20/es30 shader. */
                prec1 = _fixUniformPrecision(VertexTree->shader, prec1, type1, gcSHADER_TYPE_VERTEX);
                prec2 = _fixUniformPrecision(FragmentTree->shader, prec2, type2, gcSHADER_TYPE_FRAGMENT);
                if (prec1 != prec2)
                {
                    match = gcvFALSE;
                }

                if (isUniformImage(fragUniform) && fragUniform->imageFormat != vertUniform->imageFormat)
                {
                    match = gcvFALSE;
                }

                if (GetUniformBinding(fragUniform) != GetUniformBinding(vertUniform))
                {
                    match = gcvFALSE;
                }

                if (!match)
                {
                    gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: fragment shader uniform \"%s\" "
                             "does not match with corresponding uniform in vertex shader", vertUniform->name);
                    return gcvSTATUS_UNIFORM_MISMATCH;
                }

                vertUniform->matchIndex = fragUniform->index;
                fragUniform->matchIndex = vertUniform->index;
                break;
            }
        }
    }

    /***************************************************************************
    ** III - Walk all outputs in the vertex tree to find any dead attribute.
    */

    if (!ENABLE_FULL_NEW_LINKER)
    {
        /* Walk all outputs. */
        for (i = 0; i < VertexTree->outputCount; i++)
        {
            gcOUTPUT output = VertexTree->shader->outputs[i];
            /* Output already got killed before. */
            if (output == gcvNULL)
            {
                continue;
            }

            /* check if the output is used for transform feedback */
            if (VertexTree->shader->transformFeedback.varyingCount > 0)
            {
                gctUINT i1;
                for (i1 = 0; i1 < VertexTree->shader->transformFeedback.varyingCount; i1++)
                {
                    gcOUTPUT varying = VertexTree->shader->transformFeedback.varyings[i1].output;
                    if (varying == output)
                    {
                        VertexTree->outputArray[i].isTransformFeedback = gcvTRUE;
                        break;
                    }
                }
            }

            /* Check if vertex output is dead. */
            if (!(VertexTree->outputArray[i].inUse ||
                  VertexTree->outputArray[i].isTransformFeedback)
            &&  ((gctINT) output->nameLength > 0)
            )
            {
                /* Mark vertex output as dead. */
                VertexTree->outputArray[i].tempHolding = -1;

                /* Free the gcOUTPUT structure from the shader. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL,
                                        VertexTree->shader->outputs[i]));

                /* Mark the output as invalid. */
                VertexTree->shader->outputs[i] = gcvNULL;
            }
        }
    }

    /******************************************************************************
    ** IV - Walk all storage blocks in the fragment tree and find the corresponding
    **      storage blocks in the vertex tree.
    */

    /* Walk all storage block. */
    for (i = 0; i < VertexTree->shader->storageBlockCount; ++i)
    {
        /* Get the gcATTRIBUTE pointer. */
        gcsSTORAGE_BLOCK vertStorageBlock = VertexTree->shader->storageBlocks [i];
        gcVARIABLE vertStorageVariable, fragStorageVariable;

        /* Walk all fragment storage blocks. */
        for (j = 0; j < FragmentTree->shader->storageBlockCount; ++j)
        {
            gctINT16 vertBlockMember, fragBlockMember;
            gceSTATUS status;

            /* Get the gcOUTPUT pointer. */
            gcsSTORAGE_BLOCK fragUniformBlock = FragmentTree->shader->storageBlocks[j];

            if (fragUniformBlock == gcvNULL) continue;

            /* Compare the names of the vertex storage block and fragment storage block. */
            if (vertStorageBlock->nameLength == fragUniformBlock->nameLength
                && gcmIS_SUCCESS(gcoOS_MemCmp(vertStorageBlock->name,
                                              fragUniformBlock->name,
                                              vertStorageBlock->nameLength)))
            {
                gctBOOL mismatch = gcvFALSE;

                /* check storage block array size to be the same */
                if((GetSBPrevSibling(vertStorageBlock) != -1 && GetSBPrevSibling(fragUniformBlock) == -1) ||
                   (GetSBPrevSibling(vertStorageBlock) == -1 && GetSBPrevSibling(fragUniformBlock) != -1) ||
                   (GetSBNextSibling(vertStorageBlock) != -1 && GetSBNextSibling(fragUniformBlock) == -1) ||
                   (GetSBNextSibling(vertStorageBlock) == -1 && GetSBNextSibling(fragUniformBlock) != -1))
                {
                    mismatch = gcvTRUE;
                }
                else if(GetSBPrevSibling(vertStorageBlock) == -1 &&
                        GetSBNextSibling(vertStorageBlock) != -1)
                {

                    gcsSTORAGE_BLOCK vertArrayElementBlock, fragArrayElementBlock;

                    vertArrayElementBlock = vertStorageBlock;
                    fragArrayElementBlock = fragUniformBlock;
                    while (GetSBNextSibling(vertArrayElementBlock) != -1 &&
                           GetSBNextSibling(fragArrayElementBlock) != -1)
                    {
                        status =  gcSHADER_GetStorageBlock(VertexTree->shader,
                                                           GetSBNextSibling(vertArrayElementBlock),
                                                           &vertArrayElementBlock);
                        if (gcmIS_ERROR(status))
                        {
                            /* Return on error. */
                            return status;
                        }

                        status =  gcSHADER_GetStorageBlock(FragmentTree->shader,
                                                           GetSBNextSibling(fragArrayElementBlock),
                                                           &fragArrayElementBlock);
                        if (gcmIS_ERROR(status))
                        {
                            /* Return on error. */
                            return status;
                        }
                    }
                    if(GetSBNextSibling(vertArrayElementBlock) != -1 ||
                       GetSBNextSibling(fragArrayElementBlock) != -1)
                    {
                        mismatch = gcvTRUE;
                    }
                }

                if(mismatch)
                {
                     gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block \"%s\" array size "
                              "does not match with corresponding storage block array size in fragment shader",
                              GetSBName(vertStorageBlock));

                     return gcvSTATUS_SSBO_MISMATCH;
                }

                /* Make sure the storage block memory layout are of the same. */
                if(GetSBMemoryLayout(vertStorageBlock) != GetSBMemoryLayout(fragUniformBlock))
                {
                    gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block \"%\" memory layout "
                             "does not match with corresponding storage block in fragment shader",
                             GetUBName(vertStorageBlock));

                    return gcvSTATUS_SSBO_MISMATCH;

                }
                /* Make sure the storage block memory layout are of the same.
                ** Default UBO might not have same number of uniforms
                */
                if (GetSBNumBlockElement(vertStorageBlock) != GetSBNumBlockElement(fragUniformBlock))
                {
                    gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block \"%\" member count "
                             "does not match with corresponding storage block in fragment shader",
                             GetSBName(vertStorageBlock));

                    return gcvSTATUS_SSBO_MISMATCH;

                }
                /* Make sure the storage block members are of the same type. */
                for(k = 0, vertBlockMember = GetSBFirstChild(vertStorageBlock),
                           fragBlockMember = GetSBFirstChild(fragUniformBlock);
                        k < GetSBNumBlockElement(vertStorageBlock); k++)
                {
                    gcmASSERT(vertBlockMember != -1);
                    gcmASSERT(fragBlockMember != -1);

                    status =  gcSHADER_GetVariable(VertexTree->shader,
                                                  vertBlockMember,
                                                  &vertStorageVariable);
                    if (gcmIS_ERROR(status))
                    {
                        /* Return on error. */
                        return status;
                    }

                    status =  gcSHADER_GetVariable(FragmentTree->shader,
                                                  fragBlockMember,
                                                  &fragStorageVariable);
                    if (gcmIS_ERROR(status))
                    {
                        /* Return on error. */
                        return status;
                    }

                    if (vertStorageVariable->nameLength != fragStorageVariable->nameLength
                        || !gcmIS_SUCCESS(gcoOS_MemCmp(vertStorageVariable->name,
                                                      fragStorageVariable->name,
                                                      vertStorageVariable->nameLength)))
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block member name \"%s\" "
                                 "does not match with corresponding storage block member name \"%s\" in fragment shader",
                                 vertStorageVariable->name, fragStorageVariable->name);

                        return gcvSTATUS_SSBO_MISMATCH;
                    }
                    if (GetVariableIsRowMajor(vertStorageVariable) != GetVariableIsRowMajor(fragStorageVariable))
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block member \"%s\" layout "
                                 "does not match with corresponding storage block member layout in fragment shader",
                                 vertStorageVariable->name);

                        return gcvSTATUS_SSBO_MISMATCH;
                    }
                    if (GetUniformCategory(vertStorageVariable) != GetUniformCategory(fragStorageVariable) ||
                        vertStorageVariable->u.type != fragStorageVariable->u.type)
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block member \"%s\" type "
                                 "does not match with corresponding storage block member type in fragment shader",
                                 vertStorageVariable->name);

                        return gcvSTATUS_SSBO_MISMATCH;
                    }


                    if ((GetVariableArraySize(vertStorageVariable) != GetVariableArraySize(fragStorageVariable)))
                    {
                        gcmTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_COMPILER, "gcLINKTREE_Link: verter shader storage block member \"%s\" array size %d "
                                 "does not match with corresponding storage block member array size %d in fragment shader",
                                 vertStorageVariable->name, GetVariableArraySize(vertStorageVariable),
                                 GetVariableArraySize(fragStorageVariable));

                        return gcvSTATUS_SSBO_MISMATCH;
                    }

                    vertStorageVariable->matchIndex = fragStorageVariable->index;
                    fragStorageVariable->matchIndex = vertStorageVariable->index;

                    vertBlockMember = vertStorageVariable->nextSibling;
                    fragBlockMember = fragStorageVariable->nextSibling;
                }

                /* This SBO is matched with a PS SBO, save the match index for it. */
                SetSBMatchIndex(vertStorageBlock, GetSBIndex(fragUniformBlock));
                SetSBMatchIndex(fragUniformBlock, GetSBIndex(vertStorageBlock));
            }
        }
    }

    /***************************************************************************
    ** III - Pack output and unpack input: varying packing by compiler.
    */
    if (!ENABLE_FULL_NEW_LINKER)
    {
        if (DoVaryingPacking && gcmOPT_PACKVARYING())
        {
            gcLINKTREE_RemoveUnusedAttributes(FragmentTree);

            gcLINKTREE_PackVarying(VertexTree, FragmentTree);
        }
    }

    /* Success. */
    return gcvSTATUS_OK;
}

gctBOOL
_BackwardMOV(
    IN OUT gcLINKTREE Tree,
    IN gctSIZE_T CodeIndex
    )
{
    gcLINKTREE_TEMP dependency;
    gcSL_INSTRUCTION dependentCode;
    gcsLINKTREE_LIST_PTR defined;

    /* Get shader. */
    gcSHADER shader = Tree->shader;

    /* Get instruction. */
    gcSL_INSTRUCTION code = &shader->code[CodeIndex];

    /* Get target register information. */
    gcLINKTREE_TEMP codeTemp = &Tree->tempArray[code->tempIndex];

    /* The only dependency should be a temporary register. */
    if ((codeTemp->dependencies == gcvNULL) ||
         (codeTemp->dependencies->next != gcvNULL) ||
         (codeTemp->dependencies->type != gcSL_TEMP) )
    {
        return gcvFALSE;
    }

    /* The dependent register should have only the current instruction as its
       user. */
    dependency = &Tree->tempArray[codeTemp->dependencies->index];
    if ((dependency->users != gcvNULL) &&
         ((dependency->users->next != gcvNULL) ||
          (dependency->users->type != gcSL_NONE) ||
          (dependency->users->index != (gctINT) CodeIndex)) )
    {
        return gcvFALSE;
    }

    /* The dependent register is not an output. */
    if (dependency->lastUse == -1)
    {
        return gcvFALSE;
    }

    /* The register cannot be defined more than once. */
    if (codeTemp->defined->next != gcvNULL)
    {
        return gcvFALSE;
    }

    /* Make sure the entire dependency is used. */
    if (gcmSL_TARGET_GET(code->temp, Enable) != dependency->usage)
    {
        return gcvFALSE;
    }

    /* Make sure the dependency is swizzling the entire register usage. */
    {
        gcSL_SWIZZLE swizzle = gcmSL_SOURCE_GET(code->source0, Swizzle);
        gcSL_SWIZZLE enabledSwizzle = _Enable2Swizzle(gcmSL_TARGET_GET(code->temp, Enable));
        if (enabledSwizzle != swizzle)
        {
            return gcvFALSE;
        }
    }

    /* Make sure the dependency is not indexed accessing */
    for (defined = dependency->defined;
         defined != gcvNULL;
         defined = defined->next)
    {
        dependentCode = &shader->code[defined->index];
        if (gcmSL_TARGET_GET(dependentCode->temp, Indexed) != gcSL_NOT_INDEXED)
        {
            return gcvFALSE;
        }

        /*For PS shader, we don't optimize this case:
          OR_BITWISE      temp.uint(60).hp, temp.uint(55).hp, temp.uint(59).hp
          MOV             temp(38), temp(60).hp,
          where temp(38) is mediump and temp(60) is highp and
          temp(60)'s format is different in MOV and OR_BITWISE
          Since in dual16, integer and float have different behavior in highp ->mediump
        */
        if (shader->type == gcSHADER_TYPE_FRAGMENT &&
            gcmSL_SOURCE_GET(code->source0, Precision) != gcmSL_TARGET_GET(code->temp, Precision) &&
            gcmSL_SOURCE_GET(code->source0, Format) != gcmSL_TARGET_GET(dependentCode->temp, Format))
        {
            return gcvFALSE;
        }

        /* we should not do the following case:
           1: mov t1, t2
           2: add t2, t3, 1.0 (dependentCode is in between)
           3: cmp t1, t4, 5 (use)
           4: jmp 1 */
        if (defined->index > (gctINT) CodeIndex)
        {
            gcsLINKTREE_LIST_PTR user;
            for (user = codeTemp->users; user != gcvNULL; user = user->next)
            {
                if (defined->index < user->index)
                {
                    return gcvFALSE;
                }
            }
        }
    }

    /*
        Make sure the function of CodeIndex is same with the function of the dependency's lastUse.

Invalid case:

MAIN:
    ...
    49: MOV             temp.int(21).x, 6
    50: CALL            100
    51: MOV             temp(4), temp(41)
    ...
    56: MOV             temp.int(21).x, 7
    58: CALL            100
    59: MOV             temp(4), temp(41)
    ...
END FUNCTION

FUNCTION 0:
   100: MOV             temp.int(155).x, temp.int(21).hp.x <== the last use of temp(155) is 58
   ...
LOOP_BEGIN:
   ...
   200: MOV             temp.int(223).x, temp.int(155).hp.x <== the last use of temp(223) is 201
   201: TEXLD           temp(238), sampler(0+temp(223).x), temp(224).xyz
   ...
LOOP_END
   ...
END FUNCTION
    */
    {
        gctINT32   funcIdx =  0;
        gctINT32   funcId0 = -1;
        gctINT32   funcId1 = -1;

        for (funcIdx = 0; funcIdx < (gctINT32)Tree->shader->functionCount; ++funcIdx)
        {
            gctINT codeStart = Tree->shader->functions[funcIdx]->codeStart;
            gctINT codeEnd   = codeStart + Tree->shader->functions[funcIdx]->codeCount;

            if (codeStart <= dependency->lastUse
                && codeEnd > dependency->lastUse)
            {
                funcId0 = funcIdx;
            }

            if (codeStart <= (gctINT)CodeIndex
                && codeEnd > (gctINT)CodeIndex)
            {
                funcId1 = funcIdx;
            }
        }

        if (funcId0 != funcId1)
        {
            return gcvFALSE;
        }

        for (funcIdx = 0; funcIdx < (gctINT32)Tree->shader->kernelFunctionCount; ++funcIdx)
        {
            gctINT codeStart = Tree->shader->kernelFunctions[funcIdx]->codeStart;
            gctINT codeEnd   = codeStart + Tree->shader->kernelFunctions[funcIdx]->codeCount;

            if (codeStart <= dependency->lastUse
                && codeEnd > dependency->lastUse)
            {
                funcId0 = funcIdx;
            }

            if (codeStart <= (gctINT)CodeIndex
                && codeEnd > (gctINT)CodeIndex)
            {
                funcId1 = funcIdx;
            }
        }

        if (funcId0 != funcId1)
        {
            return gcvFALSE;
        }
    }

    /* Now we backward optimize the MOV, which means changing the target of
       the dependent instruction to our current target and deleting the
       dependent target. */
    for (defined = dependency->defined;
         defined != gcvNULL;
         defined = defined->next)
    {
        dependentCode = &shader->code[defined->index];

        /*dependentCode->temp        = code->temp;*/
        /* Copy the indexed value. */
        dependentCode->temp = gcmSL_TARGET_SET(dependentCode->temp, Indexed,
                                               gcmSL_TARGET_GET(code->temp, Indexed));
        dependentCode->tempIndex   = code->tempIndex;
        dependentCode->tempIndexed = code->tempIndexed;
    }

    if (dependency->crossLoopIdx != -1 &&
        (codeTemp->crossLoopIdx == -1 || codeTemp->crossLoopIdx > dependency->crossLoopIdx))
    {
        /* add the new temp to the loop head registger allocate temp list */
        _addTempToLoopHeadLiveList(Tree, codeTemp, dependency->crossLoopIdx);
    }
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, codeTemp->dependencies));
    codeTemp->dependencies = dependency->dependencies;

    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, codeTemp->defined));
    codeTemp->defined = dependency->defined;
    codeTemp->lastUse = gcmMAX(codeTemp->lastUse, dependency->lastUse);

    gcoOS_ZeroMemory(code, sizeof(*code));

    _Delete(Tree, &dependency->users);
    dependency->dependencies = gcvNULL;
    dependency->defined      = gcvNULL;

    /* Mark the target as not used. */
    dependency->inUse   = gcvFALSE;
    dependency->lastUse = -1;
    dependency->usage   = 0;

    return gcvTRUE;
}

gctBOOL
_PreviousMOV(
    IN gcLINKTREE Tree,
    IN gctUINT CodeIndex
    )
{
    gcSL_INSTRUCTION current, previous;
    gctUINT16 enable, prevEnable;
    gctUINT16 enableCurrent;
    gctSOURCE_t source0Current, source0Previous, source1Previous;
    gcsLINKTREE_LIST_PTR list;
    gcLINKTREE_TEMP tempCurrent, tempPrevious;
    gcVARIABLE variable;
    gcSL_OPCODE prevOpcode;
    gcSL_CONDITION condition;
    gctBOOL isPrevOpcodeTargetComponentWised = gcvFALSE;
    gctBOOL isPrevOpcodeSourceComponentWised = gcvFALSE;

    /* Ignore instruction 0 - there is no previous instruction. */
    if (CodeIndex == 0)
    {
        return gcvFALSE;
    }

    /* Make sure this instruction is not the target of a JMP or CALL. */
    if (Tree->hints[CodeIndex].callers != gcvNULL)
    {
        return gcvFALSE;
    }

    /* Get the current and previous instructions. */
    current  = &Tree->shader->code[CodeIndex];
    previous = &Tree->shader->code[CodeIndex - 1];
    prevOpcode = gcmSL_OPCODE_GET(previous->opcode, Opcode);
    prevEnable = gcmSL_TARGET_GET(previous->temp, Enable);

    enableCurrent = gcmSL_TARGET_GET(current->temp, Enable);
    source0Current = current->source0;
    source0Previous = previous->source0;
    source1Previous = previous->source1;

    isPrevOpcodeTargetComponentWised =
        !(prevOpcode == gcSL_CROSS || prevOpcode == gcSL_NORM || prevOpcode == gcSL_LOAD ||
            prevOpcode == gcSL_ATTR_ST || prevOpcode == gcSL_ATTR_LD ||
            gcSL_isOpcodeTexld(prevOpcode) ||
            prevOpcode == gcSL_ARCTRIG0 || prevOpcode == gcSL_ARCTRIG1);

    isPrevOpcodeSourceComponentWised =
        !(prevOpcode == gcSL_DP3 || prevOpcode == gcSL_DP4 || prevOpcode == gcSL_DP2 ||
            prevOpcode == gcSL_CROSS || prevOpcode == gcSL_NORM || prevOpcode == gcSL_LOAD ||
            prevOpcode == gcSL_ATTR_ST || prevOpcode == gcSL_ATTR_LD ||
            gcSL_isOpcodeTexld(prevOpcode) ||
            prevOpcode == gcSL_ARCTRIG0 || prevOpcode == gcSL_ARCTRIG1);

    /* Make sure the previous instruction is not acos, asin, or orther
       instruction which may casue wrong evaluation */
    if ((prevOpcode == gcSL_LOAD)
    ||  (prevOpcode == gcSL_CMP)
    ||  (prevOpcode == gcSL_F2I)
    ||  (prevOpcode == gcSL_ACOS)
    ||  (prevOpcode == gcSL_ASIN)
    ||  (prevOpcode == gcSL_SET)
    ||  (prevOpcode == gcSL_STORE1)
    ||  (prevOpcode == gcSL_BITEXTRACT)
    ||  (prevOpcode == gcSL_BITRANGE)
    ||  (prevOpcode == gcSL_BITRANGE1)
    ||  (prevOpcode == gcSL_BITINSERT)
    ||  (prevOpcode == gcSL_IMAGE_RD)
    ||  (prevOpcode == gcSL_IMAGE_RD_3D)
    ||  (prevOpcode == gcSL_NORM)
    )
    {
        return gcvFALSE;
    }
    /* Make sure the source for the current instruction is the target of the
       previous instruction. */
    if ((gcmSL_SOURCE_GET(source0Current, Type) != gcSL_TEMP)
    ||  (gcmSL_SOURCE_GET(source0Current, Indexed) != gcSL_NOT_INDEXED)
    ||  (current->source0Index != previous->tempIndex)
    ||  (gcmSL_TARGET_GET(current->temp, Indexed) != gcSL_NOT_INDEXED)
    ||  (gcmSL_TARGET_GET(previous->temp, Indexed) != gcSL_NOT_INDEXED)
    ||  (Tree->tempArray[current->source0Index].users->next != gcvNULL)
    ||  (gcmSL_TARGET_GET(current->temp, Format) != gcmSL_TARGET_GET(previous->temp, Format))
    )
    {
        return gcvFALSE;
    }

    /* Determine if current swizzle/usage and previous enable are the same. */
    enable = 0;
    if (enableCurrent & gcSL_ENABLE_X)
    {
        enable |= (1 << gcmSL_SOURCE_GET(source0Current, SwizzleX));
    }

    if (enableCurrent & gcSL_ENABLE_Y)
    {
        enable |= (1 << gcmSL_SOURCE_GET(source0Current, SwizzleY));
    }

    if (enableCurrent & gcSL_ENABLE_Z)
    {
        enable |= (1 << gcmSL_SOURCE_GET(source0Current, SwizzleZ));
    }

    if (enableCurrent & gcSL_ENABLE_W)
    {
        enable |= (1 << gcmSL_SOURCE_GET(source0Current, SwizzleW));
    }

    if (gcmSL_TARGET_GET(previous->temp, Enable) != enable)
    {
        return gcvFALSE;
    }

    /* If previous instruction is texld and source of MOV has
       swizzle seq other than enable of texld, or the enabled
       coomponents are not equal, we should abort it

       e.g.
             2: TEXLDPCF        temp(4).x, uniform(0), attribute(0).xyz
             3: MOV             temp(5).xyz, temp(4).x
    */
    if (gcSL_isOpcodeTexld(prevOpcode) &&
        (((enable & gcSL_ENABLE_X) && gcmSL_SOURCE_GET(source0Current, SwizzleX) != gcSL_SWIZZLE_X)||
         ((enable & gcSL_ENABLE_Y) && gcmSL_SOURCE_GET(source0Current, SwizzleY) != gcSL_SWIZZLE_Y)||
         ((enable & gcSL_ENABLE_Z) && gcmSL_SOURCE_GET(source0Current, SwizzleZ) != gcSL_SWIZZLE_Z)||
         ((enable & gcSL_ENABLE_W) && gcmSL_SOURCE_GET(source0Current, SwizzleW) != gcSL_SWIZZLE_W)||
         _getEnableComponentCount(enableCurrent) != _getEnableComponentCount(prevEnable)))
    {
        return gcvFALSE;
    }

    /* cannot optimize, if MOV has saturation modifier */
    if (gcmSL_OPCODE_GET(current->opcode, Sat) != 0)
    {
        return gcvFALSE;
    }

    if (!isPrevOpcodeTargetComponentWised && enableCurrent != prevEnable)
    {
        return gcvFALSE;
    }

    /***************************************************************************
    *************** Yahoo, we can fold the MOV with the previous instruction. **
    ***************************************************************************/

    tempCurrent  = &Tree->tempArray[current->tempIndex];
    tempPrevious = &Tree->tempArray[previous->tempIndex];

    /* Skip if tempPrevious is volatile. */
    variable = tempPrevious->variable;
    if (variable && (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE))
    {
        return gcvFALSE;
    }

    /* Modify define of current temp from current to previous. */
    for (list = tempCurrent->defined; list != gcvNULL; list = list->next)
    {
        if (list->index == (gctINT) CodeIndex)
        {
            list->index = CodeIndex - 1;
            break;
        }
    }

    /* Remove previous define from previous temp. */
    gcmVERIFY_OK(_RemoveItem(Tree,
                             &tempPrevious->defined,
                             gcSL_NONE,
                             CodeIndex - 1));

    /* Remove the previous temp from current temp dependency list. */
    gcmVERIFY_OK(_RemoveItem(Tree,
                             &tempCurrent->dependencies,
                             gcSL_TEMP,
                             previous->tempIndex));

    /* Remove the current usage of the previous temp. */
    gcmVERIFY_OK(_RemoveItem(Tree,
                             &tempPrevious->users,
                             gcSL_NONE,
                             CodeIndex));

    /* Test source0. */
    switch (gcmSL_SOURCE_GET(source0Previous, Type))
    {
    case gcSL_TEMP:
        /* fall through */
    case gcSL_ATTRIBUTE:
        /* Add the previous source0 dependency to the current temp. */
        gcmVERIFY_OK(
            gcLINKTREE_AddList(Tree,
                               &tempCurrent->dependencies,
                               (gcSL_TYPE) gcmSL_SOURCE_GET(source0Previous, Type),
                               previous->source0Index));

        /* Remove the previous source0 dependency to the previous temp. */
        gcmVERIFY_OK(_RemoveItem(Tree,
                                 &tempPrevious->dependencies,
                                 (gcSL_TYPE) gcmSL_SOURCE_GET(source0Previous, Type),
                                 previous->source0Index));

        /* Test for indexing. */
        if (gcmSL_SOURCE_GET(source0Previous, Indexed) != gcSL_NOT_INDEXED)
        {
            /* Add the previous source0 dependency to the current temp. */
            gcmVERIFY_OK(gcLINKTREE_AddList(Tree,
                                            &tempCurrent->dependencies,
                                            gcSL_TEMP,
                                            previous->source0Indexed));

            /* Remove the previous source0 dependency to the previous temp. */
            gcmVERIFY_OK(_RemoveItem(Tree,
                                     &tempPrevious->dependencies,
                                     gcSL_TEMP,
                                     previous->source0Indexed));
        }
        break;
    default:
        break;
    }

    /* Test source1. */
    switch (gcmSL_SOURCE_GET(source1Previous, Type))
    {
    case gcSL_TEMP:
        /* fall through */
    case gcSL_ATTRIBUTE:
        /* Add the previous source1 dependency to the current temp. */
        gcmVERIFY_OK(
            gcLINKTREE_AddList(Tree,
                               &tempCurrent->dependencies,
                               (gcSL_TYPE) gcmSL_SOURCE_GET(source1Previous, Type),
                               previous->source1Index));

        /* Remove the previous source1 dependency to the previous temp. */
        gcmVERIFY_OK(_RemoveItem(Tree,
                                 &tempPrevious->dependencies,
                                 (gcSL_TYPE) gcmSL_SOURCE_GET(source1Previous, Type),
                                 previous->source1Index));

        /* Test for indexing. */
        if (gcmSL_SOURCE_GET(source1Previous, Indexed) != gcSL_NOT_INDEXED)
        {
            /* Add the previous source1 dependency to the current temp. */
            gcmVERIFY_OK(gcLINKTREE_AddList(Tree,
                                            &tempCurrent->dependencies,
                                            gcSL_TEMP,
                                            previous->source1Indexed));

            /* Remove the previous source1 dependency to the previous temp. */
            gcmVERIFY_OK(_RemoveItem(Tree,
                                     &tempPrevious->dependencies,
                                     gcSL_TEMP,
                                     previous->source1Indexed));
        }
        break;
    default:
        break;
    }

    /* Check if the enables are the same, */
    /* and the target enable and swizzle are in the same order. */
    if (isPrevOpcodeSourceComponentWised &&
        ((enableCurrent != gcmSL_TARGET_GET(previous->temp, Enable))
    ||  ((enableCurrent & gcSL_ENABLE_X) && gcmSL_SOURCE_GET(source0Current, SwizzleX) != gcSL_SWIZZLE_X)
    ||  ((enableCurrent & gcSL_ENABLE_Y) && gcmSL_SOURCE_GET(source0Current, SwizzleY) != gcSL_SWIZZLE_Y)
    ||  ((enableCurrent & gcSL_ENABLE_Z) && gcmSL_SOURCE_GET(source0Current, SwizzleZ) != gcSL_SWIZZLE_Z)
    ||  ((enableCurrent & gcSL_ENABLE_W) && gcmSL_SOURCE_GET(source0Current, SwizzleW) != gcSL_SWIZZLE_W)))
    {
        /* Complicated case--need to change previous sources to match current target. */

        /* Modify previous source(s) to match current target. */
        if (enableCurrent & gcSL_ENABLE_X)
        {
            switch (gcmSL_SOURCE_GET(source0Current, SwizzleX))
            {
            case gcSL_SWIZZLE_X:
                /* No change is needed. */
                break;
            case gcSL_SWIZZLE_Y:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleX, gcmSL_SOURCE_GET(source0Previous, SwizzleY));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleX, gcmSL_SOURCE_GET(source1Previous, SwizzleY));
                break;
            case gcSL_SWIZZLE_Z:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleX, gcmSL_SOURCE_GET(source0Previous, SwizzleZ));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleX, gcmSL_SOURCE_GET(source1Previous, SwizzleZ));
                break;
            case gcSL_SWIZZLE_W:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleX, gcmSL_SOURCE_GET(source0Previous, SwizzleW));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleX, gcmSL_SOURCE_GET(source1Previous, SwizzleW));
                break;
            default:
                break;
            }
        }

        if (enableCurrent & gcSL_ENABLE_Y)
        {
            switch (gcmSL_SOURCE_GET(source0Current, SwizzleY))
            {
            case gcSL_SWIZZLE_X:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleY, gcmSL_SOURCE_GET(source0Previous, SwizzleX));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleY, gcmSL_SOURCE_GET(source1Previous, SwizzleX));
                break;
            case gcSL_SWIZZLE_Y:
                /* No change is needed. */
                break;
            case gcSL_SWIZZLE_Z:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleY, gcmSL_SOURCE_GET(source0Previous, SwizzleZ));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleY, gcmSL_SOURCE_GET(source1Previous, SwizzleZ));
                break;
            case gcSL_SWIZZLE_W:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleY, gcmSL_SOURCE_GET(source0Previous, SwizzleW));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleY, gcmSL_SOURCE_GET(source1Previous, SwizzleW));
                break;
            default:
                break;
            }
        }

        if (enableCurrent & gcSL_ENABLE_Z)
        {
            switch (gcmSL_SOURCE_GET(source0Current, SwizzleZ))
            {
            case gcSL_SWIZZLE_X:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleZ, gcmSL_SOURCE_GET(source0Previous, SwizzleX));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleZ, gcmSL_SOURCE_GET(source1Previous, SwizzleX));
                break;
            case gcSL_SWIZZLE_Y:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleZ, gcmSL_SOURCE_GET(source0Previous, SwizzleY));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleZ, gcmSL_SOURCE_GET(source1Previous, SwizzleY));
                break;
            case gcSL_SWIZZLE_Z:
                /* No change is needed. */
                break;
            case gcSL_SWIZZLE_W:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleZ, gcmSL_SOURCE_GET(source0Previous, SwizzleW));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleZ, gcmSL_SOURCE_GET(source1Previous, SwizzleW));
                break;
            default:
                break;
            }
        }

        if (enableCurrent & gcSL_ENABLE_W)
        {
            switch (gcmSL_SOURCE_GET(source0Current, SwizzleW))
            {
            case gcSL_SWIZZLE_X:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleW, gcmSL_SOURCE_GET(source0Previous, SwizzleX));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleW, gcmSL_SOURCE_GET(source1Previous, SwizzleX));
                break;
            case gcSL_SWIZZLE_Y:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleW, gcmSL_SOURCE_GET(source0Previous, SwizzleY));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleW, gcmSL_SOURCE_GET(source1Previous, SwizzleY));
                break;
            case gcSL_SWIZZLE_Z:
                previous->source0 = gcmSL_SOURCE_SET(previous->source0, SwizzleW, gcmSL_SOURCE_GET(source0Previous, SwizzleZ));
                previous->source1 = gcmSL_SOURCE_SET(previous->source1, SwizzleW, gcmSL_SOURCE_GET(source1Previous, SwizzleZ));
                break;
            case gcSL_SWIZZLE_W:
                /* No change is needed. */
                break;
            default:
                break;
            }
        }
    }

    condition = gcmSL_TARGET_GET(previous->temp, Condition);

    /* Fold it! */
    previous->temp        = current->temp;
    previous->temp = gcmSL_TARGET_SET(previous->temp, Condition, condition);
    previous->tempIndex   = current->tempIndex;
    previous->tempIndexed = current->tempIndexed;

    gcoOS_ZeroMemory(current, gcmSIZEOF(*current));

    return gcvTRUE;
}

gceSTATUS
gcLINKTREE_Optimize(
    IN OUT gcLINKTREE Tree
    )
{
    gctSIZE_T i;
    gcSL_INSTRUCTION code;
    gcSL_INSTRUCTION usedCode;
    gcSHADER shader = Tree->shader;

    /***************************************************************************
    ** I - Find all MOV instructions.
    */

    /* Walk all instructions. */
    for (i = 0; i < shader->codeCount; i++)
    {
        /* Get instruction. */
        code = &shader->code[i];

        /* Test for MOV instruction. */
        if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MOV &&
            gcmSL_OPCODE_GET(code->opcode, Sat) == gcSL_NO_SATURATE)
        {
            gcsLINKTREE_LIST_PTR dependency;
            gcsLINKTREE_LIST_PTR user;
            gcLINKTREE_TEMP dependencyTemp;
            gctINT reDefinedPc;
            gcsLINKTREE_LIST_PTR defined;
            gctUINT16 enable;
            gctUINT8 swizzleOfMOVSrc;
            gcVARIABLE variable;
            gctBOOL   diffType;

            /* Get the temporary register. */
            gcLINKTREE_TEMP codeTemp = &Tree->tempArray[code->tempIndex];
            gcLINKTREE_TEMP sourceTemp;
            gcCROSS_FUNCTION_LIST functionList;

            /* See if we can fold this MOV with the previous instruction. */
            if (_PreviousMOV(Tree, i))
            {
                continue;
            }

            /* Skip if codeTemp is volatile. */
            variable = codeTemp->variable;
            if (variable && (variable->qualifier & gcvTYPE_QUALIFIER_VOLATILE))
            {
                continue;
            }

            /* Cannot optimize a MOV if the temporary register is not fully
               written to here. */
            if (gcmSL_TARGET_GET(code->temp, Enable) ^ codeTemp->usage)
            {
                continue;
            }

            /* Cannot optimize a MOV if the source is a constant. */
            if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_CONSTANT)
            {
                continue;
            }

            /* Cannot optimize a MOV if the target is an argument. */
            if (codeTemp->owner != gcvNULL)
            {
                continue;
            }

            /* Cannot optimize a MOV if the source using a indexed. */
            if (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED)
            {
                continue;
            }

            /* there is implicit type conversion inside the code, thus bail out
                when the type is not the same */
            diffType = gcvFALSE;
            for (user = codeTemp->users; user != gcvNULL; user = user->next)
            {
                gcSL_INSTRUCTION userCode = &shader->code[user->index];
                if ((gcmSL_SOURCE_GET(userCode->source0, Type) == gcSL_TEMP) &&
                    (userCode->source0Index == code->tempIndex) &&
                    (gcmSL_SOURCE_GET(userCode->source0, Format) != gcmSL_SOURCE_GET(code->tempIndex, Format)))
                {
                    diffType = gcvTRUE;
                    break;
                }

                if ((gcmSL_SOURCE_GET(userCode->source1, Type) == gcSL_TEMP) &&
                    (userCode->source1Index == code->tempIndex) &&
                    (gcmSL_SOURCE_GET(userCode->source1, Format) != gcmSL_SOURCE_GET(code->tempIndex, Format)))
                {
                    diffType = gcvTRUE;
                    break;
                }
            }
            if (diffType)
            {
                continue;
            }

            /* Cannot optimize a MOV if the source is an attribute or uniform
               and the temporay register is used as an output. */
            if (gcmSL_SOURCE_GET(code->source0, Type) != gcSL_TEMP)
            {
                gctBOOL propagateUniform = gcvFALSE;

                if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_UNIFORM)
                {
                    propagateUniform = gcvTRUE;
                }

                /* Walk through all users. */
                for (user = codeTemp->users; user != gcvNULL; user = user->next)
                {
                    gcSL_INSTRUCTION userCode;

                    /* Bail out if the user is an output. */
                    if (user->type == gcSL_OUTPUT)
                    {
                        break;
                    }

                    /* gcSL does not allow target temp register the same as any of the source temp register. */
                    /* TODO - Need to remove when STORE can handle uniform and swizzle, etc.*/
                    userCode = &shader->code[user->index];
                    if (user->type == gcSL_NONE)
                    {
                        if (gcmSL_OPCODE_GET(userCode->opcode, Opcode) == gcSL_STORE ||
                            gcmSL_OPCODE_GET(userCode->opcode, Opcode) == gcSL_IMAGE_WR ||
                            gcmSL_OPCODE_GET(userCode->opcode, Opcode) == gcSL_IMAGE_WR_3D ||
                            gcmSL_OPCODE_GET(userCode->opcode, Opcode) == gcSL_ATTR_ST)
                        {
                            break;
                        }
                    }

                    /* TODO - for new chip that support immediate constant, */
                    /*        need to check if constant can be immediate constant. */
                    if (propagateUniform)
                    {
                        /* If source is a uniform, check if it causes two constants in an instruction. */
                        if ((gcmSL_SOURCE_GET(userCode->source0, Type) == gcSL_TEMP) &&
                             (userCode->source0Index == code->tempIndex) &&
                             (gcmSL_SOURCE_GET(userCode->source1, Type) == gcSL_CONSTANT))
                        {
                            break;
                        }
                        if ((gcmSL_SOURCE_GET(userCode->source1, Type) == gcSL_TEMP) &&
                             (userCode->source1Index == code->tempIndex) &&
                             (gcmSL_SOURCE_GET(userCode->source0, Type) == gcSL_CONSTANT))
                        {
                            break;
                        }
                    }
                }

                if (user != gcvNULL)
                {
                    /* Don't optimize MOV. */
                    continue;
                }
            }

            /* Cannot optimize if the source is an argument. */
            else if ((gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP)
            &&       (Tree->tempArray[code->source0Index].owner != gcvNULL)
            )
            {
                continue;
            }

            /* Try a backwards optimization. */
            if (_BackwardMOV(Tree, i))
            {
                continue;
            }
            /* Cannot optimize a MOV if there are multiple dependencies. */
            if ((codeTemp->dependencies != gcvNULL) &&
                (codeTemp->dependencies->next != gcvNULL))
            {
                continue;
            }

            /* Cannot optimize a MOV if the dependency is an output. */
            if ((codeTemp->dependencies != gcvNULL)
            &&  (codeTemp->dependencies->type == gcSL_TEMP)
            &&  (Tree->tempArray[codeTemp->dependencies->index].lastUse == -1)
            )
            {
                continue;
            }

            /* Cannot optimize a MOV if there are multiple definintions. */
            if (codeTemp->defined->next != gcvNULL)
            {
                continue;
            }

            /* Cannot optimize a MOV if the target is used as an index. */
            if (codeTemp->isIndex || codeTemp->isIndexing)
            {
                continue;
            }

            /* cannot optimize, if MOV has saturation modifier */
            if (gcmSL_OPCODE_GET(code->opcode, Sat) != 0)
            {
                continue;
            }


            /*
            Invalid case:
            49: MOV             temp(116).x, temp(21).x
            50: CALL            100

            BEGIN:
            100: ......
            120: MUL             temp(46).x, temp(116).x, 0.500000    <==the lastUse of temp(46) is 50.
            ....
            164: MOV             temp(226).x, temp(46).x              <== the lastUse of temp(226) is 165.
            165: TEXLOD          sampler(0+temp(223).x), temp(226).x
            END
            */
            if (codeTemp->dependencies != gcvNULL)
            {
                gctINT      depLastUse = Tree->tempArray[codeTemp->dependencies->index].lastUse;
                gctINT      funcIdx =  0;
                gctINT      funcId0 = -1;
                gctINT      funcId1 = -1;

                for (funcIdx = 0; funcIdx < (gctINT32)Tree->shader->functionCount; ++funcIdx)
                {
                    gctINT codeStart = Tree->shader->functions[funcIdx]->codeStart;
                    gctINT codeEnd   = codeStart + Tree->shader->functions[funcIdx]->codeCount;

                    if (codeStart <= codeTemp->defined->index
                        && codeEnd > codeTemp->defined->index)
                    {
                        funcId0 = funcIdx;
                    }

                    if (codeStart <= depLastUse
                        && codeEnd > depLastUse)
                    {
                        funcId1 = funcIdx;
                    }
                }

                if (funcId0 != funcId1)
                {
                    continue;
                }

                for (funcIdx = 0; funcIdx < (gctINT32)Tree->shader->kernelFunctionCount; ++funcIdx)
                {
                    gctINT codeStart = Tree->shader->kernelFunctions[funcIdx]->codeStart;
                    gctINT codeEnd   = codeStart + Tree->shader->kernelFunctions[funcIdx]->codeCount;

                    if (codeStart <= codeTemp->defined->index
                        && codeEnd > codeTemp->defined->index)
                    {
                        funcId0 = funcIdx;
                    }

                    if (codeStart <= depLastUse
                        && codeEnd > depLastUse)
                    {
                        funcId1 = funcIdx;
                    }
                }

                if (funcId0 != funcId1)
                {
                    continue;
                }
            }

            /* Cannot optimize a MOV if the target enable and source swizzle are not in the same order.
               But why we need such limitation???? We should remove this later. */
            swizzleOfMOVSrc = gcmSL_SOURCE_GET(code->source0, Swizzle);
            if (swizzleOfMOVSrc != gcSL_SWIZZLE_XXXX &&
                swizzleOfMOVSrc != gcSL_SWIZZLE_YYYY &&
                swizzleOfMOVSrc != gcSL_SWIZZLE_ZZZZ &&
                swizzleOfMOVSrc != gcSL_SWIZZLE_WWWW)
            {
                enable = gcmSL_TARGET_GET(code->temp, Enable);
                if ((enable & gcSL_ENABLE_X) && gcmSL_SOURCE_GET(code->source0, SwizzleX) != gcSL_SWIZZLE_X)
                {
                    continue;
                }

                if ((enable & gcSL_ENABLE_Y) && gcmSL_SOURCE_GET(code->source0, SwizzleY) != gcSL_SWIZZLE_Y)
                {
                    continue;
                }

                if ((enable & gcSL_ENABLE_Z) && gcmSL_SOURCE_GET(code->source0, SwizzleZ) != gcSL_SWIZZLE_Z)
                {
                    continue;
                }

                if ((enable & gcSL_ENABLE_W) && gcmSL_SOURCE_GET(code->source0, SwizzleW) != gcSL_SWIZZLE_W)
                {
                    continue;
                }
            }

            /* DependencyTemp cannot be redefined before all users. */

            /* Find the closest dependency temp's redefined pc. */
            if (codeTemp->dependencies != gcvNULL)
            {

                dependencyTemp = &Tree->tempArray[codeTemp->dependencies->index];
                reDefinedPc = shader->codeCount;
                for (defined = dependencyTemp->defined; defined; defined = defined->next)
                {
                    gctINT definedPc = defined->index;

                    if (definedPc > (gctINT) i && definedPc < reDefinedPc)
                    {
                        reDefinedPc = definedPc;
                    }
                }

                /* Check if any user after the dependency temp's redefined pc. */
                for (user = codeTemp->users; user != gcvNULL; user = user->next)
                {
                    /* Output user index is always infinite */
                    if ((user->index >= reDefinedPc) || (user->type == gcSL_OUTPUT))
                    {
                        break;
                    }
                }

                if (user)
                {
                    continue;
                }
            }

            /*
                1: mul temp(1), attr(1), 4      users: 5        dep: ...
                5: mov temp(5), temp(1)         users: 8, 12    dep: 1
                8: add temp(8), temp(5), ...    users: ...      dep: 5, ...
                12: mul temp(12), temp(5), ...  users: ...      dep: 5, ...

                1:                              users: 8, 12    dep: ...
                8:  add temp(8), temp(1), ...   users: ...      dep: 1
                12: mul temp(12), temp(1), ...  users: ...      dep: 1

                (5) FOR EACH USER:  PATCH CODE
                                    REMOVE (5) FROM DEP
                                    ADD DEP FROM (5)
                (5) FOR EACH DEP:   REMOVE (5) FROM USER
                                    ADD USERS FROM (5)
            */

            /* For all users, patch the source and modify their dependencies. */
            for (user = codeTemp->users; user != gcvNULL; user = user->next)
            {
                gctSOURCE_t source = code->source0;
                gcSL_OPCODE  opcode;

                /* Get the user instruction and target register. */
                gcSL_INSTRUCTION userCode = &shader->code[user->index];
                gcLINKTREE_TEMP userTemp  = &Tree->tempArray[userCode->tempIndex];

                /* Get source operands. */
                gctSOURCE_t source0 = userCode->source0;
                gctSOURCE_t source1 = userCode->source1;

                switch (gcmSL_OPCODE_GET(userCode->opcode, Opcode))
                {
                case gcSL_JMP:
                    /* fall through */
                case gcSL_CALL:
                    /* fall through */
                case gcSL_KILL:
                    /* fall through */
                case gcSL_NOP:
                    /* fall through */
                case gcSL_RET:
                    userTemp = gcvNULL;
                    break;
                default:
                    break;
                }

                if (user->type == gcSL_OUTPUT)
                {
                    Tree->outputArray[user->index].tempHolding =
                        code->source0Index;

                    if (Tree->shader->outputs[user->index])
                    {
                        Tree->shader->outputs[user->index]->tempIndex =
                            code->source0Index;
                    }

                    continue;
                }

                /* Test if source0 is the target of the MOV. */
                if ((gcmSL_SOURCE_GET(source0, Type) == gcSL_TEMP) &&
                     (userCode->source0Index == code->tempIndex) )
                {
                    source0 = gcmSL_SOURCE_SET(source0, Type, gcmSL_SOURCE_GET(source, Type));
                    source0 = gcmSL_SOURCE_SET(source0, Indexed,gcmSL_SOURCE_GET(source, Indexed));
                    source0 = gcmSL_SOURCE_SET(source0, Neg, gcmSL_SOURCE_GET(source, Neg) ^ gcmSL_SOURCE_GET(source0, Neg));
                    source0 = gcmSL_SOURCE_SET(source0, Abs, gcmSL_SOURCE_GET(source, Abs) | gcmSL_SOURCE_GET(source0, Abs));
                    source0 = gcmSL_SOURCE_SET(source0, SwizzleX, _SelectSwizzle(gcmSL_SOURCE_GET(source0, SwizzleX), source));
                    source0 = gcmSL_SOURCE_SET(source0, SwizzleY, _SelectSwizzle(gcmSL_SOURCE_GET(source0, SwizzleY), source));
                    source0 = gcmSL_SOURCE_SET(source0, SwizzleZ, _SelectSwizzle(gcmSL_SOURCE_GET(source0, SwizzleZ), source));
                    source0 = gcmSL_SOURCE_SET(source0, SwizzleW, _SelectSwizzle(gcmSL_SOURCE_GET(source0, SwizzleW), source));

                    userCode->source0        = source0;
                    userCode->source0Index   = code->source0Index;
                    userCode->source0Indexed = code->source0Indexed;
                }

                /* Test if source1 is the target of the MOV. */
                if ((gcmSL_SOURCE_GET(source1, Type) == gcSL_TEMP) &&
                     (userCode->source1Index == code->tempIndex) )
                {
                    source1 = gcmSL_SOURCE_SET(source1, Type, gcmSL_SOURCE_GET(source, Type));
                    source1 = gcmSL_SOURCE_SET(source1, Indexed, gcmSL_SOURCE_GET(source, Indexed));
                    source1 = gcmSL_SOURCE_SET(source1, Neg, gcmSL_SOURCE_GET(source, Neg) ^ gcmSL_SOURCE_GET(source1, Neg));
                    source1 = gcmSL_SOURCE_SET(source1, Abs, gcmSL_SOURCE_GET(source, Abs) | gcmSL_SOURCE_GET(source1, Abs));
                    source1 = gcmSL_SOURCE_SET(source1, SwizzleX, _SelectSwizzle(gcmSL_SOURCE_GET(source1, SwizzleX), source));
                    source1 = gcmSL_SOURCE_SET(source1, SwizzleY, _SelectSwizzle(gcmSL_SOURCE_GET(source1, SwizzleY), source));
                    source1 = gcmSL_SOURCE_SET(source1, SwizzleZ, _SelectSwizzle(gcmSL_SOURCE_GET(source1, SwizzleZ), source));
                    source1 = gcmSL_SOURCE_SET(source1, SwizzleW, _SelectSwizzle(gcmSL_SOURCE_GET(source1, SwizzleW), source));

                    userCode->source1        = *(gctSOURCE_t *) &source1;
                    userCode->source1Index   = code->source0Index;
                    userCode->source1Indexed = code->source0Indexed;
                }

                opcode = gcmSL_OPCODE_GET(userCode->opcode, Opcode);
                if (gcSL_isOpcodeTexld(opcode))
                {
                    gcSL_INSTRUCTION texAttrCode = &shader->code[user->index - 1];
                    gcSL_OPCODE  texAttrOpcode;

                    texAttrOpcode = gcmSL_OPCODE_GET(texAttrCode->opcode, Opcode);
                    if (gcSL_isOpcodeTexldModifier(texAttrOpcode) &&
                        !(texAttrOpcode == gcSL_TEXGRAD || texAttrOpcode == gcSL_TEXGATHER))
                    {
                        gctSOURCE_t source1OfTexAttrCode = texAttrCode->source1;

                        if ((gcmSL_SOURCE_GET(source1OfTexAttrCode, Type) == gcSL_TEMP) &&
                             (texAttrCode->source1Index == code->tempIndex) )
                        {
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, Type, gcmSL_SOURCE_GET(source, Type));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, Indexed, gcmSL_SOURCE_GET(source, Indexed));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, Format, gcmSL_SOURCE_GET(source, Format));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, SwizzleX, _SelectSwizzle(gcmSL_SOURCE_GET(source1OfTexAttrCode, SwizzleX), source));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, SwizzleY, _SelectSwizzle(gcmSL_SOURCE_GET(source1OfTexAttrCode, SwizzleY), source));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, SwizzleZ, _SelectSwizzle(gcmSL_SOURCE_GET(source1OfTexAttrCode, SwizzleZ), source));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, SwizzleW, _SelectSwizzle(gcmSL_SOURCE_GET(source1OfTexAttrCode, SwizzleW), source));

                            texAttrCode->source1        = *(gctSOURCE_t *) &source1OfTexAttrCode;
                            texAttrCode->source1Index   = code->source0Index;
                            texAttrCode->source1Indexed = code->source0Indexed;
                        }
                    }
                    else if (texAttrOpcode == gcSL_TEXGRAD || texAttrOpcode == gcSL_TEXGATHER)
                    {
                        gctSOURCE_t source0OfTexAttrCode = texAttrCode->source0;
                        gctSOURCE_t source1OfTexAttrCode = texAttrCode->source1;

                        if ((gcmSL_SOURCE_GET(source0OfTexAttrCode, Type) == gcSL_TEMP) &&
                             (texAttrCode->source0Index == code->tempIndex) )
                        {
                            source0OfTexAttrCode = gcmSL_SOURCE_SET(source0OfTexAttrCode, Type, gcmSL_SOURCE_GET(source, Type));
                            source0OfTexAttrCode = gcmSL_SOURCE_SET(source0OfTexAttrCode, Indexed, gcmSL_SOURCE_GET(source, Indexed));
                            source0OfTexAttrCode = gcmSL_SOURCE_SET(source0OfTexAttrCode, Format, gcmSL_SOURCE_GET(source, Format));
                            source0OfTexAttrCode = gcmSL_SOURCE_SET(source0OfTexAttrCode, SwizzleX, _SelectSwizzle(gcmSL_SOURCE_GET(source0OfTexAttrCode, SwizzleX), source));
                            source0OfTexAttrCode = gcmSL_SOURCE_SET(source0OfTexAttrCode, SwizzleY, _SelectSwizzle(gcmSL_SOURCE_GET(source0OfTexAttrCode, SwizzleY), source));
                            source0OfTexAttrCode = gcmSL_SOURCE_SET(source0OfTexAttrCode, SwizzleZ, _SelectSwizzle(gcmSL_SOURCE_GET(source0OfTexAttrCode, SwizzleZ), source));
                            source0OfTexAttrCode = gcmSL_SOURCE_SET(source0OfTexAttrCode, SwizzleW, _SelectSwizzle(gcmSL_SOURCE_GET(source0OfTexAttrCode, SwizzleW), source));

                            texAttrCode->source0        = *(gctSOURCE_t *) &source0OfTexAttrCode;
                            texAttrCode->source0Index   = code->source0Index;
                            texAttrCode->source0Indexed = code->source0Indexed;
                        }
                        if ((gcmSL_SOURCE_GET(source1OfTexAttrCode, Type) == gcSL_TEMP) &&
                             (texAttrCode->source1Index == code->tempIndex) )
                        {
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, Type, gcmSL_SOURCE_GET(source, Type));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, Indexed, gcmSL_SOURCE_GET(source, Indexed));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, Format, gcmSL_SOURCE_GET(source, Format));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, SwizzleX, _SelectSwizzle(gcmSL_SOURCE_GET(source1OfTexAttrCode, SwizzleX), source));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, SwizzleY, _SelectSwizzle(gcmSL_SOURCE_GET(source1OfTexAttrCode, SwizzleY), source));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, SwizzleZ, _SelectSwizzle(gcmSL_SOURCE_GET(source1OfTexAttrCode, SwizzleZ), source));
                            source1OfTexAttrCode = gcmSL_SOURCE_SET(source1OfTexAttrCode, SwizzleW, _SelectSwizzle(gcmSL_SOURCE_GET(source1OfTexAttrCode, SwizzleW), source));

                            texAttrCode->source1        = *(gctSOURCE_t *) &source1OfTexAttrCode;
                            texAttrCode->source1Index   = code->source0Index;
                            texAttrCode->source1Indexed = code->source0Indexed;
                        }
                    }
                }

                if (gcmSL_OPCODE_GET(userCode->opcode, Opcode) != gcSL_STORE &&
                    gcmSL_OPCODE_GET(userCode->opcode, Opcode) != gcSL_IMAGE_WR &&
                    gcmSL_OPCODE_GET(userCode->opcode, Opcode) != gcSL_IMAGE_WR_3D &&
                    gcmSL_OPCODE_GET(userCode->opcode, Opcode) != gcSL_ATTR_ST)
                {
                    if (userTemp != gcvNULL)
                    {
                        /* Remove target dependency. */
                        _RemoveItem(Tree,
                                    &userTemp->dependencies,
                                    gcSL_TEMP,
                                    code->tempIndex);

                        /* Append source dependencies. */
                        for (dependency = codeTemp->dependencies;
                             dependency != gcvNULL;
                             dependency = dependency->next)
                        {
                            gcmVERIFY_OK(
                                gcLINKTREE_AddList(Tree,
                                                   &userTemp->dependencies,
                                                   dependency->type,
                                                   dependency->index));
                        }
                    }

                    if (gcmSL_TARGET_GET(userCode->temp, Indexed) != gcSL_NOT_INDEXED)
                    {
                        if (Tree->tempArray[userCode->tempIndex].variable)
                        {
                            gcVARIABLE variable = Tree->tempArray[userCode->tempIndex].variable;

                            gctUINT startIndex, endIndex, j;
                            gcmASSERT(isUniformNormal(variable));
                            gcSHADER_GetVariableIndexingRange(Tree->shader, variable, gcvFALSE,
                                                              &startIndex, &endIndex);
                            gcmASSERT(startIndex == variable->tempIndex);

                            for (j = startIndex + 1; j < endIndex; j++)
                            {
                                /* Get pointer to temporary register. */
                                gcLINKTREE_TEMP userTemp = &Tree->tempArray[j];

                                /* Remove target dependency. */
                                _RemoveItem(Tree,
                                            &userTemp->dependencies,
                                            gcSL_TEMP,
                                            code->tempIndex);

                                /* Append source dependencies. */
                                for (dependency = codeTemp->dependencies;
                                     dependency != gcvNULL;
                                     dependency = dependency->next)
                                {
                                    gcmVERIFY_OK(
                                        gcLINKTREE_AddList(Tree,
                                                           &userTemp->dependencies,
                                                           dependency->type,
                                                           dependency->index));
                                }
                            }
                        }
                    }
                }
                else if (userCode->tempIndex == code->tempIndex)
                {
                    userCode->temp        = gcmSL_TARGET_SET(userCode->temp, Indexed,
                                                             gcmSL_SOURCE_GET(source, Indexed));
                    userCode->tempIndex   = code->source0Index;
                    userCode->tempIndexed = code->source0Indexed;
                }
            }

            /* For all dependencies, modify their users. */
            for (dependency = codeTemp->dependencies;
                 dependency != gcvNULL;
                 dependency = dependency->next)
            {
                /* Get root of users tree. */
                gcsLINKTREE_LIST_PTR * root = gcvNULL;
                gctINT * lastUse = gcvNULL;

                switch (dependency->type)
                {
                case gcSL_ATTRIBUTE:
                    root    = &Tree->attributeArray[dependency->index].users;
                    lastUse = &Tree->attributeArray[dependency->index].lastUse;
                    break;

                case gcSL_UNIFORM:
                    root    = gcvNULL;
                    lastUse = gcvNULL;
                    break;

                case gcSL_TEMP:
                    root    = &Tree->tempArray[dependency->index].users;
                    lastUse = &Tree->tempArray[dependency->index].lastUse;
                    break;

                default:
                    gcmFATAL("ERROR: Invalid dependency %u at 0x%x",
                          dependency->type,
                          dependency);
                }

                if ((root != gcvNULL) && (lastUse != gcvNULL))
                {
                    /* Update last usage. */
                    if (lastUse && ((codeTemp->lastUse > *lastUse) || (codeTemp->lastUse == -1)) )
                    {
                        *lastUse = codeTemp->lastUse;
                    }

                    /* Remove the current instruction as a user. */
                    _RemoveItem(Tree, root, gcSL_NONE, i);

                    /* Append all users. */
                    for (user = codeTemp->users; user != gcvNULL; user = user->next)
                    {
                        gcmVERIFY_OK(gcLINKTREE_AddList(Tree,
                                                        root,
                                                        user->type,
                                                        user->index));
                    }
                }
            }

            /* If this temp register has a cross function list,
            ** we need to copy this list to the source temp register.
            **/
            if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP)
            {
                sourceTemp = &Tree->tempArray[code->source0Index];
                functionList = codeTemp->crossFuncList;

                while(functionList)
                {
                    gcmASSERT(functionList->callIndex >= 0);
                    _addTempToFunctionLiveList(Tree, sourceTemp, functionList->callIndex);
                    functionList = functionList->next;
                }
            }

            /* Set current instruction to NOP. */
            gcoOS_ZeroMemory(code, sizeof(struct _gcSL_INSTRUCTION));

            /* Delete the target's definition tree. */
            _Delete(Tree, &codeTemp->defined);

            /* Delete the target's dependency tree. */
            _Delete(Tree, &codeTemp->dependencies);

            /* Delete the target's user tree. */
            _Delete(Tree, &codeTemp->users);

            /* Mark the target as not used. */
            codeTemp->inUse   = gcvFALSE;
            codeTemp->lastUse = -1;
            codeTemp->usage   = 0;
            gcmASSERT(codeTemp->crossLoopIdx == -1);
        }
    }

    /* II - Find all MOVs with constants. */

    /* Walk all instructions. */
    for (i = 0; i < shader->codeCount; i++)
    {
        /* Get instruction. */
        code = &shader->code[i];

        /* Test for MOV instruction. */
        if ((gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MOV)

        /* Test if the source is a constant. */
        &&  (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_CONSTANT)

        /* Test if instruction has saturate. */
        && (gcmSL_OPCODE_GET(code->opcode, Sat) == gcSL_NO_SATURATE)

        /* We cannot optimize an output register. */
        &&  (code->tempIndex < Tree->tempCount)
        &&  (Tree->tempArray[code->tempIndex].lastUse > 0)
        &&  (Tree->tempArray[code->tempIndex].lastUse < (gctINT)shader->codeCount)
        )
        {
            gctTARGET_t target = code->temp;
            gcLINKTREE_TEMP temp = &Tree->tempArray[code->tempIndex];
            gctINT index;
            gctUINT32 valueInt;
            gcuFLOAT_UINT32 value;

            /* Input argument passing can not be used for constant propagation,
            ** because the user instruction of temp register may before the call instruction.
            */
            if (temp->inputOrOutput == gcvFUNCTION_INPUT || temp->inputOrOutput == gcvFUNCTION_INOUT)
            {
                continue;
            }

            usedCode = &shader->code[temp->lastUse];

            /* The used instruction has dest */
            if (gcmSL_TARGET_GET(usedCode->temp, Enable))
            {
                /* The dest dependen is more than one */
                if (usedCode->tempIndex < Tree->tempCount &&
                    Tree->tempArray[usedCode->tempIndex].dependencies != gcvNULL &&
                    Tree->tempArray[usedCode->tempIndex].dependencies->next != gcvNULL)
                    continue;
            }

            valueInt = (code->source0Index & 0xFFFF)
                     | (code->source0Indexed << 16);
            value.u = valueInt;

            for (index = 0; index < 4; ++index)
            {
                if (gcmSL_TARGET_GET(target, Enable) & 1)
                {
                    switch (temp->constUsage[index])
                    {
                    case 0:
                        temp->constUsage[index] = 1;
                        /* Use memcpy instead of operation "=" */
                        gcoOS_MemCopy(&temp->constValue[index], &value, sizeof(value));
                        break;

                    case 1:
                        {
                            /* convert constant value from gctFLOAT to gctUINT to make sure compare correctly */
                            gctUINT preValue = temp->constValue[index].u;
                            gctUINT thisValue = value.u;
                            if (preValue != thisValue)
                            {
                                temp->constUsage[index] = -1;
                            }
                        }
                        break;

                    default:
                        break;
                    }
                }

                target = gcmSL_TARGET_SET(target,
                                          Enable,
                                          gcmSL_TARGET_GET(target,
                                                           Enable) >> 1);
            }
        }

        else
        switch (gcmSL_OPCODE_GET(code->opcode, Opcode))
        {
        case gcSL_JMP:
            /* fall through */
        case gcSL_CALL:
            /* fall through */
        case gcSL_KILL:
            /* fall through */
        case gcSL_NOP:
            /* fall through */
        case gcSL_RET:
            break;

        default:
            {
                gcSL_OPCODE  opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
                /* Mark entire register as non optimizable. */
                if (!gcSL_isOpcodeTexldModifier(opcode))
                {
                    gcLINKTREE_TEMP temp = &Tree->tempArray[code->tempIndex];

                    temp->constUsage[0] =
                    temp->constUsage[1] =
                    temp->constUsage[2] =
                    temp->constUsage[3] = -1;
                }
            }
            break;
        }
    }

    /* Success. */
    return gcvSTATUS_OK;
}

static gceSTATUS
_InitializeSamplerAddress(
    IN OUT gcSHADER Shader,
    IN OUT gctINT * NewPhysical,
    IN gceSHADER_FLAGS Flags,
    OUT gctBOOL * RemapSamplerAddress
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i, sampler = 0;
    gctINT samplerBase;
    gctINT vsBase, psBase;
    gctBOOL remapSamplerAddress = gcvFALSE;

    gcoHAL_QuerySamplerBase(gcvNULL,
                            gcvNULL,
                            &vsBase,
                            gcvNULL,
                            &psBase);

    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SAMPLER_BASE_OFFSET))
    {
        vsBase = 0;
    }

    /* Determine starting sampler index. */
    samplerBase = (Shader->type == gcSHADER_TYPE_VERTEX)
            ? vsBase
            : psBase;

    gcSHADER_CheckUniformUsage(Shader, Flags);

    for (i = 0; i < (gctINT)Shader->uniformCount; ++i)
    {
        gcUNIFORM uniform = Shader->uniforms[i];

        if (uniform == gcvNULL) continue;

        if (isUniformSampler(uniform))
        {
            if (isUniformUsedInShader(uniform) ||
                isUniformSamplerCalculateTexSize(uniform))
            {
                NewPhysical[i] = sampler + samplerBase;
                sampler += GetUniformArraySize(uniform);
            }

            if (NewPhysical[i] != GetUniformPhysical(uniform))
            {
                remapSamplerAddress = gcvTRUE;
            }
        }
    }

    if (RemapSamplerAddress)
    {
        *RemapSamplerAddress = remapSamplerAddress;
    }

    return status;
}

static gctBOOL
_IsExistOtherSamplerIdx(
    IN gcSHADER Shader,
    IN gctUINT32 StartCodeIdx,
    IN gctUINT32 EndCodeIdx,
    IN gctUINT16 TempIndex
    )
{
    gctUINT32 i;
    gcSL_INSTRUCTION code;

    if (StartCodeIdx >= EndCodeIdx)
    {
        return gcvFALSE;
    }

    for (i = EndCodeIdx - 1; i > StartCodeIdx; i--)
    {
        code = Shader->code + i;

        if ((gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_GET_SAMPLER_IDX &&
            code->tempIndex == TempIndex)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

/*
** Some samplers are not used in this shader and they are not allocated for physical register,
** so we need to recompute the sampler physical address.
*/
gceSTATUS
gcLINKTREE_ComputeSamplerPhysicalAddress(
    IN OUT gcLINKTREE Tree
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER shader = Tree->shader;
    gctUINT32 i;
    gcSL_INSTRUCTION code;
    gcSL_OPCODE opcode;
    gctPOINTER pointer = gcvNULL;
    gctINT *newPhysical = gcvNULL;
    gctBOOL remapSamplerAddress = gcvFALSE;
    gcUNIFORM uniform;

    if (shader->uniformCount == 0)
        return status;

    status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(gctINT) * shader->uniformCount, &pointer);

    if (pointer == gcvNULL)
        return status;

    newPhysical = (gctINT *)pointer;
    gcoOS_MemFill(newPhysical, 0xFF, gcmSIZEOF(gctINT) * shader->uniformCount);

    /* Compute all sampler uniform address. */
    _InitializeSamplerAddress(shader, newPhysical, Tree->flags, &remapSamplerAddress);

    for (i = 0; i < shader->codeCount; ++i)
    {
        gctBOOL removeThisCode = gcvTRUE;
        gctBOOL removeUse = gcvTRUE;
        gcsLINKTREE_LIST_PTR user, nextUser;

        code = &shader->code[i];
        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

        if (opcode != gcSL_GET_SAMPLER_IDX)
        {
            continue;
        }

        /* This sampler assignment is for function argument, just change it to ADD/MOV. */
        if ((gctINT)Tree->tempArray[code->tempIndex].inputOrOutput != -1)
        {
            /* New CG should handle this in VIR. */
            union
            {
                gctFLOAT f;
                gctUINT16 hex[2];
                gctUINT32 hex32;
            } index, offsetConst;
            gcSL_FORMAT format;
            gctINT newIndex;
            gctUINT32 offsetIndex;
            gcSL_INDEXED mode;

            /* Get the sampler uniform. */
            gcmONERROR(gcSHADER_GetUniform(shader,
                                            gcmSL_INDEX_GET(code->source0Index, Index),
                                            &uniform));
            /* Get the format. */
            format = (gcSL_FORMAT)gcmSL_TARGET_GET(code->temp, Format);

            /* Get the array index. */
            mode = (gcSL_INDEXED)gcmSL_SOURCE_GET(code->source0, Indexed);

            if (mode == gcSL_NOT_INDEXED)
            {
                offsetIndex = 0;
                offsetConst.hex32 = (gctUINT32)gcmSL_INDEX_GET(code->source0Index, ConstValue);
            }
            else
            {
                offsetIndex = code->source0Indexed;
                offsetConst.hex32 = 0;
            }

            /* Update physical address. */
            if (ENABLE_FULL_NEW_LINKER)
            {
                /* Clean the indexed and move the indexed to SOURCE1. */
                code->source0 = gcmSL_SOURCE_SET(code->source0, Indexed, gcSL_NOT_INDEXED);
                code->source0Index = gcmSL_INDEX_SET(code->source0Index, ConstValue, 0);
                code->source0Indexed = 0;

                if (mode == gcSL_NOT_INDEXED)
                {
                    code->source1 = gcmSL_SOURCE_SET(code->source0, Type, gcSL_CONSTANT);
                    code->source1 = gcmSL_SOURCE_SET(code->source1, Format, format);
                    code->source1Index = offsetConst.hex[0];
                    code->source1Indexed = offsetConst.hex[1];
                }
                else
                {
                    gcSL_SWIZZLE swizzle = (gcSL_SWIZZLE)gcmComposeSwizzle(mode - 1,
                                                                           mode - 1,
                                                                           mode - 1,
                                                                           mode - 1);
                    code->source1 = gcmSL_SOURCE_SET(code->source0, Type, gcSL_TEMP);
                    code->source1 = gcmSL_SOURCE_SET(code->source1, Format, format);
                    code->source1 = gcmSL_SOURCE_SET(code->source1, Swizzle, swizzle);
                    code->source1Index = (gctUINT16)offsetIndex;
                    code->source1Indexed = 0;
                }
            }
            else
            {
                /* Get the new physical address. */
                newIndex = newPhysical[uniform->index];
                if (format == gcSL_FLOAT)
                {
                    index.f = (gctFLOAT)newIndex + offsetConst.f;
                }
                else
                {
                    index.hex32 = (gctINT)newIndex + offsetConst.hex32;
                }

                code->source0 = gcmSL_SOURCE_SET(code->source0, Type, gcSL_CONSTANT);
                code->source0 = gcmSL_SOURCE_SET(code->source0, Format, format);
                code->source0 = gcmSL_SOURCE_SET(code->source0, Indexed, gcSL_NOT_INDEXED);
                code->source0Index = index.hex[0];
                code->source0Indexed = index.hex[1];

                if (mode == gcSL_NOT_INDEXED)
                {
                    code->opcode = gcSL_MOV;
                    code->source1 = 0;
                    code->source1Index = 0;
                    code->source1Indexed = 0;
                }
                else
                {
                    gcSL_SWIZZLE swizzle = (gcSL_SWIZZLE)gcmComposeSwizzle(mode - 1,
                                                                           mode - 1,
                                                                           mode - 1,
                                                                           mode - 1);
                    code->opcode = gcSL_ADD;
                    code->source1 = gcmSL_SOURCE_SET(code->source0, Type, gcSL_TEMP);
                    code->source1 = gcmSL_SOURCE_SET(code->source1, Swizzle, swizzle);
                    code->source1Index = (gctUINT16)offsetIndex;
                    code->source1Indexed = 0;
                }
            }
            continue;
        }

        /* Map SAMPLER index to UNIFORM index. */
        for (user = Tree->tempArray[code->tempIndex].users; user;)
        {
            gcSL_INSTRUCTION userCode;
            gcSL_INDEXED indexed0, indexed1;
            userCode = shader->code + user->index;

            if (_IsExistOtherSamplerIdx(shader, i, (gctUINT32)user->index, code->tempIndex))
            {
                user = user->next;
                removeUse = gcvFALSE;
                continue;
            }

            if (!gcSL_isOpcodeTexld(gcmSL_OPCODE_GET(userCode->opcode, Opcode)) &&
                !gcSL_isOpcodeTexldModifier(gcmSL_OPCODE_GET(userCode->opcode, Opcode)))
            {
                user = user->next;
                removeThisCode = gcvFALSE;
                removeUse = gcvFALSE;
                continue;
            }

            indexed0 = (gcSL_INDEXED)gcmSL_SOURCE_GET(userCode->source0, Indexed);
            indexed1 = (gcSL_INDEXED)gcmSL_SOURCE_GET(userCode->source1, Indexed);

            gcmASSERT((indexed0 != gcSL_NOT_INDEXED) || (indexed1 != gcSL_NOT_INDEXED));

            /* Update source0. */
            if (indexed0 != gcSL_NOT_INDEXED && userCode->source0Indexed == code->tempIndex)
            {
                gcmASSERT(gcmSL_SOURCE_GET(userCode->source0, Type) == gcSL_SAMPLER);

                userCode->source0 = code->source0;
                userCode->source0 = gcmSL_SOURCE_SET(userCode->source0, Swizzle, gcSL_SWIZZLE_XYZW);
                userCode->source0Index = code->source0Index;
                userCode->source0Indexed = code->source0Indexed;
            }

            /* Update source1. */
            if (indexed1 != gcSL_NOT_INDEXED && userCode->source1Indexed == code->tempIndex)
            {
                gcmASSERT(gcmSL_SOURCE_GET(userCode->source1, Type) == gcSL_SAMPLER);

                userCode->source1 = code->source0;
                userCode->source1 = gcmSL_SOURCE_SET(userCode->source1, Swizzle, gcSL_SWIZZLE_XYZW);
                userCode->source1Index = code->source0Index;
                userCode->source1Indexed = code->source0Indexed;
            }

            nextUser = user->next;
            _RemoveItem(Tree, &Tree->tempArray[code->tempIndex].users, gcSL_NONE, user->index);
            user = nextUser;
        }

        if (removeUse)
        {
            Tree->tempArray[code->tempIndex].inUse = 0;
            Tree->tempArray[code->tempIndex].lastUse = 0;
        }

        if (removeThisCode)
        {
            /* This temp register is not used any more, change this instruction to NOP. */
            code->opcode = gcSL_NOP;
            code->temp = code->tempIndex = code->tempIndexed = 0;
            code->source0 = code->source0Index = code->source0Indexed = 0;
            code->source1 = code->source1Index = code->source1Indexed = 0;
        }
    }

    /* Update sampler address. */
    if (remapSamplerAddress)
    {
        for (i = 0; i < shader->uniformCount; i++)
        {
            uniform = shader->uniforms[i];

            if (uniform && isUniformSampler(uniform))
            {
                SetUniformPhysical(uniform, newPhysical[i]);
            }
        }
    }

OnError:
    gcmOS_SAFE_FREE(gcvNULL, newPhysical);
    return status;
}

gceSTATUS
gcLINKTREE_Cleanup(
    IN OUT gcLINKTREE Tree
    )
{
    gctSIZE_T i;
    gcSHADER shader = Tree->shader;

    for (i = 0; i < shader->codeCount; ++i)
    {
        gcSL_INSTRUCTION code = shader->code + i;
        gcLINKTREE_TEMP temp;
        gctUINT16 opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);

        if (gcSL_isOpcodeHaveNoTarget(opcode))
        {
            continue;
        }

        /* check to skip SET.Z */
        if ((opcode == gcSL_SET || opcode == gcSL_CMP) &&
            gcmSL_TARGET_GET(code->temp, Condition) == gcSL_ZERO)
        {
            if ((i + 1) < shader->codeCount)
            {
                gcSL_INSTRUCTION relatedCode = code + 1;

                /* next instruction is a SET.NZ ? */
                if ((gcmSL_OPCODE_GET(relatedCode->opcode, Opcode) == gcSL_SET ||
                    gcmSL_OPCODE_GET(relatedCode->opcode, Opcode) == gcSL_CMP) &&
                    (gcmSL_TARGET_GET(relatedCode->temp, Condition) == gcSL_NOT_ZERO) &&
                    Tree->tempArray[relatedCode->tempIndex].inUse)
                {
                    continue;
                }
            }
        }
        temp = &Tree->tempArray[code->tempIndex];
        if (!temp->inUse)
        {
            gcmSL_OPCODE_UPDATE(code->opcode, Opcode, gcSL_NOP);
            code->temp           = 0;
            code->tempIndex      = 0;
            code->tempIndexed    = 0;
            code->source0        = 0;
            code->source0Index   = 0;
            code->source0Indexed = 0;
            code->source1        = 0;
            code->source1Index   = 0;
            code->source1Indexed = 0;

            if (temp->dependencies != gcvNULL)
            {
                _Delete(Tree, &temp->dependencies);
            }

            if (temp->users != gcvNULL)
            {
                _Delete(Tree, &temp->users);
            }
        }
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcLINKTREE_CheckAPILevelResource(
    IN OUT gcLINKTREE Tree
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gctUINT         uniformCount = 0, samplerCount = 0, imageCount = 0, atomicCount = 0;
    gctUINT         maxUniformCount = 0, maxSamplerCount = 0, maxImageCount = 0, maxAtomicCount = 0;
    gctUINT         i;
    gcSHADER        shader;

    if (!Tree)
        return status;

    shader = Tree->shader;
    /* check uniform usage: if a uniform is used in shader or LTC expression */
    gcSHADER_CheckUniformUsage(shader, Tree->flags);

    if (shader->clientApiVersion == gcvAPI_OPENCL)
    {
        return status;
    }

    if (shader->clientApiVersion != gcvAPI_OPENGL_ES20 &&
             shader->clientApiVersion != gcvAPI_OPENGL_ES30 &&
             shader->clientApiVersion != gcvAPI_OPENGL_ES31)
    {
        return status;
    }

    /* I: Check uniform resource. */
    switch (shader->type)
    {
    case gcSHADER_TYPE_VERTEX:
        maxUniformCount = GetGLMaxVertexUniformVectors();
        maxSamplerCount = GetGLMaxVertexTextureImageUnits();
        maxImageCount = GetGLMaxVertexImageUniforms();
        maxAtomicCount = GetGLMaxVertexAtomicCounters();
        break;
    case gcSHADER_TYPE_TCS:
        maxUniformCount = GetGLMaxTCSUniformVectors();
        maxSamplerCount = GetGLMaxTCSTextureImageUnits();
        maxImageCount = GetGLMaxTCSImageUniforms();
        maxAtomicCount = GetGLMaxTCSAtomicCounters();
        break;
    case gcSHADER_TYPE_TES:
        maxUniformCount = GetGLMaxTESUniformVectors();
        maxSamplerCount = GetGLMaxTESTextureImageUnits();
        maxImageCount = GetGLMaxTESImageUniforms();
        maxAtomicCount = GetGLMaxTESAtomicCounters();
        break;
    case gcSHADER_TYPE_GEOMETRY:
        maxUniformCount = GetGLMaxGSUniformVectors();
        maxSamplerCount = GetGLMaxGSTextureImageUnits();
        maxImageCount = GetGLMaxGSImageUniforms();
        maxAtomicCount = GetGLMaxGSAtomicCounters();
        break;
    case gcSHADER_TYPE_FRAGMENT:
        maxUniformCount = GetGLMaxFragmentUniformVectors();
        maxSamplerCount = GetGLMaxFragTextureImageUnits();
        maxImageCount = GetGLMaxFragmentImageUniforms();
        maxAtomicCount = GetGLMaxFragmentAtomicCounters();
        break;
    case gcSHADER_TYPE_COMPUTE:
        maxUniformCount = GetGLMaxComputeUniformComponents() / 4;
        maxSamplerCount = GetGLMaxComputeTextureImageUnits();
        maxImageCount = GetGLMaxCombinedImageUniforms();
        maxAtomicCount = GetGLMaxComputeAtomicCounters();
        break;
    default:
        return status;
    }

    /* Make sure that sampler uniforms that related to built-in uniform lod_min_max and level_base_size are actived. */
    for (i = 0; i < (gctINT)shader->uniformCount; ++i)
    {
        gcUNIFORM uniform = shader->uniforms[i], sampler;

        if (!uniform) continue;
        if (!isUniformLodMinMax(uniform) &&
            !isUniformLevelBaseSize(uniform) &&
            !isUniformExtraForTexGather(uniform)&&
            !isUniformMLSampler(uniform))
        {
            continue;
        }

        sampler = shader->uniforms[uniform->parent];
        gcmASSERT(uniform->parent < (gctINT16)shader->uniformCount && sampler);
        /* Mark this sampler as use to calucate texture size.  */
        SetUniformFlag(sampler, gcvUNIFORM_FLAG_SAMPLER_CALCULATE_TEX_SIZE);
    }

    for (i = 0; i < shader->uniformCount; i++)
    {
        gcUNIFORM uniform = shader->uniforms[i];

        if (!uniform)
            continue;

        if (isUniformSamplerCalculateTexSize(uniform))
        {
            samplerCount += (gctUINT)GetUniformArraySize(uniform);
            if (samplerCount > maxSamplerCount)
                return gcvSTATUS_TOO_MANY_UNIFORMS;
            continue;
        }

        if (uniform->name[0] == '#')
            continue;

        /* skip non-used uniforms. */
        if (!isUniformUsedInShader(uniform) && !isUniformUsedInLTC(uniform))
            continue;

        /* skip SSBO base address uniform. */
        if (isUniformStorageBlockAddress(uniform))
            continue;

        /* skip UBO that without LOAD. */
        if (isUniformBlockAddress(uniform) && !isUniformUsedInShader(uniform))
            continue;

        /* skip UBO member. */
        if (isUniformBlockMember(uniform))
            continue;

        /* skip compiler generated uniforms. */
        if (isUniformCompilerGen(uniform) || isUniformImmediate(uniform))
            continue;

        if (isUniformSampler(uniform))
        {
            samplerCount += (gctUINT)GetUniformArraySize(uniform);

            if (samplerCount > maxSamplerCount)
                return gcvSTATUS_TOO_MANY_UNIFORMS;
        }
        else if (isUniformImage(uniform))
        {
            imageCount += (gctUINT)GetUniformArraySize(uniform);
            if (imageCount > maxImageCount)
                return gcvSTATUS_TOO_MANY_UNIFORMS;
        }
        else if (isUniformAtomicCounter(uniform))
        {
            atomicCount += (gctUINT)GetUniformArraySize(uniform);
            if (atomicCount > maxAtomicCount)
                return gcvSTATUS_TOO_MANY_UNIFORMS;
        }
        else
        {
            gctUINT32 rows = 0;

            gcTYPE_GetTypeInfo(uniform->u.type, gcvNULL, &rows, 0);
            if(gcmType_Kind(uniform->u.type) == gceTK_ATOMIC ||
               isUniformMatrix(uniform) ||
               !isUniformArray(uniform) ||
               !isUniformNormal(uniform) ||
               !(Tree->flags & gcvSHADER_REMOVE_UNUSED_UNIFORMS))
            {
                SetUniformUsedArraySize(uniform, GetUniformArraySize(uniform));
            }
            uniformCount += rows * GetUniformUsedArraySize(uniform);
            if (uniformCount > maxUniformCount)
                return gcvSTATUS_TOO_MANY_UNIFORMS;
        }
    }

    return status;
}


/* for orignal code Index (before removing NOPs) in Shader,
 * get the new index to NOP removed code, if the original code
 * is NOP, find the next code which is non NOP and get the
 * the new index, if all the rest codes are NOPs, return index
 * to Shader's last instruction (lastInstruction -1) */
static gctINT
_GetNewIndex2NextCode(
    IN gcSHADER    Shader,
    IN gctINT16 *  Index_map,
    IN gctINT      Map_entries,
    IN gctINT      Index
    )
{
    gctSIZE_T new_index;

    gcmASSERT(Index >= 0 && Index < Map_entries);

    while (Index < Map_entries && Index_map[Index] == -1)
        Index++;

    if (Index >= Map_entries)
    {
        /* no next non NOP instruction found, use the lastInstruction */
        new_index = Shader->lastInstruction;
    }
    else
    {
        new_index = Index_map[Index];
    }

    gcmASSERT(new_index == Shader->lastInstruction ||
              gcmSL_OPCODE_GET(Shader->code[new_index].opcode, Opcode) != gcSL_NOP);
    return new_index;
}

/* for orignal code Index (before removing NOPs) in Shader,
 * get the new index to NOP removed code, if the original code
 * is NOP, find the previous code which is non NOP and get the
 * the new index, if all the rest codes are NOPs, return
 * Shader's first Instruction */
gctINT
_GetNewIndex2PrevCode(
    IN gcSHADER    Shader,
    IN gctINT16 *  Index_map,
    IN gctINT      Map_entries,
    IN gctINT      Index
    )
{
    gctSIZE_T new_index;

    gcmASSERT(Index >= 0);

    while (Index >= 0 && Index_map[Index] == -1)
        Index--;

    if (Index < 0)
    {
        /* no next non NOP instruction found, use the lastInstruction */
        new_index = 0;
    }
    else
    {
        new_index = Index_map[Index];
    }

    gcmASSERT(new_index == 0 ||
              Shader->code[new_index].opcode != gcSL_NOP);
    return new_index;
}


gceSTATUS
gcLINKTREE_RemoveNops(
    IN OUT gcLINKTREE Tree
    )
{
    gctSIZE_T    i;
    gcSHADER     Shader         = Tree->shader;

    gceSTATUS    status;
    gctINT16 *   index_map;
    gctSIZE_T    map_entries    = Shader->codeCount;
    gctSIZE_T    bytes          = map_entries * gcmSIZEOF(gctINT16);
    gctPOINTER   pointer        = gcvNULL;
    gctINT       nop_counts     = 0;
    gctINT       first_nop_index=-1;  /* first available nop index */
    gctBOOL      has_jump_to_last_nop_stmt = gcvFALSE;

     /* Check for empty shader */
    if (bytes == 0)
    {
        return gcvSTATUS_OK;
    }

    gcmASSERT(Shader->lastInstruction <= Shader->codeCount);

    /* Allocate array of gcsSL_JUMPED_FROM structures. */
    status = gcoOS_Allocate(gcvNULL, bytes, &pointer);

    if (gcmIS_ERROR(status))
    {
        /* Return on error. */
        return status;
    }

    index_map = pointer;

    /* Initialize array with -1. */
    for (i = 0; i < Shader->codeCount; ++i)
        index_map[i] = -1;

    /* check if the lastInstruction is NOP */
    if (gcmSL_OPCODE_GET(Shader->code[Shader->lastInstruction-1].opcode, Opcode) == gcSL_NOP)
    {
        /* find the pervious non NOP instruction */
        gctINT last = Shader->lastInstruction;
        while (last > 0 &&
               gcmSL_OPCODE_GET(Shader->code[--last].opcode, Opcode) == gcSL_NOP) ;

        if (gcmSL_OPCODE_GET(Shader->code[last].opcode, Opcode) == gcSL_NOP)
        {
            /* all instructions are NOP */
            gcoOS_Free(gcvNULL, index_map);
            return gcvSTATUS_OK;
        }
        else
        {
            /* found non NOP instruction */
            Shader->codeCount = Shader->lastInstruction = last+1;
        }
    } /* if */

    /* Walk through all the instructions, finding JMP instructions. */
    for (i = 0; i < Shader->codeCount; ++i)
    {
        if (gcmSL_OPCODE_GET(Shader->code[i].opcode, Opcode) == gcSL_NOP)
        {
            if (first_nop_index == -1)
                first_nop_index = i;

            nop_counts ++;
        }
        else
        {
            /* not a nop, copy to last nop location if there is one */
            if (first_nop_index != -1)
            {
                /* copy the instruction */
                Shader->code[first_nop_index] = Shader->code[i];
                gcmSL_OPCODE_UPDATE(Shader->code[i].opcode,Opcode, gcSL_NOP);

                /* map the index */
                index_map[i] = (gctINT16) first_nop_index;

                /* make the next instruction to be first available nop instruction */
                first_nop_index++;
            }
            else
            {
                index_map[i] = (gctINT16) i;
            }
        }
    } /* for */

    /* fix the lastInstruction */
    Shader->codeCount =
        Shader->lastInstruction = Shader->lastInstruction - nop_counts;

    for (i = 0; i < Shader->codeCount; ++i)
    {
        /* fix up jmp/call. */
        if (gcmSL_OPCODE_GET(Shader->code[i].opcode, Opcode) == gcSL_JMP ||
            gcmSL_OPCODE_GET(Shader->code[i].opcode, Opcode) == gcSL_CALL)
        {
            /* Get target of jump instrution. */
            gctINT new_label =
                _GetNewIndex2NextCode(Shader,
                                      index_map,
                                      map_entries,
                                      Shader->code[i].tempIndex);

            if (new_label >= (gctINT)Shader->lastInstruction)
                has_jump_to_last_nop_stmt = gcvTRUE;

            Shader->code[i].tempIndex = (gctUINT16)new_label;
        }
    } /* for */

    if (has_jump_to_last_nop_stmt == gcvTRUE)
    {
        /* the code can jump to last NOP stmt:
         *
         *    005   JMP 010
         *    ...
         *    010   NOP
         *    011   NOP
         *
         * we need to keep the last NOP in the code to
         * make it semantic correct after remove nops
         */
        gcmASSERT(gcmSL_OPCODE_GET(Shader->code[Shader->lastInstruction].opcode, Opcode) == gcSL_NOP);
        Shader->codeCount =
            Shader->lastInstruction = Shader->lastInstruction + 1;
    } /* if */

    /* fix functions */
    /******************** Check the argument temp index Function **********************/
    for (i = 0; i < Shader->functionCount; ++i)
    {
        gcFUNCTION function = Shader->functions[i];
        gctINT newCodeStart;
        gctINT newCodeCount = 0;
        gctSIZE_T j;

        newCodeStart =
            _GetNewIndex2NextCode(Shader,
                                  index_map,
                                  map_entries,
                                  function->codeStart);

        /* count the new code in the function */
        for (j = 0; j < function->codeCount; j++)
        {
            if (index_map[function->codeStart + j] != -1)
                newCodeCount++;
        }

        gcmASSERT(newCodeCount != 0);  /* at least one instruction (RET) */

        function->codeStart = newCodeStart;
        function->codeCount = newCodeCount;
    } /* for */

    /*************** Check the argument temp index for Kernel Function *****************/
    for (i = 0; i < Shader->kernelFunctionCount; ++i)
    {
        gcKERNEL_FUNCTION kernelFunction = Shader->kernelFunctions[i];
        gctINT newCodeStart;
        gctINT newCodeCount = 0;
        gctSIZE_T j;

        newCodeStart =
            _GetNewIndex2NextCode(Shader,
                                  index_map,
                                  map_entries,
                                  kernelFunction->codeStart);

        /* count the new code in the function */
        for (j = 0; j < kernelFunction->codeCount; j++)
        {
            if (index_map[kernelFunction->codeStart + j] != -1)
                newCodeCount++;
        }

        kernelFunction->codeStart = newCodeStart;
        kernelFunction->codeCount = newCodeCount;
        kernelFunction->codeEnd   = newCodeStart + newCodeCount;
    } /* for */


    do {
        gcSL_BRANCH_LIST            branch;

        /* Walk all attributes. */
        for (i = 0; i < Tree->attributeCount; i++)
        {
            gcLINKTREE_ATTRIBUTE attribute = &Tree->attributeArray[i];
            gcsLINKTREE_LIST_PTR users;

            if (attribute->lastUse >= 0)
            {
                attribute->lastUse = _GetNewIndex2NextCode(Shader,
                                                       index_map,
                                                       map_entries,
                                                       attribute->lastUse);
            }

            /* Loop while there are users. */
            users = attribute->users;
            while (users != gcvNULL)
            {
                users->index = _GetNewIndex2NextCode(Shader,
                                                     index_map,
                                                     map_entries,
                                                     users->index);

                /* Remove the user from the list. */
                users = users->next;
            }
        } /* for */

        /* Loop through all temporary registers. */
        for (i = 0; i < Tree->tempCount; i++)
        {
            gcLINKTREE_TEMP             tempArray = &Tree->tempArray[i];
            gcsLINKTREE_LIST_PTR        defined, users;

            /* Only process if temporary register is defined. */
            if (tempArray->defined == gcvNULL)
            {
                continue;
            }

            if (tempArray->lastUse >= 0)
                tempArray->lastUse =  _GetNewIndex2NextCode(Shader,
                                                            index_map,
                                                            map_entries,
                                                            tempArray->lastUse);

            /* Loop while there are definitions. */
            defined = tempArray->defined;
            while (defined != gcvNULL)
            {
                /* fix the definition's index. */
                defined->index = _GetNewIndex2NextCode(Shader,
                                                       index_map,
                                                       map_entries,
                                                       defined->index);

                defined = defined->next;
            }

            /* Loop while we have users. */
            users = tempArray->users;
            while (users != gcvNULL)
            {
                /* fix the users' index. */
                users->index = _GetNewIndex2NextCode(Shader,
                                                     index_map,
                                                     map_entries,
                                                     users->index);

                users = users->next;
            }
        } /* for */

        branch = Tree->branch;
        while (branch != gcvNULL)
        {
            /* fix branch's target. */
            branch->target = _GetNewIndex2NextCode(Shader,
                                                   index_map,
                                                   map_entries,
                                                   branch->target);
            branch = branch->next;
        } /* while */

        /* add the last code to index_map if it is an non-NOP code. */
        if (gcmSL_OPCODE_GET(Shader->code[map_entries - 1].opcode, Opcode) != gcSL_NOP)
        {
            index_map[map_entries - 1] = (gctINT16) (Shader->codeCount - 1);
        }

        /* compact hints */
        for (i = 0; i < map_entries; ++i)
        {
            if (index_map[i] != -1 && index_map[i] != (gctINT16)i)
            {
                gcsCODE_HINT_PTR hint;
                gctINT new_hint_index;

                /* find the hint's new location in hint array */
                new_hint_index = index_map[i];
                gcmASSERT(new_hint_index < (gctINT)i);

                /* copy to new location */
                Tree->hints[new_hint_index] = Tree->hints[i];
                hint = &Tree->hints[new_hint_index];

                /* fix the hint's lastUseForTemp */
                if (hint->lastUseForTemp >= 0)
                {
                    hint->lastUseForTemp =
                        _GetNewIndex2NextCode(Shader,
                                              index_map,
                                              map_entries,
                                              hint->lastUseForTemp);
                }

                /* fix callers */
                if (hint->callers)
                {
                    gcsCODE_CALLER_PTR    callers = hint->callers;
                    while (callers != gcvNULL)
                    {
                        callers->caller =
                            _GetNewIndex2NextCode(Shader,
                                                  index_map,
                                                  map_entries,
                                                  callers->caller);
                        callers = callers->next;
                    }
                } /* if */
            } /* if */
        } /* for */
    } while (0);

    /* Free index_map. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, index_map));

    /* Return success. */
    return gcvSTATUS_OK;
} /* gcLINKTREE_RemoveNops */

static gceSTATUS
_gcCheckShadersVersion(
    IN gcSHADER VertexShader,
    IN gcSHADER FragmentShader
    )
{
    gctUINT vertexLangVersion, fragLangVersion;

    vertexLangVersion = VertexShader->compilerVersion[0] & 0xFF;

    fragLangVersion = FragmentShader->compilerVersion[0] & 0xFF;

    if(vertexLangVersion != fragLangVersion) {
#if gcdDEBUG
        gctUINT8 *verVersionPtr = (gctUINT8 *)&vertexLangVersion;
        gctUINT8 *fragVersionPtr = (gctUINT8 *)&fragLangVersion;

        gcmFATAL("_gcCheckShadersVersion: verter shader's version of %u "
                 "does not match with fragment shader's version of %u",
                 (verVersionPtr[0] == 'E' && verVersionPtr[1] == 'S')  ? 100 : 300,
                 (fragVersionPtr[0] == 'E' && fragVersionPtr[1] == 'S')  ? 100 : 300);
#endif
        return gcvSTATUS_INVALID_DATA;
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
PatchShaders(
    IN gcSHADER PreRAShader,
    IN gcSHADER FragmentShader
    )
{
    gctSIZE_T i;
    gceSTATUS status = gcvSTATUS_OK;
    gctINT position = -1;
    gctBOOL hasPositionW = gcvFALSE;

    for (i = 0; i < FragmentShader->attributeCount; ++i)
    {
        /* the attribute is not in use and was freed */
        if (FragmentShader->attributes[i] == gcvNULL)
            continue;

        if (FragmentShader->attributes[i]->nameLength == gcSL_POSITION)
        {
            gctSIZE_T j;

            for (j = 0; j < PreRAShader->outputCount; ++j)
            {
                if ((PreRAShader->outputs[j] != gcvNULL)
                &&  (PreRAShader->outputs[j]->nameLength == gcSL_POSITION)
                )
                {
                    position = PreRAShader->outputs[j]->tempIndex;
                    gcmASSERT(PreRAShader->outputs[j]->precision == gcSHADER_PRECISION_HIGH);
                }

                if ((PreRAShader->outputs[j] != gcvNULL)
                &&  (PreRAShader->outputs[j]->nameLength == gcSL_POSITION_W)
                )
                {
                    hasPositionW = gcvTRUE;
                    break;
                }
            }
        }
        else if (!ENABLE_FULL_NEW_LINKER && FragmentShader->attributes[i]->nameLength == gcSL_POINT_COORD)
        {
            gctSIZE_T j;

            gcmATTRIBUTE_SetIsTexture(FragmentShader->attributes[i], gcvTRUE);

            for (j = 0; j < PreRAShader->outputCount; ++j)
            {
                if ((PreRAShader->outputs[j] != gcvNULL)
                &&  (PreRAShader->outputs[j]->nameLength == gcSL_POINT_COORD)
                )
                {
                    break;
                }
            }

            if (j == PreRAShader->outputCount)
            {
                gcmERR_BREAK(gcSHADER_AddOutput(PreRAShader,
                                                "#PointCoord",
                                                gcSHADER_FLOAT_X2,
                                                1,
                                                0,
                                                gcSHADER_PRECISION_DEFAULT));
            }

            break;
        }
    }

    if (position != -1)
    {
        if (!hasPositionW
        &&  (gcoHAL_IsFeatureAvailable1(gcvNULL,
                                        gcvFEATURE_SHADER_HAS_W)
             == gcvSTATUS_FALSE)
        )
        {
            gctINT temp = -1;

            for (i = 0; i < PreRAShader->codeCount; ++i)
            {
                gcSL_OPCODE opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(PreRAShader->code[i].opcode, Opcode);

                if (gcSL_isOpcodeHaveNoTarget(opcode))
                {
                    continue;
                }

                if (PreRAShader->code[i].tempIndex > temp)
                {
                    temp = PreRAShader->code[i].tempIndex;
                }
            }

            do
            {
                gcATTRIBUTE positionW;

                gcmERR_BREAK(gcSHADER_AddOpcode(PreRAShader,
                                                gcSL_MOV,
                                                (gctUINT16) (temp + 1),
                                                0x1,
                                                gcSL_FLOAT,
                                                gcSHADER_PRECISION_HIGH));

                gcmERR_BREAK(gcSHADER_AddSource(PreRAShader,
                                                gcSL_TEMP,
                                                (gctUINT16) position,
                                                gcSL_SWIZZLE_WWWW,
                                                gcSL_FLOAT,
                                                gcSHADER_PRECISION_HIGH));

                gcmERR_BREAK(gcSHADER_AddOutput(PreRAShader,
                                                "#Position.w",
                                                gcSHADER_FLOAT_X1,
                                                1,
                                                (gctUINT16) (temp + 1),
                                                gcSHADER_PRECISION_HIGH));

                gcmERR_BREAK(gcSHADER_Pack(PreRAShader));

                gcmERR_BREAK(gcSHADER_AddAttribute(FragmentShader,
                                                   "#Position.w",
                                                   gcSHADER_FLOAT_X1,
                                                   1,
                                                   gcvFALSE,
                                                   gcSHADER_SHADER_DEFAULT,
                                                   gcSHADER_PRECISION_HIGH,
                                                   &positionW));
            }
            while (gcvFALSE);
        }
    }



    return status;
}

static gctBOOL
_FindTexLodAndTexBias(
    IN gcSHADER VertexShader,
    IN gcSHADER FragmentShader
    )
{
    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TEXTURE_BIAS_LOD_FIX) == gcvSTATUS_FALSE)
    {
        gcShaderCodeInfo  vertexCodeInfo, fragCodeInfo;

        gcoOS_ZeroMemory(&vertexCodeInfo, gcmSIZEOF(gcShaderCodeInfo));
        if (VertexShader)
        {
            gcSHADER_CountCode(VertexShader, &vertexCodeInfo);
        }

        gcoOS_ZeroMemory(&fragCodeInfo, gcmSIZEOF(gcShaderCodeInfo));
        if (FragmentShader)
        {
            gcSHADER_CountCode(FragmentShader, &fragCodeInfo);
        }

        /* check if gcSL_TEXBIAS or gcSL_TEXLOD is used */
        if (vertexCodeInfo.codeCounter[gcSL_TEXLOD] != 0 ||
            fragCodeInfo.codeCounter[gcSL_TEXBIAS] != 0)
        {
            return gcvTRUE;
        }
        return gcvFALSE;
    }
    return gcvFALSE;
}

static void
_TraceStrictModelViewProjection(
    gcLINKTREE Tree,
    gctINT Temp,
    gctINT MVPStartIndex,
    gctINT * MatchCount
    )
{
    gcLINKTREE_TEMP temp = &Tree->tempArray[Temp];
    gcsLINKTREE_LIST_PTR define = gcvNULL;
    gcSHADER_LIST list = gcvNULL;
    gcSL_INSTRUCTION code;

    if (temp == gcvNULL)
        return;

    if (temp->defined == gcvNULL)
        return;

    define = temp->defined;

    /* gl_Position can only have one definition. */
    if (temp->defined->next != gcvNULL)
    {
        return;
    }

    if (define == gcvNULL)
        return;

    code = Tree->shader->code + define->index;

    if (code->opcode == gcSL_ADD)
    {
        gcSHADER_FindList(Tree->shader, Tree->shader->wClipTempIndexList, Temp, &list);
    }
    else
    {
        return;
    }

    if (list != gcvNULL)
    {
        gcSL_TYPE type0, type1;
        gcSL_SWIZZLE swizzle;
        gcSL_ENABLE enable;
        gcUNIFORM uniform = gcvNULL, uniform1 = gcvNULL;
        gcATTRIBUTE attribute = gcvNULL;
        gctINT index0, index1;
        gctINT attributeIndex = -1, uniformIndex = -1, uniformIndex1 = -1;
        gctINT i, patternCount = 0, channelCount = 0, mulCount = 0;
        gctBOOL isMVPHoldByLTC = gcvFALSE;

        type0 = list->data0 & 0xffff;
        index0 = list->data0 >> 16;
        type1 = list->data1 & 0xffff;
        index1 = list->data1 >> 16;

        /*---------------------------I: Find the uniform---------------------------*/
        if (type0 == gcSL_UNIFORM)
        {
            uniformIndex = index0;
        }
        else if (type0 == gcSL_CONSTANT)
        {
            gcSHADER_LIST list = gcvNULL;

            uniformIndex = index0 & 0xff;
            uniformIndex1 = (index0 >> 8) & 0xff;

            type0 = gcSL_UNIFORM;
            gcSHADER_FindListByData(Tree->shader, Tree->shader->wClipUniformIndexList, uniformIndex, uniformIndex1, &list);
            if (list != gcvNULL)
            {
                index0 = list->index;
            }
            isMVPHoldByLTC = gcvTRUE;
        }
        else
        {
            if (index0 >= (gctINT)Tree->tempCount || Tree->tempArray[index0].defined == gcvNULL)
                return;

            /* Right now, we only check one define.*/
            if (Tree->tempArray[index0].defined->next != gcvNULL)
            {
                return;
            }
            code = Tree->shader->code + Tree->tempArray[index0].defined->index;

            /* Skip non uniform assignment or indexed uniform. */
            if (code->opcode != gcSL_MOV ||
                gcmSL_SOURCE_GET(code->source0, Type) != gcSL_UNIFORM ||
                gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED)
            {
                return;
            }
            uniformIndex = code->source0Index;
        }

        if (uniformIndex == -1)
            return;
        gcmASSERT((gctSIZE_T)uniformIndex < Tree->shader->uniformCount);
        uniform = Tree->shader->uniforms[uniformIndex];

        /* Skip uniform array, non-user defined uniform. */
        if (uniform->arraySize != 1 || uniform->u.type != gcSHADER_FLOAT_4X4 || uniform->nameLength < 0)
            return;

        if (uniformIndex1 != -1)
        {
            gcmASSERT((gctSIZE_T)uniformIndex1 < Tree->shader->uniformCount);
            uniform1 = Tree->shader->uniforms[uniformIndex1];

            /* Skip uniform array, non-user defined uniform. */
            if (uniform1->arraySize != 1 || uniform1->u.type != gcSHADER_FLOAT_4X4 || uniform1->nameLength < 0)
                return;
        }

        /*---------------------------II: Check the attribute---------------------------*/
        if (type1 == gcSL_ATTRIBUTE)
        {
            attributeIndex = index1;
            attribute = Tree->shader->attributes[attributeIndex];
            for (i = define->index - 1; i >=0; i--)
            {
                code = Tree->shader->code + i;

                if (code->opcode != gcSL_MUL) continue;

                if ((gcSL_TYPE)gcmSL_SOURCE_GET(code->source0, Type) != type0 ||
                    code->source0Index != (gctUINT16)index0)
                {
                    continue;
                }

                patternCount = define->index - i;
                break;
            }
        }
        else
        {
            gcSHADER_LIST list2 = gcvNULL;

            gcSHADER_FindList(Tree->shader, Tree->shader->wClipTempIndexList, index1, &list2);

            if (list2 != gcvNULL)
            {
                gctINT startIndex = 1;

                if (uniform1 != gcvNULL)
                    startIndex = 2;
                _TraceStrictModelViewProjection(Tree, list2->index, startIndex, MatchCount);

                if (*MatchCount != 0)
                {
                    uniform->modelViewProjection = *MatchCount - startIndex;
                    if (uniform1 != gcvNULL)
                    {
                        uniform1->modelViewProjection = uniform->modelViewProjection + 1;
                    }
                    return;
                }
            }

            for (i = define->index - 1; i >=0; i--)
            {
                code = Tree->shader->code + i;

                if (code->opcode != gcSL_MUL) continue;

                if ((gcSL_TYPE)gcmSL_SOURCE_GET(code->source0, Type) != type0 ||
                    code->source0Index != (gctUINT16)index0)
                {
                    continue;
                }

                patternCount = define->index - i;

                /* Right now, all channel must be seriation. */
                if (gcmSL_SOURCE_GET(code->source1, Swizzle) != gcSL_SWIZZLE_XXXX)
                    return;

                type1 = gcmSL_SOURCE_GET(code->source1, Type);
                if (type1 == gcSL_ATTRIBUTE)
                {
                    attributeIndex = code->source1Index;
                    attribute = Tree->shader->attributes[attributeIndex];
                }
                else
                {
                    if (type1 != gcSL_TEMP)
                        return;

                    /* Right now, we only check one define.*/
                    if (Tree->tempArray[code->source1Index].defined->next)
                    {
                        return;
                    }

                    code = Tree->shader->code + Tree->tempArray[code->source1Index].defined->index;

                    if (code->opcode != gcSL_MOV ||
                        gcmSL_SOURCE_GET(code->source0, Type) != gcSL_ATTRIBUTE ||
                        gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED)
                    {
                        return;
                    }
                    attributeIndex = code->source0Index;
                    attribute = Tree->shader->attributes[attributeIndex];
                    /* All channels of this attribute must be used in order. */
                    swizzle = gcmSL_SOURCE_GET(code->source0, Swizzle);
                    enable = gcmSL_TARGET_GET(code->temp, Enable);

                    if (attribute->type == gcSHADER_FLOAT_X1)
                    {
                        if (swizzle != gcSL_SWIZZLE_XXXX || enable != gcSL_ENABLE_X)
                            return;
                    }
                    else if (attribute->type == gcSHADER_FLOAT_X2)
                    {
                        if (swizzle != gcSL_SWIZZLE_XYYY || enable != gcSL_ENABLE_XY)
                            return;
                    }
                    else if (attribute->type == gcSHADER_FLOAT_X3)
                    {
                        if (swizzle != gcSL_SWIZZLE_XYZZ || enable != gcSL_ENABLE_XYZ)
                            return;
                    }
                    else if (attribute->type == gcSHADER_FLOAT_X4)
                    {
                        if (swizzle != gcSL_SWIZZLE_XYZW || enable != gcSL_ENABLE_XYZW)
                            return;
                    }
                }
                break;
            }
        }

        if (attributeIndex == -1)
            return;

        /* Find the attribute. */
        if (attribute->nameLength < 0 || attribute->arraySize > 1 || attribute->type > gcSHADER_FLOAT_X4)
            return;

        channelCount = attribute->type - gcSHADER_FLOAT_X1 + 1;

        if (channelCount == 1)
        {
            if (patternCount != 1)
                return;
        }
        else if (channelCount == 2)
        {
            if (patternCount != 3)
                return;
        }
        else if (channelCount == 3)
        {
            if (patternCount != 5)
                return;
        }
        else
        {
            gcmASSERT(channelCount == 4);
            if (patternCount != 6)
                return;
        }

        /* Check the swizzle and count of MUL operation. */
        for (i = define->index - patternCount; i < define->index; i++)
        {
            code = Tree->shader->code + i;

            if (code->opcode != gcSL_MUL) continue;

            swizzle = gcmComposeSwizzle(mulCount, mulCount, mulCount, mulCount);

            if ((gcSL_SWIZZLE)gcmSL_SOURCE_GET(code->source1, Swizzle) != swizzle)
            {
                return;
            }

            mulCount++;
        }

        if (mulCount != channelCount)
            return;

        /* Make sure the value of w channel is 1. */
        if (channelCount != 4)
        {
            code = Tree->shader->code + define->index;

            if (!isMVPHoldByLTC)
            {
                if (gcmSL_SOURCE_GET(code->source1, Type) != gcSL_UNIFORM ||
                    gcmSL_INDEX_GET(code->source1Index, Index) != uniformIndex ||
                    gcmSL_INDEX_GET(code->source1Index, ConstValue) != 3)
                {
                    return;
                }
            }
            else
            {
                if (gcmSL_SOURCE_GET(code->source1, Type) != gcSL_UNIFORM ||
                    gcmSL_INDEX_GET(code->source1Index, Index) != index0 + channelCount)
                {
                    return;
                }
            }
        }

#if gcdUSE_WCLIP_PATCH
        if (attributeIndex != -1 && uniformIndex != -1)
        {
            if (MatchCount)
            {
                *MatchCount = MVPStartIndex + 1;
            }
            Tree->shader->uniforms[uniformIndex]->modelViewProjection = MVPStartIndex + 1;

            if (uniformIndex1 != -1)
            {
                Tree->shader->uniforms[uniformIndex]->modelViewProjection = MVPStartIndex + 2;
                if (MatchCount)
                {
                    *MatchCount = MVPStartIndex + 2;
                }
            }

            gcmATTRIBUTE_SetIsPosition(Tree->shader->attributes[attributeIndex], gcvTRUE);
            Tree->strictWClipMatch = gcvTRUE;

            if (MatchCount)
            {
                Tree->MVPCount = *MatchCount;
            }
        }
#endif
    }
}

static void
_TraceModelViewProjection(
    gcLINKTREE VertexTree,
    gctBOOL_PTR TempArray,
    gctINT Temp,
    gctINT Counter
    )
{
    gcsLINKTREE_LIST_PTR list;

    TempArray[Temp] = gcvTRUE;

    /* Check if any source is mat4 uniform. */
    for (list = VertexTree->tempArray[Temp].defined;
         list != gcvNULL;
         list = list->next)
    {
        gctUINT index = list->index;

        if (index < VertexTree->shader->codeCount)
        {
            gcSL_INSTRUCTION code = &VertexTree->shader->code[index];
            if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_UNIFORM)
            {
                gctUINT uniformIndex = gcmSL_INDEX_GET(code->source0Index, Index);
                gcSHADER_LIST list;

                if (uniformIndex < VertexTree->shader->uniformCount)
                {
                    gcUNIFORM uniform = VertexTree->shader->uniforms[uniformIndex];
                    if (uniform->u.type == gcSHADER_FLOAT_4X4 &&
                        uniform->modelViewProjection == 0)
                    {
                        Counter++;
                        uniform->modelViewProjection = Counter;
                    }
                    else if (isUniformLoadtimeConstant(uniform) &&
                        uniform->u.type == gcSHADER_FLOAT_X4 &&
                        gcSHADER_FindList(VertexTree->shader, VertexTree->shader->wClipUniformIndexList, (gctINT)uniformIndex, &list))
                    {
                        Counter++;
                        VertexTree->shader->uniforms[list->data0]->modelViewProjection = Counter;
                        Counter++;
                        VertexTree->shader->uniforms[list->data1]->modelViewProjection = Counter;
                    }
                }
            }
            if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_UNIFORM)
            {
                gctUINT uniformIndex = gcmSL_INDEX_GET(code->source1Index, Index);
                gcSHADER_LIST list;

                if (uniformIndex < VertexTree->shader->uniformCount)
                {
                    gcUNIFORM uniform = VertexTree->shader->uniforms[uniformIndex];
                    if (uniform->u.type == gcSHADER_FLOAT_4X4 &&
                        uniform->modelViewProjection == 0)
                    {
                        Counter++;
                        uniform->modelViewProjection = Counter;
                    }
                    else if (isUniformLoadtimeConstant(uniform) &&
                        uniform->u.type == gcSHADER_FLOAT_X4 &&
                        gcSHADER_FindList(VertexTree->shader, VertexTree->shader->wClipUniformIndexList, (gctINT)uniformIndex, &list))
                    {
                        Counter++;
                        VertexTree->shader->uniforms[list->data0]->modelViewProjection = Counter;
                        Counter++;
                        VertexTree->shader->uniforms[list->data1]->modelViewProjection = Counter;
                    }
                }
            }
        }
    }

    for (list = VertexTree->tempArray[Temp].dependencies;
         list != gcvNULL;
         list = list->next)
    {
#if gcdUSE_WCLIP_PATCH
        if (list->type == gcSL_ATTRIBUTE)
        {
            gcmATTRIBUTE_SetIsPosition(VertexTree->shader->attributes[list->index], gcvTRUE);
        }
        else if (list->type == gcSL_TEMP)
#else
        if (list->type == gcSL_TEMP)
#endif
        {
            if (!TempArray[list->index])
            {
                _TraceModelViewProjection(VertexTree, TempArray, list->index, Counter);
            }
        }
    }
}

static void
_TraceWChannelEqualToZ(
    gcLINKTREE Tree,
    gctINT PositionIndex
    )
{
    gctINT isWChannelAssigned = gcvFALSE;
    gcLINKTREE_TEMP temp = &Tree->tempArray[PositionIndex];
    gcsLINKTREE_LIST_PTR define = gcvNULL;
    gcSL_INSTRUCTION code;
    gcSL_ENABLE enable;

    for (define = temp->defined; define; define = define->next)
    {
        if (define->index < 0) continue;

        code = Tree->shader->code + define->index;
        enable = gcmSL_TARGET_GET(code->temp, Enable);

        if (enable & gcSL_ENABLE_W)
        {
            /* Skip if there are more than one assignment for W channel. */
            if (!isWChannelAssigned)
                isWChannelAssigned = gcvTRUE;
            else
            {
                Tree->WChannelEqualToZ = gcvFALSE;
                return;
            }
        }

        if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MOV &&
            gcmSL_SOURCE_GET(code->source0, Indexed) == gcSL_NOT_INDEXED &&
            gcmSL_SOURCE_GET(code->source0, SwizzleW) == gcSL_SWIZZLE_Z)
        {
            Tree->WChannelEqualToZ = gcvTRUE;
        }
        else
        {
            return;
        }
    }
}

void
gcLINKTREE_FindModelViewProjection(
    gcLINKTREE VertexTree
    )
{
    gctSIZE_T i, bytes;
    gctSIZE_T count = 0;
    gctBOOL_PTR tempArray = gcvNULL;
    gctINT matchCount = 0;
    gctINT positionIndex = -1;

    /* Get the gl_Position index. */
    for (i = 0; i < VertexTree->outputCount; ++i)
    {
        if (VertexTree->shader->outputs[i] /* removed output could be null */ &&
            VertexTree->shader->outputs[i]->nameLength == gcSL_POSITION)
        {
            positionIndex = VertexTree->outputArray[i].tempHolding;
            break;
        }
    }

    if (positionIndex == -1)
        return;

    /*
    ** Check if there is Pos.w = Pos.z. This is a very rough check.
    */
    _TraceWChannelEqualToZ(VertexTree, positionIndex);

    /* Check if there is no mat4 uniform. */
    for (i = 0; i < VertexTree->shader->uniformCount; i++)
    {
        if (VertexTree->shader->uniforms[i]->u.type == gcSHADER_FLOAT_4X4)
        {
            count++;
        }
    }
    if (count == 0)
    {
        return;
    }

    bytes = VertexTree->tempCount * gcmSIZEOF(gctBOOL);

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER *) &tempArray));

    if (tempArray == gcvNULL)
        return;

    gcoOS_ZeroMemory(tempArray, bytes);

    /* Check the MVP dependency. */
    _TraceStrictModelViewProjection(VertexTree, positionIndex, 0, &matchCount);

    if (VertexTree->strictWClipMatch)
    {
        count = matchCount;
    }
    else
    {
        count = 0;
    }

    _TraceModelViewProjection(VertexTree, tempArray, positionIndex, 0);

    gcoOS_Free(gcvNULL, tempArray);
}

gceSTATUS
gcLINKTREE_MarkAllAsUsed(
    IN gcLINKTREE Tree
    )
{
    gctUINT32 i;

    for (i = 0; i < Tree->attributeCount; ++i)
    {
        if (!gcmATTRIBUTE_packedAway(Tree->shader->attributes[i]))
        Tree->attributeArray[i].inUse = gcvTRUE;
    }

    for (i = 0; i < Tree->tempCount; ++i)
    {
        Tree->tempArray[i].inUse = gcvTRUE;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcLINKTREE_MarkAllAsUsedwithRA(
    IN gcLINKTREE Tree
    )
{
    gctUINT32 i;

    for (i = 0; i < Tree->attributeCount; ++i)
    {
        if (Tree->shader->attributes[i] == gcvNULL)
            continue;

        if ((!gcmATTRIBUTE_packedAway(Tree->shader->attributes[i])) &&
            (!gcmATTRIBUTE_isNotUsed(Tree->shader->attributes[i])))
        {
            Tree->attributeArray[i].inUse = gcvTRUE;
        }
    }

    for (i = 0; i < Tree->tempCount; ++i)
    {
        Tree->tempArray[i].inUse = gcvTRUE;
    }

    return gcvSTATUS_OK;
}

static gctBOOL
_TempIsUsedForIndexedOnly(
    IN gcLINKTREE Tree,
    IN gctUINT16 TempIndex
    )
{
    gctBOOL indexedOnly = gcvTRUE;
    gcLINKTREE_TEMP tempReg = &Tree->tempArray[TempIndex];
    gcsLINKTREE_LIST_PTR user = Tree->tempArray[TempIndex].users;
    gcSL_INSTRUCTION code = gcvNULL;
    gcSL_OPCODE opcode;
    gctBOOL checkTarget;
    union
    {
        gctUINT16 hex[2];
        gctFLOAT floatValue;
    } constValue;

    /* Not used as an index. */
    if (!tempReg->isIndex)
    {
        return gcvFALSE;
    }

    /* Only used as an index. */
    if (indexedOnly && !tempReg->usedAsNormalSrc)
    {
        return gcvTRUE;
    }

    /* Check if all none-indexed-used users are also used as an index. */
    while (user)
    {
        if (user->type == gcSL_OUTPUT)
        {
            return gcvFALSE;
        }

        gcmASSERT(user->index >= 0 && user->index < (gctINT)Tree->shader->codeCount);
        code = &Tree->shader->code[user->index];

        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

        /*
        ** We need to make sure that we can convert the other source to INTEGER,
        ** now we only consider CONSTANT.
        */
        if ((gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED &&
             code->source0Indexed == TempIndex)
            ||
            (gcmSL_SOURCE_GET(code->source1, Indexed) != gcSL_NOT_INDEXED &&
             code->source1Indexed == TempIndex))
        {
            user = user->next;
            continue;
        }

        if (opcode == gcSL_JMP || opcode == gcSL_CALL)
        {
            checkTarget = gcvFALSE;
        }
        else
        {
            checkTarget = gcvTRUE;
        }

        if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP &&
            gcmSL_SOURCE_GET(code->source0, Indexed) == gcSL_NOT_INDEXED &&
            gcmSL_INDEX_GET(code->source0Index, Index) == TempIndex)
        {
            if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_CONSTANT)
            {
                constValue.hex[0] = code->source1Index;
                constValue.hex[1] = code->source1Indexed;
                if ((gctFLOAT)((gctINT)constValue.floatValue) != constValue.floatValue)
                {
                    indexedOnly = gcvFALSE;
                }
                else if (checkTarget)
                {
                    indexedOnly = _TempIsUsedForIndexedOnly(Tree, code->tempIndex);
                }
            }
            else
            {
                indexedOnly = gcvFALSE;
            }
        }

        if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP &&
            gcmSL_SOURCE_GET(code->source1, Indexed) == gcSL_NOT_INDEXED &&
            gcmSL_INDEX_GET(code->source1Index, Index) == TempIndex)
        {
            if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_CONSTANT)
            {
                constValue.hex[0] = code->source0Index;
                constValue.hex[1] = code->source0Indexed;
                if ((gctFLOAT)((gctINT)constValue.floatValue) != constValue.floatValue)
                {
                    indexedOnly = gcvFALSE;
                }
                else if (checkTarget)
                {
                    indexedOnly = _TempIsUsedForIndexedOnly(Tree, code->tempIndex);
                }
            }
            else
            {
                indexedOnly = gcvFALSE;
            }
        }

        if (!indexedOnly)
        {
            return gcvFALSE;
        }

        user = user->next;
    }

    return gcvTRUE;
}

static void
_ConvertUsersOfCONV(
    IN OUT gcLINKTREE Tree,
    gctUINT16 TempIndex,
    gcSL_FORMAT Format
    )
{
    gcLINKTREE_TEMP tempReg = &Tree->tempArray[TempIndex];
    gcsLINKTREE_LIST_PTR user = Tree->tempArray[TempIndex].users;
    gcSL_INSTRUCTION code;
    gcSL_OPCODE opcode;
    gctBOOL checkTarget = gcvTRUE;
    union
    {
        gctUINT16 hex[2];
        gctINT32 intValue;
        gctFLOAT floatValue;
    } constValue;

    /* Change the format of this register. */
    tempReg->format = Format;

    while (user)
    {
        gcmASSERT(user->index >= 0 && user->index < (gctINT)Tree->shader->codeCount);
        code = &Tree->shader->code[user->index];

        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

        if ((gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED &&
             code->source0Indexed == TempIndex)
            ||
            (gcmSL_SOURCE_GET(code->source1, Indexed) != gcSL_NOT_INDEXED &&
             code->source1Indexed == TempIndex))
        {
            user = user->next;
            continue;
        }

        if (opcode == gcSL_JMP || opcode == gcSL_CALL)
        {
            checkTarget = gcvFALSE;
        }
        else
        {
            checkTarget = gcvTRUE;
        }

        if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP &&
            gcmSL_SOURCE_GET(code->source0, Indexed) == gcSL_NOT_INDEXED &&
            gcmSL_INDEX_GET(code->source0Index, Index) == TempIndex)
        {
            gcmASSERT(gcmSL_SOURCE_GET(code->source1, Type) == gcSL_CONSTANT);

            /* Change target/source to INTEGER. */
            if (checkTarget)
            {
                code->temp = gcmSL_TARGET_SET(code->temp, Format, Format);
            }
            code->source0 = gcmSL_SOURCE_SET(code->source0, Format, Format);
            code->source1 = gcmSL_SOURCE_SET(code->source1, Format, Format);
            constValue.hex[0] = code->source1Index;
            constValue.hex[1] = code->source1Indexed;
            constValue.intValue = (gctINT)constValue.floatValue;
            code->source1Index = constValue.hex[0];
            code->source1Indexed = constValue.hex[1];

            /* Change the register. */
            if (checkTarget)
            {
                _ConvertUsersOfCONV(Tree, code->tempIndex, Format);
            }
        }

        if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP &&
            gcmSL_SOURCE_GET(code->source1, Indexed) == gcSL_NOT_INDEXED &&
            gcmSL_INDEX_GET(code->source1Index, Index) == TempIndex)
        {
            gcmASSERT(gcmSL_SOURCE_GET(code->source0, Type) == gcSL_CONSTANT);

            /* Change target/source to INTEGER. */
            if (checkTarget)
            {
                code->temp = gcmSL_TARGET_SET(code->temp, Format, Format);
            }
            code->source0 = gcmSL_SOURCE_SET(code->source0, Format, Format);
            code->source1 = gcmSL_SOURCE_SET(code->source1, Format, Format);
            constValue.hex[0] = code->source0Index;
            constValue.hex[1] = code->source0Indexed;
            constValue.intValue = (gctINT)constValue.floatValue;
            code->source0Index = constValue.hex[0];
            code->source0Indexed = constValue.hex[1];

            /* Change the register. */
            if (checkTarget)
            {
                _ConvertUsersOfCONV(Tree, code->tempIndex, Format);
            }
        }

        user = user->next;
    }
}

static gceSTATUS
_ConvertCONV(
    IN OUT gcLINKTREE Tree,
    gctUINT32 CodeIndex,
    gctUINT16 TempIndex,
    gctBOOL ConvertToF2IOnly,
    gctBOOL * ReConstruct,
    gctUINT32 * CodeOffset
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER shader = Tree->shader;
    gcSL_INSTRUCTION code, I2FCode, absCode, floorCode, mulCode;
    gcSL_FORMAT format;
    gcSL_ENABLE enable;
    gcSL_SWIZZLE swizzle;
    gctUINT16 newTempIndex, origTempIndex;
    gctBOOL hasInteger = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SUPPORT_INTEGER);
    gcePATCH_ID patchId = gcvPATCH_INVALID;

    gcoHAL_GetPatchID(gcvNULL, &patchId);

    if (ConvertToF2IOnly && patchId == gcvPATCH_GLBM25)
    {
        format = gcSL_INT16;
    }
    else
    {
        format = gcSL_INTEGER;
    }

    /*
    ** If we only need to convert CONV to F2I,
    ** we need to convert all non-indexed users and temp register's format.
    */
    if (ConvertToF2IOnly)
    {
        /* Change CONV to F2I. */
        code = &shader->code[CodeIndex];
        code->opcode = gcSL_F2I;
        code->temp = gcmSL_TARGET_SET(code->temp, Format, format);
        code->source1 = code->source1Index = code->source1Indexed = 0;

        _ConvertUsersOfCONV(Tree, TempIndex, format);
    }
    else
    {
        code = &shader->code[CodeIndex];
        if (hasInteger)
        {
            /* Change CONV to F2I. */
            code->opcode = gcSL_F2I;
            code->temp = gcmSL_TARGET_SET(code->temp, Format, format);
            code->source1 = code->source1Index = code->source1Indexed = 0;

            /* Insert a I2F. */
            gcmONERROR(gcSHADER_InsertNOP2BeforeCode(shader, CodeIndex + 1, 1));

            /* Fill the new code. */
            code = &shader->code[CodeIndex];
            I2FCode = &shader->code[CodeIndex + 1];

            I2FCode->opcode = gcSL_I2F;
            I2FCode->temp = gcmSL_TARGET_SET(code->temp, Format, gcSL_FLOAT);
            I2FCode->tempIndex = code->tempIndex;
            I2FCode->tempIndexed = code->tempIndexed;

            I2FCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                             | gcmSL_SOURCE_SET(0, Format, gcSL_INTEGER)
                             | gcmSL_SOURCE_SET(0, Precision, gcmSL_TARGET_GET(I2FCode->temp, Precision))
                             | gcmSL_SOURCE_SET(0, Swizzle, _Enable2Swizzle(gcmSL_TARGET_GET(I2FCode->temp, Enable)));
            I2FCode->source0Index = I2FCode->tempIndex;
        }
        else
        {
            format = gcSL_FLOAT;
            enable = (gcSL_ENABLE)gcmSL_TARGET_GET(code->temp, Enable);
            swizzle = (gcSL_SWIZZLE)_Enable2SwizzleWShift(enable);
            newTempIndex = (gctUINT16)gcSHADER_NewTempRegs(shader, 1, gcSHADER_FLOAT_X4);
            origTempIndex = code->tempIndex;

            /* sign t0, source */
            code->opcode = gcSL_SIGN;
            code->temp = gcmSL_TARGET_SET(code->temp, Format, format);
            code->tempIndex = newTempIndex;
            code->source0 = gcmSL_SOURCE_SET(code->source0, Format, format);
            code->source1 = code->source1Index = code->source1Indexed = 0;

            /* Insert three codes. */
            gcmONERROR(gcSHADER_InsertNOP2BeforeCode(shader, CodeIndex + 1, 3));
            code = &shader->code[CodeIndex];
            absCode = &shader->code[CodeIndex + 1];
            floorCode = &shader->code[CodeIndex + 2];
            mulCode = &shader->code[CodeIndex + 3];

            /* abs target, source */
            gcoOS_MemCopy(absCode, code, gcmSIZEOF(struct _gcSL_INSTRUCTION));
            absCode->opcode = gcSL_ABS;
            absCode->tempIndex = origTempIndex;

            /* floor target, target */
            gcoOS_MemCopy(floorCode, absCode, gcmSIZEOF(struct _gcSL_INSTRUCTION));
            floorCode->opcode = gcSL_FLOOR;
            floorCode->source0 = gcmSL_SOURCE_SET(floorCode->source0, Type, gcSL_TEMP);
            floorCode->source0 = gcmSL_SOURCE_SET(floorCode->source0, Indexed, gcSL_NOT_INDEXED);
            floorCode->source0 = gcmSL_SOURCE_SET(floorCode->source0, Swizzle, swizzle);
            floorCode->source0Index = origTempIndex;

            /* mul target, target, t0 */
            gcoOS_MemCopy(mulCode, floorCode, gcmSIZEOF(struct _gcSL_INSTRUCTION));
            mulCode->opcode = gcSL_MUL;
            mulCode->source1 = mulCode->source0;
            mulCode->source1Index = newTempIndex;
        }

        if (ReConstruct)
        {
            *ReConstruct = gcvTRUE;
        }

        if (CodeOffset)
        {
            if (hasInteger)
            {
                *CodeOffset = 1;
            }
            else
            {
                *CodeOffset = 3;
            }
        }
     }

OnError:
    return status;
}

static gctBOOL
_ConvertCONVForOneShader(
    IN OUT gcLINKTREE * Tree,
    IN OUT gctBOOL * Changed,
    IN gctBOOL Dump
    )
{
    gctBOOL             supportMOVAI;
    gcLINKTREE          tree = *Tree;
    gcSHADER            shader = tree->shader;
    gctUINT32           i, origCodeCount = shader->codeCount, newCodeCount;
    gctBOOL             changed = gcvFALSE;
    gctBOOL             reConstruct = gcvFALSE;

    /* Only check for GLES 20. */
    if (shader->clientApiVersion != gcvAPI_OPENGL_ES20)
    {
        return gcvFALSE;
    }

    /*
    ** Right now we only convert:
    ** CONV.RTZ float, float, float ==> F2I int, float.
    */

    supportMOVAI = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SUPPORT_MOVAI);

    for (i = 0; i < origCodeCount; i++)
    {
        gcSL_INSTRUCTION    code;
        gcSL_OPCODE         opcode;
        gcSL_ROUND          rounding;
        gcSL_FORMAT         targetFormat, src0Format;
        gcSL_TYPE           src1Type;
        gctBOOL             convertToF2IOnly = gcvFALSE;
        gctUINT32           codeOffset = 0;
        union
        {
            gctUINT16 hex[2];
            gcSL_FORMAT hex32;
        } src1Format;

        code = &shader->code[i];
        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);
        rounding = (gcSL_ROUND)gcmSL_OPCODE_GET(code->opcode, Round);
        targetFormat = (gcSL_FORMAT)gcmSL_TARGET_GET(code->temp, Format);
        src0Format = (gcSL_FORMAT)gcmSL_SOURCE_GET(code->source0, Format);
        src1Type = (gcSL_TYPE)gcmSL_SOURCE_GET(code->source1, Type);

        if (opcode != gcSL_CONV || rounding != gcSL_ROUND_RTZ ||
            targetFormat != gcSL_FLOAT || src0Format != gcSL_FLOAT ||
            src1Type != gcSL_CONSTANT)
        {
            continue;
        }

        /* Get the convet format. */
        src1Format.hex[0] = code->source1Index;
        src1Format.hex[1] = code->source1Indexed;

        if (src1Format.hex32 != gcSL_FLOAT)
        {
            continue;
        }

        /*
        ** Only if this temp reg is used for indexed only, and this chip can support MOVAI,
        ** we can change this CONV to F2I; otherwise we change this to F2I&I2F.
        */

        if (supportMOVAI && _TempIsUsedForIndexedOnly(tree, code->tempIndex))
        {
            convertToF2IOnly = gcvTRUE;
        }

        if (tree->tempArray[code->tempIndex].lastUse == -1)
        {
            convertToF2IOnly = gcvFALSE;
        }

        _ConvertCONV(tree, i, code->tempIndex, convertToF2IOnly, &reConstruct, &codeOffset);

        i += codeOffset;

        changed = gcvTRUE;
    }

    if (reConstruct)
    {
        gceSHADER_FLAGS flags = tree->flags;

        newCodeCount = tree->shader->codeCount;
        gcmVERIFY_OK(gcLINKTREE_Construct((gcoOS)gcvNULL, &tree));
        gcmVERIFY_OK(gcLINKTREE_Build(tree, (*Tree)->shader, flags));
        if (flags & gcvSHADER_DEAD_CODE)
        {
            gcmVERIFY_OK(gcLINKTREE_RemoveDeadCode(tree));
        }
        else
        {
            /* Mark all temps and attributes as used. */
            gcmVERIFY_OK(gcLINKTREE_MarkAllAsUsedwithRA(tree));
        }
        if (flags & gcvSHADER_OPTIMIZER)
        {
            gcmVERIFY_OK(gcLINKTREE_Optimize(tree));
        }
        (*Tree)->shader->codeCount = origCodeCount;
        gcLINKTREE_Destroy(*Tree);
        *Tree = tree;
        (*Tree)->shader->codeCount = newCodeCount;
    }

    if (changed && Dump)
    {
        _DumpLinkTree("After convert CONV instruction.", tree, gcvFALSE);
    }

    if (Changed && changed)
    {
        *Changed = changed;
    }

    return gcvTRUE;
}

gceSTATUS
gcLINKTREE_ConvertCONV(
    IN OUT gcLINKTREE * Tree[],
    IN gctBOOL DoVaryingPacking,
    IN gctBOOL Link,
    IN gctBOOL Dump
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT32           i;
    gctBOOL             changed = gcvFALSE;
    gcLINKTREE          linkTrees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};

    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i++)
    {
        if (Tree[i] && *Tree[i])
        {
            if (!_ConvertCONVForOneShader(Tree[i], &changed, Dump))
            {
                break;
            }
            linkTrees[i] = *Tree[i];
        }
    }

    if (changed && Link)
    {
        gcLINKTREE_Link(linkTrees, DoVaryingPacking);
    }

    return status;
}

typedef struct _gcsSL_JUMPED_FROM   gcsSL_JUMPED_FROM,
                                    * gcsSL_JUMPED_FROM_PTR;
struct _gcsSL_JUMPED_FROM
{
    gctBOOL blockBegin;
    gctBOOL reached;
};

gceSTATUS
gcSHADER_OptimizeJumps(
    IN gcoOS Os,
    IN gcSHADER Shader
    )
{
    gctUINT32 i;
    gctUINT16 label;
    gceSTATUS status;
    gcsSL_JUMPED_FROM_PTR jumps;
    gctSIZE_T bytes = Shader->codeCount* gcmSIZEOF(gcsSL_JUMPED_FROM);
    gctPOINTER pointer = gcvNULL;

     /* Check for empty shader */
    if (bytes == 0)
    {
        return gcvSTATUS_OK;
    }

    /* Allocate array of gcsSL_JUMPED_FROM structures. */
    status = gcoOS_Allocate(Os, bytes, &pointer);

    if (gcmIS_ERROR(status))
    {
        /* Return on error. */
        return status;
    }

    jumps = pointer;

    /* Initialize array as empty. */
    gcoOS_MemFill(jumps, 0, bytes);

    /* Walk through all the instructions, finding JMP instructions. */
    for (i = 0; i < Shader->codeCount; ++i)
    {
        /* Only look for JMP instructions. */
        if (gcmSL_OPCODE_GET(Shader->code[i].opcode, Opcode) == gcSL_JMP)
        {
            /* Get target of jump instrution. */
            label = Shader->code[i].tempIndex;

            /* If target is next instruction, replace the jump with nop. */
            if (label == i + 1)
            {
                gcmSL_OPCODE_UPDATE(Shader->code[i].opcode, Opcode, gcSL_NOP);
                continue;
            }

            /* Check for a JMP.ALWAYS. */
            if ((i + 1 < Shader->codeCount)
            &&  (gcmSL_TARGET_GET(Shader->code[i].temp, Condition) ==
                    gcSL_ALWAYS)
            )
            {
                /* Mark this as a beginning of a new block. */
                jumps[i + 1].blockBegin = gcvTRUE;
            }

            /* Loop while the target of the JMP is another JMP.ALWAYS. */
            while ((label < Shader->codeCount)
            &&     (gcmSL_OPCODE_GET(Shader->code[label].opcode, Opcode) == gcSL_JMP)
            &&     (gcmSL_TARGET_GET(Shader->code[label].temp, Condition) ==
                       gcSL_ALWAYS)
                   /* Skip backward jump because it will screw up loop scope identification. */
            &&     (Shader->code[label].tempIndex > label)
            )
            {
                /* Load the new target for a serial JMP. */
                label = Shader->code[label].tempIndex;
            }

            /* Store the target of the final JMP. */
            Shader->code[i].tempIndex = label;

            /* Mark the target of the final JMP as reached. */
            if (label < Shader->codeCount)
            {
                jumps[label].reached = gcvTRUE;
            }
        }
    }

    /* Now walk all the gcsSL_JUMPED_FROM structures. */
    for (i = 0; i < Shader->codeCount; ++i)
    {
        /* If this is the beginning of a new block and the code has not been
        ** reached, convert the instruction into a NOP. */
        if (jumps[i].blockBegin && !jumps[i].reached)
        {
            gcmSL_OPCODE_UPDATE(Shader->code[i].opcode, Opcode, gcSL_NOP);
        }
    }

    /* Free the array of gcsSL_JUMPED_FROM structures. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(Os, jumps));

    /* Return success. */
    return gcvSTATUS_OK;
}

static gceSTATUS
_CreateTransformFeedbackStateUniform(
    IN  gcSHADER       VertexShader,
    OUT gcUNIFORM *    UniformPtr
    )
{
    gceSTATUS             status = gcvSTATUS_OK;
    gcUNIFORM             uniform;
    gctCHAR               name[32];
    gctUINT               offset;

    /* construct TransformFeedbackStateUniform name */
    offset = 0;
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(name,
                           gcmSIZEOF(name),
                           &offset,
                           "#__TransformFeedbackState%d", /* private (internal)
                                                           * uniform starts with '#' */
                           VertexShader->_id));

    gcmONERROR(gcSHADER_AddUniform(VertexShader, name,
                                   gcSHADER_INTEGER_X1,
                                   1 /* Length */,
                                   gcSHADER_PRECISION_HIGH,
                                   &uniform));
    SetUniformFlag(uniform, gcvUNIFORM_KIND_TRANSFORM_FEEDBACK_STATE);
    SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
    *UniformPtr = uniform;
OnError:
    return status;
}

static gctINT
_getFeedbackOutputSize(
    IN gcSHADER_TYPE      type
    )
{
    switch (type) {
    case gcSHADER_FLOAT_X1:
        return 4;
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_UINT_X1:
        return 4;  /* size of int */
    default:
        gcmASSERT(gcvFALSE);
        break;
    }
    return 0;
}

gctSIZE_T
gcSHADER_GetVarTempRegInfo(
    IN gcSHADER              Shader,
    IN gcOUTPUT              Varying,
    IN gctBOOL               IsArray,
    OUT gcsVarTempRegInfo *  VarTempRegInfo,
    OUT gctSIZE_T *          Size
    )
{
    gceSTATUS          status = gcvSTATUS_OK;
    gctINT             sz = 0;
    gctSIZE_T          totalSize = 0;
    gctINT             components;
    gctINT             rows;
    gcSHADER_TYPE      componentType;
    gctINT             componentOutputSize;  /* the feedback output size is
                                                different than component size */
    gcSHADER_TYPE *    tempTypeArray;
    gctUINT            bytes;
    gcSHADER_TYPE      tempType    = Varying->type;

    /* only one element for each output */
    bytes = sizeof(gcSHADER_TYPE);
    /* allocate the temp register type array */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              bytes,
                              (gctPOINTER *) &tempTypeArray));

    rows        = gcmType_Rows(tempType);
    components  = gcmType_Comonents(tempType);
    gcmASSERT(tempType != gcSHADER_UNKONWN_TYPE &&
                 components > 0 && components <= 4);
    componentType = gcmType_ComonentType(tempType);
    componentOutputSize = _getFeedbackOutputSize(componentType);
    sz = rows * components * componentOutputSize;
    if (IsArray)
    {
        sz *= Varying->arraySize;
    }
    gcmASSERT(sz > 0);

    VarTempRegInfo->varying         = Varying;
    VarTempRegInfo->tempRegTypes    = tempTypeArray;
    VarTempRegInfo->tempRegCount    = rows;
    VarTempRegInfo->isArray         = IsArray;
    VarTempRegInfo->streamoutSize   = 0;

    VarTempRegInfo->streamoutSize += sz;
    totalSize += sz;

    *Size = totalSize;

OnError:
    return status;
}


gceSTATUS
gcSHADER_ComputeTotalFeedbackVaryingsSize(
    IN gcSHADER Shader
    )
{
    gceSTATUS              status       = gcvSTATUS_OK;
    gctSIZE_T              totalSize    = 0;
    gcsTRANSFORM_FEEDBACK *xfbk         = &Shader->transformFeedback;
    gctINT                 i;
    const gctINT           varyingCount = xfbk->varyingCount;
    gcsVarTempRegInfo *    varRegInfos  = gcvNULL;

    if(varyingCount == 0 || xfbk->varRegInfos != gcvNULL)
    {
        return gcvSTATUS_OK;
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              varyingCount * sizeof(gcsVarTempRegInfo),
                              (gctPOINTER *) &varRegInfos));
    gcoOS_ZeroMemory(varRegInfos, varyingCount * sizeof(gcsVarTempRegInfo));
    xfbk->varRegInfos = varRegInfos;
    for (i = 0; i < varyingCount; i++)
    {
        gctSIZE_T sz = 0;
        gcSHADER_GetVarTempRegInfo(Shader,
                                   xfbk->varyings[i].output,
                                   xfbk->varyings[i].isArray,
                                   &varRegInfos[i],
                                   &sz);
        totalSize += sz;
    }
    xfbk->totalSize = totalSize;

OnError:
    return status;
}

static gceSTATUS
_CreatePatchUniform(
    IN  gcSHADER         Shader,
    IN  gctCONST_STRING  BaseName,
    IN  gctINT           Index,
    IN  gcSHADER_TYPE    type,
    IN  gceUNIFORM_FLAGS uFlag,
    IN  gctBOOL          AppendId2Name,
    IN  gcSHADER_PRECISION Precision,
    OUT gcUNIFORM *      UniformPtr
    )
{
    gceSTATUS             status = gcvSTATUS_OK;
    gcUNIFORM             uniform = gcvNULL;
    gctCHAR               name[512];
    gctUINT               offset, i;
    gctUINT32             uniformNameLength;
    gctCONST_STRING       uniformName;

    /* construct name */
    offset = 0;

    if (AppendId2Name)
    {
        if (Index != -1)
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(name,
                                   gcmSIZEOF(name),
                                   &offset,
                                   "#%s%d_%d", /* private (internal)
                                                  * uniform starts with '#' */
                                   BaseName,
                                   Shader->_id,
                                   Index));
        }
        else
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(name,
                                   gcmSIZEOF(name),
                                   &offset,
                                   "#%s%d", /* private (internal)
                                               * uniform starts with '#' */
                                   BaseName,
                                   Shader->_id));
        }
    }
    else
    {
        gcmVERIFY_OK(
                gcoOS_PrintStrSafe(name,
                                   gcmSIZEOF(name),
                                   &offset,
                                   "#%s", /* private (internal)
                                             * uniform starts with '#' */
                                   BaseName));
    }

    for (i = 0; i < Shader->uniformCount; i ++)
    {
        if(!Shader->uniforms[i]) continue;

        gcmVERIFY_OK(gcUNIFORM_GetName(Shader->uniforms[i],
                                       &uniformNameLength, &uniformName));

        if (uniformNameLength == (gctUINT32) gcoOS_StrLen(name, gcvNULL))
        {
            if (gcoOS_MemCmp(name, uniformName, uniformNameLength) == 0)
            {
                uniform = Shader->uniforms[i];

                break;
            }
        }
    }

    if (uniform == gcvNULL)
    {
        gcmONERROR(gcSHADER_AddUniform(Shader, name,
                                       type,
                                       1 /* Length */,
                                       Precision,
                                       &uniform));
        SetUniformFlag(uniform, uFlag);
    }
    else
    {
        gcmASSERT(UniformHasFlag(uniform, uFlag));
        gcmASSERT(uniform->u.type == type);
    }

    *UniformPtr = uniform;

OnError:
    return status;
}

gcSL_FORMAT
gcGetFormatFromType(
    IN gcSHADER_TYPE Type
    )
{
    gcSL_FORMAT format = gcSL_FLOAT;

    switch (Type)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_FLOAT_4X4:
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X4:
        format = gcSL_FLOAT;
        break;

    case gcSHADER_INTEGER_X1:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
        format = gcSL_INTEGER;
        break;

    case gcSHADER_UINT_X1:
    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
        format = gcSL_UINT32;
        break;

    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_BOOLEAN_X4:
        format = gcSL_BOOLEAN;
        break;

    case gcSHADER_INT64_X1:
    case gcSHADER_INT64_X2:
    case gcSHADER_INT64_X3:
    case gcSHADER_INT64_X4:
        format = gcSL_INT64;
        break;

    case gcSHADER_UINT64_X1:
    case gcSHADER_UINT64_X2:
    case gcSHADER_UINT64_X3:
    case gcSHADER_UINT64_X4:
        format = gcSL_UINT64;
        break;

    case gcSHADER_SAMPLER_1D:
    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_BUFFER:
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_CUBE_SHADOW:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_EXTERNAL_OES:
    case gcSHADER_SAMPLER_1D_ARRAY:
    case gcSHADER_SAMPLER_1D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:
        format = gcSL_FLOAT;
        break;

    case gcSHADER_ISAMPLER_2D:
    case gcSHADER_ISAMPLER_3D:
    case gcSHADER_ISAMPLER_BUFFER:
    case gcSHADER_ISAMPLER_CUBIC:
    case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_ISAMPLER_2D_ARRAY:
        format = gcSL_INTEGER;
        break;

    case gcSHADER_USAMPLER_2D:
    case gcSHADER_USAMPLER_3D:
    case gcSHADER_USAMPLER_BUFFER:
    case gcSHADER_USAMPLER_CUBIC:
    case gcSHADER_USAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_USAMPLER_2D_ARRAY:
        format = gcSL_UINT32;
        break;

    case gcSHADER_SAMPLER_2D_MS:
    case gcSHADER_SAMPLER_2D_MS_ARRAY:
        format = gcSL_FLOAT;
        break;

    case gcSHADER_ISAMPLER_2D_MS:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:
        format = gcSL_INTEGER;
        break;

    case gcSHADER_USAMPLER_2D_MS:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:
        format = gcSL_UINT32;
        break;

    case gcSHADER_IIMAGE_2D:                     /* 0x39 */
    case gcSHADER_IIMAGE_3D:                     /* 0x3B */
    case gcSHADER_IIMAGE_CUBE:                   /* 0x3E */
    case gcSHADER_IIMAGE_2D_ARRAY:               /* 0x41 */
    case gcSHADER_IIMAGE_CUBEMAP_ARRAY:          /* 0x52 */
    case gcSHADER_IIMAGE_BUFFER:                 /* 0x5A */
        format = gcSL_INTEGER;
        break;


    case gcSHADER_UIMAGE_2D:                     /* 0x3A */
    case gcSHADER_UIMAGE_3D:                     /* 0x3C */
    case gcSHADER_UIMAGE_CUBE:                   /* 0x3F */
    case gcSHADER_UIMAGE_2D_ARRAY:               /* 0x42 */
    case gcSHADER_UIMAGE_CUBEMAP_ARRAY:          /* 0x53 */
    case gcSHADER_UIMAGE_BUFFER:                 /* 0x5B */
        format = gcSL_UINT32;
        break;

    case gcSHADER_IMAGE_2D:                      /* 0x17 */
    case gcSHADER_IMAGE_3D:                      /* 0x18 */
    case gcSHADER_IMAGE_CUBE:                    /* 0x3D */
    case gcSHADER_IMAGE_CUBEMAP_ARRAY:           /* 0x50 */
    case gcSHADER_IMAGE_2D_ARRAY:                /* 0x40 */
    case gcSHADER_IMAGE_1D:                      /* 0x44 */
    case gcSHADER_IMAGE_1D_ARRAY:                /* 0x45 */
    case gcSHADER_IMAGE_1D_BUFFER:               /* 0x46 */
    case gcSHADER_IMAGE_BUFFER:                  /* 0x59 */
        format = gcSL_FLOAT;
        break;

    case gcSHADER_SAMPLER:
        format = gcSL_UINT32;
        break;

    case gcSHADER_ATOMIC_UINT:
        format = gcSL_UINT32;
        break;

    /* float16 */
    case gcSHADER_FLOAT16_X1:      /* half2 */
    case gcSHADER_FLOAT16_X2:      /* half2 */
    case gcSHADER_FLOAT16_X3:      /* half3 */
    case gcSHADER_FLOAT16_X4:      /* half4 */
    case gcSHADER_FLOAT16_X8:      /* half8 */
    case gcSHADER_FLOAT16_X16:     /* half16 */
    case gcSHADER_FLOAT16_X32:     /* half32 */
        format = gcSL_FLOAT16;
        break;

    /*  boolean  */
    case gcSHADER_BOOLEAN_X8:
    case gcSHADER_BOOLEAN_X16:
    case gcSHADER_BOOLEAN_X32:
        format = gcSL_BOOLEAN;
        break;

    /* uchar vectors  */
    case gcSHADER_UINT8_X1:
    case gcSHADER_UINT8_X2:
    case gcSHADER_UINT8_X3:
    case gcSHADER_UINT8_X4:
    case gcSHADER_UINT8_X8:
    case gcSHADER_UINT8_X16:
    case gcSHADER_UINT8_X32:
        format = gcSL_UINT8;
        break;

    /* char vectors  */
    case gcSHADER_INT8_X1:
    case gcSHADER_INT8_X2:
    case gcSHADER_INT8_X3:
    case gcSHADER_INT8_X4:
    case gcSHADER_INT8_X8:
    case gcSHADER_INT8_X16:
    case gcSHADER_INT8_X32:
        format = gcSL_INT8;
        break;

    /* ushort vectors */
    case gcSHADER_UINT16_X1:
    case gcSHADER_UINT16_X2:
    case gcSHADER_UINT16_X3:
    case gcSHADER_UINT16_X4:
    case gcSHADER_UINT16_X8:
    case gcSHADER_UINT16_X16:
    case gcSHADER_UINT16_X32:
        format = gcSL_UINT16;
        break;

    /* short vectors */
    case gcSHADER_INT16_X1:
    case gcSHADER_INT16_X2:
    case gcSHADER_INT16_X3:
    case gcSHADER_INT16_X4:
    case gcSHADER_INT16_X8:
    case gcSHADER_INT16_X16:
    case gcSHADER_INT16_X32:
        format = gcSL_INT16;
        break;

    /* packed data type */
    /* packed float16 (2 bytes per element) */
    case gcSHADER_FLOAT16_P2:      /* half2 */
    case gcSHADER_FLOAT16_P3:      /* half3 */
    case gcSHADER_FLOAT16_P4:      /* half4 */
    case gcSHADER_FLOAT16_P8:      /* half8 */
    case gcSHADER_FLOAT16_P16:     /* half16 */
    case gcSHADER_FLOAT16_P32:     /* half32 */
        format = gcSL_FLOAT16;
        break;

    /* packed boolean (1 byte per element) */
    case gcSHADER_BOOLEAN_P2:    /* bool2, bvec2 */
    case gcSHADER_BOOLEAN_P3:
    case gcSHADER_BOOLEAN_P4:
    case gcSHADER_BOOLEAN_P8:
    case gcSHADER_BOOLEAN_P16:
    case gcSHADER_BOOLEAN_P32:
        format = gcSL_BOOLEAN;
        break;

    /* uchar vectors (1 byte per element) */
    case gcSHADER_UINT8_P2:
    case gcSHADER_UINT8_P3:
    case gcSHADER_UINT8_P4:
    case gcSHADER_UINT8_P8:
    case gcSHADER_UINT8_P16:
    case gcSHADER_UINT8_P32:
        format = gcSL_UINT8;
        break;

    /* char vectors (1 byte per element) */
    case gcSHADER_INT8_P2:
    case gcSHADER_INT8_P3:
    case gcSHADER_INT8_P4:
    case gcSHADER_INT8_P8:
    case gcSHADER_INT8_P16:
    case gcSHADER_INT8_P32:
        format = gcSL_INT8;
        break;

    /* ushort vectors (2 bytes per element) */
    case gcSHADER_UINT16_P2:
    case gcSHADER_UINT16_P3:
    case gcSHADER_UINT16_P4:
    case gcSHADER_UINT16_P8:
    case gcSHADER_UINT16_P16:
    case gcSHADER_UINT16_P32:
        format = gcSL_UINT16;
        break;

    /* short vectors (2 bytes per element) */
    case gcSHADER_INT16_P2:
    case gcSHADER_INT16_P3:
    case gcSHADER_INT16_P4:
    case gcSHADER_INT16_P8:
    case gcSHADER_INT16_P16:
    case gcSHADER_INT16_P32:
        format = gcSL_INT16;
        break;

    default:
        gcmASSERT(0);
    }
    return format;
}

/* generate transform feedback write to base address at Offset for the Varying,
   if BufferBaseAddress is not NULL, then the base address is in the uniform,
   otherwise the base address is in temp register BufferBaseAddressTempIndex,
   return wrote bytes in Size */
gceSTATUS
_generateFeedbackWrite(
    IN gcSHADER            VertexShader,
    IN gcsVarTempRegInfo * VaryingRegInfo,
    IN gcUNIFORM           BufferBaseAddress,
    IN gctINT              BufferBaseAddressTempIndex,
    IN gctINT              Offset,
    OUT gctSIZE_T *        Size)
{
    gceSTATUS     status = gcvSTATUS_OK;
    gctINT        currentOffset = Offset;
    gctUINT       startIndex;
    gctINT        i, j;
    gcSHADER      shader = VertexShader;
    gcOUTPUT      varying = gcvNULL;
    static const gcSL_ENABLE component2Enable[] = {
             gcSL_ENABLE_X,
             gcSL_ENABLE_XY,
             gcSL_ENABLE_XYZ,
             gcSL_ENABLE_XYZW
    };

    gcmASSERT(Offset >= 0 && VaryingRegInfo && VaryingRegInfo->varying != gcvNULL);

    /* Get the first element of an array. */
    gcmONERROR(gcSHADER_GetOutputByTempIndex(VertexShader,
                                             VaryingRegInfo->varying->tempIndex,
                                             &varying));

    for (j = 0; j < varying->arraySize; j++)
    {
        gcmONERROR(gcSHADER_GetOutputByTempIndex(VertexShader,
                                                 VaryingRegInfo->varying->tempIndex + (gctUINT16)(j * VaryingRegInfo->tempRegCount),
                                                 &varying));

        for (i = 0; i < VaryingRegInfo->tempRegCount; i ++)
        {
            gcSHADER_TYPE tempType    = varying->type;
            gctINT        components  = gcmType_Comonents(tempType);
            gcSL_FORMAT   format      = gcGetFormatFromType(tempType);
            gcSL_ENABLE   enable;

            startIndex = varying->tempIndex;

            gcmASSERT(tempType != gcSHADER_UNKONWN_TYPE &&
                      components > 0 && components <= 4);
            enable = component2Enable[components - 1];
            /* write one temp register at a time

                 src0.x <= BufferBaseAddress
                 src1.x <= currentOffset
                 dest   <= temp[i]
                 store temp[i], src1.x, src2

                 currentOffset += byteSize(tempType);
             */
            gcSHADER_AddOpcode(shader, gcSL_STORE,
                               (gctINT16)(startIndex+i),
                               enable, format, gcSHADER_PRECISION_DEFAULT);
            if (BufferBaseAddress != gcvNULL)
            {
                gcSHADER_AddSourceUniformFormatted(shader, BufferBaseAddress,
                                                   gcSL_SWIZZLE_XXXX,
                                                   gcSL_NOT_INDEXED,
                                                   gcSL_INTEGER);
            }
            else
            {
                gcSHADER_AddSource(shader, gcSL_TEMP,
                                   (gctUINT16)BufferBaseAddressTempIndex,
                                   gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
            }
            gcSHADER_AddSourceConstantFormatted(shader, (void *)&currentOffset,
                                                gcSL_INTEGER);

            /* adjust offset */
            currentOffset += components * gcmType_ComponentByteSize;
        }

        if (!VaryingRegInfo->isArray)
            break;
    }

    *Size = currentOffset - Offset;

OnError:
    return status;
}

static gctINT
_getUnusedTempIndex(
    IN gcSHADER Shader,
    IN gctINT NumTemp
    )
{
    return gcSHADER_NewTempRegs(Shader, NumTemp, gcSHADER_FLOAT_X1);
}

/* find the temp register assigned to gl_vertexID variable */
static gcVARIABLE
_findVexterInstIDTemp(
    IN gcSHADER Shader,
    IN gceBuiltinNameKind vtxOrInstIdName
    )
{
    gcVARIABLE    vertexInstIDVariable  = gcvNULL;
    gctUINT       i;

    gcmASSERT(Shader->type == gcSHADER_TYPE_VERTEX || Shader->type == gcSHADER_TYPE_TES);
    for (i = 0; i < Shader->variableCount; i++)
    {
        gcVARIABLE variable = Shader->variables[i];

        /* attributes could be null if it is not used */
        if (variable == gcvNULL)
            continue;
        if (variable->nameLength == (gctINT)vtxOrInstIdName)
        {
            vertexInstIDVariable = variable;
        }
    }

    if (vertexInstIDVariable == gcvNULL)
    {
        gctINT vertexInstIDTemp = _getUnusedTempIndex(Shader, 1);
        gctINT16 varIndex = -1;
        gctINT size = 1;
        gcSHADER_TYPE  vertexInstIDType =
            gcoHAL_IsFeatureAvailable1(gcvNULL,
                                       gcvFEATURE_VERTEX_INST_ID_AS_INTEGER) == gcvSTATUS_TRUE ?
                                       gcSHADER_INTEGER_X1 : gcSHADER_FLOAT_X1;

        gcSHADER_AddVariableEx(Shader,
                               (vtxOrInstIdName == gcSL_VERTEX_ID) ? "#VertexID" : "#InstanceID",
                               vertexInstIDType,
                               1,
                               &size,
                               (gctUINT16)vertexInstIDTemp,
                               gcSHADER_VAR_CATEGORY_NORMAL,
                               gcSHADER_PRECISION_HIGH,
                               0,
                               -1, /* parent */
                               -1, /* prevSibling */
                               &varIndex);
        gcmASSERT(varIndex >= 0);
        vertexInstIDVariable = Shader->variables[varIndex];
        SetVariableIsCompilerGenerated(vertexInstIDVariable);
    }

    return vertexInstIDVariable;
}


static gctINT
_getVertexIDTemp(
    IN gcSHADER   VertexShader,
    IN gctBOOL    needVertexIDReversePatch
    )
{
    gcVARIABLE     vertexIDVariable = _findVexterInstIDTemp(VertexShader, gcSL_VERTEX_ID);
    gcVARIABLE     instIDVariable = _findVexterInstIDTemp(VertexShader, gcSL_INSTANCE_ID);
    gcUNIFORM      uStartVertex, uTotalVtxCountPerInstance;
    gctINT         vertexIDTemp1, vertexIDTemp2, vertexIDTemp3;
    gcSL_FORMAT    format;

    gcmASSERT(vertexIDVariable->precision == gcSHADER_PRECISION_HIGH &&
              instIDVariable->precision == gcSHADER_PRECISION_HIGH);
    vertexIDTemp1 = _getUnusedTempIndex(VertexShader, 1);

    if (needVertexIDReversePatch)
    {
        _CreatePatchUniform(VertexShader,
                            "sh_startVertex",
                            0,
                            gcSHADER_INTEGER_X1,
                            gcvUNIFORM_KIND_GENERAL_PATCH,
                            gcvFALSE,
                            gcSHADER_PRECISION_HIGH,
                            &uStartVertex);

        format = gcSL_INTEGER;
        SetUniformFlag(uStartVertex, gcvUNIFORM_FLAG_COMPILER_GEN);

        /* vertexIndex = gl_vertexID - startVertex */
        gcSHADER_AddOpcode(VertexShader, gcSL_SUB, (gctUINT16)vertexIDTemp1,
                           gcSL_ENABLE_X, format, gcSHADER_PRECISION_HIGH);
        gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDVariable->tempIndex,
                           gcSL_SWIZZLE_XXXX, format, gcSHADER_PRECISION_HIGH);
        gcSHADER_AddSourceUniformFormatted(VertexShader,
                                           uStartVertex,
                                           gcSL_SWIZZLE_XXXX,
                                           gcSL_NOT_INDEXED,
                                           format);
    }
    else
    {
        vertexIDTemp1 = vertexIDVariable->tempIndex;
    }

    /*
    ** If FE don't reset vertexID when a new instance start,
    ** then we don't need to evaluate the vertexID.
    */
    if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_FE_START_VERTEX_SUPPORT))
    {
        return vertexIDTemp1;
    }

    vertexIDTemp2 = _getUnusedTempIndex(VertexShader, 1);
    vertexIDTemp3 = _getUnusedTempIndex(VertexShader, 1);

    _CreatePatchUniform(VertexShader,
                        "sh_totalVtxCountPerInstance",
                        0,
                        gcSHADER_INTEGER_X1,
                        gcvUNIFORM_KIND_GENERAL_PATCH,
                        gcvFALSE,
                        gcSHADER_PRECISION_HIGH,
                        &uTotalVtxCountPerInstance);
    format = gcSL_INTEGER;
    SetUniformFlag(uTotalVtxCountPerInstance, gcvUNIFORM_FLAG_COMPILER_GEN);

    /* vertexIndex = vertexIndex + totalVertexCount * gl_instanceID */
    gcSHADER_AddOpcode(VertexShader, gcSL_MUL, (gctUINT16)vertexIDTemp2,
                       gcSL_ENABLE_X, format, gcSHADER_PRECISION_HIGH);
    gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)instIDVariable->tempIndex,
                       gcSL_SWIZZLE_XXXX, format, gcSHADER_PRECISION_HIGH);
    gcSHADER_AddSourceUniformFormatted(VertexShader,
                                       uTotalVtxCountPerInstance,
                                       gcSL_SWIZZLE_XXXX,
                                       gcSL_NOT_INDEXED,
                                       format);

    gcSHADER_AddOpcode(VertexShader, gcSL_ADD, (gctUINT16)vertexIDTemp3,
                       gcSL_ENABLE_X, format, gcSHADER_PRECISION_HIGH);
    gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDTemp1,
                       gcSL_SWIZZLE_XXXX, format, gcSHADER_PRECISION_HIGH);
    gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDTemp2,
                       gcSL_SWIZZLE_XXXX, format, gcSHADER_PRECISION_HIGH);

    /* no conversion is needed if the type is integer */
    return vertexIDTemp3;

}

gceSTATUS
gcAddTransformFeedback(
    IN gcSHADER   VertexShader,
    IN gctBOOL    needVertexIDReversePatch
    )
{
    gceSTATUS             status = gcvSTATUS_OK;
    gcsTRANSFORM_FEEDBACK *xfbk  = &VertexShader->transformFeedback;
    gcUNIFORM             xfbkState;
    gctUINT               label;
    gctINT                i;
    gctINT32              offset = 0;
    gctSIZE_T             sz;
    const gctINT          zeroConstant = 0;
    gctINT                temp1, temp2;
    gctINT                vertexIDTemp;
    gcUNIFORM *           separateBuffers = gcvNULL;
    gcUNIFORM             buffer = gcvNULL;

    /* create TransformFeedbackStateUniform */
    gcmONERROR(_CreateTransformFeedbackStateUniform(VertexShader,
                                                    &xfbkState));
    /* add conditonal jump code:
          if (xfbkState.x == 0) goto label;
     */
    label  = gcSHADER_FindNextUsedLabelId(VertexShader);
    gcmONERROR(gcSHADER_AddOpcodeConditionalFormatted(VertexShader,
                                                    gcSL_JMP,
                                                    gcSL_EQUAL,
                                                    gcSL_INTEGER,
                                                    label));
    gcmONERROR(gcSHADER_AddSourceUniformFormatted(VertexShader,
                                                  xfbkState,
                                                  gcSL_SWIZZLE_XXXX,
                                                  0,
                                                  gcSL_INTEGER));
    gcmONERROR(gcSHADER_AddSourceConstantFormatted(VertexShader,
                                                   (void *)&zeroConstant,
                                                   gcSL_INTEGER));

    /* compute the total size of all feedback varyings */
    gcSHADER_ComputeTotalFeedbackVaryingsSize(VertexShader);

    vertexIDTemp = _getVertexIDTemp(VertexShader, needVertexIDReversePatch);

    if (xfbk->bufferMode == gcvFEEDBACK_INTERLEAVED)
    {
        gcUNIFORM     xfbkInterLeavedBuffer;
        gctINT        totalSize = xfbk->totalSize;
        /*   Interleaved Mode
         *
         * --- +--------------------+ <--- xfbkInterLeavedBuffer
         *  ^  |   varying 0        |
         *  |  |                    |
         *  |  +--------------------+      vertexID=0
         *  |  |   varying 1        |
         *  |  |                    |      varyingOffset[0] = 0;
         *  |  +--------------------+      varyingOffset[i] = varyingOffset[i-1]
         *  |  |   ...              |                         + varyingSize[i-1];
         *  |  +--------------------+
         *  |  |                    |      TotalBlockSz = Sum(varyingSize[0:n-1]);
         *  |  |   varying n-1      |
         *  v  |                    |
         * --- +--------------------+ <---  vertexID=1
         *     |                    |
         *     |   ...              |
         *     |                    |
         *     |                    |
         *     +--------------------+ <--- curBase = xfbkInterLeavedBuffer +
         *     |                    |                vertexID * TotalBlockSz;
         *     |                    |
         *     |                    |
         *     |                    |
         *     |                    |
         *     |                    |
         *     |                    |
         *     +--------------------+
         */

        /* create tranformFeedbackBuffer uniform */
        gcmONERROR(_CreatePatchUniform(VertexShader,
                              "InterleavedTransformFeedbackBuffer",
                              -1,
                              gcSHADER_INTEGER_X1,
                              gcvUNIFORM_KIND_TRANSFORM_FEEDBACK_BUFFER,
                              gcvTRUE,
                              gcSHADER_PRECISION_HIGH,
                              &xfbkInterLeavedBuffer));
        SetUniformFlag(xfbkInterLeavedBuffer, gcvUNIFORM_FLAG_COMPILER_GEN);
        xfbk->feedbackBuffer.interleavedBufUniform = xfbkInterLeavedBuffer;

        /* interleaved mode, add following code to VertexShader:

               if (xfbkState.x != 0)
               {
                   gctINT32 baseAddr = (char *)xfbkInterLeavedBuffer;
                   gctINT32 offset   = gl_VertexID * xfbk->totalSize;

                   for (int i = 0; i < xfbk->varyingCount; i++)
                   {
                       size_t outputSize = _getVaryingOutputSize(xfbk->varying[i]);
                       memcopy(baseAddr, &xfbk>varying[i], outputSize);
                       baseAddr += outputSize;
                   }
               }
         */

        /* base address = xfbkInterLeavedBuffer + vertexID * TotalBlockSz; */
        temp1 = _getUnusedTempIndex(VertexShader, 1);
        temp2 = _getUnusedTempIndex(VertexShader, 1);

        /* We don't have MAD operation in current IR, we need to do MUL and ADD */

        /* create MUL temp1.x, totalSize, temp[VertxID].x */
        gcSHADER_AddOpcode(VertexShader, gcSL_MUL, (gctUINT16)temp1,
                          gcSL_ENABLE_X, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
        gcSHADER_AddSourceConstantFormatted(VertexShader, (void *)&totalSize,
                                            gcSL_INTEGER);
        gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDTemp,
                           gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);

        /* create ADD temp2.x, temp1.x, InterleavedBufferAddr */
        gcSHADER_AddOpcode(VertexShader, gcSL_ADD, (gctUINT16)temp2,
                           gcSL_ENABLE_X, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
        gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)temp1,
                           gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
        gcSHADER_AddSourceUniformFormatted(VertexShader,
                                           xfbkInterLeavedBuffer,
                                           gcSL_SWIZZLE_XXXX,
                                           gcSL_NOT_INDEXED,
                                           gcSL_INTEGER);

        for (i=0; i < (gctINT)xfbk->varyingCount; i++)
        {
            gcmONERROR(_generateFeedbackWrite(VertexShader,
                                              &xfbk->varRegInfos[i],
                                              gcvNULL,
                                              temp2, /* base addr temp index */
                                              offset,
                                              &sz));
            offset += sz;
        }
    }
    else
    {
        /*  Separate Mode
         *
         *     +--------------+ <--- SeparateBufferAddr[0]
         *     |   varying 0  |
         *     |              |       vertexID=0
         *     |              |
         *     +--------------+ <---  vertexID=1
         *     |              |
         *     |   ...        |
         *     |              |
         *     |              |
         *     +--------------+ <--- curBase = SeparateBufferAddr[0] +
         *     |              |                vertexID * varyingSize[0];
         *     |              |
         *     |              |
         *     +--------------+
         *
         *     +--------------+ <--- SeparateBufferAddr[1]
         *     |   varying 1  |
         *     |              |       vertexID=0
         *     |              |
         *     +--------------+ <---  vertexID=1
         *     |              |
         *     |   ...        |
         *     |              |
         *     |              |
         *     +--------------+ <--- curBase = SeparateBufferAddr[1] +
         *     |              |                vertexID * varyingSize[1];
         *     |              |
         *     |              |
         *     +--------------+
         *
         */

        gcmASSERT(xfbk->bufferMode == gcvFEEDBACK_SEPARATE);
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                 xfbk->varyingCount * sizeof(gcUNIFORM),
                                 (gctPOINTER *) &separateBuffers));
        for (i=0; i < (gctINT)xfbk->varyingCount; i++)
        {
            /* create tranformFeedbackBuffer uniform */
            gcmONERROR(_CreatePatchUniform(VertexShader,
                                  "SeparateTransformFeedbackBuffer",
                                  i,
                                  gcSHADER_INTEGER_X1,
                                  gcvUNIFORM_KIND_TRANSFORM_FEEDBACK_BUFFER,
                                  gcvTRUE,
                                  gcSHADER_PRECISION_HIGH,
                                  &buffer));
            SetUniformFlag(buffer, gcvUNIFORM_FLAG_COMPILER_GEN);
            separateBuffers[i] = buffer;

            /* base address = separateBuffers[i] + vertexID * varyingSize[i]; */
            temp1 = _getUnusedTempIndex(VertexShader, 1);
            temp2 = _getUnusedTempIndex(VertexShader, 1);

            /* We don't have MAD operation in current IR, we need to do MUL and ADD */

            /* create MUL temp1.x, totalSize, temp[VertxID].x */
            gcSHADER_AddOpcode(VertexShader, gcSL_MUL, (gctUINT16)temp1,
                              gcSL_ENABLE_X, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
            gcSHADER_AddSourceConstantFormatted(VertexShader,
                                               (void *)&xfbk->varRegInfos[i].streamoutSize,
                                                gcSL_INTEGER);
            gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)vertexIDTemp,
                               gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);

            /* create ADD temp2.x, temp1.x, InterleavedBufferAddr */
            gcSHADER_AddOpcode(VertexShader, gcSL_ADD, (gctUINT16)temp2,
                               gcSL_ENABLE_X, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
            gcSHADER_AddSource(VertexShader, gcSL_TEMP, (gctUINT16)temp1,
                               gcSL_SWIZZLE_XXXX, gcSL_INTEGER, gcSHADER_PRECISION_HIGH);
            gcSHADER_AddSourceUniformFormatted(VertexShader,
                                               separateBuffers[i],
                                               gcSL_SWIZZLE_XXXX,
                                               gcSL_NOT_INDEXED,
                                               gcSL_INTEGER);


            gcmONERROR(_generateFeedbackWrite(VertexShader,
                                              &xfbk->varRegInfos[i],
                                              gcvNULL,
                                              temp2,
                                              0,
                                              &sz));
        }
        xfbk->feedbackBuffer.separateBufUniforms = separateBuffers;
    }

    /* add label */
    gcSHADER_AddLabel(VertexShader, label);
    /* add nop for the label */
    gcSHADER_AddOpcode(VertexShader, gcSL_NOP, 0, 0, 0, 0);
    gcSHADER_AddSource(VertexShader, gcSL_NONE, 0, 0, 0, 0);
    gcSHADER_AddSource(VertexShader, gcSL_NONE, 0, 0, 0, 0);

    gcSHADER_Pack(VertexShader);
    return status;

OnError:
    if (separateBuffers)
        gcoOS_Free(gcvNULL,separateBuffers);

    return status;
}

gceSTATUS
gcAddVertexAndInstanceIdPatch(
    IN gcSHADER             VertexShader,
    IN gctBOOL              XFBPatch,
    OUT gctBOOL*            NeedVertexIDReversePatch
    )
{
    gceSTATUS             status = gcvSTATUS_OK;
    gctUINT               i, f, mainEnter, addCodeCount;
    gctINT                tempRegNo;
    gcSL_INSTRUCTION      code, addCode, movCode, conCode;
    gctBOOL               bVertexIdUsed = gcvFALSE, bInstanceIdUsed = gcvFALSE, bInMainRoutine;
    gctBOOL               needXFB = XFBPatch;
    gcFUNCTION            function;
    gcUNIFORM             uStartVertex;
    gcSL_FORMAT           format;
    gcVARIABLE            vtxIdVariable = gcvNULL, intanceIdVariable = gcvNULL;

    /* 1. Check whether vertexID/instanceID is used within whole shader.*/
    for (i = 0; i < VertexShader->variableCount; ++ i)
    {
        gcVARIABLE variable = VertexShader->variables[i];

        if(!variable) continue;

        if(isVariableNormal(variable) && ((gctINT)variable->nameLength < 0))
        {
            if (variable->nameLength == gcSL_VERTEX_ID)
            {
                vtxIdVariable = variable;
            }
            else if (variable->nameLength == gcSL_INSTANCE_ID)
            {
                intanceIdVariable = variable;
            }
        }
    }

    if (!vtxIdVariable)
        bVertexIdUsed = gcvTRUE;
    if (!intanceIdVariable)
        bInstanceIdUsed = gcvTRUE;

    if (vtxIdVariable || intanceIdVariable)
    {
        for (i = 0; i < VertexShader->codeCount; i ++)
        {
            gcVARIABLE variable;
            code = &VertexShader->code[i];

            if (vtxIdVariable)
            {
                variable = vtxIdVariable;
                if ((gctINT)code->source0Index == variable->tempIndex ||
                    (gctINT)code->source1Index == variable->tempIndex ||
                    (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->source0Indexed == variable->tempIndex) ||
                    (gcmSL_SOURCE_GET(code->source1, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->source1Indexed == variable->tempIndex) ||
                    (gcmSL_TARGET_GET(code->temp, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->tempIndexed == variable->tempIndex))
                {
                    bVertexIdUsed = gcvTRUE;
                }
            }

            if (intanceIdVariable)
            {
                variable = intanceIdVariable;
                if ((gctINT)code->source0Index == variable->tempIndex ||
                    (gctINT)code->source1Index == variable->tempIndex ||
                    (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->source0Indexed == variable->tempIndex) ||
                    (gcmSL_SOURCE_GET(code->source1, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->source1Indexed == variable->tempIndex) ||
                    (gcmSL_TARGET_GET(code->temp, Indexed) != gcSL_NOT_INDEXED &&
                    (gctINT)code->tempIndexed == variable->tempIndex))
                {
                    bInstanceIdUsed = gcvTRUE;
                }
            }

            if (bVertexIdUsed && bInstanceIdUsed)
                break;
        }
    }

    if (!vtxIdVariable)
        bVertexIdUsed = gcvFALSE;
    if (!intanceIdVariable)
        bInstanceIdUsed = gcvFALSE;

    if (!bVertexIdUsed && !bInstanceIdUsed && !needXFB)
        return gcvSTATUS_OK;

    if (needXFB)
    {
        vtxIdVariable = _findVexterInstIDTemp(VertexShader, gcSL_VERTEX_ID);
        intanceIdVariable = _findVexterInstIDTemp(VertexShader, gcSL_INSTANCE_ID);
    }

    /* Find the enter of main function. */
    for (mainEnter = 0; mainEnter < VertexShader->codeCount; mainEnter++)
    {
        bInMainRoutine = gcvTRUE;

        /* Determine ownership of the code for functions. */
        for (f = 0; f < VertexShader->functionCount; ++f)
        {
            function = VertexShader->functions[f];

            if ((mainEnter >= function->codeStart) &&
                (mainEnter < function->codeStart + function->codeCount))
            {
                bInMainRoutine = gcvFALSE;
                break;
            }
        }

        if (bInMainRoutine)
        {
            break;
        }
    }

    /* 2. Convert vertexID/instanceID if needed. */
    if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VERTEX_INST_ID_AS_ATTRIBUTE))
    {
        addCodeCount = 0;
        if (needXFB)
        {
            addCodeCount = 4;
        }
        else
        {
            if (bVertexIdUsed)
                addCodeCount += 2;
            if (bInstanceIdUsed)
                addCodeCount += 2;
        }

        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(VertexShader, mainEnter, addCodeCount));

        if (needXFB || bVertexIdUsed)
        {
            /* Convert vertexID. */
            conCode = &VertexShader->code[mainEnter];
            movCode = &VertexShader->code[mainEnter + 1];

            conCode->opcode = gcSL_F2I;
            conCode->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZW)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
            conCode->tempIndex = (gctUINT16)gcSHADER_NewTempRegs(VertexShader, 1, gcSHADER_FLOAT_X4);

            conCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
            conCode->source0Index = vtxIdVariable->tempIndex;

            movCode->opcode = gcSL_MOV;
            movCode->temp = conCode->temp;
            movCode->tempIndex = vtxIdVariable->tempIndex;

            movCode->source0 = gcmSL_SOURCE_SET(conCode->source0, Format, gcSL_INTEGER);
            movCode->source0Index = conCode->tempIndex;

            mainEnter += 2;
        }

        if (needXFB || bInstanceIdUsed)
        {
            /* Convert instanceID. */
            conCode = &VertexShader->code[mainEnter];
            movCode = &VertexShader->code[mainEnter + 1];

            conCode->opcode = gcSL_F2I;
            conCode->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZW)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
            conCode->tempIndex = (gctUINT16)gcSHADER_NewTempRegs(VertexShader, 1, gcSHADER_FLOAT_X4);

            conCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
            conCode->source0Index = intanceIdVariable->tempIndex;

            movCode->opcode = gcSL_MOV;
            movCode->temp = conCode->temp;
            movCode->tempIndex = intanceIdVariable->tempIndex;

            movCode->source0 = gcmSL_SOURCE_SET(conCode->source0, Format, gcSL_INTEGER);
            movCode->source0Index = conCode->tempIndex;
            mainEnter += 2;
        }

        gcmONERROR(gcSHADER_Pack(VertexShader));
    }

    /* 3. Add patch at the entry of shader */
    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI0) &&
        !gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_FE_START_VERTEX_SUPPORT) &&
        bVertexIdUsed && vtxIdVariable)
    {
        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(VertexShader, mainEnter, 2));

        addCode = &VertexShader->code[mainEnter];
        movCode = &VertexShader->code[mainEnter + 1];

        gcmONERROR(_CreatePatchUniform(VertexShader,
                                        "sh_startVertex",
                                        0,
                                        gcSHADER_INTEGER_X1,
                                        gcvUNIFORM_KIND_GENERAL_PATCH,
                                        gcvFALSE,
                                        gcSHADER_PRECISION_HIGH,
                                        &uStartVertex));

        format = gcSL_INTEGER;

        /* gl_vertexID = gl_vertexID + startVertex */

        tempRegNo = _getUnusedTempIndex(VertexShader, 1);

        addCode->opcode = gcSL_ADD;
        addCode->temp = gcmSL_TARGET_SET(0, Format, format) |
                          gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X) |
                          gcmSL_TARGET_SET(0, Precision, gcSHADER_PRECISION_HIGH);
        addCode->tempIndex = (gctUINT16)tempRegNo;
        addCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP) |
                             gcmSL_SOURCE_SET(0, Format, format)  |
                             gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_HIGH)  |
                             gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
        addCode->source0Index = (gctUINT16)vtxIdVariable->tempIndex;
        addCode->source1 = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM) |
                             gcmSL_SOURCE_SET(0, Format, format)  |
                             gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_HIGH)  |
                             gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
        addCode->source1Index = (gctUINT16)uStartVertex->index;
        SetUniformFlag(uStartVertex, gcvUNIFORM_FLAG_DIRECTLY_ADDRESSED);
        SetUniformFlag(uStartVertex, gcvUNIFORM_FLAG_COMPILER_GEN);

        movCode->opcode = gcSL_MOV;
        movCode->temp = gcmSL_TARGET_SET(0, Format, format) |
                        gcmSL_TARGET_SET(0, Precision, gcSHADER_PRECISION_HIGH) |
                          gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X);
        movCode->tempIndex = (gctUINT16)vtxIdVariable->tempIndex;
        movCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP) |
                             gcmSL_SOURCE_SET(0, Format, format)  |
                             gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_HIGH)  |
                             gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
        movCode->source0Index = (gctUINT16)tempRegNo;
        gcmONERROR(gcSHADER_Pack(VertexShader));

        *NeedVertexIDReversePatch = gcvTRUE;
    }

    gcmONERROR(gcSHADER_Pack(VertexShader));

OnError:
    return status;
}

typedef struct _gcsMovConstToVector
{
    gctUINT          tempIndex;     /* temp index for the constant vector */
    gcSL_ENABLE      enable;
    gcSL_FORMAT      format;        /* the format of each component */
    gctINT           assignNo;      /* number of distinct assignement */
    gctBOOL          hasNonConstComponent;
    gctBOOL          redefintion;   /* redefinition of some component */
    gctBOOL          useBeforeLastDefine;
    gctBOOL          skippedComponent;
    gcSL_INSTRUCTION component[4];  /* instruction to assign x/y/z/w component */
    gcUNIFORM        uniform;       /* the compiler generated vector
                                       uniform for the constant */
} gcsMovConstToVector;

typedef gcsList gcConstVectorList;

static gceSTATUS
_constVectorListAllocate(
    IN gctUINT32 Bytes,
    OUT gctPOINTER * Memory
    )
{
    return gcoOS_Allocate(gcvNULL, Bytes, Memory);
};

static gceSTATUS
_constVectorListFree(
    IN gctPOINTER Memory
    )
{
    return gcoOS_Free(gcvNULL, Memory);
}

const gcsAllocator constVectorListAllocator = {_constVectorListAllocate, _constVectorListFree };

/* the Key is temp register index, the Data is pointer to gcsMovConstToVector
 * return true if the temp register index is the same as tempIndex in Data
 */
static gctBOOL
CompareIndex (
     IN void *    Data,
     IN void *    Key
     )
{
    gctINT regIndex1 = ((gcsMovConstToVector *)Data)->tempIndex;
    gctINT regIndex2 = ((gctUINT)(gctUINTPTR_T)Key);

    return  regIndex1 ==  regIndex2 ;
} /* CompareIndex */

static void
_addInstToConstVectorNode(
    IN OUT gcsMovConstToVector * Node,
    IN gcSL_INSTRUCTION          Inst
    )
{
    gcSL_ENABLE  instEnable;
    gcSL_FORMAT  format;
    gcmASSERT(Node != gcvNULL);
    instEnable = gcmSL_TARGET_GET(Inst->temp, Enable);
    format = gcmSL_TARGET_GET(Inst->temp, Format);

    if (Node->format == gcSL_INVALID)
    {
        Node->format = format;
    }
    else if (Node->format != format)
    {
        /* components type mismatch,
         * only put same type date to constant vector */
        Node->skippedComponent = gcvTRUE;
        return;
    }
    if ((instEnable & Node->enable) != gcSL_ENABLE_NONE)
    {
        /* redefinition of some component */
        Node->redefintion = gcvTRUE;
        return ;
    }
    Node->enable |= instEnable;
    if (instEnable & gcSL_ENABLE_X)
    {
        Node->component[0] = Inst;
    }
    if (instEnable & gcSL_ENABLE_Y)
    {
        Node->component[1] = Inst;
    }
    if (instEnable & gcSL_ENABLE_Z)
    {
        Node->component[2] = Inst;
    }
    if (instEnable & gcSL_ENABLE_W)
    {
        Node->component[3] = Inst;
    }
    Node->assignNo++;
}

gceSTATUS
_AddInstToConstVectorList(
     IN gcConstVectorList*    List,
     IN gcSL_INSTRUCTION      Inst
     )
{
    gceSTATUS             status     = gcvSTATUS_OK;
    gctUINT               tempIndex  = Inst->tempIndex;
    gcsMovConstToVector * node;
    /* check if there is a node in the list which has the same temp
     * register Index
     */
    gcmASSERT(gcmSL_OPCODE_GET(Inst->opcode, Opcode) == gcSL_MOV &&
              gcmSL_SOURCE_GET(Inst->source0, Type) == gcSL_CONSTANT);

     node = (gcsMovConstToVector*)gcList_FindNode(List,
                                         (gctPOINTER)(gctUINTPTR_T)tempIndex,
                                         CompareIndex);
    if (node == gcvNULL)
    {
        /* allocate node */
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  sizeof(gcsMovConstToVector),
                                  (gctPOINTER *)&node));
        gcoOS_ZeroMemory(node, sizeof(gcsMovConstToVector));

        status = gcList_AddNode(List, (gctPOINTER)node);
    } /* if */

    _addInstToConstVectorNode(node, Inst);

OnError:
    return status;
} /* _AddInstToConstVectorList */

/* create constant uniform for the constant vector */
static gceSTATUS
_createUniformForConstVector(
    IN gcSHADER                  Shader,
    IN OUT gcsMovConstToVector * ConstVectorNode
    )
{
    gceSTATUS      status   = gcvSTATUS_OK;
    gcSHADER_TYPE  type;
    gcUNIFORM      uniform;
    gcsValue       constValue;
    gctINT         i;
    gctINT         componentCount = 0;

    constValue.i32_v4[0] = constValue.i32_v4[1] =
        constValue.i32_v4[2] =constValue.i32_v4[3] = 0;

    /* get the type */
    if (ConstVectorNode->component[3] != gcvNULL)
    {
        componentCount = 4;
    }
    else if (ConstVectorNode->component[2] != gcvNULL)
    {
        componentCount = 3;
    }
    else if (ConstVectorNode->component[1] != gcvNULL)
    {
        componentCount = 2;
    }
    else
    {
        componentCount = 1;
    }

    type = gcGetShaderTypeFromFormatAndComponentCount(ConstVectorNode->format,
                                                      componentCount,
                                                      1 /* rows */);
    /* get the value */
    for (i=0; i<4; i++)
    {
        if (ConstVectorNode->component[i] != gcvNULL)
        {
            constValue.i32_v4[i] =
                (ConstVectorNode->component[i]->source0Index  & 0xFFFF) |
                    (ConstVectorNode->component[i]->source0Indexed  << 16);
        }
    }

    /* create uniform and initialize it with constValue */
    gcmONERROR(gcSHADER_CreateConstantUniform(Shader, type,
                                              &constValue, &uniform));

    /* return the created constant uniform */
    ConstVectorNode->uniform = uniform;

OnError:
    return status;
}

static void
_changeUserToUseConstUniform(
    IN gctINT                   CodeIndex,
    IN OUT gcSL_INSTRUCTION     Inst,
    IN gcsMovConstToVector *   ConstVectorNode
    )
{
    gctSOURCE_t  source;
    gctUINT      index;
    gctINT       i;
    gctUINT16    opcode = gcmSL_OPCODE_GET(Inst->opcode, Opcode);

    for (i = 0; i < 2; i++)  /* iterate through source 0 and source 1 */
    {
        source = (i == 0) ? Inst->source0 : Inst->source1;
        index  = (i == 0) ? Inst->source0Index : Inst->source1Index;

        if (gcmSL_SOURCE_GET(source, Type) == gcSL_TEMP &&
            index == ConstVectorNode->tempIndex)
        {
            gcmASSERT(gcmSL_SOURCE_GET(source, Indexed) == gcSL_NOT_INDEXED);
            /* change source type and index */
            source = gcmSL_SOURCE_SET(source, Type, gcSL_UNIFORM);
            if (i == 0)
            {
                Inst->source0      = source;
                Inst->source0Index = ConstVectorNode->uniform->index;
            }
            else
            {
                Inst->source1      = source;
                Inst->source1Index = ConstVectorNode->uniform->index;
            }
            /* keep the format and swizzle same */
        }
    }

    if (CodeIndex > 0 && gcSL_isOpcodeTexld(opcode))
    {
        /*
        ** This temp may be used as the source0/source1 of instruction "texgrad",
        ** we need to update them too.
        */
        gcSL_INSTRUCTION prevCode = Inst - 1;
        gctUINT16        prevOpcode = gcmSL_OPCODE_GET(prevCode->opcode, Opcode);

        if (gcSL_isOpcodeTexldModifier(prevOpcode))
        {
            _changeUserToUseConstUniform(CodeIndex - 1, prevCode, ConstVectorNode);
        }
    }
}


/* remove the dependency on TempIndex from the Code */
static void
_removeTempFromCodeDependency(
    IN gcLINKTREE         Tree,
    IN gcSL_INSTRUCTION   Code,
    IN gctINT             TempIndex
    )
{
    gcSL_ENABLE enable      = gcmSL_TARGET_GET(Code->temp, Enable);

    if (enable == gcSL_ENABLE_NONE)
        return;  /* no output is produced */

    _RemoveItem(Tree,
                &Tree->tempArray[Code->tempIndex].dependencies,
                gcSL_TEMP,
                TempIndex);
}

static gctBOOL
_needKeepTemp(
    IN gcLINKTREE_TEMP       Temp,
    IN gcsMovConstToVector * Cv
    )
{
    if (Temp->isIndexing         ||
        Cv->redefintion          ||
        Cv->hasNonConstComponent ||
        Cv->skippedComponent     ||
        Cv->useBeforeLastDefine
        )
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

/* create assignment
 *
 *      MOV  temp, uniform
 *
 * and remove all constant value assignment for temp
 */
static void
_replaceConstMoveWithUniformAssign(
    IN gcLINKTREE            Tree,
    IN gcLINKTREE_TEMP       Temp,
    IN gcsMovConstToVector * Cv
    )
{
    gcsLINKTREE_LIST_PTR node;
    gctINT               i;
    gcSL_INSTRUCTION     assignInst = gcvNULL;

    gcmASSERT(Cv->assignNo > 1);

    for (i=0; i<4; i++)
    {
        if (Cv->component[i] == gcvNULL)
            continue;

        if  (assignInst == gcvNULL)
        {
            gctSOURCE_t source0;
            assignInst = Cv->component[i];
            source0 = assignInst->source0;

            /* change the first MOV to assgin constant vector */
            assignInst->temp = gcmSL_TARGET_SET(assignInst->temp,
                                                Enable, Cv->enable);
            source0 = gcmSL_SOURCE_SET(source0, Type, gcSL_UNIFORM);
            source0 = gcmSL_SOURCE_SET(source0, Indexed, gcSL_NOT_INDEXED);
            source0 = gcmSL_SOURCE_SET(source0, Swizzle,
                                       _Enable2SwizzleWShift(Cv->enable));

            assignInst->source0 = source0;
            assignInst->source0Index = Cv->uniform->index;
            assignInst->source0Indexed = 0;
        }
        else if (Cv->component[i] != assignInst) /* may happen: MOV temp(1).xy, 1.0 */
        {
            /* remove rest of MOVs */
            gcoOS_ZeroMemory(Cv->component[i],
                             gcmSIZEOF(struct _gcSL_INSTRUCTION));
        }
    }

    /* update definition list */
    for (node = Temp->defined; node != gcvNULL; )
    {
        gcSL_INSTRUCTION     code = Tree->shader->code + node->index;
        gcsLINKTREE_LIST_PTR next = node->next;

        /* remove all definitions except for the one changed to assign
         * const vector */
        if (code != assignInst)
        {
            _RemoveItem(Tree,
                        &Temp->defined,
                        gcSL_TEMP,
                        node->index);
        }
        node = next;
    }
}

static void
_replaceUserWithConstantUniform(
    IN gcLINKTREE            Tree,
    IN gcLINKTREE_TEMP       Temp,
    IN gcsMovConstToVector * Cv
    )
{
    gcsLINKTREE_LIST_PTR node;
    gcsLINKTREE_LIST_PTR user;
    /* change all the users to use the constant uniform */
    for (user = Temp->users;
         user != gcvNULL;
         user = user->next)
    {
        gcSL_INSTRUCTION code = Tree->shader->code + user->index;

        _changeUserToUseConstUniform(user->index, code, Cv);

        /* remove the dependency from the user */
        _removeTempFromCodeDependency(Tree, code, Temp->index);
    }

    /* Remove the temporary register usage. */
    Temp->inUse   = gcvFALSE;
    Temp->lastUse = -1;
    Temp->usage   = 0;

    /* Remove all definitions. */
    for (node = Temp->defined;
         node != gcvNULL;
         node = node->next)
    {
        gcSL_INSTRUCTION code = Tree->shader->code + node->index;
        /* Replace instruction with a NOP. */
        gcoOS_ZeroMemory(code, gcmSIZEOF(struct _gcSL_INSTRUCTION));
    }
    _Delete(Tree, &Temp->defined);

    /* Remove all dependencies. */
    _Delete(Tree, &Temp->dependencies);

    /* Remove all users. */
    _Delete(Tree, &Temp->users);
}

/*******************************************************************************
**                          gcLINKTREE_AllocateConstantUniform
********************************************************************************
**
**  Allocate uniform vector for constant vector:
**      Current IR don't support initialized varible, they have to move to
**      temp register before accessed as vector, which caused unnecessary
**      MOVs in final code. To generate better code, this phase identify
**      the MOVs which move constant values to a vector and replace them
**      with compile-time constant uniform.
**
**  INPUT:
**
**      gcLINKTREE Tree
**          Pointer to a gcLINKTREE structure.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gcLINKTREE_AllocateConstantUniform(
    IN gcLINKTREE Tree
    )
{
    gctUINT i;
    gctUINT maxShaderUniforms, vsUniform, psUniform;
    gctUINT curUsedUniform     = 0;

    gcSHADER_GetUniformVectorCount(Tree->shader, &curUsedUniform);

    gcmVERIFY_OK(gcoHAL_QueryShaderCaps(
                   gcvNULL,
                   gcvNULL,
                   &vsUniform,
                   &psUniform,
                   gcvNULL,
                   gcvNULL,
                   gcvNULL,
                   gcvNULL,
                   gcvNULL));

    maxShaderUniforms = (Tree->shader->type == gcSHADER_TYPE_VERTEX)
                        ? vsUniform
                        : psUniform;

    /* If there is not uniform memory left, skip this optimization. */
    if (curUsedUniform >= maxShaderUniforms)
    {
        return gcvSTATUS_OK;
    }

     /* Walk all temporary registers. */
    for (i = 0; i < Tree->tempCount; i++)
    {
        gcLINKTREE_TEMP temp = &Tree->tempArray[i];

        /* skip 64bit variable */
        if (isFormat64bit(temp->format)) continue;

        /* Only process if temporary register is used and more than one
           definition for the temp, and it is not depended on other variable. */
        if (temp->inUse
        &&  (temp->defined != gcvNULL)
        &&  (temp->defined->next != gcvNULL)
        &&  (temp->dependencies == gcvNULL)
        &&  (temp->users != gcvNULL)
        )
        {
            gcsMovConstToVector cv;
            gctINT minDefPC, maxDefPC;
            gctINT minUsePC;
            gcsLINKTREE_LIST_PTR node;
            gcsLINKTREE_LIST_PTR user;
            gctBOOL              userIsOutput  = gcvFALSE;
            gctBOOL              userIsIndexed = gcvFALSE;

            /* find the first user instruciton index */
            minUsePC = temp->users->index;
            for (user = temp->users;
                 user != gcvNULL;
                 user = user->next)
            {
                gcSL_INSTRUCTION code;

                if (user->type == gcSL_OUTPUT)
                {
                    userIsOutput = gcvTRUE;
                    break;
                }

                /* If the user is a output, the index of user maybe exceed the code number. */
                code = Tree->shader->code + user->index;

                /* check if the user is indexed */
                if ((gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP &&
                     code->source0Index == i &&
                     gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED) ||
                    (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP &&
                     code->source1Index == i &&
                     gcmSL_SOURCE_GET(code->source1, Indexed) != gcSL_NOT_INDEXED) )
                {
                    userIsIndexed = gcvTRUE;
                    break;
                }

                if (user->index < minUsePC)
                {
                    minUsePC = user->index;
                }
            }

            /* TODO: handle user is output case or indexed */
            if (userIsOutput || userIsIndexed)
                continue;

            gcoOS_ZeroMemory(&cv, sizeof(gcsMovConstToVector));

            cv.tempIndex = i;
            cv.format    = gcSL_INVALID;
            minDefPC = maxDefPC = temp->defined->index;

            for (node = temp->defined;
                 node != gcvNULL;
                 node = node->next)
            {
                gcSL_INSTRUCTION code = Tree->shader->code + node->index;
                if (node->index < minDefPC)
                {
                    minDefPC = node->index;
                }
                else if (node->index > maxDefPC)
                {
                    maxDefPC = node->index;
                }


                if (node->index >= minUsePC)
                {
                    /* the defintion is after the first use, we ignore
                       the definitions after the first use */
                    cv.useBeforeLastDefine = gcvTRUE;
                    break;
                }
                if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MOV &&
                    gcmSL_SOURCE_GET(code->source0, Type) == gcSL_CONSTANT)
                {
                    _addInstToConstVectorNode(&cv, code);
                    if (cv.redefintion)
                        break;
                }
                else
                {
                    cv.hasNonConstComponent = gcvTRUE;
                    break;
                }
            }

            /* bail out if some definition is non-constant or defined after the
             * first use, or no multiple constant assignment to the temp register,
             * or some component is redefined, or the defines are not in the same
             * basic block
             */
            if (cv.assignNo <= 1 ||
                !_isCodeInSameBB(Tree, minDefPC, maxDefPC, gcvNULL, gcvNULL))
            {
                continue;
            }

            /* create constant uniform from the constant vector */
            _createUniformForConstVector(Tree->shader, &cv);
            curUsedUniform++;

            if (_needKeepTemp(temp, &cv))
            {
                /* if the temp can not be removed, we need to create assignment
                 *
                 *      MOV  temp, uniform
                 *
                 * and remove all constant value assignment for temp
                 */
                _replaceConstMoveWithUniformAssign(Tree, temp, &cv);
            }
            else
            {
                _replaceUserWithConstantUniform(Tree, temp, &cv);
            }
        }
    }

    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
_splitInstructionHasSameDestAndSrcTempIndex(
    IN gcSHADER Shader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T origCodeNum = Shader->lastInstruction;
    gctSIZE_T i, insertPoint;
    gcSL_INSTRUCTION inst, newInst[2] = {gcvNULL, gcvNULL}, movInst = gcvNULL;
    gcSL_FORMAT format[2] = {0, 0};
    gcSL_SWIZZLE swizzle[2] = {0, 0};
    gcSL_ENABLE enable[2] = {0, 0};
    gctBOOL isCodeChanged = gcvFALSE;
    gctBOOL isCmpPair = gcvFALSE;
    gctUINT16 newTemp[2] = {0, 0};

    for (i = 0; i < origCodeNum; i++)
    {
        inst = &Shader->code[i];

        isCmpPair = gcvFALSE;

        if (inst->opcode == gcSL_RET || inst->opcode == gcSL_CALL || inst->opcode == gcSL_JMP)
            continue;

        insertPoint = i;

        if ((inst->opcode == gcSL_SET || inst->opcode == gcSL_CMP) &&
             gcmSL_TARGET_GET(inst->temp, Condition) == gcSL_NOT_ZERO)
        {
            if (i > 0)
            {
                if ((Shader->code[i-1].opcode == gcSL_SET || Shader->code[i-1].opcode == gcSL_CMP) &&
                    gcmSL_TARGET_GET(Shader->code[i-1].temp, Condition) == gcSL_ZERO)
                {
                    insertPoint = i - 1;
                    if (i > 1)
                    {
                        movInst = &Shader->code[i-2];
                        if ((movInst->opcode == gcSL_MOV) &&
                            (gcmSL_SOURCE_GET(movInst->source0, Type) == gcSL_TEMP) &&
                            (inst->tempIndex == movInst->source0Index))
                        {
                            isCmpPair = gcvTRUE;
                            newTemp[0] = movInst->tempIndex;
                            newTemp[1] = movInst->tempIndex;
                        }
                    }
                }
            }
        }

        if ((gcmSL_SOURCE_GET(inst->source0, Type) == gcSL_TEMP && inst->tempIndex == inst->source0Index) ||
            (gcmSL_SOURCE_GET(inst->source1, Type) == gcSL_TEMP && inst->tempIndex == inst->source1Index))
        {
            gctBOOL isTwoSourceSameRegister = gcvFALSE;

            /* If source0 and source1 have the same temp register and different swizzle value,
            ** we need to add two MOV instructions.
            */
            if (gcmSL_SOURCE_GET(inst->source0, Type) == gcSL_TEMP &&
                gcmSL_SOURCE_GET(inst->source1, Type) == gcSL_TEMP &&
                inst->source0Index == inst->source1Index &&
                (gcmSL_SOURCE_GET(inst->source0, Format) != gcmSL_SOURCE_GET(inst->source1, Format) ||
                 gcmSL_SOURCE_GET(inst->source0, Swizzle) != gcmSL_SOURCE_GET(inst->source1, Swizzle)))
            {
                isTwoSourceSameRegister = gcvTRUE;
            }

            if (isTwoSourceSameRegister)
            {
                /* Insert two NOP instructions before current instruction. */
                gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, (gctUINT)insertPoint, 2));
                inst = &Shader->code[i + 2];
                newInst[0] = &Shader->code[insertPoint];
                newInst[1] = &Shader->code[insertPoint + 1];

                enable[0] = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source0, SwizzleX),
                                                       (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source0, SwizzleY),
                                                       (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source0, SwizzleZ),
                                                       (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source0, SwizzleW));
                format[0] = gcmSL_SOURCE_GET(inst->source0, Format);
                swizzle[0] = _Enable2SwizzleWShift(enable[0]);

                enable[1] = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source1, SwizzleX),
                                                       (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source1, SwizzleY),
                                                       (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source1, SwizzleZ),
                                                       (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source1, SwizzleW));
                format[1] = gcmSL_SOURCE_GET(inst->source1, Format);
                swizzle[1] = _Enable2SwizzleWShift(enable[1]);
            }
            else
            {
                /* Insert a NOP instruction before current instruction. */
                if (!isCmpPair)
                {
                    gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, (gctUINT)insertPoint, 1));
                    inst = &Shader->code[i+1];
                    newInst[0] = &Shader->code[insertPoint];
                }

                /* Get the source format. */
                if (gcmSL_SOURCE_GET(inst->source0, Type) == gcSL_TEMP && inst->tempIndex == inst->source0Index)
                {
                    enable[0] = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source0, SwizzleX),
                                                           (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source0, SwizzleY),
                                                           (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source0, SwizzleZ),
                                                           (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source0, SwizzleW));
                    format[0] = gcmSL_SOURCE_GET(inst->source0, Format);
                    swizzle[0] = _Enable2SwizzleWShift(enable[0]);
                }
                else
                {
                    enable[0] = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source1, SwizzleX),
                                                           (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source1, SwizzleY),
                                                           (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source1, SwizzleZ),
                                                           (gcSL_SWIZZLE) gcmSL_SOURCE_GET(inst->source1, SwizzleW));
                    format[0] = gcmSL_SOURCE_GET(inst->source1, Format);
                    swizzle[0] = _Enable2SwizzleWShift(enable[0]);
                }
            }

            /* Mov the original source to a new temp register and use the new register as the source. */
            if (!isCmpPair)
            {
                newTemp[0] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
                newInst[0]->opcode = gcSL_MOV;
                newInst[0]->temp = gcmSL_TARGET_SET(0, Format, format[0])
                                          | gcmSL_TARGET_SET(0, Enable, enable[0]);
                newInst[0]->tempIndex = newTemp[0];
                newInst[0]->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                                          | gcmSL_SOURCE_SET(0, Format, format[0])
                                          | gcmSL_SOURCE_SET(0, Swizzle, swizzle[0]);
                newInst[0]->source0Index = inst->tempIndex;
            }

            if (isTwoSourceSameRegister)
            {
                newTemp[1] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
                newInst[1]->opcode = gcSL_MOV;
                newInst[1]->temp = gcmSL_TARGET_SET(0, Format, format[1])
                                          | gcmSL_TARGET_SET(0, Enable, enable[1]);
                newInst[1]->tempIndex = newTemp[1];
                newInst[1]->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                                          | gcmSL_SOURCE_SET(0, Format, format[1])
                                          | gcmSL_SOURCE_SET(0, Swizzle, swizzle[1]);
                newInst[1]->source0Index = inst->tempIndex;
            }

            /* Update the original instruction. */
            if (isTwoSourceSameRegister)
            {
                inst->source0Index = newTemp[0];
                inst->source1Index = newTemp[1];
            }
            else
            {
                if (gcmSL_SOURCE_GET(inst->source0, Type) == gcSL_TEMP && inst->tempIndex == inst->source0Index)
                {
                    inst->source0Index = newTemp[0];
                }
                if (gcmSL_SOURCE_GET(inst->source1, Type) == gcSL_TEMP && inst->tempIndex == inst->source1Index)
                {
                    inst->source1Index = newTemp[0];
                }
            }

            /* Mov the next instruction. */
            if (isTwoSourceSameRegister)
            {
                origCodeNum += 2;
                i += 2;
            }
            else if (!isCmpPair)
            {
                origCodeNum++;
                i++;
            }

            if (!isCodeChanged)
                isCodeChanged = gcvTRUE;

            Shader->instrIndex = gcSHADER_OPCODE;
        }
    }

    if (isCodeChanged)
    {
        gcmONERROR(gcSHADER_Pack(Shader));
        if (gcSHADER_DumpOptimizer(Shader))
        {
            gcOpt_Dump(gcvNULL, "After add mov before instructions hold the same target index and source index", gcvNULL, Shader);
        }
    }
    return status;

OnError:
    return status;
}

static gceSTATUS
_ConvertIntegerBranchToFloat(
    IN gcSHADER         Shader
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctBOOL isCodeChange = gcvFALSE;
    gctINT i;
    gcSL_INSTRUCTION currentCode, newInst[2] = {gcvNULL, gcvNULL};
    gcSL_FORMAT format;
    gcSL_PRECISION precision;
    gcSL_SWIZZLE swizzle[2] = {0, 0};
    gcSL_ENABLE enable[2] = {0, 0};
    gctUINT16 newTemp[2] = {0, 0};
    static const gcSL_ENABLE component2Enable[] = {
             gcSL_ENABLE_X,
             gcSL_ENABLE_XY,
             gcSL_ENABLE_XYZ,
             gcSL_ENABLE_XYZW
    };

    for (i = (gctINT)(Shader->codeCount - 1); i >= 0; i--)
    {
        currentCode = &Shader->code[i];
        format = gcmSL_SOURCE_GET(currentCode->source0, Format);

        /* Skip non-jump instructions. */
        if (gcmSL_OPCODE_GET(currentCode->opcode, Opcode) != gcSL_JMP) continue;

        /* Skip always jump instructions.*/
        if (gcmSL_TARGET_GET(currentCode->temp, Condition) == gcSL_ALWAYS) continue;

        /* Skip float jump instructions. */
        if (format == gcSL_FLOAT || format == gcSL_FLOAT16 ||
            format == gcSL_FLOAT64) continue;

        /* Insert two NOPS before current instruction. */
        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, (gctUINT)i, 2));

        currentCode = &Shader->code[i + 2];
        newInst[0] = &Shader->code[i];
        newInst[1] = &Shader->code[i + 1];

        precision = gcmSL_SOURCE_GET(currentCode->source0, Precision);

        /* Fill the first gcSL_I2F instruction. */
        enable[0] = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(currentCode->source0, SwizzleX),
                                               (gcSL_SWIZZLE) gcmSL_SOURCE_GET(currentCode->source0, SwizzleY),
                                               (gcSL_SWIZZLE) gcmSL_SOURCE_GET(currentCode->source0, SwizzleZ),
                                               (gcSL_SWIZZLE) gcmSL_SOURCE_GET(currentCode->source0, SwizzleW));
        enable[0] = component2Enable[_getEnableComponentCount(enable[0]) - 1];
        newTemp[0] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);

        newInst[0]->opcode = gcSL_I2F;
        newInst[0]->temp = gcmSL_TARGET_SET(0, Format, gcSL_FLOAT)
                                  | gcmSL_TARGET_SET(0, Enable, enable[0])
                                  | gcmSL_TARGET_SET(0, Precision, precision);
        newInst[0]->tempIndex = newTemp[0];
        newInst[0]->source0 = currentCode->source0;
        newInst[0]->source0Index = currentCode->source0Index;
        newInst[0]->source0Indexed = currentCode->source0Indexed;

        /* Fill the second gcSL_I2F instruction. */
        enable[1] = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(currentCode->source1, SwizzleX),
                                               (gcSL_SWIZZLE) gcmSL_SOURCE_GET(currentCode->source1, SwizzleY),
                                               (gcSL_SWIZZLE) gcmSL_SOURCE_GET(currentCode->source1, SwizzleZ),
                                               (gcSL_SWIZZLE) gcmSL_SOURCE_GET(currentCode->source1, SwizzleW));
        enable[1] = component2Enable[_getEnableComponentCount(enable[1]) - 1];
        newTemp[1] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);

        newInst[1]->opcode = gcSL_I2F;
        newInst[1]->temp = gcmSL_TARGET_SET(0, Format, gcSL_FLOAT)
                                  | gcmSL_TARGET_SET(0, Enable, enable[1])
                                  | gcmSL_TARGET_SET(0, Precision, precision);
        newInst[1]->tempIndex = newTemp[1];
        newInst[1]->source0 = currentCode->source1;
        newInst[1]->source0Index = currentCode->source1Index;
        newInst[1]->source0Indexed = currentCode->source1Indexed;

        /* Update the original instruction. */
        swizzle[0] = _Enable2SwizzleWShift(enable[0]);
        swizzle[1] = _Enable2SwizzleWShift(enable[1]);

        currentCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                                        | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                                        | gcmSL_SOURCE_SET(0, Swizzle, swizzle[0]);
        currentCode->source0Index = newTemp[0];
        currentCode->source0Indexed = 0;

        currentCode->source1 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                                        | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                                        | gcmSL_SOURCE_SET(0, Swizzle, swizzle[1]);
        currentCode->source1Index = newTemp[1];
        currentCode->source1Indexed = 0;

        isCodeChange = gcvTRUE;
        Shader->instrIndex = gcSHADER_OPCODE;
    }

    if (isCodeChange)
    {
        gcmONERROR(gcSHADER_Pack(Shader));
        if (gcSHADER_DumpOptimizer(Shader))
        {
            gcOpt_Dump(gcvNULL, "Convert integer branch to float, it may lose precision!!!!!", gcvNULL, Shader);
        }
    }

    gcmONERROR(status);
    return status;

OnError:
    return status;
}


static gceSTATUS
_converrtImageReadToTexld(
    IN gcSHADER         Shader
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gcKERNEL_FUNCTION   kernelFunction = gcvNULL;
    gctSIZE_T           origUniformCount;
    gctSIZE_T           imageSamplerCount = 0;
    gctSIZE_T           i, j;
    gcSL_INSTRUCTION    inst, instNext;
    gcUNIFORM           uniform;
    gctUINT8            imageNum;
    gctBOOL             isConstantSamplerType;
    gctUINT32           samplerType;
    gctBOOL             isCodeChanged = gcvFALSE;

    /* Get main kernel function. */
    gcmASSERT(Shader->kernelFunctions);
    for (i = 0; i < Shader->kernelFunctionCount; i++)
    {
        if (Shader->kernelFunctions[i] == gcvNULL) continue;

        if (Shader->kernelFunctions[i]->isMain)
        {
            kernelFunction = Shader->kernelFunctions[i];
            break;
        }
    }

    if (kernelFunction == gcvNULL)
    {
        gcmASSERT(kernelFunction);
        return gcvSTATUS_INVALID_DATA;
    }

    if (kernelFunction->imageSamplerCount)
    {
        /* Delete the old imageSamplers. */
        kernelFunction->imageSamplerCount = 0;
    }

    /* Get the original uniform count to get new uniform index. */
    origUniformCount = Shader->uniformCount;

    for (i = 0; i < origUniformCount; i++)
    {
        if (Shader->uniforms[i])
        {
            gcSHADER_TYPE type = Shader->uniforms[i]->u.type;
            if ((type == gcSHADER_IMAGE_2D) || (type == gcSHADER_IMAGE_3D) ||
                (type == gcSHADER_IMAGE_1D) || (type == gcSHADER_IMAGE_1D_ARRAY) ||
                (type == gcSHADER_IMAGE_1D_BUFFER) || (type == gcSHADER_IMAGE_2D_ARRAY))
            {
                break;
            }
        }
    }

    if (i == origUniformCount)
    {
        /* No image used in shader. */
        return status;
    }

    for (i = 0; i < Shader->codeCount; i++)
    {
        inst = &Shader->code[i];

        if (gcmSL_OPCODE_GET(inst->opcode, Opcode) != gcSL_IMAGE_SAMPLER)
        {
            continue;
        }

        i++;
        instNext = &Shader->code[i];
        if (gcmSL_OPCODE_GET(instNext->opcode, Opcode) != gcSL_IMAGE_RD &&
            gcmSL_OPCODE_GET(instNext->opcode, Opcode) != gcSL_IMAGE_RD_3D)
        {
            gcmASSERT(inst->opcode != gcSL_IMAGE_RD &&
                      inst->opcode != gcSL_IMAGE_RD_3D);
            continue;
        }

        gcmASSERT(inst->source0 == instNext->source0);
        gcmASSERT(inst->source0Index == instNext->source0Index);
        gcmASSERT(inst->source0Indexed == instNext->source0Indexed);

        if(gcmSL_SOURCE_GET(inst->source0, Type) != gcSL_UNIFORM) {
            gctSOURCE_t *uniformSrc = gcvNULL;
            gctUINT16 uniformSrcIndex;
            gctUINT tempIndex;

            gcmASSERT(gcmSL_SOURCE_GET(inst->source0, Type) == gcSL_TEMP);

            tempIndex = gcmSL_INDEX_GET(inst->source0Index, Index);
            /* find image uniform */
            for (j = i - 1; j >= 0; j--)
            {
                gcSL_INSTRUCTION prevInst;

                prevInst = &Shader->code[j];
                if (prevInst->tempIndex == tempIndex)
                {
                    gcSL_TYPE srcType;

                    gcmASSERT(gcmSL_OPCODE_GET(prevInst->opcode, Opcode) == gcSL_MOV);

                    srcType = gcmSL_SOURCE_GET(prevInst->source0, Type);
                    if(srcType != gcSL_NONE)
                    {
                        if(srcType == gcSL_UNIFORM)
                        {
                            uniformSrc = &prevInst->source0;
                            uniformSrcIndex = prevInst->source0Index;
                        }
                        else if(srcType == gcSL_TEMP)
                        {
                            tempIndex = gcmSL_INDEX_GET(prevInst->source0Index, Index);
                            continue;
                        }
                        else break;
                    }
                    else
                    {
                        srcType = gcmSL_SOURCE_GET(prevInst->source1, Type);
                        if(srcType == gcSL_UNIFORM)
                        {
                            uniformSrc = &prevInst->source1;
                            uniformSrcIndex = prevInst->source1Index;
                        }
                        else if(srcType == gcSL_TEMP)
                        {
                            tempIndex = gcmSL_INDEX_GET(prevInst->source1Index, Index);
                            continue;
                        }
                        else break;
                    }
                    if(uniformSrc)
                    {
                         inst->source0 = *uniformSrc;
                         inst->source0Index = uniformSrcIndex;
                         instNext->source0 = *uniformSrc;
                         instNext->source0Index = uniformSrcIndex;
                    }
                    break;
                }
            }
        }

        gcmASSERT(gcmSL_SOURCE_GET(inst->source0, Type) == gcSL_UNIFORM);

        imageNum = (gctUINT8) inst->source0Index;
        if (gcmSL_SOURCE_GET(inst->source1, Type) == gcSL_CONSTANT)
        {
            /* hard coded defined inside kernel:
               sampler_t s = CLK_SAMPLER_NORMALIZED|...;
               a =  read_imagef(img1, sampler1, tid);  // const propagation to sampler1
             */
            isConstantSamplerType = gcvTRUE;
            samplerType = (inst->source1Index & 0xFFFF) |
                          (inst->source1Indexed << 16);
        }
        else if (gcmSL_SOURCE_GET(inst->source1, Type) == gcSL_UNIFORM)
        {
            /* pass in as argument:
                 __kernel void
                 test_multireadimage(int n, int m, sampler_t sampler,
                                     read_only image2d_t img0, ...);
             */
            isConstantSamplerType = gcvFALSE;
            samplerType = inst->source1Index;
        }
        else
        {
            /* Not supported. */
            gcmASSERT(gcvFALSE);
            return gcvSTATUS_INVALID_DATA;
        }

        for (j = 0; j < imageSamplerCount; j++)
        {
            gcsIMAGE_SAMPLER_PTR imageSampler = &kernelFunction->imageSamplers[j];
            if ((imageSampler->imageNum == imageNum) &&
                (imageSampler->isConstantSamplerType == isConstantSamplerType) &&
                (imageSampler->samplerType == samplerType))
            {
                break;
            }
        }

        if (j == imageSamplerCount)
        {
            gctCHAR    symbol[256];
            gctSIZE_T   symbolLength = 256;
            gctUINT     offset = 0;
            gcUNIFORM   imageUniform;

            /* Add new uniform. */
            imageUniform = Shader->uniforms[inst->source0Index];
            gcmVERIFY_OK(gcoOS_PrintStrSafe(symbol,
                                            symbolLength,
                                            &offset,
                                            "sampler#%s#%d",
                                            imageUniform->name,
                                            imageSamplerCount));
            if (imageUniform->u.type == gcSHADER_IMAGE_3D)
            {
                gcmONERROR(gcSHADER_AddUniform(Shader, symbol, gcSHADER_SAMPLER_3D, 1, imageUniform->precision, &uniform));
            }
            else
            {
                gcmONERROR(gcSHADER_AddUniform(Shader, symbol, gcSHADER_SAMPLER_2D, 1, imageUniform->precision, &uniform));
            }
            gcmONERROR(gcUNIFORM_SetFlags(uniform, 0 /*gcvUNIFORM_KIND_KERNEL_ARG*/));
            gcmONERROR(gcUNIFORM_SetFormat(uniform, gcSL_UINT32, gcvFALSE));
            gcmASSERT(uniform->index == (origUniformCount + imageSamplerCount));
            uniform->imageSamplerIndex = (gctUINT16) imageSamplerCount;

            /* Add imageSampler. */
            gcmONERROR(gcKERNEL_FUNCTION_AddImageSampler(kernelFunction,
                                                         imageNum,
                                                         isConstantSamplerType,
                                                         samplerType));
            imageSamplerCount++;
            gcmASSERT(imageSamplerCount == kernelFunction->imageSamplerCount);
        }

        /* Change IMAGE_SAMPLER to NOP. */
        gcSL_SetInst2NOP(inst);

        /* Change IMAGE_RD to TEXLD. */
        gcmSL_OPCODE_UPDATE(instNext->opcode, Opcode, gcSL_TEXLD);
        instNext->source0Index = (gctUINT16) (origUniformCount + j);
        instNext->source0 = gcmSL_SOURCE_SET(instNext->source0, Swizzle, gcSL_SWIZZLE_XYZW);

        if (!isCodeChanged)
        {
            isCodeChanged = gcvTRUE;
        }
    }

    if (isCodeChanged)
    {
        gcmONERROR(gcSHADER_Pack(Shader));
        if (gcSHADER_DumpOptimizer(Shader))
        {
            gcOpt_Dump(gcvNULL, "Change image_rd to texld", gcvNULL, Shader);
        }
    }
    return status;

OnError:
    return status;
}

/* The output format conversion is done in recompilation */

static gctBOOL
_ToUploadUBO(
    IN gcSHADER VertexShader,
    IN gcSHADER FragmentShader,
    OUT gctBOOL *UploadUBO
)
{
    gctBOOL uploadUBO = gcvFALSE;

    if(gcmOPT_UploadUBO() &&
       VertexShader && FragmentShader) {
        gctUINT maxVertexUniforms;
        gctUINT maxFragmentUniforms;
        gctUINT uniformCount, uboUniformCount;

        gcmVERIFY_OK(gcoHAL_QueryShaderCaps(gcvNULL,
                                            gcvNULL,
                                            &maxVertexUniforms,
                                            &maxFragmentUniforms,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL));

        do {
            gcSHADER_GetUniformVectorCount(VertexShader,
                                           &uniformCount);

            gcSHADER_GetUniformVectorCountByCategory(VertexShader,
                                                     gcSHADER_VAR_CATEGORY_BLOCK_MEMBER,
                                                     &uboUniformCount);
            if(maxVertexUniforms < (uniformCount + uboUniformCount)) break;

            gcSHADER_GetUniformVectorCount(FragmentShader,
                                           &uniformCount);

            gcSHADER_GetUniformVectorCountByCategory(FragmentShader,
                                                     gcSHADER_VAR_CATEGORY_BLOCK_MEMBER,
                                                     &uboUniformCount);
            if(maxFragmentUniforms < (uniformCount + uboUniformCount)) break;

            uploadUBO = gcvTRUE;
        } while (gcvFALSE);
    }

    if(UploadUBO) {
        *UploadUBO = uploadUBO;
    }
    return uploadUBO;
}

static gceSTATUS
_ManageUniformMembersInUBO(
    IN gcSHADER Shader,
    IN gctUINT MaxUniforms,
    OUT gctUINT *UniformsUsed,
    OUT gctBOOL * IsUsedLoadInstruction
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT maxUniforms = 0;

    gcmASSERT(Shader);
    if(Shader) {
        gctUINT uboCount;
        gctUINT i, j;
        gcsUNIFORM_BLOCK ubo;
        gctUINT uniformCount;
        gcUNIFORM uniform;
        gctUINT uniformsUsed;
        gctBOOL countDefaultUBO = gcvFALSE;

        do {
            gcmONERROR(gcSHADER_GetUniformVectorCount(Shader,
                                                      &uniformsUsed));

            gcmONERROR(gcSHADER_GetUniformBlockCount(Shader,
                                                     &uboCount));

            uniformsUsed += uboCount; /* include base address uniform */

            /* Don't include default UBO base address uniform. */
            uniformsUsed--;

            for(i = 0; i < uboCount; i++) {
                gcmONERROR(gcSHADER_GetUniformBlock(Shader,
                                                    i,
                                                    &ubo));
                if(!ubo) {
                   uniformsUsed--; /* deduct the base address uniform */
                   continue;
                }

                ubo->_finished = gcvFALSE;
                gcmONERROR(gcSHADER_GetUniformBlockUniformCount(Shader, ubo, &uniformCount));

                for(j = 0; j < uniformCount; j++) {
                    gcmONERROR(gcSHADER_GetUniformBlockUniform(Shader, ubo, j, &uniform));
                    if(!uniform) continue;

                    ResetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
                    if(isUniformIndirectlyAddressed(uniform)) {
                        if(Shader->_defaultUniformBlockIndex == (gctINT)i) { /* Exclude #DefaultUBO for now */
                            gctUINT32 components = 0, rows = 0;

                            gcTYPE_GetTypeInfo(uniform->u.type, &components, &rows, 0);
                            rows *= uniform->arraySize;
                            uniformsUsed += rows;
                        }
                        else {
                            ubo->_useLoadInst = gcvTRUE;
                            SetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
                            *IsUsedLoadInstruction = gcvTRUE;
                            if (!countDefaultUBO && (Shader->_defaultUniformBlockIndex == (gctINT)i))
                            {
                                uniformsUsed++;
                                countDefaultUBO = gcvTRUE;
                            }
                        }
                    }
                }
            }

            if(MaxUniforms < (gctUINT)uniformsUsed) {
                status = gcvSTATUS_TOO_MANY_UNIFORMS;
                gcmONERROR(status);
            }
            else {
                maxUniforms = MaxUniforms - uniformsUsed;
            }

            /* One pass on the ubo's that do not need to use LOAD instruction */
            for(i = 0; i < uboCount; i++) {
                status = gcSHADER_GetUniformBlock(Shader,
                                                  i,
                                                  &ubo);
                if (gcmIS_ERROR(status)) return status;
                if(!ubo || ubo->_useLoadInst) continue;

                gcmONERROR(gcSHADER_GetUniformBlockUniformCount(Shader, ubo, &uniformCount));

                if(uniformCount) {
                    for(j = 0; j < uniformCount; j++) {
                        gctUINT32 components = 0, rows = 0;

                        gcmONERROR(gcSHADER_GetUniformBlockUniform(Shader, ubo, j, &uniform));
                        if(!uniform) continue;
                        gcmASSERT(!isUniformUsedInShader(uniform));

                        if(isUniformIndirectlyAddressed(uniform) &&
                           Shader->_defaultUniformBlockIndex == (gctINT)i) continue; /* already processed */

                        gcTYPE_GetTypeInfo(uniform->u.type, &components, &rows, 0);
                        rows *= uniform->arraySize;
                        if(maxUniforms < (gctSIZE_T)rows) {
                            SetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
                            ubo->_useLoadInst = gcvTRUE;
                            *IsUsedLoadInstruction = gcvTRUE;
                            if (!countDefaultUBO && (Shader->_defaultUniformBlockIndex == (gctINT)i))
                            {
                                uniformsUsed++;
                                countDefaultUBO = gcvTRUE;
                            }
                        }
                        else {
                            maxUniforms -= rows;
                        }
                    }
                }
                if(!ubo->_useLoadInst) {
                    maxUniforms++;
                }
                ubo->_finished = gcvTRUE;
            }

            /* another pass on ubo's not yet completely processed */
            for(i = 0; i < uboCount; i++) {
                status = gcSHADER_GetUniformBlock(Shader,
                                                  i,
                                                  &ubo);
                if (gcmIS_ERROR(status)) return status;
                if(!ubo || ubo->_finished) continue;

                gcmASSERT(ubo->_useLoadInst);
                gcmONERROR(gcSHADER_GetUniformBlockUniformCount(Shader, ubo, &uniformCount));

                for(j = 0; j < uniformCount; j++) {
                    gctUINT32 components = 0, rows = 0;

                    gcmONERROR(gcSHADER_GetUniformBlockUniform(Shader, ubo, j, &uniform));
                    if(!uniform ||
                       isUniformUsedInShader(uniform))continue;

                    if(isUniformIndirectlyAddressed(uniform) &&
                       Shader->_defaultUniformBlockIndex == (gctINT)i) continue; /* already processed */

                    gcTYPE_GetTypeInfo(uniform->u.type, &components, &rows, 0);
                    rows *= uniform->arraySize;
                    if(maxUniforms < (gctSIZE_T)rows) {
                        SetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
                        *IsUsedLoadInstruction = gcvTRUE;
                        if (!countDefaultUBO && (Shader->_defaultUniformBlockIndex == (gctINT)i))
                        {
                            uniformsUsed++;
                            countDefaultUBO = gcvTRUE;
                        }
                    }
                    else {
                        maxUniforms -= rows;
                    }
                }
                ubo->_finished = gcvTRUE;
            }

            if(maxUniforms) {
                /* final pass on ubo's to redistribute the available uniforms */
                for(i = 0; i < uboCount; i++) {
                    gctBOOL hasSkippedUniform = gcvFALSE;

                    status = gcSHADER_GetUniformBlock(Shader,
                                                      i,
                                                      &ubo);
                    if (gcmIS_ERROR(status)) return status;
                    if(!ubo || !ubo->_useLoadInst) continue;

                    gcmONERROR(gcSHADER_GetUniformBlockUniformCount(Shader, ubo, &uniformCount));

                    for(j = 0; j < uniformCount; j++) {
                        gctUINT32 components = 0, rows = 0;

                        gcmONERROR(gcSHADER_GetUniformBlockUniform(Shader, ubo, j, &uniform));
                        if(!uniform ||
                           !isUniformUsedInShader(uniform))continue;

                        if(isUniformIndirectlyAddressed(uniform)) {
                            if(Shader->_defaultUniformBlockIndex != (gctINT)i) {
                                hasSkippedUniform = gcvTRUE;
                            }
                            continue;
                        }

                        gcTYPE_GetTypeInfo(uniform->u.type, &components, &rows, 0);
                        rows *= uniform->arraySize;
                        if(maxUniforms >= (gctSIZE_T)rows) {
                            ResetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
                            maxUniforms -= rows;
                        }
                        else {
                            hasSkippedUniform = gcvTRUE;
                        }
                    }
                    if(!hasSkippedUniform) {
                        ubo->_useLoadInst = gcvFALSE;
                        maxUniforms++;
                    }
                    if(maxUniforms == 0) break;
                }
            }
        } while (gcvFALSE);
    }

    if(UniformsUsed) {
        *UniformsUsed = MaxUniforms - maxUniforms;
    }

OnError:
    return status;
}

#if !DX_SHADER
#if VSC_BUILD
gceSTATUS
gcLinkTree2VirShader(
    IN  gcLINKTREE              Tree,
    IN  VSC_PRIMARY_MEM_POOL*   Pmp,
    IN VSC_HW_CONFIG            *hwCfg,
    OUT VIR_Shader**            VirShader
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Tree=0x%x ",Tree);
    do
    {
        VIR_Shader *              virShader;
        VSC_ErrCode               errCode;
        gcLINKTREE                tree = Tree;
        gcSHADER                  shader = tree->shader;
        VIR_ShaderKind            shaderKind = VIR_SHADER_UNKNOWN;
        gctBOOL                   dumpCGV = gcSHADER_DumpCodeGenVerbose(shader);

        switch (shader->type) {
        case gcSHADER_TYPE_VERTEX:
            shaderKind = VIR_SHADER_VERTEX;
            break;
        case gcSHADER_TYPE_FRAGMENT:
            shaderKind = VIR_SHADER_FRAGMENT;
            break;
        case gcSHADER_TYPE_CL:
            shaderKind = VIR_SHADER_CL;
            break;
        case gcSHADER_TYPE_COMPUTE:
            shaderKind = VIR_SHADER_COMPUTE;
            break;
        case gcSHADER_TYPE_TCS:
            shaderKind = VIR_SHADER_TESSELLATION_CONTROL;
            break;
        case gcSHADER_TYPE_TES:
            shaderKind = VIR_SHADER_TESSELLATION_EVALUATION;
            break;
        case gcSHADER_TYPE_GEOMETRY:
            shaderKind = VIR_SHADER_GEOMETRY;
            break;
        default:
            gcmASSERT(gcvFALSE);
            break;
        }

        errCode = VIR_Shader_Construct(gcvNULL,
                                       shaderKind,
                                       &Pmp->mmWrapper,
                                       &virShader);
        if (VSC_ERR_NONE != errCode)
        {
            gcmASSERT(gcvFALSE);
        }

        gcmONERROR(gcSHADER_Conv2VIR(shader, hwCfg, virShader));

        if (dumpCGV)
        {
            VIR_Shader_Dump(gcvNULL, "Converted VIR shader IR.", virShader, gcvTRUE);
        }

        *VirShader = virShader;

    } while (gcvFALSE);

    /* Success. */
    gcmFOOTER_ARG("*Tree=0x%x", *Tree);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcVirShader2LinkTree(
    IN  VIR_Shader*              VirShader,
    OUT gcLINKTREE*              Tree
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("VirShader=0x%x ",VirShader);
    do
    {
        gcLINKTREE                tree = *Tree;
        gcSHADER                  shader = tree->shader;
        gceSHADER_FLAGS           flags  = tree->flags;
        gctBOOL                   dumpCGV = gcSHADER_DumpCodeGenVerbose(shader);

        /* Destroy old link tree. */
        gcLINKTREE_Destroy(tree);

        gcmONERROR(gcSHADER_ConvFromVIR(shader, VirShader, flags));
        if (dumpCGV)
        {
            gcDump_Shader(gcvNULL, "Converted gcSL shader IR (from VIR).", gcvNULL, shader, gcvTRUE);
        }

        /* Rebuild link tree by converted shader. */
        gcmERR_BREAK(gcLINKTREE_Construct((gcoOS)gcvNULL, Tree));

        tree = *Tree;
        gcmERR_BREAK(gcLINKTREE_Build(tree, shader, flags));

        if ((flags & gcvSHADER_DEAD_CODE) &&
            !gcShaderHwRegAllocated(shader))
        {
            gcmONERROR(gcLINKTREE_RemoveDeadCode(tree));
        }
        else
        {
            /* Mark all temps and attributes as used. */
            gcmONERROR(gcLINKTREE_MarkAllAsUsedwithRA(tree));
        }

        if ((flags & gcvSHADER_OPTIMIZER) &&
            !gcShaderHwRegAllocated(shader))
        {
            gcmONERROR(gcLINKTREE_Optimize(tree));
        }

        if (dumpCGV)
        {
            _DumpLinkTree("Converted gcSL shader link tree (from VIR)", tree, gcvFALSE);
        }
    } while (gcvFALSE);

    /* Success. */
    gcmFOOTER_ARG("*Tree=0x%x", *Tree);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#endif

#define USE_NEW_LINKER            1

gceSTATUS
gcLinkTreeThruVirShaders(
    IN  gcLINKTREE*             Trees[],
    IN  gctBOOL                 clKernel,
    IN  gceSHADER_FLAGS         Flags,
    IN  gctBOOL                 doPreVaryingPacking,
    OUT gctUINT32 *             StateBufferSize,
    OUT gctPOINTER *            StateBuffer,
    OUT gcsHINT_PTR *           Hints
    )
{
    gcLINKTREE*                 VsTree  = Trees[gceSGSK_VERTEX_SHADER];
    gcLINKTREE*                 TcsTree = Trees[gceSGSK_TC_SHADER];
    gcLINKTREE*                 TesTree = Trees[gceSGSK_TE_SHADER];
    gcLINKTREE*                 GeoTree = Trees[gceSGSK_GEOMETRY_SHADER];
    gcLINKTREE*                 FsTree  = Trees[gceSGSK_FRAGMENT_SHADER];
    gcLINKTREE*                 CsTree  = Trees[gceSGSK_COMPUTE_SHADER];
    gcLINKTREE                  linkTrees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gceSTATUS                   status = gcvSTATUS_OK;
    VSC_PRIMARY_MEM_POOL        pmp;
    gctUINT                     i;

#if VSC_BUILD
    gcmHEADER_ARG("Trees=0x%x ",Trees);
    do
    {
        VIR_Shader                     *vsVirShader = gcvNULL, *fsVirShader = gcvNULL, *csVirShader = gcvNULL,
                                       *tcsVirShader = gcvNULL, *tesVirShader = gcvNULL, *geoVirShader = gcvNULL;
        gcSHADER                       vsShader = gcvNULL, fsShader = gcvNULL, csShader =gcvNULL,
                                       tcsShader = gcvNULL, tesShader = gcvNULL, geoShader = gcvNULL;
        VSC_HW_CONFIG                  hwCfg;
        VSC_PROGRAM_LINKER_PARAM       pgComParam;
        VSC_SHADER_COMPILER_PARAM      scParam;
        gctUINT8 *                     stateBuffer = gcvNULL;
        gctBOOL                        dumpCGV = gcvFALSE;
        gceAPI                         clientAPI = gcvAPI_OPENGL_ES30;
        VSC_OPTN_SEPGenOptions*        sep_options = VSC_OPTN_Options_GetSEPGenOptions(VSC_OPTN_Get_Options());

        /* Initialize 512KB PMP for shader use. */
        vscPMP_Intialize(&pmp, gcvNULL, 512*1024, sizeof(void *), gcvTRUE /*pooling*/);

        gcmONERROR(gcQueryShaderCompilerHwCfg(gcvNULL, &hwCfg));

        if (VsTree && *VsTree)
        {
            gcmONERROR(gcLinkTree2VirShader(*VsTree, &pmp, &hwCfg, &vsVirShader));

            vsShader = (*VsTree)->shader;
            clientAPI = vsVirShader->clientApiVersion;
            dumpCGV = gcSHADER_DumpCodeGenVerbose(vsShader);
            VSC_OPTN_SEPGenOptions_SetTrace(sep_options, gcSHADER_DumpCodeGen((*VsTree)->shader));
        }

        if (TcsTree && *TcsTree)
        {
            gcmONERROR(gcLinkTree2VirShader(*TcsTree, &pmp, &hwCfg, &tcsVirShader));

            tcsShader = (*TcsTree)->shader;
            clientAPI = tcsVirShader->clientApiVersion;
            dumpCGV = gcSHADER_DumpCodeGenVerbose(tcsShader);
            VSC_OPTN_SEPGenOptions_SetTrace(sep_options, gcSHADER_DumpCodeGen((*TcsTree)->shader));
        }

        if (TesTree && *TesTree)
        {
            gcmONERROR(gcLinkTree2VirShader(*TesTree, &pmp, &hwCfg, &tesVirShader));

            tesShader = (*TesTree)->shader;
            clientAPI = tesVirShader->clientApiVersion;
            dumpCGV = gcSHADER_DumpCodeGenVerbose(tesShader);
            VSC_OPTN_SEPGenOptions_SetTrace(sep_options, gcSHADER_DumpCodeGen((*TesTree)->shader));
        }

        if (GeoTree && *GeoTree)
        {
            gcmONERROR(gcLinkTree2VirShader(*GeoTree, &pmp, &hwCfg, &geoVirShader));

            geoShader = (*GeoTree)->shader;
            clientAPI = geoVirShader->clientApiVersion;
            dumpCGV = gcSHADER_DumpCodeGenVerbose(geoShader);
            VSC_OPTN_SEPGenOptions_SetTrace(sep_options, gcSHADER_DumpCodeGen((*GeoTree)->shader));
        }

        if (FsTree && *FsTree)
        {
            gcmONERROR(gcLinkTree2VirShader(*FsTree, &pmp, &hwCfg, &fsVirShader));

            fsShader = (*FsTree)->shader;
            clientAPI = fsVirShader->clientApiVersion;
            dumpCGV = gcSHADER_DumpCodeGenVerbose(fsShader);
            VSC_OPTN_SEPGenOptions_SetTrace(sep_options, gcSHADER_DumpCodeGen((*FsTree)->shader));

            if (Flags & gcvSHADER_USE_ALPHA_KILL)
            {
                VIR_Shader_SetFlag(fsVirShader, VIR_SHFLAG_PS_NEED_ALPHA_KILL_PATCH);
            }
        }

        if (CsTree && *CsTree)
        {
            gcmONERROR(gcLinkTree2VirShader(*CsTree, &pmp, &hwCfg, &csVirShader));

            csShader = (*CsTree)->shader;
            clientAPI = csVirShader->clientApiVersion;
            dumpCGV = gcSHADER_DumpCodeGenVerbose(csShader);
            VSC_OPTN_SEPGenOptions_SetTrace(sep_options, gcSHADER_DumpCodeGen((*CsTree)->shader));
        }

        if (clKernel)
        {
            gcoOS_ZeroMemory(&scParam, sizeof(scParam));

            scParam.pShader = csVirShader;
            scParam.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS|
                                 VSC_COMPILER_FLAG_COMPILE_CODE_GEN;
            scParam.cfg.clientAPI = clientAPI;
            scParam.cfg.optFlags = VSC_COMPILER_OPT_FULL;
            scParam.cfg.pHwCfg = &hwCfg;
        }
        else
        {
            gcoOS_ZeroMemory(&pgComParam, sizeof(pgComParam));
            pgComParam.pShaderArray[VSC_SHADER_STAGE_VS] = vsVirShader;
            pgComParam.pShaderArray[VSC_SHADER_STAGE_HS] = tcsVirShader;
            pgComParam.pShaderArray[VSC_SHADER_STAGE_DS] = tesVirShader;
            pgComParam.pShaderArray[VSC_SHADER_STAGE_GS] = geoVirShader;
            pgComParam.pShaderArray[VSC_SHADER_STAGE_PS] = fsVirShader;
            pgComParam.pShaderArray[VSC_SHADER_STAGE_CS] = csVirShader;
            pgComParam.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS|
                                    VSC_COMPILER_FLAG_COMPILE_CODE_GEN;

            if (Flags & gcvSHADER_SEPERATED_PROGRAM)
            {
                pgComParam.cfg.cFlags |= VSC_COMPILER_FLAG_SEPERATED_SHADERS;
            }

            if (Flags & gcvSHADER_RECOMPILER)
            {
                pgComParam.cfg.cFlags |= VSC_COMPILER_FLAG_RECOMPILING;
            }

            /* Only enable this for new CG right now.*/
            if (hwCfg.hwFeatureFlags.samplerRegFileUnified &&
                StateBuffer)
            {
                pgComParam.cfg.cFlags |= VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC;
            }

            pgComParam.cfg.clientAPI = clientAPI;
            pgComParam.cfg.optFlags = VSC_COMPILER_OPT_FULL;
            if (Flags & gcvSHADER_DISABLE_DEFAULT_UBO)
            {
                pgComParam.cfg.optFlags &= ~VSC_COMPILER_OPT_CONSTANT_REG_SPILLABLE;
            }
            pgComParam.cfg.pHwCfg = &hwCfg;

            pgComParam.pGlApiCfg = gcGetGLSLCaps();
        }

        if (StateBuffer)
        {
            VSC_HW_PIPELINE_SHADERS_STATES hwPgStates = {0};

            if (clKernel)
            {
                gcmONERROR(vscCreateKernel(&scParam, gcvNULL, &hwPgStates));
            }
            else
            {
                gcmONERROR(vscLinkProgram(&pgComParam, gcvNULL, &hwPgStates));
            }

            if (VsTree && *VsTree)
            {
                hwPgStates.hints.vertexShaderId = VIR_Shader_GetId(vsVirShader);

                gcLINKTREE_Destroy(*VsTree);
                *VsTree = gcvNULL;

                gcmONERROR(gcSHADER_ConvFromVIR(vsShader, vsVirShader, Flags));
            }

            if (FsTree && *FsTree)
            {
                hwPgStates.hints.fragmentShaderId = VIR_Shader_GetId(fsVirShader);

                gcLINKTREE_Destroy(*FsTree);
                *FsTree = gcvNULL;

                gcmONERROR(gcSHADER_ConvFromVIR(fsShader, fsVirShader, Flags));
            }

            if (CsTree && *CsTree)
            {
                hwPgStates.hints.fragmentShaderId = VIR_Shader_GetId(csVirShader);

                gcLINKTREE_Destroy(*CsTree);
                *CsTree = gcvNULL;

                gcmONERROR(gcSHADER_ConvFromVIR(csShader, csVirShader, Flags));
            }

            if (TcsTree && *TcsTree)
            {
                hwPgStates.hints.tcsShaderId = VIR_Shader_GetId(tcsVirShader);

                gcLINKTREE_Destroy(*TcsTree);
                *TcsTree = gcvNULL;

                gcmONERROR(gcSHADER_ConvFromVIR(tcsShader, tcsVirShader, Flags));
            }

            if (TesTree && *TesTree)
            {
                hwPgStates.hints.tesShaderId = VIR_Shader_GetId(tesVirShader);

                gcLINKTREE_Destroy(*TesTree);
                *TesTree = gcvNULL;

                gcmONERROR(gcSHADER_ConvFromVIR(tesShader, tesVirShader, Flags));
            }

            if (GeoTree && *GeoTree)
            {
                hwPgStates.hints.gsShaderId = VIR_Shader_GetId(geoVirShader);

                gcLINKTREE_Destroy(*GeoTree);
                *GeoTree = gcvNULL;

                gcmONERROR(gcSHADER_ConvFromVIR(geoShader, geoVirShader, Flags));
            }

            gcmASSERT(hwPgStates.stateBufferSize != 0 && hwPgStates.pStateBuffer != gcvNULL);

            /* Allocate a new state buffer. */
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      *StateBufferSize + hwPgStates.stateBufferSize,
                                      (gctPOINTER*)&stateBuffer));

            /* Copy the old state buffer if there is any. */
            if (*StateBufferSize > 0)
            {
                gcmASSERT(*StateBuffer != gcvNULL);

                gcoOS_MemCopy(stateBuffer, *StateBuffer, *StateBufferSize);
            }

            gcoOS_MemCopy(stateBuffer + *StateBufferSize, hwPgStates.pStateBuffer, hwPgStates.stateBufferSize);

            /* Free any old state buffer. */
            if (*StateBufferSize > 0)
            {
                gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, *StateBuffer));
            }

            /* Set new state buffer. */
            *StateBuffer      = stateBuffer;
            *StateBufferSize += hwPgStates.stateBufferSize;
            stateBuffer       = gcvNULL;

            /* Allocate a new Hints. */
            gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(struct _gcsHINT), (gctPOINTER*)Hints));
            gcoOS_MemCopy(*Hints, &hwPgStates.hints, sizeof(struct _gcsHINT));

            /* Delete program states returned by compiler */
            gcmONERROR(gcmOS_SAFE_FREE(gcvNULL, hwPgStates.pStateBuffer));
            hwPgStates.stateBufferSize = 0;
        }
        else
        {
            VSC_OPTN_RAOptions*    ra_options = VSC_OPTN_Options_GetRAOptions(VSC_OPTN_Get_Options());
            VSC_OPTN_MCGenOptions* mcgen_options = VSC_OPTN_Options_GetMCGenOptions(VSC_OPTN_Get_Options());

            /* Force RA and MC-gen disabled as we dont need generate states */
            VSC_OPTN_RAOptions_SetSwitchOn(ra_options, gcvFALSE);
            VSC_OPTN_MCGenOptions_SetSwitchOn(mcgen_options, gcvFALSE);

            if (clKernel)
            {
                gcmONERROR(vscCreateKernel(&scParam, gcvNULL, gcvNULL));
            }
            else
            {
                gcmONERROR(vscLinkProgram(&pgComParam, gcvNULL, gcvNULL));
            }

            if (VsTree && *VsTree)
            {
                gcVirShader2LinkTree(vsVirShader, VsTree);
            }

            if (TcsTree && *TcsTree)
            {
                gcVirShader2LinkTree(tcsVirShader, TcsTree);
            }

            if (TesTree && *TesTree)
            {
                gcVirShader2LinkTree(tesVirShader, TesTree);
            }

            if (GeoTree && *GeoTree)
            {
                gcVirShader2LinkTree(geoVirShader, GeoTree);
            }

            if (FsTree && *FsTree)
            {
                gcVirShader2LinkTree(fsVirShader, FsTree);
            }

            if (CsTree && *CsTree)
            {
                gcVirShader2LinkTree(csVirShader, CsTree);
            }

            if (CsTree == gcvNULL || *CsTree == gcvNULL)
            {
                if ((Flags & gcvSHADER_SEPERATED_PROGRAM) == 0)
                {
                    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
                    {
                        if (Trees[i])
                        {
                            linkTrees[i] = *Trees[i];
                        }
                    }

                    /* Link vertex and fragment shaders, do varying packing this time. */
                    gcmERR_BREAK(gcLINKTREE_Link(linkTrees, !doPreVaryingPacking));

                    /* Remove unused attributes. */
                    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
                    {
                        if (linkTrees[i])
                        {
                            gcmERR_BREAK(gcLINKTREE_RemoveUnusedAttributes(linkTrees[i]));

                            if (dumpCGV)
                            {
                                _DumpLinkTree("Linked shader (after VIR pass)", linkTrees[i], gcvFALSE);
                            }
                        }
                    }
                }
            }
            else
            {
                if (dumpCGV)
                {
                    _DumpLinkTree("After VIR kernel tree.", *CsTree, gcvFALSE);
                }
            }
        }

        vscPMP_Finalize(&pmp);

    }while (gcvFALSE);

    /* Success. */
    gcmFOOTER_ARG("*Trees=0x%x", *Trees);
    return gcvSTATUS_OK;

OnError:
    vscPMP_Finalize(&pmp);
    /* Return the status. */
    gcmFOOTER();
#endif
    return status;
}

gceSTATUS
gcGoThroughVIRPass_Conv2VIR(
    IN gcLINKTREE *             Tree,
    IN VSC_PRIMARY_MEM_POOL*    pmp,
    OUT VIR_Shader **           Shader
    )
{
    gceSTATUS                 status = gcvSTATUS_OK;
    VIR_Shader *              virShader;
    VSC_ErrCode               errCode;
    gcLINKTREE                tree = *Tree;
    gcSHADER                  shader = tree->shader;

    VIR_ShaderKind            shaderKind = VIR_SHADER_UNKNOWN;

    gctBOOL                   dumpCGV = gcSHADER_DumpCodeGenVerbose(shader);

    gcmHEADER();

    switch (shader->type) {
    case gcSHADER_TYPE_VERTEX:
        shaderKind = VIR_SHADER_VERTEX;
        break;
    case gcSHADER_TYPE_FRAGMENT:
        shaderKind = VIR_SHADER_FRAGMENT;
        break;
    case gcSHADER_TYPE_CL:
        shaderKind = VIR_SHADER_CL;
        break;
    case gcSHADER_TYPE_COMPUTE:
        shaderKind = VIR_SHADER_COMPUTE;
        break;
    case gcSHADER_TYPE_TCS:
        shaderKind = VIR_SHADER_TESSELLATION_CONTROL;
        break;
    case gcSHADER_TYPE_TES:
        shaderKind = VIR_SHADER_TESSELLATION_EVALUATION;
        break;
    case gcSHADER_TYPE_GEOMETRY:
        shaderKind = VIR_SHADER_GEOMETRY;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* Initialize 512KB PMP for shader use. */
    vscPMP_Intialize(pmp, gcvNULL, 512*1024, sizeof(void *), gcvTRUE /*pooling*/);

    errCode = VIR_Shader_Construct(gcvNULL,
                                    shaderKind,
                                    &pmp->mmWrapper,
                                    &virShader);
    if (VSC_ERR_NONE != errCode)
    {
        gcmASSERT(gcvFALSE);
    }

    gcmONERROR(gcSHADER_Conv2VIR(shader, gcvNULL, virShader));

    if (dumpCGV)
    {
        VIR_Shader_Dump(gcvNULL, "Converted VIR shader IR.", virShader, gcvTRUE);
    }

    *Shader = virShader;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcGoThroughVIRPass_Compile(
    IN OUT gcLINKTREE *     Tree,
    IN OUT VIR_Shader *     Shader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    VSC_HW_CONFIG             hwCfg;
    VSC_SHADER_COMPILER_PARAM shComParam;

    /* A temp VIR path for current ugent request */
    gcQueryShaderCompilerHwCfg(gcvNULL, &hwCfg);
    shComParam.pShader = Shader;
    shComParam.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS|
                            VSC_COMPILER_FLAG_COMPILE_CODE_GEN;
    shComParam.cfg.clientAPI = Shader->clientApiVersion;
    shComParam.cfg.optFlags = VSC_COMPILER_OPT_FULL;
    shComParam.cfg.pHwCfg = &hwCfg;
    status = vscCompileShader(&shComParam, gcvNULL);

    return status;
}

gceSTATUS
gcGoThroughVIRPass_NewTree(
    IN OUT gcLINKTREE *         Tree,
    IN VSC_PRIMARY_MEM_POOL*    pmp,
    IN VIR_Shader *             Shader
    )
{
    gceSTATUS                 status = gcvSTATUS_OK;
    gcLINKTREE                tree = *Tree;
    gcSHADER                  shader = tree->shader;
    gceSHADER_FLAGS           flags = tree->flags;
    gctBOOL                   dumpCGV = gcSHADER_DumpCodeGenVerbose(shader);

    gcmHEADER();
    /* Destroy old link tree. */
    gcLINKTREE_Destroy(tree);
    gcmONERROR(gcSHADER_ConvFromVIR(shader, Shader, flags));
    if (dumpCGV)
    {
        gcDump_Shader(gcvNULL, "Converted gcSL shader IR (from VIR).", gcvNULL, shader, gcvTRUE);
    }

    vscPMP_Finalize(pmp);

    /* Rebuild link tree by converted shader. */
    gcmONERROR(gcLINKTREE_Construct((gcoOS)gcvNULL, Tree));

    tree = *Tree;
    gcmONERROR(gcLINKTREE_Build(tree, shader, flags));
    if(shader->type & gcSHADER_TYPE_VERTEX)
    {
        gcLINKTREE_FindModelViewProjection(tree);
    }

    if ((flags & gcvSHADER_DEAD_CODE) &&
        !gcShaderHwRegAllocated(shader))
    {
        gcmONERROR(gcLINKTREE_RemoveDeadCode(tree));
    }
    else
    {
        /* Mark all temps and attributes as used. */
        gcmONERROR(gcLINKTREE_MarkAllAsUsedwithRA(tree));
    }

    if ((flags & gcvSHADER_OPTIMIZER) &&
        !gcShaderHwRegAllocated(shader))
    {
        gcmONERROR(gcLINKTREE_Optimize(tree));
    }

    if (dumpCGV)
    {
        _DumpLinkTree("Converted gcSL shader link tree (from VIR)", tree, gcvFALSE);
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcGoThroughVIRPass(
    IN OUT gcLINKTREE *     Tree
    )
{
    gceSTATUS status = gcvSTATUS_OK;
#if VSC_BUILD
    gcmHEADER_ARG("Tree=0x%x ",*Tree);
    do
    {
        VIR_Shader *              virShader;
        VSC_PRIMARY_MEM_POOL      pmp;

        gcmONERROR(gcGoThroughVIRPass_Conv2VIR(Tree, &pmp, &virShader));
        gcmONERROR(gcGoThroughVIRPass_Compile(Tree, virShader));
        gcmONERROR(gcGoThroughVIRPass_NewTree(Tree, &pmp, virShader));
    } while (gcvFALSE);
    /* Success. */
    gcmFOOTER_ARG("*Tree=0x%x", *Tree);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
#endif
    return status;
}
#endif  /* !DX_SHADER */

/******************************************************************************\
|********************************* Linker Code ********************************|
\******************************************************************************/

static gctINT32
_GetDataTypeByteOffset(
    IN  gctINT32 BaseOffset,
    IN  gcSHADER_TYPE DataType,
    IN  gceINTERFACE_BLOCK_LAYOUT_ID MemoryLayout,
    IN  gctBOOL IsArray,
    OUT gctINT16 *MatrixStride,
    OUT gctINT32 *ArrayStride,
    OUT gctINT16 *Alignment
)
{
    gctINT32 size = 0;
    gctINT16 matrixStride = 0;
    gctINT16 alignment = 0;

    gctBOOL std140 = (MemoryLayout & gcvINTERFACE_BLOCK_STD140) != 0;
    gctBOOL packed = (MemoryLayout & gcvINTERFACE_BLOCK_PACKED) != 0;

    switch (DataType)
    {
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_FLOAT_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_UINT_X1:
    case gcSHADER_ATOMIC_UINT:
        if (IsArray && std140) {
            alignment = 16;
            size = 16;
        }
        else {
            alignment = 4;
            size = 4;
        }
        break;

    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_FLOAT_X2:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_UINT_X2:
        if (IsArray && std140) {
            alignment = 16;
            size = 16;
        }
        else {
            alignment = 8;
            size = 8;
        }
        break;

    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_UINT_X3:
        alignment = 16;
        if (packed) {
            size = 12;
        }
        else {
            size = 16;
        }
        break;

    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_FLOAT_X4:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_UINT_X4:
        alignment = 16;
        size = 16;
        break;

    case gcSHADER_FLOAT_2X2:
        if (std140) {
            alignment = 16;
            size = 16 * 2;
        }
        else {
            alignment = 8;
            size = 8 * 2;
        }
        matrixStride = alignment;
        break;

    case gcSHADER_FLOAT_2X3:
        if (std140) {
            alignment = 16;
            size = 16 * 2;
        }
        else if (packed) {
            alignment = 12;
            size = 12 * 2;
        }
        else {
            alignment = 16;
            size = 16 * 2;
        }
        matrixStride = alignment;
        break;

    case gcSHADER_FLOAT_2X4:
        if (std140) {
            alignment = 16;
            size = 16 * 2;
        }
        else {
            alignment = 16;
            size = 16 * 2;
        }
        matrixStride = alignment;
        break;

    case gcSHADER_FLOAT_3X2:
        if (std140) {
            alignment = 16;
            size = 16 * 3;
        }
        else {
            alignment = 8;
            size = 16 * 3;
        }
        matrixStride = alignment;
        break;

    case gcSHADER_FLOAT_3X3:
        if (std140) {
            alignment = 16;
            size = 16 * 3;
        }
        else if (packed) {
             alignment = 12;
             size = 12 * 3;
        }
        else {
            alignment = 16;
            size = 16 * 3;
        }
        matrixStride = alignment;
        break;

    case gcSHADER_FLOAT_3X4:
        if (std140) {
            alignment = 16;
            size = 16 * 3;
        }
        else {
            alignment = 16;
            size = 16 * 3;
        }
        matrixStride = alignment;
        break;

    case gcSHADER_FLOAT_4X2:
        if (std140) {
            alignment = 16;
            size = 16 * 4;
        }
        else {
            alignment = 8;
            size = 8 * 4;
        }
        matrixStride = alignment;
        break;

    case gcSHADER_FLOAT_4X3:
        if (std140) {
            alignment = 16;
            size = 16 * 4;
        }
        else if (packed) {
            alignment = 12;
            size = 12 * 4;
        }
        else {
            alignment = 16;
            size = 16 * 4;
        }
        matrixStride = alignment;
        break;

    case gcSHADER_FLOAT_4X4:
        alignment = 16;
        size = 16 * 4;
        matrixStride = alignment;
        break;

    default:
        alignment = 0;
        size = 0;
        matrixStride = 0;
     }

    if (MatrixStride) *MatrixStride = matrixStride;
    if (ArrayStride)  *ArrayStride  = size;
    if (Alignment)    *Alignment    = alignment;

    return  packed ?  BaseOffset : gcmALIGN(BaseOffset, alignment);
}


static gceSTATUS
_GetBaseAlignmentForStruct(
    IN gcSHADER Shader,
    IN gceINTERFACE_BLOCK_LAYOUT_ID MemoryLayout,
    IN gcUNIFORM Uniform,
    OUT gctINT16 *StructAlignment
    )
{
    gceSTATUS status;
    gcUNIFORM uniform;
    gctINT16 alignment = 0;

    uniform = Uniform;
    while (uniform)
    {
        gctINT16 childAlignment = 0;

        if(isUniformStruct(uniform))
        {
            if(uniform->firstChild != -1) {
                gcUNIFORM structMember;

                status = gcSHADER_GetUniform(Shader,
                                             uniform->firstChild,
                                             &structMember);
                if (gcmIS_ERROR(status)) return status;

                _GetBaseAlignmentForStruct(Shader,
                                           MemoryLayout,
                                           structMember,
                                           &childAlignment);
            }
        }
        else
        {
            _GetDataTypeByteOffset(0,
                                   uniform->u.type,
                                   MemoryLayout,
                                   isUniformArray(uniform),
                                   gcvNULL,
                                   gcvNULL,
                                   &childAlignment);
        }

        if (childAlignment > alignment)
            alignment = childAlignment;

        if(uniform->nextSibling != -1) {
            status = gcSHADER_GetUniform(Shader,
                                         uniform->nextSibling,
                                         &uniform);
            if (gcmIS_ERROR(status)) return status;
        }
        else break;
    }

    if (StructAlignment)
        *StructAlignment = alignment;
    return gcvSTATUS_OK;
}

gceSTATUS
_ConvUniformToUniformBlockMember(
    IN gcSHADER prevShader,
    IN gcsUNIFORM_BLOCK prevUB,
    IN gcSHADER Shader,
    IN gcsUNIFORM_BLOCK UniformBlock,
    IN gcUNIFORM Uniform,
    IN gctINT32 *Offset
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcUNIFORM uniform = Uniform;
    gctINT32 offset = *Offset;
    gctINT16 alignment;

    while (uniform)
    {
        if(isUniformStruct(uniform))
        {
            if (uniform->firstChild != -1)
            {
                gcUNIFORM structMember;

                gcmONERROR(gcSHADER_GetUniform(Shader,
                                               uniform->firstChild,
                                               &structMember));

                gcmONERROR(_GetBaseAlignmentForStruct(Shader,
                                                      GetUBMemoryLayout(UniformBlock),
                                                      structMember,
                                                      &alignment));
                offset = gcmALIGN(offset, alignment);

                gcmONERROR(_ConvUniformToUniformBlockMember(prevShader,
                                                            prevUB,
                                                            Shader,
                                                            UniformBlock,
                                                            structMember,
                                                            &offset));
                offset = gcmALIGN(offset, 16);
                uniform->blockIndex = GetUBBlockIndex(UniformBlock);
            }
        }
        else if (gcmType_Kind(uniform->u.type) != gceTK_SAMPLER)
        {
            gctUINT32 idx;
            gctUINT32 prevUBCount = 0;
            gcUNIFORM matchedUniform = gcvNULL;

            SetUniformCategory(uniform, gcSHADER_VAR_CATEGORY_BLOCK_MEMBER);
            uniform->blockIndex = GetUBBlockIndex(UniformBlock);
            uniform->isRowMajor = gcvFALSE;

            if (prevShader && prevUB)
            {
                /* If VS defined same uniform, keep to use offset gotten from VS. */
                gcmONERROR(gcSHADER_GetUniformBlockUniformCount(prevShader, prevUB, &prevUBCount));
                for (idx = 0; idx < prevUBCount; ++idx)
                {
                    gcUNIFORM prevUniform = gcvNULL;
                    gcmONERROR(gcSHADER_GetUniformBlockUniform(prevShader, prevUB, idx, &prevUniform));
                    if (prevUniform &&
                        gcmIS_SUCCESS(gcoOS_StrCmp(prevUniform->name, uniform->name)))
                    {
                        if (prevUniform->u.type != uniform->u.type)
                        {
                            status = gcvSTATUS_UNIFORM_MISMATCH;
                            break;
                        }
                        matchedUniform = prevUniform;
                    }
                }
            }

            if (status != gcvSTATUS_OK)
            {
                /* bail out if error */
                break;
            }
            if (matchedUniform)
            {
                uniform->offset       = matchedUniform->offset;
                uniform->arrayStride  = matchedUniform->arrayStride;
                uniform->matrixStride = matchedUniform->matrixStride;
            }
            else
            {
                gctINT16 matrixStride;
                gctINT32 arrayStride;
                gctBOOL isArray = isUniformArray(uniform);
                gctINT arrayLength = 1, i;

                uniform->offset =  _GetDataTypeByteOffset(offset,
                                                          uniform->u.type,
                                                          GetUBMemoryLayout(UniformBlock),
                                                          isArray,
                                                          &matrixStride,
                                                          &arrayStride,
                                                          &alignment);
                uniform->arrayStride  = isArray ? arrayStride : 0;
                uniform->matrixStride = matrixStride;

                for (i = 0; i < uniform->arrayLengthCount; i++)
                {
                    arrayLength *= uniform->arrayLengthList[i];
                }
                offset = uniform->offset + arrayStride * arrayLength;
            }
        }

        if (uniform->nextSibling == -1) break;

        gcmONERROR(gcSHADER_GetUniform(Shader,
                                       uniform->nextSibling,
                                       &uniform));
    }

    *Offset = offset;

OnError:
    return status;
}

gceSTATUS
gcInsertUniformToUB(
    gcSHADER Shader,
    gcsUNIFORM_BLOCK UniformBlock,
    gcUNIFORM Uniform
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Shader=0x%x UniformBlock=0x%x Uniform=0x%x", Shader, UniformBlock, Uniform);

    gcmVERIFY_OBJECT(Shader, gcvOBJ_SHADER);
    gcmVERIFY_OBJECT(UniformBlock, gcvOBJ_UNIFORM_BLOCK);
    gcmVERIFY_OBJECT(Uniform, gcvOBJ_UNIFORM);

    if (GetUBFirstChild(UniformBlock) == -1)
    {
        GetUBFirstChild(UniformBlock) = Uniform->index;
        Uniform->prevSibling = -1;
    }
    else
    {
        gcUNIFORM current = gcvNULL, prev = gcvNULL;

        gcmONERROR(gcSHADER_GetUniform(Shader, GetUBFirstChild(UniformBlock), &current));
        while (current)
        {
            /* Insert uniform here in the offset order */
            if (Uniform->offset < current->offset)
            {
                Uniform->prevSibling = current->prevSibling;
                Uniform->nextSibling = current->index;
                current->prevSibling = Uniform->index;

                if (prev)
                {
                    prev->nextSibling = Uniform->index;
                }
                else
                {
                    gcmASSERT(GetUBFirstChild(UniformBlock) == current->index);
                    SetUBFirstChild(UniformBlock, Uniform->index);
                }

                break;
            }

            /* Append to the tail */
            if (current->nextSibling == -1)
            {
                current->nextSibling = Uniform->index;
                Uniform->prevSibling = current->index;
                break;
            }

            prev = current;
            gcmONERROR(gcSHADER_GetUniform(Shader, current->nextSibling, &current));
        }
    }
    GetUBNumBlockElement(UniformBlock)++;

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
_gcCreateDefaultUBO(
    IN gcSHADER VertexShader,
    IN gcSHADER FragmentShader
    )
{
    gctINT32 offset = 0;
    gcsSHADER_VAR_INFO blockInfo[1];
    gcsUNIFORM_BLOCK vertUB = gcvNULL;
    gcsUNIFORM_BLOCK fragUB = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("VertexShader=0x%x FragmentShader=0x%x", VertexShader, FragmentShader);

    gcoOS_ZeroMemory(blockInfo, sizeof(blockInfo[0]));
    blockInfo->varCategory = gcSHADER_VAR_CATEGORY_BLOCK;
    blockInfo->format = gcSL_FLOAT;
    blockInfo->precision = gcSHADER_PRECISION_DEFAULT;
    blockInfo->arraySize = 1;
    blockInfo->u.numBlockElement = 0;
    blockInfo->prevSibling = -1;
    blockInfo->nextSibling = -1;
    blockInfo->parent = -1;

    if (VertexShader && VertexShader->_defaultUniformBlockIndex == -1)
    {
        gctUINT i;
        gcUNIFORM prevSiblingUniform = gcvNULL;

        for (i = 0; i < VertexShader->uniformCount; i++)
        {
            gcUNIFORM uniform = VertexShader->uniforms[i];

            if (uniform == gcvNULL ||
                isUniformLoadtimeConstant(uniform) ||
                isUniformCompiletimeInitialized(uniform) ||
                isUniformBlockAddress(uniform) ||
                !isUniformShaderDefined(uniform) ||
                uniform->blockIndex != -1 ||
                uniform->parent != -1 ||
                (isUniformNormal(uniform) &&
                (gcmType_Kind(uniform->u.type) == gceTK_SAMPLER ||
                 gcmType_Kind(uniform->u.type) == gceTK_IMAGE ||
                 gcmType_Kind(uniform->u.type) == gceTK_ATOMIC)))
            {
                continue;
            }

            /* if the alignment of a struct uniform equals 0, it means the uniform does not take up memory, then we
               can skip it for default ubo. for example:
               struct structType
               {
                   mediump sampler2D m0;
                   mediump samplerCube m1;
               }
               uniform structType u_var;
               ideally, we need a function to compute the size of a struct uniform, but a function computing struct
               alignment is good enough */
            if(isUniformStruct(uniform))
            {
                gctINT16 structAlignment;
                _GetBaseAlignmentForStruct(VertexShader, gcvINTERFACE_BLOCK_NONE, uniform, &structAlignment);
                if (structAlignment == 0)
                {
                    continue;
                }
            }

            if (vertUB == gcvNULL)
            {
                gcmONERROR(gcSHADER_AddUniformBlock(VertexShader,
                                                    "#DefaultUBO",
                                                    blockInfo,
                                                    gcvINTERFACE_BLOCK_PACKED,
                                                    -1,
                                                    0,
                                                    &vertUB));

                uniform->prevSibling = -1;
                SetUBFirstChild(vertUB, uniform->index);
                prevSiblingUniform = uniform;
            }

            gcmONERROR(_ConvUniformToUniformBlockMember(gcvNULL,
                                                        gcvNULL,
                                                        VertexShader,
                                                        vertUB,
                                                        uniform,
                                                        &offset));

            if (GetUBNumBlockElement(vertUB)++)
            {
                gcmASSERT(prevSiblingUniform);
                uniform->prevSibling = prevSiblingUniform->index;
                prevSiblingUniform->nextSibling = uniform->index;
                prevSiblingUniform = uniform;
            }
        }
    }

    if (FragmentShader && FragmentShader->_defaultUniformBlockIndex == -1)
    {
        gctUINT i;

        for (i = 0; i < FragmentShader->uniformCount; i++)
        {
            gcUNIFORM uniform = FragmentShader->uniforms[i];

            if (uniform == gcvNULL ||
                isUniformLoadtimeConstant(uniform) ||
                isUniformCompiletimeInitialized(uniform) ||
                isUniformBlockAddress(uniform) ||
                !isUniformShaderDefined(uniform) ||
                uniform->blockIndex != -1 ||
                uniform->parent != -1 ||
                (isUniformNormal(uniform) &&
                (gcmType_Kind(uniform->u.type) == gceTK_SAMPLER ||
                 gcmType_Kind(uniform->u.type) == gceTK_IMAGE||
                 gcmType_Kind(uniform->u.type) == gceTK_ATOMIC)))
            {
                continue;
            }

            /* if the alignment of a struct uniform equals 0, it means the uniform does not take up memory, then we
               can skip it for default ubo. for example:
               struct structType
               {
                   mediump sampler2D m0;
                   mediump samplerCube m1;
               }
               uniform structType u_var;
               ideally, we need a function to compute the size of a struct uniform, but a function computing struct
               alignment is good enough */
            if(isUniformStruct(uniform))
            {
                gctINT16 structAlignment;
                _GetBaseAlignmentForStruct(FragmentShader, gcvINTERFACE_BLOCK_NONE, uniform, &structAlignment);
                if (structAlignment == 0)
                {
                    continue;
                }
            }

            if (fragUB == gcvNULL)
            {
                gcmONERROR(gcSHADER_AddUniformBlock(FragmentShader,
                                                    "#DefaultUBO",
                                                    blockInfo,
                                                    gcvINTERFACE_BLOCK_PACKED,
                                                    -1,
                                                    0,
                                                    &fragUB));
            }

            gcmONERROR(_ConvUniformToUniformBlockMember(VertexShader,
                                                        vertUB,
                                                        FragmentShader,
                                                        fragUB,
                                                        uniform,
                                                        &offset));

            gcmONERROR(gcInsertUniformToUB(FragmentShader, fragUB, uniform));
        }
    }

    if (vertUB)
    {
        gcmASSERT(offset > 0);
        SetUBBlockSize(vertUB, offset);
    }

    if (fragUB)
    {
        gcmASSERT(offset > 0);
        SetUBBlockSize(fragUB, offset);
    }

OnError:
    gcmFOOTER();
    return status;
}


static gceSTATUS
_gcCreateConstantUBO(
    IN gcSHADER Shader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSHADER_VAR_INFO blockInfo[1];
    gctUINT maxShaderUniforms, vsUniform, psUniform;
    gctUINT32 curUsedUniform     = 0;
    gcsUNIFORM_BLOCK constUBO;
    gcUNIFORM uniform;

    gcmHEADER_ARG("Shader=0x%x", Shader);

    gcmASSERT(Shader);

    gcSHADER_GetUniformVectorCount(Shader, &curUsedUniform);

    gcmONERROR(gcoHAL_QueryShaderCaps(
                   gcvNULL,
                   gcvNULL,
                   &vsUniform,
                   &psUniform,
                   gcvNULL,
                   gcvNULL,
                   gcvNULL,
                   gcvNULL,
                   gcvNULL));

    maxShaderUniforms = (Shader->type == gcSHADER_TYPE_VERTEX)
                        ? vsUniform
                        : psUniform;

    /* If there is not uniform memory left, skip this. */
    if (curUsedUniform >= maxShaderUniforms)
    {
        return gcvSTATUS_OK;
    }

    if(Shader->constUniformBlockIndex != -1) return gcvSTATUS_OK;

    blockInfo->varCategory = gcSHADER_VAR_CATEGORY_BLOCK;
    blockInfo->format = gcSL_FLOAT;
    blockInfo->precision = gcSHADER_PRECISION_DEFAULT;
    blockInfo->arraySize = 1;
    blockInfo->u.numBlockElement = 0;
    blockInfo->prevSibling = -1;
    blockInfo->nextSibling = -1;
    blockInfo->parent = -1;

    gcmONERROR(gcSHADER_AddUniformBlock(Shader,
                                      Shader->type == gcSHADER_TYPE_VERTEX ?
                                            "#ConstantUBO_Vertex" :
                                            Shader->type == gcSHADER_TYPE_FRAGMENT ? "#ConstantUBO_Fragment" : "#ConstantUBO_Compute",
                                      blockInfo,
                                      gcvINTERFACE_BLOCK_SHARED,
                                      -1,
                                      0,
                                      &constUBO));

    gcmONERROR(gcSHADER_GetUniform(Shader,
                                   GetUBIndex(constUBO),
                                   &uniform));

    SetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_IN_SHADER);

    gcmFOOTER();
    return status;
OnError:
    gcmFOOTER();
    return status;
}

static gctBOOL
_IsMatrixDataType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X4:
    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_FLOAT_4X4:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

static gctUINT
_GetMatrixDataTypeColumnCount(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
        return 2;

    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X4:
        return 3;

    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_FLOAT_4X4:
        return 4;

    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
        return 1;

    default:
        gcmASSERT(0);
        return 4;
    }
}

static gceSTATUS
_GetIndexedRegAndModeForLoadInstruction(
    IN gcSHADER Shader,
    IN gctUINT CodeIndex,
    IN gcUNIFORM Uniform,
    OUT gctUINT16 * IndexedRegIndex,
    OUT gctUINT16 * ConstIndex,
    OUT gcSL_INDEXED * IndexedMode,
    OUT gcSL_SWIZZLE * Swizzle,
    OUT gctBOOL * InsertNewCode
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT16 indexedRegIndex = 0, constIndex = 0;
    gcSL_INDEXED indexedMode = gcSL_NOT_INDEXED;
    gcSL_INSTRUCTION code, Code = &Shader->code[CodeIndex];
    gcSL_SWIZZLE swizzle = gcSL_SWIZZLE_XYZW;
    gctINT offset = 0;
    gctUINT16 *offsetPtr = gcvNULL;
    gctBOOL swizzleChanged = gcvFALSE;

    /* If this uniform is not a matrix, the array index reg is holding by the previous instruction of uniform offset.*/
    if (!_IsMatrixDataType(Uniform->u.type))
    {
        code = Code - 2;
        gcmASSERT(gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MUL);

        if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP)
        {
            swizzle = gcmSL_SOURCE_GET(code->source0, SwizzleX);
            gcmASSERT(swizzle <= gcSL_SWIZZLE_W);

            indexedRegIndex = code->source0Index;
            indexedMode = (gcSL_INDEXED)(swizzle + 1);
        }
        else
        {
            gcSL_INSTRUCTION insertCode;
            gctUINT16 newTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_INTEGER_X1);

            gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, CodeIndex, 1));
            insertCode = &Shader->code[CodeIndex];
            code = insertCode - 2;
            insertCode->opcode = gcSL_MOV;
            insertCode->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                             | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                             | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                             | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_MEDIUM);
            insertCode->tempIndex = newTempIndex;
            insertCode->source0 = code->source0;
            insertCode->source0Index = code->source0Index;
            insertCode->source0Indexed = code->source0Indexed;

            indexedRegIndex = newTempIndex;
            indexedMode = gcSL_INDEXED_X;

            if (InsertNewCode)
            {
                *InsertNewCode = gcvTRUE;
            }
        }
    }
    else
    {
        gcSL_INSTRUCTION arrayIndexCode, matrixIndexCode;
        gcSL_TYPE arrayIndexType, matrixIndexType;

        for (code = Code; code != gcvNULL; code--)
        {
            if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_ADD &&
                gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP &&
                gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP)
            {
                break;
            }
        }

        gcmASSERT(code != gcvNULL);

        arrayIndexCode = code - 1;
        matrixIndexCode = code - 2;

        gcmASSERT(arrayIndexCode && matrixIndexCode &&
                  gcmSL_OPCODE_GET(arrayIndexCode->opcode, Opcode) == gcSL_MUL &&
                  gcmSL_OPCODE_GET(matrixIndexCode->opcode, Opcode) == gcSL_MUL);

        arrayIndexType = gcmSL_SOURCE_GET(arrayIndexCode->source0, Type);
        matrixIndexType = gcmSL_SOURCE_GET(matrixIndexCode->source0, Type);

        /* Both array and matrix is indexed. */
        if (arrayIndexType == gcSL_TEMP && matrixIndexType == gcSL_TEMP)
        {
            matrixIndexCode--;
            swizzle = gcmSL_SOURCE_GET(matrixIndexCode->source0, SwizzleX);
            gcmASSERT(swizzle <= gcSL_SWIZZLE_W);

            indexedRegIndex = matrixIndexCode->source0Index;
            indexedMode = (gcSL_INDEXED)(swizzle + 1);
        }
        /* Only matrix is indexed. */
        else if (arrayIndexType == gcSL_CONSTANT)
        {
            gcmASSERT(matrixIndexType == gcSL_TEMP);

            offsetPtr = (gctUINT16 *) (&offset);
            offsetPtr[0] = arrayIndexCode->source0Index;
            offsetPtr[1] = arrayIndexCode->source0Indexed;

            offset *= _GetMatrixDataTypeColumnCount(Uniform->u.type);
            constIndex = (gctUINT16)offset;

            swizzle = gcmSL_SOURCE_GET(matrixIndexCode->source0, SwizzleX);
            gcmASSERT(swizzle <= gcSL_SWIZZLE_W);

            indexedRegIndex = matrixIndexCode->source0Index;
            indexedMode = (gcSL_INDEXED)(swizzle + 1);
        }
        else
        {
            gcmASSERT(arrayIndexType == gcSL_TEMP && matrixIndexType == gcSL_CONSTANT);

            offsetPtr = (gctUINT16 *) (&offset);
            offsetPtr[0] = matrixIndexCode->source0Index;
            offsetPtr[1] = matrixIndexCode->source0Indexed;

            constIndex = (gctUINT16)offset;

            swizzle = gcmSL_SOURCE_GET(arrayIndexCode->source0, SwizzleX);
            gcmASSERT(swizzle <= gcSL_SWIZZLE_W);

            indexedRegIndex = arrayIndexCode->source0Index;
            indexedMode = (gcSL_INDEXED)(swizzle + 1);
        }

        /*
        ** If this matrix is row_major, this constant is the swizzle.
        ** Otherwise, this matrix is split to vectors, and this constant is the vector index.
        */
        code = Code - 2;
        if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_CONSTANT)
        {
            offsetPtr = (gctUINT16 *) (&offset);
            offsetPtr[0] = code->source1Index;
            offsetPtr[1] = code->source1Indexed;

            offset /= Uniform->matrixStride;

            if (Uniform->isRowMajor)
            {
                swizzleChanged = gcvTRUE;
                swizzle = gcmComposeSwizzle(offset, offset, offset, offset);
            }
            else
            {
                constIndex += (gctUINT16)offset;
            }
        }
    }

    if (IndexedRegIndex)
    {
        *IndexedRegIndex = indexedRegIndex;
    }

    if (ConstIndex)
    {
        *ConstIndex = constIndex;
    }

    if (IndexedMode)
    {
        *IndexedMode = indexedMode;
    }

    if (swizzleChanged && Swizzle)
    {
        *Swizzle = swizzle;
    }

OnError:
    return status;
}

static gceSTATUS
_gcChangeLoadToMovUniform(
    IN gcSHADER Shader,
    IN gctBOOL IsDefaultUBO
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER shader = Shader;

    if (IsDefaultUBO && shader->_defaultUniformBlockIndex == -1)
        return status;

    if (shader && shader->uniformBlockCount)
    {
       gcUNIFORM blockUniform, blockUniformMember;
       gctINT offset[1];
       gctUINT16 *offsetPtr;
       gctUINT16 regIndex;
       gctINT startChannel;
       gctUINT curInstIdx;
       gcSL_INSTRUCTION code, prevCode;
       gctBOOL isIndexedLoad = gcvFALSE;

       /* Walk all instructions to change load to mov. */
       for (curInstIdx = 0; curInstIdx < shader->codeCount; curInstIdx++)
       {
           gctUINT uboIndex = 0;
           gctBOOL insertNewCode = gcvFALSE;

           /* Get instruction. */
           code = &shader->code[curInstIdx];

           /* Determine temporary register usage. */
           if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_LOAD ||
               gcmSL_SOURCE_GET(code->source0, Type) != gcSL_UNIFORM) continue;

           if (gcmSL_SOURCE_GET(code->source1, Type) != gcSL_CONSTANT)
           {
               isIndexedLoad = gcvTRUE;
               /* Get the offset of this uniform member from the previous instruction. */
               prevCode = code - 1;

               gcmASSERT(gcmSL_SOURCE_GET(prevCode->source1, Type) == gcSL_CONSTANT);

               offsetPtr = (gctUINT16 *) offset;
               offsetPtr[0] = prevCode->source1Index;
               offsetPtr[1] = prevCode->source1Indexed;
           }
           else
           {
               isIndexedLoad = gcvFALSE;
               offsetPtr = (gctUINT16 *) offset;
               offsetPtr[0] = code->source1Index;
               offsetPtr[1] = code->source1Indexed;
           }

           gcmASSERT((gcmSL_SOURCE_GET(code->source1, Format) == gcSL_INTEGER ||
                      gcmSL_SOURCE_GET(code->source1, Format) == gcSL_UINT32));

           gcmONERROR(gcSHADER_GetUniform(shader,
                                          gcmSL_INDEX_GET(code->source0Index, Index),
                                          &blockUniform));
           if (!isUniformUBOAddress(blockUniform)) continue;

           if (IsDefaultUBO)
           {
                gcsUNIFORM_BLOCK uniformBlock = gcvNULL;
                gcmONERROR(gcSHADER_GetUniformBlock(shader, blockUniform->blockIndex, &uniformBlock));
                if (GetUBBlockIndex(uniformBlock) != shader->_defaultUniformBlockIndex)
                    continue;
           }

           gcmASSERT(gcmSL_SOURCE_GET(code->source0, Indexed) == gcSL_NOT_INDEXED);
           uboIndex = gcmSL_INDEX_GET(code->source0Index, ConstValue) + (code->source0Indexed << 2);

           blockUniformMember = _FindUniformBlockMember(shader,
                                                        blockUniform,
                                                        uboIndex,
                                                        offset[0],
                                                        &regIndex,
                                                        &startChannel);

           if (blockUniformMember)
           {
               gcSL_ENABLE enable = gcSL_ENABLE_X;
               gctUINT16 indexedRegIndex = 0;
               gcSL_INDEXED indexedMode = gcSL_NOT_INDEXED;
               gctINT enableChannelNum = 0, i = 0;
               gcSL_ENABLE startEnable = gcSL_ENABLE_X;
               gcSL_ENABLE tempEnable = gcSL_ENABLE_X;
               gcSL_SWIZZLE swizzle = gcSL_SWIZZLE_XYZW;

               if (isUniformUsedInShader(blockUniformMember))
                   continue;

               /* Get the source swizzle. */
               enable = (gcSL_ENABLE)gcmSL_TARGET_GET(code->temp, Enable);
               enableChannelNum = 0;
               startEnable = (gcSL_ENABLE)(gcSL_ENABLE_X << startChannel);
               tempEnable = startEnable;
               enableChannelNum = _GetNumUsedComponents(enable);
               for (i = 1; i < enableChannelNum; i++)
               {
                   tempEnable |= (gcSL_ENABLE)(startEnable << i);
               }
               swizzle = (gcSL_SWIZZLE)_Enable2Swizzle(tempEnable);

               if (isIndexedLoad)
               {
                   gcmONERROR(_GetIndexedRegAndModeForLoadInstruction(shader,
                                                                      curInstIdx,
                                                                      blockUniformMember,
                                                                      &indexedRegIndex,
                                                                      &regIndex,
                                                                      &indexedMode,
                                                                      &swizzle,
                                                                      &insertNewCode));


               }
               else
               {
                   indexedMode = gcSL_NOT_INDEXED;
               }
               /* Edit the current instruction to a MOV */
               if (insertNewCode)
               {
                   code = &shader->code[curInstIdx + 1];
               }
               code->opcode = gcSL_MOV;
               code->source0 = code->source0Index = code->source0Indexed = 0;
               code->source1 = code->source1Index = code->source1Indexed = 0;
               code->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM)
                             | gcmSL_SOURCE_SET(0, Indexed, indexedMode)
                             | gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_MEDIUM)
                             | gcmSL_SOURCE_SET(0, Format, blockUniformMember->format)
                             | gcmSL_SOURCE_SET(0, Swizzle, (gctUINT8)swizzle);

               code->source0Index = gcmSL_INDEX_SET(0, Index, blockUniformMember->index)
                                  | gcmSL_INDEX_SET(0, ConstValue, regIndex);

               if (indexedMode == gcSL_NOT_INDEXED)
               {
                   code->source0Indexed = regIndex & ~3;
                   SetUniformFlag(blockUniformMember, gcvUNIFORM_FLAG_DIRECTLY_ADDRESSED);
               }
               else
               {
                   code->source0Indexed = indexedRegIndex;
                   SetUniformFlag(blockUniformMember, gcvUNIFORM_FLAG_INDIRECTLY_ADDRESSED);
               }
           }
           else
           {
               gcmFATAL("ERROR: Invalid uniform block address \'0x%x\' and offset %d",
                        blockUniform,
                        offset[0]);
               continue;
           }

           if (insertNewCode)
           {
               curInstIdx++;
           }
       }
    }

OnError:
    return status;
}

static gctBOOL
_IsTempResolvedToConstant(
    IN gcSHADER Shader,
    IN gctUINT16 TempIndex,
    IN gctINT CurInstIdx,
    IN OUT gctINT *ConstVal,
    OUT gcSL_INSTRUCTION *FoundInst
    )
{
    gcSL_INSTRUCTION foundInst = gcvNULL;
    gctINT instIdx;
    gctBOOL isConst = gcvFALSE;
    gctBOOL isConst0 = gcvTRUE;
    gctBOOL isConst1 = gcvTRUE;
    gctINT constRes = -1;
    gctUINT16 *constValPtr;
    gctINT searchLimit;

    for (instIdx = CurInstIdx, searchLimit = 0; instIdx >= 0 && searchLimit < 3; instIdx--, searchLimit++)
    {
        gcSL_INSTRUCTION code;
        gcSL_OPCODE opcode;

        code = &Shader->code[instIdx];
        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

        if (opcode == gcSL_JMP || opcode == gcSL_CALL || opcode == gcSL_RET) break;

        if(code->tempIndex == TempIndex)
        {
            gctINT constVal0 = 0;
            gctINT constVal1 = 0;

            if (opcode == gcSL_ADD || opcode == gcSL_MOV || opcode == gcSL_SUB ||
                opcode == gcSL_MUL || opcode == gcSL_LSHIFT || opcode == gcSL_RSHIFT)
            {
                if(gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP)
                {
                    isConst0 = _IsTempResolvedToConstant(Shader,
                                                         code->source0Index,
                                                         instIdx - 1,
                                                         &constVal0,
                                                         gcvNULL);
                }
                else if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_CONSTANT)
                {
                    constValPtr = (gctUINT16 *) &constVal0;
                    constValPtr[0] = code->source0Index;
                    constValPtr[1] = code->source0Indexed;
                }
                else if(gcmSL_SOURCE_GET(code->source0, Type) != gcSL_NONE)
                {
                    isConst0 = gcvFALSE;
                }

                if(gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP)
                {
                    isConst1 = _IsTempResolvedToConstant(Shader,
                                                         code->source1Index,
                                                         instIdx - 1,
                                                         &constVal1,
                                                         gcvNULL);
                }
                else if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_CONSTANT)
                {
                    constValPtr = (gctUINT16 *) &constVal1;
                    constValPtr[0] = code->source1Index;
                    constValPtr[1] = code->source1Indexed;
                }
                else if(gcmSL_SOURCE_GET(code->source1, Type) != gcSL_NONE)
                {
                    isConst1 = gcvFALSE;
                }

                isConst = isConst0 && isConst1;
                if(isConst)
                {
                    foundInst = gcvNULL;
                    switch(gcmSL_OPCODE_GET(code->opcode, Opcode))
                    {
                    case gcSL_ADD:
                        constRes = constVal0 + constVal1;
                        break;

                    case gcSL_SUB:
                        constRes = constVal0 - constVal1;
                        break;

                    case gcSL_MOV:
                        constRes = constVal0 ? constVal0 : constVal1;
                        break;

                    case gcSL_MUL:
                        constRes = constVal0 * constVal1;
                        break;

                    case gcSL_RSHIFT:
                        constRes = constVal0 >> constVal1;
                        break;

                    case gcSL_LSHIFT:
                        constRes = constVal0 << constVal1;
                        break;

                    default:
                        gcmASSERT(0);
                        constRes = -1;
                        isConst = gcvFALSE;
                        break;
                    }
                }
                else if(isConst0 == gcvFALSE && isConst1 == gcvFALSE)
                {
                    constRes = -1;
                }
                else if(isConst0 == gcvFALSE)
                {
                    constRes = constVal1;
                    foundInst = code;
                }
                else if(isConst1 == gcvFALSE)
                {
                    constRes = constVal0;
                    foundInst = code;
                }
                else
                {
                    constRes = -1;
                }
            }
            else
            {
                constRes = -1;
            }
            break;
        }
    }

    *ConstVal = constRes;
    if(FoundInst)
    {
       *FoundInst = foundInst;
    }
    return isConst;
}

static gctBOOL
_IsTempOffsetToConstantMemoryAddressReg(
    IN gcSHADER Shader,
    IN gctUINT16 TempIndex,
    IN gctINT CurInstIdx,
    IN OUT gctINT *Offset
    )
{
    gctINT instIdx;
    gctBOOL found = gcvFALSE;
    gctUINT16 *offsetPtr;

    if(TempIndex == _gcdOCL_ConstantMemoryAddressRegIndex)
    {
        return gcvTRUE;
    }

    for (instIdx = CurInstIdx; instIdx >= 0; instIdx--)
    {
        gcSL_INSTRUCTION code;
        gcSL_OPCODE opcode;

        code = &Shader->code[instIdx];

        if(code->tempIndex == TempIndex)
        {
            gctINT offset0 = 0;
            gctINT offset1 = 0;

            opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);
            if (opcode == gcSL_ADD || opcode == gcSL_MOV || opcode == gcSL_SUB ||
                opcode == gcSL_MUL || opcode == gcSL_LSHIFT || opcode == gcSL_RSHIFT)
            {
                if(gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP)
                {
                    found = _IsTempOffsetToConstantMemoryAddressReg(Shader,
                                                                    code->source0Index,
                                                                    instIdx - 1,
                                                                    &offset0);
                }
                else if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_CONSTANT)
                {
                    offsetPtr = (gctUINT16 *) &offset0;
                    offsetPtr[0] = code->source0Index;
                    offsetPtr[1] = code->source0Indexed;
                }
                else
                {
                    *Offset = -1;
                    return gcvFALSE;
                }
                if(gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP)
                {
                    found = _IsTempOffsetToConstantMemoryAddressReg(Shader,
                                                                    code->source1Index,
                                                                    instIdx - 1,
                                                                    &offset1) ? gcvTRUE : found;
                }
                else if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_CONSTANT)
                {
                    offsetPtr = (gctUINT16 *) &offset1;
                    offsetPtr[0] = code->source1Index;
                    offsetPtr[1] = code->source1Indexed;
                }
                else if(gcmSL_SOURCE_GET(code->source1, Type) != gcSL_NONE)
                {
                    *Offset = -1;
                    return gcvFALSE;
                }

                if(found && offset0 >= 0 && offset1 >= 0)
                {
                    gctINT offsetRes;
                    gcSL_OPCODE opcode;
                    opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);

                    switch(opcode)
                    {
                    case gcSL_ADD:
                        offsetRes = offset0 + offset1;
                        break;

                    case gcSL_MOV:
                        offsetRes = offset0 ? offset0 : offset1;
                        break;

                    case gcSL_MUL:
                        offsetRes = offset0 * offset1;
                        break;

                    case gcSL_RSHIFT:
                        offsetRes = offset0 >> offset1;
                        break;

                    case gcSL_LSHIFT:
                        offsetRes = offset0 << offset1;
                        break;

                    default:
                        gcmASSERT(0);
                        offsetRes = -1;
                        found = gcvFALSE;
                        break;
                    }
                    *Offset = offsetRes;
                }
            }
            else
            {
                *Offset = -1;
            }
            break;
        }
    }
    return found;
}

static gceSTATUS
_gcOCL_ChangeLoadToMovUniform(
    IN gcSHADER Shader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER shader = Shader;

    if (shader && shader->uniformBlockCount)
    {
       gcUNIFORM blockUniform, blockUniformMember;
       gctINT offset;
       gctINT constVal;
       gctUINT16 *offsetPtr;
       gctUINT16 regIndex;
       gctINT startChannel;
       gctINT curInstIdx;
       gcSL_INSTRUCTION code;
       gcSL_INSTRUCTION indexedRegCode = gcvNULL;
       gctINT arrayStride = 1;

       gcmONERROR(gcSHADER_GetUniformByName(Shader,
                                            "#constant_address",
                                            17,
                                            &blockUniform));
       gcmASSERT(blockUniform);
       if (!blockUniform || !isUniformConstantAddressSpace(blockUniform)) return gcvSTATUS_INVALID_DATA;

       /* Walk all instructions to change load to mov. */
       for (curInstIdx = 0; (gctUINT)curInstIdx < shader->codeCount; curInstIdx++)
       {
           /* Get instruction. */
           code = &shader->code[curInstIdx];

           /* Determine temporary register usage. */
           if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_LOAD ||
               gcmSL_SOURCE_GET(code->source0, Type) != gcSL_TEMP) continue;

           indexedRegCode = gcvNULL;
           offset = 0;
           if(!_IsTempOffsetToConstantMemoryAddressReg(Shader,
                                                       code->source0Index,
                                                       curInstIdx - 1,
                                                       &offset)) continue;

           if(gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP)
           {
               if(_IsTempResolvedToConstant(Shader,
                                            code->source1Index,
                                            curInstIdx - 1,
                                            &constVal,
                                            &indexedRegCode))
               {
                   offset += constVal;
               }
               else
               {
                   gcSL_OPCODE opcode;
                   if(constVal < 0) continue;

                   /*handle indexing here */
                   gcmASSERT(indexedRegCode);

                   opcode = gcmSL_OPCODE_GET(indexedRegCode->opcode, Opcode);
                   switch(opcode)
                   {
                   case gcSL_ADD:
                       offset += constVal;
                       break;

                   case gcSL_SUB:
                       offset -= constVal;
                       break;

                   case gcSL_MOV:
                       break;

                   case gcSL_MUL:
                       arrayStride = constVal;
                       break;

                   case gcSL_LSHIFT:
                       arrayStride = 1 << constVal;
                       break;

                   default:
                       gcmASSERT(0);
                       continue;
                   }
               }
           }
           else if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_CONSTANT)
           {
               offsetPtr = (gctUINT16 *) &constVal;
               offsetPtr[0] = code->source1Index;
               offsetPtr[1] = code->source1Indexed;
               offset += constVal;
           }
           else if(gcmSL_SOURCE_GET(code->source1, Type) != gcSL_NONE)
           {
               continue;
           }

           gcmASSERT((gcmSL_SOURCE_GET(code->source1, Format) == gcSL_INTEGER ||
                      gcmSL_SOURCE_GET(code->source1, Format) == gcSL_UINT32));

           blockUniformMember = _FindUniformBlockMember(shader,
                                                        blockUniform,
                                                        0,
                                                        offset,
                                                        &regIndex,
                                                        &startChannel);

           if(blockUniformMember)
           {
               gcSL_FORMAT format;
               gcSL_ENABLE enable;
               gctUINT8 effectiveSwizzle;
               gctUINT lastInstruction;
               gcSHADER_INSTRUCTION_INDEX instrIndex;
               gctUINT16 tempIndex, indexedRegIndex;
               gcSL_INDEXED indexedMode;
               gctINT enableChannelNum, i;
               gcSL_ENABLE startEnable;
               gcSL_ENABLE tempEnable;
               gcSL_SWIZZLE swizzle;

               if (isUniformUsedInShader(blockUniformMember))
                   continue;

               if (indexedRegCode &&
                   (GetUniformVectorSize(blockUniformMember) > 4 ||  /* limit vector size to not > 4 */
                    !(GetUniformArraySize(blockUniformMember) > 1) ||  /* not an array */
                    GetUniformArrayStride(blockUniformMember) != arrayStride))
               {
                   gcsUNIFORM_BLOCK uniformBlock;

                   gcmONERROR(gcSHADER_GetUniformBlock(shader,
                                                       blockUniform->blockIndex,
                                                       &uniformBlock));
                   uniformBlock->_useLoadInst = gcvTRUE;
                   continue;
               }

               regIndex *= (gctUINT16)(GetUniformVectorSize(blockUniformMember) + 3)/4;  /* adjust for extended vector size of 8 or 16 */
               format = gcmSL_TARGET_GET(code->temp, Format);
               enable = gcmSL_TARGET_GET(code->temp, Enable);
               enableChannelNum = 0;
               startEnable = gcSL_ENABLE_X << startChannel;
               tempEnable = startEnable;
               lastInstruction = shader->lastInstruction;
               instrIndex = shader->instrIndex;
               /* Edit the current instruction to a MOV */
               shader->lastInstruction = curInstIdx;
               shader->instrIndex = gcSHADER_OPCODE;
               tempIndex = code->tempIndex;
               gcoOS_ZeroMemory(code, sizeof(struct _gcSL_INSTRUCTION));
               gcmONERROR(gcSHADER_AddOpcode(shader,
                                             gcSL_MOV,
                                             tempIndex,
                                             enable,
                                             format,
                                             gcSHADER_PRECISION_MEDIUM));

               enableChannelNum = _GetNumUsedComponents(enable);

               for (i = 1; i < enableChannelNum; i++)
               {
                   tempEnable |= startEnable << i;
               }

               effectiveSwizzle = _Enable2Swizzle(tempEnable);

               if (indexedRegCode)
               {
                   swizzle = gcmSL_SOURCE_GET(indexedRegCode->source0, SwizzleX);
                   gcmASSERT(swizzle <= gcSL_SWIZZLE_W);

                   indexedRegIndex = indexedRegCode->source0Index;
                   indexedMode = (gcSL_INDEXED)(swizzle + 1);

                   gcmONERROR(gcSHADER_AddSourceUniformIndexedFormattedWithPrecision(shader,
                                                                                     blockUniformMember,
                                                                                     (gctUINT8) effectiveSwizzle,
                                                                                     (gctUINT16) regIndex,
                                                                                     indexedMode,
                                                                                     gcSL_NONE_INDEXED,
                                                                                     indexedRegIndex,
                                                                                     blockUniformMember->format,
                                                                                     gcSHADER_PRECISION_MEDIUM));
               }
               else
               {
                   gcmONERROR(gcSHADER_AddSourceUniformFormatted(shader,
                                                                 blockUniformMember,
                                                                 effectiveSwizzle,
                                                                 (gctUINT16) regIndex,
                                                                 blockUniformMember->format));
               }
               SetUniformFlag(blockUniformMember, gcvUNIFORM_FLAG_MOVED_TO_DUB);
               shader->lastInstruction = lastInstruction;
               shader->instrIndex = instrIndex;
           }
           else
           {
               gcmFATAL("ERROR: Invalid uniform block address \'0x%x\' and offset %d",
                        blockUniform,
                        offset);
               continue;
           }
       }
    }

OnError:
    return status;
}

static gctBOOL
_hasUniformType(
    IN gcUNIFORM Uniform
)
{
    if (isUniformNormal(Uniform) ||
        isUniformBlockMember(Uniform) ||
        isUniformBlockAddress(Uniform) ||
        isUniformLodMinMax(Uniform) ||
        isUniformLevelBaseSize(Uniform) ||
        isUniformSampleLocation(Uniform) ||
        isUniformMultiSampleBuffers(Uniform))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

#if !DX_SHADER
static gceSTATUS
_ChangeAtomicCounterUniform2BaseAddrBindingUniform(
    IN gcSHADER      Shader
    )
{
    gctUINT     i      = 0;
    gceSTATUS   status = gcvSTATUS_OK;

    gcmHEADER_ARG("Shader=0x%x", Shader);

    gcmASSERT(Shader);

    for(i = 0; i < Shader->lastInstruction; ++i)
    {
        gctINT           j    = 0;

        for(j = 0; j < 2; ++j)
        {
            gctSOURCE_t              *source             = j == 0 ? &Shader->code[i].source0 : &Shader->code[i].source1;
            gctUINT16                *sourceIndex        = j == 0 ? &Shader->code[i].source0Index : &Shader->code[i].source1Index;
            gctUINT16                *sourceIndexed      = j == 0 ? &Shader->code[i].source0Indexed : &Shader->code[i].source1Indexed;
            gcSL_TYPE                 src_type;
            gcSL_FORMAT               src_format;
            gctINT                    index              = 0;
            gctUINT                   constIndex         = 0;
            gcUNIFORM                 uniform            = gcvNULL;
            gcUNIFORM                 bindingUniform     = gcvNULL;
            gctUINT16                 newRegNo[3];
            gctUINT                   offset             = 0;

            struct _gcSL_INSTRUCTION  loadInst, addInst, mulInst;

            src_type = gcmSL_SOURCE_GET(*source, Type);
            if(src_type != gcSL_UNIFORM)
            {
                continue;
            }

            index   = gcmSL_INDEX_GET(*sourceIndex, Index);
            uniform = Shader->uniforms[index];
            if (uniform == gcvNULL)
            {
                continue;
            }

            if(!_hasUniformType(uniform))
            {
                continue;
            }

            if (gcmType_Kind(uniform->u.type) != gceTK_ATOMIC)
            {
                continue;
            }

            gcmASSERT(uniform->baseBindingIdx >= 0 && uniform->baseBindingIdx < (gctINT16)Shader->uniformCount);

            SetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_IN_SHADER);
            bindingUniform = Shader->uniforms[uniform->baseBindingIdx];

            if (bindingUniform == gcvNULL)
            {
                gcmASSERT(gcvFALSE);
            }

            memset(&loadInst, 0, sizeof(loadInst));
            memset(&addInst, 0, sizeof(addInst));
            memset(&mulInst, 0, sizeof(mulInst));

            newRegNo[0] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
            newRegNo[1] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
            newRegNo[2] = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);

            src_format = gcmSL_SOURCE_GET(*source, Format);
            if(gcmSL_SOURCE_GET(*source, Indexed) == gcSL_NOT_INDEXED)
            {
                /*
                     3: ATOMADD         temp.uint(4).x, uniform.uint(2 + 3).hp.y, 1

                     3: LOAD            temp.uint(61).x, uniform.uint(1).y, offset + 3
                     4: ATOMADD         temp.uint(4).x, temp.uint(61).x, 1
                */
                gcmSL_OPCODE_UPDATE(loadInst.opcode, Opcode, gcSL_LOAD);
                loadInst.temp        = gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                                      | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                                      | gcmSL_TARGET_SET(0, Precision, gcSHADER_PRECISION_HIGH)
                                      | gcmSL_TARGET_SET(0, Format, src_format);
                loadInst.tempIndex   = newRegNo[0];

                loadInst.source0     = *source;
                loadInst.source0     = gcmSL_SOURCE_SET(loadInst.source0, Indexed, gcSL_NOT_INDEXED);
                loadInst.source0     = gcmSL_SOURCE_SET(loadInst.source0, Precision, gcSHADER_PRECISION_HIGH);

                constIndex = gcmSL_INDEX_GET(*sourceIndex, ConstValue) + *sourceIndexed;
                offset     = uniform->offset + constIndex * 4;

                loadInst.source0Index = *sourceIndex;
                loadInst.source0Index = gcmSL_INDEX_SET(loadInst.source0Index, Index, uniform->baseBindingIdx);
                loadInst.source0Index = gcmSL_INDEX_SET(loadInst.source0Index, ConstValue, 0);

                loadInst.source1        = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                        | gcmSL_SOURCE_SET(0, Format, gcSL_UINT32);
                loadInst.source1Index   = ((gctUINT16 *) &offset)[0];
                loadInst.source1Indexed = ((gctUINT16 *) &offset)[1];

                *source      = gcmSL_SOURCE_SET(*source, Type, gcSL_TEMP);
                *source      = gcmSL_SOURCE_SET(*source, Swizzle, gcSL_SWIZZLE_XXXX);
                *source      = gcmSL_SOURCE_SET(*source, Indexed, gcSL_NOT_INDEXED);
                *source      = gcmSL_SOURCE_SET(*source, Precision, gcSHADER_PRECISION_HIGH);

                *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, Index, newRegNo[0]);
                *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, ConstValue, 0);

                *sourceIndexed = 0;

                gcSHADER_InsertNOP2BeforeCode(Shader, i, 1);
                Shader->code[i] = loadInst;

                ++i;
            }
            else
            {
                /*
                     3: ATOMADD         temp.uint(4).x, uniform.uint(2 + temp(3).y + 3).hp.y, 1

                     2: MUL             temp.uint(60).x, temp(3).y, 4
                     3: ADD             temp.uint(61).x, temp(60).x, offset + 3
                     4: LOAD            temp.uint(62).x, uniform.uint(1).y, temp.uint(61).x
                     5: ATOMADD         temp.uint(4).x, temp.uint(62).x, 1
                */
                gcSL_SWIZZLE srcSwz = gcSL_SWIZZLE_XXXX;

                constIndex = gcmSL_INDEX_GET(*sourceIndex, ConstValue);
                offset     = uniform->offset + constIndex * 4;

                gcmSL_OPCODE_UPDATE(mulInst.opcode, Opcode, gcSL_MUL);
                mulInst.temp        = gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                                    | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                                    | gcmSL_TARGET_SET(0, Format, gcSL_UINT32);
                mulInst.tempIndex   = newRegNo[2];

                switch (gcmSL_SOURCE_GET(*source, Indexed))
                {
                case gcSL_INDEXED_X:
                    srcSwz = gcSL_SWIZZLE_XXXX;
                    break;
                case gcSL_INDEXED_Y:
                    srcSwz = gcSL_SWIZZLE_YYYY;
                    break;
                case gcSL_INDEXED_Z:
                    srcSwz = gcSL_SWIZZLE_ZZZZ;
                    break;
                case gcSL_INDEXED_W:
                    srcSwz = gcSL_SWIZZLE_WWWW;
                    break;
                default:
                    gcmASSERT(0);
                    break;
                }

                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Swizzle, srcSwz);
                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Type, gcSL_TEMP);
                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Indexed, gcSL_NOT_INDEXED);
                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Precision, gcSHADER_PRECISION_DEFAULT);
                mulInst.source0     = gcmSL_SOURCE_SET(mulInst.source0, Format, gcSL_INTEGER);

                mulInst.source0Index = *sourceIndexed;
                mulInst.source0Index = gcmSL_INDEX_SET(mulInst.source0Index, ConstValue, 0);

                mulInst.source1      = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                     | gcmSL_SOURCE_SET(0, Format, gcSL_UINT32);
                mulInst.source1Index   = 4;
                mulInst.source1Indexed = 0;


                gcmSL_OPCODE_UPDATE(addInst.opcode, Opcode, gcSL_ADD);
                addInst.temp        = gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                                    | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                                    | gcmSL_TARGET_SET(0, Format, gcSL_UINT32);
                addInst.tempIndex   = newRegNo[0];


                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Swizzle, gcSL_SWIZZLE_XXXX);
                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Type, gcSL_TEMP);
                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Indexed, gcSL_NOT_INDEXED);
                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Precision, gcSHADER_PRECISION_DEFAULT);
                addInst.source0     = gcmSL_SOURCE_SET(addInst.source0, Format, gcSL_INTEGER);

                addInst.source0Index = newRegNo[2];
                addInst.source0Index = gcmSL_INDEX_SET(addInst.source0Index, ConstValue, 0);

                addInst.source1      = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                     | gcmSL_SOURCE_SET(0, Format, gcSL_UINT32);
                addInst.source1Index   = ((gctUINT16 *) &offset)[0];
                addInst.source1Indexed = ((gctUINT16 *) &offset)[1];


                gcmSL_OPCODE_UPDATE(loadInst.opcode, Opcode, gcSL_LOAD);
                loadInst.temp        = gcmSL_TARGET_SET(0, Condition, gcSL_ALWAYS)
                                     | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                                     | gcmSL_TARGET_SET(0, Precision, gcSHADER_PRECISION_HIGH)
                                     | gcmSL_TARGET_SET(0, Format, src_format);
                loadInst.tempIndex   = newRegNo[1];

                loadInst.source0     = *source;
                loadInst.source0     = gcmSL_SOURCE_SET(loadInst.source0, Indexed, gcSL_NOT_INDEXED);
                loadInst.source0     = gcmSL_SOURCE_SET(loadInst.source0, Precision, gcSHADER_PRECISION_HIGH);

                loadInst.source0Index = *sourceIndex;
                loadInst.source0Index = gcmSL_INDEX_SET(loadInst.source0Index, Index, uniform->baseBindingIdx);
                loadInst.source0Index = gcmSL_INDEX_SET(loadInst.source0Index, ConstValue, 0);

                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Swizzle, gcSL_SWIZZLE_XXXX);
                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Type, gcSL_TEMP);
                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Indexed, gcSL_NOT_INDEXED);
                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Precision, gcSHADER_PRECISION_DEFAULT);
                loadInst.source1     = gcmSL_SOURCE_SET(loadInst.source1, Format, gcSL_INTEGER);

                loadInst.source1Index   = newRegNo[0];

                *source      = gcmSL_SOURCE_SET(*source, Swizzle, gcSL_SWIZZLE_XXXX);
                *source     = gcmSL_SOURCE_SET(*source, Type, gcSL_TEMP);
                *source     = gcmSL_SOURCE_SET(*source, Indexed, gcSL_NOT_INDEXED);
                *source     = gcmSL_SOURCE_SET(*source, Precision, gcSHADER_PRECISION_HIGH);

                *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, Index, newRegNo[1]);
                *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, ConstValue, 0);

                *sourceIndexed = 0;

                gcSHADER_InsertNOP2BeforeCode(Shader, i, 3);

                Shader->code[i]     = mulInst;
                Shader->code[i + 1] = addInst;
                Shader->code[i + 2] = loadInst;

                i += 3;
            }
        }
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
_PreprocessHeplerInvocation(
    gcSHADER FragmentShader
    )
{
    gctUINT               i, j, f;
    gctINT                tempIdx;
    gceSTATUS             status = gcvSTATUS_OK;
    gctBOOL               hasHelperInvocation = gcvFALSE, bInMainRoutine;
    gcATTRIBUTE           attribHelperInvocation = gcvNULL, attrib;
    gctSOURCE_t           *source;
    gctUINT16             *sourceIndex;
    gctINT                attribIdx;
    gcFUNCTION            function;
    gcSL_INSTRUCTION      setCode;

    gcmHEADER_ARG("FragmentShader=0x%x", FragmentShader);

    gcmASSERT(FragmentShader);

    for (i = 0; i < FragmentShader->attributeCount; ++i)
    {
        /* PS varying may be deleted by optimization. */
        if (FragmentShader->attributes[i] &&
            FragmentShader->attributes[i]->nameLength == gcSL_HELPER_INVOCATION)
        {
            attribHelperInvocation = FragmentShader->attributes[i];
            break;
        }
    }

    if (attribHelperInvocation)
    {
        for (i = 0; i < FragmentShader->lastInstruction; ++i)
        {
            for (j = 0; j < 2; ++j)
            {
                source        = (j == 0) ? &FragmentShader->code[i].source0 : &FragmentShader->code[i].source1;
                sourceIndex   = (j == 0) ? &FragmentShader->code[i].source0Index : &FragmentShader->code[i].source1Index;

                if (gcmSL_SOURCE_GET(*source, Type) != gcSL_ATTRIBUTE)
                {
                    continue;
                }

                attribIdx = gcmSL_INDEX_GET(*sourceIndex, Index);

                if (attribHelperInvocation->index == attribIdx)
                {
                    hasHelperInvocation = gcvTRUE;
                    break;
                }
            }
        }
    }

    if (hasHelperInvocation)
    {
        for (i = 0; i < FragmentShader->lastInstruction; i ++)
        {
            bInMainRoutine = gcvTRUE;

            /* Determine ownership of the code for functions. */
            for (f = 0; f < FragmentShader->functionCount; ++f)
            {
                function = FragmentShader->functions[f];

                if ((i >= function->codeStart) &&
                    (i < function->codeStart + function->codeCount))
                {
                    bInMainRoutine = gcvFALSE;
                    break;
                }
            }

            if (bInMainRoutine)
            {
                break;
            }
        }

        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(FragmentShader, i, 1));

        setCode = &FragmentShader->code[i];

        tempIdx = _getUnusedTempIndex(FragmentShader, 1);

        if (gcoHAL_IsFeatureAvailable1(NULL, gcvFEATURE_HELPER_INVOCATION))
        {
            setCode->opcode = gcSL_SET;
            setCode->temp = gcmSL_TARGET_SET(0, Format, gcSL_BOOLEAN)  |
                            gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X) |
                            gcmSL_TARGET_SET(0, Condition, gcSL_EQUAL);
            setCode->tempIndex = (gctUINT16)tempIdx;

            setCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_ATTRIBUTE)  |
                               gcmSL_SOURCE_SET(0, Format, gcSL_BOOLEAN)  |
                               gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_MEDIUM)  |
                               gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
            setCode->source0Index = gcmSL_INDEX_SET(0, Index, attribHelperInvocation->index);

            setCode->source1 = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT) |
                               gcmSL_SOURCE_SET(0, Format, gcSL_BOOLEAN);
            setCode->source1Index = 1;
        }
        else
        {
            setCode->opcode = gcSL_MOV;
            setCode->temp = gcmSL_TARGET_SET(0, Format, gcSL_BOOLEAN)  |
                            gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X);
            setCode->tempIndex = (gctUINT16)tempIdx;

            setCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)  |
                               gcmSL_SOURCE_SET(0, Format, gcSL_BOOLEAN)  |
                               gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX);
            setCode->source0Index = setCode->source0Indexed = 0;

            setCode->source1 = 0;
            setCode->source0Index = setCode->source0Indexed = 0;
        }

        for (i ++; i < FragmentShader->lastInstruction; ++i)
        {
            for (j = 0; j < 2; ++j)
            {
                source        = (j == 0) ? &FragmentShader->code[i].source0 : &FragmentShader->code[i].source1;
                sourceIndex   = (j == 0) ? &FragmentShader->code[i].source0Index : &FragmentShader->code[i].source1Index;

                if (gcmSL_SOURCE_GET(*source, Type) != gcSL_ATTRIBUTE)
                {
                    continue;
                }

                attribIdx = gcmSL_INDEX_GET(*sourceIndex, Index);
                attrib = FragmentShader->attributes[attribIdx];
                if (attrib == gcvNULL)
                {
                    continue;
                }

                gcmASSERT(attrib->index == attribIdx);

                if (attrib->nameLength != gcSL_HELPER_INVOCATION)
                {
                    continue;
                }

                *source = gcmSL_SOURCE_SET(*source, Type, gcSL_TEMP);
                *sourceIndex = (gctUINT16)tempIdx;
            }
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
_Implement32BitModulus(
    IN gcSHADER Shader,
    IN gctBOOL IsInt32,
    IN gctBOOL IsMod,
    IN gcSL_INSTRUCTION Code
    )
{
    gctUINT16 newSource0Index, newSource1Index, newTargetIndex;
    gctUINT16 newTemp0, newTemp1, newTemp2, newTemp3, newTemp4, newTemp5, newTemp6;
    gctUINT16 newTemp7, newTemp8, newTemp9, newTemp10;
    gctUINT16 int32Temp0 = 0, int32Temp1 = 0, int32Temp2 = 0;
    gctUINT endLabel, label1, label2, label3;
    gctINT constZero = 0, constOne = 1;
    gctUINT constFour = 0xf0000004 /* (32<<23) + 4 */;
    gcSL_FORMAT targetType = IsInt32 ? gcSL_INTEGER : gcSL_UINT32;
    gcSHADER_PRECISION precision = gcmSL_SOURCE_GET(Code->source0, Precision) ? gcSHADER_PRECISION_HIGH : gcSHADER_PRECISION_MEDIUM;
    gcSL_INSTRUCTION code;

    newSource0Index = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    newSource1Index = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);

    newTargetIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);

    /* common temp register. */
    newTemp0 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X1);
    newTemp1 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    newTemp2 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X1);
    newTemp3 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    newTemp4 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    newTemp5 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    newTemp6 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    newTemp7 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    newTemp8 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    newTemp9 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    newTemp10 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);

    if (IsInt32)
    {
        int32Temp0 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_INTEGER_X1);
        int32Temp1 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
        int32Temp2 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X1);
    }

    /*
    ** Mov the source to the new source index, so new sources are in X channel without indexed.
    */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader,gcSL_MOV, newSource0Index, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, targetType, precision);
    code = Shader->code + Shader->lastInstruction;
    code->source0 = Code->source0;
    code->source0Index = Code->source0Index;
    code->source0Indexed = Code->source0Indexed;

    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MOV, newSource1Index, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, targetType, precision);
    code = Shader->code + Shader->lastInstruction;
    code->source0 = Code->source1;
    code->source0Index = Code->source1Index;
    code->source0Indexed = Code->source1Indexed;

    /* Start. */

    if (IsInt32)
    {
        /* r0.32 = sign(Y) */
        gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_SIGN, int32Temp0, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_INT32, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newSource1Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_INT32, precision);

        /* R = sign(X) */
        gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_SIGN, newTargetIndex, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_INT32, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newSource0Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_INT32, precision);

        /* r0.32 = r0.32 * R */
        gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MUL, int32Temp0, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_INT32, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, int32Temp0, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_INT32, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTargetIndex, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_INT32, precision);

        /* r1.32 = abs(X) */
        gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_ABS, int32Temp1, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newSource0Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_INT32, precision);

        /* r2.32 = abs(Y) */
        gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_ABS, int32Temp2, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newSource1Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_INT32, precision);
    }

    /* If Y > 1, normal step, else result X/Y = X */
    endLabel  = gcSHADER_FindNextUsedLabelId(Shader);
    gcSHADER_AddOpcodeConditional(Shader, gcSL_JMP, gcSL_LESS_OR_EQUAL, endLabel);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, IsInt32 ? int32Temp2 : newSource1Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceConstantFormatted(Shader, &constOne, gcSL_UINT32);

    /* Normal step. */
    /* Goto float, r0 = ITOF(y) */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_I2F, newTemp0, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_FLOAT, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, IsInt32 ? int32Temp2 : newSource1Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* r1 = 4026531844 + r0 */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_ADD, newTemp1, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceConstantFormatted(Shader, &constFour, gcSL_UINT32);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp0, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_FLOAT, precision);

    /* r2 = RCP(r1) */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_RCP, newTemp2, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_FLOAT, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp1, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_FLOAT, precision);

    /* r3 = FTOI(r2) */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_F2I, newTemp3, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp2, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_FLOAT, precision);

    /* r4 = 0 - Y */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_SUB, newTemp4, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceConstantFormatted(Shader, &constZero, gcSL_UINT32);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, IsInt32 ? int32Temp2 : newSource1Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* r5 = r4 * r3 */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MUL, newTemp5, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp4, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp3, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* r10 = r5 * r3(MULHI) */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MULHI, newTemp10, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp5, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp3, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* r6 = r10 + r3 */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_ADD, newTemp6, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp10, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp3, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* r7 = r6 * Y*/
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MUL, newTemp7, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp6, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, IsInt32 ? int32Temp2 : newSource1Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* if (r7 > r4), goto label1. */
    label1 = gcSHADER_FindNextUsedLabelId(Shader);
    gcSHADER_AddOpcodeConditional(Shader, gcSL_JMP, gcSL_GREATER, label1);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp7, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp4, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* r6 = r6 + 1 */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_ADD, newTemp6, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp6, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceConstantFormatted(Shader, &constOne, gcSL_UINT32);

    /* label1: r8 = r6 * X(MULHI) */
    gcSHADER_AddLabel(Shader, label1);
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MULHI, newTemp8, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp6, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, IsInt32 ? int32Temp1 : newSource0Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* r9 = r8 * r4 */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MUL, newTemp9, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp8, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp4, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* r9 = X + r9 */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_ADD, newTemp9, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, IsInt32 ? int32Temp1 : newSource0Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp9, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* if r9 < Y, goto label2 */
    label2 = gcSHADER_FindNextUsedLabelId(Shader);
    gcSHADER_AddOpcodeConditional(Shader, gcSL_JMP, gcSL_LESS, label2);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp9, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, IsInt32 ? int32Temp2 : newSource1Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* R = r8 + 1 */
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_ADD, newTargetIndex, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, targetType, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp8, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);
    gcSHADER_AddSourceConstantFormatted(Shader, &constOne, gcSL_UINT32);

    /* goto label3 */
    label3 = gcSHADER_FindNextUsedLabelId(Shader);
    gcSHADER_AddOpcodeConditional(Shader, gcSL_JMP, gcSL_ALWAYS, label3);

    /* label2: R = r8 */
    gcSHADER_AddLabel(Shader, label2);
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MOV, newTargetIndex, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, targetType, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTemp8, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    /* goto label3*/
    gcSHADER_AddOpcodeConditional(Shader, gcSL_JMP, gcSL_ALWAYS, label3);

    /* End Label: R = X */
    gcSHADER_AddLabel(Shader, endLabel);
    gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MOV, newTargetIndex, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, targetType, precision);
    gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, IsInt32 ? int32Temp1 : newSource0Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, gcSL_UINT32, precision);

    gcSHADER_AddLabel(Shader, label3);
    if (IsInt32)
    {
        /* R = r0.32 * R*/
        gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MUL, newTargetIndex, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, targetType, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, int32Temp0, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, targetType, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTargetIndex, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, targetType, precision);
    }

    if (IsMod)
    {
        /* label3: R = Y * R */
        gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_MUL, newTargetIndex, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, targetType, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newSource1Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, targetType, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTargetIndex, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, targetType, precision);

        /* R = X - R */
        gcSHADER_AddOpcodeIndexedWithPrecision(Shader, gcSL_SUB, newTargetIndex, gcSL_ENABLE_X, gcSL_NOT_INDEXED, 0, targetType, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newSource0Index, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, targetType, precision);
        gcSHADER_AddSourceIndexedWithPrecision(Shader, gcSL_TEMP, newTargetIndex, gcSL_SWIZZLE_XXXX, gcSL_NOT_INDEXED, 0, targetType, precision);
    }

    /* Now we finish compute, mov the new target to the original target. */
    gcmSL_OPCODE_UPDATE(Code->opcode, Opcode, gcSL_MOV);
    Code->source0 = gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_X)
                  | gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                  | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
#if SOURCE_is_32BIT
                  | gcmSL_SOURCE_SET(0, Precision, precision)
#endif
                  | gcmSL_SOURCE_SET(0, Format, targetType);
    Code->source0Index = newTargetIndex;
    Code->source0Indexed = 0;
    Code->source1 = 0;
    Code->source1Index = 0;
    Code->source1Indexed = 0;

    return gcvSTATUS_OK;
}

gctBOOL isPowerOf2(gctUINT v)
{
    return ((v != 0) && ((v & (~v + 1)) == v));
}

/* TODO: move the implementation of 32bit modulus to library and change
 * the code to builtin intrinsic library in FE */
static gceSTATUS
_gcConvert32BitModulus(
    IN gcSHADER Shader,
    IN gctBOOL  Dump
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;
    gcSL_INSTRUCTION code;
    gctINT addCodeCount;
    gctUINT lastInst = Shader->lastInstruction;
    gctBOOL changed = gcvFALSE;

    if (Shader->lastInstruction == 0 || Shader->codeCount == 0)
        return status;

    for (i = (gctINT)Shader->lastInstruction - 1; i >= 0; i--)
    {
        gctBOOL isInt32 = gcvFALSE, isMod = gcvTRUE;
        gcSL_OPCODE opcode;
        gcSL_FORMAT format;

        code = &Shader->code[i];
        opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
        format = gcmSL_TARGET_GET(code->temp, Format);

        if (opcode != gcSL_MOD && opcode != gcSL_DIV)
            continue;

        if (format != gcSL_INT32 && format != gcSL_UINT32)
        {
            continue;
        }

        if (format == gcSL_INT32)
        {
            isInt32 = gcvTRUE;
            addCodeCount = 31;
        }
        else
        {
            addCodeCount = 25;
        }

        if (opcode == gcSL_DIV)
        {
            isMod = gcvFALSE;
            addCodeCount -= 2;
        }

        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, i, (gctUINT)addCodeCount));
        Shader->lastInstruction = i;
        Shader->instrIndex = gcSHADER_OPCODE;
        gcmONERROR(_Implement32BitModulus(Shader, isInt32, isMod, &Shader->code[i + addCodeCount]));
        gcShaderSetHas32BitModulus(Shader);

        lastInst +=addCodeCount;
        Shader->lastInstruction = lastInst;
        changed = gcvTRUE;
        gcSHADER_Pack(Shader);
    }

    if (Dump && changed)
    {
        gcOpt_Dump(gcvNULL, "After convert 32bit Modulus for shader.", gcvNULL, Shader);
    }

OnError:
    return status;
}

static gctBOOL
_isIndexNeedToBeUpdated(
    IN gcArgumentPacking * ArgumentPacking,
    IN gctINT ArgCount,
    IN gctUINT16 TempIndex,
    OUT gctINT * ArgIndex
    )
{
    gctBOOL matched = gcvFALSE;
    gctINT i;

    for (i = 0; i < ArgCount; i++)
    {
        gcArgumentPacking argPacking = ArgumentPacking[i];

        if (argPacking.isPacked && argPacking.argument.index == TempIndex)
        {
            matched = gcvTRUE;
            if (ArgIndex)
            {
                *ArgIndex = i;
            }
            break;
        }
    }

    return matched;
}

static gceSTATUS
_fixArgumentAsTarget(
    IN gcSL_INSTRUCTION Code,
    IN gcArgumentPacking * ArgumentPacking,
    IN gctINT ArgCount
    )
{
    gctINT argIndex = 0;
    gcArgumentPacking argPacking;

    if (gcmSL_OPCODE_GET(Code->opcode, Opcode) == gcSL_JMP)
        return gcvSTATUS_OK;

    if (_isIndexNeedToBeUpdated(ArgumentPacking, ArgCount, Code->tempIndex, &argIndex))
    {
        argPacking = ArgumentPacking[argIndex];
        Code->tempIndex = argPacking.newIndex;
        _fixEnable(Code, argPacking.mapping);
        _fixSwizzle(Code, argPacking.mapping);
    }

    if (gcmSL_TARGET_GET(Code->temp, Indexed) != gcSL_NOT_INDEXED &&
        _isIndexNeedToBeUpdated(ArgumentPacking, ArgCount, Code->tempIndexed, &argIndex))
    {
        gcSL_INDEXED indexCompnent = gcmSL_TARGET_GET(Code->temp, Indexed);
        argPacking = ArgumentPacking[argIndex];

        /* change indexed value */
        Code->tempIndexed = argPacking.newIndex;
        switch (argPacking.mapping)
        {
        case gcCMAP_XY2ZW:
            if (indexCompnent == gcSL_INDEXED_X)
            {
                Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed, gcSL_INDEXED_Z);
            }
            else if (indexCompnent == gcSL_INDEXED_Y)
            {
                Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed, gcSL_INDEXED_W);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
            break;

        case gcCMAP_X2Y:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_Y);
            break;

        case gcCMAP_X2Z:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_Z);
            break;

        case gcCMAP_X2W:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            Code->temp = gcmSL_TARGET_SET(Code->temp, Indexed,gcSL_INDEXED_W);
            break;

        default:
            break;
        }
    }
    return gcvSTATUS_OK;
}

static gceSTATUS
_fixArgumentAsSource(
    IN OUT gctSOURCE_t * Source,
    IN OUT gctUINT16 * SourceIndex,
    IN OUT gctUINT16 * SourceIndexed,
    IN gcArgumentPacking * ArgumentPacking,
    IN gctINT ArgCount
    )
{
    gctINT argIndex = 0;
    gcArgumentPacking argPacking;
    gctUINT16 tempIndex = *SourceIndex;

    /* fix indexed */
    if (gcmSL_SOURCE_GET(*Source, Type) != gcSL_NONE &&
        gcmSL_SOURCE_GET(*Source, Indexed) != gcSL_NOT_INDEXED &&
        _isIndexNeedToBeUpdated(ArgumentPacking, ArgCount, *SourceIndexed, &argIndex)
        )
    {
        gcSL_INDEXED indexCompnent = gcmSL_SOURCE_GET(*Source, Indexed);
        argPacking = ArgumentPacking[argIndex];

        tempIndex = gcmSL_INDEX_GET(*SourceIndex, Index);

        /* change indexed value */
        *SourceIndexed = argPacking.newIndex;
        switch (argPacking.mapping)
        {
        case gcCMAP_XY2ZW:
            if (indexCompnent == gcSL_INDEXED_X)
            {
                *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_Z);
            }
            else if (indexCompnent == gcSL_INDEXED_Y)
            {
                *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_W);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }
            break;
        case gcCMAP_X2Y:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_Y);
            break;
        case gcCMAP_X2Z:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_Z);
            break;
        case gcCMAP_X2W:
            gcmASSERT(indexCompnent == gcSL_INDEXED_X);
            *Source = gcmSL_SOURCE_SET(*Source, Indexed,gcSL_INDEXED_W);
            break;
        default:
            break;
        }
    }

    if (gcmSL_SOURCE_GET(*Source, Type) == gcSL_TEMP &&
        _isIndexNeedToBeUpdated(ArgumentPacking, ArgCount, tempIndex, &argIndex))
    {
        argPacking = ArgumentPacking[argIndex];

        *SourceIndex = gcmSL_INDEX_SET(*SourceIndex, Index, argPacking.newIndex);
        _mappingSourceComponent(Source, argPacking.mapping);
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_packingArugmentsForSingleFunction(
    IN gcSHADER Shader,
    IN gcFUNCTION Function,
    OUT gctBOOL * Changed
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT enableCount[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
    gctUINT32 i;
    gctINT argCount = 0;
    gctINT *vec1Arg = gcvNULL, *vec2Arg = gcvNULL, *vec3Arg = gcvNULL;
    gctINT vec1Count = 0, vec2Count = 0, vec3Count = 0;
    gctINT vec1_i=0, vec2_i=0, vec3_i=0;
    gctPOINTER pointer = gcvNULL;
    gctBOOL changed = gcvFALSE;
    gcArgumentPacking * argumentPacking = gcvNULL;

    gcmONERROR(gcoOS_Allocate(gcvNULL, Function->argumentCount * gcmSIZEOF(struct _gcArgumentPacking), &pointer));
    gcoOS_ZeroMemory(pointer, Function->argumentCount * sizeof(struct _gcArgumentPacking));
    argumentPacking = pointer;

    for (i = 0; i < Function->argumentCount; i++)
    {
        gcsFUNCTION_ARGUMENT_PTR argument = &Function->arguments[i];

        argumentPacking[i].argument.enable = argument->enable;
        argumentPacking[i].argument.index = argument->index;
        argumentPacking[i].argument.qualifier = argument->qualifier;
        argumentPacking[i].argument.precision = argument->precision;
        argumentPacking[i].argument.variableIndex = argument->variableIndex;

        if (argument->qualifier == gcvFUNCTION_INPUT &&
            argument->variableIndex != 0xffff)
        {
            gcVARIABLE variable = Shader->variables[argument->variableIndex];

            /* Don't pack sampler/image/atomic/matrix. */
            if ((isNormalType(GetVariableType(variable))) &&
                (gcmType_Rows(GetVariableType(variable)) == 1))
            {
                argCount++;
                argumentPacking[i].needPacked = gcvTRUE;
            }
        }
    }

    if (argCount < 2)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, argumentPacking));
        return status;
    }

    /* Allocate the packing array for vec1/vec2/vec3. */
    gcmONERROR(gcoOS_Allocate(gcvNULL, argCount * gcmSIZEOF(gctINT), &pointer));
    gcoOS_ZeroMemory(pointer, argCount * gcmSIZEOF(gctINT));
    vec1Arg = pointer;

    gcmONERROR(gcoOS_Allocate(gcvNULL, argCount * gcmSIZEOF(gctINT), &pointer));
    gcoOS_ZeroMemory(pointer, argCount * gcmSIZEOF(gctINT));
    vec2Arg = pointer;

    gcmONERROR(gcoOS_Allocate(gcvNULL, argCount * gcmSIZEOF(gctINT), &pointer));
    gcoOS_ZeroMemory(pointer, argCount * gcmSIZEOF(gctINT));
    vec3Arg = pointer;

    /* Count all type input arguments number. */
    for (i = 0; i < Function->argumentCount; i++)
    {
        if (!argumentPacking[i].needPacked)
            continue;

        switch(enableCount[argumentPacking[i].argument.enable])
        {
        case 1:
            vec1Arg[vec1Count++] = i;
            break;
        case 2:
            vec2Arg[vec2Count++] = i;
            break;
        case 3:
            vec3Arg[vec3Count++] = i;
            break;
        default:
            break;
        }
    }

    /* vec2_2 packing modes. */
    for (; vec2_i < vec2Count - 1;)
    {
        gcArgumentPacking * firstArg = &argumentPacking[vec2Arg[vec2_i++]];
        gcArgumentPacking * secondArg = &argumentPacking[vec2Arg[vec2_i++]];

        /* Update the first argument. */
        firstArg->argument.enable = gcSL_ENABLE_XYZW;

        /* Update the packed argument. */
        secondArg->isPacked = gcvTRUE;
        secondArg->mapping = gcCMAP_XY2ZW;
        secondArg->newIndex = firstArg->argument.index;

        changed = gcvTRUE;
    }

    /* vec3_1 packing modes. */
    for (; vec3_i < vec3Count;)
    {
        gcArgumentPacking * firstArg = gcvNULL, * secondArg = gcvNULL;

        /* There is no enough vec1 argument left. */
        if (vec1_i >= vec1Count)
            break;

        firstArg = &argumentPacking[vec3Arg[vec3_i++]];
        secondArg = &argumentPacking[vec1Arg[vec1_i++]];

        /* Update the first argument. */
        firstArg->argument.enable = gcSL_ENABLE_XYZW;

        /* Update the packed argument. */
        secondArg->isPacked = gcvTRUE;
        secondArg->mapping = gcCMAP_X2W;
        secondArg->newIndex = firstArg->argument.index;
        changed = gcvTRUE;
    }

    /* handle remaining vec2: vec2_1_1 packing */
    if (vec2_i == vec2Count - 1 && vec1_i < vec1Count)
    {
        gcArgumentPacking * firstArg = &argumentPacking[vec2Arg[vec2_i++]];
        gcArgumentPacking * secondArg = &argumentPacking[vec1Arg[vec1_i++]];

        /* Update the first argument. */
        firstArg->argument.enable = gcSL_ENABLE_XYZ;

        /* Update the packed argument. */
        secondArg->isPacked = gcvTRUE;
        secondArg->mapping = gcCMAP_X2Z;
        secondArg->newIndex = firstArg->argument.index;

        if (vec1_i < vec1Count)
        {
            gcArgumentPacking * thirdArg = &argumentPacking[vec1Arg[vec1_i++]];

            firstArg->argument.enable = gcSL_ENABLE_XYZW;

            thirdArg->isPacked = gcvTRUE;
            thirdArg->mapping = gcCMAP_X2W;
            thirdArg->newIndex = firstArg->argument.index;
        }
        changed = gcvTRUE;
    }

    /* left vec1 arguments. */
    for (; vec1_i < vec1Count - 1;)
    {
        gcArgumentPacking * firstArg = &argumentPacking[vec1Arg[vec1_i++]];
        gcArgumentPacking * secondArg = &argumentPacking[vec1Arg[vec1_i++]];

        /* Update the first argument. */
        firstArg->argument.enable = gcSL_ENABLE_XY;

        /* Update the packed argument. */
        secondArg->isPacked = gcvTRUE;
        secondArg->mapping = gcCMAP_X2Y;
        secondArg->newIndex = firstArg->argument.index;

        if (vec1_i < vec1Count)
        {
            gcArgumentPacking * thirdArg = &argumentPacking[vec1Arg[vec1_i++]];

            firstArg->argument.enable = gcSL_ENABLE_XYZ;

            thirdArg->isPacked = gcvTRUE;
            thirdArg->mapping = gcCMAP_X2Z;
            thirdArg->newIndex = firstArg->argument.index;

            if (vec1_i < vec1Count)
            {
                gcArgumentPacking * fouthArg = &argumentPacking[vec1Arg[vec1_i++]];

                firstArg->argument.enable = gcSL_ENABLE_XYZW;

                fouthArg->isPacked = gcvTRUE;
                fouthArg->mapping = gcCMAP_X2W;
                fouthArg->newIndex = firstArg->argument.index;
            }
        }
        changed = gcvTRUE;
    }

    /* Update shader code. */
    for (i = 0; i < Shader->codeCount; i++)
    {
        gcSL_INSTRUCTION code = &Shader->code[i];

        if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_CALL)
            continue;
        /* Update the dest. */
        _fixArgumentAsTarget(code, argumentPacking, Function->argumentCount);
        /* Update the source0. */
        _fixArgumentAsSource(&code->source0, &code->source0Index, &code->source0Indexed, argumentPacking, Function->argumentCount);
        /* Update the source1. */
        _fixArgumentAsSource(&code->source1, &code->source1Index, &code->source1Indexed, argumentPacking, Function->argumentCount);
    }

    /* Remove the packed arguments from function argument list. */
    if (changed)
    {
        gctUINT32 argCount = Function->argumentCount;
        gcVARIABLE variable = gcvNULL;
        gcSHADER_TYPE newType;
        gcSL_FORMAT format = gcSL_FLOAT;
        gcSHADER_TYPE_KIND kind = gceTK_FLOAT;

        if (Function->arguments != gcvNULL)
        {
            gcmVERIFY_OK(
                gcmOS_SAFE_FREE(gcvNULL, Function->arguments));
        }
        Function->argumentCount = 0;
        Function->argumentArrayCount = 0;

        for (i = 0; i < argCount; i++)
        {
            if (argumentPacking[i].argument.qualifier == gcvFUNCTION_INPUT &&
                argumentPacking[i].argument.variableIndex != 0xffff)
            {
                gcmONERROR(gcSHADER_GetVariable(Shader,
                                                argumentPacking[i].argument.variableIndex,
                                                &variable));
                kind = gcmType_Kind(GetVariableType(variable));

                switch (kind)
                {
                case gceTK_INT:
                    format = gcSL_INTEGER;
                    break;

                case gceTK_INT64:
                    format = gcSL_INT64;
                    break;

                case gceTK_UINT:
                    format = gcSL_UINT32;
                    break;

                case gceTK_UINT64:
                    format = gcSL_UINT64;
                    break;

                case gceTK_BOOL:
                    format = gcSL_BOOLEAN;
                    break;

                default:
                    format = gcSL_FLOAT;
                    break;
                }

            }

            if (!argumentPacking[i].isPacked)
            {
                gcmONERROR(gcFUNCTION_AddArgument(Function,
                                                  argumentPacking[i].argument.variableIndex,
                                                  argumentPacking[i].argument.index,
                                                  argumentPacking[i].argument.enable,
                                                  argumentPacking[i].argument.qualifier,
                                                  argumentPacking[i].argument.precision,
                                                  argumentPacking[i].argument.flags & gceFUNCTION_ARGUMENT_FLAG_IS_PRECISE));

                /* Update the type of argument variable. */
                if (variable)
                {
                    newType = gcGetShaderTypeFromFormatAndComponentCount(format,
                                                                         enableCount[argumentPacking[i].argument.enable],
                                                                         1);
                    SetVariableType(variable, newType);
                }
            }
            else
            {
                /* Mark the argument variable as not used. */
                if (variable)
                {
                    SetVariableIsNotUsed(variable);
                }
            }
        }
        Function->packedAwayArgNo += argCount - Function->argumentCount;
    }

    if (Changed)
    {
        *Changed = changed;
    }

OnError:
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, argumentPacking));
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, vec1Arg));
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, vec2Arg));
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, vec3Arg));
    return status;
}

static gceSTATUS
_gcPackingFunctionArgument(
    IN gcSHADER Shader,
    IN gctBOOL  Dump
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL changed = gcvFALSE;
    gctUINT32 i;

    if (1 || Shader->functionCount == 0)
        return status;

    /* III: Pack arguments for each functions. */
    for (i = 0; i < Shader->functionCount; i++)
    {
        gcFUNCTION function = Shader->functions[i];

        if (function->argumentCount < 2)
            continue;

        gcmONERROR(_packingArugmentsForSingleFunction(Shader, function, &changed));
    }

    if (Dump && changed)
    {
        gcOpt_Dump(gcvNULL, "After packing function argument for shader.", gcvNULL, Shader);
    }

OnError:
    return status;
}

static gceSTATUS
_gcLINKTREE_ReplaceColor2FrontBackColor(
    IN gcSHADER FragmentShader
);

static gceSTATUS
_gcLINKTREE_CreateColorOutput(
    IN gcSHADER VertexShader
);

gceSTATUS
gcSetUniformShaderKind(
    IN gcSHADER Shader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 i;

    for (i = 0; i < Shader->uniformCount; i++)
    {
        gcUNIFORM uniform = Shader->uniforms[i];

        if (!uniform) continue;
        SetUniformShaderKind(uniform, Shader->type);
    }

    for (i = 0; i < Shader->uniformBlockCount; i++)
    {
        gcsUNIFORM_BLOCK ubo = Shader->uniformBlocks[i];

        if (!ubo) continue;
        SetUBShaderKind(ubo, Shader->type);
    }

    return status;
}

static gceSTATUS
_IgnoreAllReferenceToBoundingBox(
    IN gcSHADER Shader
    )
{
    gctBOOL isTemp = gcvFALSE;
    gctUINT i;
    gctUINT16 indexMin = (gctUINT16)-1, indexMax = (gctUINT16)-1;
    gctUINT16 offset;
    gcSL_TYPE type;

    if (GetShaderType(Shader) == gcSHADER_TYPE_TCS)
    {
        isTemp = gcvTRUE;
        type = gcSL_TEMP;
        offset = 1;
    }
    else if (GetShaderType(Shader) == gcSHADER_TYPE_TES)
    {
        isTemp = gcvFALSE;
        type = gcSL_ATTRIBUTE;
        offset = 0;
    }
    else
    {
        return gcvSTATUS_OK;
    }

    /* Finid gl_BoundingBox[]. */
    if (isTemp)
    {
        for (i = 0; i < Shader->outputCount; i++)
        {
            gcOUTPUT output = Shader->outputs[i];

            if (output && output->nameLength == gcSL_BOUNDING_BOX)
            {
                indexMin = output->tempIndex;
                indexMax = indexMin + offset;
                break;
            }
        }
    }
    else
    {
        for (i = 0; i < Shader->attributeCount; i++)
        {
            gcATTRIBUTE attribute = Shader->attributes[i];

            if (attribute && attribute->nameLength == gcSL_BOUNDING_BOX)
            {
                indexMin = attribute->index;
                indexMax = indexMin + offset;
                break;
            }
        }
    }

    if (indexMin == (gctUINT16)-1)
    {
        return gcvSTATUS_OK;
    }

    /*
    ** For TCS, change all assignments to NOPs and all value to 0.0.
    ** For TES, change all value to 0.0.
    */
    for (i = 0; i < Shader->codeCount; i++)
    {
        gcSL_INSTRUCTION code = &Shader->code[i];
        gcSL_OPCODE opcode;
        gctSOURCE_t source;
        gctUINT16 sourceIndex;
        gctUINT16 sourceIndexed;

        if (!code) continue;

        opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

        /* Check assignment. */
        if (isTemp &&
            (opcode != gcSL_CALL && opcode != gcSL_JMP))
        {
            if (code->tempIndex >= indexMin &&
                code->tempIndex <= indexMax)
            {
                code->opcode = gcmSL_OPCODE_SET(0, Opcode, gcSL_NOP);
                code->temp = code->tempIndex = code->tempIndexed = 0;
                code->source0 = code->source0Index = code->source0Indexed = 0;
                code->source1 = code->source1Index = code->source1Indexed = 0;
                continue;
            }
        }

        /* Check source0. */
        source = code->source0;
        sourceIndex = code->source0Index;
        sourceIndexed = code->source0Indexed;

        if ((gcSL_TYPE)gcmSL_SOURCE_GET(source, Type) == type &&
            (gcmSL_INDEX_GET(sourceIndex, Index) >= indexMin
             &&
             gcmSL_INDEX_GET(sourceIndex, Index) <= indexMax))
        {
            source = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                   | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                   | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
            sourceIndex = sourceIndexed = 0;
        }
        else if (isTemp &&
                 gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED &&
                 sourceIndex >= indexMin &&
                 sourceIndex <= indexMax)
        {
            source = gcmSL_SOURCE_SET(source, Indexed, gcSL_NOT_INDEXED);
            sourceIndexed = 0;
        }
        code->source0 = source;
        code->source0Index = sourceIndex;
        code->source0Indexed = sourceIndexed;

        /* Check source1. */
        source = code->source1;
        sourceIndex = code->source1Index;
        sourceIndexed = code->source1Indexed;

        if ((gcSL_TYPE)gcmSL_SOURCE_GET(source, Type) == type &&
            (gcmSL_INDEX_GET(sourceIndex, Index) >= indexMin
             &&
             gcmSL_INDEX_GET(sourceIndex, Index) <= indexMax))
        {
            source = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                   | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                   | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
            sourceIndex = sourceIndexed = 0;
        }
        else if (isTemp &&
                 gcmSL_SOURCE_GET(source, Indexed) != gcSL_NOT_INDEXED &&
                 sourceIndex >= indexMin &&
                 sourceIndex <= indexMax)
        {
            source = gcmSL_SOURCE_SET(source, Indexed, gcSL_NOT_INDEXED);
            sourceIndexed = 0;
        }
        code->source1 = source;
        code->source1Index = sourceIndex;
        code->source1Indexed = sourceIndexed;
    }

    return gcvSTATUS_OK;
}

/*******************************************************************************
**                                gcLinkShaders
********************************************************************************
**
**  Link two shaders and generate a harwdare specific state buffer by compiling
**  the compiler generated code through the resource allocator and code
**  generator.
**
**  INPUT:
**
**      gcSHADER VertexShader
**          Pointer to a gcSHADER object holding information about the compiled
**          vertex shader.
**
**      gcSHADER FragmentShader
**          Pointer to a gcSHADER object holding information about the compiled
**          fragment shader.
**
**      gceSHADER_FLAGS Flags
**          Compiler flags.  Can be any of the following:
**
**              gcvSHADER_DEAD_CODE       - Dead code elimination.
**              gcvSHADER_RESOURCE_USAGE  - Resource usage optimizaion.
**              gcvSHADER_OPTIMIZER       - Full optimization.
**              gcvSHADER_USE_GL_Z        - Use OpenGL ES Z coordinate.
**              gcvSHADER_USE_GL_POSITION - Use OpenGL ES gl_Position.
**              gcvSHADER_USE_GL_FACE     - Use OpenGL ES gl_FaceForward.
**
**          gcvSHADER_LOADTIME_OPTIMZATION is set if load-time optimizaiton
**          is needed
**
**  OUTPUT:
**
**      gctSIZE_T * StateBufferSize
**          Pointer to a variable receiving the number of bytes in the buffer
**          returned in 'StateBuffer'.
**
**      gctPOINTER * StateBuffer
**          Pointer to a variable receiving a buffer pointer that contains the
**          states required to download the shaders into the hardware.
**
**      gcsHINT_PTR * Hints
**          Pointer to a variable receiving a gcsHINT structure pointer that
**          contains information required when loading the shader states.
*/
gceSTATUS
gcLinkShaders(
    IN gcSHADER VertexShader,
    IN gcSHADER FragmentShader,
    IN gceSHADER_FLAGS Flags,
    OUT gctUINT32 * StateBufferSize,
    OUT gctPOINTER * StateBuffer,
    OUT gcsHINT_PTR * Hints
    )
{
    gcoOS os;
    gceSTATUS status = gcvSTATUS_OK;
    gcLINKTREE vertexTree = gcvNULL;
    gcLINKTREE fragmentTree = gcvNULL;
    gceSHADER_OPTIMIZATION opt;
    gctBOOL    dumpVertCGV = gcvFALSE;
    gctBOOL    dumpFragCGV = gcvFALSE;
    gctBOOL    dumpCGV;
    gctBOOL    enableDefaultUBO = gcvFALSE;
    gctBOOL uploadUBO = gcvFALSE;
    gcSHADER shader, vsTemp = gcvNULL, fsTemp = gcvNULL;
    gctUINT32_PTR vertexVersion = gcvNULL, fragmentVersion = gcvNULL;
    gctINT i;
    gctBOOL doPreVaryingPacking;
    gcSHADER    Shaders[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gcLINKTREE  linkTrees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gcLINKTREE* trees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};

    gcmHEADER_ARG("VertexShader=0x%x FragmentShader=0x%x Flags=%x",
                  VertexShader, FragmentShader, Flags);

    /* Verify the arguments. */
    if (VertexShader)
    {
        dumpVertCGV = gcSHADER_DumpCodeGenVerbose(VertexShader);
        gcmVERIFY_OBJECT(VertexShader, gcvOBJ_SHADER);
    }
    if (FragmentShader)
    {
        dumpFragCGV = gcSHADER_DumpCodeGenVerbose(FragmentShader);
        gcmVERIFY_OBJECT(FragmentShader, gcvOBJ_SHADER);
    }

    dumpCGV = dumpVertCGV || dumpFragCGV;

    /* Get the shader language version for VS and PS. */
    if (VertexShader)
    {
        gcSHADER_GetCompilerVersion(VertexShader, &vertexVersion);
        if (gcSHADER_DumpOptimizer(VertexShader))
        {
            gcOpt_Dump(gcvNULL, "Incoming Vertex Shader", gcvNULL, VertexShader);
        }
    }
    if (FragmentShader)
    {
        gcSHADER_GetCompilerVersion(FragmentShader, &fragmentVersion);
        if (gcSHADER_DumpOptimizer(FragmentShader))
        {
            gcOpt_Dump(gcvNULL, "Incoming Fragment Shader", gcvNULL, FragmentShader);
        }
    }

    /* check if the shaders linked together are the same versions
     *  Note: this may not be true for some version of shaders, check with spec!
     */
    if ((vertexVersion && fragmentVersion) && vertexVersion[1] != fragmentVersion[1])
    {
        return gcvSTATUS_SHADER_VERSION_MISMATCH;
    }

    /* if there is intrinsic builtin functions,
       compile the builtin libary and
       link the library functions into the shader*/
    Shaders[gceSGSK_VERTEX_SHADER] = VertexShader;
    Shaders[gceSGSK_FRAGMENT_SHADER] = FragmentShader;
    status = _LinkBuiltinLibs(Shaders);
    if (gcmIS_ERROR(status))
    {
        gcmFATAL("ERROR: Cannot Link builtin libs.");
        return status;
    }

    if (VertexShader)
        gcSHADER_ConvertSamplerAssignForParameter(VertexShader);
    if (FragmentShader)
        gcSHADER_ConvertSamplerAssignForParameter(FragmentShader);

    if (VertexShader)
    {
        status = _ChangeAtomicCounterUniform2BaseAddrBindingUniform(VertexShader);
        if (gcmIS_ERROR(status))
        {
            gcmFATAL("ERROR: Cannot create base binding uniform on vertex shader.");
            return status;
        }
    }

    if (FragmentShader)
    {
        status = _ChangeAtomicCounterUniform2BaseAddrBindingUniform(FragmentShader);
        if (gcmIS_ERROR(status))
        {
            gcmFATAL("ERROR: Cannot create base binding uniform on fragment shader.");
            return status;
        }

        /* HW does not support explicit helper-invocation to shader, to reduce complex of
           next stages of BE, just put helper-invocation reference at one place */
        _PreprocessHeplerInvocation(FragmentShader);
    }

    if (Flags & gcvSHADER_OPTIMIZER)
    {
        if (!VSC_OPTN_RAOptions_GetSwitchOn(VSC_OPTN_Options_GetRAOptions(VSC_OPTN_Get_Options())))
        {
            if (VertexShader)
            {
                _gcPackingFunctionArgument(VertexShader, dumpVertCGV);
            }

            if (FragmentShader)
            {
                _gcPackingFunctionArgument(FragmentShader, dumpFragCGV);
            }
        }

        if (VertexShader)
        {
            status = _gcLINKTREE_CreateColorOutput(VertexShader);
            if (gcmIS_ERROR(status))
            {
                gcmFATAL("ERROR: Cannot create _gcLINKTREE_CreateColorOutput on vertex shader.");
                return status;
            }
        }
    }

    if (FragmentShader)
    {
        status = _gcLINKTREE_ReplaceColor2FrontBackColor(FragmentShader);
        if (gcmIS_ERROR(status))
        {
            gcmFATAL("ERROR: Cannot create _gcLINKTREE_ReplaceColor2FrontBackColor on fragment shader.");
            return status;
        }
    }

    /* set the flag for optimizer and BE */
    gcSetOptimizerOption(Flags);
    opt = gcGetOptimizerOption()->optFlags;

    /* Initialize our pass manager who will take over all passes
       whether to trigger or not */
    {
#if VSC_BUILD
        VSC_OPTN_RAOptions* ra_options;
        gctBOOL isVirRAOn;

        ra_options          = VSC_OPTN_Options_GetRAOptions(VSC_OPTN_Get_Options());
        isVirRAOn           = VSC_OPTN_RAOptions_GetSwitchOn(ra_options);
        /* need to do pre varying packing if VIR RA is on, since VIR RA may pack
         * the varyings by itself and conflict with varyingPacking */
        doPreVaryingPacking = (gcmOPT_UseVIRCodeGen() == VIRCG_None || isVirRAOn);
#else
        doPreVaryingPacking = (gcmOPT_UseVIRCodeGen() != VIRCG_None);
#endif

        if (ENABLE_FULL_NEW_LINKER)
        {
            doPreVaryingPacking = gcvFALSE;
        }
    }

    /* Extract the gcoOS object pointer. */
    os = gcvNULL;

    if ((VertexShader && VertexShader->enableDefaultUBO) &&
        (FragmentShader && FragmentShader->enableDefaultUBO) &&
        gcmOPT_CreateDefaultUBO() != 0
        )
    {
        if (ENABLE_FULL_NEW_LINKER)
        {
            enableDefaultUBO = gcvFALSE;
        }
        else
        {
            enableDefaultUBO = gcvTRUE;
        }
    }
    else
    {
       if (VertexShader)
       {
           VertexShader->enableDefaultUBO = gcvFALSE;
       }

       if (FragmentShader)
       {
           FragmentShader->enableDefaultUBO = gcvFALSE;
       }
    }

    if (enableDefaultUBO &&
        gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1))
    {
        gctUINT maxVertexUniforms;
        gctUINT maxFragmentUniforms;
        gctUINT uniformsUsed = 0;
        gcSHADER shader;
        gctINT i;
        gcoHARDWARE hardware;
        gctBOOL IsUsedLoadInstruction[2] = {gcvFALSE, gcvFALSE};

        gcoHAL_GetHardware(gcvNULL, &hardware);
        gcmASSERT(hardware);

        gcmVERIFY_OK(gcoHAL_QueryShaderCaps(gcvNULL,
                                            gcvNULL,
                                            &maxVertexUniforms,
                                            &maxFragmentUniforms,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL));

        if (VertexShader && VertexShader->uniformCount > 0)
        {
            IsUsedLoadInstruction[0] = gcvTRUE;
        }
        if (FragmentShader && FragmentShader->uniformCount > 0)
        {
            IsUsedLoadInstruction[1] = gcvTRUE;
        }

        if (VertexShader && FragmentShader)
        {
            status = _gcCreateDefaultUBO(VertexShader, FragmentShader);
            if (gcmIS_ERROR(status))
            {
                return status;
            }
        }

        if(VertexShader && VertexShader->_defaultUniformBlockIndex != -1)
        {
            if (gcmIS_ERROR(status))
            {
                gcmFATAL("ERROR: Cannot change MOV uniform to LOAD for vertex shader \'0x%x\'",
                         VertexShader);
                return status;
            }

            status = _ManageUniformMembersInUBO(VertexShader,
                                                maxVertexUniforms,
                                                &uniformsUsed,
                                                &IsUsedLoadInstruction[0]);
            if (gcmIS_ERROR(status))
            {
                gcmFATAL("ERROR: Cannot properly perform management of uniform members within UBO for vertex shader \'0x%x\'",
                         VertexShader);
                return status;
            }
        }

        if(FragmentShader && FragmentShader->_defaultUniformBlockIndex != -1)
        {
            if (gcmIS_ERROR(status))
            {
                gcmFATAL("ERROR: Cannot change MOV uniform to LOAD for fragment shader \'0x%x\'",
                         FragmentShader);
                return status;
            }

            status = _ManageUniformMembersInUBO(FragmentShader,
                                                maxFragmentUniforms,
                                                gcvNULL,
                                                &IsUsedLoadInstruction[1]);
            if (gcmIS_ERROR(status))
            {
                gcmFATAL("ERROR: Cannot properly perform management of uniform members within UBO for vertex shader \'0x%x\'",
                         FragmentShader);
                return status;
            }
        }

        for(i = 0, shader = VertexShader; i < 2; i++, shader = FragmentShader)
        {
           /* Change load instruction of uniform block to mov from uniform register:
              when uniform is not UsedLoadInstruction */
            if (shader && IsUsedLoadInstruction[i])
            {
                gcmERR_BREAK(_gcChangeLoadToMovUniform(shader, gcvFALSE));
            }
        }
    }

    do
    {
        if(!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1) ||
           _ToUploadUBO(VertexShader, FragmentShader, &uploadUBO))
        {
            /* TODO -
            ** actually we should change load to mov for UBO as many as possible for all chips,
            ** it would reduce shader instruction count and improve performance.
            */
            for(i = 0, shader = VertexShader; i < 2; i++, shader = FragmentShader)
            {
                if (shader)
                {
                    gcmERR_BREAK(_gcChangeLoadToMovUniform(shader, gcvFALSE));
                }
            }
        }

        /*
        ** we need to do some conversion for integer branch.
        */
        if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SUPPORT_INTEGER_BRANCH) &&
            (Flags & gcvSHADER_ONCE_OPTIMIZER))
        {
            if (VertexShader)
            {
                gcmERR_BREAK(_ConvertIntegerBranchToFloat(VertexShader));
            }

            if (FragmentShader)
            {
                gcmERR_BREAK(_ConvertIntegerBranchToFloat(FragmentShader));
            }
        }

        /* Create a UBO that hold the constant uniforms generated by compiler.
        ** If the constant uniform is out of resource, move it to this UBO.
        */
        if (!ENABLE_FULL_NEW_LINKER)
        {
            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1) &&
                gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SHADER_ENHANCEMENTS2) &&
                !gcmOPT_NOIMMEDIATE())
            {
                if (VertexShader && VertexShader->replaceIndex == gcvMACHINECODE_COUNT)
                {
                    status = _gcCreateConstantUBO(VertexShader);
                    if (gcmIS_ERROR(status))
                    {
                        gcmFATAL("ERROR: Cannot create constant uniform block for vertex shader \'0x%x\'",
                                 VertexShader);
                        return status;
                    }
                }
                if (FragmentShader && FragmentShader->replaceIndex == gcvMACHINECODE_COUNT)
                {
                    status = _gcCreateConstantUBO(FragmentShader);
                    if (gcmIS_ERROR(status))
                    {
                        gcmFATAL("ERROR: Cannot create constant uniform block for fragment shader \'0x%x\'",
                                 FragmentShader);
                        return status;
                    }
                }
            }
        }

        /* VertexId and Transform Feedback*/
        if (VertexShader)
        {
            gctBOOL needVertexIDReversePatch
                = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_FE_START_VERTEX_SUPPORT)
                ? gcvTRUE
                : gcvFALSE;

            gctBOOL hwTFBsupport = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HW_TFB);

            /* Do vertexId patch */
            if (Flags & gcvSHADER_ONCE_OPTIMIZER)
            {
                gctBOOL xfbPatch = (VertexShader->transformFeedback.varyingCount > 0) && !hwTFBsupport;
                /* After this, both vertexID/instanceID are integer. */
                gcmERR_BREAK(gcAddVertexAndInstanceIdPatch(VertexShader, xfbPatch, &needVertexIDReversePatch));

                /* Do xfb patch */
                if (xfbPatch)
                {
                    gcmERR_BREAK(gcAddTransformFeedback(VertexShader, needVertexIDReversePatch));
                    if (dumpVertCGV)
                    {
                        gcOpt_Dump(gcvNULL, "After Transform Feedback", gcvNULL, VertexShader);
                    }
                }
            }
        }

        /* vertex shader full optimization */
        if (VertexShader)
        {
            /* Add a extra mov if target temp register the same as any of the source temp register. */
            gcmERR_BREAK(_splitInstructionHasSameDestAndSrcTempIndex(VertexShader));
            gcmERR_BREAK(gcSHADER_SetOptimizationOption(VertexShader, opt));
            gcmERR_BREAK(gcOptimizeShader(VertexShader, gcvNULL));
        }

        /* fragment shader full optimization */
        if (FragmentShader)
        {
            /* Add a extra mov if target temp register the same as any of the source temp register. */
            gcmERR_BREAK(_splitInstructionHasSameDestAndSrcTempIndex(FragmentShader));
            gcmERR_BREAK(gcSHADER_SetOptimizationOption(FragmentShader, opt));
            gcmERR_BREAK(gcOptimizeShader(FragmentShader, gcvNULL));
        }

        /* Remove some flags depending on the IP. */
        if ((Flags & gcvSHADER_USE_GL_Z) &&
            gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_USE_GL_Z) == gcvSTATUS_FALSE)
        {
            Flags &= ~gcvSHADER_USE_GL_Z;
        }

        if (VertexShader && FragmentShader)
        {
            gcmERR_BREAK(_gcCheckShadersVersion(VertexShader, FragmentShader));
            /* Special case for Fragment Shader using PointCoord. */
            gcmERR_BREAK(PatchShaders(VertexShader, FragmentShader));
        }

        if (Flags & gcvSHADER_OPTIMIZER)
        {
            if (VertexShader)
            {
                /* Optimize the jumps in the vertex shader. */
                gcmERR_BREAK(gcSHADER_OptimizeJumps(os, VertexShader));

                /* Compact the vertex shader. */
                gcmERR_BREAK(CompactShader(os, VertexShader));
            }

            if (FragmentShader)
            {
                /* Optimize the jumps in the fragment shader. */
                gcmERR_BREAK(gcSHADER_OptimizeJumps(os, FragmentShader));

                /* Compact the fragment shader. */
                gcmERR_BREAK(CompactShader(os, FragmentShader));
            }
        }

        if (VertexShader)
        {
            gcmERR_BREAK(_gcConvert32BitModulus(VertexShader, dumpVertCGV));
        }

        if (FragmentShader)
        {
            gcmERR_BREAK(_gcConvert32BitModulus(FragmentShader, dumpFragCGV));
        }

        if (_FindTexLodAndTexBias(VertexShader, FragmentShader))
        {
            Flags |= gcvSHADER_TEXLD_W;
        }

        if (VertexShader)
        {
            /* Build the vertex shader tree. */
            gcmERR_BREAK(gcLINKTREE_Construct(os, &vertexTree));
            /* Build the vertex shader tree. */
            gcmERR_BREAK(gcLINKTREE_Build(vertexTree, VertexShader, Flags));
            if (dumpVertCGV)
                _DumpLinkTree("Incoming vertex shader", vertexTree, gcvFALSE);

            /* Find model view project matrix for w-clip. */
            gcLINKTREE_FindModelViewProjection(vertexTree);
        }

        if (FragmentShader)
        {
            /* Build the fragment shader tree. */
            gcmERR_BREAK(gcLINKTREE_Construct(os, &fragmentTree));
            /* Build the fragment shader tree. */
            gcmERR_BREAK(gcLINKTREE_Build(fragmentTree, FragmentShader, Flags));
            if (dumpFragCGV)
                _DumpLinkTree("Incoming fragment shader", fragmentTree, gcvFALSE);

            /* Remove dead code from the fragment shader. */
            if (Flags & gcvSHADER_DEAD_CODE)
            {
                gcmERR_BREAK(gcLINKTREE_RemoveDeadCode(fragmentTree));
                if (dumpFragCGV)
                    _DumpLinkTree("Removed dead code from the fragment shader", fragmentTree, gcvFALSE);
            }
            else
            {
                /* Mark all temps and attributes as used. */
                gcmERR_BREAK(gcLINKTREE_MarkAllAsUsed(fragmentTree));
            }
        }

        if ((Flags & gcvSHADER_SEPERATED_PROGRAM) == 0)
        {
            /* delay varying packing till VIRCG finished */
            /* Link vertex and fragment shaders. */
            linkTrees[gceSGSK_VERTEX_SHADER] = vertexTree;
            linkTrees[gceSGSK_FRAGMENT_SHADER] = fragmentTree;
            gcmERR_BREAK(gcLINKTREE_Link(linkTrees, doPreVaryingPacking));
            if (vertexTree && fragmentTree && dumpCGV)
            {
                _DumpLinkTree("Linked vertex shader", vertexTree, gcvFALSE);
                _DumpLinkTree("Linked fragment shader", fragmentTree, gcvFALSE);
            }
        }

        /* Only continue if we need to do code generation. */
        if (StateBufferSize != gcvNULL)
        {
            if (VertexShader)
            {
                if (Flags & gcvSHADER_DEAD_CODE)
                {
                    /* Remove dead code from the vertex shader. */
                    gcmERR_BREAK(gcLINKTREE_RemoveDeadCode(vertexTree));
                    if (dumpVertCGV)
                        _DumpLinkTree("Removed dead code from the vertex shader", vertexTree, gcvFALSE);
                }
                else
                {
                    /* Mark all temps and attributes as used. */
                    gcmERR_BREAK(gcLINKTREE_MarkAllAsUsed(vertexTree));
                }
            }

            if (Flags & gcvSHADER_DEAD_CODE)
            {
                /* Remove unused attributes. */
                if (VertexShader)
                {
                    gcmERR_BREAK(gcLINKTREE_RemoveUnusedAttributes(vertexTree));
                    if (dumpVertCGV)
                        _DumpLinkTree("Remove unused attrib the vertex shader", vertexTree, gcvFALSE);
                }

                if (FragmentShader && VertexShader)
                {
                    gcmERR_BREAK(gcLINKTREE_RemoveUnusedAttributes(fragmentTree));
                    if (dumpFragCGV)
                        _DumpLinkTree("Remove unused attrib the fragment shader", fragmentTree, gcvFALSE);
                }
            }

            if (Flags & gcvSHADER_OPTIMIZER)
            {
                if (VertexShader)
                {
                    /* Recompute the vertex shader. */
                    gcmERR_BREAK(gcLINKTREE_ComputeSamplerPhysicalAddress(vertexTree));
                    if (dumpVertCGV)
                        _DumpLinkTree("Recompute the vertex shader", vertexTree, gcvFALSE);

                    /* Optimize the vertex shader by removing MOV instructions. */
                    gcmERR_BREAK(gcLINKTREE_Optimize(vertexTree));
                    if (dumpVertCGV)
                        _DumpLinkTree("Optimized the vertex shader", vertexTree, gcvFALSE);

                    /* Allocate const vector uniform for vertex tree */
                    gcmERR_BREAK(gcLINKTREE_AllocateConstantUniform(vertexTree));
                    if (dumpVertCGV)
                        _DumpLinkTree("Allocate const vector uniform for vertex tree.", vertexTree, gcvFALSE);
                }

                if (FragmentShader)
                {
                    /* Recompute the fragment shader. */
                    gcmERR_BREAK(gcLINKTREE_ComputeSamplerPhysicalAddress(fragmentTree));
                    if (dumpFragCGV)
                        _DumpLinkTree("Recompute the fragment shader", fragmentTree, gcvFALSE);

                    /* Optimize the fragment shader by removing MOV instructions. */
                    gcmERR_BREAK(gcLINKTREE_Optimize(fragmentTree));
                    if (dumpFragCGV)
                        _DumpLinkTree("Optimized the fragment shader", fragmentTree, gcvFALSE);

                    /* Allocate const vector uniform for fragment tree */
                    gcmERR_BREAK(gcLINKTREE_AllocateConstantUniform(fragmentTree));
                    if (dumpFragCGV)
                        _DumpLinkTree("Allocate const vector uniform for fragment tree.", fragmentTree, gcvFALSE);

                    /* Clean up the fragment shader. */
                    gcmERR_BREAK(gcLINKTREE_Cleanup(fragmentTree));
                    if (dumpFragCGV)
                        _DumpLinkTree("Cleaned up the fragment tree.", fragmentTree, gcvFALSE);
                }

                /* API level resource check. */
                gcmERR_BREAK(gcLINKTREE_CheckAPILevelResource(vertexTree));
                gcmERR_BREAK(gcLINKTREE_CheckAPILevelResource(fragmentTree));

                /* Convert CONV. */
                trees[gceSGSK_VERTEX_SHADER] = &vertexTree;
                trees[gceSGSK_FRAGMENT_SHADER] = &fragmentTree;
                gcmERR_BREAK(gcLINKTREE_ConvertCONV(trees, doPreVaryingPacking, gcvTRUE, dumpVertCGV));
            }

            /* go through VIR pass*/
            if (gcmOPT_UseVIRCodeGen() != VIRCG_None)
            {
                gctBOOL doVertexShader = (VertexShader &&
                                         gcSHADER_GoVIRPass(VertexShader));
                gctBOOL doFragmentShader = (FragmentShader &&
                                            gcSHADER_GoVIRPass(FragmentShader));

#if !USE_NEW_LINKER
                VIR_Shader *              vsShader = gcvNULL;
                VIR_Shader *              psShader = gcvNULL;
                VSC_PRIMARY_MEM_POOL      vsPmp;
                VSC_PRIMARY_MEM_POOL      psPmp;

                if (doVertexShader)
                {
                    gcmERR_BREAK(gcGoThroughVIRPass_Conv2VIR(&vertexTree, &vsPmp, &vsShader));
                    gcmERR_BREAK(gcGoThroughVIRPass_Compile(&vertexTree, vsShader));
                }
                if (doFragmentShader)
                {
                    gcmERR_BREAK(gcGoThroughVIRPass_Conv2VIR(&fragmentTree, &psPmp, &psShader));
                    gcmERR_BREAK(gcGoThroughVIRPass_Compile(&fragmentTree, psShader));
                }

                if(doVertexShader && doFragmentShader)
                {
                    VSC_PRIMARY_MEM_POOL allShadersPmp;
                    VSC_AllShaders all_shaders;
                    VSC_OPTN_UF_AUBO_Options* aubo_options;
                    char buffer[4096];
                    VIR_Dumper dumper;

                    gcoOS_ZeroMemory(&dumper, sizeof(dumper));
                    vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));

                    vscPMP_Intialize(&allShadersPmp, gcvNULL, 512*1024, sizeof(void *), gcvTRUE /*pooling*/);
                    VSC_AllShaders_Initialize(&all_shaders, vsShader, gcvNULL, gcvNULL, gcvNULL, psShader, gcvNULL, gcvNULL, &dumper, &allShadersPmp.mmWrapper);

                    /* */
                    VSC_AllShaders_LinkUniforms(&all_shaders);

                    /* Create Default UBO*/
                    aubo_options = VSC_OPTN_Options_GetAUBOOptions(VSC_OPTN_Get_Options());
                    if (VSC_OPTN_UF_AUBO_Options_GetSwitchOn(aubo_options) && !(Flags & gcvSHADER_DISABLE_DEFAULT_UBO))
                    {
                        VSC_UF_UtilizeAuxUBO(&all_shaders, aubo_options);
                    }

                    vscPMP_Finalize(&allShadersPmp);
                }
                if (doVertexShader)
                {
                    gcmERR_BREAK(gcGoThroughVIRPass_NewTree(&vertexTree, &vsPmp, vsShader));
                    _gcConstructDefaultUBO(VertexShader);
                }
                if (doFragmentShader)
                {
                    gcmERR_BREAK(gcGoThroughVIRPass_NewTree(&fragmentTree, &psPmp, psShader));
                    _gcConstructDefaultUBO(FragmentShader);
                }

                if (doVertexShader && doFragmentShader)
                {
                    /* Link vertex and fragment shaders, do varying packing this time. */
                    linkTrees[gceSGSK_VERTEX_SHADER] = vertexTree;
                    linkTrees[gceSGSK_FRAGMENT_SHADER] = fragmentTree;
                    gcmERR_BREAK(gcLINKTREE_Link(vertexTree, fragmentTree, !doPreVaryingPacking));
                    /* Remove unused attributes. */
                    gcmERR_BREAK(gcLINKTREE_RemoveUnusedAttributes(vertexTree));
                    gcmERR_BREAK(gcLINKTREE_RemoveUnusedAttributes(fragmentTree));

                    if (dumpCGV)
                    {
                        _DumpLinkTree("Linked vertex shader (after VIR pass)", vertexTree, gcvFALSE);
                        _DumpLinkTree("Linked fragment shader (after VIR pass)", fragmentTree, gcvFALSE);
                    }
                }
#else

                if (doVertexShader || doFragmentShader)
                {
                    gcLINKTREE* trees[gcMAX_SHADERS_IN_LINK_GOURP];
                    gctINT      i;
                    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i++) trees[i] = gcvNULL;

                    trees[gceSGSK_VERTEX_SHADER] = &vertexTree;
                    trees[gceSGSK_FRAGMENT_SHADER] = &fragmentTree;

                    if (ENABLE_FULL_NEW_LINKER)
                    {
                        status = gcLinkTreeThruVirShaders(trees, gcvFALSE, Flags,
                                                          doPreVaryingPacking, StateBufferSize, StateBuffer, Hints);
                        break;
                    }
                    else
                    {
                        gcmERR_BREAK(gcLinkTreeThruVirShaders(trees, gcvFALSE, Flags,
                                                              doPreVaryingPacking, gcvNULL, gcvNULL, gcvNULL));
                    }
                }
#endif
            }

            /* Check if we need to enable icache. */
            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SH_INSTRUCTION_PREFETCH))
            {
                gcmASSERT(gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SHADER_HAS_INSTRUCTION_CACHE));
                if (vertexTree)
                    vertexTree->useICache = gcvTRUE;
                if (fragmentTree)
                    fragmentTree->useICache = gcvTRUE;
            }
            else if (VertexShader && FragmentShader)
            {
                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SHADER_HAS_INSTRUCTION_CACHE))
                {
                    gcoHARDWARE hardware;

                    gcoHAL_GetHardware(gcvNULL, &hardware);
                    gcmASSERT(hardware);

                    if ((VertexShader->codeCount + FragmentShader->codeCount) >= (gctSIZE_T)hardware->config->instructionCount)
                    {
                        if (vertexTree)
                            vertexTree->useICache = gcvTRUE;
                        if (fragmentTree)
                            fragmentTree->useICache = gcvTRUE;
                    }
                    else
                    {
                        if (vertexTree)
                            vertexTree->useICache = gcvFALSE;
                        if (fragmentTree)
                            fragmentTree->useICache = gcvFALSE;
                    }

                }
                else
                {
                    if (vertexTree)
                        vertexTree->useICache = gcvFALSE;
                    if (fragmentTree)
                        fragmentTree->useICache = gcvFALSE;
                }
            }

            /* Generate vertex shader states. */
            if (VertexShader)
            {
                if(gcmOPT_UseVIRCodeGen() != VIRCG_None)
                {
                    gcmERR_BREAK(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_VERTEX, &vsTemp));
                    gcmERR_BREAK(gcSHADER_Copy(vsTemp, (gcSHADER)VertexShader));
                }

                if (gcSHADER_DumpFinalIR(VertexShader))
                {
                    gcDump_Shader(gcvNULL, "Final vertex shader IR.", gcvNULL, VertexShader, gcvTRUE);
                }

                if(vertexTree->hints)
                    vertexTree->hints->uploadedUBO = uploadUBO;

                gcmERR_BREAK(gcLINKTREE_GenerateStates(gcvNULL,
                                                       &vertexTree,
                                                       Flags,
                                                       gcvNULL,
                                                       gcvNULL,
                                                       StateBufferSize,
                                                       StateBuffer,
                                                       Hints));

                gcmERR_BREAK(gcSetUniformShaderKind(VertexShader));

                if(gcmOPT_UseVIRCodeGen() != VIRCG_None)
                {
                    /* Only copy insts */
                    if (vsTemp->lastInstruction > VertexShader->lastInstruction)
                    {
                        gcoOS_Free(gcvNULL, VertexShader->code);
                        gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION)* vsTemp->lastInstruction,
                                       (gctPOINTER*)&VertexShader->code);
                    }

                    if(vsTemp->lastInstruction > 0)
                    {
                        gcoOS_MemCopy(VertexShader->code, vsTemp->code, sizeof(struct _gcSL_INSTRUCTION)* vsTemp->lastInstruction);
                    }

                    gcmERR_BREAK(gcSHADER_Destroy(vsTemp));
                    vsTemp = gcvNULL;
                }
            }

            /* Generate fragment shader states. */
            if (FragmentShader)
            {
                if(gcmOPT_UseVIRCodeGen() != VIRCG_None)
                {
                    gcmERR_BREAK(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_FRAGMENT, &fsTemp));
                    gcmERR_BREAK(gcSHADER_Copy(fsTemp, (gcSHADER)FragmentShader));
                }

                if (gcSHADER_DumpFinalIR(FragmentShader))
                {
                    gcDump_Shader(gcvNULL, "Final fragment shader IR.", gcvNULL, FragmentShader, gcvTRUE);
                }

                if(fragmentTree->hints)
                    fragmentTree->hints->uploadedUBO = uploadUBO;

                gcmERR_BREAK(gcLINKTREE_GenerateStates(gcvNULL,
                                                       &fragmentTree,
                                                       Flags,
                                                       gcvNULL,
                                                       gcvNULL,
                                                       StateBufferSize,
                                                       StateBuffer,
                                                       Hints));

                gcmERR_BREAK(gcSetUniformShaderKind(FragmentShader));

                if (Hints && *Hints && fragmentTree->hints)
                {
                    (*Hints)->psHasDiscard = fragmentTree->hints->psHasDiscard;
                }
                else
                {
                    (*Hints)->psHasDiscard = gcvFALSE;
                }

                if(gcmOPT_UseVIRCodeGen() != VIRCG_None)
                {
                    /* Only copy insts */
                    if (fsTemp->lastInstruction > FragmentShader->lastInstruction)
                    {
                        gcoOS_Free(gcvNULL, FragmentShader->code);
                        gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION)* fsTemp->lastInstruction,
                                       (gctPOINTER*)&FragmentShader->code);
                    }

                    if(fsTemp->lastInstruction > 0)
                    {
                        gcoOS_MemCopy(FragmentShader->code, fsTemp->code, sizeof(struct _gcSL_INSTRUCTION)* fsTemp->lastInstruction);
                    }

                    gcmERR_BREAK(gcSHADER_Destroy(fsTemp));
                    fsTemp = gcvNULL;
                }
            }

            /* Check instruction limit. */
            if (Hints && *Hints && (*Hints)->unifiedStatus.instruction &&
                VertexShader && FragmentShader && !vertexTree->useICache)
            {
                if ((*Hints)->vsInstCount + (*Hints)->fsInstCount > (*Hints)->maxInstCount)
                {
                    status = gcvSTATUS_TOO_MANY_INSTRUCTION;
                    break;
                }
            }

            /* Check uniform limit. */
            if (Hints && *Hints && (*Hints)->unifiedStatus.constant &&
                VertexShader && FragmentShader)
            {
                if ((*Hints)->vsConstCount + (*Hints)->fsConstCount > (*Hints)->maxConstCount)
                {
                    status = gcvSTATUS_TOO_MANY_UNIFORMS;
                    break;
                }
            }
        }
    }
    while (gcvFALSE);

    if (vsTemp != gcvNULL)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(vsTemp));
    }

    if (fsTemp != gcvNULL)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(fsTemp));
    }

    if (vertexTree != gcvNULL)
    {
        /* Destroy the vertex shader tree. */
        gcmVERIFY_OK(gcLINKTREE_Destroy(vertexTree));
    }

    if (fragmentTree != gcvNULL)
    {
        /* Destroy the fragment shader tree. */
        gcmVERIFY_OK(gcLINKTREE_Destroy(fragmentTree));
    }

    /* Return the status. */
    if (gcmIS_SUCCESS(status) && (StateBufferSize != gcvNULL))
    {
        gcmFOOTER_ARG("*StateBufferSize=%lu *StateBuffer=0x%x *Hints=0x%x",
                                  *StateBufferSize, *StateBuffer, *Hints);
    }
    else
    {
        /* Free state buffer if link failed. */
        if (*StateBuffer != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *StateBuffer));
        }

        /* Free up Hints. */
        if (*Hints != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *Hints));
        }
        gcmFOOTER();
    }
    return status;
}

gceSTATUS
_gcLinkFullGraphicsShaders(
    IN gcSHADER* Shaders, /* Indexed by gcsSHADER_GROUP_SHADER_KIND */
    IN gceSHADER_FLAGS Flags,
    OUT gctUINT32 * StateBufferSize,
    OUT gctPOINTER * StateBuffer,
    OUT gcsHINT_PTR * Hints
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoOS os;
    gceSHADER_OPTIMIZATION opt;
    gctBOOL    dumpCGV[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gctBOOL    dumpAllCGV = gcvFALSE;
    gctUINT32_PTR shVersion = gcvNULL, thisShVersion;
    gcLINKTREE    shaderTrees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gcLINKTREE*   tempShaderTrees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gctINT i;
    gctUINT offset = 0;
    gctBOOL firstShot = gcvFALSE, doPreVaryingPacking = gcvFALSE;
    gctCHAR     headsup[256];

    gctCONST_STRING shaderName[] =
    {
        "vertex",
        "compute",
        "tess control",
        "tess evaluate",
        "geometry",
        "fragment"
    };

    gcmHEADER_ARG("Shaders=0x%x Flags=%x", Shaders, Flags);

    firstShot = gcvFALSE;
    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (Shaders[i])
        {
            /* Verify the arguments. */
            dumpCGV[i] = gcSHADER_DumpCodeGenVerbose(Shaders[i]);
            gcmVERIFY_OBJECT(Shaders[i], gcvOBJ_SHADER);

            dumpAllCGV |= dumpCGV[i];

            gcmONERROR(gcSHADER_GetCompilerVersion(Shaders[i], &thisShVersion));
            if (gcSHADER_DumpOptimizer(Shaders[i]))
            {
                offset = 0;
                gcoOS_PrintStrSafe(headsup, sizeof(headsup), &offset,
                                   "Incoming %s Shader", shaderName[i]);
                gcOpt_Dump(gcvNULL, headsup, gcvNULL, Shaders[i]);
            }

            /* check if the shaders linked together are the same versions
             *  Note: this may not be true for some version of shaders, check with spec!
             */
            if (firstShot)
            {
                if ((shVersion && thisShVersion) && shVersion[1] != thisShVersion[1])
                {
                    return gcvSTATUS_SHADER_VERSION_MISMATCH;
                }
            }
            else
            {
                shVersion = thisShVersion;
                firstShot = gcvTRUE;
            }
        }
    }

    /* if there is intrinsic builtin functions,
       compile the builtin libary and
       link the library functions into the shader*/
    gcmONERROR(_LinkBuiltinLibs(Shaders));

    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (Shaders[i])
        {
            /*
            ** Ignore all references to the gl_BoundingBox in TS since we don't support it.
            */
            gcmONERROR(_IgnoreAllReferenceToBoundingBox(Shaders[i]));

            gcmONERROR(gcSHADER_ConvertSamplerAssignForParameter(Shaders[i]));

            status = _ChangeAtomicCounterUniform2BaseAddrBindingUniform(Shaders[i]);
            if (gcmIS_ERROR(status))
            {
                offset = 0;
                gcoOS_PrintStrSafe(headsup, sizeof(headsup), &offset,
                                   "ERROR: Cannot create base binding uniform on %s shader.", shaderName[i]);

                gcmFATAL(headsup);
                return status;
            }

            if (i == gceSGSK_FRAGMENT_SHADER)
            {
                /* HW does not support explicit helper-invocation to shader, to reduce complex of
                   next stages of BE, just put helper-invocation reference at one place */
                _PreprocessHeplerInvocation(Shaders[i]);
            }

            if (Flags & gcvSHADER_OPTIMIZER)
            {
                if (!VSC_OPTN_RAOptions_GetSwitchOn(VSC_OPTN_Options_GetRAOptions(VSC_OPTN_Get_Options())))
                {
                    _gcPackingFunctionArgument(Shaders[i], dumpCGV[i]);
                }

                if (i == gceSGSK_VERTEX_SHADER)
                {
                    status = _gcLINKTREE_CreateColorOutput(Shaders[i]);
                    if (gcmIS_ERROR(status))
                    {
                        gcmFATAL("ERROR: Cannot create _gcLINKTREE_CreateColorOutput on vertex shader.");
                        return status;
                    }
                }
            }

            if (i == gceSGSK_FRAGMENT_SHADER)
            {
                status = _gcLINKTREE_ReplaceColor2FrontBackColor(Shaders[i]);
                if (gcmIS_ERROR(status))
                {
                    gcmFATAL("ERROR: Cannot create _gcLINKTREE_ReplaceColor2FrontBackColor on fragment shader.");
                    return status;
                }
            }
        }
    }

    /* set the flag for optimizer and BE */
    gcSetOptimizerOption(Flags);
    opt = gcGetOptimizerOption()->optFlags;

    /* Extract the gcoOS object pointer. */
    os = gcvNULL;

    /*
    ** we need to do some conversion for integer branch.
    */
    if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SUPPORT_INTEGER_BRANCH) &&
        (Flags & gcvSHADER_ONCE_OPTIMIZER))
    {
        for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
        {
            if (Shaders[i])
            {
                gcmONERROR(_ConvertIntegerBranchToFloat(Shaders[i]));
            }
        }
    }

    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (Shaders[i])
        {
            if (i == gceSGSK_VERTEX_SHADER)
            {
                /* Do vertexId patch */
                if (Flags & gcvSHADER_ONCE_OPTIMIZER)
                {
                    /* After this, both vertexID/instanceID are integer. */
                    gcmONERROR(gcAddVertexAndInstanceIdPatch(Shaders[i], gcvFALSE, gcvNULL));
                }
            }

            /* Add a extra mov if target temp register the same as any of the source temp register. */
            gcmONERROR(_splitInstructionHasSameDestAndSrcTempIndex(Shaders[i]));
            gcmONERROR(gcSHADER_SetOptimizationOption(Shaders[i], opt));
            gcmONERROR(gcOptimizeShader(Shaders[i], gcvNULL));
        }
    }

    /* Remove some flags depending on the IP. */
    if ((Flags & gcvSHADER_USE_GL_Z) &&
        gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_USE_GL_Z) == gcvSTATUS_FALSE)
    {
        Flags &= ~gcvSHADER_USE_GL_Z;
    }

    if (Shaders[gceSGSK_FRAGMENT_SHADER])
    {
        if (Shaders[gceSGSK_GEOMETRY_SHADER])
        {
            gcmONERROR(PatchShaders(Shaders[gceSGSK_GEOMETRY_SHADER], Shaders[gceSGSK_FRAGMENT_SHADER]));
        }
        else if (Shaders[gceSGSK_TC_SHADER])
        {
            gcmONERROR(PatchShaders(Shaders[gceSGSK_TC_SHADER], Shaders[gceSGSK_FRAGMENT_SHADER]));
        }
        else if (Shaders[gceSGSK_VERTEX_SHADER])
        {
            gcmONERROR(PatchShaders(Shaders[gceSGSK_VERTEX_SHADER], Shaders[gceSGSK_FRAGMENT_SHADER]));
        }
    }

    if (Flags & gcvSHADER_OPTIMIZER)
    {
        for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
        {
            if (Shaders[i])
            {
                /* Optimize the jumps in shader. */
                gcmONERROR(gcSHADER_OptimizeJumps(os, Shaders[i]));

                /* Compact shader. */
                gcmONERROR(CompactShader(os, Shaders[i]));
            }
        }
    }

    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (Shaders[i])
        {
            gcmONERROR(_gcConvert32BitModulus(Shaders[i], dumpCGV[i]));

            /* Build the vertex shader tree. */
            gcmONERROR(gcLINKTREE_Construct(os, &shaderTrees[i]));
            /* Build the vertex shader tree. */
            gcmONERROR(gcLINKTREE_Build(shaderTrees[i], Shaders[i], Flags));
            if (dumpCGV[i])
            {
                offset = 0;
                gcoOS_PrintStrSafe(headsup, sizeof(headsup), &offset,
                                   "Incoming %s Shader", shaderName[i]);
                _DumpLinkTree(headsup, shaderTrees[i], gcvFALSE);
            }

            if (i == gceSGSK_VERTEX_SHADER)
            {
                /* Find model view project matrix for w-clip. */
                gcLINKTREE_FindModelViewProjection(shaderTrees[i]);
            }

            if (i != gceSGSK_VERTEX_SHADER)
            {
                /* Remove dead code from the fragment shader. */
                if (Flags & gcvSHADER_DEAD_CODE)
                {
                    gcmONERROR(gcLINKTREE_RemoveDeadCode(shaderTrees[i]));
                    if (dumpCGV[i])
                        _DumpLinkTree("Removed dead code from the fragment shader", shaderTrees[i], gcvFALSE);
                }
                else
                {
                    /* Mark all temps and attributes as used. */
                    gcmONERROR(gcLINKTREE_MarkAllAsUsed(shaderTrees[i]));
                }
            }
        }
    }

    /* Link shaders. */
    if ((Flags & gcvSHADER_SEPERATED_PROGRAM) == 0)
    {
        gcmONERROR(gcLINKTREE_Link(shaderTrees, doPreVaryingPacking));
    }

    /* set TES input vertex count based on TCS output vertex count */
    if (Shaders[gceSGSK_TC_SHADER] &&
        Shaders[gceSGSK_TE_SHADER])
    {
        Shaders[gceSGSK_TE_SHADER]->shaderLayout.tes.tessPatchInputVertices =
            Shaders[gceSGSK_TC_SHADER]->shaderLayout.tcs.tcsPatchOutputVertices;
    }

    if (StateBufferSize != gcvNULL)
    {
        for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
        {
            if (Shaders[i])
            {
                if (i == gceSGSK_VERTEX_SHADER)
                {
                    if (Flags & gcvSHADER_DEAD_CODE)
                    {
                        /* Remove dead code from the vertex shader. */
                        gcmONERROR(gcLINKTREE_RemoveDeadCode(shaderTrees[i]));
                        if (dumpCGV[i])
                            _DumpLinkTree("Removed dead code from the vertex shader", shaderTrees[i], gcvFALSE);
                    }
                    else
                    {
                        /* Mark all temps and attributes as used. */
                        gcmONERROR(gcLINKTREE_MarkAllAsUsed(shaderTrees[i]));
                    }
                }

                if (Flags & gcvSHADER_DEAD_CODE)
                {
                    gcmONERROR(gcLINKTREE_RemoveUnusedAttributes(shaderTrees[i]));
                    if (dumpCGV[i])
                    {
                        offset = 0;
                        gcoOS_PrintStrSafe(headsup, sizeof(headsup), &offset,
                                           "Remove unused attrib the %s shader", shaderName[i]);
                        _DumpLinkTree(headsup, shaderTrees[i], gcvFALSE);
                    }
                }

                if (Flags & gcvSHADER_OPTIMIZER)
                {
                    /* Recompute the vertex shader. */
                    gcmONERROR(gcLINKTREE_ComputeSamplerPhysicalAddress(shaderTrees[i]));
                    if (dumpCGV[i])
                    {
                        offset = 0;
                        gcoOS_PrintStrSafe(headsup, sizeof(headsup), &offset,
                                           "Recompute the %s shader", shaderName[i]);
                        _DumpLinkTree(headsup, shaderTrees[i], gcvFALSE);
                    }

                    /* Optimize the vertex shader by removing MOV instructions. */
                    gcmONERROR(gcLINKTREE_Optimize(shaderTrees[i]));
                    if (dumpCGV[i])
                    {
                        offset = 0;
                        gcoOS_PrintStrSafe(headsup, sizeof(headsup), &offset,
                                           "Optimized the %s shader", shaderName[i]);
                        _DumpLinkTree(headsup, shaderTrees[i], gcvFALSE);
                    }

                    /* Allocate const vector uniform for vertex tree */
                    gcmONERROR(gcLINKTREE_AllocateConstantUniform(shaderTrees[i]));
                    if (dumpCGV[i])
                    {
                        offset = 0;
                        gcoOS_PrintStrSafe(headsup, sizeof(headsup), &offset,
                                           "Allocate const vector uniform for %s tree", shaderName[i]);
                        _DumpLinkTree(headsup, shaderTrees[i], gcvFALSE);
                    }

                    if (i == gceSGSK_FRAGMENT_SHADER)
                    {
                        /* Clean up the fragment shader. */
                        gcmONERROR(gcLINKTREE_Cleanup(shaderTrees[i]));
                        if (dumpCGV[i])
                            _DumpLinkTree("Cleaned up the fragment tree.", shaderTrees[i], gcvFALSE);
                    }

                    /* API level resource check. */
                    gcmONERROR(gcLINKTREE_CheckAPILevelResource(shaderTrees[i]));
                }

                tempShaderTrees[i] = &shaderTrees[i];
            }
        }

        /* Convert CONV. */
        gcmONERROR(gcLINKTREE_ConvertCONV(tempShaderTrees, doPreVaryingPacking, gcvFALSE, gcvFALSE));

        gcmONERROR(gcLinkTreeThruVirShaders(tempShaderTrees, gcvFALSE,
                                            Flags, doPreVaryingPacking,
                                            StateBufferSize, StateBuffer, Hints));
    }

OnError:
    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (shaderTrees[i])
        {
            /* Destroy the shader tree. */
            gcmVERIFY_OK(gcLINKTREE_Destroy(shaderTrees[i]));
        }
    }

    if (gcmIS_SUCCESS(status) && (StateBufferSize != gcvNULL))
    {
        /* Return the status. */
        gcmFOOTER_ARG("*StateBufferSize=%lu *StateBuffer=0x%x *Hints=0x%x",
                                    *StateBufferSize, *StateBuffer, *Hints);
    }
    else
    {
        /* Free state buffer if link failed. */
        if (*StateBuffer != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *StateBuffer));
        }

        /* Free up Hints. */
        if (*Hints != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *Hints));
        }
        gcmFOOTER();
    }

    return status;
}

/*******************************************************************************
**                                gcLinkKernel
********************************************************************************
**
**    Link OpenCL kernel and generate a hardware specific state buffer by compiling
**    the compiler generated code through the resource allocator and code
**    generator.
**
**    INPUT:
**
**        gcSHADER Kernel
**            Pointer to a gcSHADER object holding information about the compiled
**            OpenCL kernel.
**
**        gceSHADER_FLAGS Flags
**            Compiler flags.  Can be any of the following:
**
**                gcvSHADER_DEAD_CODE       - Dead code elimination.
**                gcvSHADER_RESOURCE_USAGE  - Resource usage optimizaion.
**                gcvSHADER_OPTIMIZER       - Full optimization.
**
**          gcvSHADER_LOADTIME_OPTIMZATION is set if load-time optimizaiton
**          is needed
**
**    OUTPUT:
**
**        gctSIZE_T * StateBufferSize
**            Pointer to a variable receiving the number of bytes in the buffer
**            returned in 'StateBuffer'.
**
**        gctPOINTER * StateBuffer
**            Pointer to a variable receiving a buffer pointer that contains the
**            states required to download the shaders into the hardware.
**
**        gcsHINT_PTR * Hints
**            Pointer to a variable receiving a gcsHINT structure pointer that
**            contains information required when loading the shader states.
*/
gceSTATUS
gcLinkKernel(
    IN gcSHADER             Kernel,
    IN gceSHADER_FLAGS      Flags,
    OUT gctUINT32 *         StateBufferSize,
    OUT gctPOINTER *        StateBuffer,
    OUT gcsHINT_PTR *       Hints
    )
{
    gceSTATUS               status;
    gcLINKTREE              kernelTree = gcvNULL;
    gceSHADER_OPTIMIZATION  opt;
    gctBOOL                 dumpCGVerbose, newCGDone = gcvFALSE;
    gcSHADER    Shaders[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};

    gcmHEADER_ARG("Kernel=0x%x Flags=%x",
                  Kernel, Flags);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Kernel, gcvOBJ_SHADER);

    /* set the flag for optimizer and BE */
    gcSetOptimizerOption(Flags);
    opt = gcGetOptimizerOption()->optFlags;
    dumpCGVerbose = gcSHADER_DumpCodeGenVerbose(Kernel);

    if (gcSHADER_DumpOptimizer(Kernel))
    {
        gcOpt_Dump(gcvNULL, "Incoming Shader", gcvNULL, Kernel);
    }

    /* if there is intrinsic builtin functions,
       compile the builtin libary and
       link the library functions into the shader*/
    Shaders[gceSGSK_CL_SHADER] = Kernel;
    status = _LinkBuiltinLibs(Shaders);
    if (gcmIS_ERROR(status))
    {
        gcmFATAL("ERROR: Cannot Link builtin libs.");
        return status;
    }

    /* Extract the gcoOS object pointer. */

    if (gcSHADER_CheckBugFixes10())
    {
        /* Full optimization. */
        gcmVERIFY_OK(gcSHADER_SetOptimizationOption(Kernel, opt));
    }
    else
    {
        gcmVERIFY_OK(gcSHADER_SetOptimizationOption(Kernel,
                           opt | gcvOPTIMIZATION_LOAD_SW_W));
    }

    if (gcmOPT_CreateDefaultUBO() &&
        Kernel->uniformBlockCount &&
        gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1))
    {
        gctUINT maxVertexUniforms;
        gctUINT maxFragmentUniforms;
        gctUINT uniformsUsed = 0;
        gcoHARDWARE hardware;
        gctBOOL isUsedLoadInstruction = Kernel->uniformCount ? gcvTRUE : gcvFALSE;

        gcoHAL_GetHardware(gcvNULL, &hardware);
        gcmASSERT(hardware);

        Kernel->enableDefaultUBO = gcvTRUE;

        gcmVERIFY_OK(gcoHAL_QueryShaderCaps(gcvNULL,
                                            gcvNULL,
                                            &maxVertexUniforms,
                                            &maxFragmentUniforms,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL,
                                            gcvNULL));


        status = _ManageUniformMembersInUBO(Kernel,
                                            maxVertexUniforms + maxFragmentUniforms,
                                            &uniformsUsed,
                                            &isUsedLoadInstruction);
        if (gcmIS_ERROR(status))
        {
            gcmFATAL("ERROR: Cannot properly perform management of uniform members within UBO for OPENCL kernel shader \'0x%x\'",
                     Kernel);
            gcmONERROR(status);
        }

       /* Change load instruction of uniform block to mov from uniform register:
          when uniform is not UsedLoadInstruction */
        if (isUsedLoadInstruction)
        {
            gcmONERROR(_gcOCL_ChangeLoadToMovUniform(Kernel));
        }

        if (gcSHADER_DumpOptimizer(Kernel))
        {
            gcOpt_Dump(gcvNULL, "After UBO Transformation", gcvNULL, Kernel);
        }
    }

    gcmONERROR(_splitInstructionHasSameDestAndSrcTempIndex(Kernel));

    /* Optimize kernel shader. */
    /*gcmVERIFY_OK(gcSHADER_SetOptimizationOption(Kernel, gcvOPTIMIZATION_NONE));*/
    gcmONERROR(gcOptimizeShader(Kernel, gcvNULL));

    /* Clear kernel functions arguments number. */
    Kernel->maxKernelFunctionArgs = 0;

    if (Flags & gcvSHADER_IMAGE_PATCHING)
    {
        /* Read_image patching. */
        /* Create image-sampler pairs, */
        /*    and replace IMAGE_SAMPLER-IMAGE_RD pairs with TEXLD. */
        gcmONERROR(_converrtImageReadToTexld(Kernel));
    }

    if (Flags & gcvSHADER_OPTIMIZER)
    {
        /* Optimize the jumps in the kernel shader. */
        gcmONERROR(gcSHADER_OptimizeJumps(gcvNULL, Kernel));

        /* Compact the kernel shader. */
        gcmONERROR(CompactShader(gcvNULL, Kernel));
    }

    /* Construct the kernel shader tree. */
    gcmONERROR(gcLINKTREE_Construct(gcvNULL, &kernelTree));

    /* Build the kernel shader tree. */
    gcmONERROR(gcLINKTREE_Build(kernelTree, Kernel, Flags));
    if (dumpCGVerbose)
        _DumpLinkTree("Incoming kernel shader", kernelTree, gcvFALSE);
    /*_FindPositionAttribute(kernelTree);*/

    /* Only continue if we need to do code generation. */
    if (StateBufferSize != gcvNULL)
    {
        if (Flags & gcvSHADER_DEAD_CODE)
        {
            /* Remove dead code from the kernel shader. */
            gcmONERROR(gcLINKTREE_RemoveDeadCode(kernelTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Removed dead code from the kernel shader", kernelTree, gcvFALSE);
        }
        else
        {
            /* Mark all temps and attributes as used. */
            gcmONERROR(gcLINKTREE_MarkAllAsUsed(kernelTree));
        }

        if (Flags & gcvSHADER_OPTIMIZER)
        {
            /* Optimize the kernel shader by removing MOV instructions. */
            gcmONERROR(gcLINKTREE_Optimize(kernelTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Optimized the kernel shader", kernelTree, gcvFALSE);

            /* Allocate const vector uniform for kernel tree */
            gcmONERROR(gcLINKTREE_AllocateConstantUniform(kernelTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Cleaned up the kernel tree.", kernelTree, gcvFALSE);

            /* Clean up the kernel shader. */
            gcmONERROR(gcLINKTREE_Cleanup(kernelTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Cleaned up the kernel tree.", kernelTree, gcvFALSE);

            /* API level resource check. */
            gcmONERROR(gcLINKTREE_CheckAPILevelResource(kernelTree));
        }

        /* go through VIR pass*/
        if (gcSHADER_GoVIRPass(Kernel))
        {
#if !USE_NEW_LINKER
            gcmONERROR(gcGoThroughVIRPass(&kernelTree));
            if (dumpCGVerbose)
                _DumpLinkTree("After VIR kernel tree.", kernelTree, gcvFALSE);
#else
            gcLINKTREE* trees[gcMAX_SHADERS_IN_LINK_GOURP];
            gctINT      i;
            for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i++) trees[i] = gcvNULL;

            trees[gceSGSK_CL_SHADER] = &kernelTree;

            if (ENABLE_FULL_NEW_LINKER)
            {
                gcmONERROR(gcLinkTreeThruVirShaders(trees, gcvTRUE, Flags,
                                                    gcvFALSE, StateBufferSize, StateBuffer, Hints));
                newCGDone = gcvTRUE;
            }
            else
            {
                gcmONERROR(gcLinkTreeThruVirShaders(trees, gcvTRUE, Flags,
                                                    gcvFALSE, gcvNULL, gcvNULL, gcvNULL));
                newCGDone = gcvFALSE;
            }
#endif
        }

        if (!newCGDone)
        {
            if (Flags & gcvSHADER_DEAD_CODE)
            {
                gcmONERROR(gcLINKTREE_RemoveUnusedAttributes(kernelTree));
                if (dumpCGVerbose)
                    _DumpLinkTree("Remove unused attributes for the kernel tree.", kernelTree, gcvFALSE);
            }

            if (gcSHADER_DumpFinalIR(Kernel))
            {
                gcDump_Shader(gcvNULL, "Final kernel shader IR.", gcvNULL, Kernel, gcvTRUE);
            }

            /* Generate kernel shader states. */
            gcmONERROR(gcLINKTREE_GenerateStates(gcvNULL,
                                                 &kernelTree,
                                                 Flags,
                                                 gcvNULL,
                                                 gcvNULL,
                                                 StateBufferSize,
                                                 StateBuffer,
                                                 Hints));

            gcmONERROR(gcSetUniformShaderKind(Kernel));
        }
    }

    if (kernelTree)
    {
        /* Destroy the kernel shader tree. */
        gcmVERIFY_OK(gcLINKTREE_Destroy(kernelTree));
    }

    /* Return the status. */
    gcmFOOTER_ARG("*StateBufferSize=%lu *StateBuffer=0x%x *Hints=0x%x",
                  gcmOPT_VALUE(StateBufferSize), gcmOPT_VALUE(StateBuffer), gcmOPT_VALUE(Hints));
    return status;

OnError:
    if (kernelTree != gcvNULL)
    {
        /* Destroy the kernel shader tree. */
        gcmVERIFY_OK(gcLINKTREE_Destroy(kernelTree));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

static gceSTATUS
_checkComputeShaderSanity(
    IN gcSHADER             ComputeShader
    )
{
    gceSTATUS               status = gcvSTATUS_OK;

    /* check if work group size is specified [4.4.1.1 Compute Shader Inputs] :
     * if a program object contains any compute shaders, at least one must
     * contain an input layout qualifier specifying a fixed local group size
     * for the program, or a link-time error will occur.
     */
    if (ComputeShader->shaderLayout.compute.workGroupSize[0] == 0 ||
        ComputeShader->shaderLayout.compute.workGroupSize[1] == 0 ||
        ComputeShader->shaderLayout.compute.workGroupSize[2] == 0 )
    {
        status = gcvSTATUS_CS_NO_WORKGROUP_SIZE;
        gcmONERROR(status);
    }
    else
    {
    }
OnError:
    /* Return the status. */
    return status;
}


/*******************************************************************************
**                                _gcLinkComputeShader
********************************************************************************
**
**    Link compute shader and generate a hardware specific state buffer by compiling
**    the compiler generated code through the resource allocator and code
**    generator.
**
**    INPUT:
**
**        gcSHADER ComputeShader
**            Pointer to a gcSHADER object holding information about the compiled
**            compute shader.
**
**        gceSHADER_FLAGS Flags
**            Compiler flags.  Can be any of the following:
**
**                gcvSHADER_DEAD_CODE       - Dead code elimination.
**                gcvSHADER_RESOURCE_USAGE  - Resource usage optimizaion.
**                gcvSHADER_OPTIMIZER       - Full optimization.
**
**          gcvSHADER_LOADTIME_OPTIMZATION is set if load-time optimizaiton
**          is needed
**
**    OUTPUT:
**
**        gctSIZE_T * StateBufferSize
**            Pointer to a variable receiving the number of bytes in the buffer
**            returned in 'StateBuffer'.
**
**        gctPOINTER * StateBuffer
**            Pointer to a variable receiving a buffer pointer that contains the
**            states required to download the shaders into the hardware.
**
**        gcsHINT_PTR * Hints
**            Pointer to a variable receiving a gcsHINT structure pointer that
**            contains information required when loading the shader states.
*/
static gceSTATUS
_gcLinkComputeShader(
    IN gcSHADER             ComputeShader,
    IN gceSHADER_FLAGS      Flags,
    OUT gctUINT32 *         StateBufferSize,
    OUT gctPOINTER *        StateBuffer,
    OUT gcsHINT_PTR *       Hints
    )
{
    gceSTATUS               status;
    gcLINKTREE              computeTree = gcvNULL;
    gceSHADER_OPTIMIZATION  opt;
    gctBOOL                 dumpCGVerbose, newCGDone = gcvFALSE;
    gcSHADER                Shaders[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gcLINKTREE              *trees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};

    gcmHEADER_ARG("ComputeShader=0x%x Flags=%x",
                  ComputeShader, Flags);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(ComputeShader, gcvOBJ_SHADER);

    /* set the flag for optimizer and BE */
    gcSetOptimizerOption(Flags);
    opt = gcGetOptimizerOption()->optFlags;
    dumpCGVerbose = gcSHADER_DumpCodeGenVerbose(ComputeShader);

    if (ComputeShader)
    {
        if (gcSHADER_DumpOptimizer(ComputeShader))
        {
            gcOpt_Dump(gcvNULL, "Incoming Compute Shader", gcvNULL, ComputeShader);
        }
    }

    /* Extract the gcoOS object pointer. */

    /* check shader sanity */
    gcmONERROR(_checkComputeShaderSanity(ComputeShader));

    if (gcSHADER_CheckBugFixes10())
    {
        /* Full optimization. */
        gcmVERIFY_OK(gcSHADER_SetOptimizationOption(ComputeShader, opt));
    }
    else
    {
        gcmVERIFY_OK(gcSHADER_SetOptimizationOption(ComputeShader,
                           opt | gcvOPTIMIZATION_LOAD_SW_W));
    }

    /* if there is intrinsic builtin functions,
       compile the builtin libary and
       link the library functions into the shader*/
    Shaders[gceSGSK_COMPUTE_SHADER] = ComputeShader;
    gcmONERROR(_LinkBuiltinLibs(Shaders));

    gcmONERROR(gcSHADER_ConvertSamplerAssignForParameter(ComputeShader));

    if ((Flags & gcvSHADER_OPTIMIZER) && ComputeShader &&
        !VSC_OPTN_RAOptions_GetSwitchOn(VSC_OPTN_Options_GetRAOptions(VSC_OPTN_Get_Options())))
    {
        _gcPackingFunctionArgument(ComputeShader, dumpCGVerbose);
    }

    if (!ENABLE_FULL_NEW_LINKER)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1) &&
            gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SHADER_ENHANCEMENTS2) &&
            !gcmOPT_NOIMMEDIATE())
        {
            if (ComputeShader && ComputeShader->replaceIndex == gcvMACHINECODE_COUNT)
            {
                status = _gcCreateConstantUBO(ComputeShader);
                if (gcmIS_ERROR(status))
                {
                    gcmFATAL("ERROR: Cannot create constant uniform block for compute shader \'0x%x\'",
                                ComputeShader);
                    return status;
                }
            }
        }
    }

    gcmONERROR(_splitInstructionHasSameDestAndSrcTempIndex(ComputeShader));

    /* Optimize compute shader. */
    /*gcmVERIFY_OK(gcSHADER_SetOptimizationOption(ComputeShader, gcvOPTIMIZATION_NONE));*/
    gcmONERROR(gcOptimizeShader(ComputeShader, gcvNULL));

    if (Flags & gcvSHADER_OPTIMIZER)
    {
        /* Optimize the jumps in the compute shader. */
        gcmONERROR(gcSHADER_OptimizeJumps(gcvNULL, ComputeShader));

        /* Compact the compute shader. */
        gcmONERROR(CompactShader(gcvNULL, ComputeShader));
    }

    /* Create base binding uniform for atomic counter */
    gcmONERROR(_ChangeAtomicCounterUniform2BaseAddrBindingUniform(ComputeShader));

    /* Convert 32bit mod. */
    gcmONERROR(_gcConvert32BitModulus(ComputeShader, dumpCGVerbose));

    /* Construct the compute shader tree. */
    gcmONERROR(gcLINKTREE_Construct(gcvNULL, &computeTree));

    /* Build the compute shader tree. */
    gcmONERROR(gcLINKTREE_Build(computeTree, ComputeShader, Flags));
    if (dumpCGVerbose)
        _DumpLinkTree("Incoming compute shader", computeTree, gcvFALSE);
    /*_FindPositionAttribute(computeTree);*/

    /* Only continue if we need to do code generation. */
    if (StateBufferSize != gcvNULL)
    {
        if (Flags & gcvSHADER_DEAD_CODE)
        {
            /* Remove dead code from the compute shader. */
            gcmONERROR(gcLINKTREE_RemoveDeadCode(computeTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Removed dead code from the compute shader", computeTree, gcvFALSE);
        }
        else
        {
            /* Mark all temps and attributes as used. */
            gcmONERROR(gcLINKTREE_MarkAllAsUsed(computeTree));
        }

        if (Flags & gcvSHADER_OPTIMIZER)
        {
            gcmONERROR(gcLINKTREE_ComputeSamplerPhysicalAddress(computeTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Recompute the sampler address", computeTree, gcvFALSE);

            /* Optimize the compute shader by removing MOV instructions. */
            gcmONERROR(gcLINKTREE_Optimize(computeTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Optimized the compute shader", computeTree, gcvFALSE);

            /* Allocate const vector uniform for compute tree */
            gcmONERROR(gcLINKTREE_AllocateConstantUniform(computeTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Allocate Constant Uniform for compute tree.", computeTree, gcvFALSE);

            /* Clean up the compute shader. */
            gcmONERROR(gcLINKTREE_Cleanup(computeTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Cleaned up the compute tree.", computeTree, gcvFALSE);
        }

        if (Flags & gcvSHADER_DEAD_CODE)
        {
            gcmONERROR(gcLINKTREE_RemoveUnusedAttributes(computeTree));
            if (dumpCGVerbose)
                _DumpLinkTree("Remove unused attributes for the compute tree.", computeTree, gcvFALSE);
        }

        gcmONERROR(gcLINKTREE_CheckAPILevelResource(computeTree));

        /* Convert CONV. */
        trees[gceSGSK_COMPUTE_SHADER] = &computeTree;
        gcmONERROR(gcLINKTREE_ConvertCONV(trees, gcvFALSE, gcvFALSE, dumpCGVerbose));

        /* go through VIR pass*/
        if (gcmOPT_UseVIRCodeGen() != VIRCG_None)
        {
#if !USE_NEW_LINKER
            VIR_Shader* computeShader;
            VSC_PRIMARY_MEM_POOL pmp;
            gcmONERROR(gcGoThroughVIRPass_Conv2VIR(&computeTree, &pmp, &computeShader));
            gcmONERROR(gcGoThroughVIRPass_Compile(&computeTree, computeShader));

            {
                VSC_PRIMARY_MEM_POOL allShadersPmp;
                VSC_AllShaders all_shaders;
                VSC_OPTN_UF_AUBO_Options* aubo_options;
                char buffer[4096];
                VIR_Dumper dumper;

                gcoOS_ZeroMemory(&dumper, sizeof(dumper));
                vscDumper_Initialize(&dumper.baseDumper, gcvNULL, gcvNULL, buffer, sizeof(buffer));

                vscPMP_Intialize(&allShadersPmp, gcvNULL, 512*1024, sizeof(void *), gcvTRUE /*pooling*/);
                VSC_AllShaders_Initialize(&all_shaders, gcvNULL, gcvNULL, gcvNULL, gcvNULL, gcvNULL, computeShader, gcvNULL, &dumper, &allShadersPmp.mmWrapper);

                /* */
                VSC_AllShaders_LinkUniforms(&all_shaders);

                /* Create Default UBO*/
                aubo_options = VSC_OPTN_Options_GetAUBOOptions(VSC_OPTN_Get_Options());
                if (VSC_OPTN_UF_AUBO_Options_GetSwitchOn(aubo_options))
                {
                    VSC_UF_UtilizeAuxUBO(&all_shaders, aubo_options);
                }

                vscPMP_Finalize(&allShadersPmp);
            }
            gcmONERROR(gcGoThroughVIRPass_NewTree(&computeTree, &pmp, computeShader));
            _gcConstructDefaultUBO(ComputeShader);
#else
            gcLINKTREE* trees[gcMAX_SHADERS_IN_LINK_GOURP];
            gctINT      i;
            for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i++) trees[i] = gcvNULL;

            trees[gceSGSK_COMPUTE_SHADER] = &computeTree;

            if (ENABLE_FULL_NEW_LINKER)
            {
                gcmONERROR(gcLinkTreeThruVirShaders(trees, gcvFALSE, Flags,
                                                    gcvFALSE, StateBufferSize, StateBuffer, Hints));
                newCGDone = gcvTRUE;
            }
            else
            {
                gcmONERROR(gcLinkTreeThruVirShaders(trees, gcvFALSE, Flags,
                                                    gcvFALSE, gcvNULL, gcvNULL, gcvNULL));
                newCGDone = gcvFALSE;
            }
#endif
        }

        if (!newCGDone)
        {
            if (gcSHADER_DumpFinalIR(ComputeShader))
            {
                gcDump_Shader(gcvNULL, "Final compute shader IR.", gcvNULL, ComputeShader, gcvTRUE);
            }

            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SH_INSTRUCTION_PREFETCH))
            {
                computeTree->useICache = gcvTRUE;
            }
            else if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SHADER_HAS_INSTRUCTION_CACHE))
            {
                gcoHARDWARE hardware;

                gcoHAL_GetHardware(gcvNULL, &hardware);
                gcmASSERT(hardware);

                if (ComputeShader->codeCount >= (gctSIZE_T)hardware->config->instructionCount)
                {
                    computeTree->useICache = gcvTRUE;
                }
                else
                {
                    computeTree->useICache = gcvFALSE;
                }
            }
            /* Generate kernel shader states. */
            gcmONERROR(gcLINKTREE_GenerateStates(gcvNULL,
                                                 &computeTree,
                                                 Flags,
                                                 gcvNULL,
                                                 gcvNULL,
                                                 StateBufferSize,
                                                 StateBuffer,
                                                 Hints));
        }

        gcmONERROR(gcSetUniformShaderKind(ComputeShader));
    }

    if (computeTree != gcvNULL)
    {
        /* Destroy the compute shader tree. */
        gcmVERIFY_OK(gcLINKTREE_Destroy(computeTree));
    }

    /* Return the status. */
    gcmFOOTER_ARG("*StateBufferSize=%lu *StateBuffer=0x%x *Hints=0x%x",
                  gcmOPT_VALUE(StateBufferSize), gcmOPT_VALUE(StateBuffer), gcmOPT_VALUE(Hints));
    return status;

OnError:
    if (computeTree != gcvNULL)
    {
        /* Destroy the compute shader tree. */
        gcmVERIFY_OK(gcLINKTREE_Destroy(computeTree));
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

void
_checkSetShaderGroup(
    IN gctINT               ShaderCount,
    IN gcSHADER *           ShaderArray,
    IN gcsShaderGroup *     ShaderGroup)
{
    gctINT i;

    gcoOS_ZeroMemory(ShaderGroup, sizeof(gcsShaderGroup));
    ShaderGroup->validGroup = gcvTRUE;
    for (i=0; i < ShaderCount; i++)
    {
        if (ShaderArray[i] == gcvNULL)
        {
            continue;
        }

        gcmASSERT(ShaderArray[i]->object.type  == gcvOBJ_SHADER ||
                  ShaderArray[i]->object.type  == gcvOBJ_VIR_SHADER);
        if (ShaderArray[i]->object.type  == gcvOBJ_SHADER)
        {
            switch (ShaderArray[i]->type) {
            case gcSHADER_TYPE_VERTEX:
                if (ShaderGroup->shaderGroup[gceSGSK_VERTEX_SHADER] != gcvNULL)
                {
                    ShaderGroup->validGroup = gcvFALSE;
                    return;
                }
                ShaderGroup->shaderGroup[gceSGSK_VERTEX_SHADER] = ShaderArray[i];
                break;
            case gcSHADER_TYPE_FRAGMENT:
                if (ShaderGroup->shaderGroup[gceSGSK_FRAGMENT_SHADER] != gcvNULL)
                {
                    ShaderGroup->validGroup = gcvFALSE;
                    return;
                }
                ShaderGroup->shaderGroup[gceSGSK_FRAGMENT_SHADER] = ShaderArray[i];
                break;
            case gcSHADER_TYPE_TCS:
                if (ShaderGroup->shaderGroup[gceSGSK_TC_SHADER] != gcvNULL)
                {
                    ShaderGroup->validGroup = gcvFALSE;
                    return;
                }
                ShaderGroup->shaderGroup[gceSGSK_TC_SHADER] = ShaderArray[i];
                break;
            case gcSHADER_TYPE_TES:
                if (ShaderGroup->shaderGroup[gceSGSK_TE_SHADER] != gcvNULL)
                {
                    ShaderGroup->validGroup = gcvFALSE;
                    return;
                }
                ShaderGroup->shaderGroup[gceSGSK_TE_SHADER] = ShaderArray[i];
                break;
            case gcSHADER_TYPE_GEOMETRY:
                if (ShaderGroup->shaderGroup[gceSGSK_GEOMETRY_SHADER] != gcvNULL)
                {
                    ShaderGroup->validGroup = gcvFALSE;
                    return;
                }
                ShaderGroup->shaderGroup[gceSGSK_GEOMETRY_SHADER] = ShaderArray[i];
                break;
            case gcSHADER_TYPE_CL:
                if (ShaderGroup->shaderGroup[gceSGSK_CL_SHADER] != gcvNULL)
                {
                    ShaderGroup->validGroup = gcvFALSE;
                    return;
                }
                ShaderGroup->shaderGroup[gceSGSK_CL_SHADER] = ShaderArray[i];
                ShaderGroup->hasComputeOrCLShader = gcvTRUE;
                break;
            case gcSHADER_TYPE_COMPUTE:
                if (ShaderGroup->shaderGroup[gceSGSK_COMPUTE_SHADER] != gcvNULL)
                {
                    ShaderGroup->validGroup = gcvFALSE;
                    return;
                }
                ShaderGroup->shaderGroup[gceSGSK_COMPUTE_SHADER] = ShaderArray[i];
                ShaderGroup->hasComputeOrCLShader = gcvTRUE;
                break;
            default:
                ShaderGroup->validGroup = gcvFALSE;
                return;
            }
        }
        else if (ShaderArray[i]->object.type  == gcvOBJ_VIR_SHADER)
        {
            /* TODO: handle vir shaders */
            ShaderGroup->validGroup = gcvFALSE;
            break;
        }
        else
        {
            ShaderGroup->validGroup = gcvFALSE;
            break;
        }
    }
    if (ShaderGroup->hasComputeOrCLShader &&
        (ShaderGroup->shaderGroup[gceSGSK_VERTEX_SHADER] != gcvNULL ||
         ShaderGroup->shaderGroup[gceSGSK_FRAGMENT_SHADER] != gcvNULL ||
         ShaderGroup->shaderGroup[gceSGSK_TC_SHADER] != gcvNULL ||
         ShaderGroup->shaderGroup[gceSGSK_TE_SHADER] != gcvNULL ||
         ShaderGroup->shaderGroup[gceSGSK_GEOMETRY_SHADER] != gcvNULL))
    {
        ShaderGroup->validGroup = gcvFALSE;
    }
    return;
}

static gceSTATUS
_verifyNonArrayedPerVertex(
    IN gcSHADER             Shader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 i;

    if (Shader->type == gcSHADER_TYPE_TCS ||
        Shader->type == gcSHADER_TYPE_TES ||
        Shader->type == gcSHADER_TYPE_GEOMETRY)
    {
        for (i = 0; i < Shader->attributeCount; i++)
        {
            gcATTRIBUTE attr = Shader->attributes[i];

            if (attr != gcvNULL && gcmATTRIBUTE_isPerVertexNotArray(attr))
            {
                status = gcvSTATUS_VARYING_TYPE_MISMATCH;
                return status;
            }
        }
    }

    if (Shader->type == gcSHADER_TYPE_TCS)
    {
        for (i = 0; i < Shader->outputCount; i++)
        {
            gcOUTPUT output = Shader->outputs[i];

            if (output != gcvNULL && gcmOUTPUT_isPerVertexNotArray(output))
            {
                status = gcvSTATUS_VARYING_TYPE_MISMATCH;
                return status;
            }
        }
    }

    return status;
}

static gceSTATUS
_verifyNotStagesRelatedError(
    IN gcsShaderGroup       ShaderGroup
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;

    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (ShaderGroup.shaderGroup[i] == gcvNULL)
        {
            continue;
        }

        if (gcmIS_ERROR(ShaderGroup.shaderGroup[i]->hasNotStagesRelatedLinkError))
        {
            return ShaderGroup.shaderGroup[i]->hasNotStagesRelatedLinkError;
        }

        /* verify non-arrayed per-vertex variables. */
        status = _verifyNonArrayedPerVertex(ShaderGroup.shaderGroup[i]);
        if (gcmIS_ERROR(status))
        {
            return status;
        }
    }

    return status;
}

/*******************************************************************************
**                                gcLinkProgram
********************************************************************************
**
**    Link a list of shaders and generate a hardware specific state buffer by compiling
**    the compiler generated code through the resource allocator and code
**    generator.
**
**    INPUT:
**        gctINT ShaderCount
**            number of gcSHADER object in the shader array
**
**        gcSHADER *ShaderArray
**            Array of gcSHADER object holding information about the compiled
**            shader.
**
**        gceSHADER_FLAGS Flags
**            Compiler flags.  Can be any of the following:
**
**                gcvSHADER_DEAD_CODE       - Dead code elimination.
**                gcvSHADER_RESOURCE_USAGE  - Resource usage optimizaion.
**                gcvSHADER_OPTIMIZER       - Full optimization.
**
**          gcvSHADER_LOADTIME_OPTIMZATION is set if load-time optimizaiton
**          is needed
**
**    OUTPUT:
**
**        gctSIZE_T * StateBufferSize
**            Pointer to a variable receiving the number of bytes in the buffer
**            returned in 'StateBuffer'.
**
**        gctPOINTER * StateBuffer
**            Pointer to a variable receiving a buffer pointer that contains the
**            states required to download the shaders into the hardware.
**
**        gcsHINT_PTR * Hints
**            Pointer to a variable receiving a gcsHINT structure pointer that
**            contains information required when loading the shader states.
*/
gceSTATUS
gcLinkProgram(
    IN gctINT               ShaderCount,
    IN gcSHADER *           ShaderArray,
    IN gceSHADER_FLAGS      Flags,
    OUT gctUINT32 *         StateBufferSize,
    OUT gctPOINTER *        StateBuffer,
    OUT gcsHINT_PTR *       Hints
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gcsShaderGroup          shGroup;
    gcmHEADER_ARG("ShaderCount=%d ShaderArray=0x%x Flags=%x",
                  ShaderCount, ShaderArray, Flags);

    if (ShaderCount > gcMAX_SHADERS_IN_PROGRAM)
    {
        gcmFOOTER();
        return gcvSTATUS_TOO_MANY_SHADERS;
    }

    /* init shader group */
    gcoOS_ZeroMemory(&shGroup,
                     gcmSIZEOF(gcsShaderGroup));

    _checkSetShaderGroup(ShaderCount, ShaderArray, &shGroup);
    if (shGroup.validGroup != gcvTRUE)
    {
        gcmFOOTER();
        return gcvSTATUS_LINK_INVALID_SHADERS;
    }

    status = _verifyNotStagesRelatedError(shGroup);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    if (shGroup.hasComputeOrCLShader)
    {
        status = _gcLinkComputeShader(shGroup.shaderGroup[gceSGSK_COMPUTE_SHADER],
                                        Flags,
                                        StateBufferSize,
                                        StateBuffer,
                                        Hints);
    }
    else
    {
        /* for now we only have vertex and fragment shader */
        if (shGroup.shaderGroup[gceSGSK_TC_SHADER] == gcvNULL &&
            shGroup.shaderGroup[gceSGSK_TE_SHADER] == gcvNULL &&
            shGroup.shaderGroup[gceSGSK_GEOMETRY_SHADER] == gcvNULL)
        {
            status = gcLinkShaders(shGroup.shaderGroup[gceSGSK_VERTEX_SHADER],
                                   shGroup.shaderGroup[gceSGSK_FRAGMENT_SHADER],
                                   Flags,
                                   StateBufferSize,
                                   StateBuffer,
                                   Hints
                                   );
        }
        else
        {
            status = _gcLinkFullGraphicsShaders(shGroup.shaderGroup,
                                                Flags,
                                                StateBufferSize,
                                                StateBuffer,
                                                Hints
                                                );
        }
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
_LinkProgramPipeline(
    IN gcSHADER VertexShader,
    IN gcSHADER FragmentShader,
    OUT gctUINT32 * StateBufferSize,
    OUT gctPOINTER * StateBuffer,
    OUT gcsHINT_PTR * Hints)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcLINKTREE vertexTree = gcvNULL;
    gcLINKTREE fragmentTree = gcvNULL;
    gcLINKTREE linkTrees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gcLINKTREE* trees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gcSHADER vsTemp = gcvNULL, fsTemp = gcvNULL;
    gceSHADER_FLAGS Flags =  (gcvSHADER_RESOURCE_USAGE         |
                              gcvSHADER_OPTIMIZER              |
                              gcvSHADER_USE_GL_POINT_COORD     |
                              gcvSHADER_USE_GL_POSITION        |
                              gcvSHADER_REMOVE_UNUSED_UNIFORMS);

    gcmHEADER_ARG("VertexShader=0x%x FragmentShader=0x%x",
                  VertexShader, FragmentShader);

    gcmASSERT(VertexShader && FragmentShader);

    do
    {
        gcmERR_BREAK(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_VERTEX, &vsTemp));
        gcmERR_BREAK(gcSHADER_Copy(vsTemp, (gcSHADER)VertexShader));

        gcmERR_BREAK(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_FRAGMENT, &fsTemp));
        gcmERR_BREAK(gcSHADER_Copy(fsTemp, (gcSHADER)FragmentShader));

        gcmERR_BREAK(_gcCheckShadersVersion(vsTemp, fsTemp));

        /* Special case for Fragment Shader using PointCoord. */
        gcmERR_BREAK(PatchShaders(vsTemp, fsTemp));

        gcmERR_BREAK(gcLINKTREE_Construct(gcvNULL, &vertexTree));
        gcmERR_BREAK(gcLINKTREE_Build(vertexTree, vsTemp, Flags));
        gcLINKTREE_FindModelViewProjection(vertexTree);
        if (gcShaderHwRegAllocated(vsTemp))
        {
            gcmERR_BREAK(gcLINKTREE_MarkAllAsUsedwithRA(vertexTree));
        }
        else
        {
            gcmERR_BREAK(gcLINKTREE_MarkAllAsUsed(vertexTree));
        }

        gcmERR_BREAK(gcLINKTREE_Construct(gcvNULL, &fragmentTree));
        gcmERR_BREAK(gcLINKTREE_Build(fragmentTree, fsTemp, Flags));
        if (gcShaderHwRegAllocated(fsTemp))
        {
            gcmERR_BREAK(gcLINKTREE_MarkAllAsUsedwithRA(fragmentTree));
        }
        else
        {
            gcmERR_BREAK(gcLINKTREE_MarkAllAsUsed(fragmentTree));
        }

        /* Fill tree pointers. */
        trees[gceSGSK_VERTEX_SHADER] = &vertexTree;
        trees[gceSGSK_FRAGMENT_SHADER] = &fragmentTree;
        linkTrees[gceSGSK_VERTEX_SHADER] = vertexTree;
        linkTrees[gceSGSK_FRAGMENT_SHADER] = fragmentTree;

        /* Convert CONV. */
        gcmERR_BREAK(gcLINKTREE_ConvertCONV(trees, gcvTRUE, gcvFALSE, gcvFALSE));

        if (ENABLE_FULL_NEW_LINKER)
        {
            gcmERR_BREAK(gcLinkTreeThruVirShaders(trees, gcvFALSE, Flags,
                                                    gcvFALSE, StateBufferSize, StateBuffer, Hints));
        }
        else
        {
            gcmERR_BREAK(gcLINKTREE_Link(linkTrees, gcvTRUE));

            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SHADER_HAS_INSTRUCTION_CACHE))
            {
                gcoHARDWARE hardware;

                gcoHAL_GetHardware(gcvNULL, &hardware);
                gcmASSERT(hardware);

                if (((vsTemp->codeCount + fsTemp->codeCount) >= (gctSIZE_T)hardware->config->instructionCount) ||
                    (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SH_INSTRUCTION_PREFETCH))
                    )
                {
                    if (vertexTree)
                        vertexTree->useICache = gcvTRUE;
                    if (fragmentTree)
                        fragmentTree->useICache = gcvTRUE;
                }
                else
                {
                    if (vertexTree)
                        vertexTree->useICache = gcvFALSE;
                    if (fragmentTree)
                        fragmentTree->useICache = gcvFALSE;
                }
            }
            else
            {
                if (vertexTree)
                    vertexTree->useICache = gcvFALSE;
                if (fragmentTree)
                    fragmentTree->useICache = gcvFALSE;
            }

            gcmERR_BREAK(gcLINKTREE_GenerateStates(gcvNULL,
                                                   &vertexTree,
                                                   Flags,
                                                   gcvNULL,
                                                   gcvNULL,
                                                   StateBufferSize,
                                                   StateBuffer,
                                                   Hints));

            gcmERR_BREAK(gcSetUniformShaderKind(VertexShader));

            gcmERR_BREAK(gcLINKTREE_GenerateStates(gcvNULL,
                                                   &fragmentTree,
                                                   Flags,
                                                   gcvNULL,
                                                   gcvNULL,
                                                   StateBufferSize,
                                                   StateBuffer,
                                                   Hints));

            gcmERR_BREAK(gcSetUniformShaderKind(FragmentShader));
        }
    }
    while (gcvFALSE);

    if (vertexTree != gcvNULL)
    {
        /* Destroy the vertex shader tree. */
        gcmVERIFY_OK(gcLINKTREE_Destroy(vertexTree));
    }

    if (fragmentTree != gcvNULL)
    {
        /* Destroy the fragment shader tree. */
        gcmVERIFY_OK(gcLINKTREE_Destroy(fragmentTree));
    }

    if (vsTemp != gcvNULL)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(vsTemp));
    }

    if (fsTemp != gcvNULL)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(fsTemp));
    }

    /* Return the status. */
    if (gcmIS_SUCCESS(status) && (StateBufferSize != gcvNULL))
    {
        gcmFOOTER_ARG("*StateBufferSize=%lu *StateBuffer=0x%x *Hints=0x%x",
                                  *StateBufferSize, *StateBuffer, *Hints);
    }
    else
    {
        /* Free state buffer if link failed. */
        if (*StateBuffer != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *StateBuffer));
        }

        /* Free up Hints. */
        if (*Hints != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *Hints));
        }
        gcmFOOTER();
    }
    return status;
}

gceSTATUS
_LinkFullGraphicsProgramPipeline(
    IN gcSHADER* Shaders, /* Indexed by gcsSHADER_GROUP_SHADER_KIND */
    OUT gctUINT32 * StateBufferSize,
    OUT gctPOINTER * StateBuffer,
    OUT gcsHINT_PTR * Hints)
{
    gceSTATUS       status = gcvSTATUS_OK;
    gcLINKTREE      shaderTrees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gcSHADER        tempShaders[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gcLINKTREE*     tempShaderTrees[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gctBOOL         firstShot = gcvFALSE;
    gctUINT32_PTR   shVersion = gcvNULL, thisShVersion;
    gctUINT         j, k;
    gctINT          i;
    gceSHADER_FLAGS Flags =  (gcvSHADER_RESOURCE_USAGE         |
                              gcvSHADER_OPTIMIZER              |
                              gcvSHADER_USE_GL_Z               |
                              gcvSHADER_USE_GL_POINT_COORD     |
                              gcvSHADER_USE_GL_POSITION        |
                              gcvSHADER_REMOVE_UNUSED_UNIFORMS);

    gcSHADER_KIND shaderKind[] =
    {
        gcSHADER_TYPE_VERTEX,
        gcSHADER_TYPE_COMPUTE,
        gcSHADER_TYPE_TCS,
        gcSHADER_TYPE_TES,
        gcSHADER_TYPE_GEOMETRY,
        gcSHADER_TYPE_FRAGMENT
    };

    gcmHEADER_ARG("Shaders=0x%x", Shaders);

    firstShot = gcvFALSE;
    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (Shaders[i])
        {
            gcmONERROR(gcSHADER_GetCompilerVersion(Shaders[i], &thisShVersion));

            /* check if the shaders linked together are the same versions
             *  Note: this may not be true for some version of shaders, check with spec!
             */
            if (firstShot)
            {
                if ((shVersion && thisShVersion) && shVersion[1] != thisShVersion[1])
                {
                    return gcvSTATUS_SHADER_VERSION_MISMATCH;
                }
            }
            else
            {
                shVersion = thisShVersion;
                firstShot = gcvTRUE;
            }

            gcmONERROR(gcSHADER_Construct(gcvNULL, shaderKind[i], &tempShaders[i]));
            gcmONERROR(gcSHADER_Copy(tempShaders[i], (gcSHADER)Shaders[i]));
        }
    }

    /* If pre-RA shader has transform-feedback, then bound it */
    for (i = gceSGSK_GEOMETRY_SHADER; i >= gceSGSK_VERTEX_SHADER; i --)
    {
        if (Shaders[i] && Shaders[i]->transformFeedback.varyingCount > 0)
        {
            gcmASSERT(Shaders[i]->transformFeedback.varyings);

            tempShaders[i]->transformFeedback.varyingCount = Shaders[i]->transformFeedback.varyingCount;
            tempShaders[i]->transformFeedback.bufferMode = Shaders[i]->transformFeedback.bufferMode;
            tempShaders[i]->transformFeedback.stateUniform = gcvNULL;
            tempShaders[i]->transformFeedback.feedbackBuffer.interleavedBufUniform = gcvNULL;
            tempShaders[i]->transformFeedback.varRegInfos = gcvNULL;
            tempShaders[i]->transformFeedback.shaderTempCount = Shaders[i]->transformFeedback.shaderTempCount;
            tempShaders[i]->transformFeedback.totalSize = Shaders[i]->transformFeedback.totalSize;

            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      gcmSIZEOF(gcsTFBVarying) * Shaders[i]->transformFeedback.varyingCount,
                                      (gctPOINTER*)&tempShaders[i]->transformFeedback.varyings));

            for (j = 0; j < Shaders[i]->transformFeedback.varyingCount; ++j)
            {
                if (Shaders[i]->transformFeedback.varyings[j].name)
                {
                    gctUINT nameLength = gcoOS_StrLen(Shaders[i]->transformFeedback.varyings[j].name, gcvNULL) + 1;
                    gcmONERROR(gcoOS_Allocate(gcvNULL,
                                              nameLength,
                                              (gctPOINTER*)&tempShaders[i]->transformFeedback.varyings[j].name));
                    gcoOS_MemCopy(tempShaders[i]->transformFeedback.varyings[j].name,
                                  Shaders[i]->transformFeedback.varyings[j].name,
                                  nameLength);

                    tempShaders[i]->transformFeedback.varyings[j].arraySize = Shaders[i]->transformFeedback.varyings[j].arraySize;
                    tempShaders[i]->transformFeedback.varyings[j].isWholeTFBed = Shaders[i]->transformFeedback.varyings[j].isWholeTFBed;
                    tempShaders[i]->transformFeedback.varyings[j].isArray = Shaders[i]->transformFeedback.varyings[j].isArray;

                    for (k = 0; k < Shaders[i]->outputCount; k++)
                    {
                        if (Shaders[i]->outputs[k] == Shaders[i]->transformFeedback.varyings[j].output)
                        {
                            tempShaders[i]->transformFeedback.varyings[j].output = tempShaders[i]->outputs[k];
                            break;
                        }
                    }
                }
            }

            break;
        }
    }

    /* set TES input vertex count based on TCS output vertex count */
    if (tempShaders[gceSGSK_TC_SHADER] &&
        tempShaders[gceSGSK_TE_SHADER])
    {
        tempShaders[gceSGSK_TE_SHADER]->shaderLayout.tes.tessPatchInputVertices =
            tempShaders[gceSGSK_TC_SHADER]->shaderLayout.tcs.tcsPatchOutputVertices;
    }

    if (Shaders[gceSGSK_FRAGMENT_SHADER])
    {
        if (Shaders[gceSGSK_GEOMETRY_SHADER])
        {
            gcmONERROR(PatchShaders(Shaders[gceSGSK_GEOMETRY_SHADER], Shaders[gceSGSK_FRAGMENT_SHADER]));
        }
        else if (Shaders[gceSGSK_TC_SHADER])
        {
            gcmONERROR(PatchShaders(Shaders[gceSGSK_TC_SHADER], Shaders[gceSGSK_FRAGMENT_SHADER]));
        }
        else if (Shaders[gceSGSK_VERTEX_SHADER])
        {
            gcmONERROR(PatchShaders(Shaders[gceSGSK_VERTEX_SHADER], Shaders[gceSGSK_FRAGMENT_SHADER]));
        }
    }

    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (Shaders[i])
        {
            gcmONERROR(gcLINKTREE_Construct(gcvNULL, &shaderTrees[i]));
            gcmONERROR(gcLINKTREE_Build(shaderTrees[i], tempShaders[i], Flags));
            gcLINKTREE_FindModelViewProjection(shaderTrees[i]);
            if (gcShaderHwRegAllocated(tempShaders[i]))
            {
                gcmONERROR(gcLINKTREE_MarkAllAsUsedwithRA(shaderTrees[i]));
            }
            else
            {
                gcmONERROR(gcLINKTREE_MarkAllAsUsed(shaderTrees[i]));
            }

            tempShaderTrees[i] = &shaderTrees[i];
        }
    }

    gcmONERROR(gcLinkTreeThruVirShaders(tempShaderTrees, gcvFALSE, Flags,
                                        gcvFALSE, StateBufferSize, StateBuffer, Hints));

OnError:
    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
        if (shaderTrees[i])
        {
            /* Destroy the shader tree. */
            gcmVERIFY_OK(gcLINKTREE_Destroy(shaderTrees[i]));
        }

        if (tempShaders[i] != gcvNULL)
        {
            gcmVERIFY_OK(gcSHADER_Destroy(tempShaders[i]));
        }
    }

    if (gcmIS_SUCCESS(status) && (StateBufferSize != gcvNULL))
    {
        /* Return the status. */
        gcmFOOTER_ARG("*StateBufferSize=%lu *StateBuffer=0x%x *Hints=0x%x",
                                    *StateBufferSize, *StateBuffer, *Hints);
    }
    else
    {
        /* Free state buffer if link failed. */
        if (*StateBuffer != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *StateBuffer));
        }

        /* Free up Hints. */
        if (*Hints != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *Hints));
        }
        gcmFOOTER();
    }

    return status;
}

gceSTATUS
gcLinkProgramPipeline(
    IN gctINT               ShaderCount,
    IN gcSHADER *           ShaderArray,
    OUT gctUINT32 *         StateBufferSize,
    OUT gctPOINTER *        StateBuffer,
    OUT gcsHINT_PTR *       Hints
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gcsShaderGroup          shGroup;
    gcmHEADER_ARG("ShaderCount=%d ShaderArray=0x%x",
                  ShaderCount, ShaderArray);

    if (ShaderCount > gcMAX_SHADERS_IN_LINK_GOURP)
    {
        gcmFOOTER();
        return gcvSTATUS_TOO_MANY_SHADERS;
    }

    /* init shader group */
    _checkSetShaderGroup(ShaderCount, ShaderArray, &shGroup);
    if (shGroup.validGroup != gcvTRUE)
    {
        gcmFOOTER();
        return gcvSTATUS_LINK_INVALID_SHADERS;
    }

    status = _verifyNotStagesRelatedError(shGroup);
    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    if (!shGroup.hasComputeOrCLShader)
    {
        /* for now we only have vertex and fragment shader */
        if (shGroup.shaderGroup[gceSGSK_TC_SHADER] == gcvNULL &&
            shGroup.shaderGroup[gceSGSK_TE_SHADER] == gcvNULL &&
            shGroup.shaderGroup[gceSGSK_GEOMETRY_SHADER] == gcvNULL)
        {
            status = _LinkProgramPipeline(shGroup.shaderGroup[gceSGSK_VERTEX_SHADER],
                                          shGroup.shaderGroup[gceSGSK_FRAGMENT_SHADER],
                                          StateBufferSize,
                                          StateBuffer,
                                          Hints
                                          );
        }
        else
        {
            status = _LinkFullGraphicsProgramPipeline(shGroup.shaderGroup,
                                                      StateBufferSize,
                                                      StateBuffer,
                                                      Hints
                                                      );
        }
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
_ValidateIOVariables(
    IN gcSHADER             UpperShader,
    IN gcOUTPUT             Output,
    IN gcSHADER             LowerShader,
    IN gcATTRIBUTE          Input
    )
{
    gctSTRING               inputName, outputName;
    gcsIO_BLOCK             ioBlock = gcvNULL;
    gctBOOL                 inputBlockMember = gcvFALSE, outputBlockMember = gcvFALSE;

    inputName = GetATTRName(Input);
    outputName = GetOutputName(Output);

    /* If this variable is an IO block member, then we don't need to check the instance name. */
    if (GetATTRIOBlockIndex(Input) != -1)
    {
        ioBlock = LowerShader->ioBlocks[GetATTRIOBlockIndex(Input)];
        if (GetSBInstanceNameLength(ioBlock) > 0)
        {
            gcoOS_StrStr(inputName, ".", &inputName);
        }
        inputBlockMember = gcvTRUE;
    }

    if (GetOutputIOBlockIndex(Output) != -1)
    {
        ioBlock = UpperShader->ioBlocks[GetOutputIOBlockIndex(Output)];
        if (GetSBInstanceNameLength(ioBlock) > 0)
        {
            gcoOS_StrStr(outputName, ".", &outputName);
        }
        outputBlockMember = gcvTRUE;
    }

    if (outputBlockMember != inputBlockMember)
    {
        return gcvSTATUS_OK;
    }

    /* The names are same */
    if (gcmIS_SUCCESS(gcoOS_StrCmp(inputName, outputName)) &&
        (Input->location == -1 || Output->location == -1))
    {
        if (GetOutputLocation(Output) != GetATTRLocation(Input)     ||
            GetOutputPrecision(Output) != GetATTRPrecision(Input)   ||
            GetOutputOrigType(Output) != GetATTRType(Input)         ||
            GetOutputArraySize(Output) != GetATTRArraySize(Input)   ||
            GetOutputFieldIndex(Output) != GetATTRFieldIndex(Input) ||
            gcmOUTPUT_isArray(Output) != GetATTRIsArray(Input)      ||
            GetOutputShaderMode(Output) != GetATTRShaderMode(Input))
        {
            return gcvSTATUS_VARYING_TYPE_MISMATCH;
        }
        return gcvSTATUS_TRUE;
    }
    /* Declared with same location */
    else if (Input->location != -1 && Input->location == Output->location)
    {
        if (GetOutputPrecision(Output) != GetATTRPrecision(Input)   ||
            GetOutputOrigType(Output) != GetATTRType(Input)         ||
            GetOutputArraySize(Output) != GetATTRArraySize(Input)   ||
            GetOutputFieldIndex(Output) != GetATTRFieldIndex(Input) ||
            gcmOUTPUT_isArray(Output) != GetATTRIsArray(Input)      ||
            GetOutputShaderMode(Output) != GetATTRShaderMode(Input))
        {
            return gcvSTATUS_VARYING_TYPE_MISMATCH;
        }
        return gcvSTATUS_TRUE;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
_ValidateIoBetweenTwoShaderStages(
    IN gcSHADER             UpperShader,
    IN gcSHADER             LowerShader
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctUINT32               i, j;
    gcATTRIBUTE             input = gcvNULL;
    gcOUTPUT                output = gcvNULL;
    gcsIO_BLOCK             inputBlock = gcvNULL;
    gcsIO_BLOCK             outputBlock = gcvNULL;
    gctBOOL                 *outMatched = gcvNULL;
    gctPOINTER              pointer = gcvNULL;

    /* Check normal I/O. */
    if (UpperShader->outputCount > 0 || UpperShader->ioBlockCount > 0)
    {
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(gctBOOL) *
                                  gcmMAX(UpperShader->outputCount, UpperShader->ioBlockCount),
                                  &pointer));
        gcoOS_ZeroMemory(pointer,
                         gcmSIZEOF(gctBOOL) *
                         gcmMAX(UpperShader->outputCount, UpperShader->ioBlockCount));
        outMatched = (gctBOOL *)pointer;
    }

    for (i = 0; i < LowerShader->attributeCount; i++)
    {
        input = LowerShader->attributes[i];

        /* Builtin input does not need a matching VS input */
        if (input == gcvNULL || GetATTRNameLength(input) < 0)
        {
            continue;
        }

        for (j = 0; j < UpperShader->outputCount; j++)
        {
            output = UpperShader->outputs[j];

            if (output == gcvNULL || GetOutputNameLength(output) < 0)
            {
                continue;
            }

            gcmONERROR(_ValidateIOVariables(UpperShader, output, LowerShader, input));
            if (status == gcvSTATUS_TRUE)
            {
                outMatched[j] = gcvTRUE;
                break;
            }
        }

        if (j == UpperShader->outputCount)
        {
            status = gcvSTATUS_VARYING_TYPE_MISMATCH;
            gcmONERROR(status);
        }
    }

    /* User-defined output doesn't have a matching input declared */
    for (j = 0; j < UpperShader->outputCount; ++j)
    {
        output = UpperShader->outputs[j];

        /* Skip builtin outputs. */
        if (output == gcvNULL || GetOutputNameLength(output) < 0 || GetOutputArrayIndex(output) != 0)
        {
            continue;
        }

        if (!outMatched[j])
        {
            status = gcvSTATUS_VARYING_TYPE_MISMATCH;
            gcmONERROR(status);
        }
    }

    if (outMatched)
    {
        gcoOS_ZeroMemory(outMatched,
                         gcmSIZEOF(gctBOOL) *
                         gcmMAX(UpperShader->outputCount, UpperShader->ioBlockCount));
    }

    /* Check IO blocks */
    for (i = 0; i < LowerShader->ioBlockCount; i++)
    {
        inputBlock = LowerShader->ioBlocks[i];

        if (inputBlock == gcvNULL || GetSBNameLength(inputBlock) < 0 ||
            GetSVIIsOutput(GetSBShaderVarInfo(inputBlock)))
        {
            continue;
        }

        for (j = 0; j < UpperShader->ioBlockCount; j++)
        {
            outputBlock = UpperShader->ioBlocks[j];

            if (outputBlock == gcvNULL || GetSBNameLength(outputBlock) < 0 ||
                !GetSVIIsOutput(GetSBShaderVarInfo(outputBlock)))
            {
                continue;
            }

            /* We don't need to check the instance name. */
            if ((GetSBNameLength(inputBlock) == GetSBNameLength(outputBlock))
                &&
                gcmIS_SUCCESS(gcoOS_StrNCmp(inputBlock->name,
                                            outputBlock->name,
                                            GetSBNameLength(inputBlock)))
                )
            {
                if ((GetSVIIsArray(GetSBShaderVarInfo(inputBlock)) !=
                     GetSVIIsArray(GetSBShaderVarInfo(outputBlock))) ||
                    (GetSVIArraySize(GetSBShaderVarInfo(inputBlock)) !=
                     GetSVIArraySize(GetSBShaderVarInfo(outputBlock))))
                {
                    status = gcvSTATUS_VARYING_TYPE_MISMATCH;
                    gcmONERROR(status);
                }
                break;
            }
        }

        if (j == UpperShader->ioBlockCount)
        {
            status = gcvSTATUS_VARYING_TYPE_MISMATCH;
            gcmONERROR(status);
        }
    }

OnError:
    if (outMatched)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, outMatched));
    }
    return status;
}

/*******************************************************************************
**                                gcValidateProgramPipeline
********************************************************************************
**
**    Validate program pipeline.
**
**    INPUT:
**        gctINT ShaderCount
**            number of gcSHADER object in the shader array
**
**        gcSHADER *ShaderArray
**            Array of gcSHADER object holding information about the compiled
**            shader.
**
*/
gceSTATUS
gcValidateProgramPipeline(
    IN gctINT               ShaderCount,
    IN gcSHADER *           ShaderArray
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctINT                  i;
    gcSHADER                upperShader = gcvNULL;

    gcmHEADER_ARG("ShaderCount=%d ShaderArray=0x%x", ShaderCount, ShaderArray);

    for (i = 0; i < ShaderCount; i++)
    {
        if (upperShader != gcvNULL)
        {
            gcmONERROR(_ValidateIoBetweenTwoShaderStages(upperShader,
                                                         ShaderArray[i]));
        }
        upperShader = ShaderArray[i];
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

#endif /* !DX_SHADER */

gceSTATUS
_LinkBuiltinLibs(
    IN gcSHADER* Shaders
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT   i;

    for (i = 0; i < gcMAX_SHADERS_IN_LINK_GOURP; i ++)
    {
#if DX_SHADER
        if (Shaders[i] && GetShaderHasIntrinsicBuiltin(Shaders[i]))
        {
            gcSHADER libBinary = gcvNULL;

            status = gcSHADER_CompileBuiltinLibrary(
                                    Shaders[i],
                                    gcSHADER_TYPE_LIBRARY,
                                    gcLIB_DX_BUILTIN,
                                    &libBinary);

            if (gcmIS_ERROR(status))
            {
                return status;
            }

            status = gcSHADER_LinkBuiltinLibrary(
                                    Shaders[i],
                                    libBinary,
                                    gcLIB_DX_BUILTIN);

            if (gcmIS_ERROR(status))
            {
                return status;
            }

            /* after linking successfully, reset the flag */
            SetShaderHasIntrinsicBuiltin(Shaders[i], gcvFALSE);
        }
#else
        if (Shaders[i] && GetShaderNeedPatchForCentroid(Shaders[i]))
        {
            status = gcSHADER_PatchCentroidVaryingAsCenter(Shaders[i]);

            if (gcmIS_ERROR(status))
            {
                return status;
            }

            /* after patch successfully, reset the flag */
            SetShaderNeedPatchForCentroid(Shaders[i], gcvFALSE);
        }

        if (Shaders[i] && GetShaderHasIntrinsicBuiltin(Shaders[i]))
        {
            gcSHADER libBinary = gcvNULL;
            gcLibType libType = gcLIB_BUILTIN;

            if (Shaders[i]->type == gcSHADER_TYPE_CL)
            {
                libType = gcLIB_CL_BUILTIN;
                status = gcSHADER_CompileCLBuiltinLibrary(Shaders[i],
                                                          gcSHADER_TYPE_LIBRARY,
                                                          libType,
                                                          &libBinary);
                if (gcmIS_ERROR(status))
                {
                    return status;
                }
            }
            else
            {
                status = gcSHADER_CompileBuiltinLibrary(Shaders[i],
                                                        gcSHADER_TYPE_LIBRARY,
                                                        libType,
                                                        &libBinary);
                if (gcmIS_ERROR(status))
                {
                    return status;
                }
            }

            status = gcSHADER_LinkBuiltinLibrary(Shaders[i],
                                                 libBinary,
                                                 libType);
            if (gcmIS_ERROR(status))
            {
                return status;
            }

            /* after linking successfully, reset the flag */
            SetShaderHasIntrinsicBuiltin(Shaders[i], gcvFALSE);
        }
#endif

        if (Shaders[i] && gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(GetShaderOutputBlends(Shaders[i])))
        {
            gcSHADER libBinary = gcvNULL;

            status = gcSHADER_CompileBuiltinLibrary(
                                    Shaders[i],
                                    GetShaderType(Shaders[i]),
                                    gcLIB_BLEND_EQUATION,
                                    &libBinary);

            if (gcmIS_ERROR(status))
            {
                return status;
            }

            status = gcSHADER_LinkBuiltinLibrary(
                                    Shaders[i],
                                    libBinary,
                                    gcLIB_BLEND_EQUATION);

            if (gcmIS_ERROR(status))
            {
                return status;
            }

            /* after linking successfully, reset the flag */
            ClearShaderOutputBlends(Shaders[i]);
        }

        /* After link all built-in functions, analyze them. */
        if (Shaders[i])
        {
            gcSHADER_AnalyzeFunctions(Shaders[i], gcvFALSE);
        }
    }
    return status;
}

gctBOOL
gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(
    IN gceLAYOUT_QUALIFIER Qualifier
    )
{
    gctBOOL     isHalti4 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI4);

    if(isHalti4)
    {
        return Qualifier & gcvLAYOUT_QUALIFIER_BLEND_HW_NOT_SUPPORT_EQUATIONS_HALTI4;
    }
    else
    {
        return Qualifier & gcvLAYOUT_QUALIFIER_BLEND_HW_NOT_SUPPORT_EQUATIONS;
    }
}

/** Program binary file header format: -
       Word 1:  gcmCC('P', 'R', 'G', 'M') Program binary file signature
       Word 2:  ('\od' '\od' '\od' '\od') od = octal digits; program binary file version
       Word 3:  ('\E' '\S' '\0' '\0') or Language type
                ('C' 'L' '\0' '\0')
       Word 4: ('\0' '\0' '\10' '\0') chip model  e.g. gc800
       Wor5: ('\1' '\3' '\6' '\4') chip version e.g. 4.6.3_rc1
       Word 6: size of program binary file in bytes excluding this header */
#define _gcdProgramBinaryHeaderSize   (6 * 4)  /*Program binary file header size in bytes*/

/*******************************************************************************
**  _gcLoadProgramHeader
**
**  Load a shader proggram from a binary buffer.  The binary buffer is layed out
**  as follows:
**      // Six word header
**      // Signature, must be 'S','H','D','R'.
**      gctINT8             signature[4];
**      gctUINT32           prgBinFileVersion;
**      gctUINT32           laguage;
**      gctUINT32           chipModel;
**      gctUINT32           chipVersion;
**      gctUINT32           binarySize;
**
**  INPUT:
**
**      gctPOINTER Buffer
**          Pointer to a program binary buffer containing the program data to load.
**
**      gctSIZE_T BufferSize
**          Number of bytes inside the binary buffer pointed to by 'Buffer'.
**
**  OUTPUT:
**      gctUINT32 *Language
**          the third word in the header containing Language type
**
*/
static gceSTATUS
_gcLoadProgramHeader(
    IN gctPOINTER Buffer,
    IN gctSIZE_T BufferSize,
    OUT gctPOINTER Language
    )
{
    gctUINT32 * signature;
    gctSIZE_T bytes;
    gctUINT32 *version;
    gctUINT32 *language;
    gctUINT32 *chipModel;
    gctUINT32 *chipVersion;
    gctUINT32 *size;
    gctUINT8 *bytePtr;

    gcmHEADER_ARG("Buffer=0x%x BufferSize=%lu",
                  Buffer, BufferSize);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Buffer != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(BufferSize > 0);

    /* Load the Header */
    bytes = BufferSize;
    if(bytes < _gcdProgramBinaryHeaderSize) {
        /* Invalid file format */
        gcmFATAL("_gcLoadProgramHeader: Invalid program binary file format");
        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
        return gcvSTATUS_INVALID_DATA;
    }

    /* Verify the signature. */
    signature = (gctUINT32 *) Buffer;
    if (*signature != gcmCC('P', 'R', 'G', 'M')) {
        /* Signature mismatch. */
        gcmFATAL("_gcLoadProgramHeader: Signature does not match with 'PRGM'");
        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
        return gcvSTATUS_INVALID_DATA;
    }

    /* Word 2: program binary file version # */
    version = (gctUINT32 *) (signature + 1);
    if(*version > gcdSL_PROGRAM_BINARY_FILE_VERSION) {
#if gcdDEBUG
        gctUINT8 *inputVerPtr, *curVerPtr;
        gctUINT32 curVer[1] = {gcdSL_PROGRAM_BINARY_FILE_VERSION};

        inputVerPtr = (gctUINT8 *) version;
        curVerPtr = (gctUINT8 *) curVer;
        gcmFATAL("_gcLoadProgramHeader: program binary file's version of %u.%u.%u:%u "
                 "is newer than current version %u.%u.%u:%u",
                 inputVerPtr[0], inputVerPtr[1], inputVerPtr[2], inputVerPtr[3],
                 curVerPtr[0], curVerPtr[1], curVerPtr[2], curVerPtr[3]);
#endif
        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
        return gcvSTATUS_INVALID_DATA;
    }

    /* Word 3: language type */
    language = (gctUINT32 *) (version + 1);
    bytePtr = (gctUINT8 *) language;

    if((bytePtr[0] == 'E' && bytePtr[1] != 'S') ||
       (bytePtr[0] == 'C' && bytePtr[1] != 'L') ||
       (bytePtr[0] != 'E' && bytePtr[0] != 'C')) {
       gcmFATAL("_gcLoadProgramHeader: unrecognizable laguage type \"%c%c\"", bytePtr[0], bytePtr[1]);
       gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
       return gcvSTATUS_INVALID_DATA;
    }

    /* Word 4: chip model */
    chipModel = (gctUINT32 *) (language + 1);

    /* Word 5: chip version */
    chipVersion  = (gctUINT32 *) (chipModel + 1);

    /* Word 6: size of program binary excluding header */
    bytes -= _gcdProgramBinaryHeaderSize;
    size = (gctUINT32 *) (chipVersion + 1);
    if(bytes != *size) {
        /* program binary size mismatch. */
        gcmFATAL("_gcLoadProgramHeader: program binary size %u does not match actual file size %u",
                 bytes, *size);
        gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
        return gcvSTATUS_INVALID_DATA;
    }
    /* Success. */
    *(gctUINT32 *)Language = *language;
    gcmFOOTER_ARG("*Language=%u", *language);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**                                gcSaveProgram
********************************************************************************
**
**  Save pre-compiled shaders and pre-linked programs to a binary file.
**
**  INPUT:
**
**      gcSHADER VertexShader
**          Pointer to vertex shader object.
**
**      gcSHADER FragmentShader
**          Pointer to fragment shader object.
**
**      gctUINT32 ProgramBufferSize
**          Number of bytes in 'ProgramBuffer'.
**
**      gctPOINTER ProgramBuffer
**          Pointer to buffer containing the program states.
**
**      gcsHINT_PTR Hints
**          Pointer to HINTS structure for program states.
**
**  OUTPUT:
**
**      gctPOINTER * Binary
**          Pointer to a variable receiving the binary data to be saved.
**
**      gctUINT32 * BinarySize
**          Pointer to a variable receiving the number of bytes inside 'Binary'.
*/
gceSTATUS
gcSaveProgram(
    IN gcSHADER VertexShader,
    IN gcSHADER FragmentShader,
    IN gctUINT32 ProgramBufferSize,
    IN gctPOINTER ProgramBuffer,
    IN gcsHINT_PTR Hints,
    OUT gctPOINTER * Binary,
    OUT gctUINT32 * BinarySize
    )
{
    gctUINT32 vertexShaderBytes;
    gctUINT32 fragmentShaderBytes;
    gctUINT32 bytes;
    gctUINT8_PTR bytePtr;
    gctUINT32 bufferSize;
    gctUINT8_PTR buffer = gcvNULL;
    gceSTATUS status;

    gcmHEADER_ARG("VertexShader=0x%x FragmentShader=0x%x ProgramBufferSize=%lu "
                  "ProgramBuffer=0x%x Hints=0x%x",
                  VertexShader, FragmentShader, ProgramBufferSize,
                  ProgramBuffer, Hints);

    do
    {
        /* Get size of vertex shader bibary. */
        gcmERR_BREAK(gcSHADER_Save(VertexShader,
                                   gcvNULL,
                                   &vertexShaderBytes));

        /* Get size of fragment shader binary. */
        gcmERR_BREAK(gcSHADER_Save(FragmentShader,
                                   gcvNULL,
                                   &fragmentShaderBytes));

        /* Compute number of bytes required for binary buffer. */
        bufferSize = _gcdProgramBinaryHeaderSize
                     + gcmSIZEOF(gctUINT32) + gcmALIGN(vertexShaderBytes, 4)
                     + gcmSIZEOF(gctUINT32) + gcmALIGN(fragmentShaderBytes, 4)
                     + gcmSIZEOF(gctUINT32) + ProgramBufferSize
                     + gcmSIZEOF(gctUINT32) + gcSHADER_GetHintSize();

        if (BinarySize)
        {
            *BinarySize = bufferSize;
        }

        if (Binary == gcvNULL)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }

        if (*Binary)
        {
            /* a buffer has been passed here */
            /* Make sure the buffer is large enough. */
            if(BinarySize)
            {
                if (*BinarySize < bufferSize)
                {
                    *BinarySize = bufferSize;
                    gcmFOOTER_ARG("*BinarySize=%d status=%d",
                                  *BinarySize, gcvSTATUS_BUFFER_TOO_SMALL);
                    return gcvSTATUS_BUFFER_TOO_SMALL;
                }
           }
        }
        else
        {
            /* Allocate binary buffer. */
            gcmERR_BREAK(gcoOS_Allocate(gcvNULL,
                                        bufferSize,
                                        Binary));
        }

        /* Cast binary buffer to UINT8 pointer for easy byte copying. */
        buffer    = (gctUINT8_PTR) *Binary;

        /* Word 1: program binary signature */
        buffer[0] = 'P';
        buffer[1] = 'R';
        buffer[2] = 'G';
        buffer[3] = 'M';
        buffer += 4;

        /* Word 2: binary file version # */
        *(gctUINT32 *) buffer = gcdSL_PROGRAM_BINARY_FILE_VERSION;
        buffer += sizeof(gctUINT32);

        /* Word 3: language type */
        *(gctUINT32 *) buffer = VertexShader->compilerVersion[0];
        buffer += sizeof(gctUINT32);

        /* Word 4: chip model */
        *(gctUINT32 *) buffer = gcmCC('\0', '\0', '\0', '\0');
        buffer += sizeof(gctUINT32);

        /* Word 5: chip version */
        *(gctUINT32 *) buffer = gcmCC('\1', gcvVERSION_PATCH, gcvVERSION_MINOR, gcvVERSION_MAJOR);
        buffer += sizeof(gctUINT32);

        /* Word 6: size of binary excluding header */
        *(gctUINT32 *) buffer = bufferSize - _gcdProgramBinaryHeaderSize;
        buffer += sizeof(gctUINT32);

        /* Copy the size of the vertex shader binary. */
        gcoOS_MemCopy(buffer,
                      &vertexShaderBytes,
                      gcmSIZEOF(gctUINT32));
        buffer += gcmSIZEOF(gctUINT32);

        /* Save the vertex shader binary. */
        gcmERR_BREAK(gcSHADER_Save(VertexShader,
                                   buffer,
                                   &vertexShaderBytes));
        bytePtr = buffer + vertexShaderBytes;
        buffer += gcmALIGN(vertexShaderBytes, 4);
        while (bytePtr < buffer)
        {
            /* clear the alignment bytes to help file comparison when needed*/
            *bytePtr++ = '\0';
        }

        /* Copy the size of the fragment shader binary. */
        gcoOS_MemCopy(buffer,
                      &fragmentShaderBytes,
                      gcmSIZEOF(gctUINT32));
        buffer += gcmSIZEOF(gctUINT32);

        /* Save the fragment shader binary. */
        gcmERR_BREAK(gcSHADER_Save(FragmentShader,
                                   buffer,
                                   &fragmentShaderBytes));
        bytePtr = buffer + fragmentShaderBytes;
        buffer += gcmALIGN(fragmentShaderBytes, 4);
        while (bytePtr < buffer)
        {
            /* clear the alignment bytes to help file comparison when needed*/
            *bytePtr++ = '\0';
        }

        /* Copy the size of the state buffer. */
        gcoOS_MemCopy(buffer,
                      &ProgramBufferSize,
                      gcmSIZEOF(gctUINT32));
        buffer += gcmSIZEOF(gctUINT32);

        /* Copy the state buffer. */
        gcoOS_MemCopy(buffer,
                      ProgramBuffer,
                      ProgramBufferSize);
        buffer += ProgramBufferSize;

        /* Copy the size of the HINT structure. */
        bytes = gcSHADER_GetHintSize();
        gcoOS_MemCopy(buffer,
                      &bytes,
                      gcmSIZEOF(gctUINT32));
        buffer += gcmSIZEOF(gctUINT32);

        /* Copy the HINT structure. */
        gcoOS_MemCopy(buffer, Hints, bytes);
        buffer += bytes;

        /* Assert we copied the right number of bytes */
        gcmASSERT(buffer - *BinarySize == *Binary);

        /* Success. */
        gcmFOOTER_ARG("*Binary=0x%x *BinarySize=%ul", *Binary, *BinarySize);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    if (gcmIS_ERROR(status) && (buffer != gcvNULL))
    {
        /* Free binary buffer on error. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *Binary));

        /* Reset pointers on error. */
        *Binary     = gcvNULL;
        if (BinarySize)
        {
            *BinarySize = 0;
        }
    }

    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                                gcLoadProgram
********************************************************************************
**
**    Load pre-compiled shaders and pre-linked programs from a binary file.
**
**    INPUT:
**
**        gctPOINTER Binary
**            Pointer to the binary data loaded.
**
**        gctUINT32 BinarySize
**            Number of bytes in 'Binary'.
**
**        gcSHADER VertexShader
**            Pointer to a vertex shader object.
**
**        gcSHADER FragmentShader
**            Pointer to a fragment shader object.
**
**    OUTPUT:
**
**        gctUINT32 * ProgramBufferSize
**            Pointer to a variable receiving the number of bytes in the buffer
**            returned in 'ProgramBuffer'.
**
**        gctPOINTER * ProgramBuffer
**            Pointer to a variable receiving a buffer pointer that contains the
**            states required to download the shaders into the hardware.
**            if ProgramBuffer is NULL, no states will be loaded.
**
**        gcsHINT_PTR * Hints
**            Pointer to a variable receiving a gcsHINT structure pointer that
**            contains information required when loading the shader states.
**            if Hints is NULL, no hints will be loaded.
*/
gceSTATUS
gcLoadProgram(
    IN gctPOINTER Binary,
    IN gctUINT32 BinarySize,
    OUT gcSHADER VertexShader,
    OUT gcSHADER FragmentShader,
    OUT gctUINT32 * ProgramBufferSize,
    OUT gctPOINTER * ProgramBuffer,
    OUT gcsHINT_PTR * Hints
    )
{
    gcoOS os;
    gctUINT32 bytes;
    gctUINT8_PTR buffer;
    gctUINT32 *bufferSize;
    gctUINT32 alignBufferSize;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT8 language[4];
    gctPOINTER pointer;

    gcmHEADER_ARG("Binary=0x%x BinarySize=%lu",
                  Binary, BinarySize);

    do {
        if (ProgramBufferSize)
        {
            *ProgramBufferSize = 0;
        }
        if (ProgramBuffer)
        {
            if (ProgramBufferSize == gcvNULL)
            {
                gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
                return gcvSTATUS_INVALID_DATA;
            }
            *ProgramBuffer = gcvNULL;
        }
        if (Hints)
        {
            *Hints = gcvNULL;
        }

        /* Extract the gcoOS object pointer. */
        os = gcvNULL;

        gcmERR_BREAK(_gcLoadProgramHeader(Binary, BinarySize, (gctPOINTER)language));

        if (language[0] != 'E' || language[1] != 'S')
        {
           gcmFATAL("gcLoadProgram: expect language type 'ES' instead of %c%c",
                    language[0], language[1]);
           gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
           return gcvSTATUS_INVALID_DATA;
        }

        buffer = (gctUINT8 *)Binary + _gcdProgramBinaryHeaderSize;
        bytes = BinarySize - _gcdProgramBinaryHeaderSize;

        bufferSize = (gctUINT32 *) buffer;

        if (bytes < sizeof(gctUINT32) || bytes < (*bufferSize + sizeof(gctUINT32)))
        {
           /* Invalid vertex shader size. */
           gcmFATAL("gcLoadProgram: Invalid vertex shader size %u", bytes);
           gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
           return gcvSTATUS_INVALID_DATA;
        }

        bytes -=  sizeof(gctUINT32);
        buffer += gcmSIZEOF(gctUINT32);

        /* Load vertex shader bibary. */
        gcmERR_BREAK(gcSHADER_Load(VertexShader,
                                   buffer,
                                   *bufferSize));

        alignBufferSize = gcmALIGN(*bufferSize, 4);
        buffer += alignBufferSize;
        bytes -=  alignBufferSize;
        bufferSize = (gctUINT32 *) buffer;

        if (bytes < sizeof(gctUINT32) || bytes < (*bufferSize + sizeof(gctUINT32)))
        {
           /* Invalid fragment shader size. */
           gcmFATAL("gcLoadProgram: Invalid fragment shader size %u", bytes);
           gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
           return gcvSTATUS_INVALID_DATA;
        }

        bytes -=  sizeof(gctUINT32);
        buffer += gcmSIZEOF(gctUINT32);

        /* Load fragment shader binary. */
        gcmERR_BREAK(gcSHADER_Load(FragmentShader,
                                   buffer,
                                   *bufferSize));

        alignBufferSize = gcmALIGN(*bufferSize, 4);
        buffer += alignBufferSize;
        bytes -=  alignBufferSize;
        bufferSize = (gctUINT32 *) buffer;

        if (bytes < sizeof(gctUINT32) || bytes < (*bufferSize + sizeof(gctUINT32)))
        {
           /* Invalid program states size. */
           gcmFATAL("gcLoadProgram: Invalid program states size %u", bytes);
           gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
           return gcvSTATUS_INVALID_DATA;
        }

        bytes -=  gcmSIZEOF(gctUINT32);
        buffer += gcmSIZEOF(gctUINT32);

        if (ProgramBuffer)
        {
          /* Allocate program states buffer. */
          gcmERR_BREAK(gcoOS_Allocate(os,
                                      *bufferSize,
                                      &pointer));
          *ProgramBuffer = pointer;
          *ProgramBufferSize = *bufferSize;

          gcoOS_MemCopy(*ProgramBuffer,
                        buffer,
                        *bufferSize);
        }

        buffer += *bufferSize;
        bytes -=  *bufferSize;
        bufferSize = (gctUINT32 *) buffer;

        if (bytes < sizeof(gctUINT32) ||
            bytes < (*bufferSize + sizeof(gctUINT32)) ||
            (Hints && *bufferSize != gcSHADER_GetHintSize()) )
        {
           /* Invalid hint size. */
           gcmFATAL("gcLoadProgram: Invalid hints size %u", bytes);
           gcmFOOTER_ARG("status=%d", gcvSTATUS_INVALID_DATA);
           return gcvSTATUS_INVALID_DATA;
        }

        buffer += gcmSIZEOF(gctUINT32);

        if (Hints)
        {
          /* Allocate hint structure buffer. */
          gcmERR_BREAK(gcoOS_Allocate(os,
                                      gcSHADER_GetHintSize(),
                                      &pointer));

          *Hints = pointer;

          /* Copy the HINT structure. */
          gcoOS_MemCopy(*Hints,
                        buffer,
                        *bufferSize);
        }

        buffer += *bufferSize;

        /* Assert we copied the right number of bytes */
        gcmASSERT(buffer - BinarySize == Binary);

        /* Success. */
        gcmFOOTER_ARG("*ProgramBufferSize=%lu *ProgramBuffer=0x%x *Hints=0x%x",
                      ProgramBufferSize ? *ProgramBufferSize : 0,
                      ProgramBuffer ? *ProgramBuffer : gcvNULL,
                      Hints ? *Hints : gcvNULL);

        return gcvSTATUS_OK;
    }
    while (gcvFALSE);
    /* Return the status. */
    gcmFOOTER_ARG("*ProgramBufferSize=%lu *ProgramBuffer=0x%x *Hints=0x%x",
                   ProgramBufferSize ? *ProgramBufferSize : 0,
                   ProgramBuffer ? *ProgramBuffer : gcvNULL,
                   Hints ? *Hints : gcvNULL);
    return status;
}

static gceSTATUS
_gcLINKTREE_CreateColorOutput(
    IN gcSHADER VertexShader
)
{
    gceSTATUS status            = gcvSTATUS_OK;
    gctBOOL   hasFrontColor     = gcvFALSE;
    gctBOOL   hasBackColor      = gcvFALSE;
    gctBOOL   hasFront2Color    = gcvFALSE;
    gctBOOL   hasBack2Color     = gcvFALSE;
    gcATTRIBUTE  varyingColor   = gcvNULL;
    gcATTRIBUTE  varying2Color  = gcvNULL;

    gctUINT   i                 = 0;

    for (i = 0; i < VertexShader->attributeCount; ++i)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(VertexShader->attributes[i]->name, "#AttrColor")))
        {
            varyingColor = VertexShader->attributes[i];
        }
        if (gcmIS_SUCCESS(gcoOS_StrCmp(VertexShader->attributes[i]->name, "#AttrSecondaryColor")))
        {
            varying2Color = VertexShader->attributes[i];
        }
    }

    if(varyingColor == gcvNULL &&
       varying2Color == gcvNULL )
    {
        return status;
    }

    for (i = 0; i < VertexShader->outputCount; ++i)
    {
        if(VertexShader->outputs[i]->nameLength >= 0)
        {
            continue;
        }

        if (VertexShader->outputs[i]->nameLength == gcSL_FRONT_COLOR)
        {
            hasFrontColor = gcvTRUE;
        }
        else if (VertexShader->outputs[i]->nameLength == gcSL_BACK_COLOR)
        {
            hasBackColor = gcvTRUE;
        }
        else if (VertexShader->outputs[i]->nameLength == gcSL_FRONT_SECONDARY_COLOR)
        {
            hasFront2Color = gcvTRUE;
        }
        else if (VertexShader->outputs[i]->nameLength == gcSL_BACK_SECONDARY_COLOR)
        {
            hasBack2Color = gcvTRUE;
        }
    }

    if(varyingColor)
    {
        if (!hasFrontColor)
        {
            gctUINT16 t = (gctUINT16)gcSHADER_NewTempRegs(VertexShader, 1, gcSHADER_FLOAT_X4);
            gcmONERROR(gcSHADER_AddOutput(VertexShader, "gl_FrontColor", gcSHADER_FLOAT_X4, 1, t, gcSHADER_PRECISION_DEFAULT));
        }

        if (!hasBackColor)
        {
            gctUINT16 t = (gctUINT16)gcSHADER_NewTempRegs(VertexShader, 1, gcSHADER_FLOAT_X4);
            gcmONERROR(gcSHADER_AddOutput(VertexShader, "gl_BackColor", gcSHADER_FLOAT_X4, 1, t, gcSHADER_PRECISION_DEFAULT));
        }

        gcmOUTPUT_SetIsStaticallyUsed(varyingColor, gcvFALSE);
    }

    if(varying2Color)
    {
        if (!hasFront2Color)
        {
            gctUINT16 t = (gctUINT16)gcSHADER_NewTempRegs(VertexShader, 1, gcSHADER_FLOAT_X4);
            gcmONERROR(gcSHADER_AddOutput(VertexShader, "gl_FrontSecondaryColor", gcSHADER_FLOAT_X4, 1, t, gcSHADER_PRECISION_DEFAULT));
        }

        if (!hasBack2Color)
        {
            gctUINT16 t = (gctUINT16)gcSHADER_NewTempRegs(VertexShader, 1, gcSHADER_FLOAT_X4);
            gcmONERROR(gcSHADER_AddOutput(VertexShader, "gl_BackSecondaryColor", gcSHADER_FLOAT_X4, 1, t, gcSHADER_PRECISION_DEFAULT));
        }

        gcmOUTPUT_SetIsStaticallyUsed(varying2Color, gcvFALSE);
    }

OnError:
    return status;
}

/*
** Using gl_FrontColor/gl_BackColor and gl_FrontSecondaryColor/gl_BackSecondaryColor to
** replace gl_Color/gl_SecondaryColor.
*/
static gceSTATUS
_gcLINKTREE_ReplaceColor2FrontBackColor(
    IN gcSHADER FragmentShader
)
{
    gceSTATUS    status            = gcvSTATUS_OK;
    gcATTRIBUTE  varyingColor   = gcvNULL;
    gcATTRIBUTE  varying2Color  = gcvNULL;
    gcATTRIBUTE  frontColor     = gcvNULL;
    gcATTRIBUTE  backColor      = gcvNULL;
    gcATTRIBUTE  front2Color    = gcvNULL;
    gcATTRIBUTE  back2Color     = gcvNULL;
    gcATTRIBUTE  frontFacing    = gcvNULL;
    gcATTRIBUTE  fColor         = gcvNULL;
    gcATTRIBUTE  bColor         = gcvNULL;
    gctUINT32    attrIdx, i;
    gctINT       mainStartIdx;
    gctUINT16    realColor = 0, real2Color = 0;
    gctUINT      origLastInstruction = FragmentShader->lastInstruction;

    /* Find gl_Color/gl_SecondaryColor. */
    for (attrIdx = 0; attrIdx < FragmentShader->attributeCount; ++attrIdx)
    {
        gctBOOL compareName = gcvFALSE;

        if (FragmentShader->attributes[attrIdx] == gcvNULL)
        {
            continue;
        }

        if (FragmentShader->attributes[attrIdx]->nameLength > 0)
            compareName = gcvTRUE;

        if (compareName &&
            FragmentShader->attributes[attrIdx]->nameLength == (gctINT)gcoOS_StrLen("#VaryingColor", gcvNULL) &&
            gcmIS_SUCCESS(gcoOS_StrCmp(FragmentShader->attributes[attrIdx]->name, "#VaryingColor")))
        {
            varyingColor = FragmentShader->attributes[attrIdx];
        }
        else if (compareName &&
            FragmentShader->attributes[attrIdx]->nameLength == (gctINT)gcoOS_StrLen("#VaryingSecondaryColor", gcvNULL) &&
            gcmIS_SUCCESS(gcoOS_StrCmp(FragmentShader->attributes[attrIdx]->name, "#VaryingSecondaryColor")))
        {
            varying2Color = FragmentShader->attributes[attrIdx];
        }
        else if (!compareName &&
            FragmentShader->attributes[attrIdx]->nameLength == gcSL_FRONT_COLOR)
        {
            frontColor = FragmentShader->attributes[attrIdx];
        }
        else if (!compareName &&
            FragmentShader->attributes[attrIdx]->nameLength == gcSL_BACK_COLOR)
        {
            backColor = FragmentShader->attributes[attrIdx];
        }
        else if (!compareName &&
            FragmentShader->attributes[attrIdx]->nameLength == gcSL_FRONT_SECONDARY_COLOR)
        {
            front2Color = FragmentShader->attributes[attrIdx];
        }
        else if (!compareName &&
            FragmentShader->attributes[attrIdx]->nameLength == gcSL_BACK_SECONDARY_COLOR)
        {
            back2Color = FragmentShader->attributes[attrIdx];
        }
        else if (!compareName &&
            FragmentShader->attributes[attrIdx]->nameLength == gcSL_FRONT_FACING)
        {
            frontFacing = FragmentShader->attributes[attrIdx];
        }
    }

    if (!varyingColor && !varying2Color)
    {
        return status;
    }

    /* Find frontColor/backColor or frontSecondaryColor/backSecondaryColor. */
    if (varyingColor)
    {
        if (frontColor == gcvNULL)
        {
            gcmONERROR(gcSHADER_AddAttribute(FragmentShader,
                "gl_FrontColor",
                gcSHADER_FLOAT_X4,
                1,
                gcvFALSE,
                gcSHADER_SHADER_DEFAULT,
                varyingColor->precision,
                &frontColor));
        }

        if (backColor == gcvNULL)
        {
            gcmONERROR(gcSHADER_AddAttribute(FragmentShader,
                "gl_BackColor",
                gcSHADER_FLOAT_X4,
                1,
                gcvFALSE,
                gcSHADER_SHADER_DEFAULT,
                varyingColor->precision,
                &backColor));
        }
    }

    if (varying2Color)
    {
        if (front2Color == gcvNULL)
        {
            gcmONERROR(gcSHADER_AddAttribute(FragmentShader,
                "gl_FrontSecondaryColor",
                gcSHADER_FLOAT_X4,
                1,
                gcvFALSE,
                gcSHADER_SHADER_DEFAULT,
                varying2Color->precision,
                &front2Color));
        }

        if (back2Color == gcvNULL)
        {
            gcmONERROR(gcSHADER_AddAttribute(FragmentShader,
                "gl_BackSecondaryColor",
                gcSHADER_FLOAT_X4,
                1,
                gcvFALSE,
                gcSHADER_SHADER_DEFAULT,
                varying2Color->precision,
                &back2Color));
        }
    }

    if (frontFacing == gcvNULL)
    {
        gcmONERROR(gcSHADER_AddAttribute(FragmentShader,
            "gl_FrontFacing",
            gcSHADER_FLOAT_X1,
            1,
            gcvFALSE,
            gcSHADER_SHADER_DEFAULT,
            gcSHADER_PRECISION_MEDIUM,
            &frontFacing));
    }

    /*
    ** Insert:
    ** CMP.z dest, dest0, src2
    ** CMP.nz dest, dest0, src1
    */
    gcmONERROR(gcSHADER_FindMainFunction(FragmentShader, &mainStartIdx, gcvNULL));

    if (varyingColor != gcvNULL)
    {
        fColor  = frontColor;
        bColor  = backColor;

        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(FragmentShader, mainStartIdx, 2));
        gcOpt_Dump(gcvNULL, "Incoming Fragment Shader", gcvNULL, FragmentShader);
        FragmentShader->instrIndex = (mainStartIdx == 0) ? gcSHADER_OPCODE : gcSHADER_SOURCE1;
        FragmentShader->lastInstruction = (mainStartIdx == 0) ? 0 : (mainStartIdx - 1);

        realColor = (gctUINT16)gcSHADER_NewTempRegs(FragmentShader, 1, gcSHADER_FLOAT_X4);

        /* CMP.z dest, dest0, src2 */
        gcmONERROR(gcSHADER_AddOpcode2(FragmentShader, gcSL_CMP, gcSL_ZERO, realColor, gcSL_ENABLE_XYZW, gcSL_FLOAT, bColor->precision));
        gcmONERROR(gcSHADER_AddSourceAttribute(FragmentShader, frontFacing, gcSL_SWIZZLE_XXXX, 0));
        gcmONERROR(gcSHADER_AddSourceAttribute(FragmentShader, bColor, gcSL_SWIZZLE_XYZW, 0));

        /* CMP.nz dest, dest0, src1 */
        gcmONERROR(gcSHADER_AddOpcode2(FragmentShader, gcSL_CMP, gcSL_NOT_ZERO, realColor, gcSL_ENABLE_XYZW, gcSL_FLOAT, fColor->precision));
        gcmONERROR(gcSHADER_AddSourceAttribute(FragmentShader, frontFacing, gcSL_SWIZZLE_XXXX, 0));
        gcmONERROR(gcSHADER_AddSourceAttribute(FragmentShader, fColor, gcSL_SWIZZLE_XYZW, 0));

        FragmentShader->lastInstruction = origLastInstruction + 2;
    }

    if (varying2Color != gcvNULL)
    {
        fColor  = front2Color;
        bColor  = back2Color;

        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(FragmentShader, mainStartIdx, 2));
        FragmentShader->instrIndex = gcSHADER_SOURCE1;
        FragmentShader->lastInstruction = (mainStartIdx == 0) ? 0 : (mainStartIdx - 1);

        real2Color = (gctUINT16)gcSHADER_NewTempRegs(FragmentShader, 1, gcSHADER_FLOAT_X4);

        /* CMP.z dest, dest0, src2 */
        gcmONERROR(gcSHADER_AddOpcode2(FragmentShader, gcSL_CMP, gcSL_ZERO, real2Color, gcSL_ENABLE_XYZW, gcSL_FLOAT, bColor->precision));
        gcmONERROR(gcSHADER_AddSourceAttribute(FragmentShader, frontFacing, gcSL_SWIZZLE_XXXX, 0));
        gcmONERROR(gcSHADER_AddSourceAttribute(FragmentShader, bColor, gcSL_SWIZZLE_XYZW, 0));

        /* CMP.nz dest, dest0, src1 */
        gcmONERROR(gcSHADER_AddOpcode2(FragmentShader, gcSL_CMP, gcSL_NOT_ZERO, real2Color, gcSL_ENABLE_XYZW, gcSL_FLOAT, fColor->precision));
        gcmONERROR(gcSHADER_AddSourceAttribute(FragmentShader, frontFacing, gcSL_SWIZZLE_XXXX, 0));
        gcmONERROR(gcSHADER_AddSourceAttribute(FragmentShader, fColor, gcSL_SWIZZLE_XYZW, 0));

        FragmentShader->lastInstruction = origLastInstruction + 2;
    }

    /* Replace gl_Color/gl_SecondaryColor with realColor/real2Color. */
    for (i = 0; i < FragmentShader->lastInstruction; ++i)
    {
        gctINT           j    = 0;

        for (j = 0; j < 2; ++j)
        {
            gctSOURCE_t              *source             = j == 0 ? &FragmentShader->code[i].source0 : &FragmentShader->code[i].source1;
            gctUINT16                *sourceIndex        = j == 0 ? &FragmentShader->code[i].source0Index : &FragmentShader->code[i].source1Index;
            gcSL_TYPE                 src_type;
            gctINT                    index              = 0;
            gctUINT16                 newColor;

            src_type = gcmSL_SOURCE_GET(*source, Type);
            if (src_type != gcSL_ATTRIBUTE)
            {
                continue;
            }

            index = gcmSL_INDEX_GET(*sourceIndex, Index);
            if (varyingColor != gcvNULL && index == varyingColor->index)
            {
                newColor = realColor;
            }
            else if (varying2Color != gcvNULL && index == varying2Color->index)
            {
                newColor = real2Color;
            }
            else
            {
                continue;
            }

            *source      = gcmSL_SOURCE_SET(*source, Type, gcSL_TEMP);
            *source      = gcmSL_SOURCE_SET(*source, Indexed, gcSL_NOT_INDEXED);
            *sourceIndex = gcmSL_INDEX_SET(*sourceIndex, Index, newColor);
        }
    }

    /* Mark gl_Color/gl_SecondaryColor as unused. */
    if (varyingColor)
    {
        gcmATTRIBUTE_SetIsStaticallyUsed(varyingColor, gcvFALSE);
    }
    if (varying2Color)
    {
        gcmATTRIBUTE_SetIsStaticallyUsed(varying2Color, gcvFALSE);
    }

OnError:
    return status;
}

#endif


