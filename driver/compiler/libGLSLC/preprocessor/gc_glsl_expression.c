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


#include "gc_glsl_preprocessor_int.h"

/*******************************************************************************
**
**    ppoPREPROCESSOR_GuardTokenOfThisLevel
**
**        Judge, whether the token inputStream the guard token of this level, or level
*        above this level
**
*/
gceSTATUS
ppoPREPROCESSOR_GuardTokenOfThisLevel(
                                      ppoPREPROCESSOR        PP,
                                      ppoTOKEN            Token,
                                      gctSTRING            OptGuarder,
                                      gctINT                Level,
                                      gctBOOL*            Result)
{
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    gcmASSERT(
        PP
        && Token);

    gcmASSERT(
        (0 <= Level && Level <= 10)
        ||
        PP->operators[Level] != gcvNULL
        );

    *Result =   gcvFALSE;

    if(OptGuarder == Token->poolString)
    {
        *Result = gcvTRUE;

        return gcvSTATUS_OK;
    }

    while(Level > 0)
    {
        --Level;

        gcmONERROR(
            ppoPREPROCESSOR_IsOpTokenInThisLevel(
            PP,
            Token,
            Level,
            Result
            )
            );

        if((*Result) == gcvTRUE)
        {
            /* Finded */
            return gcvSTATUS_OK;
        }
    }

    /* Should be not finded */
    gcmASSERT( gcvFALSE == *Result );

    return gcvSTATUS_OK;

OnError:
    return status;
}


/*******************************************************************************
**
**    ppoPREPROCESSOR_IsOpTokenInThisLevel
**
**        Judge, whether the token inputStream an operator defined in this level.
**
*/

gceSTATUS
ppoPREPROCESSOR_IsOpTokenInThisLevel(
                                     ppoPREPROCESSOR        PP,
                                     ppoTOKEN            Token,
                                     gctINT                Level,
                                     gctBOOL*            Result
                                     )
{

    gctSTRING* op_ptr;

    gcmASSERT(
        PP
        && Token);

    gcmASSERT(
        (0 <= Level && Level <= 10)
        ||
        PP->operators[Level] == gcvNULL
        );

    *Result = gcvFALSE;

    op_ptr = &(PP->operators[Level][1]);

    while( (*op_ptr) != 0 )
    {
        if( Token->poolString == (*op_ptr) )
        {

            *Result = gcvTRUE;

            return gcvSTATUS_OK;
        }

        ++op_ptr;
    }

    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**    ppoPREPROCESSOR_EvalInt
**
**        Evalue an integer value from the pool string in *Token*
**
*/
gceSTATUS  ppoPREPROCESSOR_EvalInt(
                                   ppoPREPROCESSOR        PP,
                                   ppoTOKEN            Token,
                                   gctINT*                Result)
{
    gctCONST_STRING      traveler    = gcvNULL;
    gctSIZE_T            len         = 0;
    gctINT               w           = 0;
    gctBOOL              temp_bool   = gcvFALSE;

    gcmASSERT(Token && Token->type == ppvTokenType_INT && Result);

    traveler = Token->poolString;

    len = gcoOS_StrLen(Token->poolString, gcvNULL);

    gcmASSERT(len >=1);

    *Result = 0;

    if(len == 1)
    {
        if(!ppoPREPROCESSOR_isnum(traveler[0]))
        {
            ppoPREPROCESSOR_Report(
                PP,
                slvREPORT_INTERNAL_ERROR,
                "The input token's type inputStream int but the poolString contains"
                "some digit not number:%c.",
                traveler[0]);
            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }
        *Result    = (traveler[0] - '0');
        return gcvSTATUS_OK;
    }

    /*len >= 2*/
    temp_bool =
        (Token->poolString[0] == '0' && Token->poolString[1] == 'x')
        ||
        (Token->poolString[0] == '0' && Token->poolString[1] == 'X');

    if(len == 2 && temp_bool)
    {
        ppoPREPROCESSOR_Report(
            PP,
            slvREPORT_ERROR,
            "%s can not be eval out.",
            Token->poolString);
        return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    }
    if(temp_bool)
    {
        /*hex*/
        while(len >= 3)
        {
            gctINT    tmp = 0;
            if(!ppoPREPROCESSOR_ishexnum(traveler[len-1])){
                ppoPREPROCESSOR_Report(
                    PP,
                    slvREPORT_INTERNAL_ERROR,
                    "eval_int : The input token's type inputStream int but \
                    the poolString contains some digit not hex number:%c.",
                    traveler[len-1]);
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }
            if(ppoPREPROCESSOR_isnum(traveler[len-1]))
            {
                tmp = traveler[len-1] - '0';
            }
            else if('a' <= traveler[len-1] && traveler[len-1] <= 'f')
            {
                tmp = traveler[len-1] - 'a' + 10;
            }
            else if('A' <= traveler[len-1] && traveler[len-1] <= 'F')
            {
                tmp = traveler[len-1] - 'A' + 10;
            }
            else
            {
                ppoPREPROCESSOR_Report(PP,
                    slvREPORT_INTERNAL_ERROR,
                    "eval_int : The input token's type inputStream int but \
                    the poolString contains some digit not hex number:%c.",
                    traveler[len-1]);
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }
            (*Result) = (*Result) + (tmp * (gctINT)ppoPREPROCESSOR_Pow((int)16, w));
            ++w;
            --len;
        }
        return gcvSTATUS_OK;
    }
    if(Token->poolString[0] == '0')
    {
        /*oct*/
        while(len >= 2)
        {
            if(!ppoPREPROCESSOR_isoctnum(traveler[len-1])){
                ppoPREPROCESSOR_Report(
                    PP,
                    slvREPORT_INTERNAL_ERROR,
                    "eval_int : The input token's type inputStream \
                    int but the poolString contains some digit not\
                    oct number:%c.", traveler[len-1]);
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }
            (*Result) = (*Result) + ((traveler[len-1] - '0') * (gctINT)ppoPREPROCESSOR_Pow((int)8, w));
            ++w;
            --len;
        }
        return gcvSTATUS_OK;
    }
    /*dec*/
    while(len){
        if(!ppoPREPROCESSOR_isnum(traveler[len-1])){
            ppoPREPROCESSOR_Report(
                PP,
                slvREPORT_INTERNAL_ERROR,
                "eval_int : The input token's type inputStream int but the \
                poolString contains some digit not number:%c.", traveler[len-1]);
            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }
        (*Result) = (*Result) + ((traveler[len-1] - '0') * (gctINT)ppoPREPROCESSOR_Pow((int)10, w));
        ++w;
        --len;
    }
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**    ppoPREPROCESSOR_Eval
**
**        ppoPREPROCESSOR_Eval used in ppoPREPROCESSOR_Eval.
**
*/

gceSTATUS    ppoPREPROCESSOR_Eval_GetToken_FILE_LINE_VERSION_GL_ES(
    ppoPREPROCESSOR    PP,
    ppoTOKEN Token,
    ppoTOKEN *Out,
    gctBOOL    *IsMatch
    )
{
    char *token_cons_str = gcvNULL;

    ppoTOKEN    newtoken    =    gcvNULL;

    gctUINT        offset        =    0;

    char        numberbuffer [128];

    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    gcoOS_MemFill(numberbuffer, 0, 128);

    *IsMatch = gcvTRUE;

    *Out = gcvNULL;

    if(Token->poolString == PP->keyword->_file_)
    {
        token_cons_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute __FILE__";

        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d", PP->currentSourceFileStringNumber);
    }
    else if(Token->poolString == PP->keyword->_line_)
    {
        token_cons_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute __LINE__";

        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d", PP->currentSourceFileLineNumber);
    }
    else if(Token->poolString == PP->keyword->_version_)
    {
        token_cons_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute __VERSION__";

        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d", PP->version);
    }
    else if(Token->poolString == PP->keyword->gl_es)
    {
        token_cons_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute GL_ES";

        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d", 1);
    }
    else if(Token->poolString == PP->keyword->gl_core_profile)
    {
        token_cons_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute GL_core_profile";

        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d", 1);
    }
    else if(Token->poolString == PP->keyword->gl_compatibility_profile)
    {
        token_cons_str = "ppoPREPROCESSOR_TextLine : Creat a new token to substitute GL_compatibility_profile";

        gcoOS_PrintStrSafe(numberbuffer, gcmSIZEOF(numberbuffer), &offset, "%d", 1);
    }
    else
    {
        *IsMatch = gcvFALSE;

        return gcvSTATUS_OK;
    }

    gcmONERROR(ppoTOKEN_Construct(PP, __FILE__, __LINE__, token_cons_str,    &newtoken));

    gcmONERROR(sloCOMPILER_AllocatePoolString(PP->compiler, numberbuffer,    &(newtoken->poolString)));

    newtoken->hideSet        = gcvNULL;

    newtoken->srcFileString    = PP->currentSourceFileStringNumber;

    newtoken->srcFileLine    = PP->currentSourceFileLineNumber;

    newtoken->type            = ppvTokenType_INT;

    *Out = newtoken;

    return gcvSTATUS_OK;
OnError:
    if (newtoken != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, newtoken));
        newtoken = gcvNULL;
    }
    return status;
}
gceSTATUS    ppoPREPROCESSOR_Eval_GetToken(
    ppoPREPROCESSOR    PP,
    ppoTOKEN*        Token,
    gctBOOL            ICareWhiteSpace
    )
{
    gceSTATUS    status  =   gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    ppoTOKEN    token    =    gcvNULL;

    gctBOOL is_predefined = gcvFALSE;

    gctBOOL token_contain_self = gcvFALSE;

    gctBOOL is_there_any_expanation_happened_internal = gcvFALSE;

    ppoMACRO_SYMBOL ms    = gcvNULL;

    ppoTOKEN expanded_head    =    gcvNULL;

    ppoTOKEN expanded_end    =    gcvNULL;

    gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &token, !ppvICareWhiteSpace));

    /* If this token is from a macro, it may be a NUL, we need to skip it. */
    while (token->type == ppvTokenType_NUL)
    {
        status = ppoTOKEN_Destroy(PP, token);
        gcmONERROR(PP->inputStream->GetToken(PP, &(PP->inputStream), &token, !ppvICareWhiteSpace));
    }

    if(token->type != ppvTokenType_ID || token->poolString == PP->keyword->defined)
    {
        *Token = token;
        return gcvSTATUS_OK;
    }

    gcmONERROR(ppoPREPROCESSOR_Eval_GetToken_FILE_LINE_VERSION_GL_ES(PP, token,Token, &is_predefined));

    if (is_predefined == gcvTRUE)
    {
        status = ppoTOKEN_Destroy(PP, token);
        token = gcvNULL;

        if (gcmIS_SUCCESS(status))
        {
            return gcvSTATUS_OK;
        }
        else
        {
            return status;
        }
    }
    /*Try to expand this ID.*/

    gcmONERROR(ppoHIDE_SET_LIST_ContainSelf(PP, token, &token_contain_self));

    gcmONERROR(ppoMACRO_MANAGER_GetMacroSymbol(PP, PP->macroManager, token->poolString, &ms));

    if (gcvTRUE == token_contain_self || gcvNULL == ms )
    {
        *Token = token;
        return gcvSTATUS_OK;
    }

    gcmONERROR(ppoINPUT_STREAM_UnGetToken(PP,&(PP->inputStream), token));

    gcmONERROR(ppoTOKEN_Destroy(PP, token));
    token = gcvNULL;

    gcmONERROR(ppoPREPROCESSOR_MacroExpand(
        PP,
        &(PP->inputStream),
        &expanded_head,
        &expanded_end,
        &is_there_any_expanation_happened_internal));

    /*both null or both not null*/
    gcmASSERT(
        (expanded_head == gcvNULL && expanded_end == gcvNULL)
        ||
        (expanded_head != gcvNULL && expanded_end != gcvNULL)
        );


    if(expanded_head == gcvNULL)
    {
        gcmONERROR(ppoPREPROCESSOR_Eval_GetToken(PP, Token, ICareWhiteSpace));
        if (gcmIS_SUCCESS(status))
        {
            return gcvSTATUS_OK;
        }
    }

    if (gcvTRUE == is_there_any_expanation_happened_internal && expanded_head != gcvNULL)
    {
        expanded_end->inputStream.base.node.prev = gcvNULL;

        PP->inputStream->base.node.next = (void*)expanded_end;

        expanded_end->inputStream.base.node.prev = (void*)PP->inputStream;

        PP->inputStream = (void*)expanded_head;

        expanded_head->inputStream.base.node.next = gcvNULL;

        gcmONERROR(ppoPREPROCESSOR_Eval_GetToken(PP, Token, ICareWhiteSpace));
        if (gcmIS_SUCCESS(status))
        {
            return gcvSTATUS_OK;
        }
    }

    *Token = expanded_head;

    return gcvSTATUS_OK;

OnError:
    if (token != gcvNULL)
    {
        gcmVERIFY_OK(ppoTOKEN_Destroy(PP, token));
        token = gcvNULL;
    }
    return status;
}

gceSTATUS
ppoPREPROCESSOR_Eval_Case_Left_Para(
                                    ppoPREPROCESSOR    PP,
                                    gctBOOL            EvaluateLine,
                                    gctBOOL            *MeetStringNum,
                                    gctINT*            Result
                                    )
{
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    ppoTOKEN token = gcvNULL;

    gcmHEADER_ARG("PP=0x%x Result=0x%x", PP, Result);

    gcmONERROR(ppoPREPROCESSOR_Eval(PP, PP->keyword->rpara, 0, EvaluateLine, MeetStringNum, Result));

    /*( expression*/
    gcmONERROR(ppoPREPROCESSOR_Eval_GetToken(PP, &token, !ppvICareWhiteSpace));

    if(token->poolString != PP->keyword->rpara)
    {
        ppoPREPROCESSOR_Report(PP,slvREPORT_ERROR, ") inputStream expected.");
        status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        gcmFOOTER();
        return status;
    }

    gcmONERROR(ppoTOKEN_Destroy(PP, token));

    gcmFOOTER_ARG("*Result=%d", *Result);
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**    ppoPREPROCESSOR_Eval
**
**        Eval an expression in the file.
**
*/
gceSTATUS
ppoPREPROCESSOR_Eval_Case_Basic_Level(
                                      ppoPREPROCESSOR    PP,
                                      ppoTOKEN           Token,
                                      gctBOOL            EvaluateLine,
                                      gctBOOL            *MeetStringNum,
                                      gctINT*            Result
                                      )
{
    if ((Token->type == ppvTokenType_ID)
    &&  gcmIS_SUCCESS(gcoOS_StrCmp(Token->poolString, "GL_FRAGMENT_PRECISION_HIGH"))
    )
    {
        *Result = 1;
        return gcvSTATUS_OK;
    }
    else if(Token->poolString == PP->keyword->lpara)
    {
        /*(*/
        return ppoPREPROCESSOR_Eval_Case_Left_Para(PP, EvaluateLine, MeetStringNum, Result);
    }

    if(Token->type != ppvTokenType_INT)
    {
        if((!PP->skipOPError) ||
           (PP->skipOPError && Token->type != ppvTokenType_ID))
        {
            /* The undefined symbol is evalated as 0 in desktop GLSL.
               But when the expression is evalated in #line, the undefined symbol cannot be treated as 0 */
            if (Token->type == ppvTokenType_ID &&
                sloCOMPILER_GetClientApiVersion(PP->compiler) == gcvAPI_OPENGL &&
                !MeetStringNum)
            {
                *Result = 0;
                return gcvSTATUS_OK;
            }
            else
            {
                ppoPREPROCESSOR_Report(
                    PP,
                    slvREPORT_ERROR,
                    "Integer is expected."
                    );
            }

            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }
        else
        {
            /*
            ** skip undefined identifiers error such as: #if 1 || AA  and # if 0 && AA
            */
            *Result = 1;
            PP->skipOPError = gcvFALSE;
            return gcvSTATUS_OK;
        }
    }

    return ppoPREPROCESSOR_EvalInt(PP, Token, Result);
}

gceSTATUS
ppoPREPROCESSOR_Eval_Case_Unary_Op(
                                   ppoPREPROCESSOR    PP,
                                   gctSTRING          OptGuarder,
                                   gctINT             Level,
                                   gctINT*            Result,
                                   gctBOOL            EvaluateLine,
                                   gctBOOL            *MeetStringNum,
                                   ppoTOKEN           Token
                                   )
{
    gceSTATUS status;

    gctBOOL is_token_in_this_level;

    gctINT result = 0;

    gcmHEADER_ARG("PP=0x%x OptGuarder=0x%x Level=%d Result=0x%x Token=0x%x",
                  PP, OptGuarder, Level, Result, Token);

    status = ppoPREPROCESSOR_IsOpTokenInThisLevel(PP, Token, Level, &is_token_in_this_level); ppmCheckOK();

    if (is_token_in_this_level)
    {
        if (Token->poolString == PP->keyword->defined)
        {
            gctSTRING            id = gcvNULL;
            ppoMACRO_SYMBOL        ms = gcvNULL;

            status = ppoPREPROCESSOR_Defined(PP, &id); ppmCheckOK();
            if (id == PP->keyword->_file_
                ||    id == PP->keyword->_line_
                ||    id == PP->keyword->_version_
                ||    (id == PP->keyword->gl_es && (sloCOMPILER_GetClientApiVersion(PP->compiler) != gcvAPI_OPENGL))
                ||    id == PP->keyword->gl_core_profile
                ||    id == PP->keyword->gl_compatibility_profile)
            {
                if (id == PP->keyword->_version_ && PP->keyword->isVersionUndefined)
                    *Result = 0;
                else
                    *Result = 1;

                gcmFOOTER_ARG("*Result=%d", *Result);
                return gcvSTATUS_OK;
            }
            else
            {
                status = ppoMACRO_MANAGER_GetMacroSymbol(PP, PP->macroManager, id, &ms); ppmCheckOK();

                if ( ms == gcvNULL )
                {
                    *Result = gcvFALSE;
                }
                else
                {
                    *Result    = gcvTRUE;
                }

                gcmFOOTER_ARG("*Result=%d", *Result);
                return gcvSTATUS_OK;
            }
        }
        else
        {
            /*other unary op.*/

            status = ppoPREPROCESSOR_Eval(PP, OptGuarder, Level, EvaluateLine, MeetStringNum, &result); ppmCheckOK();

            if (Token->poolString == PP->keyword->positive)
            {
                *Result = result;
            }
            else if (Token->poolString == PP->keyword->negative)
            {
                *Result = -1 * result;
            }
            else if (Token->poolString == PP->keyword->banti)
            {
                *Result = ~result;
            }
            else if (Token->poolString == PP->keyword->lanti)
            {
                *Result= !result;
            }
            else
            {
                ppoPREPROCESSOR_Report(PP,slvREPORT_INTERNAL_ERROR, "The op inputStream not one of ~,!,+,-.");
                status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
                gcmFOOTER();
                return status;
            }
            gcmFOOTER_ARG("*Result=%d", *Result);
            return gcvSTATUS_OK;
        }
    }
    else /*if (is_token_in_this_level)*/
    {
        /*there is not a unary op, should be a expression*/
        status = ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), Token); ppmCheckOK();

        status = ppoPREPROCESSOR_Eval(PP, OptGuarder, Level+1, EvaluateLine, MeetStringNum, Result);
        gcmFOOTER();
        return status;
    }
}

gceSTATUS
ppoPREPROCESSOR_Eval_Binary_Op(
                               ppoPREPROCESSOR    PP,
                               gctSTRING          OptGuarder,
                               gctINT             Level,
                               gctINT*            Result,
                               gctBOOL            EvaluateLine,
                               gctBOOL            *MeetStringNum,
                               ppoTOKEN           Token
                               )
{
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    gctINT result = 0;

    gctBOOL is_token_in_this_level = gcvFALSE;

    gcmHEADER_ARG("PP=0x%x OptGuarder=0x%x Level=%d Result=0x%x Token=0x%x",
                  PP, OptGuarder, Level, Result, Token);

    status = ppoINPUT_STREAM_UnGetToken(PP,&(PP->inputStream), Token); ppmCheckOK();

    status = ppoPREPROCESSOR_Eval(PP, OptGuarder, Level+1, EvaluateLine, MeetStringNum, &result); ppmCheckOK();

    *Result    = result;

    status = ppoPREPROCESSOR_Eval_GetToken(PP, &Token, !ppvICareWhiteSpace); ppmCheckOK();

    ppoPREPROCESSOR_IsOpTokenInThisLevel(PP, Token, Level, &is_token_in_this_level);

    while(is_token_in_this_level)/*shift-reduce?*/
    {
        /*
        ** set skipOPError = gcvTRUE to skip undefined identifiers error,
        ** such as: #if 1 || AA  and # if 0 && AA
        */
        if((Token->poolString == PP->keyword->lor && *Result != 0) ||
           (Token->poolString == PP->keyword->land && *Result == 0) )
        {
            PP->skipOPError = gcvTRUE;
        }

        status = ppoPREPROCESSOR_Eval(PP, OptGuarder, Level+1, EvaluateLine, MeetStringNum, &result); ppmCheckOK();

        if(Token->poolString == PP->keyword->lor)
        {
            /*    00 ||    */
            *Result = ( (*Result) || (result) );
        }
        else if(Token->poolString == PP->keyword->land)
        {
            /*    01 &&    */
            *Result = ( (*Result) && (result) );
        }
        else if(Token->poolString == PP->keyword->bor)
        {
            /*    02 |    */
            *Result = ( (*Result) | (result) );
        }
        else if(Token->poolString == PP->keyword->bex)
        {
            /*    03 ^    */
            *Result = ( (*Result) ^ (result) );
        }
        else if(Token->poolString == PP->keyword->band)
        {
            /*    04 &    */
            *Result = ( (*Result) & (result) );
        }
        else if(Token->poolString == PP->keyword->equal)
        {
            /*    05 ==    */
            *Result = ( (*Result) == (result) );
        }
        else if(Token->poolString == PP->keyword->not_equal)
        {
            /*    06 !=    */
            *Result = ( (*Result) != (result) );
        }
        else if(Token->poolString == PP->keyword->less)
        {
            /*    07 <    */
            *Result = ( (*Result) < (result) );
        }
        else if(Token->poolString == PP->keyword->more)
        {
            /*    08 >    */
            *Result = ( (*Result) > (result) );
        }
        else if(Token->poolString == PP->keyword->more_equal)
        {
            /*    09 >=    */
            *Result = ( (*Result) >= (result) );
        }
        else if(Token->poolString == PP->keyword->less_equal)
        {
            /*    10 <=    */
            *Result = ( (*Result) <= (result) );
        }
        else if(Token->poolString == PP->keyword->lshift)
        {
            /*    11 <<    */
            *Result = ( (*Result) << (result) );
        }
        else if(Token->poolString == PP->keyword->rshift)
        {
            /*    12 >>    */
            *Result = ( (*Result) >> (result) );
        }
        else if(Token->poolString == PP->keyword->plus)
        {
            /*    13 +    */
            *Result = ( (*Result) + (result) );
        }
        else if(Token->poolString == PP->keyword->minus)
        {
            /*    14 -    */
            *Result = ( (*Result) - (result) );
        }
        else if(Token->poolString == PP->keyword->mul)
        {
            /*    15 *    */
            *Result = ( (*Result) * (result) );
        }
        else if(Token->poolString == PP->keyword->div)
        {
            /*    16 /    */
            if(result == 0)
            {
                ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR, "Can not divided by 0");
                                gcmFOOTER_NO();
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }
            *Result = ( (*Result) / (result) );
        }
        else if(Token->poolString == PP->keyword->perc)
        {
            /*    17 %    */
            if(result == 0)
            {
                ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR, "Can mod with 0");
                                gcmFOOTER_NO();
                return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            }
            *Result = ( (*Result) % (result) );
        }
        else
        {
            ppoPREPROCESSOR_Report(PP, slvREPORT_INTERNAL_ERROR, "ppoPREPROCESSOR_PPeval : Here should be a op above.");
                        gcmFOOTER_NO();
            return gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
        }

        status = ppoTOKEN_Destroy(PP, Token); ppmCheckOK();

        /*shift in one Token , and then maybe get more op*/

        status = ppoPREPROCESSOR_Eval_GetToken(PP, &Token, !ppvICareWhiteSpace); ppmCheckOK();

        status = ppoPREPROCESSOR_IsOpTokenInThisLevel(PP, Token, Level, &is_token_in_this_level);    ppmCheckOK();

    }/*while*/

    status = ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), Token); ppmCheckOK();

    status = ppoTOKEN_Destroy(PP, Token);

    if (gcmIS_SUCCESS(status))
    {
        gcmFOOTER_ARG("*Result=%d", *Result);
        return gcvSTATUS_OK;
    }
    else
    {
        gcmFOOTER();
        return status;
    }
}

gceSTATUS
ppoPREPROCESSOR_Eval(
                     ppoPREPROCESSOR    PP,
                     gctSTRING          OptGuarder,
                     gctINT             Level,
                     gctBOOL            EvaluateLine,
                     gctBOOL            *MeetStringNum,
                     gctINT*            Result
                     )
{
    gceSTATUS    status            =    gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;

    ppoTOKEN    token            =    gcvNULL;

    gcmHEADER_ARG("PP=0x%x OptGuarder=0x%x Level=%d Result=0x%x",
                  PP, OptGuarder, Level, Result);

    gcmASSERT(PP && 0 <= Level && Level <= 11);

    if (gcvFALSE == PP->doWeInValidArea)
    {
        status = ppoPREPROCESSOR_ToEOL(PP);
        gcmFOOTER();
        return status;
    }

    status = ppoPREPROCESSOR_Eval_GetToken(PP, &token, !ppvICareWhiteSpace);

    ppmCheckOK();

    if(PP->operators[Level] == gcvNULL)
    {
        /*Basicest Level.*/

        status = ppoPREPROCESSOR_Eval_Case_Basic_Level(PP, token, EvaluateLine, MeetStringNum, Result);

        ppmCheckOK();

        status = ppoTOKEN_Destroy(PP, token);
        if (gcmIS_SUCCESS(status))
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
        else
        {
            gcmFOOTER();
            return status;
        }
    }

    /*This inputStream not the last Level.*/
    do
    {
        if( PP->operators[Level][0] == gcmINT2PTR(1) )
        {
            status = ppoPREPROCESSOR_Eval_Case_Unary_Op(PP,
                                                        OptGuarder,
                                                        Level,
                                                        Result,
                                                        EvaluateLine,
                                                        MeetStringNum,
                                                        token);
            ppmCheckOK();
        }
        else if ( PP->operators[Level][0] == gcmINT2PTR(2) )
        {
            status = ppoPREPROCESSOR_Eval_Binary_Op(PP,
                                                    OptGuarder,
                                                    Level,
                                                    Result,
                                                    EvaluateLine,
                                                    MeetStringNum,
                                                    token);
            ppmCheckOK();
        }
        else
        {
            ppoPREPROCESSOR_Report(
                PP,
                slvREPORT_INTERNAL_ERROR,
                "The op should be either unary or binary.");
            status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
            gcmFOOTER();
            return status;
        }

    }while(gcvFALSE);

    status = ppoTOKEN_Destroy(PP, token); ppmCheckOK();

    status = ppoPREPROCESSOR_Eval_GetToken(PP, &token, !ppvICareWhiteSpace); ppmCheckOK();

    do
    {
        gctBOOL legal_token_apprear;

        status = ppoPREPROCESSOR_GuardTokenOfThisLevel(PP, token, OptGuarder, Level, &legal_token_apprear); ppmCheckOK();

        if(!legal_token_apprear)
        {
            if (EvaluateLine &&
                OptGuarder == PP->keyword->newline)

            {
                if (MeetStringNum)
                {
                    *MeetStringNum = gcvTRUE;
                }
                status = ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), token); ppmCheckOK();

                gcmFOOTER_NO();
                return gcvSTATUS_OK;
            }
            else
            {
                if(token->poolString == PP->keyword->newline)
                {
                    ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR, "Not expected token('NewLine') in  expression.");
                }
                else
                {
                    ppoPREPROCESSOR_Report(PP, slvREPORT_ERROR, "Not expected token('%s') in  expression.", token->poolString);
                }
                gcmFOOTER_NO();
                return sloCOMPILER_Free(PP->compiler, token);
            }
        }
    }
    while(gcvFALSE);

    status = ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), token); ppmCheckOK();

    status = ppoTOKEN_Destroy(PP, token);
    if (gcmIS_SUCCESS(status))
    {
        gcmFOOTER_NO();
        return gcvSTATUS_OK;
    }
    else
    {
        gcmFOOTER();
        return status;
    }
}

