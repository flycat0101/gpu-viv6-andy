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


#include "gc_cl_preprocessor_int.h"

/*******************************************************************************
**
**    ppoPREPROCESSOR_PreprocessingFile
**
*/
gceSTATUS
ppoPREPROCESSOR_PreprocessingFile(ppoPREPROCESSOR PP)
{
    ppoTOKEN    ntoken        = gcvNULL;

    ppoTOKEN    ntoken2        = gcvNULL;

    gceSTATUS    status        = gcvSTATUS_INVALID_ARGUMENT;

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    do{
        ppmCheckFuncOk(
            ppoPREPROCESSOR_PassEmptyLine(PP)
            );

        ppmCheckFuncOk(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

        if (ntoken->type == ppvTokenType_EOF)
        {
            /*This is the end condition.*/

            return ppoTOKEN_Destroy(PP, ntoken);
        }
        else if (ntoken->poolString == PP->keyword->sharp)
        {
            /*#*/
            ppmCheckFuncOk(
                PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken2, !ppvICareWhiteSpace)
                );

            ppmCheckFuncOk(
                ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken2)
                );

            ppmCheckFuncOk(
                ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken)
                );

            ppmCheckFuncOk(
                ppoTOKEN_Destroy(PP, ntoken)/*No more usage in this path.*/
                );


            if( ntoken2->poolString == PP->keyword->eof            ||
                ntoken2->poolString == PP->keyword->newline        ||
                ntoken2->poolString == PP->keyword->if_            ||
                ntoken2->poolString == PP->keyword->ifdef        ||
                ntoken2->poolString == PP->keyword->ifndef        ||
                ntoken2->poolString == PP->keyword->pragma        ||
                ntoken2->poolString == PP->keyword->error        ||
                ntoken2->poolString == PP->keyword->line        ||
                ntoken2->poolString == PP->keyword->version        ||
                ntoken2->poolString == PP->keyword->include      ||
                ntoken2->poolString == PP->keyword->define        ||
                ntoken2->poolString == PP->keyword->undef)
            {
                ppmCheckFuncOk(
                    ppoTOKEN_Destroy(PP, ntoken2)
                    );

                status = ppoPREPROCESSOR_Group(PP, ppvIFSECTION_NONE);

                if(status != gcvSTATUS_OK) return status;
            }
            else
            {
                /*Other legal directive or inlegal directive*/

                ppoPREPROCESSOR_Report(PP,
                    clvREPORT_ERROR,
                    "Not expected symbol here \"%s\"",
                    ntoken2->poolString
                    );

                ppmCheckFuncOk(
                    ppoTOKEN_Destroy(PP, ntoken2)
                    );

                return gcvSTATUS_INVALID_ARGUMENT;
            }
        }
        else
        {
            /*Text Line*/

            PP->otherStatementHasAlreadyAppeared = gcvTRUE;

            ppmCheckFuncOk(
                ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken)
                );

            ppmCheckFuncOk(
                ppoTOKEN_Destroy(PP, ntoken)
                );

            ppmCheckFuncOk(
                ppoPREPROCESSOR_Group(PP, ppvIFSECTION_NONE)
                );
        }
    }
    while(gcvTRUE);/*group by group.*/
}

/*******************************************************************************
**
**    ppoPREPROCESSOR_Group
**
*/
gceSTATUS
ppoPREPROCESSOR_Group(ppoPREPROCESSOR PP,
                      ppeIFSECTION_TYPE    IfSectionType)
{
    ppoTOKEN                ntoken = gcvNULL;
    ppoTOKEN                ntoken2 = gcvNULL;
    gceSTATUS               status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    gcmHEADER_ARG("PP=0x%x, IfSectionType=%d", PP, IfSectionType);
    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    do
    {
        if (IfSectionType == ppvIFSECTION_INCLUDE)
        {
            gcmONERROR(
                PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
                );

            if(ntoken->type == ppvTokenType_EOF)
            {
                gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
                gcmFOOTER();
                return status;
            }
            else
            {
                gcmONERROR(
                    ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken)
                    );

                gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
                ntoken = gcvNULL;

            }
        }

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
                ntoken2->poolString == PP->keyword->include         ||
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
                            clvREPORT_ERROR,
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
                                clvREPORT_ERROR,
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
                            clvREPORT_ERROR,
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
ppoPREPROCESSOR_GroupPart(ppoPREPROCESSOR PP)
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
                 ntoken2->poolString == PP->keyword->include    ||
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
                            clvREPORT_ERROR,
                            "The version statement should appear only once.");

                        gcmONERROR(gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR);
                    }
                    if (gcvTRUE == PP->otherStatementHasAlreadyAppeared)
                    {
                        ppoPREPROCESSOR_Report(
                            PP,
                            clvREPORT_ERROR,
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
ppoPREPROCESSOR_IfSection(ppoPREPROCESSOR PP,
     ppoTOKEN             CurrentToken
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
                    clvREPORT_INTERNAL_ERROR,
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
                    clvREPORT_INTERNAL_ERROR,
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
                    clvREPORT_INTERNAL_ERROR,
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

    gceSTATUS    status        = gcvSTATUS_INVALID_ARGUMENT;

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    ppmCheckFuncOk(
        PP->inputStream->GetToken(
        PP,
        &(PP->inputStream),
        &ntoken,
        !ppvICareWhiteSpace)
        );

    if(ntoken->poolString == PP->keyword->lpara)
    {

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        ppmCheckFuncOk(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

        if(ntoken->type != ppvTokenType_ID){

            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
                "Expect and id after the defined(.");

            ppmCheckFuncOk(ppoTOKEN_Destroy(PP, ntoken));

            return gcvSTATUS_INVALID_ARGUMENT;

        }

        *Return = ntoken->poolString;

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        ppmCheckFuncOk(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

        if(ntoken->poolString != PP->keyword->rpara) {

            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
                "Expect a ) after defined(id .");

            ppmCheckFuncOk(
                ppoTOKEN_Destroy(PP, ntoken)
                );

            return gcvSTATUS_INVALID_ARGUMENT;

        }

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

    }
    else
    {

        if(ntoken->type != ppvTokenType_ID){

            ppmCheckFuncOk(
                ppoTOKEN_Destroy(PP, ntoken)
                );

            return gcvSTATUS_INVALID_ARGUMENT;

        }

        *Return = ntoken->poolString;

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

    }

    return gcvSTATUS_OK;
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
    gceSTATUS status = gcvSTATUS_INVALID_DATA;

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

    gceSTATUS status = gcvSTATUS_INVALID_DATA;

    char* creat_str = gcvNULL;

    char    numberbuffer [1024];

    gctUINT    offset = 0;

    gcoOS_MemFill(numberbuffer, 0, 1024);

    if (What == PP->keyword->_file_)
    {

        creat_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute __FILE__";
        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%s", PP->currentSourceFileStringName);

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

    ppmCheckFuncOk(
        ppoTOKEN_Construct(
        PP,
        __FILE__,
        __LINE__,
        creat_str,
        &newtoken));

    ppmCheckFuncOk(
        cloCOMPILER_AllocatePoolString(
        PP->compiler,
        numberbuffer,
        &(newtoken->poolString))
        );

    newtoken->hideSet    = gcvNULL;

    newtoken->type        = ppvTokenType_INT;

    /*newtoken->integer    = PP->currentSourceFileStringNumber;*/

    ppmCheckFuncOk(
        ppoPREPROCESSOR_AddToOutputStreamOfPP(PP, newtoken)
        );

    ppmCheckFuncOk(
        ppoTOKEN_Destroy(PP, newtoken)
        );

    return gcvSTATUS_OK;

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

    ppmCheckFuncOk(
        ppoHIDE_SET_LIST_ContainSelf(PP, ThisToken, TokenIsSelfContain)
        );

    ppmCheckFuncOk(
        ppoMACRO_MANAGER_GetMacroSymbol(
        PP,
        PP->macroManager,
        ThisToken->poolString,
        TheMacroSymbolOfThisId)
        );

    return gcvSTATUS_OK;

}
gceSTATUS
ppoPREPROCESSOR_TextLine(ppoPREPROCESSOR PP)
{
    gceSTATUS status = gcvSTATUS_INVALID_ARGUMENT;

    ppoTOKEN ntoken = gcvNULL;

    gctBOOL doWeInValidArea;

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    doWeInValidArea    = PP->doWeInValidArea;

    if(doWeInValidArea == gcvFALSE)
    {
        return ppoPREPROCESSOR_ToEOL(PP);
    }

    ppmCheckFuncOk(
        ppoPREPROCESSOR_PassEmptyLine(PP)
        );

    ppmCheckFuncOk(
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

            if (ntoken->poolString == PP->keyword->_file_
            ||    ntoken->poolString == PP->keyword->_line_)
            {
                ppmCheckFuncOk(
                    ppoPREPROCESSOR_TextLine_Handle_FILE_LINE_VERSION(PP, ntoken->poolString)
                    );

                ppmCheckFuncOk(
                    ppoTOKEN_Destroy(PP, ntoken)
                    );
            }
            else if(ntoken->type == ppvTokenType_ID)
            {
                /*Check the hide set of this ID.*/

                gctBOOL token_is_self_contain = gcvFALSE;

                ppoMACRO_SYMBOL the_macro_symbol_of_this_id = gcvNULL;

                status = ppoPREPROCESSOR_TextLine_CheckSelfContainAndIsMacroOrNot(
                    PP,
                    ntoken,
                    &token_is_self_contain,
                    &the_macro_symbol_of_this_id);
                if(status != gcvSTATUS_OK) return status;

                if(token_is_self_contain || the_macro_symbol_of_this_id == gcvNULL)
                {
                    status = ppoPREPROCESSOR_AddToOutputStreamOfPP(PP, ntoken);
                    if(status != gcvSTATUS_OK) return status;

                    status = ppoTOKEN_Destroy(PP, ntoken);
                    if(status != gcvSTATUS_OK) return status;
                }
                else
                {
                    ppoTOKEN head = gcvNULL;
                    ppoTOKEN end  = gcvNULL;
                    gctBOOL any_expanation_happened = gcvFALSE;

                    status = ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken);
                    if(status != gcvSTATUS_OK) return status;

                    status = ppoTOKEN_Destroy(PP, ntoken);
                    if(status != gcvSTATUS_OK) return status;

                    ppoPREPROCESSOR_MacroExpand(
                        PP,
                        &(PP->inputStream),
                        &head,
                        &end,
                        &any_expanation_happened);
                    if(status != gcvSTATUS_OK) return status;

                    gcmASSERT(
                        ( head == gcvNULL && end == gcvNULL )
                        ||
                        ( head != gcvNULL && end != gcvNULL ));

                    if(gcvTRUE == any_expanation_happened)
                    {
                        status = ppoPREPROCESSOR_TextLine_AddToInputAfterMacroExpand(
                            PP,
                            head,
                            end);

                        if(status != gcvSTATUS_OK) return status;
                    }
                    else
                    {
                        gcmASSERT(head == end);

                        if(head != gcvNULL)
                        {
                            status = ppoPREPROCESSOR_AddToOutputStreamOfPP(
                                PP,
                                head);
                            if(status != gcvSTATUS_OK) return status;
                        }
                    }
                }/*if(token_is_self_contain || the_macro_symbol_of_this_id == gcvNULL)*/
            }/*else if(ntoken->type == ppvTokenType_ID)*/
            else
            {
                /*Not ID*/
                ppmCheckFuncOk( ppoPREPROCESSOR_AddToOutputStreamOfPP(PP, ntoken) );

                ppmCheckFuncOk( ppoTOKEN_Destroy(PP, ntoken) );

            }

            ppmCheckFuncOk(
                PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
                );

        }/*internal while*/

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        ppmCheckFuncOk(
            ppoPREPROCESSOR_PassEmptyLine(PP)
            );

        ppmCheckFuncOk(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

    }/*extenal while*/

    ppmCheckFuncOk(
        ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken)
        );

    ppmCheckFuncOk(
        ppoTOKEN_Destroy(PP, ntoken)
        );

    return gcvSTATUS_OK;
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

    if(CurrentToken ->poolString == PP->keyword->include)
    {
        return ppoPREPROCESSOR_Include(PP);
    }
    else if (CurrentToken ->poolString == PP->keyword->define)
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

/******************************************************************************\
Version
\******************************************************************************/
gceSTATUS ppoPREPROCESSOR_Version(ppoPREPROCESSOR PP)
{
    gctBOOL doWeInValidArea = PP->doWeInValidArea;

    if(doWeInValidArea == gcvTRUE)
    {
        /*Now we just support 100*/
        ppoTOKEN    ntoken = gcvNULL;
        gceSTATUS    status = gcvSTATUS_INVALID_DATA;

        ppmCheckFuncOk(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

        if(ntoken->type != ppvTokenType_INT)
        {
            ppoPREPROCESSOR_Report(PP,
                clvREPORT_ERROR,
                "Expect a number afer the #version.");

            ppmCheckFuncOk(
                ppoTOKEN_Destroy(PP, ntoken)
                );

            return gcvSTATUS_INVALID_DATA;
        }
        if(ntoken->poolString != PP->keyword->version_100)
        {
            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
                "Expect 100 afer the #version.",
                PP->currentSourceFileStringNumber,
                PP->currentSourceFileLineNumber);

            ppmCheckFuncOk(
                ppoTOKEN_Destroy(PP, ntoken)
                );

            return gcvSTATUS_INVALID_DATA;
        }
        else
        {
            cloCOMPILER_SetVersion(PP->compiler, 100);
        }

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        return gcvSTATUS_OK;
    }
    else
    {
        return ppoPREPROCESSOR_ToEOL(PP);
    }
}
/******************************************************************************\
**
**    ppoPREPROCESSOR_Line
**
*/
gceSTATUS ppoPREPROCESSOR_Line(ppoPREPROCESSOR PP)
{
    gctBOOL doWeInValidArea    = 0;
    gctINT    line    = 0;
    gctINT    string    = 0;
    gceSTATUS    status = gcvSTATUS_INVALID_DATA;
    ppoTOKEN    ntoken1 = gcvNULL;

    gcmASSERT(PP);

    doWeInValidArea    = PP->doWeInValidArea;

    line    = PP->currentSourceFileLineNumber;
    string    = PP->currentSourceFileStringNumber;

    if(doWeInValidArea)
    {
        ppoTOKEN    ntoken = gcvNULL;

        status = gcvSTATUS_INVALID_DATA;

        ppmCheckFuncOk(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

        if(ntoken->type != ppvTokenType_INT)
        {
            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
                "Expect integer-line-number after #line.");

            return gcvSTATUS_INVALID_DATA;
        }

        /*process line number*/
        ppmCheckFuncOk(
            ppoPREPROCESSOR_EvalInt(
            PP,
            ntoken,
            &line)
            );

        if(line <= 0)
        {
            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
                "Expect positive integer-line-number after #line.");

            return gcvSTATUS_INVALID_DATA;
        }

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        ppmCheckFuncOk(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
            );

        if(ntoken->type != ppvTokenType_EOF && ntoken->type != ppvTokenType_NL)
        {
            if(ntoken->type != ppvTokenType_INT)
            {
                if (gcvSTATUS_OK == gcoOS_StrNCmp(ntoken->poolString, "\"", 1))
                {
                    gcoOS_StrCatSafe(PP->currentSourceFileStringName, 1024, ntoken->poolString);
                    do
                    {
                        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken1, !ppvICareWhiteSpace));
                        if (ntoken1->poolString == PP->keyword->eof ||
                            ntoken1->poolString == PP->keyword->newline)
                        {
                            gcmASSERT(gcvFALSE);
                            return ppoTOKEN_Destroy(PP, ntoken1);
                        }
                        if (gcvSTATUS_OK == gcoOS_StrNCmp(ntoken1->poolString, "\"", 1))
                        {
                            gcoOS_StrCatSafe(PP->currentSourceFileStringName, 1024, ntoken1->poolString);
                            gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken1));
                            ntoken1 = gcvNULL;
                            break;
                        }
                        else
                        {
                            gcoOS_StrCatSafe(PP->currentSourceFileStringName, 1024, ntoken1->poolString);
                            gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken1));
                            ntoken1 = gcvNULL;
                        }
                    } while (gcvTRUE);
                }
            }
            else
            {
                /*process poolString number*/
                ppmCheckFuncOk(
                    ppoPREPROCESSOR_EvalInt(
                    PP,
                    ntoken,
                    &string)
                    );

                if(string < 0)
                {
                    ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
                        "Expect none negative source-string-number"
                        " after #line.");
                    return gcvSTATUS_INVALID_DATA;
                }
            }
        }
        else
        {
            ppmCheckFuncOk(
                ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken)
                );
        }

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

    }
    status = ppoPREPROCESSOR_ToEOL(PP);

    if(status != gcvSTATUS_OK){return status;}

    PP->currentSourceFileStringNumber = string;

    PP->currentSourceFileLineNumber = line;

    return gcvSTATUS_OK;

OnError:
    if (ntoken1)
        ppoTOKEN_Destroy(PP, ntoken1);
    status = ppoPREPROCESSOR_ToEOL(PP);
    return status;
}
/******************************************************************************\
Error
\******************************************************************************/
gceSTATUS ppoPREPROCESSOR_Error(ppoPREPROCESSOR PP)
{
    gctBOOL doWeInValidArea = PP->doWeInValidArea;
    if(doWeInValidArea == gcvTRUE)
    {
        gceSTATUS status = gcvSTATUS_INVALID_DATA;
        ppoTOKEN    ntoken = gcvNULL;
        ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
            "Error(str:%d,lin:%d): "
            "Meet #error with:",
            PP->currentSourceFileStringNumber,
            PP->currentSourceFileLineNumber);
        status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);
        if(status != gcvSTATUS_OK){return status;}
        while(ntoken->poolString != PP->keyword->newline && ntoken->poolString != PP->keyword->eof)
        {
            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR, "%s ", ntoken->poolString);

            ppmCheckFuncOk(ppoTOKEN_Destroy(PP, ntoken));

            status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);    ppmCheckOK();
        }
        ppmCheckFuncOk(ppoTOKEN_Destroy(PP, ntoken));
        ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR, "");
        return gcvSTATUS_INVALID_DATA;
    }
    else
    {
        return ppoPREPROCESSOR_ToEOL(PP);
    }
}

/******************************************************************************\
Pragma
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_Pragma(ppoPREPROCESSOR PP)
{
    ppoTOKEN     ntoken  = gcvNULL;
    ppoTOKEN     ntoken1 = gcvNULL;
    gceSTATUS    status  = gcvSTATUS_OK;
    gctBOOL      prevValidStatus = PP->doWeInValidArea;
    cleEXTENSION Extension = clvEXTENSION_NONE;
    gctBOOL      Enable = gcvFALSE;

    /* Set current area as invalid so we can accept invalid tokens. */
    PP->doWeInValidArea = gcvFALSE;

    status = (PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));

    /* Ignore a unrecognized preprocessor token. */
    if (status != gcvSTATUS_OK)
    {
        ppoPREPROCESSOR_ToEOL(PP);
    }
    else if (ntoken->poolString == PP->keyword->eof || ntoken->poolString == PP->keyword->newline)
    {
        PP->doWeInValidArea = prevValidStatus;
        if (ntoken != gcvNULL)
        {
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        }
        return status;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "OPENCL")))
    {
        gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
        ntoken = gcvNULL;

        /* #pragma OPENCL FP_CONTRACT do nothing.

           #pragma OPENCL EXTENSION XXX:
             Supported: all, CL_VIV_asm, cl_viv_bitfield_extension, cl_viv_cmplx_extension
             other extensions will be ignored silently.
         */
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));
        if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "EXTENSION")))
        {
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            ntoken = gcvNULL;
            gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace));
            if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "all")))
            {
                Extension = clvEXTENSION_ALL;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "cl_khr_fp16")))
            {
                Extension = clvEXTENSION_CL_KHR_FP16;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "CL_VIV_asm")))
            {
                Extension = clvEXTENSION_VASM;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "cl_viv_bitfield_extension")))
            {
                Extension = clvEXTENSION_VIV_BITFIELD;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "cl_viv_cmplx_extension")))
            {
                Extension = clvEXTENSION_VIV_CMPLX;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "cl_viv_vx_extension")))
            {
                Extension = clvEXTENSION_VIV_VX;
            }
            else
            {
            }

            gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken1, !ppvICareWhiteSpace));

            if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken1->poolString, ":")))
            {
                gcmONERROR(ppoTOKEN_Destroy(PP, ntoken1));
                ntoken1 = gcvNULL;
                gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken1, !ppvICareWhiteSpace));
                if (gcmIS_SUCCESS(gcoOS_StrCmp(ntoken1->poolString, "enable")))
                {
                    Enable = gcvTRUE;
                }
            }

            if (Extension == clvEXTENSION_ALL)
            {
                gcmVERIFY_OK(cloCOMPILER_EnableExtension(
                        PP->compiler,
                        clvEXTENSION_CL_KHR_FP16,
                        Enable));
                gcmVERIFY_OK(cloCOMPILER_EnableExtension(
                        PP->compiler,
                        clvEXTENSION_VASM,
                        Enable));

                gcmVERIFY_OK(cloCOMPILER_EnableExtension(
                        PP->compiler,
                        clvEXTENSION_VIV_BITFIELD,
                        Enable));

                gcmVERIFY_OK(cloCOMPILER_EnableExtension(
                        PP->compiler,
                        clvEXTENSION_VIV_CMPLX,
                        Enable));
            }
            else
            {
                gcmVERIFY_OK(cloCOMPILER_EnableExtension(
                        PP->compiler,
                        Extension,
                        Enable));
                if (Enable)
                {
                    if (Extension == clvEXTENSION_VIV_VX)
                    {
                        gctBOOL isVX2 = gcGetHWCaps()->hwFeatureFlags.supportEVISVX2;
                        if (isVX2)
                            status = ppoPREPROCESSOR_addMacroDef_Int(PP, "VX_VERSION", "2");
                        else
                            status =  ppoPREPROCESSOR_addMacroDef_Int(PP, "VX_VERSION", "1");
                        status = ppoPREPROCESSOR_addMacroDef_Int(PP, "_VIV_VX_EXTENSION", "1");
                        status = ppoPREPROCESSOR_AddSdkDirToPath(PP);
                        if (gcmIS_ERROR(status))
                            return status;
                    }
                    else
                    {
                        ppoPREPROCESSOR_addMacroDef_Int(PP, ntoken->poolString, "1");
                    }
                }
                else
                {
                    /* TODO: need to undef the macro */
                }
            }
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken));
            ntoken = gcvNULL;
            gcmONERROR(ppoTOKEN_Destroy(PP, ntoken1));
            ntoken1 = gcvNULL;
        }
        else if(gcmIS_SUCCESS(gcoOS_StrCmp(ntoken->poolString, "FP_CONTRACT")))
        {
            /* ignore it for now */
        }
        else
        {
            ppoPREPROCESSOR_Report(PP,
                                    clvREPORT_ERROR,
                                    "OpenCL not support this pragma directive."
                                    );
        }
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
    gctSTRING                name    = gcvNULL;
    ppoTOKEN            ntoken    = gcvNULL;
    ppoMACRO_SYMBOL        ms        = gcvNULL;
    gceSTATUS            status  = gcvSTATUS_INVALID_ARGUMENT;
    gctBOOL                doWeInValidArea    = PP->doWeInValidArea;

    if(doWeInValidArea == gcvFALSE)
    {
        return ppoPREPROCESSOR_ToEOL(PP);
    }
    status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);
    if(status != gcvSTATUS_OK)
    {
        return status;
    }
    if(ntoken->type != ppvTokenType_ID){
        ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
            "Error(%d,%d) : #undef should followed by id.", PP->currentSourceFileStringNumber, PP->currentSourceFileLineNumber);

        ppoTOKEN_Destroy(PP, ntoken);

        return gcvSTATUS_INVALID_ARGUMENT;
    }

    name = ntoken->poolString;

    ppmCheckFuncOk(
        ppoMACRO_MANAGER_GetMacroSymbol(
        PP,
        PP->macroManager,
        name,
        &ms)
        );

    if(!ms || ms->undefined == gcvTRUE)
    {
        ppoPREPROCESSOR_Report(
            PP,
            clvREPORT_WARN,
            "#undef a undefined id.");

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        return gcvSTATUS_OK;
    }

    ms->undefined = gcvTRUE;

    ppoMACRO_MANAGER_DestroyMacroSymbol(
            PP,
            PP->macroManager,
            ms);

    ppmCheckFuncOk(
        ppoTOKEN_Destroy(PP, ntoken)
        );

    return gcvSTATUS_OK;
}

#define _cldFILENAME_MAX 1024

static gctBOOL
ppoPREPROCESSOR_ReadHeaderFile(
    IN gcoOS Os,
    IN ppoPREPROCESSOR PP,
    IN gctCONST_STRING FileName,
    OUT gctSIZE_T * SourceSize,
    OUT gctSTRING * Source
    )
{
    gceSTATUS   status = gcvSTATUS_NOT_FOUND;
    gctFILE     file = gcvNULL;
    gctUINT32   count = 0;
    gctSIZE_T   byteRead = 0;
    gctSTRING   source = gcvNULL;
    gctCHAR     Path[_cldFILENAME_MAX] = {'\0'};
    ppoHEADERFILEPATH headerFilePathNode = gcvNULL;
#if defined( WIN32 )
    const gctSTRING sep = "\\";
#else
    const gctSTRING sep = "/";
#endif

    gcmASSERT(FileName != gcvNULL);

    status = gcoOS_Open(Os, FileName, gcvFILE_READ, &file);
    if (status != gcvSTATUS_OK)
    {
        /* remove "./" if any */
        if (gcoOS_StrLen(FileName, gcvNULL) > 2 && *FileName == '.' && *(FileName + 1) == '/')
        {
            FileName = FileName + 2;
        }
        /* The order of searching
           1.Current Path "./";
           2."-I" and "-cl-viv-vx-extension" option added path;*/
        headerFilePathNode = PP->headerFilePathList;
        while (headerFilePathNode != gcvNULL)
        {
            gcoOS_StrCopySafe(Path,_cldFILENAME_MAX, headerFilePathNode->headerFilePath);
            gcoOS_StrCatSafe(Path, _cldFILENAME_MAX, sep);
            gcoOS_StrCatSafe(Path, _cldFILENAME_MAX, FileName);
            status = gcoOS_Open(Os, Path, gcvFILE_READ, &file);
            if (status == gcvSTATUS_OK)
            {
                break;
            }
            headerFilePathNode = (void*)headerFilePathNode->base.node.next;
        }
    }
    if (status != gcvSTATUS_OK)
    {
        ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
            "Error(%d,%d) : Cannot find the header file %s.",
            PP->currentSourceFileStringNumber,
            PP->currentSourceFileLineNumber,
            FileName);
        return gcvFALSE;
    }

    gcmVERIFY_OK(gcoOS_Seek(Os, file, 0, gcvFILE_SEEK_END));
    gcmVERIFY_OK(gcoOS_GetPos(Os, file, &count));

    source = gcvNULL;
    gcmONERROR(cloCOMPILER_Allocate(PP->compiler, count + 2, (gctPOINTER *) &source));

    if (!gcmIS_SUCCESS(status))
    {
        gcmPRINT("ERROR: Not enough memory.\n");
        return gcvFALSE;
    }

    if (!gcmIS_SUCCESS(status))
    {
        gcmPRINT("ERROR: Not enough memory.\n");
        return gcvFALSE;
    }

    gcmVERIFY_OK(gcoOS_SetPos(Os, file, 0));

    status = gcoOS_Read(Os, file, count, source, &byteRead);
    if (!gcmIS_SUCCESS(status) || byteRead != count)
    {
        ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
            "Error(%d,%d) : Failed to read the header file %s.",
            PP->currentSourceFileStringNumber,
            PP->currentSourceFileLineNumber,
            FileName);
        gcmVERIFY_OK(gcoOS_Close(Os, file));
        cloCOMPILER_Free(PP->compiler, source);
        return gcvFALSE;
    }

    source[count] = ppvCHAR_EOF;
    source[count+1] = '\0';
    *SourceSize = count + 1;
    *Source = source;

OnError:
    gcmVERIFY_OK(gcoOS_Close(Os, file));
    return gcvTRUE;
}

/******************************************************************************\
Include
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_Include(ppoPREPROCESSOR PP)
{
    gceSTATUS status = gcvSTATUS_OK;
    ppoTOKEN ntoken1 = gcvNULL, ntoken2 = gcvNULL;
    gctSIZE_T sourceSize = 0;
    gctSTRING source = gcvNULL;
    gctCHAR fileName[1024] = "\0";
    ppoINPUT_STREAM tmpbis = gcvNULL;
    ppoBYTE_INPUT_STREAM tmpbisCreated = gcvNULL;

    /* TODO: Lex the include file name */
    gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken1, !ppvICareWhiteSpace));
    if (gcvSTATUS_OK == gcoOS_StrNCmp(ntoken1->poolString, "\"", 1))
    {
        do
        {
            gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken2, !ppvICareWhiteSpace));
            if (ntoken2->poolString == PP->keyword->eof ||
                ntoken2->poolString == PP->keyword->newline)
            {
                gcmASSERT(gcvFALSE);
                return ppoTOKEN_Destroy(PP, ntoken2);
            }
            if (gcvSTATUS_OK == gcoOS_StrNCmp(ntoken2->poolString, "\"", 1))
            {
                gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken2));
                break;
            }
            else
            {
                gcoOS_StrCatSafe(fileName, 1024, ntoken2->poolString);
                gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken2));
            }
        } while (gcvTRUE);
    }

    if (!ppoPREPROCESSOR_ReadHeaderFile(gcvNULL, PP, fileName, &sourceSize, &source))
    {
        goto OnError;
    }

    gcmONERROR(ppoBYTE_INPUT_STREAM_Construct(
        PP,
        gcvNULL,/*prev node*/
        gcvNULL,/*next node*/
        __FILE__,
        __LINE__,
        "ppoPREPROCESSOR_SetSourceStrings : Creat to init CPP input stream",
        source,
        -1, /* input string number is -1 for header file */
        sourceSize,
        &tmpbisCreated
        ));

    tmpbis = (ppoINPUT_STREAM) tmpbisCreated;

    if( gcvNULL == PP->inputStream )
    {
        PP->inputStream = tmpbis;
        tmpbis->base.node.prev = gcvNULL;
        tmpbis->base.node.next = gcvNULL;
    }
    else
    {
        ppoINPUT_STREAM tmp_is = PP->inputStream;
        PP->inputStream = tmpbis;
        tmpbis->base.node.prev = (void*)tmp_is;
        tmpbis->base.node.next = (void*)gcvNULL;
        tmp_is->base.node.next = (void*)tmpbis;
    }
    status = ppoPREPROCESSOR_Group(PP, ppvIFSECTION_INCLUDE);

OnError:
    if (ntoken1 != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, ntoken1));
        ntoken1 = gcvNULL;
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
        ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
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
        name == PP->keyword->_file_       )
    {
        ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
            "Error(%d,%d) : Can not #redefine a builtin marcro %s.",
            PP->currentSourceFileStringNumber,
            PP->currentSourceFileLineNumber,
            name);
        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    }

    gcmONERROR(
        ppoMACRO_MANAGER_GetMacroSymbol(PP, PP->macroManager, name, &ms)
        );

    if (ms != gcvNULL && ms->undefined == gcvFALSE)
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
            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR, "White Space or New Line inputStream expected.");
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
            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
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
                    ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
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

        ms->undefined = gcvFALSE;
        return gcvSTATUS_OK;
    }

    if (ms && ms->undefined)
    {
        ppoMACRO_MANAGER_DestroyMacroSymbol(
            PP,
            PP->macroManager,
            ms);
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

