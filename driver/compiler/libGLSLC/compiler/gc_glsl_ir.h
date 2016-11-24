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


#ifndef __gc_glsl_ir_h_
#define __gc_glsl_ir_h_

#include "gc_glsl_compiler.h"
#include "debug/gc_vsc_debug.h"

#define sldMAX_VECTOR_COMPONENT  4  /*maximum number of components in a vector */
#define sldMAX_MATRIX_SIZE  4  /*maximum size of a matrix*/

typedef gctUINT           gctLABEL;

#define slmF2B(f)        (((f) != (gctFLOAT)0.0)? gcvTRUE : gcvFALSE)
#define slmF2I(f)        ((gctINT32)(f))
#define slmF2U(f)        ((gctUINT32)(f))

#define slmI2F(i)        ((gctFLOAT)(i))
#define slmI2B(i)        (((i) != (gctINT32)0)? gcvTRUE : gcvFALSE)
#define slmI2U(i)        ((gctUINT32)(i))

#define slmU2I(i)        ((gctINT32)(i))
#define slmU2F(i)        ((gctFLOAT)(i))
#define slmU2B(i)        (((i) != (gctUINT32)0)? gcvTRUE : gcvFALSE)

#define slmB2F(b)        ((b)? (gctFLOAT)1.0 : (gctFLOAT)0.0)
#define slmB2I(b)        ((b)? (gctINT32)1 : (gctINT32)0)
#define slmB2U(b)        ((b)? (gctUINT32)1 : (gctUINT32)0)

struct _slsNAME;
struct _slsLexToken;
struct _sloIR_POLYNARY_EXPR;

typedef gceSTATUS (*slsBuiltInFuncCheck)(
    IN sloCOMPILER Compiler,
    IN struct _slsNAME * FuncName,
    IN struct _sloIR_POLYNARY_EXPR * PolynaryExpr
    );

typedef gceSTATUS (*slsBuiltInEvaluateFunc)(void *);
typedef gceSTATUS (*slsBuiltInGenCodeFunc)(void *);

/* Data type */
typedef enum _sleSTORAGE_QUALIFIER
{
    slvSTORAGE_QUALIFIER_NONE    = 0,

    slvSTORAGE_QUALIFIER_CONST,                     /* 0x1 */

    slvSTORAGE_QUALIFIER_UNIFORM,                   /* 0x2 */
    slvSTORAGE_QUALIFIER_ATTRIBUTE,                 /* 0x3. Vertex only */
    slvSTORAGE_QUALIFIER_PERPATCH_ATTRIBUTE,        /* 0x4. TCS/TES only */
    slvSTORAGE_QUALIFIER_PERPATCH_OUTPUT,           /* 0x5. TCS/TES only */

    slvSTORAGE_QUALIFIER_VARYING_OUT,               /* 0x6. Vertex only */
    slvSTORAGE_QUALIFIER_VARYING_IN,                /* 0x7. Fragment only */
    slvSTORAGE_QUALIFIER_FRAGMENT_OUT,              /* 0x8 */

    slvSTORAGE_QUALIFIER_CONST_IN,                  /* 0x9 */
    slvSTORAGE_QUALIFIER_IN,                        /* 0xa */
    slvSTORAGE_QUALIFIER_OUT,                       /* 0xb */
    slvSTORAGE_QUALIFIER_INOUT,                     /* 0xc */
    slvSTORAGE_QUALIFIER_UNIFORM_BLOCK_MEMBER,      /* 0xd */
    slvSTORAGE_QUALIFIER_INSTANCE_ID,               /* 0xe */
    slvSTORAGE_QUALIFIER_VERTEX_ID,                 /* 0xf */
    slvSTORAGE_QUALIFIER_BUFFER,                    /* 0x10 */
    slvSTORAGE_QUALIFIER_STORAGE_BLOCK_MEMBER,      /* 0x11 */
    slvSTORAGE_QUALIFIER_SHARED,                    /* 0x12 */
    slvSTORAGE_QUALIFIER_VARYING_PATCH_OUT,         /* 0x13 */
    slvSTORAGE_QUALIFIER_VARYING_PATCH_IN,          /* 0x14 */
    slvSTORAGE_QUALIFIER_IN_IO_BLOCK,               /* 0x15 */
    slvSTORAGE_QUALIFIER_OUT_IO_BLOCK,              /* 0x16 */
    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,        /* 0x17 */
    slvSTORAGE_QUALIFIER_OUT_IO_BLOCK_MEMBER,       /* 0x18 */
} sleSTORAGE_QUALIFIER;

typedef gctUINT8 sltSTORAGE_QUALIFIER;

gctCONST_STRING
slGetStorageQualifierName(
    IN sloCOMPILER Compiler,
    IN sltSTORAGE_QUALIFIER Qualifier
    );

typedef enum _slePRECISION_QUALIFIER
{
    slvPRECISION_QUALIFIER_DEFAULT    = 0,    /* Must be zero */

    slvPRECISION_QUALIFIER_LOW,
    slvPRECISION_QUALIFIER_MEDIUM,
    slvPRECISION_QUALIFIER_HIGH,
    slvPRECISION_QUALIFIER_ANY,
} slePRECISION_QUALIFIER;

typedef gctUINT8 sltPRECISION_QUALIFIER;

typedef struct _slsDEFAULT_PRECISION
{
    /*
       Initially, they are set to slvPRECISION_QUALIFIER_DEFAULT, and then if
       'precision' statement is called, it will set to exact precision
       value
    */

    slePRECISION_QUALIFIER  floatPrecision;
    slePRECISION_QUALIFIER  intPrecision;
    slePRECISION_QUALIFIER  sampler2DPrecision;
    slePRECISION_QUALIFIER  samplerCubePrecision;
    slePRECISION_QUALIFIER  samplerExternalOesPrecision;
}slsDEFAULT_PRECISION;

typedef enum _sleMEMORY_ACCESS_QUALIFIER
{
    slvMEMORY_ACCESS_QUALIFIER_NONE           = 0,
    slvMEMORY_ACCESS_QUALIFIER_COHERENT       = 0x1,
    slvMEMORY_ACCESS_QUALIFIER_VOLATILE       = 0x2,
    slvMEMORY_ACCESS_QUALIFIER_RESTRICT       = 0x4,
    slvMEMORY_ACCESS_QUALIFIER_READONLY       = 0x8,
    slvMEMORY_ACCESS_QUALIFIER_WRITEONLY      = 0x10,
} sleMEMORY_ACCESS_QUALIFIER;

typedef gctUINT8 sltMEMORY_ACCESS_QUALIFIER;

enum _sleELEMENT_TYPE
{
    slvTYPE_VOID            = 0,

    slvTYPE_BOOL,
    slvTYPE_INT,
    slvTYPE_UINT,
    slvTYPE_FLOAT,

    slvTYPE_SAMPLER2D,
    slvTYPE_SAMPLERCUBE,

    slvTYPE_STRUCT,

    slvTYPE_SAMPLER3D,
    slvTYPE_SAMPLER1DARRAY,
    slvTYPE_SAMPLER2DARRAY,
    slvTYPE_SAMPLERBUFFER,
    slvTYPE_SAMPLER1DARRAYSHADOW,
    slvTYPE_SAMPLER2DARRAYSHADOW,

    slvTYPE_SAMPLER2DSHADOW,
    slvTYPE_SAMPLERCUBESHADOW,
    slvTYPE_SAMPLERCUBEARRAYSHADOW,
    slvTYPE_SAMPLERCUBEARRAY,

    slvTYPE_ISAMPLER2D,
    slvTYPE_ISAMPLERCUBE,
    slvTYPE_ISAMPLERCUBEARRAY,
    slvTYPE_ISAMPLER3D,
    slvTYPE_ISAMPLER2DARRAY,
    slvTYPE_ISAMPLERBUFFER,

    slvTYPE_USAMPLER2D,
    slvTYPE_USAMPLERCUBE,
    slvTYPE_USAMPLERCUBEARRAY,
    slvTYPE_USAMPLER3D,
    slvTYPE_USAMPLER2DARRAY,
    slvTYPE_USAMPLERBUFFER,

    slvTYPE_SAMPLER2DMS,
    slvTYPE_ISAMPLER2DMS,
    slvTYPE_USAMPLER2DMS,
    slvTYPE_SAMPLER2DMSARRAY,
    slvTYPE_ISAMPLER2DMSARRAY,
    slvTYPE_USAMPLER2DMSARRAY,

    slvTYPE_SAMPLEREXTERNALOES,

    slvTYPE_IMAGE2D,
    slvTYPE_IIMAGE2D,
    slvTYPE_UIMAGE2D,
    slvTYPE_IMAGEBUFFER,
    slvTYPE_IIMAGEBUFFER,
    slvTYPE_UIMAGEBUFFER,
    slvTYPE_IMAGE2DARRAY,
    slvTYPE_IIMAGE2DARRAY,
    slvTYPE_UIMAGE2DARRAY,
    slvTYPE_IMAGE3D,
    slvTYPE_IIMAGE3D,
    slvTYPE_UIMAGE3D,
    slvTYPE_IMAGECUBE,
    slvTYPE_IMAGECUBEARRAY,
    slvTYPE_IIMAGECUBE,
    slvTYPE_IIMAGECUBEARRAY,
    slvTYPE_UIMAGECUBE,
    slvTYPE_UIMAGECUBEARRAY,

    slvTYPE_UNIFORM_BLOCK,
    slvTYPE_STORAGE_BLOCK,
    slvTYPE_GEN_SAMPLER,
    slvTYPE_GEN_ISAMPLER,
    slvTYPE_GEN_USAMPLER,
    slvTYPE_ATOMIC_UINT,
    slvTYPE_IO_BLOCK,
    slvTYPE_TOTAL_COUNT                /*total count of types. No new type to be defined beyond here*/
};

#define sldFirstGenSamplerType slvTYPE_GEN_SAMPLER
#define sldLastGenSamplerType slvTYPE_GEN_USAMPLER

typedef gctUINT8    sltELEMENT_TYPE;

struct _slsNAME_SPACE;

struct _slsMATRIX_SIZE;
typedef struct _slsMATRIX_SIZE
{
  gctUINT8  rowCount;      /* vector if rowCount >0 and
                                     columnCount = 0 */
  gctUINT8  columnCount;  /* 0 means not matrix, column dimension */
} slsMATRIX_SIZE;

typedef enum _sleLAYOUT_ID
{
    slvLAYOUT_NONE                                 = 0x0,
    slvLAYOUT_BLEND_SUPPORT_MULTIPLY               = 0x1 ,
    slvLAYOUT_BLEND_SUPPORT_OVERLAY                = 0x2 ,
    slvLAYOUT_BLEND_SUPPORT_DARKEN                 = 0x4 ,
    slvLAYOUT_BLEND_SUPPORT_LIGHTEN                = 0x8 ,
    slvLAYOUT_BLEND_SUPPORT_COLORDODGE             = 0x10 ,
    slvLAYOUT_BLEND_SUPPORT_COLORBURN              = 0x20 ,
    slvLAYOUT_BLEND_SUPPORT_HARDLIGHT              = 0x40 ,
    slvLAYOUT_BLEND_SUPPORT_SOFTLIGHT              = 0x80 ,
    slvLAYOUT_BLEND_SUPPORT_DIFFERENCE             = 0x100 ,
    slvLAYOUT_BLEND_SUPPORT_EXCLUSION              = 0x200 ,
    slvLAYOUT_BLEND_SUPPORT_HSL_HUE                = 0x400 ,
    slvLAYOUT_BLEND_SUPPORT_HSL_SATURATION         = 0x800 ,
    slvLAYOUT_BLEND_SUPPORT_HSL_COLOR              = 0x1000 ,
    slvLAYOUT_BLEND_SUPPORT_HSL_LUMINOSITY         = 0x2000 ,
    slvLAYOUT_BLEND_SUPPORT_SCREEN                 = 0x4000 ,
    slvLAYOUT_PACKED                               = 0x8000,
    slvLAYOUT_SHARED                               = 0x10000,
    slvLAYOUT_STD140                               = 0x20000,
    slvLAYOUT_ROW_MAJOR                            = 0x40000,
    slvLAYOUT_COLUMN_MAJOR                         = 0x80000,
    slvLAYOUT_LOCATION                             = 0x100000,
    slvLAYOUT_WORK_GROUP_SIZE_X                    = 0x200000,
    slvLAYOUT_WORK_GROUP_SIZE_Y                    = 0x400000,
    slvLAYOUT_WORK_GROUP_SIZE_Z                    = 0x800000,
    slvLAYOUT_STD430                               = 0x1000000,
    slvLAYOUT_BINDING                              = 0x2000000,
    slvLAYOUT_OFFSET                               = 0x4000000,
    slvLAYOUT_IMAGE_FORMAT                         = 0x8000000,
    slvLAYOUT_EARLY_FRAGMENT_TESTS                 = 0x10000000,

} sleLAYOUT_ID;

typedef enum _sleLAYOUT_ID_EXT
{
    /* TS layout. */
    slvLAYOUT_EXT_NONE                             = 0x0,
    slvLAYOUT_EXT_TS_TRIANGLES                     = 0x1,
    slvLAYOUT_EXT_TS_QUADS                         = 0x2,
    slvLAYOUT_EXT_TS_ISOLINES                      = 0x4,
    slvLAYOUT_EXT_TS_PRIMITIVE_MODE                = slvLAYOUT_EXT_TS_TRIANGLES |
                                                     slvLAYOUT_EXT_TS_QUADS |
                                                     slvLAYOUT_EXT_TS_ISOLINES,
    slvLAYOUT_EXT_EQUAL_SPACING                    = 0x8,
    slvLAYOUT_EXT_FRACTIONAL_EVEN_SPACING          = 0x10,
    slvLAYOUT_EXT_FRACTIONAL_ODD_SPACING           = 0x20,
    slvlAYOUT_EXT_VERTEX_SPACING                   = slvLAYOUT_EXT_EQUAL_SPACING |
                                                     slvLAYOUT_EXT_FRACTIONAL_EVEN_SPACING |
                                                     slvLAYOUT_EXT_FRACTIONAL_ODD_SPACING,
    slvLAYOUT_EXT_CW                               = 0x40,
    slvLAYOUT_EXT_CCW                              = 0x80,
    slvlAYOUT_EXT_ORERING                          = slvLAYOUT_EXT_CW |
                                                     slvLAYOUT_EXT_CCW,
    slvLAYOUT_EXT_POINT_MODE                       = 0x100,
    slvLAYOUT_EXT_LINES_MODE                       = 0x100,
    slvLAYOUT_EXT_TRIANGLES_MODE                   = 0x100,
    slvLAYOUT_EXT_POINTMODE                        = slvLAYOUT_EXT_POINT_MODE |
                                                     slvLAYOUT_EXT_LINES_MODE |
                                                     slvLAYOUT_EXT_TRIANGLES_MODE,

    slvLAYOUT_EXT_VERTICES                         = 0x200,
    /* GS layout. */
    slvLAYOUT_EXT_GS_POINTS                        = 0x400,
    slvLAYOUT_EXT_GS_LINES                         = 0x800,
    slvLAYOUT_EXT_GS_LINES_ADJACENCY               = 0x1000,
    slvLAYOUT_EXT_GS_TRIANGLES                     = 0x2000,
    slvLAYOUT_EXT_GS_TRIANGLES_ADJACENCY           = 0x4000,
    slvLAYOUT_EXT_GS_LINE_STRIP                    = 0x8000,
    slvLAYOUT_EXT_GS_TRIANGLE_STRIP                = 0x10000,
    slvLAYOUT_EXT_GS_IN_PRIMITIVE                  = slvLAYOUT_EXT_GS_LINES |
                                                     slvLAYOUT_EXT_GS_LINES_ADJACENCY |
                                                     slvLAYOUT_EXT_GS_TRIANGLES |
                                                     slvLAYOUT_EXT_GS_TRIANGLES_ADJACENCY,
    slvLAYOUT_EXT_GS_OUT_PRIMITIVE                 = slvLAYOUT_EXT_GS_LINE_STRIP |
                                                     slvLAYOUT_EXT_GS_TRIANGLE_STRIP,
    slvLAYOUT_EXT_GS_PRIMITIVE                     = slvLAYOUT_EXT_GS_IN_PRIMITIVE |
                                                     slvLAYOUT_EXT_GS_OUT_PRIMITIVE |
                                                     slvLAYOUT_EXT_GS_POINTS,
    slvLAYOUT_EXT_MAX_VERTICES                     = 0x20000,
    slvLAYOUT_EXT_INVOCATIONS                      = 0x40000,
} sleLAYOUT_ID_EXT;

typedef enum _sleIMAGE_FORMAT
{
    slvLAYOUT_IMAGE_FORMAT_DEFAULT   = 0,
    slvLAYOUT_IMAGE_FORMAT_RGBA32F,
    slvLAYOUT_IMAGE_FORMAT_RGBA16F,
    slvLAYOUT_IMAGE_FORMAT_R32F,
    slvLAYOUT_IMAGE_FORMAT_RGBA8,
    slvLAYOUT_IMAGE_FORMAT_RGBA8_SNORM,
    slvLAYOUT_IMAGE_FORMAT_RGBA32I,
    slvLAYOUT_IMAGE_FORMAT_RGBA16I,
    slvLAYOUT_IMAGE_FORMAT_RGBA8I,
    slvLAYOUT_IMAGE_FORMAT_R32I,
    slvLAYOUT_IMAGE_FORMAT_RGBA32UI,
    slvLAYOUT_IMAGE_FORMAT_RGBA16UI,
    slvLAYOUT_IMAGE_FORMAT_RGBA8UI,
    slvLAYOUT_IMAGE_FORMAT_R32UI,

} sleIMAGE_FORMAT;

#define sldLAYOUT_MEMORY_BIT_FIELDS      (slvLAYOUT_PACKED | slvLAYOUT_SHARED | slvLAYOUT_STD140 | slvLAYOUT_STD430)

#define sldLAYOUT_MATRIX_BIT_FIELDS      (slvLAYOUT_ROW_MAJOR | slvLAYOUT_COLUMN_MAJOR)

#define sldLAYOUT_WORK_GROUP_SIZE_FIELDS (slvLAYOUT_WORK_GROUP_SIZE_X | slvLAYOUT_WORK_GROUP_SIZE_Y | slvLAYOUT_WORK_GROUP_SIZE_Z)

#define sldLAYOUT_BLEND_SUPPORT_BIT_FIELDS  (slvLAYOUT_BLEND_SUPPORT_MULTIPLY| \
                                             slvLAYOUT_BLEND_SUPPORT_SCREEN| \
                                             slvLAYOUT_BLEND_SUPPORT_OVERLAY| \
                                             slvLAYOUT_BLEND_SUPPORT_DARKEN| \
                                             slvLAYOUT_BLEND_SUPPORT_LIGHTEN| \
                                             slvLAYOUT_BLEND_SUPPORT_COLORDODGE| \
                                             slvLAYOUT_BLEND_SUPPORT_COLORBURN| \
                                             slvLAYOUT_BLEND_SUPPORT_HARDLIGHT| \
                                             slvLAYOUT_BLEND_SUPPORT_SOFTLIGHT| \
                                             slvLAYOUT_BLEND_SUPPORT_DIFFERENCE| \
                                             slvLAYOUT_BLEND_SUPPORT_EXCLUSION| \
                                             slvLAYOUT_BLEND_SUPPORT_HSL_HUE| \
                                             slvLAYOUT_BLEND_SUPPORT_HSL_SATURATION| \
                                             slvLAYOUT_BLEND_SUPPORT_HSL_COLOR| \
                                             slvLAYOUT_BLEND_SUPPORT_HSL_LUMINOSITY)

typedef enum _sleTES_PRIMITIVE_MODE
{
    slvTES_PRIMITIVE_MODE_NONE                          = -1,
    slvTES_PRIMITIVE_MODE_TRIANGLES,
    slvTES_PRIMITIVE_MODE_QUADS,
    slvTES_PRIMITIVE_MODE_ISOLINES,
} slvTES_PRIMITIVE_MODE;

typedef enum _sleTES_VERTEX_SPACING
{
    slvTES_VERTEX_SPACING_NONE                          = -1,
    slvTES_VERTEX_SPACING_EQUAL_SPACING,
    slvTES_VERTEX_SPACING_FRACTIONAL_EVEN_SPACING,
    slvTES_VERTEX_SPACING_FRACTIONAL_ODD_SPACING,
} slvTES_VERTEX_SPACING;

typedef enum _sleTES_ORDERING
{
    slvTES_ORDERING_NONE                                = -1,
    slvTES_ORDERING_CCW,
    slvTES_ORDERING_CW,
} slvTES_ORDERING;

typedef enum _sleTES_POINT_MODE
{
    slvTES_POINT_MODE_NONE                              = -1,
    slvTES_POINT_MODE_POINT_MODE,
} slvTES_POINT_MODE;

typedef enum _sleGS_PRIMITIVE
{
    slvGS_PRIMITIVE_NONE                                = -1,
    slvGS_POINTS,
    slvGS_LINES,
    slvGS_LINES_ADJACENCY,
    slvGS_TRIANGLES,
    slvGS_TRIANGLES_ADJACENCY,
    slvGS_LINE_STRIP,
    slvGS_TRIANGLE_STRIP,
} slvGS_PRIMITIVE;

typedef struct _slsLAYOUT_QUALIFIER
{
    gctINT                          location;    /* location of interface variable */
    gctUINT                         workGroupSize[3];    /* local workgroup size */
    gctUINT                         binding;    /* binding for interface block */
    gctUINT                         offset;
    gctINT16                        imageFormat;
    slvTES_PRIMITIVE_MODE           tesPrimitiveMode;
    slvTES_VERTEX_SPACING           tesVertexSpacing;
    slvTES_ORDERING                 tesOrdering;
    slvTES_POINT_MODE               tesPointMode;
    slvGS_PRIMITIVE                 gsPrimitive;
    gctINT                          gsInvocationTime;
    gctINT                          maxGSVerticesNumber; /* This is for GS. */
    gctINT                          maxVerticesNumber; /* This is queried from HW. */
    gctINT                          verticesNumber; /* This is setted by layout. */
    gcUNIFORM                       tcsInputVerticesUniform;
    sleLAYOUT_ID                    id;    /* layout ids */
    sleLAYOUT_ID_EXT                ext_id; /* layout ids EXT*/
} slsLAYOUT_QUALIFIER;

#define slsLAYOUT_Initialize(Layout) \
    do \
    { \
        gcoOS_ZeroMemory(Layout, gcmSIZEOF(slsLAYOUT_QUALIFIER)); \
        (Layout)->tesVertexSpacing      = slvTES_VERTEX_SPACING_EQUAL_SPACING; \
        (Layout)->tesOrdering           = slvTES_ORDERING_CCW; \
        (Layout)->tesPointMode          = slvTES_POINT_MODE_NONE; \
        (Layout)->gsPrimitive           = slvGS_PRIMITIVE_NONE; \
        (Layout)->gsInvocationTime      = -1; \
        (Layout)->maxGSVerticesNumber   = -1; \
        (Layout)->verticesNumber        = -1; \
    } \
    while (gcvFALSE)

typedef enum _sleINTERPOLATION_QUALIFIER
{
    slvINTERPOLATION_QUALIFIER_DEFAULT,
    slvINTERPOLATION_QUALIFIER_FLAT = 1,
    slvINTERPOLATION_QUALIFIER_SMOOTH = 2,
} sleINTERPOLATION_QUALIFIER;

typedef gctUINT8 sltINTERPOLATION_QUALIFIER;

typedef enum _sleAUXILIARY_QUALIFIER
{
    slvAUXILIARY_QUALIFIER_DEFAULT      = 0,
    slvAUXILIARY_QUALIFIER_CENTER       = 0,
    slvAUXILIARY_QUALIFIER_CENTROID     = 1,
    slvAUXILIARY_QUALIFIER_SAMPLE       = 2,
} sleAUXILIARY_QUALIFIER;

typedef gctUINT8 sltAUXILIARY_QUALIFIER;

typedef enum _sleQUALIFIERS_FLAG
{
    slvQUALIFIERS_FLAG_NONE                             = 0x0,
    slvQUALIFIERS_FLAG_LAYOUT                           = 0x1,
    slvQUALIFIERS_FLAG_INTERPOLATION                    = 0x2,
    slvQUALIFIERS_FLAG_AUXILIARY                        = 0x4,
    slvQUALIFIERS_FLAG_PRECISION                        = 0x8,
    slvQUALIFIERS_FLAG_STORAGE                          = 0x10,
    slvQUALIFIERS_FLAG_MEMORY_ACCESS                    = 0x20,
    slvQUALIFIERS_FLAG_INVARIANT                        = 0x40,
    slvQUALIFIERS_FLAG_PATCH                            = 0x80,
    slvQUALIFIERS_FLAG_PRECISE                          = 0x100,
    slvQUALIFIERS_FLAG_KINDS                            = slvQUALIFIERS_FLAG_LAYOUT |
                                                          slvQUALIFIERS_FLAG_INTERPOLATION |
                                                          slvQUALIFIERS_FLAG_AUXILIARY |
                                                          slvQUALIFIERS_FLAG_PRECISION |
                                                          slvQUALIFIERS_FLAG_STORAGE |
                                                          slvQUALIFIERS_FLAG_MEMORY_ACCESS |
                                                          slvQUALIFIERS_FLAG_INVARIANT |
                                                          slvQUALIFIERS_FLAG_PRECISE,
    slvQUALIFIERS_FLAG_EXPECTING_CENTROID_OR_IN_OR_OUT  = 0x200,
    slvQUALIFIERS_FLAG_EXPECTING_IN_OR_OUT              = 0x400,
    slvQUALIFIERS_FLAG_EXPECTING_IN                     = 0x800,
    slvQUALIFIERS_FLAG_EXPECTING_OUT                    = 0x1000,
    slvQUALIFIERS_FLAG_ORDERS                           = slvQUALIFIERS_FLAG_EXPECTING_CENTROID_OR_IN_OR_OUT |
                                                          slvQUALIFIERS_FLAG_EXPECTING_IN_OR_OUT |
                                                          slvQUALIFIERS_FLAG_EXPECTING_IN |
                                                          slvQUALIFIERS_FLAG_EXPECTING_OUT,
    slvQUALIFIERS_FLAG_USE_AS_INTERPOLATE_FUNCTION      = 0x2000,
} sleQUALIFIERS_FLAG;

typedef gctUINT32 sltQUALIFIERS_FLAG;

typedef struct _slsQUALIFIERS
{
    slsLAYOUT_QUALIFIER         layout;
    sltINTERPOLATION_QUALIFIER  interpolation;
    sltAUXILIARY_QUALIFIER      auxiliary;
    sltPRECISION_QUALIFIER      precision;
    sltSTORAGE_QUALIFIER        storage;
    sltMEMORY_ACCESS_QUALIFIER  memoryAccess;
    sltQUALIFIERS_FLAG          flags;
} slsQUALIFIERS;

#define slsQUALIFIERS_HAS_FLAG(q, f)        ((q)->flags & (f))
#define slsQUALIFIERS_SET_FLAG(q, f)        ((q)->flags |= (f))
#define slsQUALIFIERS_RESET_FLAG(q, f)      ((q)->flags &= ~(f))
#define slsQUALIFIERS_GET_KINDS(q)          ((q)->flags & slvQUALIFIERS_FLAG_KINDS)
#define slsQUALIFIERS_KIND_IS(q, f)         (((q)->flags & slvQUALIFIERS_FLAG_KINDS) == (f))
#define slsQUALIFIERS_KIND_ISNOT(q, f)      (((q)->flags & slvQUALIFIERS_FLAG_KINDS) != (f))
#define slsQUALIFIERS_GET_ORDERS(q)         ((q)->flags & slvQUALIFIERS_FLAG_ORDERS)
#define slsQUALIFIERS_GET_AUXILIARY(q)      ((q)->auxiliary)
#define slsQUALIFIERS_SET_AUXILIARY(q, a)   ((q)->auxiliary = (a))

typedef enum _sleFUNC_FLAG
{
    slvFUNC_NONE                = 0x0000,       /* none */
    slvFUNC_ATOMIC              = 0x0001,       /* atomic functions. */
    slvFUNC_HAS_MEM_ACCESS      = 0x0002,       /* has memory access. */
    slvFUNC_HAS_VAR_ARG         = 0x0004,       /* has var argument. */
    slvFUNC_HAS_VOID_PARAM      = 0x0010,       /* has void param. */
    slvFUNC_IS_INTRINSIC        = 0x0020,       /* is an intrinsic function. */
    slvFUNC_DEFINED             = 0x0040,       /* is defined before. */
    slvFUNC_TREAT_FLOAT_AS_INT  = 0x0080,       /* treat float as int. */
} sleBUILT_IN_FUNC_FLAG;

typedef gctUINT8 sltFUNC_FLAG;

#define slsFUNC_HAS_FLAG(q, f)        ((q)->flags & (f))
#define slsFUNC_SET_FLAG(q, f)        ((q)->flags |= (f))
#define slsFUNC_RESET_FLAG(q, f)      ((q)->flags &= ~(f))

typedef enum _sleIB_FLAG
{
    slvIB_NONE                  = 0x0000,       /* none */
    slvIB_SHARED                = 0x0001,       /* block for shared variables. */
} sleIB_FLAG;

#define slsIB_HAS_FLAG(q, f)          ((q)->flags & (f))
#define slsIB_SET_FLAG(q, f)          ((q)->flags |= (f))
#define slsIB_RESET_FLAG(q, f)        ((q)->flags &= ~(f))

typedef struct _slsDATA_TYPE
{
    slsDLINK_NODE     node;

    gctINT            type;    /*lexical type*/

    slsQUALIFIERS     qualifiers;

    sltELEMENT_TYPE   elementType;

    slsMATRIX_SIZE    matrixSize;

    /*
    ** 0 means not array. If this is an arrays of arrays, the value means the current level array size.
    */
    gctINT            arrayLength;
    gctINT            arrayLengthCount;  /* arrayLengthCount > 1 means it is an array of arrays. */
    gctINT *          arrayLengthList; /* Array length list for an array of arrays. */

    /* The array level for this datatype, the left-most is 0, the right-most is arrayLengthCount - 1. */
    gctINT            arrayLevel;

    /* Whether this datatype is a element of a unsized array, it is only used by a SSBO member. */
    gctBOOL           isInheritFromUnsizedDataType;

    /* Whether this datatype is a per-vertex array variable. */
    gctINT            isPerVertexArray;

    /* Only for struct or interface block, use this to check if two structs are equal. */
    struct _slsNAME_SPACE *    orgFieldSpace;

    /* Only for struct or interface block */
    struct _slsNAME_SPACE *    fieldSpace;
}
slsDATA_TYPE;

gceSTATUS
slsDATA_TYPE_Construct(
    IN sloCOMPILER Compiler,
    IN gctINT TokenType,
    IN struct _slsNAME_SPACE * FieldSpace,
    OUT slsDATA_TYPE ** DataType
    );

gceSTATUS
slsDATA_TYPE_ConstructArray(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * ElementDataType,
    IN gctINT ArrayLength,
    OUT slsDATA_TYPE ** DataType
    );

gceSTATUS
slsDATA_TYPE_ConstructArraysOfArrays(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * ElementDataType,
    IN gctINT ArrayLengthCount,
    IN gctINT * ArrayLengthList,
    IN gctBOOL IsAppend,
    OUT slsDATA_TYPE ** DataType
    );

gceSTATUS
slsDATA_TYPE_ConstructElement(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * CompoundDataType,
    OUT slsDATA_TYPE ** DataType
    );

gceSTATUS
slsDATA_TYPE_Clone(
    IN sloCOMPILER Compiler,
    IN sltSTORAGE_QUALIFIER StorageQualifier,
    IN sltPRECISION_QUALIFIER PrecisionQualifier,
    IN slsDATA_TYPE * Source,
    OUT slsDATA_TYPE **    DataType
    );

gceSTATUS
slsDATA_TYPE_Destory(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType
    );

gceSTATUS
slsDATA_TYPE_Dump(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType
    );

gctBOOL
slsDATA_TYPE_IsEqual(
    IN slsDATA_TYPE * DataType1,
    IN slsDATA_TYPE * DataType2
    );

gctUINT
slsDATA_TYPE_GetSize(
    IN slsDATA_TYPE * DataType
    );

gctUINT
slsDATA_TYPE_GetFieldOffset(
    IN slsDATA_TYPE * StructDataType,
    IN struct _slsNAME * FieldName
    );

gctBOOL
slsDATA_TYPE_IsInitializableTo(
IN slsDATA_TYPE * LDataType,
IN slsDATA_TYPE * RDataType
);

gctBOOL
slsDATA_TYPE_IsAssignableAndComparable(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType
    );

gctBOOL
slsDATA_TYPE_IsArrayHasImplicitLength(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * DataType
    );

gcSHADER_TYPE
slsDATA_TYPE_ConvElementDataType(
    IN slsDATA_TYPE * DataType
    );

gctINT
slsDATA_TYPE_GetLogicalCountForAnArray(
    IN slsDATA_TYPE * DataType
    );

gctUINT
slsDATA_TYPE_GetLogicalOperandCount(
    IN slsDATA_TYPE * DataType,
    IN gctBOOL bCalcTypeSize
    );

#define slmPromoteIntToVector(IntValue, Vz, Res)  \
  do {  \
    int i; \
    for(i=0; i < (Vz); i++) { \
      (Res)[i].intValue = (IntValue); \
    } \
  } while (gcvFALSE)

#define sldLowestRankedInteger  slvTYPE_BOOL
#define sldHighestRankedInteger slvTYPE_UINT
#define sldHighestRankedFloat slvTYPE_FLOAT

#define sldArithmeticTypeCount (sldHighestRankedFloat - sldLowestRankedInteger + 1)

#define slmIsElementTypeArithmetic(EType) \
 ((EType) >= sldLowestRankedInteger && (EType) <= sldHighestRankedFloat)

#define slmIsElementTypeBoolean(EType) \
   ((EType) == slvTYPE_BOOL)

#define slmIsElementTypeInteger(EType) \
 ((EType) >= sldLowestRankedInteger && (EType) <= sldHighestRankedInteger)

#define slmIsElementTypeStrictInteger(EType) \
 ((EType) > sldLowestRankedInteger && (EType) <= sldHighestRankedInteger)

#define slmIsElementTypeUnsigned(EType) \
   ((EType) == slvTYPE_UINT)

#define slmIsElementTypeSigned(EType) \
   ((EType) == slvTYPE_INT)

#define slmIsElementTypeFloating(EType) \
 ((EType) == slvTYPE_FLOAT)

#define slmDATA_TYPE_IsIntegerType(d) \
 ((d)->arrayLength == 0  && \
  slmIsElementTypeInteger((d)->elementType))

#define slmDATA_TYPE_IsArithmeticType(d) \
 ((d)->arrayLength == 0  && \
  slmIsElementTypeArithmetic((d)->elementType))

#define slmIsElementTypeGenSamplerType(EType) \
  ((EType) >= sldFirstGenSamplerType && (EType) <= sldLastGenSamplerType)

#define slmDATA_TYPE_IsGenType(d) \
  slmIsElementTypeGenType((d)->elementType)

#define slmDATA_TYPE_vectorSize_GET(d) ((d)->matrixSize.columnCount? (gctUINT)0 : (d)->matrixSize.rowCount)
#define slmDATA_TYPE_vectorSize_NOCHECK_GET(d) ((d)->matrixSize.rowCount)
#define slmDATA_TYPE_vectorSize_SET(d, s) do \
        { (d)->matrixSize.rowCount = s; \
       (d)->matrixSize.columnCount = 0; \
        } \
        while (gcvFALSE)

#define slmDATA_TYPE_matrixSize_GET(d) ((d)->matrixSize.columnCount)
#define slmDATA_TYPE_matrixRowCount_GET(d) ((d)->matrixSize.rowCount)
#define slmDATA_TYPE_matrixColumnCount_GET(d) ((d)->matrixSize.columnCount)
#define slmDATA_TYPE_matrixSize_SET(d, r, c) do \
        { (d)->matrixSize.rowCount = (r); \
       (d)->matrixSize.columnCount = (c); \
        } \
        while (gcvFALSE)

#define slmDATA_TYPE_matrixRowCount_SET(d, c) (d)->matrixSize.rowCount = (c)
#define slmDATA_TYPE_matrixColumnCount_SET(d, c) (d)->matrixSize.columnCount = (c)

#define slmDATA_TYPE_layoutId_GET(d) ((d)->qualifiers.layout.id)
#define slmDATA_TYPE_layoutId_SET(d, i)  do \
    { (d)->qualifiers.layout.id |= (i); \
    } \
    while (gcvFALSE)

#define slmDATA_TYPE_layoutLocation_GET(d) ((d)->qualifiers.layout.location)
#define slmDATA_TYPE_layoutLocation_SET(d, l) do \
    { \
        (d)->qualifiers.layout.id |= slvLAYOUT_LOCATION; \
        (d)->qualifiers.layout.location = (l); \
    } \
    while (gcvFALSE)

#define slmDATA_TYPE_layoutBinding_GET(d) \
    (slmDATA_TYPE_layoutId_GET(d) & slvLAYOUT_BINDING \
        ? (d)->qualifiers.layout.binding \
        : 0)

#define slmDATA_TYPE_layoutBinding_SET(d, l) do \
    { \
        (d)->qualifiers.layout.id |= slvLAYOUT_BINDING; \
        (d)->qualifiers.layout.binding = (l); \
    } \
    while (gcvFALSE)

#define slmDATA_TYPE_layoutOffset_GET(d) \
    (slmDATA_TYPE_layoutId_GET(d) & slvLAYOUT_OFFSET \
        ? (d)->qualifiers.layout.offset \
        : 0)

#define slmDATA_TYPE_layoutOffset_SET(d, l) do \
    { \
        (d)->qualifiers.layout.id |= slvLAYOUT_OFFSET; \
        (d)->qualifiers.layout.offset = (l); \
    } \
    while (gcvFALSE)

#define slmDATA_TYPE_imageFormat_GET(d) \
    (slmDATA_TYPE_layoutId_GET(d) & slvLAYOUT_IMAGE_FORMAT \
        ? (d)->qualifiers.layout.imageFormat \
        : 0)

#define slmDATA_TYPE_imageFormat_SET(d, l) do \
    { \
        (d)->qualifiers.layout.id |= slvLAYOUT_IMAGE_FORMAT; \
        (d)->qualifiers.layout.imageFormat = (l); \
    } \
    while (gcvFALSE)

#define slmDATA_TYPE_HasLayoutQualifier(d) (slmDATA_TYPE_layoutId_GET(d))

#define slmDATA_TYPE_IsSquareMat(dataType) \
    (slmDATA_TYPE_matrixColumnCount_GET(dataType) != 0 && \
         (slmDATA_TYPE_matrixRowCount_GET(dataType) == slmDATA_TYPE_matrixColumnCount_GET(dataType)))

#define slsDATA_TYPE_IsVoid(dataType) \
    ((dataType)->elementType == slvTYPE_VOID)

#define slsDATA_TYPE_IsSampler(dataType) \
    ((dataType)->elementType >= slvTYPE_SAMPLER2D && (dataType)->elementType <= slvTYPE_UIMAGECUBEARRAY && \
        (dataType)->elementType != slvTYPE_STRUCT)

#define slsDATA_TYPE_IsImage(dataType) \
    ((dataType)->elementType >= slvTYPE_IMAGE2D && (dataType)->elementType <= slvTYPE_UIMAGECUBEARRAY && \
        (dataType)->elementType != slvTYPE_STRUCT)

#define slsDATA_TYPE_IsFloatImage(dataType) \
    ((dataType)->elementType == slvTYPE_IMAGE2D || (dataType)->elementType == slvTYPE_IMAGE2DARRAY || \
     (dataType)->elementType == slvTYPE_IMAGE3D || (dataType)->elementType == slvTYPE_IMAGECUBE || \
     (dataType)->elementType == slvTYPE_IMAGEBUFFER || (dataType)->elementType == slvTYPE_IMAGECUBEARRAY)

#define slsDATA_TYPE_IsIntImage(dataType) \
    ((dataType)->elementType == slvTYPE_IIMAGE2D || (dataType)->elementType == slvTYPE_IIMAGE2DARRAY || \
     (dataType)->elementType == slvTYPE_IIMAGE3D || (dataType)->elementType == slvTYPE_IIMAGECUBE || \
     (dataType)->elementType == slvTYPE_IIMAGEBUFFER || (dataType)->elementType == slvTYPE_IIMAGECUBEARRAY)

#define slsDATA_TYPE_IsUintImage(dataType) \
    ((dataType)->elementType == slvTYPE_UIMAGE2D || (dataType)->elementType == slvTYPE_UIMAGE2DARRAY || \
     (dataType)->elementType == slvTYPE_UIMAGE3D || (dataType)->elementType == slvTYPE_UIMAGECUBE || \
     (dataType)->elementType == slvTYPE_UIMAGEBUFFER || (dataType)->elementType == slvTYPE_UIMAGECUBEARRAY)

#define slsDATA_TYPE_NeedMemoryAccess(dataType) (slsDATA_TYPE_IsImage(dataType))

#define slsDATA_TYPE_IsSamplerShadow(dataType) \
    ((dataType)->elementType == slvTYPE_SAMPLER2DSHADOW || \
     (dataType)->elementType == slvTYPE_SAMPLERCUBESHADOW || \
     (dataType)->elementType == slvTYPE_SAMPLER2DARRAYSHADOW || \
     (dataType)->elementType == slvTYPE_SAMPLER1DARRAYSHADOW)

#define slsDATA_TYPE_IsSamplerMS(dataType) \
    ((dataType)->elementType == slvTYPE_SAMPLER2DMS || \
     (dataType)->elementType == slvTYPE_ISAMPLER2DMS || \
     (dataType)->elementType == slvTYPE_USAMPLER2DMS)

#define slsDATA_TYPE_IsSamplerMSARRAY(dataType) \
    ((dataType)->elementType == slvTYPE_SAMPLER2DMSARRAY || \
     (dataType)->elementType == slvTYPE_ISAMPLER2DMSARRAY || \
     (dataType)->elementType == slvTYPE_USAMPLER2DMSARRAY)

#define slsDATA_TYPE_IsFSampler(dataType) \
    ((dataType)->elementType == slvTYPE_SAMPLER2D || \
     (dataType)->elementType == slvTYPE_SAMPLER3D || \
     (dataType)->elementType == slvTYPE_SAMPLERCUBE || \
     (dataType)->elementType == slvTYPE_SAMPLERCUBEARRAY || \
     (dataType)->elementType == slvTYPE_SAMPLER1DARRAY || \
     (dataType)->elementType == slvTYPE_SAMPLER2DARRAY || \
     (dataType)->elementType == slvTYPE_SAMPLER1DARRAYSHADOW || \
     (dataType)->elementType == slvTYPE_SAMPLER2DARRAYSHADOW || \
     (dataType)->elementType == slvTYPE_SAMPLER2DSHADOW || \
     (dataType)->elementType == slvTYPE_SAMPLER2DMS || \
     (dataType)->elementType == slvTYPE_SAMPLER2DMSARRAY || \
     (dataType)->elementType == slvTYPE_SAMPLERBUFFER)

#define slsDATA_TYPE_IsISampler(dataType) \
    ((dataType)->elementType == slvTYPE_ISAMPLER2D || \
     (dataType)->elementType == slvTYPE_ISAMPLER3D || \
     (dataType)->elementType == slvTYPE_ISAMPLERCUBE || \
     (dataType)->elementType == slvTYPE_ISAMPLERCUBEARRAY || \
     (dataType)->elementType == slvTYPE_ISAMPLER2DARRAY || \
     (dataType)->elementType == slvTYPE_ISAMPLER2DMS || \
     (dataType)->elementType == slvTYPE_ISAMPLER2DMSARRAY || \
     (dataType)->elementType == slvTYPE_ISAMPLERBUFFER)

#define slsDATA_TYPE_IsUSampler(dataType) \
    ((dataType)->elementType == slvTYPE_USAMPLER2D || \
     (dataType)->elementType == slvTYPE_USAMPLER3D || \
     (dataType)->elementType == slvTYPE_USAMPLERCUBE || \
     (dataType)->elementType == slvTYPE_USAMPLERCUBEARRAY || \
     (dataType)->elementType == slvTYPE_USAMPLER2DARRAY || \
     (dataType)->elementType == slvTYPE_USAMPLER2DMS || \
     (dataType)->elementType == slvTYPE_USAMPLER2DMSARRAY || \
     (dataType)->elementType == slvTYPE_USAMPLERBUFFER)

#define slsDATA_TYPE_IsSamplerBuffer(dataType) \
    ((dataType)->elementType == slvTYPE_SAMPLERBUFFER || \
     (dataType)->elementType == slvTYPE_ISAMPLERBUFFER || \
     (dataType)->elementType == slvTYPE_USAMPLERBUFFER)

#define slsDATA_TYPE_IsImageBuffer(dataType) \
    ((dataType)->elementType == slvTYPE_IMAGEBUFFER || \
     (dataType)->elementType == slvTYPE_IIMAGEBUFFER || \
     (dataType)->elementType == slvTYPE_UIMAGEBUFFER)

#define slsDATA_TYPE_IsTextureBuffer(dataType) \
    (slsDATA_TYPE_IsSamplerBuffer(dataType) || slsDATA_TYPE_IsImageBuffer(dataType))

#define slsDATA_TYPE_IsBVecOrIVecOrVec(dataType) \
    ((dataType)->arrayLength == 0 && slmDATA_TYPE_vectorSize_GET(dataType) != 0)

#define slsDATA_TYPE_IsVectorType(dataType) \
        slsDATA_TYPE_IsBVecOrIVecOrVec(dataType)

#define slmDATA_TYPE_IsSameVectorType(dataType1, dataType2) \
 ((dataType1)->elementType == (dataType2)->elementType \
  && slmDATA_TYPE_vectorSize_GET(dataType1) ==  \
     slmDATA_TYPE_vectorSize_GET(dataType2))

#define slsDATA_TYPE_IsMat(dataType) \
    ((dataType)->arrayLength == 0 && (dataType)->matrixSize.columnCount != 0)

#define slsDATA_TYPE_IsUnderlyingMat(dataType) \
    ((dataType)->matrixSize.columnCount != 0)

#define slsDATA_TYPE_IsScalar(dataType) \
     (slmDATA_TYPE_IsArithmeticType(dataType) \
            && (dataType)->matrixSize.rowCount == 0 \
        && (dataType)->matrixSize.columnCount == 0)

#define slsDATA_TYPE_IsArray(dataType) \
    ((dataType)->arrayLength != 0)

#define slsDATA_TYPE_IsAtomic(dataType) \
    ((dataType)->type == T_ATOMIC_UINT)

#define slsDATA_TYPE_IsArrayLengthImplicit(dataType) \
    ((dataType)->arrayLength == -1)

#define slsDATA_TYPE_IsInheritFromUnsizedDataType(dataType) \
    ((dataType)->isInheritFromUnsizedDataType)

#define slsDATA_TYPE_IsArraysOfArrays(dataType) \
    ((dataType)->arrayLengthCount > 1)

#define slmDATA_TYPE_IsScalarInteger(dataType) \
  (slmIsElementTypeInteger((dataType)->elementType) && \
   slsDATA_TYPE_IsScalar(dataType))

#define slmDATA_TYPE_IsStrictScalarInteger(dataType) \
  (slmIsElementTypeStrictInteger((dataType)->elementType) && \
   slsDATA_TYPE_IsScalar(dataType))

#define slmDATA_TYPE_IsScalarUInteger(dataType) \
  (slmIsElementTypeUnsigned((dataType)->elementType) && \
   slsDATA_TYPE_IsScalar(dataType))

#define slmDATA_TYPE_IsScalarFloating(dataType) \
  (slmIsElementTypeFloating((dataType)->elementType) && \
   slsDATA_TYPE_IsScalar(dataType))

#define slsDATA_TYPE_IsBool(dataType) \
  (slmIsElementTypeBoolean((dataType)->elementType) && \
   slsDATA_TYPE_IsScalar(dataType))

#define slsDATA_TYPE_IsInt(dataType) \
        slmDATA_TYPE_IsScalarInteger(dataType)

#define slsDATA_TYPE_IsStrictInt(dataType) \
        slmDATA_TYPE_IsStrictScalarInteger(dataType)

#define slsDATA_TYPE_IsUInt(dataType) \
        slmDATA_TYPE_IsScalarUInteger(dataType)

#define slsDATA_TYPE_IsFloat(dataType) \
        slmDATA_TYPE_IsScalarFloating(dataType)

#define slsDATA_TYPE_IsBVec(dataType) \
    (slmIsElementTypeBoolean(dataType->elementType) \
         && slmDATA_TYPE_vectorSize_GET(dataType) !=0 \
            && (dataType)->arrayLength == 0)

#define slsDATA_TYPE_IsIVec(dataType) \
    (slmIsElementTypeInteger((dataType)->elementType) \
         && slmDATA_TYPE_vectorSize_GET(dataType) !=0 \
        && (dataType)->arrayLength == 0)

#define slsDATA_TYPE_IsSignedIVec(dataType) \
    (slmIsElementTypeSigned((dataType)->elementType) \
         && slmDATA_TYPE_vectorSize_GET(dataType) !=0 \
        && (dataType)->arrayLength == 0)

#define slsDATA_TYPE_IsUnsignedIVec(dataType) \
    (slmIsElementTypeUnsigned((dataType)->elementType) \
         && slmDATA_TYPE_vectorSize_GET(dataType) !=0 \
        && (dataType)->arrayLength == 0)

#define slsDATA_TYPE_IsVec(dataType) \
    (slmIsElementTypeFloating((dataType)->elementType) \
         && slmDATA_TYPE_vectorSize_GET(dataType) !=0 \
        && (dataType)->arrayLength == 0)

#define slsDATA_TYPE_IsBoolOrBVec(dataType) \
    (slmIsElementTypeBoolean(dataType->elementType) \
         && slmDATA_TYPE_matrixSize_GET(dataType) == 0  \
        && (dataType)->arrayLength == 0)

#define slsDATA_TYPE_IsIntOrIVec(dataType) \
    (slmIsElementTypeInteger(dataType->elementType) \
         && slmDATA_TYPE_matrixSize_GET(dataType) == 0  \
        && (dataType)->arrayLength == 0)

#define slsDATA_TYPE_IsFloatOrVec(dataType) \
    (slmIsElementTypeFloating(dataType->elementType) \
         && slmDATA_TYPE_matrixSize_GET(dataType) == 0  \
        && (dataType)->arrayLength == 0)

#define slsDATA_TYPE_IsFloatOrVecOrMat(dataType) \
    ((dataType)->arrayLength == 0 \
        && (dataType)->elementType == slvTYPE_FLOAT)

#define slsDATA_TYPE_IsVecOrMat(dataType) \
    ((dataType)->elementType == slvTYPE_FLOAT \
        && ((dataType)->matrixSize.rowCount != 0 || (dataType)->matrixSize.columnCount != 0) \
        && (dataType)->arrayLength == 0)

#define slsDATA_TYPE_IsStruct(dataType) \
    ((dataType)->arrayLength == 0 && \
     (dataType)->elementType == slvTYPE_STRUCT)

#define slsDATA_TYPE_IsUnderlyingStruct(dataType) \
    ((dataType)->elementType == slvTYPE_STRUCT)

#define slsDATA_TYPE_IsUniformBlock(dataType) \
    ((dataType)->arrayLength == 0 && \
     (dataType)->elementType == slvTYPE_UNIFORM_BLOCK)

#define slsDATA_TYPE_IsUnderlyingUniformBlock(dataType) \
    ((dataType)->elementType == slvTYPE_UNIFORM_BLOCK)

#define slsDATA_TYPE_IsStorageBlock(dataType) \
    ((dataType)->arrayLength == 0 && \
     (dataType)->elementType == slvTYPE_STORAGE_BLOCK)

#define slsDATA_TYPE_IsUnderlyingStorageBlock(dataType) \
    ((dataType)->elementType == slvTYPE_STORAGE_BLOCK)

#define slsDATA_TYPE_IsUnderlyingIOBlock(dataType) \
    ((dataType)->elementType == slvTYPE_IO_BLOCK)

#define slsDATA_TYPE_IsUnderlyingIOBlockMember(dataType) \
    ((dataType)->qualifiers.storage == slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER || \
     (dataType)->qualifiers.storage == slvSTORAGE_QUALIFIER_OUT_IO_BLOCK_MEMBER)

#define slsDATA_TYPE_IsInterfaceBlock(dataType) \
    ((dataType)->arrayLength == 0 && \
     ((dataType)->elementType == slvTYPE_UNIFORM_BLOCK || \
     (dataType)->elementType == slvTYPE_STORAGE_BLOCK || \
     (dataType)->elementType == slvTYPE_IO_BLOCK))

#define slsDATA_TYPE_IsUnderlyingInterfaceBlock(dataType) \
    (slsDATA_TYPE_IsUnderlyingUniformBlock(dataType) || \
     slsDATA_TYPE_IsUnderlyingStorageBlock(dataType) || \
     slsDATA_TYPE_IsUnderlyingIOBlock(dataType))

#define slmDATA_TYPE_IsHigherPrecision(dataType1, dataType2) \
    ((dataType1)->qualifiers.precision >= (dataType2)->qualifiers.precision)

#define slmIsElementTypeSamplerArray(EType) \
      (((EType) == slvTYPE_ISAMPLER2DARRAY) || \
       ((EType) == slvTYPE_USAMPLER2DARRAY) || \
       ((EType) == slvTYPE_SAMPLER1DARRAY) || \
       ((EType) == slvTYPE_SAMPLER1DARRAYSHADOW) || \
       ((EType) == slvTYPE_SAMPLER2DARRAY) || \
       ((EType) == slvTYPE_SAMPLER2DARRAYSHADOW))

#define slsDATA_TYPE_IsOpaque(dataType) \
    (slsDATA_TYPE_IsSampler(dataType) || \
     slsDATA_TYPE_IsAtomic(dataType))

#define slsDATA_TYPE_IsPerVertexArray(dataType) \
    ((dataType)->isPerVertexArray == gcvTRUE)

#define slsDATA_TYPE_SetPerVertexArray(dataType, d) do \
    { \
        (dataType)->isPerVertexArray = (d); \
    } \
    while (gcvFALSE)

gceSTATUS
sloCOMPILER_CreateDataType(
    IN sloCOMPILER Compiler,
    IN gctINT TokenType,
    IN struct _slsNAME_SPACE * FieldSpace,
    OUT slsDATA_TYPE ** DataType
    );

gceSTATUS
sloCOMPILER_CreateArrayDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * ElementDataType,
    IN gctINT ArrayLength,
    OUT slsDATA_TYPE ** DataType
    );

gceSTATUS
sloCOMPILER_CreateArraysOfArraysDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * ElementDataType,
    IN gctINT ArrayLengthCount,
    IN gctINT * ArrayLengthList,
    IN gctBOOL IsAppend,
    OUT slsDATA_TYPE ** DataType
    );

gceSTATUS
sloCOMPILER_CreateElementDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * CompoundDataType,
    OUT slsDATA_TYPE ** DataType
    );

gceSTATUS
sloCOMPILER_CloneDataType(
    IN sloCOMPILER Compiler,
    IN sltSTORAGE_QUALIFIER StorageQualifier,
    IN sltPRECISION_QUALIFIER PreicisionQualifier,
    IN slsDATA_TYPE * Source,
    OUT slsDATA_TYPE **DataType
    );

gceSTATUS
sloCOMPILER_DuplicateFieldSpaceForDataType(
    IN sloCOMPILER Compiler,
    IN OUT slsDATA_TYPE * DataType
    );

gceSTATUS
sloCOMPILER_InsertArrayForDataType(
    IN sloCOMPILER Compiler,
    IN slsDATA_TYPE * SourceDataType,
    IN gctINT ArrayLength,
    OUT slsDATA_TYPE ** DataType
    );

/* Name and Name space. */
typedef enum _sleNAME_TYPE
{
    slvVARIABLE_NAME,
    slvPARAMETER_NAME,
    slvFUNC_NAME,
    slvSTRUCT_NAME,
    slvFIELD_NAME,
    slvINTERFACE_BLOCK_NAME
}
sleNAME_TYPE;

struct _slsNAME_SPACE;
struct _sloIR_SET;
struct _sloIR_CONSTANT;
struct _sloIR_POLYNARY_EXPR;
struct _slsLOGICAL_REG;
struct _sloIR_LABEL;

struct _slsINTERFACE_BLOCK_MEMBER;
typedef struct _slsINTERFACE_BLOCK_MEMBER
{
    slsDLINK_NODE node;
    struct _slsNAME *name;
    gctBOOL isActive;
}
slsINTERFACE_BLOCK_MEMBER;

typedef struct _slsNAME
{
    slsDLINK_NODE                           node;
    struct _slsNAME_SPACE *                 mySpace;
    gctUINT                                 lineNo;
    gctUINT                                 stringNo;
    sleNAME_TYPE                            type;
    slsDATA_TYPE *                          dataType;
    sltPOOL_STRING                          symbol;
    gctBOOL                                 isBuiltIn;
    sleEXTENSION                            extension;
    /* Whether this variable is a per-vertex array, TC/GS only. */
    gctBOOL                                 isPerVertexArray;
    /* Whether this variable is a per-vertex variable, but not declare as an array, TC/GS only. */
    /* When this flag is TRUE, we should report a LINK error as specs required. */
    gctBOOL                                 isPerVertexNotAnArray;

    union
    {
        struct
        {
            /* Only for constant variables */
            struct _sloIR_CONSTANT *        constant;
            /* if variable belongs to an interface block which can be a uniform or storage block */
            struct _slsNAME *               interfaceBlock;
            /* for a sampler with LODmin, LODmax:
               uniform is a vec2 where the x component is LODmin
               and the y component is LODmax */
            gcUNIFORM                       lodMinMax;
            /* base level of sampler containing the width and height */
            gcUNIFORM                       levelBaseSize;
            /* flag to indicate if variable is local */
            gctBOOL                         isLocal;
            /* flag to indicate if variable is referenced in shader */
            gctBOOL                         isReferenced;
            /* flag to indicate if variable can be used as iteration unroll index */
            gctBOOL                         isCanUsedAsUnRollLoopIndex;
        }
        variableInfo;

        struct
        {
            struct _slsNAME *               aliasName;
        }
        parameterInfo;

        /* Only for function names */
        struct
        {
            /*
            ** Parameters are saved in localSpace.
            ** For ES20, functionBodySpace is sub-space of localSpace;
            ** For ES30 and above, functionBodySpace is localSpace.
            */
            /* function's definition name space. */
            struct _slsNAME_SPACE *         localSpace;
            gceINTRINSICS_KIND              intrinsicKind;
            /* this is used to find the builtin function in library */
            sltPOOL_STRING                  mangled_symbol;
            slsBuiltInFuncCheck             function;
            slsBuiltInEvaluateFunc          evaluate;
            slsBuiltInGenCodeFunc           genCode;
            /* Function flag. */
            sltFUNC_FLAG                    flags;
            /* function's body name space. */
            struct _slsNAME_SPACE *         functionBodySpace;
            struct _sloIR_SET     *         funcBody;
        }
        funcInfo;

        /* Only for interface block names */
        struct
        {
            slsDLINK_LIST                   members;
            union
            {
                gcsUNIFORM_BLOCK            uniformBlock;
                gcsSTORAGE_BLOCK            storageBlock;
                gcsIO_BLOCK                 ioBlock;
                gcsINTERFACE_BLOCK_INFO *   interfaceBlockInfo;
            }
            u;
            sleIB_FLAG                      flags;
        }
        interfaceBlockContent;
    }
    u;

    struct
    {
        gctUINT                             logicalRegCount;
        struct _slsLOGICAL_REG *            logicalRegs;
        gctBOOL                             useAsTextureCoord;
        gctBOOL                             isCounted;
        /* Only for the function/parameter names */
        gcFUNCTION                          function;
    }
    context;
}
slsNAME;

#define slsNAME_IsPerVertexArray(name) \
    ((name)->isPerVertexArray == gcvTRUE)

#define slsNAME_SetPerVertexArray(name, d) do \
    { \
        (name)->isPerVertexArray = (d); \
    } \
    while (gcvFALSE)


typedef struct _slsASM_OPCODE
{
    gctUINT                lineNo;

    gctUINT                stringNo;

    gctUINT                opcode;

    gctUINT                round;

    gctUINT                satuate;

    gctUINT                threadMode;

    gctUINT                condition;
}
slsASM_OPCODE;


typedef enum _sleASM_MODIFIER_TYPE
{
    sleASM_MODIFIER_OPND_PRECISION      = 0,
    sleASM_MODIFIER_OPND_FORMAT         = 1,
    sleASM_MODIFIER_OPND_ABS            = 2,
    sleASM_MODIFIER_OPND_NEG            = 3,

    sleASM_MODIFIER_OPCODE_CONDITION    = 0,
    sleASM_MODIFIER_OPCODE_THREAD_MODE  = 1,
    sleASM_MODIFIER_OPCODE_ROUND        = 2,
    sleASM_MODIFIER_OPCODE_SAT          = 3,

    sleASM_MODIFIER_COUNT               = 4,
}
sleASM_MODIFIER_TYPE;

typedef struct _slsASM_MODIFIER
{
    sleASM_MODIFIER_TYPE type;
    gctINT               value;
}
slsASM_MODIFIER;

typedef struct _slsASM_MODIFIERS
{
    slsASM_MODIFIER modifiers[sleASM_MODIFIER_COUNT];
}
slsASM_MODIFIERS;

gceSTATUS
sloNAME_BindFuncBody(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName,
    IN struct _sloIR_SET * FuncBody
    );

gceSTATUS
sloNAME_GetParamCount(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName,
    OUT gctUINT * ParamCount
    );

gceSTATUS
slsNAME_BindAliasParamNames(
    IN sloCOMPILER Compiler,
    IN OUT slsNAME * FuncDefName,
    IN slsNAME * FuncDeclName
    );

gceSTATUS
slsNAME_Dump(
    IN sloCOMPILER Compiler,
    IN slsNAME * Name
    );

typedef struct _slsNAME_SPACE
{
    slsDLINK_NODE            node;

    struct _slsNAME_SPACE *  parent;

    slsDLINK_LIST            names;

    slsDLINK_LIST            subSpaces;

    /* Precision has same scope rule as variables, so put it */
    /* in namespace to better management. Only used for parsing */
    /* time, codegen does not use it */
    /* Now, precision is used in Low power shader FP16 medium feature */
    sltPRECISION_QUALIFIER   defaultPrecision[slvTYPE_TOTAL_COUNT];

    sltPOOL_STRING           spaceName;

    VSC_DIE                  die;
}
slsNAME_SPACE;

gceSTATUS
slsNAME_Destory(
    IN sloCOMPILER Compiler,
    IN slsNAME * Name
    );

gceSTATUS
slsNAME_SPACE_SetSpaceName(
    IN slsNAME_SPACE * Space,
    IN sltPOOL_STRING SpaceName
    );

gceSTATUS
slsNAME_SPACE_Construct(
    IN sloCOMPILER Compiler,
    IN sltPOOL_STRING SpaceName,
    IN slsNAME_SPACE * Parent,
    OUT slsNAME_SPACE ** NameSpace
    );

gceSTATUS
slsNAME_SPACE_Destory(
    IN sloCOMPILER Compiler,
    IN slsNAME_SPACE * NameSpace
    );

gceSTATUS
slsNAME_SPACE_Dump(
    IN sloCOMPILER Compiler,
    IN slsNAME_SPACE * NameSpace
    );

gceSTATUS
slsNAME_SPACE_Search(
    IN sloCOMPILER Compiler,
    IN slsNAME_SPACE * NameSpace,
    IN sltPOOL_STRING Symbol,
    IN gctBOOL Recursive,
    IN gctBOOL MangleNameMatch,
    OUT slsNAME ** Name
    );

gceSTATUS
slsNAME_SPACE_CheckNewFuncName(
    IN sloCOMPILER Compiler,
    IN slsNAME_SPACE * NameSpace,
    IN slsNAME * FuncName,
    OUT slsNAME ** FirstFuncName
    );

/*
** Functions can only be declared within the global name space.
*/
gceSTATUS
slsNAME_SPACE_CheckFuncInGlobalNamespace(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName
    );

gceSTATUS
slsNAME_SPACE_BindFuncName(
    IN sloCOMPILER Compiler,
    IN slsNAME_SPACE * NameSpace,
    IN OUT struct _sloIR_POLYNARY_EXPR * PolynaryExpr
    );

gceSTATUS
slsNAME_SPACE_CreateName(
    IN sloCOMPILER Compiler,
    IN slsNAME_SPACE * NameSpace,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleNAME_TYPE Type,
    IN slsDATA_TYPE * DataType,
    IN sltPOOL_STRING Symbol,
    IN gctBOOL IsBuiltIn,
    IN sleEXTENSION Extension,
    IN gctBOOL CheckExistedName,
    OUT slsNAME ** Name
    );

gceSTATUS
slsNAME_Initialize(
    IN sloCOMPILER Compiler,
    IN slsNAME * Name,
    IN gctBOOL InitGeneralPart
    );

gceSTATUS
sloCOMPILER_CreateName(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleNAME_TYPE Type,
    IN slsDATA_TYPE * DataType,
    IN sltPOOL_STRING Symbol,
    IN sleEXTENSION Extension,
    IN gctBOOL CheckExistedName,
    OUT slsNAME ** Name
    );

gceSTATUS
sloCOMPILER_SetCheckFunctionForBuiltInFunction(
    IN sloCOMPILER Compiler,
    IN slsBuiltInFuncCheck Function,
    IN slsNAME * FuncName
    );

gceSTATUS
sloCOMPILER_CreateAuxiliaryName(
    IN sloCOMPILER Compiler,
    IN slsNAME* refName,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsDATA_TYPE * DataType,
    OUT slsNAME ** Name
    );

gceSTATUS
sloCOMPILER_SearchName(
    IN sloCOMPILER Compiler,
    IN sltPOOL_STRING Symbol,
    IN gctBOOL Recursive,
    OUT slsNAME ** Name
    );

gceSTATUS
sloCOMPILER_SearchBuiltinName(
    IN sloCOMPILER Compiler,
    IN gctSTRING Symbol,
    OUT slsNAME ** Name
    );

gceSTATUS
sloCOMPILER_SearchIntrinsicBuiltinName(
    IN sloCOMPILER Compiler,
    IN gctSTRING Symbol,
    OUT slsNAME ** Name
    );

gceSTATUS
sloCOMPILER_CheckNewFuncName(
    IN sloCOMPILER Compiler,
    IN slsNAME * FuncName,
    OUT slsNAME ** FirstFuncName
    );

gceSTATUS
sloCOMPILER_CreateNameSpace(
    IN sloCOMPILER Compiler,
    IN sltPOOL_STRING SpaceName,
    OUT slsNAME_SPACE ** NameSpace
    );

gceSTATUS
sloCOMPILER_CreateAuxGlobalNameSpace(
    IN sloCOMPILER Compiler,
    OUT slsNAME_SPACE ** NameSpace
    );

gctBOOL
slNameIsLocal(
IN sloCOMPILER Compiler,
IN slsNAME * Name
);

slsNAME_SPACE *
sloCOMPILER_GetCurrentSpace(
IN sloCOMPILER Compiler
);

slsNAME_SPACE *
sloCOMPILER_GetGlobalSpace(
IN sloCOMPILER Compiler
);

slsNAME_SPACE *
sloCOMPILER_GetBuiltInSpace(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_MergeLayoutId(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *DefaultLayout,
    IN OUT slsLAYOUT_QUALIFIER *Layout
    );

gceSTATUS
sloCOMPILER_MergeInterFaceLayoutId(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *DefaultLayout,
    IN gctBOOL IsAtomicCounter,
    IN gctBOOL IsInterFace,
    IN OUT slsLAYOUT_QUALIFIER *Layout
);

gctBOOL
sloCOMPILER_DefaultComputeGroupLayoutMatch(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *Layout
    );

gceSTATUS
sloCOMPILER_SetDefaultLayout(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER *Layout,
    IN sltSTORAGE_QUALIFIER StorageQualifier
    );

gceSTATUS
sloCOMPILER_GetDefaultLayout(
    IN sloCOMPILER Compiler,
    OUT slsLAYOUT_QUALIFIER *Layout,
    IN sltSTORAGE_QUALIFIER StorageQualifier
    );

gceSTATUS
sloCOMPILER_UpdateDefaultLayout(
    IN sloCOMPILER Compiler,
    IN slsLAYOUT_QUALIFIER Layout,
    IN sltSTORAGE_QUALIFIER StorageQualifier
    );

gceSTATUS
sloCOMPILER_PopCurrentNameSpace(
    IN sloCOMPILER Compiler,
    OUT slsNAME_SPACE ** PrevNameSpace
    );

slsNAME_SPACE *
sloCOMPILER_GetUnnamedSpace(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_PushUnnamedSpace(
    IN sloCOMPILER Compiler,
    OUT slsNAME_SPACE ** UnnamedSpace
    );

gceSTATUS
sloCOMPILER_AtGlobalNameSpace(
    IN sloCOMPILER Compiler,
    OUT gctBOOL * AtGlobalNameSpace
    );

gceSTATUS
sloCOMPILER_SetDefaultPrecision(
    IN sloCOMPILER Compiler,
    IN sltELEMENT_TYPE typeToSet,
    IN sltPRECISION_QUALIFIER precision
    );

gceSTATUS
sloCOMPILER_GetDefaultPrecision(
    IN sloCOMPILER Compiler,
    IN sltELEMENT_TYPE typeToGet,
    OUT sltPRECISION_QUALIFIER *precision
    );

sleLAYOUT_ID
sloCOMPILER_SearchLayoutId(
    IN sloCOMPILER Compiler,
    IN struct _slsLexToken *LayoutId
    );

/* Type of IR objects. */
typedef enum _sleIR_OBJECT_TYPE
{
    slvIR_UNKNOWN           = 0,

    slvIR_SET               = gcmCC('S','E','T','\0'),
    slvIR_ITERATION         = gcmCC('I','T','E','R'),
    slvIR_JUMP              = gcmCC('J','U','M','P'),
    slvIR_LABEL             = gcmCC('L','A','B','\0'),
    slvIR_VARIABLE          = gcmCC('V','A','R','\0'),
    slvIR_CONSTANT          = gcmCC('C','N','S','T'),
    slvIR_UNARY_EXPR        = gcmCC('U','N','R','Y'),
    slvIR_BINARY_EXPR       = gcmCC('B','N','R','Y'),
    slvIR_SELECTION         = gcmCC('S','E','L','T'),
    slvIR_SWITCH            = gcmCC('S','W','I','T'),
    slvIR_POLYNARY_EXPR     = gcmCC('P','O','L','Y'),
    slvIR_VIV_ASM           = gcmCC('A','S','M','\0'),
}
sleIR_OBJECT_TYPE;

/* sloIR_BASE object. */
struct _slsVTAB;
typedef struct _slsVTAB *    sltVPTR;

struct _sloIR_BASE
{
    slsDLINK_NODE        node;

    sltVPTR                vptr;

    gctUINT                lineNo;

    gctUINT                stringNo;

    gctUINT                lineEnd;
};

typedef struct _sloIR_BASE *            sloIR_BASE;

typedef gceSTATUS
(* sltDESTROY_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE This
    );

typedef gceSTATUS
(* sltDUMP_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE This
    );

struct _slsVISITOR;

typedef gceSTATUS
(* sltACCEPT_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE This,
    IN struct _slsVISITOR * Visitor,
    IN OUT gctPOINTER Parameters
    );

typedef struct _slsVTAB
{
    sleIR_OBJECT_TYPE    type;

    sltDESTROY_FUNC_PTR    destroy;

    sltDUMP_FUNC_PTR    dump;

    sltACCEPT_FUNC_PTR    accept;
}
slsVTAB;

#define sloIR_BASE_Initialize(base, finalVPtr, finalLineNo, finalStringNo, finalLineEndNo) \
    do \
    { \
        (base)->vptr        = (finalVPtr); \
        (base)->lineNo        = (finalLineNo); \
        (base)->stringNo    = (finalStringNo); \
        (base)->lineEnd     = (finalLineEndNo);\
    } \
    while (gcvFALSE)

#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
#    define slmVERIFY_IR_OBJECT(obj, objType) \
        do \
        { \
            if (((obj) == gcvNULL) || (((sloIR_BASE)(obj))->vptr->type != (objType))) \
            { \
                gcmASSERT(((obj) != gcvNULL) && (((sloIR_BASE)(obj))->vptr->type == (objType))); \
                return gcvSTATUS_INVALID_OBJECT; \
            } \
        } \
        while (gcvFALSE)
#else
#    define slmVERIFY_IR_OBJECT(obj, objType) do {} while (gcvFALSE)
#endif

#define sloIR_OBJECT_GetType(obj) \
    ((obj)->vptr->type)

#define sloIR_OBJECT_Destroy(compiler, obj) \
    ((obj)->vptr->destroy((compiler), (obj)))

#define sloIR_OBJECT_Dump(compiler, obj) \
    ((obj)->vptr->dump((compiler), (obj)))

#define sloIR_OBJECT_Accept(compiler, obj, visitor, parameters) \
    ((obj)->vptr->accept((compiler), (obj), (visitor), (parameters)))

/* sloIR_SET object. */
typedef enum _sleSET_TYPE
{
    slvDECL_SET,
    slvSTATEMENT_SET,
    slvEXPR_SET
}
sleSET_TYPE;

struct _sloIR_SET
{
    struct _sloIR_BASE    base;

    sleSET_TYPE            type;

    slsDLINK_LIST        members;

    slsNAME *            funcName;    /* Only for the function definition */
};

typedef struct _sloIR_SET *                sloIR_SET;

gceSTATUS
sloIR_SET_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleSET_TYPE Type,
    OUT sloIR_SET * Set
    );

gceSTATUS
sloIR_SET_AddMember(
    IN sloCOMPILER Compiler,
    IN sloIR_SET Set,
    IN sloIR_BASE Member
    );

gceSTATUS
sloIR_SET_GetMemberCount(
    IN sloCOMPILER Compiler,
    IN sloIR_SET Set,
    OUT gctUINT * MemberCount
    );

gceSTATUS
sloCOMPILER_AddExternalDecl(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE ExternalDecl
    );

/* sloIR_EXPR object. */
struct _sloIR_EXPR
{
    struct _sloIR_BASE    base;

    slsDATA_TYPE *        dataType;

    slsDATA_TYPE *        toBeDataType;

    slsASM_MODIFIERS     *asmMods;
};

typedef struct _sloIR_EXPR *            sloIR_EXPR;

#define sloIR_EXPR_Initialize(expr, finalVPtr, finalLineNo, finalStringNo, finalLineEndNo, exprDataType) \
    do \
    { \
        sloIR_BASE_Initialize(&(expr)->base, (finalVPtr), (finalLineNo), (finalStringNo), (finalLineEndNo)); \
        \
        (expr)->dataType = (exprDataType); \
        (expr)->toBeDataType = gcvNULL; \
        (expr)->asmMods  = gcvNULL; \
    } \
    while (gcvFALSE)

#define sloIR_EXPR_Initialize_With_ASM_Opnd_Mods(expr, finalVPtr, finalLineNo, finalStringNo, exprDataType, asmOpndMods) \
    do \
    { \
        sloIR_BASE_Initialize(&(expr)->base, (finalVPtr), (finalLineNo), (finalStringNo), (finalLineNo)); \
        \
        (expr)->dataType = (exprDataType); \
        (expr)->toBeDataType = gcvNULL; \
        (expr)->asmOpndMods = (asmOpndMods); \
    } \
    while (gcvFALSE)

#define sloIR_EXPR_ImplicitConversionDone(expr)     ((expr)->toBeDataType && ((expr)->dataType->elementType != (expr)->toBeDataType->elementType))
#define sloIR_EXPR_SetToBeTheSameDataType(expr)     ((expr)->toBeDataType = (expr)->dataType)

/* sloIR_ITERATION object. */
typedef enum _sleITERATION_TYPE
{
    slvFOR,
    slvWHILE,
    slvDO_WHILE
}
sleITERATION_TYPE;

struct _sloIR_ITERATION
{
    struct _sloIR_BASE      base;
    sleITERATION_TYPE       type;
    sloIR_EXPR              condExpr;
    sloIR_BASE              loopBody;
    slsNAME_SPACE           *forSpace;
    sloIR_BASE              forInitStatement;
    sloIR_EXPR              forRestExpr;
    /* The atomic-related operation count within loop body. */
    gctUINT                 atomicOpCount;
};

typedef struct _sloIR_ITERATION *        sloIR_ITERATION;

gceSTATUS
sloIR_ITERATION_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleITERATION_TYPE Type,
    IN sloIR_EXPR CondExpr,
    IN sloIR_BASE LoopBody,
    IN slsNAME_SPACE * ForSpace,
    IN sloIR_BASE ForInitStatement,
    IN sloIR_EXPR ForRestExpr,
    OUT sloIR_ITERATION * Iteration
    );

/* sloIR_JUMP object. */
typedef enum _sleJUMP_TYPE
{
    slvCONTINUE,
    slvBREAK,
    slvRETURN,
    slvDISCARD
}
sleJUMP_TYPE;

gctCONST_STRING
slGetIRJumpTypeName(
    IN sleJUMP_TYPE JumpType
    );

struct _sloIR_JUMP
{
    struct _sloIR_BASE    base;

    sleJUMP_TYPE        type;

    sloIR_EXPR            returnExpr;
};

typedef struct _sloIR_JUMP *            sloIR_JUMP;

gceSTATUS
sloIR_JUMP_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleJUMP_TYPE Type,
    IN sloIR_EXPR ReturnExpr,
    OUT sloIR_JUMP * Jump
    );

/* sloIR_SWITCH label type. */
typedef enum _sleLABEL_TYPE
{
    slvCASE,
    slvDEFAULT
}
sleLABEL_TYPE;

/* sloIR_LABEL object. */
struct _sloIR_LABEL
{
    struct _sloIR_BASE base;
    sleLABEL_TYPE type;
    struct _sloIR_LABEL *nextCase;
    gctLABEL programCounter;
    struct _sloIR_CONSTANT *caseValue;
    gctUINT16 caseNumber;
};

typedef struct _sloIR_LABEL *sloIR_LABEL;

void
sloIR_LABEL_Initialize(
IN gctUINT LineNo,
IN gctUINT StringNo,
IN OUT sloIR_LABEL Label
);

gceSTATUS
sloIR_LABEL_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    OUT sloIR_LABEL *Label
    );

/* sloIR_VARIABLE object. */
struct _sloIR_VARIABLE
{
    struct _sloIR_EXPR    exprBase;

    slsNAME *            name;
};

typedef struct _sloIR_VARIABLE *        sloIR_VARIABLE;

gceSTATUS
sloIR_VARIABLE_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsNAME * Name,
    OUT sloIR_VARIABLE * Variable
    );

/* sloIR_CONSTANT object. */
typedef union _sluCONSTANT_VALUE
{
    gctBOOL        boolValue;
    gctINT32    intValue;
    gctUINT32     uintValue;
    gctFLOAT    floatValue;
}
sluCONSTANT_VALUE;

struct _sloIR_CONSTANT
{
    struct _sloIR_EXPR    exprBase;

    gctUINT            valueCount;

    sluCONSTANT_VALUE * values;
    slsNAME * variable;
    gctBOOL allValuesEqual;
};

typedef struct _sloIR_CONSTANT *        sloIR_CONSTANT;

gceSTATUS
sloIR_CONSTANT_SetValues(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT Constant,
    IN gctUINT ValueCount,
    IN sluCONSTANT_VALUE * Values
    );

gceSTATUS
sloIR_CONSTANT_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsDATA_TYPE * DataType,
    OUT sloIR_CONSTANT * Constant
    );

gceSTATUS
sloIR_CONSTANT_Initialize(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsDATA_TYPE * DataType,
    IN gctUINT ValueCount,
    IN sluCONSTANT_VALUE * Values,
    IN OUT sloIR_CONSTANT Constant
    );

gceSTATUS
sloIR_CONSTANT_Clone(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sloIR_CONSTANT Source,
    OUT sloIR_CONSTANT * Constant
    );

gceSTATUS
sloIR_CONSTANT_Destroy(
    IN sloCOMPILER Compiler,
    IN sloIR_BASE This
    );

gceSTATUS
sloIR_CONSTANT_AddValues(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT Constant,
    IN gctUINT ValueCount,
    IN sluCONSTANT_VALUE * Values
    );

gceSTATUS
sloIR_CONSTANT_GetBoolValue(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT sluCONSTANT_VALUE * Value
    );

gceSTATUS
sloIR_CONSTANT_GetIntValue(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT sluCONSTANT_VALUE * Value
    );

gceSTATUS
sloIR_CONSTANT_GetFloatValue(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT Constant,
    IN gctUINT ValueNo,
    OUT sluCONSTANT_VALUE * Value
    );

typedef gceSTATUS
(* sltEVALUATE_FUNC_PTR)(
    IN sltELEMENT_TYPE Type,
    IN OUT sluCONSTANT_VALUE * Value
    );

gctBOOL
sloIR_CONSTANT_CheckAndSetAllValuesEqual(
IN sloCOMPILER Compiler,
IN sloIR_CONSTANT Constant
);

gceSTATUS
sloIR_CONSTANT_Evaluate(
    IN sloCOMPILER Compiler,
    IN OUT sloIR_CONSTANT Constant,
    IN sltEVALUATE_FUNC_PTR Evaluate
    );

/* sloIR_UNARY_EXPR object. */
typedef enum _sleUNARY_EXPR_TYPE
{
    slvUNARY_FIELD_SELECTION,
    slvUNARY_COMPONENT_SELECTION,

    slvUNARY_POST_INC,
    slvUNARY_POST_DEC,
    slvUNARY_PRE_INC,
    slvUNARY_PRE_DEC,

    slvUNARY_NEG,

    slvUNARY_NOT_BITWISE,
    slvUNARY_NOT
}
sleUNARY_EXPR_TYPE;

typedef enum _sleCOMPONENT
{
    slvCOMPONENT_X    = 0x0,
    slvCOMPONENT_Y    = 0x1,
    slvCOMPONENT_Z    = 0x2,
    slvCOMPONENT_W    = 0x3
}
sleCOMPONENT;

typedef struct _slsCOMPONENT_SELECTION
{
    gctUINT8    components;

    gctUINT8    x;

    gctUINT8    y;

    gctUINT8    z;

    gctUINT8    w;
}
slsCOMPONENT_SELECTION;

extern slsCOMPONENT_SELECTION    ComponentSelection_X;
extern slsCOMPONENT_SELECTION    ComponentSelection_Y;
extern slsCOMPONENT_SELECTION    ComponentSelection_Z;
extern slsCOMPONENT_SELECTION    ComponentSelection_W;
extern slsCOMPONENT_SELECTION    ComponentSelection_XY;
extern slsCOMPONENT_SELECTION    ComponentSelection_XYZ;
extern slsCOMPONENT_SELECTION    ComponentSelection_XYZW;

gctBOOL
slIsRepeatedComponentSelection(
    IN slsCOMPONENT_SELECTION * ComponentSelection
    );

gctCONST_STRING
slGetIRUnaryExprTypeName(
    IN sleUNARY_EXPR_TYPE UnaryExprType
    );

struct _sloIR_UNARY_EXPR
{
    struct _sloIR_EXPR            exprBase;

    sleUNARY_EXPR_TYPE            type;

    sloIR_EXPR                    operand;

    union
    {
        slsNAME *                fieldName;                /* Only for the field selection */

        slsCOMPONENT_SELECTION    componentSelection;        /* Only for the component selection */
    }
    u;
};

typedef struct _sloIR_UNARY_EXPR *        sloIR_UNARY_EXPR;

gceSTATUS
sloIR_UNARY_EXPR_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleUNARY_EXPR_TYPE Type,
    IN sloIR_EXPR Operand,
    IN slsNAME * FieldName,                                /* Only for the field selection */
    IN slsCOMPONENT_SELECTION * ComponentSelection,        /* Only for the component selection */
    OUT sloIR_UNARY_EXPR * UnaryExpr
    );

gceSTATUS
sloIR_UNARY_EXPR_Evaluate(
    IN sloCOMPILER Compiler,
    IN sleUNARY_EXPR_TYPE Type,
    IN sloIR_CONSTANT Constant,
    IN slsNAME * FieldName,                                /* Only for the field selection */
    IN slsCOMPONENT_SELECTION * ComponentSelection,        /* Only for the component selection */
    OUT sloIR_CONSTANT * ResultConstant
    );

/* sloIR_BINARY_EXPR object. */
typedef enum _sleBINARY_EXPR_TYPE
{
    slvBINARY_SUBSCRIPT,

    slvBINARY_ADD,
    slvBINARY_SUB,
    slvBINARY_MUL,
    slvBINARY_DIV,
    slvBINARY_MOD,

    slvBINARY_AND_BITWISE,
    slvBINARY_OR_BITWISE,
    slvBINARY_XOR_BITWISE,

    slvBINARY_LSHIFT,
    slvBINARY_RSHIFT,

    slvBINARY_GREATER_THAN,
    slvBINARY_LESS_THAN,
    slvBINARY_GREATER_THAN_EQUAL,
    slvBINARY_LESS_THAN_EQUAL,

    slvBINARY_EQUAL,
    slvBINARY_NOT_EQUAL,

    slvBINARY_AND,
    slvBINARY_OR,
    slvBINARY_XOR,

    slvBINARY_SEQUENCE,

    slvBINARY_ASSIGN,

    slvBINARY_LEFT_ASSIGN,
    slvBINARY_RIGHT_ASSIGN,
    slvBINARY_AND_ASSIGN,
    slvBINARY_XOR_ASSIGN,
    slvBINARY_OR_ASSIGN,

    slvBINARY_MUL_ASSIGN,
    slvBINARY_DIV_ASSIGN,
    slvBINARY_ADD_ASSIGN,
    slvBINARY_SUB_ASSIGN,
    slvBINARY_MOD_ASSIGN
}
sleBINARY_EXPR_TYPE;

struct _sloIR_BINARY_EXPR
{
    struct _sloIR_EXPR    exprBase;

    sleBINARY_EXPR_TYPE    type;

    sloIR_EXPR            leftOperand;

    sloIR_EXPR            rightOperand;

    struct
    {
        /* If type is slvBINARY_SUBSCRIPT and leftOperand is vector, */
        /* then we need generate an auxiliary symbol to hold scalar */
        /* array */
        /* mat2Array is only used for IO block.*/
        struct _slsVEC2ARRAY*            vec2Array;
        struct _slsVEC2ARRAY*            mat2Array;
    }u;
};

typedef struct _sloIR_BINARY_EXPR *        sloIR_BINARY_EXPR;

gceSTATUS
sloIR_BINARY_EXPR_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctUINT LineEndNo,
    IN sleBINARY_EXPR_TYPE Type,
    IN sloIR_EXPR LeftOperand,
    IN sloIR_EXPR RightOperand,
    OUT sloIR_BINARY_EXPR * BinaryExpr
    );

gceSTATUS
sloIR_BINARY_EXPR_Evaluate(
    IN sloCOMPILER Compiler,
    IN sleBINARY_EXPR_TYPE Type,
    IN sloIR_CONSTANT LeftConstant,
    IN sloIR_CONSTANT RightConstant,
    OUT sloIR_CONSTANT * ResultConstant
    );

/* sloIR_SELECTION object. */
struct _sloIR_SELECTION
{
    struct _sloIR_EXPR    exprBase;

    sloIR_EXPR            condExpr;

    sloIR_BASE            trueOperand;

    sloIR_BASE            falseOperand;
};

typedef struct _sloIR_SELECTION *        sloIR_SELECTION;

gceSTATUS
sloIR_SELECTION_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsDATA_TYPE * DataType,
    IN sloIR_EXPR CondExpr,
    IN sloIR_BASE TrueOperand,
    IN sloIR_BASE FalseOperand,
    OUT sloIR_SELECTION * Selection
    );

/* sloIR_SWITCH object. */
struct _sloIR_SWITCH
{
    struct _sloIR_EXPR    exprBase;
    sloIR_EXPR        condExpr;
    sloIR_BASE        switchBody;
    sloIR_LABEL        cases;
};

typedef struct _sloIR_SWITCH *sloIR_SWITCH;

gceSTATUS
sloIR_SWITCH_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sloIR_EXPR CondExpr,
    IN sloIR_BASE SwitchBody,
    IN sloIR_LABEL Cases,
    OUT sloIR_SWITCH * Selection
    );

/* sloIR_POLYNARY_EXPR object. */
typedef enum _slePOLYNARY_EXPR_TYPE
{
    slvPOLYNARY_CONSTRUCT_NONE,
    slvPOLYNARY_CONSTRUCT_FLOAT,
    slvPOLYNARY_CONSTRUCT_INT,
    slvPOLYNARY_CONSTRUCT_UINT,
    slvPOLYNARY_CONSTRUCT_BOOL,
    slvPOLYNARY_CONSTRUCT_VEC2,
    slvPOLYNARY_CONSTRUCT_VEC3,
    slvPOLYNARY_CONSTRUCT_VEC4,
    slvPOLYNARY_CONSTRUCT_BVEC2,
    slvPOLYNARY_CONSTRUCT_BVEC3,
    slvPOLYNARY_CONSTRUCT_BVEC4,
    slvPOLYNARY_CONSTRUCT_IVEC2,
    slvPOLYNARY_CONSTRUCT_IVEC3,
    slvPOLYNARY_CONSTRUCT_IVEC4,
    slvPOLYNARY_CONSTRUCT_UVEC2,
    slvPOLYNARY_CONSTRUCT_UVEC3,
    slvPOLYNARY_CONSTRUCT_UVEC4,
    slvPOLYNARY_CONSTRUCT_MAT2,
    slvPOLYNARY_CONSTRUCT_MAT2X3,
    slvPOLYNARY_CONSTRUCT_MAT2X4,
    slvPOLYNARY_CONSTRUCT_MAT3,
    slvPOLYNARY_CONSTRUCT_MAT3X2,
    slvPOLYNARY_CONSTRUCT_MAT3X4,
    slvPOLYNARY_CONSTRUCT_MAT4,
    slvPOLYNARY_CONSTRUCT_MAT4X2,
    slvPOLYNARY_CONSTRUCT_MAT4X3,
    slvPOLYNARY_CONSTRUCT_STRUCT,
    slvPOLYNARY_CONSTRUCT_ARRAY,
    slvPOLYNARY_CONSTRUCT_ARRAYS_OF_ARRAYS,

    slvPOLYNARY_FUNC_CALL
}
slePOLYNARY_EXPR_TYPE;

struct _sloIR_POLYNARY_EXPR
{
    struct _sloIR_EXPR        exprBase;

    slePOLYNARY_EXPR_TYPE    type;

    sltPOOL_STRING            funcSymbol;    /* Only for the function call */

    slsNAME *                funcName;    /* Only for the function call */

    sloIR_SET                operands;
};

typedef struct _sloIR_POLYNARY_EXPR *    sloIR_POLYNARY_EXPR;

gctCONST_STRING
slGetIRPolynaryExprTypeName(
    IN slePOLYNARY_EXPR_TYPE PolynaryExprType
    );

gceSTATUS
sloIR_POLYNARY_EXPR_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slePOLYNARY_EXPR_TYPE Type,
    IN slsDATA_TYPE * DataType,
    IN sltPOOL_STRING FuncSymbol,
    OUT sloIR_POLYNARY_EXPR * PolynaryExpr
    );


struct _sloIR_VIV_ASM
{
    struct _sloIR_BASE        base;

    slsASM_OPCODE             opcode;

    sloIR_SET                 operands;

    slsASM_MODIFIERS        **opndMods;
};

typedef struct _sloIR_VIV_ASM * sloIR_VIV_ASM;

gceSTATUS
sloIR_VIV_ASM_Construct(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN slsASM_OPCODE *AsmOpcode,
    OUT sloIR_VIV_ASM * VivAsm
    );

gceSTATUS
sloCOMPILER_BindFuncCall(
    IN sloCOMPILER Compiler,
    IN OUT sloIR_POLYNARY_EXPR PolynaryExpr
    );

gceSTATUS
sloIR_POLYNARY_EXPR_Evaluate(
    IN sloCOMPILER Compiler,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    OUT sloIR_CONSTANT * ResultConstant
    );

/* Visitor */
typedef gceSTATUS
(* sltVISIT_SET_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_SET Set,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_ITERATION_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_ITERATION Iteration,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_JUMP_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_JUMP Jump,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_LABEL_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_LABEL Label,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_VARIABLE_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_VARIABLE Variable,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_CONSTANT_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_CONSTANT Constant,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_UNARY_EXPR_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_UNARY_EXPR UnaryExpr,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_BINARY_EXPR_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_BINARY_EXPR BinaryExpr,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_SELECTION_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_SELECTION Selection,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_SWITCH_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_SWITCH Selection,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_POLYNARY_EXPR_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN OUT gctPOINTER Parameters
    );

typedef gceSTATUS
(* sltVISIT_VIV_ASM_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN struct _slsVISITOR * Visitor,
    IN sloIR_VIV_ASM VivAsm,
    IN OUT gctPOINTER Parameters
    );

typedef struct _slsVISITOR
{
    slsOBJECT                       object;
    sltVISIT_SET_FUNC_PTR           visitSet;
    sltVISIT_ITERATION_FUNC_PTR     visitIteration;
    sltVISIT_JUMP_FUNC_PTR          visitJump;
    sltVISIT_LABEL_FUNC_PTR        visitLabel;
    sltVISIT_VARIABLE_FUNC_PTR      visitVariable;
    sltVISIT_CONSTANT_FUNC_PTR      visitConstant;
    sltVISIT_UNARY_EXPR_FUNC_PTR    visitUnaryExpr;
    sltVISIT_BINARY_EXPR_FUNC_PTR   visitBinaryExpr;
    sltVISIT_SELECTION_FUNC_PTR     visitSelection;
    sltVISIT_SWITCH_FUNC_PTR        visitSwitch;
    sltVISIT_POLYNARY_EXPR_FUNC_PTR    visitPolynaryExpr;
    sltVISIT_VIV_ASM_FUNC_PTR       visitVivAsm;
}
slsVISITOR;

gceSTATUS
sloCOMPILER_SetVecConstant(
    IN sloCOMPILER Compiler,
    IN slsNAME *ConstantVariable
    );

gceSTATUS
sloCOMPILER_GetVecConstant(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT Constant,
    OUT slsNAME **ConstantVariable
    );

gceSTATUS
sloCOMPILER_GetVecConstantLists(
    IN sloCOMPILER Compiler,
    IN sltELEMENT_TYPE ElementType,
    OUT slsDLINK_LIST **ConstantList
    );

gceSTATUS
slMakeImplicitConversionForOperand(
    IN sloCOMPILER Compiler,
    IN OUT sloIR_EXPR Operand,
    IN slsDATA_TYPE* DataTypeToMatch
    );

gceSTATUS
slMakeImplicitConversionForOperandPair(
    IN sloCOMPILER Compiler,
    IN OUT sloIR_EXPR LeftOperand,
    IN OUT sloIR_EXPR RightOperand,
    IN gctBOOL RightOnly
    );
#endif /* __gc_glsl_ir_h_ */
