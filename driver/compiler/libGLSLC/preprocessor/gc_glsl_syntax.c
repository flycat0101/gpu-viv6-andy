/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_glsl_preprocessor_int.h"

/*temporary -- will be removed when halti support is stable */
static int _slHaltiSupport = _ppd_HALTI_SUPPORT;

typedef struct _slsEXTENSION_INFO
{
   gctSTRING str;
   sleEXTENSION  flag;
   gctBOOL   enable;
   gctBOOL   disable;
   gctBOOL   warn;
   gctBOOL   require;
   gctUINT   minLanguageVersion;
   gctSTRING alias;
   gctBOOL   checked;
}
slsEXTENSION_INFO;

/* need to add extension macros to _PredefinedMacros[] when adding new extension,
** and Caps->extensions within gcInitGLSLCaps.
*/
static slsEXTENSION_INFO _DefinedExtensions[] =
{
    {"all",   slvEXTENSION_ALL, gcvFALSE, gcvTRUE, gcvTRUE, gcvFALSE, 0, gcvNULL},
    {"GL_KHR_blend_equation_advanced", slvEXTENSION_BLEND_EQUATION_ADVANCED, gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE, _SHADER_ES31_VERSION, gcvNULL},

    {"GL_OES_standard_derivatives", slvEXTENSION_STANDARD_DERIVATIVES, gcvTRUE, gcvTRUE, gcvFALSE, gcvTRUE, 0, gcvNULL},
    {"GL_OES_texture_3D", slvEXTENSION_TEXTURE_3D, gcvTRUE, gcvTRUE, gcvFALSE, gcvFALSE, 0, gcvNULL},
    {"GL_OES_EGL_image_external", slvEXTENSION_EGL_IMAGE_EXTERNAL, gcvTRUE, gcvTRUE, gcvFALSE, gcvTRUE, 0, gcvNULL},
    {"GL_OES_EGL_image_external_essl3", slvEXTENSION_EGL_IMAGE_EXTERNAL_ESSL3, gcvTRUE, gcvTRUE, gcvFALSE, gcvTRUE, 0, gcvNULL},
    {"GL_OES_texture_storage_multisample_2d_array", slvEXTENSION_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY, gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE, _SHADER_ES31_VERSION, gcvNULL},
    {"GL_OES_shader_image_atomic", slvEXTENSION_IMAGE_ATOMIC, gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE, _SHADER_ES31_VERSION, gcvNULL},
    /* sample shading extension. */
    {"GL_OES_sample_variables", slvEXTENSION_SAMPLE_VARIABLES, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_HALTI_VERSION, gcvNULL},
    {"GL_OES_shader_multisample_interpolation", slvEXTENSION_SHADER_MULTISAMPLE_INTERPOLATION, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_HALTI_VERSION, gcvNULL},

    {"GL_EXT_texture_array", slvEXTENSION_TEXTURE_ARRAY, gcvTRUE, gcvTRUE, gcvFALSE, gcvFALSE, 0, gcvNULL},
    {"GL_EXT_frag_depth", slvEXTENSION_FRAG_DEPTH, gcvTRUE, gcvTRUE, gcvFALSE, gcvFALSE, 0, gcvNULL},
    {"GL_EXT_shadow_samplers", slvEXTENSION_SHADOW_SAMPLER, gcvFALSE, gcvTRUE, gcvFALSE, gcvTRUE, 0, gcvNULL},
    /* cube array extension. */
    {"GL_EXT_texture_cube_map_array", slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, "GL_OES_texture_cube_map_array"},
    /* TS extension. */
    {"GL_EXT_tessellation_shader", slvEXTENSION_TESSELLATION_SHADER, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, "GL_OES_tessellation_shader"},
    {"GL_EXT_tessellation_point_size", slvEXTENSION_TESSELLATION_POINT_SIZE, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, "GL_OES_tessellation_point_size",},
    /* GS extension. */
    {"GL_EXT_geometry_shader", slvEXTENSION_EXT_GEOMETRY_SHADER, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, "GL_OES_geometry_shader"},
    {"GL_EXT_geometry_point_size", slvEXTENSION_EXT_GEOMETRY_POINT_SIZE, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, "GL_OES_geometry_point_size"},
    /* IO block extension. */
    {"GL_EXT_shader_io_blocks", slvEXTENSION_IO_BLOCKS, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, "GL_OES_shader_io_blocks"},
    /* GPU_Shader5 extension. */
    {"GL_EXT_gpu_shader5", slvEXTENSION_GPU_SHADER5, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, "GL_OES_gpu_shader5"},
    /* shader implicit conversions extension. */
    {"GL_EXT_shader_implicit_conversions", slvEXTENSION_EXT_SHADER_IMPLICIT_CONVERSIONS, gcvTRUE, gcvFALSE, gcvFALSE, gcvTRUE, _SHADER_ES31_VERSION, gcvNULL},
    /* texture buffer extension. */
    {"GL_EXT_texture_buffer", slvEXTENSION_EXT_TEXTURE_BUFFER, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, "GL_OES_texture_buffer"},
    /* primitive bounding box extension. */
    {"GL_EXT_primitive_bounding_box", slvEXTENSION_EXT_PRIMITIVE_BOUNDING_BOX, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, gcvNULL},
    /* frame buffer fetch extension. */
    {"GL_EXT_shader_framebuffer_fetch", slvEXTENSION_SHADER_FRAMEBUFFER_FETCH, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, 0, "EXT_shader_framebuffer_fetch"},
    /* ANDROID_extension_pack_es31a extension. */
    {"GL_ANDROID_extension_pack_es31a", slvEXTENSION_ANDROID_EXTENSION_PACK_ES31A, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, _SHADER_ES31_VERSION, gcvNULL},

    {"GL_VIV_asm", slvEXTENSION_VASM, gcvTRUE, gcvTRUE, gcvFALSE, gcvFALSE, 0, gcvNULL }, /* It is a internal option. */
};

#define __sldDefinedExtensionsCount (gcmSIZEOF(_DefinedExtensions) / gcmSIZEOF(slsEXTENSION_INFO))

static gceSTATUS _AddExtensionMacro(
    ppoPREPROCESSOR         PP,
    slsEXTENSION_INFO *     ExtInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    ExtInfo->require     =
    ExtInfo->enable      =
    ExtInfo->disable     =
    ExtInfo->warn        =
    gcoOS_StrStr(PP->extensionString, ExtInfo->str, gcvNULL);

    return status;
}

/*
** Init extension table:
** If a APP use a es30 context and write a es20 shader,
** extension "GL_EXT_shadow_samplers" should be enable.
*/
gceSTATUS ppoPREPROCESSOR_InitExtensionTable(ppoPREPROCESSOR PP)
{
    gctSIZE_T i;
    gceSTATUS status = gcvSTATUS_OK;

    for (i = 0; i < __sldDefinedExtensionsCount; i++)
    {
        switch (_DefinedExtensions[i].flag)
        {
        case slvEXTENSION_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;
        case slvEXTENSION_BLEND_EQUATION_ADVANCED:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_EXT_TEXTURE_BUFFER:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_SHADOW_SAMPLER:
            if (sloCOMPILER_GetClientApiVersion(PP->compiler) == gcvAPI_OPENGL_ES30)
            {
                _DefinedExtensions[i].enable = gcvTRUE;
            }
            else
            {
                _DefinedExtensions[i].enable = gcvFALSE;
            }
            break;

        case slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_IO_BLOCKS:
        case slvEXTENSION_GPU_SHADER5: /* FMA */
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_EXT_GEOMETRY_SHADER:
        case slvEXTENSION_EXT_GEOMETRY_POINT_SIZE:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_TESSELLATION_SHADER:
        case slvEXTENSION_TESSELLATION_POINT_SIZE:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_SAMPLE_VARIABLES:
        case slvEXTENSION_SHADER_MULTISAMPLE_INTERPOLATION:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_IMAGE_ATOMIC:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_EXT_PRIMITIVE_BOUNDING_BOX:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_SHADER_FRAMEBUFFER_FETCH:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        case slvEXTENSION_ANDROID_EXTENSION_PACK_ES31A:
            _AddExtensionMacro(PP, &_DefinedExtensions[i]);
            break;

        default:
            break;
        }
    }

    return status;
}


/*******************************************************************************
**
**    ppoPREPROCESSOR_PreprocessingFile
**
*/
gceSTATUS
ppoPREPROCESSOR_PreprocessingFile(
    IN ppoPREPROCESSOR      PP
    )
{
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    ppoPREPROCESSOR_InitExtensionTable(PP);

    status = ppoPREPROCESSOR_Group(PP, ppvIFSECTION_NONE);

    return status;
}

/*******************************************************************************
**
**    ppoPREPROCESSOR_Group
**
*/
gceSTATUS
ppoPREPROCESSOR_Group(
    IN ppoPREPROCESSOR      PP,
    IN ppeIFSECTION_TYPE    IfSectionType
    )
{
    ppoTOKEN                ntoken = gcvNULL;
    ppoTOKEN                ntoken2 = gcvNULL;
    gceSTATUS               status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    gcmHEADER_ARG("PP=0x%x, IfSectionType=%d", PP, IfSectionType);
    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    do
    {
        gcmONERROR(ppoPREPROCESSOR_PassEmptyLine(PP));

        gcmONERROR(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

        if(ntoken->type == ppvTokenType_EOF)
        {
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            gcmFOOTER();
            return status;
        }

        if (ntoken->poolString != PP->keyword->sharp)
        {
            PP->otherStatementHasAlreadyAppeared = gcvTRUE;
            /* We set this flag to TRUE only when we are in the valid area. */
            if (PP->doWeInValidArea)
                PP->nonpreprocessorStatementHasAlreadyAppeared = gcvTRUE;
        }

        /* preprocessor */
        if (ntoken->poolString == PP->keyword->sharp && ntoken->hideSet == gcvNULL)
        {
            /*#*/
            gcmONERROR(
                PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken2, gcvFALSE)
                );

            gcmASSERT(ntoken2->hideSet == gcvNULL);

            gcmONERROR(
                ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken2)
                );

            gcmONERROR(
                ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken)
                );

            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            ntoken = gcvNULL;

            if (ntoken2->poolString == PP->keyword->eof             ||
                ntoken2->poolString == PP->keyword->newline         ||
                ntoken2->poolString == PP->keyword->if_             ||
                ntoken2->poolString == PP->keyword->ifdef           ||
                ntoken2->poolString == PP->keyword->ifndef          ||
                ntoken2->poolString == PP->keyword->pragma          ||
                ntoken2->poolString == PP->keyword->error           ||
                ntoken2->poolString == PP->keyword->line            ||
                ntoken2->poolString == PP->keyword->version         ||
                ntoken2->poolString == PP->keyword->extension       ||
                ntoken2->poolString == PP->keyword->define          ||
                ntoken2->poolString == PP->keyword->undef)
            {
                gcmONERROR(
                    ppoTOKEN_Destroy(PP, ntoken2)
                    );
                ntoken2 = gcvNULL;

                gcmONERROR(
                    ppoPREPROCESSOR_GroupPart(PP)
                    );
            }
            else
            {
                if (ntoken2->poolString != PP->keyword->else_ &&
                    ntoken2->poolString != PP->keyword->elif  &&
                    ntoken2->poolString != PP->keyword->endif)
                {
                    if (PP->doWeInValidArea)
                    {
                        /*Other legal directive or inlegal directive*/
                        ppoPREPROCESSOR_Report(PP,
                            slvREPORT_ERROR,
                            "Not expected symbol here \"%s\"",
                            ntoken2->poolString
                            );

                        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken2));
                        ntoken2 = gcvNULL;

                        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
                    }
                    else
                    {
                        gcmONERROR(
                            ppoTOKEN_Destroy(PP, ntoken2)
                            );
                        ntoken2 = gcvNULL;

                        gcmONERROR(
                            ppoPREPROCESSOR_GroupPart(PP)
                            );
                    }
                }
                /*
                ** Within "#if"/"#elif", it allows "#else", "#elif" and "endif".
                ** Within "#else", it only allows "endif".
                */
                else
                {
                    if (IfSectionType == ppvIFSECTION_IF    ||
                        IfSectionType == ppvIFSECTION_ELSE  ||
                        IfSectionType == ppvIFSECTION_ELIF)
                    {
                        if (IfSectionType == ppvIFSECTION_ELSE &&
                            ntoken2->poolString != PP->keyword->endif)
                        {
                            /*Other legal directive or inlegal directive*/
                            ppoPREPROCESSOR_Report(PP,
                                slvREPORT_ERROR,
                                "Not expected symbol here \"%s\"",
                                ntoken2->poolString
                                );

                            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken2));
                            ntoken2 = gcvNULL;

                            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
                        }
                        /*this # should be part of another group.*/
                        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken2));

                        gcmFOOTER();
                        return status;
                    }
                    else
                    {
                        /*Other legal directive or inlegal directive*/
                        ppoPREPROCESSOR_Report(PP,
                            slvREPORT_ERROR,
                            "Not expected symbol here \"%s\"",
                            ntoken2->poolString
                            );

                        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken2));
                        ntoken2 = gcvNULL;

                        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
                    }
                }
            }
        }
        /* Text line or "#" generated by macro */
        else
        {
            /*Text Line*/
            gcmONERROR(
                ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken)
                );

            gcmONERROR(
                ppoTOKEN_Destroy(PP, ntoken)
                );
            ntoken = gcvNULL;

            gcmONERROR(
                ppoPREPROCESSOR_GroupPart(PP)
                );
        }
    }
    while(gcvTRUE);

OnError:
    if (ntoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
    }

    if (ntoken2 != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken2));
        ntoken2 = gcvNULL;
    }

    gcmFOOTER();
    return status;
}

/******************************************************************************\
**
**    ppoPREPROCESSOR_GroupPart
**
*/
gceSTATUS
ppoPREPROCESSOR_GroupPart(
    IN ppoPREPROCESSOR      PP
    )
{
    ppoTOKEN                ntoken = gcvNULL;
    ppoTOKEN                ntoken2 = gcvNULL;
    gceSTATUS               status = gcvSTATUS_OK;

    gcmHEADER_ARG("PP=0x%x", PP);
    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, gcvFALSE));

    if (ntoken->poolString == PP->keyword->sharp &&
        ntoken->hideSet == gcvNULL)
    {
        /*#*/
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken2, !ppvICareWhiteSpace));
        /*hideSet should be gcvNULL*/
        gcmASSERT(ntoken2->hideSet == gcvNULL);

        /*# EOF or NL*/
        if (ntoken2->poolString == PP->keyword->eof ||
            ntoken2->poolString == PP->keyword->newline)
        {
            /* Do nothing. */
        }
        /*# if/ifdef/ifndef*/
        else if (ntoken2->poolString == PP->keyword->if_    ||
                 ntoken2->poolString == PP->keyword->ifdef  ||
                 ntoken2->poolString == PP->keyword->ifndef)
        {
            PP->otherStatementHasAlreadyAppeared = gcvTRUE;
            gcmONERROR(ppoPREPROCESSOR_IfSection(PP, ntoken2));
        }
        /* #pragma\error\line\version\extension\define\undef */
        else if (ntoken2->poolString == PP->keyword->pragma     ||
                 ntoken2->poolString == PP->keyword->error      ||
                 ntoken2->poolString == PP->keyword->line       ||
                 ntoken2->poolString == PP->keyword->version    ||
                 ntoken2->poolString == PP->keyword->extension  ||
                 ntoken2->poolString == PP->keyword->define     ||
                 ntoken2->poolString == PP->keyword->undef)
        {
            if (gcvTRUE == PP->doWeInValidArea)
            {
                if (ntoken2->poolString == PP->keyword->version)
                {
                    if (gcvTRUE == PP->versionStatementHasAlreadyAppeared)
                    {
                        ppoPREPROCESSOR_Report(
                            PP,
                            slvREPORT_ERROR,
                            "The version statement should appear only once.");

                        gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
                    }
                    if (gcvTRUE == PP->otherStatementHasAlreadyAppeared)
                    {
                        ppoPREPROCESSOR_Report(
                            PP,
                            slvREPORT_ERROR,
                            "The version statement should appear "
                            "before any other statement except space and comment.");

                        gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
                    }
                    PP->versionStatementHasAlreadyAppeared = gcvTRUE;
                }
                else
                {
                    PP->otherStatementHasAlreadyAppeared = gcvTRUE;
                }
            }

            if(PP->outputTokenStreamHead != gcvNULL &&
               PP->outputTokenStreamHead->type != ppvTokenType_NUL &&
               PP->outputTokenStreamHead->type != ppvTokenType_NL &&
               PP->outputTokenStreamHead->type != ppvTokenType_EOF)
            {
                PP->outputTokenStreamHead->hasTrailingControl = gcvTRUE;
            }

            gcmONERROR(ppoPREPROCESSOR_ControlLine(PP, ntoken2));
        }
        else
        {
            gcmONERROR(ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken2));
            gcmONERROR(ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken));
            gcmONERROR(ppoPREPROCESSOR_TextLine(PP));
        }

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken2));
        ntoken = ntoken2 = gcvNULL;
    }
    else
    {
        /*Text Line*/
        gcmONERROR(ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken));
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;

        gcmONERROR(ppoPREPROCESSOR_TextLine(PP));
    }

    gcmFOOTER();
    return status;

OnError:
    if (ntoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
    }

    if (ntoken2 != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken2));
        ntoken2 = gcvNULL;
    }

    gcmFOOTER();
    return status;
}
/*******************************************************************************
**
**    ppoPREPROCESSOR_IfSection
**
**
*/
gceSTATUS
ppoPREPROCESSOR_IfSection(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             CurrentToken
    )
{
    ppoTOKEN        ntoken          = gcvNULL;
    ppoTOKEN        ntoken1         = gcvNULL;
    ppoTOKEN        ntoken2         = gcvNULL;
    ppoTOKEN        newt            = gcvNULL;
    gctINT          evalresult      = 0;
    gctBOOL         legalfounded    = 0;
    gctBOOL         pplegal_backup  = gcvFALSE;
    gctBOOL         matchElse       = gcvFALSE, matchEndIf = gcvFALSE;
    gceSTATUS       status          = gcvSTATUS_OK;

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    /*store PP's doWeInValidArea env vars*/
    gcmASSERT(
        CurrentToken->poolString == PP->keyword->if_      ||
        CurrentToken->poolString == PP->keyword->ifdef    ||
        CurrentToken->poolString == PP->keyword->ifndef);

    if (CurrentToken->poolString == PP->keyword->ifdef)
    {
        gcmONERROR(
            ppoTOKEN_Construct(PP, __FILE__, __LINE__, "Creat for ifdef.", &newt)
            );

        newt->hideSet        = gcvNULL;
        newt->poolString    = PP->keyword->defined;
        newt->type            = ppvTokenType_ID;

        gcmONERROR(
            ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), newt)
            );

        gcmONERROR(
            ppoTOKEN_Destroy(PP, newt)
            );
        newt = gcvNULL;
    }
    else if (CurrentToken->poolString == PP->keyword->ifndef)
    {
        /*push defined back.*/
        gcmONERROR(
            ppoTOKEN_Construct(
            PP,
            __FILE__,
            __LINE__,
            "Creat for ifndef, defined.",
            &newt
            )
            );

        newt->hideSet = gcvNULL;
        newt->poolString = PP->keyword->defined;
        newt->type = ppvTokenType_ID;

        gcmONERROR(
            ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, newt)
            );

        gcmONERROR(
            ppoTOKEN_Destroy(PP, newt)
            );
        newt = gcvNULL;

        /*push ! back.*/
        gcmONERROR(
            ppoTOKEN_Construct(
            PP,
            __FILE__,
            __LINE__,
            "Creat for ifndef,!.",
            &newt
            )
            );

        newt->hideSet = gcvNULL;
        newt->poolString = PP->keyword->lanti;
        newt->type = ppvTokenType_PUNC;

        gcmONERROR(
            ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, newt)
            );

        gcmONERROR(
            ppoTOKEN_Destroy(PP, newt)
            );
        newt = gcvNULL;
    }

    pplegal_backup = PP->doWeInValidArea;

    if (PP->doWeInValidArea)
    {
        gcmONERROR(
            ppoPREPROCESSOR_Eval(PP, PP->keyword->newline, 0, gcvFALSE, gcvNULL, &evalresult)
            );

        /*set enviroment variable doWeInValidArea.*/
        PP->doWeInValidArea = (PP->doWeInValidArea) && (!!evalresult);
        legalfounded =legalfounded || (gctBOOL)evalresult;

    }
    else
    {
        PP->doWeInValidArea = PP->doWeInValidArea;
        legalfounded = legalfounded;
    }

    gcmONERROR(ppoPREPROCESSOR_Group(PP, ppvIFSECTION_IF));

    /*set enviroment variable doWeInValidArea.*/
    PP->doWeInValidArea = pplegal_backup;

    /* match #else, #elif or #endif. */
    while (gcvTRUE)
    {
        gcmONERROR(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken1, !ppvICareWhiteSpace)
            );

        if (ntoken1->poolString != PP->keyword->sharp)
        {
            ppoPREPROCESSOR_Report(PP,
                    slvREPORT_INTERNAL_ERROR,
                    "This symbol should be #.");
            status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            gcmONERROR(status);
        }

        gcmONERROR(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken2, !ppvICareWhiteSpace)
            );

        if (ntoken2->poolString == PP->keyword->else_)
        {
            matchElse = gcvTRUE;
        }
        else if (ntoken2->poolString == PP->keyword->endif)
        {
            matchEndIf = gcvTRUE;
        }
        else if (ntoken2->poolString != PP->keyword->elif)
        {
            ppoPREPROCESSOR_Report(PP,
                    slvREPORT_INTERNAL_ERROR,
                    "This symbol should be #else, #elif or #endif.");
            status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            gcmONERROR(status);
        }

        ppoTOKEN_Destroy(PP, ntoken1);
        ppoTOKEN_Destroy(PP, ntoken2);
        ntoken1 = ntoken2 = gcvNULL;

        if (matchElse || matchEndIf)
        {
            break;
        }

        /* match #elif */
        pplegal_backup = PP->doWeInValidArea;

        if (PP->doWeInValidArea && !legalfounded)
        {
            gcmONERROR(
                ppoPREPROCESSOR_Eval(
                PP,
                PP->keyword->newline,
                0,
                gcvFALSE,
                gcvNULL,
                &evalresult
                )
                );

            PP->doWeInValidArea = PP->doWeInValidArea && (!legalfounded) && (!!evalresult);
            legalfounded =legalfounded || (gctBOOL)evalresult;

        }
        else
        {
            PP->doWeInValidArea = PP->doWeInValidArea && (!legalfounded);
            legalfounded = legalfounded;
        }

        /*do not care the result of the evaluation*/
        gcmONERROR(ppoPREPROCESSOR_Group(PP, ppvIFSECTION_ELIF));

        /*backroll doWeInValidArea env*/
        PP->doWeInValidArea = pplegal_backup;
    }/*while(gcvTRUE)*/

    if (matchEndIf)
    {
        /* Check if there are tokens after #endif. */
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        if (ntoken && ntoken->type != ppvTokenType_EOF && ntoken->type != ppvTokenType_NL)
        {
            status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            gcmONERROR(status);
        }
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
        return status;
    }
    /* match #else */
    else if (matchElse)
    {
        /* Check if there are tokens after #else. */
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        if (ntoken && ntoken->type != ppvTokenType_NL)
        {
            status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            gcmONERROR(status);
        }
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;

        /*set doWeInValidArea backup*/
        pplegal_backup = PP->doWeInValidArea;
        PP->doWeInValidArea = PP->doWeInValidArea && (!legalfounded);

        gcmONERROR(ppoPREPROCESSOR_Group(PP, ppvIFSECTION_ELSE));

        /*backroll doWeInValidArea env*/
        PP->doWeInValidArea = pplegal_backup;

        /* must follow #endif. */
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken1, !ppvICareWhiteSpace));
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken2, !ppvICareWhiteSpace));

        if (ntoken->poolString != PP->keyword->sharp ||
            ntoken1->poolString != PP->keyword->endif ||
            ntoken2 == gcvNULL || (ntoken2->type != ppvTokenType_EOF && ntoken2->type != ppvTokenType_NL))
        {
            ppoPREPROCESSOR_Report(PP,
                    slvREPORT_INTERNAL_ERROR,
                    "This symbol should be #endif.");
            status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            gcmONERROR(status);
        }
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken1));
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken2));
        ntoken = ntoken1 = ntoken2 = gcvNULL;
    }

    return gcvSTATUS_OK;

OnError:
    if (ntoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
    }
    if (ntoken1 != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken1));
        ntoken1 = gcvNULL;
    }
    if (ntoken2 != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken2));
        ntoken2 = gcvNULL;
    }
    if (newt != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, newt));
        newt = gcvNULL;
    }
    return status;
}

/******************************************************************************\
Defined
Parse out the id in or not in ().
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_Defined(ppoPREPROCESSOR PP, gctSTRING* Return)
{
    ppoTOKEN    ntoken        = gcvNULL;

    gceSTATUS    status        = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    gcmONERROR(
        PP->inputStream->GetToken(
        PP,
        &(PP->inputStream),
        &ntoken,
        !ppvICareWhiteSpace)
        );

    if(ntoken->poolString == PP->keyword->lpara)
    {

        gcmONERROR(
            ppoTOKEN_Destroy(PP, ntoken)
            );
        ntoken = gcvNULL;

        gcmONERROR(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

        if(ntoken->type != ppvTokenType_ID){

            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
                "Expect and id after the defined(.");

            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));

            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

        }

        *Return = ntoken->poolString;

        gcmONERROR(
            ppoTOKEN_Destroy(PP, ntoken)
            );
        ntoken = gcvNULL;

        gcmONERROR(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

        if(ntoken->poolString != PP->keyword->rpara) {

            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
                "Expect a ) after defined(id .");

            gcmONERROR(
                ppoTOKEN_Destroy(PP, ntoken)
                );

            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

        }

        gcmONERROR(
            ppoTOKEN_Destroy(PP, ntoken)
            );

    }
    else
    {

        if(ntoken->type != ppvTokenType_ID){

            gcmONERROR(
                ppoTOKEN_Destroy(PP, ntoken)
                );

            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

        }

        *Return = ntoken->poolString;

        gcmONERROR(
            ppoTOKEN_Destroy(PP, ntoken)
            );

    }

    return gcvSTATUS_OK;

OnError:
    if (ntoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
    }
    return status;
}


/******************************************************************************\
Args Macro Expand
In this function, we treat the HeadIn as a Input Stream, we inputStream doWeInValidArea to do
Macro Expanation use current Macro Context.
And the expanded token stream inputStream store in HeadOut and EndOut.
If the HeadIn inputStream gcvNULL, then then HeadOut and EndOut will be gcvNULL, too.
The Outs counld be gcvNULL, when the expanation inputStream just NOTHING.

WARINIG!!!
Every node in the HeadIn should not be released outside.
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_ArgsMacroExpand_AddTokenToOut(
                                ppoPREPROCESSOR PP,
                                ppoTOKEN    InHead,
                                ppoTOKEN    InEnd,
                                ppoTOKEN    *OutHead,
                                ppoTOKEN    *OutEnd)
{
    if(*OutHead == gcvNULL)
    {
        gcmASSERT(*OutEnd == gcvNULL);

        *OutHead = InHead;
        *OutEnd = InEnd;

        InHead->inputStream.base.node.next = gcvNULL;
        InEnd->inputStream.base.node.prev = gcvNULL;
    }
    else
    {
        gcmASSERT(*OutEnd != gcvNULL);

        (*OutEnd)->inputStream.base.node.prev = (void*)InHead;

        InHead->inputStream.base.node.next = (void*)(*OutEnd);
        InEnd->inputStream.base.node.prev = gcvNULL;

        (*OutEnd) = InEnd;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
ppoPREPROCESSOR_ArgsMacroExpand_LinkBackToIS(
                                ppoPREPROCESSOR PP,
                                ppoTOKEN    *IS,
                                ppoTOKEN    *InHead,
                                ppoTOKEN    *InEnd)
{
    if((*IS) == gcvNULL)
    {
        *IS = *InHead;

        (*InEnd)->inputStream.base.node.prev = gcvNULL;
    }
    else
    {
        (*IS)->inputStream.base.node.next = (void*)(*InEnd);
        (*InEnd)->inputStream.base.node.prev = (void*)(*IS);
        (*InHead)->inputStream.base.node.next = gcvNULL;
        (*IS) = (*InHead);
    }

    return gcvSTATUS_OK;
}
gceSTATUS
ppoPREPROCESSOR_ArgsMacroExpand(
                                ppoPREPROCESSOR PP,
                                ppoTOKEN    *IS,
                                ppoTOKEN    *OutHead,
                                ppoTOKEN    *OutEnd)
{
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    *OutHead = gcvNULL;
    *OutEnd  = gcvNULL;

    if( *IS == gcvNULL )    return gcvSTATUS_OK;

    while((*IS))
    {
        /*
        more input-token, more parsing-action.
        which is diff with text line.
        */

        if ((*IS)->type == ppvTokenType_ID)
        {
            /*
            do macro expand
            */
            gctBOOL any_expanation_happened = gcvFALSE;

            ppoTOKEN expanded_id_head = gcvNULL;
            ppoTOKEN expanded_id_end = gcvNULL;

            status = ppoPREPROCESSOR_MacroExpand(
                    PP,
                    (ppoINPUT_STREAM*)IS,
                    &expanded_id_head,
                    &expanded_id_end,
                    &any_expanation_happened);
            if(status != gcvSTATUS_OK) return status;

            gcmASSERT(
                (expanded_id_head == gcvNULL && expanded_id_end == gcvNULL)
                ||
                (expanded_id_head != gcvNULL && expanded_id_end != gcvNULL));

            if (expanded_id_head != gcvNULL)
            {
                if (expanded_id_head->poolString == PP->keyword->_line_)
                {
                    char    numberbuffer [128];
                    gctUINT offset = 0;

                    gcoOS_MemFill(numberbuffer, 0, 128);

                    gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d", expanded_id_head->srcFileLine);

                    sloCOMPILER_AllocatePoolString(PP->compiler,
                        numberbuffer,
                        &(expanded_id_head->poolString));
                }
            }

            if (any_expanation_happened == gcvTRUE)
            {
                if (expanded_id_head != gcvNULL)
                {
                    /*link expanded_id back to IS*/
                    status = ppoPREPROCESSOR_ArgsMacroExpand_LinkBackToIS(
                        PP,
                        IS,
                        &expanded_id_head,
                        &expanded_id_end);
                    if(status != gcvSTATUS_OK) return status;
                }
                /*else, the id expand to nothing.*/
            }
            else
            {
                if(expanded_id_head != gcvNULL)
                {
                    /*
                    add to *Out*
                    */
                    status = ppoPREPROCESSOR_ArgsMacroExpand_AddTokenToOut(
                                PP,
                                expanded_id_head,
                                expanded_id_end,
                                OutHead,
                                OutEnd);
                    if(status != gcvSTATUS_OK) return status;
                }
                else
                {
                    gcmASSERT(0);
                }
            }
        }
        else
        {
            /*
            not id, just add to *Out*
            */
            ppoTOKEN ntoken = (*IS);

            *IS = (ppoTOKEN)((*IS)->inputStream.base.node.prev);

            status = ppoPREPROCESSOR_ArgsMacroExpand_AddTokenToOut(
                PP,
                ntoken,
                ntoken,
                OutHead,
                OutEnd);
            if(status != gcvSTATUS_OK) return status;
        }
    }/*while((*IS))*/

    return gcvSTATUS_OK;
}

gceSTATUS
ppoPREPROCESSOR_TextLine_Handle_FILE_LINE_VERSION(
    ppoPREPROCESSOR PP,
    gctSTRING What
    )
{
    ppoTOKEN newtoken = gcvNULL;

    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    char* creat_str = gcvNULL;

    char    numberbuffer [128];

    gctUINT    offset = 0;

    gcoOS_MemFill(numberbuffer, 0, 128);

    if (What == PP->keyword->_file_)
    {

        creat_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute __FILE__";
        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d\0", PP->currentSourceFileStringNumber);

    }
    else if (What == PP->keyword->_line_)
    {
        creat_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute __LINE__";
        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d\0", PP->currentSourceFileLineNumber);


    }
    else if (What == PP->keyword->_version_)
    {
        creat_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute __VERSION__";
        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d\0", PP->version);
    }
    else if(What == PP->keyword->gl_es)
    {
        creat_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute GL_ES";
        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d\0", 1);
    }
    else
    {
        gcmASSERT(0);
    }

    gcmONERROR(
        ppoTOKEN_Construct(
        PP,
        __FILE__,
        __LINE__,
        creat_str,
        &newtoken));

    gcmONERROR(
        sloCOMPILER_AllocatePoolString(
        PP->compiler,
        numberbuffer,
        &(newtoken->poolString))
        );

    newtoken->hideSet    = gcvNULL;

    newtoken->type        = ppvTokenType_INT;

    /*newtoken->integer    = PP->currentSourceFileStringNumber;*/

    gcmONERROR(
        ppoPREPROCESSOR_AddToOutputStreamOfPP(PP, newtoken)
        );

    gcmONERROR(
        ppoTOKEN_Destroy(PP, newtoken)
        );

    return gcvSTATUS_OK;

OnError:
    if (newtoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, newtoken));
    }
    return status;

}
/*******************************************************************************
**
**    ppoPREPROCESSOR_TextLine
**
*/

gceSTATUS
ppoPREPROCESSOR_TextLine_AddToInputAfterMacroExpand(
    ppoPREPROCESSOR PP,
    ppoTOKEN expanded_id_head,
    ppoTOKEN expanded_id_end
    )
{
    if(expanded_id_head)
    {
        gcmASSERT(expanded_id_end != gcvNULL);

        PP->inputStream->base.node.next = (void*)expanded_id_end;
        expanded_id_end->inputStream.base.node.prev = (void*)PP->inputStream;

        PP->inputStream = (void*)expanded_id_head;
        expanded_id_head->inputStream.base.node.next = gcvNULL;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
ppoPREPROCESSOR_TextLine_CheckSelfContainAndIsMacroOrNot(
    ppoPREPROCESSOR PP,
    ppoTOKEN  ThisToken,
    gctBOOL    *TokenIsSelfContain,
    ppoMACRO_SYMBOL *TheMacroSymbolOfThisId)
{
    gceSTATUS status;

    gcmONERROR(
        ppoHIDE_SET_LIST_ContainSelf(PP, ThisToken, TokenIsSelfContain)
        );

    gcmONERROR(
        ppoMACRO_MANAGER_GetMacroSymbol(
        PP,
        PP->macroManager,
        ThisToken->poolString,
        TheMacroSymbolOfThisId)
        );

    return gcvSTATUS_OK;

OnError:
    return status;

}
gceSTATUS
ppoPREPROCESSOR_TextLine(
    IN ppoPREPROCESSOR      PP
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    ppoTOKEN                ntoken = gcvNULL;

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    if (PP->doWeInValidArea == gcvFALSE)
    {
        return ppoPREPROCESSOR_ToEOL(PP);
    }

    gcmONERROR(
        ppoPREPROCESSOR_PassEmptyLine(PP)
        );

    gcmONERROR(
        PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
        );

    /*just check The first token of a line.*/
    while (
        ntoken->poolString != PP->keyword->eof
        &&
        !(ntoken->poolString == PP->keyword->sharp && ntoken->hideSet == gcvNULL))
    {
        /*check next token*/
        while (
            ntoken->poolString != PP->keyword->eof
            &&
            ntoken->poolString != PP->keyword->newline)
        {
            /*pre-defined macro, should not be editable.*/
            if (ntoken->poolString == PP->keyword->_file_       ||
                ntoken->poolString == PP->keyword->_line_       ||
                ntoken->poolString == PP->keyword->_version_    ||
                ntoken->poolString == PP->keyword->gl_es)
            {
                gcmONERROR(
                    ppoPREPROCESSOR_TextLine_Handle_FILE_LINE_VERSION(PP, ntoken->poolString)
                    );

                gcmONERROR(
                    ppoTOKEN_Destroy(PP, ntoken)
                    );
                ntoken = gcvNULL;
            }
            else if (ntoken->type == ppvTokenType_ID)
            {
                /*Check the hide set of this ID.*/

                gctBOOL token_is_self_contain = gcvFALSE;

                ppoMACRO_SYMBOL the_macro_symbol_of_this_id = gcvNULL;

                gcmONERROR(ppoPREPROCESSOR_TextLine_CheckSelfContainAndIsMacroOrNot(
                    PP,
                    ntoken,
                    &token_is_self_contain,
                    &the_macro_symbol_of_this_id));

                if (token_is_self_contain || the_macro_symbol_of_this_id == gcvNULL)
                {
                    gcmONERROR(ppoPREPROCESSOR_AddToOutputStreamOfPP(PP, ntoken));

                    gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
                    ntoken = gcvNULL;
                }
                else
                {
                    ppoTOKEN head = gcvNULL;
                    ppoTOKEN end  = gcvNULL;
                    gctBOOL any_expanation_happened = gcvFALSE;

                    gcmONERROR(ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken));

                    gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
                    ntoken = gcvNULL;

                    gcmONERROR(ppoPREPROCESSOR_MacroExpand(
                        PP,
                        &(PP->inputStream),
                        &head,
                        &end,
                        &any_expanation_happened));

                    gcmASSERT(
                        (head == gcvNULL && end == gcvNULL)
                        ||
                        (head != gcvNULL && end != gcvNULL));

                    if(gcvTRUE == any_expanation_happened)
                    {
                        gcmONERROR(ppoPREPROCESSOR_TextLine_AddToInputAfterMacroExpand(
                            PP,
                            head,
                            end));
                    }
                    else
                    {
                        gcmASSERT(head == end);

                        if (head != gcvNULL)
                        {
                            gcmONERROR(ppoPREPROCESSOR_AddToOutputStreamOfPP(
                                PP,
                                head));
                        }
                    }
                }/*if(token_is_self_contain || the_macro_symbol_of_this_id == gcvNULL)*/
            }/*else if(ntoken->type == ppvTokenType_ID)*/
            else
            {
                /*Not ID*/
                gcmONERROR( ppoPREPROCESSOR_AddToOutputStreamOfPP(PP, ntoken) );

                gcmONERROR( ppoTOKEN_Destroy(PP, ntoken) );
                ntoken = gcvNULL;
            }
            gcmONERROR(
                PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
                );
        }/*internal while*/

        gcmONERROR(
            ppoTOKEN_Destroy(PP, ntoken)
            );
        ntoken = gcvNULL;

        gcmONERROR(
            ppoPREPROCESSOR_PassEmptyLine(PP)
            );

        gcmONERROR(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );
    }/*extenal while*/

    gcmONERROR(
        ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken)
        );

    gcmONERROR(
        ppoTOKEN_Destroy(PP, ntoken)
        );

    return gcvSTATUS_OK;

OnError:
    if (ntoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;

    }
    return status;
}

/*control_line***************************************************************************/
gceSTATUS
ppoPREPROCESSOR_ControlLine(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             CurrentToken
    )
{
    if(!PP->doWeInValidArea)
    {
        return ppoPREPROCESSOR_ToEOL(PP);
    }

    if (CurrentToken ->poolString == PP->keyword->define)
    {
        return ppoPREPROCESSOR_Define(PP);
    }
    else if (CurrentToken->poolString == PP->keyword->undef)
    {
        return ppoPREPROCESSOR_Undef(PP);
    }
    else if (CurrentToken->poolString == PP->keyword->error)
    {
        return ppoPREPROCESSOR_Error(PP);
    }
    else if (CurrentToken->poolString == PP->keyword->pragma)
    {
        return ppoPREPROCESSOR_Pragma(PP);
    }
    else if (CurrentToken->poolString == PP->keyword->extension)
    {
        return ppoPREPROCESSOR_Extension(PP);
    }
    else if (CurrentToken->poolString == PP->keyword->version)
    {
        return ppoPREPROCESSOR_Version(PP);
    }
    else if (CurrentToken->poolString == PP->keyword->line)
    {
        return ppoPREPROCESSOR_Line(PP);
    }
    else
    {
        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    }
}

typedef struct _slsVERSION_INFO
{
    gctUINT     langVersion;
    gctBOOL     isGLVersion;
    gctSTRING   postfixString;
    gctBOOL     isSupport;
}
slsVERSION_INFO;

slsVERSION_INFO _DefinedVersionInfo[] =
{
    /* GL version. */
    {110,   gcvTRUE,    gcvNULL,    gcvFALSE},
    {120,   gcvTRUE,    gcvNULL,    gcvFALSE},
    /* GLES version. */
    {100,   gcvFALSE,   gcvNULL,    gcvFALSE},
    {300,   gcvFALSE,   "es",       gcvFALSE},
    {310,   gcvFALSE,   "es",       gcvFALSE},
    {320,   gcvFALSE,   "es",       gcvFALSE},
};

#define __sldDefinedVersionInfoCount (gcmSIZEOF(_DefinedVersionInfo) / gcmSIZEOF(slsVERSION_INFO))

static gceSTATUS _InitializeVersionInfoTable(ppoPREPROCESSOR PP, sleSHADER_TYPE ShaderType)
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctBOOL     isClientVersion = (sloCOMPILER_GetClientApiVersion(PP->compiler) == gcvAPI_OPENGL);
    gctBOOL     isFromLibShader = (ShaderType == slvSHADER_TYPE_LIBRARY);
    gctUINT     i;

    for (i = 0; i < __sldDefinedVersionInfoCount; i++)
    {
        _DefinedVersionInfo[i].isSupport = gcvFALSE;

        /* So far GL client can only support GLSL. */
        if (_DefinedVersionInfo[i].isGLVersion)
        {
            _DefinedVersionInfo[i].isSupport = isClientVersion;
        }
        else
        {
            _DefinedVersionInfo[i].isSupport = !isClientVersion || isFromLibShader;

            if (_DefinedVersionInfo[i].langVersion > 100)
            {
                _DefinedVersionInfo[i].isSupport &= _slHaltiSupport;
            }
        }
    }

    return status;
}

/******************************************************************************\
Version
OpenGL version------GLSL Version
2.0                 110
2.1                 120
3.0                 130
3.1                 140
3.2                 150
3.3                 330
4.0                 400
4.1                 410
4.2                 420
4.3                 430

OpenGLES version----GLSL Version
2.0                 100
3.0                 300
3.1                 310
3.2                 320
\******************************************************************************/
gceSTATUS ppoPREPROCESSOR_Version(ppoPREPROCESSOR PP)
{
    gctBOOL             doWeInValidArea = PP->doWeInValidArea;
    gceSTATUS           status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    gctUINT32           langVersion = 0;
    slsVERSION_INFO*    versionInfo = gcvNULL;
    gctUINT             i;
    sleSHADER_TYPE      shaderType;

    sloCOMPILER_GetShaderType(PP->compiler, &shaderType);

    /* Initialize the versionInfo table first. */
    _InitializeVersionInfoTable(PP, shaderType);

    if (doWeInValidArea == gcvTRUE)
    {
        ppoTOKEN    ntoken = gcvNULL;
        ppoTOKEN    nextToken = gcvNULL;

        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        if (ntoken->type != ppvTokenType_INT)
        {
            ppoPREPROCESSOR_Report(PP,
                                   slvREPORT_ERROR,
                                   "Expect a number afer the #version.");
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));

            status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            return status;
        }

        /* Get the version integer number first. */
        gcmONERROR(gcoOS_StrToInt(ntoken->poolString, (gctINT *)&langVersion));

        /* Find the match versionInfo.*/
        for (i = 0; i < __sldDefinedVersionInfoCount; i++)
        {
            if (_DefinedVersionInfo[i].langVersion == langVersion)
            {
                versionInfo = &_DefinedVersionInfo[i];
                break;
            }
        }

        /* No matched versionInfo found or can't support this version. */
        if (versionInfo == gcvNULL || !versionInfo->isSupport)
        {
            ppoPREPROCESSOR_Report(PP,
                                   slvREPORT_ERROR,
                                   "Can not support version %d.",
                                   langVersion);

            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }

        /* Check if we need a postfix token after the version number. */
        if (versionInfo->postfixString != gcvNULL)
        {
            gcmONERROR(PP->inputStream->GetToken(PP,
                                                 &(PP->inputStream),
                                                 &nextToken,
                                                 !ppvICareWhiteSpace));

            if (!gcmIS_SUCCESS(gcoOS_StrCmp(nextToken->poolString, versionInfo->postfixString)))
            {
                ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
                                       "Expect '%s' afer the #version directive.",
                                       versionInfo->postfixString);

                gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
                gcmONERROR(ppoTOKEN_Destroy(PP, nextToken));
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }
            gcmONERROR(ppoTOKEN_Destroy(PP, nextToken));
        }

        gcmONERROR(PP->inputStream->GetToken(PP,
                                             &(PP->inputStream),
                                             &nextToken,
                                             !ppvICareWhiteSpace));

        if (nextToken && nextToken->poolString != PP->keyword->newline)
        {
            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
                                   "The #version directive must be followed by a newline");

            gcmONERROR(ppoTOKEN_Destroy(PP, nextToken));
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }

        /* Set the version. */
        PP->version = langVersion;
        sloCOMPILER_SetLanguageVersion(PP->compiler, langVersion);

        gcmONERROR(ppoTOKEN_Destroy(PP, nextToken));
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        return status;
    }
    else
    {
        status = ppoPREPROCESSOR_ToEOL(PP);
        return status;
    }

OnError:
    return status;
}


/******************************************************************************\
**
**    ppoPREPROCESSOR_EvalLineToken(ppoPREPROCESSOR PP,
**                                    gctINT MemberCount,
**                                    ppoTOKEN *Replacement)
**
*/

gctINT
ppoPREPROCESSOR_EvalLineToken(
   ppoPREPROCESSOR PP,
   gctINT MemberCount,
   ppoTOKEN *Replacement
)
{
    gceSTATUS status;
    ppoTOKEN nextToken = gcvNULL;
    ppoTOKEN temp = gcvNULL;
    gctINT count = 0;

    gcmASSERT(Replacement);

    status = ppoPREPROCESSOR_Eval_GetToken(PP,
                                           &nextToken,
                                           !ppvICareWhiteSpace);
    if(gcmIS_ERROR(status) || nextToken == gcvNULL) {
       Replacement[0] = gcvNULL;
       return 0;
    }

   while(nextToken) {
       switch(nextToken->type) {
       case ppvTokenType_NUL:
          temp = nextToken;
          break;

       default:
          if(count == MemberCount) {
             temp = nextToken;
             count++;
          }
          else  {
             Replacement[count++] = nextToken;
             temp = gcvNULL;
          }
          break;
       }

       nextToken = (ppoTOKEN)nextToken->inputStream.base.node.prev;
       if(temp) {
          ppoTOKEN_Destroy(PP, temp);
       }
   }

   return count;
}

/******************************************************************************\
**
**    ppoPREPROCESSOR_Line
**
*/

gceSTATUS ppoPREPROCESSOR_Line(ppoPREPROCESSOR PP)
{
    gceSTATUS    status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    gctINT       line    = 0;
    gctINT       string    = 0;
    gctBOOL      meetStringNum = gcvFALSE;

    gcmASSERT(PP);

    line = PP->currentSourceFileLineNumber;
    string = PP->currentSourceFileStringNumber;

    /* #line must have, after macro substitution, one of the following two forms :
    ** 1) #line line
    ** 2) #line line source-string-number
    ** where line and source-string-number are constant integral expressions.
    */
    if (PP->doWeInValidArea)
    {
        status = ppoPREPROCESSOR_Eval(PP, PP->keyword->newline, 0, gcvTRUE, &meetStringNum, &line);

        if (!gcmIS_SUCCESS(status))
        {
            ppoPREPROCESSOR_Report(PP,
                                   slvREPORT_ERROR,
                                   "Expect integer-line-number after #line.");

            gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
        }

        if (line < 0)
        {
            ppoPREPROCESSOR_Report(PP,
                                   slvREPORT_ERROR,
                                   "Expect positive integer-line-number after #line.");
            gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
        }
    }

    if (meetStringNum)
    {
        status = ppoPREPROCESSOR_Eval(PP, PP->keyword->newline, 0, gcvFALSE, gcvNULL, &string);

        if (!gcmIS_SUCCESS(status))
        {
            ppoPREPROCESSOR_Report(PP,
                                   slvREPORT_ERROR,
                                   "Expect integer-string-number after #line.");

            gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
        }

        if (string < 0)
        {
            ppoPREPROCESSOR_Report(PP,
                                   slvREPORT_ERROR,
                                   "Expect positive integer-string-number after #line.");
            gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
        }
    }

    gcmONERROR(ppoPREPROCESSOR_ToEOL(PP));

    PP->currentSourceFileStringNumber = string;
    /* according to  GLSL_ES_Specification, don't need to line + 1*/
    PP->currentSourceFileLineNumber = line;

    return gcvSTATUS_OK;

OnError:
    return status;
}

/******************************************************************************\
Extension
\******************************************************************************/
gceSTATUS ppoPREPROCESSOR_Extension(ppoPREPROCESSOR PP)
{
    gctBOOL doWeInValidArea = PP->doWeInValidArea;
    gceSTATUS   status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    ppoTOKEN    ntoken = gcvNULL;
    ppoTOKEN    feature = gcvNULL;
    ppoTOKEN    behavior = gcvNULL;

    gcmHEADER_ARG("PP=0x%x", PP);

    if(doWeInValidArea == gcvTRUE)
    {
        gceSTATUS    status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        gctUINT  i;
        gctINT    extensionIndex = -1;


        if(PP->nonpreprocessorStatementHasAlreadyAppeared)
        {
            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR, "Extension directives must occur before any non-preprrocessor tokens.");
            ppoPREPROCESSOR_ToEOL(PP);
            gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
        }

        status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);ppmCheckOK();

        if(ntoken->type != ppvTokenType_ID)
        {
            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR, "Expect extension name here.");
            gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
        }

        feature = ntoken;


        for(i=0; i < __sldDefinedExtensionsCount; i++) {

           if ((gcmIS_SUCCESS(gcoOS_StrCmp(feature->poolString, _DefinedExtensions[i].str)))
            || (_DefinedExtensions[i].alias
                && (gcmIS_SUCCESS(gcoOS_StrCmp(feature->poolString, _DefinedExtensions[i].alias)))
               )
              )
           {
              extensionIndex = i;
              break;
            }
        }

          if(extensionIndex < 0) {
           ppoPREPROCESSOR_Report(PP,slvREPORT_WARN,"Extension : %s is not provided by this compiler.", feature->poolString);
        }

        ntoken = gcvNULL;
        status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);ppmCheckOK();

        if(ntoken->poolString != PP->keyword->colon)
        {
            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR, "Expect : here.");
            gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
        }

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;

        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        if( ntoken->poolString != PP->keyword->require    &&
            ntoken->poolString != PP->keyword->enable    &&
            ntoken->poolString != PP->keyword->warn        &&
            ntoken->poolString != PP->keyword->disable)
        {
            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
                "Expect 'require' or 'enable' or 'warn' or 'disable' here.");
            gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
        }

        behavior = ntoken;
        ntoken = gcvNULL;

        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        if(ntoken->poolString != PP->keyword->newline && ntoken->poolString != PP->keyword->eof)
        {
            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,"Expect 'New Line' or 'End of File' here.");

            gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
        }


        /*sematic checking*/

        /*require*/
        if(behavior->poolString == PP->keyword->require)
        {
            if(feature->poolString == PP->keyword->all)
            {
                ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,"Expect all's behavior should be warn or disable.");
                gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
            }
            else if(extensionIndex < 0 ||
                    _DefinedExtensions[extensionIndex].require == gcvFALSE ||
                    sloCOMPILER_GetLanguageVersion(PP->compiler) < _DefinedExtensions[extensionIndex].minLanguageVersion)
            {
                ppoPREPROCESSOR_Report(PP,
                                       slvREPORT_ERROR,"Extension : %s is not provided by this compiler.",
                                       feature->poolString);
                gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
            }
            else
            {
                gcmVERIFY_OK(sloCOMPILER_EnableExtension(PP->compiler,
                                                         _DefinedExtensions[extensionIndex].flag,
                                                         gcvTRUE));
            }
        }

        /*enable*/
        if( behavior->poolString == PP->keyword->enable)
        {
            if(feature->poolString == PP->keyword->all)
            {
                ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,"Expect all's behavior should be warn or disable.");
                gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);

            }
            else if(extensionIndex < 0 ||
                    _DefinedExtensions[extensionIndex].enable == gcvFALSE ||
                    sloCOMPILER_GetLanguageVersion(PP->compiler) < _DefinedExtensions[extensionIndex].minLanguageVersion)
            {
                ppoPREPROCESSOR_Report(PP,
                                       slvREPORT_WARN,"Extension : %s is not provided by this compiler.",
                                       feature->poolString);
            }
            else
            {
                gcmVERIFY_OK(sloCOMPILER_EnableExtension(PP->compiler,
                                                         _DefinedExtensions[extensionIndex].flag,
                                                         gcvTRUE));
            }
        }

        /*warn*/
        if( behavior->poolString == PP->keyword->warn)
        {
            if(extensionIndex < 0 ||
               _DefinedExtensions[extensionIndex].warn == gcvFALSE ||
               sloCOMPILER_GetLanguageVersion(PP->compiler) < _DefinedExtensions[extensionIndex].minLanguageVersion)
            {
                ppoPREPROCESSOR_Report(PP,
                                       slvREPORT_WARN,"Extension : %s is not provided by this compiler.",
                                       feature->poolString);
            }
            else
            {
                gcmVERIFY_OK(sloCOMPILER_EnableExtension(PP->compiler,
                                                         _DefinedExtensions[extensionIndex].flag,
                                                         gcvTRUE));
                ppoPREPROCESSOR_Report(PP,
                                       slvREPORT_WARN,"Extension : %s is used.",
                                       feature->poolString);
            }
        }

        /*disable*/
        if( behavior->poolString == PP->keyword->disable)
        {
            if(extensionIndex < 0 ||
                    _DefinedExtensions[extensionIndex].disable == gcvFALSE ||
                    sloCOMPILER_GetLanguageVersion(PP->compiler) < _DefinedExtensions[extensionIndex].minLanguageVersion)
            {
                ppoPREPROCESSOR_Report(PP,
                                       slvREPORT_WARN,"Extension : %s is not provided by this compiler.",
                                       feature->poolString);
            }
            else
            {
                gcmVERIFY_OK(sloCOMPILER_EnableExtension(PP->compiler,
                                                         _DefinedExtensions[extensionIndex].flag,
                                                         gcvFALSE));
            }
        }


        gcmONERROR(
            ppoTOKEN_Destroy(PP, feature)
            );
        feature = gcvNULL;

        gcmONERROR(
            ppoTOKEN_Destroy(PP, behavior)
            );
        behavior = gcvNULL;

        gcmONERROR(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    else
    {
        status = ppoPREPROCESSOR_ToEOL(PP);
        gcmFOOTER();
        return status;
    }

OnError:
    if (feature != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, feature));
        feature = gcvNULL;
    }
    if (behavior != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, behavior));
        behavior = gcvNULL;
    }
    if (ntoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
    }
    gcmFOOTER();
    return status;
}
/******************************************************************************\
Error
\******************************************************************************/
gceSTATUS ppoPREPROCESSOR_Error(ppoPREPROCESSOR PP)
{
    gctBOOL doWeInValidArea = PP->doWeInValidArea;
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    if(doWeInValidArea == gcvTRUE)
    {
        ppoTOKEN    ntoken = gcvNULL;

        ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
            "Error(str:%d,lin:%d): "
            "Meet #error with:",
            PP->currentSourceFileStringNumber,
            PP->currentSourceFileLineNumber);

        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        while(ntoken->poolString != PP->keyword->newline && ntoken->poolString != PP->keyword->eof)
        {
            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR, "%s ", ntoken->poolString);

            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));

            gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));
        }

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));

        ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR, "");
        status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        return status;
    }
    else
    {
        status = ppoPREPROCESSOR_ToEOL(PP);
        return status;
    }

OnError:
    return status;
}
/******************************************************************************\
Pragma:
only check the validity for debug(on/off), optimize(on/off) and STDGL,
ignore all tokens that can't be recongnized.
\******************************************************************************/
gceSTATUS ppoPREPROCESSOR_Pragma(ppoPREPROCESSOR PP)
{
    ppoTOKEN    ntoken  = gcvNULL;
    gceSTATUS   status  = gcvSTATUS_OK;
    gctBOOL     prevValidStatus = PP->doWeInValidArea;

    /* Set current area as invalid so we can accept invalid tokens. */
    PP->doWeInValidArea = gcvFALSE;

    status = (PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

    /* Ignore a unrecognized preprocessor token. */
    if (status != gcvSTATUS_OK)
    {
        ppoPREPROCESSOR_ToEOL(PP);
    }
    else if (ntoken->poolString == PP->keyword->debug ||
             ntoken->poolString == PP->keyword->optimize)
    {
        gctBOOL isDebug = gcvFALSE;
        gctBOOL isOn = gcvFALSE;
        gctBOOL validToken = gcvTRUE;

        if (ntoken->poolString == PP->keyword->debug)
        {
            isDebug = gcvTRUE;
        }

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
        /*
        ** only accept on/off for options: debug, optimize
        */
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        if (ntoken->poolString != PP->keyword->lpara)
        {
            ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR,
                "Expect ( after identifier: debug\\optimize.");
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        if (ntoken->type != ppvTokenType_ID ||
            (!gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "on")) &&
            !gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "off"))))
        {
            validToken = gcvFALSE;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "on")))
        {
            isOn = gcvTRUE;
        }

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        if (ntoken->poolString != PP->keyword->rpara)
        {
            ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR,
               "Expect a ) after debug\\optimize(id .");
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

        if(ntoken->type != ppvTokenType_EOF && ntoken->type != ppvTokenType_NL)
        {
            ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR,
                "Not Expect argument after ).");
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }

        gcmONERROR(
            ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken)
            );

        if (validToken)
        {
            if (isDebug)
            {
                gcmONERROR(sloCOMPILER_SetDebug(PP->compiler, isOn));
            }
            else
            {
                gcmONERROR(sloCOMPILER_SetOptimize(PP->compiler, isOn));
            }
        }
    }
    else if (ntoken->poolString == PP->keyword->STDGL)
    {
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;

        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));
        if (ntoken->type == ppvTokenType_ID &&
            gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "invariant")))
        {
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            ntoken = gcvNULL;
            gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

            if (ntoken->poolString != PP->keyword->lpara)
            {
                ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR,
                   "Expect ( after identifier: STDGL.");
                gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }

            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            ntoken = gcvNULL;
            gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

            if (ntoken->type != ppvTokenType_ID || !gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "all")))
            {
                ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR,
                   "Expect all after identifier: invariant.");
                gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }

            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            ntoken = gcvNULL;
            gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

            if (ntoken->poolString != PP->keyword->rpara)
            {
                ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR,
                  "Expect a ) after invariant(all .");
                gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }

            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            ntoken = gcvNULL;
            gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

            if (ntoken->type != ppvTokenType_EOF && ntoken->type != ppvTokenType_NL)
            {
                ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR,
                   "Not Expect argument after ).");
                gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }

            gcmONERROR(
               ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken)
               );

            gcmONERROR(sloCOMPILER_SetOutputInvariant(PP->compiler, gcvTRUE));
       }
    }
    else
    {
        gcmONERROR(
            ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken)
            );
    }

    /* Reset invalid area. */
    PP->doWeInValidArea = prevValidStatus;
    if (ntoken != gcvNULL)
    {
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
    }
    return ppoPREPROCESSOR_ToEOL(PP);

OnError:
    if (ntoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken));
    }
    return status;
}

/******************************************************************************\
Undef
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_Undef(ppoPREPROCESSOR PP)
{
    gctSTRING            name    = gcvNULL;
    ppoTOKEN            ntoken    = gcvNULL;
    ppoMACRO_SYMBOL        ms        = gcvNULL;
    gceSTATUS            status  = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    gctBOOL                doWeInValidArea    = PP->doWeInValidArea;

    if (doWeInValidArea == gcvFALSE)
    {
        return ppoPREPROCESSOR_ToEOL(PP);
    }
    status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);
    if (status != gcvSTATUS_OK)
    {
        return status;
    }
    if (ntoken->type != ppvTokenType_ID)
    {
        ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
            "Error(%d,%d) : #undef should followed by id.", PP->currentSourceFileStringNumber, PP->currentSourceFileLineNumber);

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));

        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    }

    if (ntoken->poolString == PP->keyword->gl_es   ||
        ntoken->poolString == PP->keyword->_line_  ||
        ntoken->poolString == PP->keyword->_file_  ||
        ntoken->poolString == PP->keyword->_version_)
    {
        ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
            "Error(%d,%d) : Can not #undef builtin marcro %s.",
            PP->currentSourceFileStringNumber,
            PP->currentSourceFileLineNumber,
            ntoken->poolString);

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));

        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    }

    name = ntoken->poolString;

    gcmONERROR(
        ppoMACRO_MANAGER_GetMacroSymbol(
        PP,
        PP->macroManager,
        name,
        &ms)
        );

    if (!ms || ms->undefined == gcvTRUE)
    {
        ppoPREPROCESSOR_Report(
            PP,
            slvREPORT_WARN,
            "#undef a undefined id.");

        gcmONERROR(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        return gcvSTATUS_OK;
    }

    ms->undefined = gcvTRUE;

    gcmONERROR(
    ppoMACRO_MANAGER_DestroyMacroSymbol(
        PP,
        PP->macroManager,
        ms));

    gcmONERROR(
        ppoTOKEN_Destroy(PP, ntoken)
        );

    return gcvSTATUS_OK;

OnError:
    if (ntoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken));
        ntoken =  gcvNULL;
    }
    return status;
}

/******************************************************************************\
Define
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_Define(ppoPREPROCESSOR PP)
{
    gctSTRING            name        = gcvNULL;
    gctINT                argc        = 0;
    ppoTOKEN            argv        = gcvNULL;
    ppoTOKEN            rlst        = gcvNULL;
    ppoTOKEN            ntoken        = gcvNULL;
    ppoMACRO_SYMBOL        ms            = gcvNULL;
    gceSTATUS            status        = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    gctBOOL                doWeInValidArea        = PP->doWeInValidArea;
    gctBOOL                redefined    = gcvFALSE;
    gctBOOL                redefError    = gcvFALSE;
    gctBOOL             hasPara = gcvFALSE;
    ppoTOKEN            ntokenNext    = gcvNULL;
    ppoTOKEN            mstokenNext    = gcvNULL;

    if (doWeInValidArea == gcvFALSE)
    {
        return ppoPREPROCESSOR_ToEOL(PP);
    }

    gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

    if (ntoken->type != ppvTokenType_ID)
    {
        ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
            "Error(%d,%d) : #define should followed by id.",
            PP->currentSourceFileStringNumber,
            PP->currentSourceFileLineNumber);

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    }

    /*01 name*/
    name = ntoken->poolString;

    gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
    ntoken = gcvNULL;

    if (name == PP->keyword->_line_     ||
        name == PP->keyword->_version_  ||
        name == PP->keyword->_file_     ||
        name == PP->keyword->gl_es)
    {
        ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
            "Error(%d,%d) : Can not #redefine a builtin marcro %s.",
            PP->currentSourceFileStringNumber,
            PP->currentSourceFileLineNumber,
            name);
        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    }

    /*check if preceded by GL_ or __*/
    if (!gcoOS_StrNCmp(name , "GL_",3))
    {
        ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
            "GL_ is reserved to used by feature.");
        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    }

    if (sloCOMPILER_IsES31VersionOrAbove(PP->compiler) &&
        !gcoOS_StrNCmp(name , "__",2))
    {
        ppoPREPROCESSOR_Report(PP,slvREPORT_WARN,
            "__ is reserved to used by the compiler.");
    }

    gcmONERROR(
        ppoMACRO_MANAGER_GetMacroSymbol(PP, PP->macroManager, name, &ms)
        );

    if (ms != gcvNULL)
    {
        redefined = gcvTRUE;
    }

    gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, ppvICareWhiteSpace));

    if (ntoken->poolString == PP->keyword->lpara)
    {
        /*macro with (arguments-opt)*/
        hasPara = gcvTRUE;

        /*collect argv*/

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;

        gcmONERROR(ppoPREPROCESSOR_Define_BufferArgs(PP, &argv, &argc));
    }
    else if (ntoken->type != ppvTokenType_WS)
    {
        if (ntoken->type != ppvTokenType_NL)
        {
            gcePATCH_ID patchId = sloCOMPILER_GetPatchID(PP->compiler);

            if (!(patchId == gcvPATCH_GOOGLEEARTH && gcoOS_StrCmp(ntoken->poolString, ";") == gcvSTATUS_OK))
            {
                ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR, "White Space or New Line inputStream expected.");
            }
        }
        else
        {
            /*NL*/
            gcmONERROR(
                ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken)
                );

        }

        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
    }
    else
    {
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
    }

    /*replacement list*/

    gcmONERROR(
        ppoPREPROCESSOR_Define_BufferReplacementList(PP, &rlst)
        );

    if (redefined)
    {
        if (argc != ms->argc)
        {
            ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
                                "Can not redefine defined macro %s.",name);
            redefError = gcvTRUE;
        }
        else
        {
            ntokenNext = rlst;
            mstokenNext = ms->replacementList;

            while(ntokenNext || mstokenNext)
            {
                if (/* One of token is NULL */
                    (ntokenNext != mstokenNext && (mstokenNext == gcvNULL || ntokenNext == gcvNULL)) ||
                    /* Different replacement list */
                    (!gcmIS_SUCCESS(gcoOS_StrCmp(ntokenNext->poolString,mstokenNext->poolString))))
                {
                    ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR,
                        "Can not redefine defined macro %s.",name);
                    redefError = gcvTRUE;
                    break;
                }

                ntokenNext = (ppoTOKEN)ntokenNext->inputStream.base.node.prev;
                mstokenNext = (ppoTOKEN)mstokenNext->inputStream.base.node.prev;
            }
        }

        while (argv)
        {
            ntokenNext = (ppoTOKEN)argv->inputStream.base.node.prev;
            gcmONERROR(ppoTOKEN_Destroy(PP, argv));
            argv = ntokenNext;
        }

        while (rlst)
        {
            ntokenNext = (ppoTOKEN)rlst->inputStream.base.node.prev;
            gcmONERROR(ppoTOKEN_Destroy(PP, rlst));
            rlst = ntokenNext;
        }

        if (redefError)
        {
            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }

        return gcvSTATUS_OK;
    }

    /*make ms*/
    gcmONERROR(ppoMACRO_SYMBOL_Construct(
        (void*)PP,
        __FILE__,
        __LINE__,
        "ppoPREPROCESSOR_PPDefine : find a macro name, prepare to add a macro in the cpp's mac manager.",
        name,
        argc,
        argv,
        rlst,
        &ms));
    ms->hasPara = hasPara;

    return ppoMACRO_MANAGER_AddMacroSymbol(
        PP,
        PP->macroManager,
        ms);

OnError:
    if (ntoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;
    }
    return status;
}

