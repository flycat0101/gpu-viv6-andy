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


/*
**  Optimizer and shader output module.
*/

#include "gc_vsc.h"

#if gcdENABLE_3D

#include "old_impl/gc_vsc_old_optimizer.h"

#define _GC_OBJ_ZONE    gcvZONE_COMPILER

#define INDENT_GAP      4

/*******************************************************************************
**                              gcOptimizer Logging
*******************************************************************************/
void
gcOpt_DumpBuffer(
    IN gcoOS Os,
    IN gctFILE File,
    IN gctSTRING Buffer,
    IN gctSIZE_T ByteCount
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    if (File)
    {
        status = gcoOS_Write(Os, File, ByteCount, Buffer);
        if (gcmIS_ERROR(status))
        {
            gcmTRACE(gcvLEVEL_ERROR, "gcoOS_Write fail.");
        }
    }
    else
    {
        /* Print string to debug terminal. */
        gcoOS_Print("%s", Buffer);
    }
}

void
gcOpt_DumpMessage(
    IN gcoOS Os,
    IN gctFILE File,
    IN gctSTRING Message
    )
{
#if !gcdDEBUG
    if (File == gcvNULL) return;
#endif

    gcOpt_DumpBuffer(Os, File, Message, gcoOS_StrLen(Message, gcvNULL));
}

static gctCONST_STRING
GetCategoryName_(
    IN gcSHADER_VAR_CATEGORY Category
)
{
    switch (Category)
    {
    case gcSHADER_VAR_CATEGORY_NORMAL:
    case gcSHADER_VAR_CATEGORY_LOD_MIN_MAX:
    case gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE:
    case gcSHADER_VAR_CATEGORY_EXTRA_FOR_TEX_GATHER:
    case gcSHADER_VAR_CATEGORY_SAMPLE_LOCATION:
    case gcSHADER_VAR_CATEGORY_ENABLE_MULTISAMPLE_BUFFERS:
        return "";

    case gcSHADER_VAR_CATEGORY_STRUCT:
    case gcSHADER_VAR_CATEGORY_TOP_LEVEL_STRUCT:
        return "struct";

    case gcSHADER_VAR_CATEGORY_BLOCK:
        return "block";

    case gcSHADER_VAR_CATEGORY_BLOCK_MEMBER:
        return "block_member";

    case gcSHADER_VAR_CATEGORY_BLOCK_ADDRESS:
        return "block_addr";

    default:
        gcmASSERT(0);
        return "Invalid";
    }
}

static gctCONST_STRING
GetImageFormatQualifierName_(
    IN gceIMAGE_FORMAT Format
    )
{
    switch (Format)
    {
    /* float-image-format-qualifier. */
    case gcIMAGE_FORMAT_RGBA32F:
        return "rgba32f";
    case gcIMAGE_FORMAT_RGBA16F:
        return "rgba16f";
    case gcIMAGE_FORMAT_R32F:
        return "r32f";
    case gcIMAGE_FORMAT_RGBA8:
        return "rgba8";
    case gcIMAGE_FORMAT_RGBA8_SNORM:
        return "rgba8_snorm";

    /* int-image-format-qualifier. */
    case gcIMAGE_FORMAT_RGBA32I:
        return "rgba32i";
    case gcIMAGE_FORMAT_RGBA16I:
        return "rgba16i";
    case gcIMAGE_FORMAT_R32I:
        return "r32i";
    case gcIMAGE_FORMAT_RGBA8I:
        return "rgba8i";

    /* uint-image-format-qualifier. */
    case gcIMAGE_FORMAT_RGBA32UI:
        return "rgba32ui";
    case gcIMAGE_FORMAT_RGBA16UI:
        return "rgba16ui";
    case gcIMAGE_FORMAT_R32UI:
        return "r32ui";
    case gcIMAGE_FORMAT_RGBA8UI:
        return "rgba8ui";

    case gcIMAGE_FORMAT_DEFAULT:
        return "default";
    default:
        gcmASSERT(0);
        return gcvNULL;
    }
}

static gctCONST_STRING
GetPrecisionName_(
    IN gcSHADER_PRECISION Precision
)
{
    switch (Precision)
    {
    case gcSHADER_PRECISION_DEFAULT:
        return "defaultp";

    case gcSHADER_PRECISION_HIGH:
        return "highp";

    case gcSHADER_PRECISION_MEDIUM:
        return "mediump";

    case gcSHADER_PRECISION_LOW:
        return "lowp";

    case gcSHADER_PRECISION_ANY:
        return "anyp";

    default:
        gcmASSERT(0);
        return "Invalid";
    }
}
/*******************************************************************************
**                                  gcSL_GetName
********************************************************************************
**
**  Print the name.
**
**  INPUT:
**
**      gctUINT32 Length
**          Length of the name.
**
**      gctCONST_STRING Name
**          Pointer to the name.
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
gctINT
gcSL_GetName(
    IN gctUINT32 Length,
    IN gctCONST_STRING Name,
    OUT char * Buffer,
    gctUINT32 BufferSize
    )
{
    gctUINT offset = 0;
    gctUINT32 size;
#define _print_name(name, str)   case name:                                  \
                                    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, \
                                                 BufferSize, &offset, str)); \
                                    return offset;

    switch (Length)
    {
    _print_name(gcSL_POSITION, "#Position");
    _print_name(gcSL_POINT_SIZE, "#PointSize");
    _print_name(gcSL_COLOR, "#Color");
    _print_name(gcSL_DEPTH, "#Depth");
    _print_name(gcSL_FRONT_FACING, "#FrontFacing");
    _print_name(gcSL_POINT_COORD, "#PointCoord");
    _print_name(gcSL_POSITION_W, "#Position.w");
    _print_name(gcSL_FOG_COORD, "#FogFragCoord");
    _print_name(gcSL_VERTEX_ID, "#VertexID");
    _print_name(gcSL_INSTANCE_ID, "#InstanceID");
    _print_name(gcSL_WORK_GROUP_ID, "#WorkGroupID");
    _print_name(gcSL_LOCAL_INVOCATION_ID, "#LocalInvocationID");
    _print_name(gcSL_GLOBAL_INVOCATION_ID, "#GlobalInvocationID");
    _print_name(gcSL_HELPER_INVOCATION, "#HelperInvocation");
    _print_name(gcSL_FRONT_COLOR, "#FrontColor");
    _print_name(gcSL_BACK_COLOR, "#BackColor");
    _print_name(gcSL_FRONT_SECONDARY_COLOR, "#FrontSecondaryColor");
    _print_name(gcSL_BACK_SECONDARY_COLOR, "#BackSecondaryColor");
    _print_name(gcSL_TEX_COORD, "#TexCoord");
    _print_name(gcSL_SUBSAMPLE_DEPTH, "#Subsample_Depth");
    _print_name(gcSL_PERVERTEX, "#PerVertex");
    _print_name(gcSL_IN, "#in");
    _print_name(gcSL_OUT, "#out");
    _print_name(gcSL_INVOCATION_ID, "#InvocationID");
    _print_name(gcSL_PATCH_VERTICES_IN, "#PatchVerticesIn");
    _print_name(gcSL_PRIMITIVE_ID, "#PrimitiveID");
    _print_name(gcSL_TESS_LEVEL_OUTER, "#TessLevelOuter");
    _print_name(gcSL_TESS_LEVEL_INNER, "#TessLevelInner");
    _print_name(gcSL_LAYER, "#Layer");
    _print_name(gcSL_PRIMITIVE_ID_IN, "#PrimitiveIDIn");
    _print_name(gcSL_TESS_COORD, "#TessCoord");
    _print_name(gcSL_SAMPLE_ID, "#SampleId");
    _print_name(gcSL_SAMPLE_POSITION, "#SamplePosition");
    _print_name(gcSL_SAMPLE_MASK_IN, "#SampleMaskIn");
    _print_name(gcSL_SAMPLE_MASK, "#SampleMask");
    _print_name(gcSL_IN_POSITION, "#In_Position");
    _print_name(gcSL_IN_POINT_SIZE, "#In_PointSize");
    _print_name(gcSL_BOUNDING_BOX, "#BoundingBox");
    default:
        gcmASSERT((gctINT)Length > 0);
        break;
    }
#undef _print_name

    /* Copy the name into the buffer. */
    size = gcmMIN(Length, BufferSize - 1);
    if(size)
    {
        gcoOS_MemCopy(Buffer, Name, size);
    }
    Buffer[size] = '\0';
    return size;
}

/*******************************************************************************
**                                  _DumpSwizzle
********************************************************************************
**
**  Print the swizzle.
**
**  INPUT:
**
**      gcSL_SWIZZLE SwizzleX
**          Swizzle for the x component.
**
**      gcSL_SWIZZLE SwizzleY
**          Swizzle for the y component.
**
**      gcSL_SWIZZLE SwizzleW
**          Swizzle for the w component.
**
**      gcSL_SWIZZLE SwizzleZ
**          Swizzle for the z component.
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
_DumpSwizzle(
    IN gcSL_SWIZZLE SwizzleX,
    IN gcSL_SWIZZLE SwizzleY,
    IN gcSL_SWIZZLE SwizzleZ,
    IN gcSL_SWIZZLE SwizzleW,
    OUT char * Buffer,
    IN gctSIZE_T BufferSize
    )
{
    gctUINT offset = 0;

    static const char swizzle[] =
    {
        'x', 'y', 'z', 'w',
    };

    /* Don't print anything for the default swizzle (.xyzw). */
    if ((SwizzleX == gcSL_SWIZZLE_X)
    &&  (SwizzleY == gcSL_SWIZZLE_Y)
    &&  (SwizzleZ == gcSL_SWIZZLE_Z)
    &&  (SwizzleW == gcSL_SWIZZLE_W)
    )
    {
        return 0;
    }

    /* Print the period and the x swizzle. */
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                           ".%c", swizzle[SwizzleX]));

    /* Only continue of the other swizzles are different. */
    if ((SwizzleY != SwizzleX)
    ||  (SwizzleZ != SwizzleX)
    ||  (SwizzleW != SwizzleX)
    )
    {
        /* Append the y swizzle. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                               "%c", swizzle[SwizzleY]));

        /* Only continue of the other swizzles are different. */
        if ((SwizzleZ != SwizzleY) || (SwizzleW != SwizzleY) )
        {
            /* Append the z swizzle. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                   "%c", swizzle[SwizzleZ]));

            /* Only continue of the other swizzle are different. */
            if (SwizzleW != SwizzleZ)
            {
                /* Append the w swizzle. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                       "%c", swizzle[SwizzleW]));
            }
        }
    }

    /* Return the number of characters printed. */
    return offset;
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
    IN gcSL_TYPE        Type,
    IN gcSL_FORMAT      Format,
    IN gctUINT16        Index,
    IN gcSL_INDEXED     Mode,
    IN gctINT           Indexed,
    OUT char * Buffer,
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
                           "%s", type[Type]));
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                           "%s(%d", format[Format], gcmSL_INDEX_GET(Index, Index)));

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
                               type[gcSL_TEMP]));
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                               "(%d).%c",
                               Indexed,
                               index[Mode]));
    }
    else if (Indexed > 0)
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, "+%d", Indexed));
    }

    /* Append the final ')'. */
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, ")"));

    /* Return the number of characters printed. */
    return offset;
}

/*******************************************************************************
**                                   _DumpSource
********************************************************************************
**
**  Print a source operand.
**
**  INPUT:
**
**      gcSL_SOURCE Source
**          The source operand.
**
**      gcSL_INDEX Index
**          Index of the source operand.
**
**      gctINT Indexed
**          Index register if the source operand addressing mode is indexed.
**
**      gctBOOL AddComma
**          Prepend the source operand with a comma.
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
_DumpSource(
    IN gctSOURCE_t Source,
    IN gctUINT16 Index,
    IN gctINT Indexed,
    IN gctBOOL AddComma,
    OUT char * Buffer,
    IN gctSIZE_T BufferSize
    )
{
    gctUINT   offset = 0;
    gcSL_TYPE type   = gcmSL_SOURCE_GET(Source, Type);
    /* Only process source operand if it is valid. */
    if (type != gcSL_NONE)
    {
        if (AddComma)
        {
            /* Prefix a comma. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, ", "));
        }

        if (type == gcSL_CONSTANT)
        {
            /* Assemble the 32-bit value. */
            gctUINT32 value = Index | (Indexed << 16);

            switch (gcmSL_SOURCE_GET(Source, Format))
            {
            case gcSL_FLOAT:
                /* Print the floating point constant. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                       "%f", gcoMATH_UIntAsFloat(value)));
                break;

            case gcSL_INTEGER:
                /* Print the integer constant. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                       "%d", value));
                break;

            case gcSL_UINT32:
                /* Print the unsigned integer constant. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                       "%u", value));
                break;

            case gcSL_BOOLEAN:
                /* Print the boolean constant. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                       "%s", value ? "true" : "false"));
                break;
            }
        }
        else
        {
            gcSL_PRECISION precision = gcmSL_SOURCE_GET(Source, Precision);

            /* Decode the register. */
            offset += _DumpRegister(type,
                                    (gcSL_FORMAT) gcmSL_SOURCE_GET(Source, Format),
                                    Index,
                                    (gcSL_INDEXED) gcmSL_SOURCE_GET(Source, Indexed),
                                    Indexed,
                                    Buffer + offset,
                                    BufferSize - offset);

            if (type != gcSL_CONSTANT && type != gcSL_SAMPLER)
            {
                switch (precision)
                {
                    case gcSL_PRECISION_DEFAULT:
                        /* Print precision. */
                        gcmVERIFY_OK(
                            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                               "%s", ".dp"));
                        break;
                    case gcSL_PRECISION_HIGH:
                        /* Print precision. */
                        gcmVERIFY_OK(
                            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                               "%s", ".hp"));
                        break;
                    case gcSL_PRECISION_MEDIUM:
                        /* Print precision. */
                        gcmVERIFY_OK(
                            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                               "%s", ".mp"));
                        break;
                    case gcSL_PRECISION_LOW:
                        /* Print precision. */
                        gcmVERIFY_OK(
                            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                               "%s", ".lp"));
                        break;
                    case gcSL_PRECISION_ANY:
                        /* Print precision. */
                        gcmVERIFY_OK(
                            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                               "%s", ".anyp"));
                        break;
                    default:
                        gcmASSERT(0);
                }
            }

            if (gcmSL_SOURCE_GET(Source, Neg))
            {
                /* Print neg modifier. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                       "%s", ".neg"));

            }

            if (gcmSL_SOURCE_GET(Source, Abs))
            {
                /* Print abs modifier. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                                       "%s", ".abs"));

            }

            /* Decode the swizzle. */
            offset += _DumpSwizzle((gcSL_SWIZZLE) gcmSL_SOURCE_GET(Source, SwizzleX),
                                   (gcSL_SWIZZLE) gcmSL_SOURCE_GET(Source, SwizzleY),
                                   (gcSL_SWIZZLE) gcmSL_SOURCE_GET(Source, SwizzleZ),
                                   (gcSL_SWIZZLE) gcmSL_SOURCE_GET(Source, SwizzleW),
                                   Buffer + offset,
                                   BufferSize - offset);
        }
    }

    /* Return the number of characters printed. */
    return offset;
}

/*******************************************************************************
**                               _DumpSourceTargetFormat
********************************************************************************
**
**  Print a source which contains the target format used in gcSL_CONV.
**
**  INPUT:
**
**      gcSL_SOURCE Source
**          The source operand.
**
**      gcSL_INDEX Index
**          Index of the source operand.
**
**      gctINT Indexed
**          Index register if the source operand addressing mode is indexed.
**
**      gctBOOL AddComma
**          Prepend the source operand with a comma.
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
_DumpSourceTargetFormat(
    IN gctSOURCE_t Source,
    IN gctUINT16 Index,
    IN gctINT Indexed,
    IN gctBOOL AddComma,
    OUT char * Buffer,
    IN gctSIZE_T BufferSize
    )
{
    static gctCONST_STRING targetFormat[] =
    {
        "float",
        "int",
        "bool",
        "uint",
        "int8",
        "uint8",
        "int16",
        "uint16",
        "int64",
        "uint64",
        "snorm8",
        "unorm8",
        "float16",
        "float64",
        "snorm16",
        "unorm16",
        "invalid",
    };
    gctUINT offset = 0;
    gctUINT32 value = Index | (Indexed << 16);

    if (AddComma)
    {
        /* Prefix a comma. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, &offset, ", "));
    }
    gcmASSERT(gcmSL_SOURCE_GET(Source, Type) == gcSL_CONSTANT);
    /* Assemble the 32-bit value. */

    gcmASSERT(gcmSL_SOURCE_GET(Source, Format) == gcSL_UINT32);

    gcmASSERT(value <= gcSL_INVALID);

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(Buffer, BufferSize, &offset,
                           "%s", targetFormat[value]));

    /* Return the number of characters printed. */
    return offset;
}

/*******************************************************************************
**                                    _DumpDataFlowList
********************************************************************************
**
**  Print data flow list.
**
**  INPUT:
**
**      gctCONST_STRING Title
**          Pointer to the title for the list.
**
**      gcsLINKTREE_LIST_PTR List
**          Pointer to the first gcsLINKTREE_LIST_PTR structure to print.
**
**      gctUINT Offset
**          Pointer to the offset of output.
**
**  OUTPUT:
**
**      char * Buffer
**          Pointer to the buffer to be used as output.
*/
static void
_DumpDataFlowList(
    IN gcoOS Os,
    IN gctFILE File,
    IN gctCONST_STRING Title,
    IN gcOPT_LIST List,
    IN gctUINT * Offset,
    OUT char * Buffer,
    IN gctSIZE_T BufferSize
    )
{
    gctSIZE_T length;

    if (List == gcvNULL)
    {
        /* Don't print anything on an empty list. */
        return;
    }

    length = gcoOS_StrLen(Title, gcvNULL) + 8;    /* extra indent for instruction number. */
    length = gcmMIN(length, BufferSize);

    /* Extra indent. */
    for (; *Offset < 8; (*Offset)++)
    {
        if (*Offset >= BufferSize)
        {
            break;
        }

        Buffer[*Offset] = ' ';
    }

    /* Print the title. */
    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, Title));

    /* Walk while there are entries in the list. */
    while (List != gcvNULL)
    {
        /* Check if we have reached the right margin. */
        if (*Offset > 70)
        {
            /* Dump the assembled line. */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, ",\n"));
            gcOpt_DumpBuffer(Os, File, Buffer, *Offset);

            /* Indent to the end of the title. */
            for (*Offset = 0; *Offset < length; (*Offset)++)
            {
                Buffer[*Offset] = ' ';
            }
        }
        else if (*Offset > length)
        {
            /* Print comma and space. */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, ", "));
        }

        /* output list index. */
        if (List->index >= 0)
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
                                   "%d", List->code->id));
        }
        else if (List->index == gcvOPT_INPUT_REGISTER)
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, "input"));
        }
        else if (List->index == gcvOPT_OUTPUT_REGISTER)
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, "output"));
        }
        else if (List->index == gcvOPT_GLOBAL_REGISTER)
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, "global"));
        }
        else if (List->index == gcvOPT_UNDEFINED_REGISTER)
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, "undefined"));
        }

        /* Move to the next entry in the list. */
        List = List->next;
    }

    /* Dump the final assembled line. */
    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, "\n"));
    gcOpt_DumpBuffer(Os, File, Buffer, *Offset);
    *Offset = 0;
}

/*******************************************************************************
**                                    _DumpCodeDataFlow
********************************************************************************
**
**  Print data flow list for one code.
**
**  INPUT:
**
**      gcOPT_CODE code
**          Pointer to the code.
**
**  OUTPUT:
**
**      none
*/
static void
_DumpCodeDataFlow(
    IN gcoOS Os,
    IN gctFILE File,
    IN gcOPT_CODE code)
{
    char buffer[256];
    gctUINT offset = 0;

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                           "  %4d: ", code->id));

    if (code->users)
    {
        _DumpDataFlowList(Os,
                          File,
                          "Users: ",
                          code->users,
                          &offset,
                          buffer, gcmSIZEOF(buffer));
    }

    if (code->dependencies0)
    {
        _DumpDataFlowList(Os,
                          File,
                          "Src 0: ",
                          code->dependencies0,
                          &offset,
                          buffer, gcmSIZEOF(buffer));
    }

    if (code->dependencies1)
    {
        _DumpDataFlowList(Os,
                          File,
                          "Src 1: ",
                          code->dependencies1,
                          &offset,
                          buffer, gcmSIZEOF(buffer));
    }

    if (code->prevDefines)
    {
        _DumpDataFlowList(Os,
                          File,
                          "P Def: ",
                          code->prevDefines,
                          &offset,
                          buffer, gcmSIZEOF(buffer));
    }

    if (code->nextDefines)
    {
        _DumpDataFlowList(Os,
                          File,
                          "N Def: ",
                          code->nextDefines,
                          &offset,
                          buffer, gcmSIZEOF(buffer));
    }

    if (offset > 0)
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, "\n"));
        gcOpt_DumpBuffer(Os, File, buffer, offset);
    }
}

/*******************************************************************************
**                                    _DumpDataFlow
********************************************************************************
**
**  Print data flow list.
**
**  INPUT:
**
**      gcOPT_FUNCTION Function
**          Pointer to a function.
**
**  OUTPUT:
**
**      char * Buffer
**          Pointer to the buffer to be used as output.
*/
static void
_DumpDataFlow(
    IN gcoOS Os,
    IN gctFILE File,
    IN gcOPT_FUNCTION Function
    )
{
    gcOPT_CODE code;

    /* Output data flow for each code. */
    for (code = Function->codeHead; code != gcvNULL && code != Function->codeTail->next; code = code->next)
    {
        _DumpCodeDataFlow(Os, File, code);
    }
}

static struct _DECODE
{
    gctCONST_STRING opcode;
    gctBOOL     hasTarget;
    gctBOOL     hasLabel;
}
decode[] =
{
    { "NOP", gcvFALSE, gcvFALSE },
    { "MOV", gcvTRUE, gcvFALSE },
    { "SAT", gcvTRUE, gcvFALSE },
    { "DP3", gcvTRUE, gcvFALSE },
    { "DP4", gcvTRUE, gcvFALSE },
    { "ABS", gcvTRUE, gcvFALSE },
    { "JMP", gcvFALSE, gcvTRUE  },
    { "ADD", gcvTRUE, gcvFALSE },
    { "MUL", gcvTRUE, gcvFALSE },
    { "RCP", gcvTRUE, gcvFALSE },
    { "SUB", gcvTRUE, gcvFALSE },
    { "KILL", gcvFALSE, gcvFALSE },
    { "TEXLD", gcvTRUE, gcvFALSE },
    { "CALL", gcvFALSE, gcvTRUE  },
    { "RET", gcvFALSE, gcvFALSE },
    { "NORM", gcvTRUE, gcvFALSE },
    { "MAX", gcvTRUE, gcvFALSE },
    { "MIN", gcvTRUE, gcvFALSE },
    { "POW", gcvTRUE, gcvFALSE },
    { "RSQ", gcvTRUE, gcvFALSE },
    { "LOG", gcvTRUE, gcvFALSE },
    { "FRAC", gcvTRUE, gcvFALSE },
    { "FLOOR", gcvTRUE, gcvFALSE },
    { "CEIL", gcvTRUE, gcvFALSE },
    { "CROSS", gcvTRUE, gcvFALSE },
    { "TEXLDP", gcvTRUE, gcvFALSE },
    { "TEXBIAS", gcvFALSE, gcvFALSE },
    { "TEXGRAD", gcvFALSE, gcvFALSE },
    { "TEXLOD", gcvFALSE, gcvFALSE },
    { "SIN", gcvTRUE, gcvFALSE },
    { "COS", gcvTRUE, gcvFALSE },
    { "TAN", gcvTRUE, gcvFALSE },
    { "EXP", gcvTRUE, gcvFALSE },
    { "SIGN", gcvTRUE, gcvFALSE },
    { "STEP", gcvTRUE, gcvFALSE },
    { "SQRT", gcvTRUE, gcvFALSE },
    { "ACOS", gcvTRUE, gcvFALSE },
    { "ASIN", gcvTRUE, gcvFALSE },
    { "ATAN", gcvTRUE, gcvFALSE },
    { "SET", gcvTRUE, gcvFALSE },
    { "DSX", gcvTRUE, gcvFALSE },
    { "DSY", gcvTRUE, gcvFALSE },
    { "FWIDTH", gcvTRUE, gcvFALSE },
    { "DIV", gcvTRUE, gcvFALSE },
    { "MOD", gcvTRUE, gcvFALSE },
    { "AND_BITWISE",gcvTRUE, gcvFALSE },
    { "OR_BITWISE", gcvTRUE, gcvFALSE },
    { "XOR_BITWISE",gcvTRUE, gcvFALSE },
    { "NOT_BITWISE",gcvTRUE, gcvFALSE },
    { "LSHIFT", gcvTRUE, gcvFALSE },
    { "RSHIFT", gcvTRUE, gcvFALSE },
    { "ROTATE", gcvTRUE, gcvFALSE },
    { "BITSEL", gcvTRUE, gcvFALSE },
    { "LEADZERO", gcvTRUE, gcvFALSE },
    { "LOAD", gcvTRUE, gcvFALSE },
    { "STORE", gcvTRUE, gcvFALSE },
    { "BARRIER", gcvFALSE, gcvFALSE },
    { "STORE1", gcvTRUE, gcvFALSE },
    { "ATOMADD", gcvTRUE, gcvFALSE },
    { "ATOMSUB", gcvTRUE, gcvFALSE },
    { "ATOMXCHG", gcvTRUE, gcvFALSE },
    { "ATOMCMPXCHG",gcvTRUE, gcvFALSE },
    { "ATOMMIN", gcvTRUE, gcvFALSE },
    { "ATOMMAX", gcvTRUE, gcvFALSE },
    { "ATOMOR", gcvTRUE, gcvFALSE },
    { "ATOMAND", gcvTRUE, gcvFALSE },
    { "ATOMXOR", gcvTRUE, gcvFALSE },
    { "TEXLDPCF", gcvTRUE, gcvFALSE },
    { "TEXLDPCFPROJ",gcvTRUE, gcvFALSE },
    { "TEXLODQ", gcvTRUE, gcvFALSE },
    { "FLUSH", gcvFALSE, gcvFALSE },
    { "JMP_ANY", gcvTRUE, gcvFALSE },
    { "BITRANGE", gcvTRUE, gcvFALSE },
    { "BITRANGE1", gcvTRUE, gcvFALSE },
    { "BITEXTRACT", gcvTRUE, gcvFALSE },
    { "BITINSERT", gcvTRUE, gcvFALSE },
    { "FINDLSB", gcvTRUE, gcvFALSE },
    { "FINDMSB", gcvTRUE, gcvFALSE },
    { "IMAGE_OFFSET",gcvFALSE, gcvFALSE },
    { "IMAGE_ADDR,",gcvTRUE, gcvFALSE },
    { "SINPI", gcvTRUE, gcvFALSE },
    { "COSPI", gcvTRUE, gcvFALSE },
    { "TANPI", gcvTRUE, gcvFALSE },
    { "ADDLO", gcvTRUE, gcvFALSE },
    { "MULLO", gcvTRUE, gcvFALSE },
    { "CONV", gcvTRUE, gcvFALSE },
    { "GETEXP", gcvTRUE, gcvFALSE },
    { "GETMANT", gcvTRUE, gcvFALSE },
    { "MULHI", gcvTRUE, gcvFALSE },
    { "CMP", gcvTRUE, gcvFALSE },
    { "I2F", gcvTRUE, gcvFALSE },
    { "F2I", gcvTRUE, gcvFALSE },
    { "ADDSAT", gcvTRUE, gcvFALSE },
    { "SUBSAT", gcvTRUE, gcvFALSE },
    { "MULSAT", gcvTRUE, gcvFALSE },
    { "DP2", gcvTRUE, gcvFALSE },
    { "UNPACK", gcvTRUE, gcvFALSE },
    { "IMAGE_WR", gcvTRUE, gcvFALSE },
    { "SAMPLER_ADD",gcvTRUE, gcvFALSE },
    { "MOVA", gcvTRUE, gcvFALSE },
    { "IMAGE_RD", gcvTRUE, gcvFALSE },
    { "IMAGE_SAMPLER",gcvTRUE, gcvFALSE },
    { "NORM_MUL", gcvTRUE, gcvFALSE },
    { "NORM_DP2", gcvTRUE, gcvFALSE },
    { "NORM_DP3", gcvTRUE, gcvFALSE },
    { "NORM_DP4", gcvTRUE, gcvFALSE },
    { "PRE_DIV", gcvTRUE, gcvFALSE },
    { "PRE_LOG2", gcvTRUE, gcvFALSE },
    { "TEXGATHER", gcvFALSE, gcvFALSE },
    { "TEXFETCH_MS",gcvFALSE, gcvFALSE },
    { "POPCOUNT", gcvTRUE, gcvFALSE },
    { "BIT_REVERSAL",gcvTRUE, gcvFALSE },
    { "BYTE_REVERSAL",gcvTRUE, gcvFALSE },
    { "TEXPCF", gcvFALSE, gcvFALSE },
    { "UCARRY", gcvTRUE, gcvFALSE },
    { "TEXU", gcvTRUE, gcvFALSE },
    { "TEXU_LOD", gcvTRUE, gcvFALSE },
    { "MEM_BARRIER",gcvFALSE, gcvFALSE },
    { "SAMPLER_ASSIGN",gcvTRUE, gcvFALSE },
    { "GET_SAMPLER_IDX",gcvTRUE, gcvFALSE },
    { "IMAGE_RD_3D", gcvTRUE, gcvFALSE },
    { "IMAGE_WR_3D", gcvTRUE, gcvFALSE },
    { "CLAMP0MAX", gcvTRUE, gcvFALSE },
    { "FMA_MUL", gcvTRUE, gcvFALSE },
    { "FMA_ADD", gcvTRUE, gcvFALSE },
    { "ATTR_ST", gcvTRUE, gcvFALSE },
    { "ATTR_LD", gcvTRUE, gcvFALSE },
    { "EMIT_VERTEX", gcvFALSE, gcvFALSE },
    { "END_PRIMITIVE", gcvFALSE, gcvFALSE },
    { "ARCTRIG0", gcvTRUE, gcvFALSE },
    { "ARCTRIG1", gcvTRUE, gcvFALSE },
    { "MUL_Z", gcvTRUE, gcvFALSE },
    { "NEG", gcvTRUE, gcvFALSE },
    { "LONGLO", gcvTRUE, gcvFALSE },
    { "LONGHI", gcvTRUE, gcvFALSE },
    { "MOV_LONG", gcvTRUE, gcvFALSE },
    { "MADSAT", gcvTRUE, gcvFALSE },
    { "COPY", gcvTRUE, gcvFALSE },
    { "IMAGE_ADDR_3D,",gcvTRUE, gcvFALSE },
    { "GET_SAMPLER_LMM",gcvTRUE, gcvFALSE },
    { "GET_SAMPLER_LBS",gcvTRUE, gcvFALSE },
};

char _checkDecodeArray_size[sizeof(decode)/sizeof(decode[0]) == gcSL_MAXOPCODE];

/*******************************************************************************
**                                    _DumpIR
********************************************************************************
**
**  Print IR to File
**
**  INPUT:
**
**      gctINT CodeId
**          the instruction's code id in a funciton, do not print the id if -1
**
**      gcSL_INSTRUCTION code
**          Pointer to an instruction.
**
**  OUTPUT:
**
**      none
*/
static void
_DumpIR(
    IN gcoOS Os,
    IN gctFILE File,
    IN gctINT  CodeId,
    IN gcSL_INSTRUCTION code)
{
    char buffer[256];
    gctUINT offset = 0;

    static gctCONST_STRING condition[] =
    {
        "",
        ".NE",
        ".LE",
        ".L",
        ".EQ",
        ".G",
        ".GE",
        ".AND",
        ".OR",
        ".XOR",
        ".NZ",
        ".Z",
        ".GEZ",
        ".GZ",
        ".LEZ",
        ".LZ",
        ".ALLMSB",
        ".ANYMSB",
        ".SELMSB",
    };

    static gctCONST_STRING rounding[] =
    {
        "",
        ".RTZ",
        ".RTNE",
        ".RTP",
        ".RTN",
    };

    static const char * saturation[] =
    {
        "", /* default */
        ".SAT", /* saturate */
    };

    gctTARGET_t target;
    offset = 0;

    /* Get target. */
    target = code->temp;

    /* Instruction number, opcode and condition. */
    if (CodeId != -1) {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                           "  %4d: ",
                           CodeId));
    }
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                           "%s%s%s%s",
                           decode[gcmSL_OPCODE_GET(code->opcode, Opcode)].opcode,
                           condition[gcmSL_TARGET_GET(target, Condition)],
                           saturation[gcmSL_OPCODE_GET(code->opcode, Sat)],
                           rounding[gcmSL_OPCODE_GET(code->opcode, Round)]));

    /* Align to column 24. */
    do
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, " "));
    }
    while (offset < 24);

    /* Does the instruction specify a target? */
    if (decode[gcmSL_OPCODE_GET(code->opcode, Opcode)].hasTarget)
    {
        /* Decode register. */
        offset += _DumpRegister(gcSL_TEMP,
                                (gcSL_FORMAT) gcmSL_TARGET_GET(target, Format),
                                code->tempIndex,
                                (gcSL_INDEXED) gcmSL_TARGET_GET(target, Indexed),
                                code->tempIndexed,
                                buffer + offset,
                                gcmSIZEOF(buffer) - offset);

        switch (gcmSL_TARGET_GET(target, Precision))
        {
            case gcSL_PRECISION_DEFAULT:
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                        ".dp"));
                break;
            case gcSL_PRECISION_HIGH:
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                        ".hp"));
                break;
            case gcSL_PRECISION_MEDIUM:
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                        ".mp"));
                break;
            case gcSL_PRECISION_LOW:
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                        ".lp"));
                break;
            case gcSL_PRECISION_ANY:
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                        ".anyp"));
                break;
            default:
                gcmASSERT(0);
        }

        if (gcmSL_TARGET_GET(target, Enable) != gcSL_ENABLE_XYZW)
        {
            /* Enable dot. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "."));

            if (gcmSL_TARGET_GET(target, Enable) & gcSL_ENABLE_X)
            {
                /* Enable x. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "x"));
            }

            if (gcmSL_TARGET_GET(target, Enable) & gcSL_ENABLE_Y)
            {
                /* Enable y. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "y"));
            }

            if (gcmSL_TARGET_GET(target, Enable) & gcSL_ENABLE_Z)
            {
                /* Enable z. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "z"));
            }

            if (gcmSL_TARGET_GET(target, Enable) & gcSL_ENABLE_W)
            {
                /* Enable w. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "w"));
            }
        }
    }

    /* Does the instruction specify a label? */
    else if (decode[gcmSL_OPCODE_GET(code->opcode, Opcode)].hasLabel)
    {
        /* Append label. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "%d", code->tempIndex));
    }

    /* Append source operand 0. */
    offset += _DumpSource(code->source0,
                          code->source0Index,
                          code->source0Indexed,
                          offset > 24,
                          buffer + offset, gcmSIZEOF(buffer) - offset);

    /* Append source operand 1. */
    if(gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_CONV) {
       offset += _DumpSourceTargetFormat(code->source1,
                          code->source1Index,
                          code->source1Indexed,
                          offset > 24,
                          buffer + offset, gcmSIZEOF(buffer) - offset);
    }
    else {
       offset += _DumpSource(code->source1,
                          code->source1Index,
                          code->source1Indexed,
                          offset > 24,
                          buffer + offset, gcmSIZEOF(buffer) - offset);
    }

    /* Dump the instruction. */
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, "\n"));
    gcOpt_DumpBuffer(Os, File, buffer, offset);
}

/* dump instruction to debugger output */
void dbg_dumpIR(IN gcSL_INSTRUCTION Inst, gctINT n)
{

    _DumpIR(gcvNULL, gcvNULL, n, Inst);
}

/* dump optimizer to debugger output */
void dbg_dumpOptimizer(IN gcOPTIMIZER Optimizer)
{
    gcOpt_Dump(gcvNULL, "Dump Optimizer", Optimizer, gcvNULL);
}

/* dump shader to debugger output */
void dbg_dumpShader(IN gcSHADER Shader)
{
    gcOpt_Dump(gcvNULL, "Dump Shader", gcvNULL, Shader);
}

/* dump code with data flow to debugger output */
void dbg_dumpCode(IN gcOPT_CODE Code)
{
    _DumpIR(gcvNULL, gcvNULL, Code->id, &Code->instruction);
    _DumpCodeDataFlow(gcvNULL, gcvNULL, Code);
}

gctINT gcvDumpOption =
            0; /* gceLTC_DUMP_EVALUATION | gceLTC_DUMP_EXPESSION | gceLTC_DUMP_COLLECTING | gceLTC_DUMP_UNIFORM;*/

/* return true if the dump option Opt is set, otherwise return false */
gctBOOL
gcDumpOption(gctINT Opt)
{
    return (gcvDumpOption & Opt) != 0;
}

static void
_PostOrderVariable(
    IN gcSHADER Shader,
    IN gcVARIABLE rootVariable,
    IN gcVARIABLE firstVariable,
    IN gctBOOL_PTR StartCalc,
    IN gctINT            fisrtTempIndex,
    OUT gctUINT *        Start,
    OUT gctUINT *        End,
    OUT gcSHADER_TYPE *  TempTypeArray
)
{
    gctUINT          start = 0xffffffff, end = 0;
    gcVARIABLE       var;
    gctINT16        varIndex;

    if (!*StartCalc && (rootVariable == firstVariable))
        *StartCalc = gcvTRUE;

    if (rootVariable->firstChild != -1)
    {
        gcmASSERT (isVariableStruct(rootVariable));

        varIndex = rootVariable->firstChild;
        while (varIndex != -1)
        {
            gctUINT          startTemp = 0, endTemp = 0;

            var = Shader->variables[varIndex];

            if (!*StartCalc && (var == firstVariable))
                *StartCalc = gcvTRUE;

            _PostOrderVariable(Shader, var, firstVariable, StartCalc,
                               fisrtTempIndex,
                               &startTemp, &endTemp, TempTypeArray);

            if (*StartCalc)
            {
                /* Currently, only select the maximum range, but
                   we can improve it to get separated ranges for
                   more accurate analysis later */
                if (startTemp < start) start = startTemp;
                if (endTemp > end) end = endTemp;
            }

            varIndex = var->nextSibling;
        }
    }

    /* Process root */
    if (isVariableNormal(rootVariable))
    {
        if (*StartCalc)
        {
            gctINT arraySize = 1;

            arraySize = GetVariableKnownArraySize(rootVariable);

            start = rootVariable->tempIndex;
            end = rootVariable->tempIndex + arraySize * gcmType_Rows(rootVariable->u.type);
            if (TempTypeArray != gcvNULL)
            {
                gctINT i;
                gcmASSERT((gctINT)start >= fisrtTempIndex);
                for (i=start; i < (gctINT)end; i++)
                {
                    TempTypeArray[i - fisrtTempIndex] =
                        gcmType_RowType(rootVariable->u.type);
                }
            }
        }
    }
    else
    {
        /* Nothing to do since start and end have been cal when visiting its children. */
    }

    *Start = start;
    *End = end;
}

static gceSTATUS
_GetVariableIndexingRangeForDump(
    IN gcSHADER Shader,
    IN gcVARIABLE variable,
    IN gctBOOL whole,
    OUT gctUINT *Start,
    OUT gctUINT *End
    )
{
    gceSTATUS        status = gcvSTATUS_OK;
    gctINT           root = -1;
    gcVARIABLE       var;
    gctBOOL          bStartCalc;

    /* Find arrayed root for this variable */
    var = variable;
    while (var)
    {
        if (var->parent != -1)
        {
            gctINT parent = var->parent;
            var = Shader->variables[var->parent];

            /* Find the out-most array */
            if (GetVariableArrayLengthCount(var) > 0)
            {
                root = parent;
                break;
            }
        }
        else
            var = gcvNULL;
    }

    /* To control whole or part of struct element array */
    if (whole)
        bStartCalc = gcvTRUE;
    else
        bStartCalc = gcvFALSE;

    /* Post order to calc range */
    _PostOrderVariable(Shader, (root == -1) ? variable : Shader->variables[root],
                       variable, &bStartCalc, -1, Start, End, gcvNULL);

    return status;
}


/*******************************************************************************
**                                   gcDump_Shader
********************************************************************************
**
**  Print the shader for debugging.
**
**  INPUT:
**
**      gctCONST_STRING Text
**          Pointer to text to print before dumping shader, etc.
**
**      gcOPTIMIZER Optimizer
**          Pointer to the gcOPTIMIZER structure to print.
**
**      gcSHADER Shader
**          Pointer to a gcSHADER object holding information about the compiled shader.
**          Shader is used only when Optimizer is gcvNULL.
*/
void
gcDump_Shader(
    IN gctFILE         File,
    IN gctCONST_STRING Text,
    IN gctPOINTER      Optimizer,
    IN gcSHADER        Shader,
    IN gctBOOL         PrintHeaderFooter
    )
{
    gctSIZE_T i;
    char buffer[2048];
    gctUINT offset = 0;
    gcoOS os = gcvNULL;
    gcSHADER shader = Shader;
    gctBOOL dumpOptimizer = gcvFALSE;
    gctUINT functionCount;
    gcSL_INSTRUCTION code;
    gcOPT_CODE codeNode = gcvNULL;
    gctUINT codeCount;
    gcOPTIMIZER     optimizer = (gcOPTIMIZER) Optimizer;

    static gctCONST_STRING doubleDashLine = "===============================================================================\n";
    static gctCONST_STRING dashLine = "*******************************************************************************\n";

    gcmHEADER_ARG("File=%p Text=0x%x optimizer=%p Shader=%p",
                    File, Text, optimizer, Shader);
    gcmTRACE(gcvLEVEL_VERBOSE, "Text=%s", Text);

    gcmASSERT(optimizer != gcvNULL || Shader != gcvNULL);

    if (optimizer != gcvNULL)
    {
        dumpOptimizer = gcvTRUE;
        shader = optimizer->shader;
    }

    if (PrintHeaderFooter == gcvTRUE)
    {
        /* Print header. */
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "%s", doubleDashLine));
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "%s (id:%d)\n", Text, Shader->_id));
        gcOpt_DumpBuffer(os, File, buffer, offset);
    }

    if (dumpOptimizer)
    {
        functionCount = optimizer->functionCount;
        codeCount = optimizer->codeTail->id + 1;
        code = &optimizer->codeHead->instruction;
    }
    else
    {
        if(shader == gcvNULL)
        {
            gcmASSERT(shader);
            gcmFOOTER_NO();
            return;
        }
        functionCount = shader->functionCount + shader->kernelFunctionCount;
        codeCount = shader->codeCount;
        code = shader->code;
    }

    /***************************************************************************
    ********************************************************* Dump functions. **
    ***************************************************************************/

    if (functionCount > 0)
    {
        offset = 0;

        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "%s[FUNCTIONS]\n\n  main() := [%u-%u]\n",
                               dashLine, 0, codeCount - 1));
        gcOpt_DumpBuffer(os, File, buffer, offset);

        /* Walk all functions. */
        for (i = 0; i < functionCount; ++i)
        {
            gctUINT32 nameLength;
            char * name;
            gctUINT codeStart, codeEnd;
            gctUINT32 argumentCount;
            gcsFUNCTION_ARGUMENT_PTR arguments;
            gctUINT32 j;

            offset = 0;

            if (dumpOptimizer)
            {
                if (optimizer->functionArray[i].codeHead != gcvNULL)
                {
                    codeStart = optimizer->functionArray[i].codeHead->id;
                    codeEnd   = optimizer->functionArray[i].codeTail->id;
                }
                else
                {
                    codeStart = codeEnd = 0;
                }
                if (optimizer->functionArray[i].shaderFunction)
                {
                    gcFUNCTION shaderFunction = optimizer->functionArray[i].shaderFunction;
                    nameLength = shaderFunction->nameLength;
                    name       = shaderFunction->name;
                    argumentCount = shaderFunction->argumentCount;
                    arguments     = shaderFunction->arguments;
                }
                else
                {
                    gcKERNEL_FUNCTION kernelFunction = optimizer->functionArray[i].kernelFunction;
                    nameLength = kernelFunction->nameLength;
                    name       = kernelFunction->name;
                    argumentCount = kernelFunction->argumentCount;
                    arguments     = kernelFunction->arguments;
                }
            }
            else
            {
                if (i < shader->functionCount)
                {
                    gcFUNCTION shaderFunction = shader->functions[i];
                    codeStart = shaderFunction->codeStart;
                    codeEnd   = codeStart + shaderFunction->codeCount - 1;
                    nameLength = shaderFunction->nameLength;
                    name       = shaderFunction->name;
                    argumentCount = shaderFunction->argumentCount;
                    arguments     = shaderFunction->arguments;
                }
                else
                {
                    gcKERNEL_FUNCTION kernelFunction = shader->kernelFunctions[i - shader->functionCount];
                    codeStart = kernelFunction->codeStart;
                    codeEnd   = kernelFunction->codeEnd - 1;
                    nameLength = kernelFunction->nameLength;
                    name       = kernelFunction->name;
                    argumentCount = kernelFunction->argumentCount;
                    arguments     = kernelFunction->arguments;
                }
            }

            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, "\n  "));

            offset += gcSL_GetName(nameLength,
                                name,
                                buffer + offset,
                                gcmSIZEOF(buffer) - offset);

            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "() := [%u-%u]\n",
                                   codeStart, codeEnd));
            gcOpt_DumpBuffer(os, File, buffer, offset);

            for (j = 0; j < argumentCount; ++j)
            {
                static const char * const qualifier[] = {" I", " O", "IO" };
                static const char * const precision[] = {"dp", "hp", "mp", "lp", "anyp" };
                offset = 0;

                gcmASSERT((arguments[j].qualifier <= gcvFUNCTION_INOUT));

                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "    argument(%u) [%s] := %s temp(%u)",
                                       j,
                                       qualifier[arguments[j].qualifier],
                                       precision[arguments[j].precision],
                                       arguments[j].index));

                switch (arguments[j].enable)
                {
                case 0x1:
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           ".x"));
                    break;

                case 0x3:
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           ".xy"));
                    break;

                case 0x7:
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           ".xyz"));
                    break;
                }

                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "\n"));
                gcOpt_DumpBuffer(os, File, buffer, offset);
            }
        }
    }

    /***************************************************************************
    ** I: Dump shader.
    */

    offset = 0;
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
        "%s\n[SHADER (id:%d)]\n\n", dashLine, shader->_id));
    gcOpt_DumpBuffer(os, File, buffer, offset);

    /* Walk all attributes. */
    for (i = 0; i < shader->attributeCount; i++)
    {
        /* Get the gcATTRIBUTE pointer. */
        gcATTRIBUTE attribute = shader->attributes[i];

        if (attribute != gcvNULL)
        {
            offset = 0;

            /* Attribute header and type. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "  attribute(%d) := %s%s%s %s %s ",
                                   i,
                                   gcmATTRIBUTE_isCentroid(attribute) ? "centroid " : gcmATTRIBUTE_isSample(attribute) ? "sample " : "",
                                   gcmATTRIBUTE_isPerPatch(attribute) ? "patch " : gcmATTRIBUTE_isPerVertexArray(attribute) ? "PerVertexArray " : "",
                                   (attribute->shaderMode == gcSHADER_SHADER_FLAT) ? "flat" : "smooth",
                                   GetPrecisionName_(attribute->precision),
                                   gcmType_Name(attribute->type)));

            /* Append name. */
            offset += gcSL_GetName(attribute->nameLength,
                                attribute->name,
                                buffer + offset,
                                gcmSIZEOF(buffer) - offset);

            if (GetATTRIsArray(attribute) > 0)
            {
                /* Append array size. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "[%d]", attribute->arraySize));
            }

            /* dump the layout location*/
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " (location = %d)", attribute->location));

            /* dump the field index*/
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " (field index = %d)", attribute->fieldIndex));

            /* dump the flags */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " flags:"));
            if(gcmATTRIBUTE_isStaticallyUsed(attribute))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " Statically_Used"));
            }

            /* Dump the attribute. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, ";\n"));
            gcOpt_DumpBuffer(os, File, buffer, offset);
        }
    }

    /* walk all variables */
    for (i=0; i < shader->variableCount; i++)
    {
        /* Get the gcATTRIBUTE pointer. */
        gcVARIABLE variable = shader->variables[i];

        if (variable == gcvNULL)
            continue;
        if ((isVariableNormal(variable) && !GetVariableIsOtput(variable))  /* output is handled later */
            || isVariableBlockMember(variable))
        {
            gctUINT startIndex, endIndex;
            gctINT j;
            offset = 0;

            /* Attribute header and type. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "  %s %s ",
                                   GetPrecisionName_(variable->precision),
                                   gcmType_Name(variable->u.type)));

            /* Append name. */
            offset += gcSL_GetName(variable->nameLength,
                                variable->name,
                                buffer + offset,
                                gcmSIZEOF(buffer) - offset);

            /* Append array size. */
            for (j = 0; j < variable->arrayLengthCount; j++)
            {
                if (variable->arrayLengthList[j] == -1)
                {
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,"[]"));
                }
                else
                {
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           "[%d]", variable->arrayLengthList[j]));
                }
            }

            if (isVariableNormal(variable))
            {
                _GetVariableIndexingRangeForDump(shader, variable, gcvFALSE,
                                                  &startIndex, &endIndex);
                gcmASSERT(startIndex == variable->tempIndex);

                /* dump temp registers */
                if (endIndex > startIndex + 1)
                {
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           " ==> temp(%d - %d);\n", startIndex,
                                           endIndex -1));
                }
                else
                {
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           " ==> temp(%d);\n", startIndex));
                }
            }
            else
            {
                gcmASSERT(isVariableBlockMember(variable));
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, " :offset %d;\n", variable->offset));
            }
            /* Dump the variable. */
            gcOpt_DumpBuffer(os, File, buffer, offset);
        }
    }

    /* Walk all uniform blocks. */
    for (i = 0; i < shader->uniformBlockCount; i++)
    {
        /* Get the gcUNIFORM pointer. */
        gcsUNIFORM_BLOCK uniformBlock = shader->uniformBlocks[i];

        if (uniformBlock != gcvNULL)
        {
            offset = 0;

                /* Uniform header and type. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                    "  uniformblock(%d) := ",
                                    i));

            /* Append name. */
            offset += gcSL_GetName(uniformBlock->nameLength,
                                uniformBlock->name,
                                buffer + offset,
                                gcmSIZEOF(buffer) - offset);


            /* Append binding. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   " :binding %d", uniformBlock->interfaceBlockInfo.binding));

            /* flags */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " flags:"));
            if(HasIBIFlag(&uniformBlock->interfaceBlockInfo, gceIB_FLAG_STATICALLY_USED))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " Statically_Used"));
            }

            /* Dump the uniform. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, ";\n"));
            gcOpt_DumpBuffer(os, File, buffer, offset);
        }
    }

    /* Walk all storage blocks. */
    for (i = 0; i < shader->storageBlockCount; i++)
    {
        /* Get the gcsSTORAGE_BLOCK pointer. */
        gcsSTORAGE_BLOCK storageBlock = shader->storageBlocks[i];

        if (storageBlock != gcvNULL)
        {
            offset = 0;

                /* Uniform header and type. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                    "  storageblock(%d) := ",
                                    i));

            /* Append name. */
            offset += gcSL_GetName(storageBlock->nameLength,
                                storageBlock->name,
                                buffer + offset,
                                gcmSIZEOF(buffer) - offset);


            /* Append binding. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   " :binding %d", storageBlock->interfaceBlockInfo.binding));

            /* flags */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " flags:"));
            if(HasIBIFlag(&storageBlock->interfaceBlockInfo, gceIB_FLAG_STATICALLY_USED))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " Statically_Used"));
            }

            /* Dump the uniform. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, ";\n"));
            gcOpt_DumpBuffer(os, File, buffer, offset);
        }
    }

    /* Walk all uniforms. */
    for (i = 0; i < shader->uniformCount; i++)
    {
        /* Get the gcUNIFORM pointer. */
        gcUNIFORM uniform = shader->uniforms[i];

        if (uniform != gcvNULL)
        {
            offset = 0;

            if (isUniformImage(uniform))
            {
                /* Uniform header and type. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "  uniform(%d) := %s %s %s %s ",
                                       i,
                                       GetImageFormatQualifierName_(GetUniformImageFormat(uniform)),
                                       GetCategoryName_(GetUniformCategory(uniform)),
                                       GetPrecisionName_(uniform->precision),
                                       gcmType_Name(uniform->u.type)));
            }
            else
            {
                /* Uniform header and type. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "  uniform(%d) := %s %s %s ",
                                       i,
                                       GetCategoryName_(GetUniformCategory(uniform)),
                                       GetPrecisionName_(uniform->precision),
                                       gcmType_Name(uniform->u.type)));
            }

            /* Append name. */
            offset += gcSL_GetName(uniform->nameLength,
                                uniform->name,
                                buffer + offset,
                                gcmSIZEOF(buffer) - offset);

            if (uniform->arrayLengthCount > 0)
            {
                gctINT i;
                for (i = 0; i < uniform->arrayLengthCount; i++)
                {
                    /* Append array size. */
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           "[%d]", uniform->arrayLengthList[i]));
                }
            }

            if (uniform->blockIndex != -1)
            {
                /* Append block index. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " :blockindex %d", uniform->blockIndex));
            }

            if (uniform->offset != -1)
            {
                /* Append offset. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " :offset %d", uniform->offset));
            }

            if (uniform->physical != -1)
            {
                /* Append physical. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " :physical %d :swizzle %d", uniform->physical, uniform->swizzle));
            }

            if(gcmType_Kind(uniform->u.type) == gceTK_ATOMIC)
            {
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                    " :binding %s(%d)",
                    uniform->baseBindingIdx >= 0 ? shader->uniforms[uniform->baseBindingIdx]->name : "#undef", shader->uniforms[uniform->baseBindingIdx]->binding));
            }

            /* Dump the location. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   " :location %d",
                                   GetUniformLayoutLocation(uniform)));

            /* dump the flags */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " flags:"));
            if(isUniformInactive(uniform))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " Inactive"));
            }
            if(isUniformUsedInShader(uniform))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " Used_In_Shader"));
            }
            if(isUniformStaticallyUsed(uniform))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " Statically_Used"));
            }

            /* Dump the uniform. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, ";\n"));
            gcOpt_DumpBuffer(os, File, buffer, offset);
        }
    }

    /* Walk all outputs. */
    for (i = 0; i < shader->outputCount; i++)
    {
        /* Get the gcOUTPUT pointer. */
        gcOUTPUT output = shader->outputs[i];

        if (output != gcvNULL)
        {
            gctUINT32 rows, components;
            gctCONST_STRING typeName;

            gcTYPE_GetTypeInfo(output->type, &components, &rows, &typeName);
            offset = 0;

            /* Output header and type. */
            if (output->arraySize > 1)
            {
                gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "  output(%d) := %s%s%s %s %s[%d] ",
                                   i,
                                   gcmOUTPUT_isCentroid(output) ? "centroid " : gcmOUTPUT_isSample(output) ? "sample " : "",
                                   gcmOUTPUT_isPerPatch(output) ? "patch " : gcmOUTPUT_isPerVertexArray(output) ? "PerVertexArray " : "",
                                   (output->shaderMode == gcSHADER_SHADER_FLAT) ? "flat" : "smooth",
                                   GetPrecisionName_(output->precision),
                                   typeName, output->arraySize));
            }
            else
            {
                gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "  output(%d) := %s%s%s %s ",
                                   i,
                                   gcmOUTPUT_isCentroid(output) ? "centroid " : gcmOUTPUT_isSample(output) ? "sample " : "",
                                   gcmOUTPUT_isPerPatch(output) ? "patch " : gcmOUTPUT_isPerVertexArray(output) ? "PerVertexArray " : "",
                                   (output->shaderMode == gcSHADER_SHADER_FLAT) ? "flat" : "smooth",
                                   typeName));
            }

            /* Append name. */
            offset += gcSL_GetName(output->nameLength,
                                output->name,
                                buffer + offset, gcmSIZEOF(buffer) - offset);

            if (output->arraySize > 1)
            {
                /* Append array size. */
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "[%d]", output->arrayIndex));
            }

            /* dump the layout location*/
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " (location = %d)", output->location));

            /* dump the field index*/
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " (field index = %d)", output->fieldIndex));

            /* Append assigned temporary register. */
            if (rows > 1)
            {
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " ==> temp(%d - %d)", output->tempIndex,
                                       output->tempIndex + rows -1));
            }
            else
            {
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " ==> temp(%d)", output->tempIndex));
            }

            /* dump the flags */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " flags:"));
            if(gcmOUTPUT_isStaticallyUsed(output))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       " Statically_Used"));
            }

            /* Dump the output. */
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, ";\n"));
            gcOpt_DumpBuffer(os, File, buffer, offset);
        }
    }

    if (dumpOptimizer)
    {
        codeCount = optimizer->codeTail->id + 1;
        codeNode = optimizer->codeHead;
        code = &codeNode->instruction;
    }
    else
    {
        codeCount = shader->codeCount;
        code = shader->code;
    }
    /* ignore NOPs at end of shader */
    if(dumpOptimizer)
    {
        gcOPT_CODE tail = optimizer->codeTail;
        while (codeCount > 0 &&
            gcmSL_OPCODE_GET(tail->instruction.opcode, Opcode) == gcSL_NOP)
        {
            codeCount--;
            tail = tail->prev;
        }
    }
    else
    {
        while (codeCount > 0 &&
            gcmSL_OPCODE_GET(shader->code[codeCount - 1].opcode, Opcode) == gcSL_NOP)
        {
            codeCount--;
        }
    }
    if (codeCount != shader->codeCount)
    {
        /* add one NOP at end of shader dump */
        codeCount++;
    }
    /* Walk all code. */
    for (i = 0; i < codeCount; i++)
    {
        _DumpIR(os, File, i, code);

        /* Get next code. */
        if (dumpOptimizer)
        {
            codeNode = codeNode->next;
            if (codeNode == gcvNULL)
            {
                break;
            }
            code = &codeNode->instruction;
        }
        else
        {
            code++;
        }
    }

    /* Loadtime Optimization related data */
    if (shader->ltcUniformCount > 0)
    {

        offset = 0;
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               " ltcUniformCount (%d), ltcUniformBegin (%d) ",
                               shader->ltcUniformCount, shader->ltcUniformBegin));
        gcOpt_DumpBuffer(os, File, buffer, offset);

        for (i = 0; i < (gctUINT)shader->ltcInstructionCount; i++)
        {
            if (shader->ltcCodeUniformIndex[i] == -1 ) continue;
            offset = 0;
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   " \t ltcCodeUniformIndex[%d] = %d ",
                                   i, shader->ltcCodeUniformIndex[i]));
            gcOpt_DumpBuffer(os, File, buffer, offset);
        }


        /* LTC expressions */
        offset = 0;
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               " ltcInstructionCount (%d)",
                               shader->ltcInstructionCount));
        gcOpt_DumpBuffer(os, File, buffer, offset);

        /* Walk all code. */
        gcOpt_DumpBuffer(os, File, buffer, offset);
        code = shader->ltcExpressions;
        for (i = 0; i < shader->ltcInstructionCount; i++)
        {
            _DumpIR(os, File, i, code);

            code++;
        }
    }

    if (PrintHeaderFooter == gcvTRUE)
    {
        offset = 0;
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "%s", doubleDashLine));
        gcOpt_DumpBuffer(os, File, buffer, offset);

        if (File != gcvNULL)
        {
            gcoOS_Flush(os, File);
        }
    }

    gcmFOOTER_NO();
}

/*******************************************************************************
**                                   gcOpt_Dump
********************************************************************************
**
**  Print the shader, flow graph, and dependency tree for debugging.
**
**  INPUT:
**
**      gctCONST_STRING Text
**          Pointer to text to print before dumping shader, etc.
**
**      gcOPTIMIZER Optimizer
**          Pointer to the gcOPTIMIZER structure to print.
**
**      gcSHADER Shader
**          Pointer to a gcSHADER object holding information about the compiled shader.
**          Shader is used only when Optimizer is gcvNULL.
*/
void
gcOpt_Dump(
    IN gctFILE File,
    IN gctCONST_STRING Text,
    IN gcOPTIMIZER Optimizer,
    IN gcSHADER Shader
    )
{
    gctSIZE_T i;
    char buffer[256];
    gctUINT offset = 0;
    gcoOS os = gcvNULL;
    /*gcSHADER shader = Shader;*/
    gctBOOL dumpOptimizer = gcvFALSE;

    static gctCONST_STRING doubleDashLine = "===============================================================================\n";
    static gctCONST_STRING dashLine = "*******************************************************************************\n";

    gcmHEADER_ARG("File=%p Text=0x%x Optimizer=%p Shader=%p",
                    File, Text, Optimizer, Shader);
    gcmTRACE(gcvLEVEL_VERBOSE, "Text=%s", Text);

    gcmASSERT(Optimizer != gcvNULL || Shader != gcvNULL);


    if (Optimizer != gcvNULL)
    {
        dumpOptimizer = gcvTRUE;
        /*shader = Optimizer->shader;*/
    }

    /* Print header. */
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                           "%s", doubleDashLine));
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                           "%s\n", Text));
    gcOpt_DumpBuffer(os, File, buffer, offset);

    /***************************************************************************
    ** I: Dump shader.
    */
    gcDump_Shader(File, Text, Optimizer, Shader, gcvFALSE);


    if (dumpOptimizer)
    {
        /***************************************************************************
        ** II: Dump data flow.
        */

        /* Print header. */
        offset = 0;
        gcmVERIFY_OK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                        "\n%s\n[DATA FLOW]\n", dashLine));
        gcOpt_DumpBuffer(os, File, buffer, offset);

        /* Process functions first. */
        for (i = 0; i < Optimizer->functionCount; i++)
        {
            gcOPT_FUNCTION function = Optimizer->functionArray + i;
            gctUINT   codeStart, codeEnd;
            gctUINT   nameLength;
            char * name;
            offset = 0;
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset, "\n  "));

            if (function->shaderFunction)
            {
                gcFUNCTION shaderFunction = function->shaderFunction;
                nameLength = shaderFunction->nameLength;
                name       = shaderFunction->name;
            }
            else
            {
                gcKERNEL_FUNCTION kernelFunction = function->kernelFunction;
                nameLength = kernelFunction->nameLength;
                name       = kernelFunction->name;
            }
            offset += gcSL_GetName(nameLength,
                                name,
                                buffer + offset, gcmSIZEOF(buffer) - offset);

            if (function->codeHead != gcvNULL)
            {
                codeStart = function->codeHead->id;
                codeEnd   = function->codeTail->id;
            }
            else
            {
                codeStart = codeEnd = 0;
            }
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "() : [%u - %u]\n",
                                   codeStart, codeEnd));
            gcOpt_DumpBuffer(os, File, buffer, offset);

            _DumpDataFlow(os, File, function);
        }

        /* Process main program. */
        offset = 0;
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "\n  main() : [%u - %u]\n",
                               Optimizer->main->codeHead->id,
                               Optimizer->main->codeTail->id));
        gcOpt_DumpBuffer(os, File, buffer, offset);

        _DumpDataFlow(os, File, Optimizer->main);
    }

    offset = 0;
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                           "%s", doubleDashLine));
    gcOpt_DumpBuffer(os, File, buffer, offset);

    if (File != gcvNULL)
    {
        gcoOS_Flush(os, File);
    }

    gcmFOOTER_NO();
}
#endif


