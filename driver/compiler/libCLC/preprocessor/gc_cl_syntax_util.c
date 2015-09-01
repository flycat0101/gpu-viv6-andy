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


#include "gc_cl_preprocessor_int.h"

/******************************************************************************\
ToEOL
In    :    Any pos in the text.
Out    :    To the pos just pass the first NL following, or EOF.
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_ToEOL(ppoPREPROCESSOR PP)
{
    ppoTOKEN    ntoken = gcvNULL;

    gceSTATUS    status = gcvSTATUS_INVALID_ARGUMENT;

    ppmCheckFuncOk(
        PP->inputStream->GetToken(
        PP,
        &(PP->inputStream),
        &ntoken,
        !ppvICareWhiteSpace)
        );

    while(
        ntoken->poolString != PP->keyword->eof &&
        ntoken->poolString != PP->keyword->newline)
    {

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, gcvFALSE);            ppmCheckOK();

    }

    return ppoTOKEN_Destroy(PP, ntoken);
}
/******************************************************************************\
MatchDoubleToken
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_MatchDoubleToken(
    ppoPREPROCESSOR    PP,
    gctSTRING    NotWSStr1,
    gctSTRING    NotWSStr2,
    gctBOOL*    Match)
{
    ppoTOKEN    ntoken1 = gcvNULL;
    ppoTOKEN    ntoken2 = gcvNULL;
    gceSTATUS    status = gcvSTATUS_INVALID_ARGUMENT;

    ppmCheckFuncOk(
        ppoPREPROCESSOR_PassEmptyLine(PP)
        );

    ppmCheckFuncOk(
        PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken1, !ppvICareWhiteSpace)
        );

    ppmCheckFuncOk(
        PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken2, !ppvICareWhiteSpace)
        );

    ppmCheckFuncOk(
        ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken2)
        );

    ppmCheckFuncOk(
        ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken1)
        );

    if(    ntoken1->poolString == NotWSStr1 &&
        ntoken2->poolString == NotWSStr2)
    {
        *Match = gcvTRUE;

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken2)
            );

        return     ppoTOKEN_Destroy(PP, ntoken1);
    }
    else
    {
        *Match = gcvFALSE;

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken2)
            );

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken1)
            );

        return gcvSTATUS_OK;
    }
}


/******************************************************************************\
PassEmptyLine
In    :    Client should keep the read pos at the begin of a new line.
Out :    Server will put the pos to the next line which has some symbol
        other than NL as the first symbol in that line, or EOF.
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_PassEmptyLine(ppoPREPROCESSOR PP)
{
    ppoTOKEN    ntoken = gcvNULL;
    gceSTATUS    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);


    ppmCheckFuncOk(
        PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace)
        );

    while(ntoken->type != ppvTokenType_EOF && ntoken->poolString == PP->keyword->newline)
    {
        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

        ppmCheckFuncOk(
            PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, gcvFALSE)
            );
    }

    ppmCheckFuncOk(
        ppoINPUT_STREAM_UnGetToken(PP, &PP->inputStream, ntoken)
        );

    ppmCheckFuncOk(
        ppoTOKEN_Destroy(PP, ntoken)
        );

    return gcvSTATUS_OK;


}





/******************************************************************************\
Define Buffer RList
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_Define_BufferReplacementList(ppoPREPROCESSOR PP, ppoTOKEN* RList)
{
    ppoTOKEN    ntoken    = gcvNULL;
    ppoTOKEN    colon    = gcvNULL;
    ppoTOKEN    lastone    = gcvNULL;
    gceSTATUS    status    = gcvSTATUS_INVALID_ARGUMENT;

    gcmASSERT(RList);

    *RList = gcvNULL;

    status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);            ppmCheckOK();

    while(
        ntoken->poolString != PP->keyword->eof        &&
        ntoken->poolString != PP->keyword->newline)
    {
        colon = ntoken;

        /*Add this formal para to the para stack.*/
        if(*RList)
        {
            lastone->inputStream.base.node.prev    = (void*)colon;
            colon->inputStream.base.node.next        = (void*)lastone;
            colon->inputStream.base.node.prev        = gcvNULL;
            lastone = colon;
        }
        else
        {
            *RList = colon;
            lastone = colon;
        }
        /*Get a token, check.*/
        status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);        ppmCheckOK();

    }/*while*/

    ppmCheckFuncOk(
        ppoINPUT_STREAM_UnGetToken(PP, &(PP->inputStream), ntoken)
        );

    ppmCheckFuncOk(
        ppoTOKEN_Destroy(PP, ntoken)
        );

    return gcvSTATUS_OK;

}

/******************************************************************************\
Define BufferArgs
In    :    This function inputStream called just after the name of #define
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_Define_BufferArgs(ppoPREPROCESSOR PP, ppoTOKEN* args, gctINT* argc)
{
    ppoTOKEN ntoken = gcvNULL;
    ppoTOKEN colon    = gcvNULL;
    ppoTOKEN lastone    = gcvNULL;
    gctBOOL     boolean= gcvFALSE;
    gceSTATUS    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmASSERT( (*args) == gcvNULL && (*argc) == 0 );

    while(1)
    {
        status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);

        if(status != gcvSTATUS_OK){return status;}
        /*Get One Para*/
        if(ntoken->type != ppvTokenType_ID)
        {
            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR, "Id is expected.");
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        else
        {
            /*Check if this para inputStream already in the list*/
            status = ppoTOKEN_STREAM_FindID(PP, *args,ntoken->poolString, &boolean);
            if(status != gcvSTATUS_OK)
            {
                return status;
            }
            if(boolean)
            {
                ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
                    "The formal para name should not be the same.%s.",ntoken->poolString);
                return gcvSTATUS_INVALID_ARGUMENT;
            }

            colon = ntoken;

            /*Add this formal para to the para stack.*/
            if(*args)
            {
                lastone->inputStream.base.node.prev = (void*)colon;
                colon->inputStream.base.node.prev = gcvNULL;
                colon->inputStream.base.node.next = (void*)lastone;
                lastone = colon;
                ++(*argc);
            }
            else{
                *args = colon;
                lastone = *args;
                ++(*argc);
            }
        }
        status = PP->inputStream->GetToken(PP, &(PP->inputStream), &ntoken, !ppvICareWhiteSpace);        ppmCheckOK();

        if(ntoken->poolString != PP->keyword->comma)
        {
            if(ntoken->poolString != PP->keyword->rpara)
            {
                ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
                    "Need a ) here.");
                return gcvSTATUS_INVALID_ARGUMENT;
            }
            else
            {
                return ppoTOKEN_Destroy(PP, ntoken);
            }
        }

        ppmCheckFuncOk(
            ppoTOKEN_Destroy(PP, ntoken)
            );

    }/*while(1)*/
}
/******************************************************************************\
Buffer Actual Args
When meet a macro call in texline, we call this function to collect
the actual arguments of macro call.
(___,___,___)
__is what this function parse.
\******************************************************************************/
gceSTATUS
ppoPREPROCESSOR_BufferActualArgs(ppoPREPROCESSOR PP, ppoINPUT_STREAM *IS, ppoTOKEN* Head, ppoTOKEN* End)
{
    ppoTOKEN            ntoken        = gcvNULL;
    gctINT                locallevel    = 0;
    gceSTATUS            status        = gcvSTATUS_INVALID_DATA;

    *Head    = gcvNULL;
    *End    = gcvNULL;

    gcmASSERT(IS);

    if((*IS) == gcvNULL)
    {
        ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR, "unexpected end of file.");
        return gcvSTATUS_INVALID_DATA;
    }

    status = (*IS)->GetToken(PP, IS, &ntoken, !ppvICareWhiteSpace);if(status != gcvSTATUS_OK){return status;}

    while(1)
    {
        if(ntoken->poolString == PP->keyword->eof)
        {
            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR,
                "unexpected end of file.");

            ppmCheckFuncOk(
                ppoTOKEN_Destroy(PP, ntoken)
                );

            return gcvSTATUS_INVALID_DATA;
        }
        else    if(ntoken->poolString == PP->keyword->rpara && locallevel == 0)
        {
            if(*IS)
            {
                ppmCheckFuncOk(
                    ppoINPUT_STREAM_UnGetToken(PP, IS, ntoken)
                    );

                return ppoTOKEN_Destroy(PP, ntoken);
            }
            else
            {
                ntoken->inputStream.base.node.prev = gcvNULL;

                ntoken->inputStream.base.node.next = gcvNULL;

                (*IS)  = (ppoINPUT_STREAM)ntoken;

                return gcvSTATUS_OK;
            }
        }
        else if(ntoken->poolString == PP->keyword->comma && locallevel == 0)
        {
            if(*IS)
            {
                ppmCheckFuncOk(
                    ppoINPUT_STREAM_UnGetToken(PP, IS, ntoken)
                    );

                ppmCheckFuncOk(
                    ppoTOKEN_Destroy(PP, ntoken)
                    );
            }
            else
            {
                ntoken->inputStream.base.node.prev = gcvNULL;

                ntoken->inputStream.base.node.next = gcvNULL;

                (*IS)  = (ppoINPUT_STREAM)ntoken;
            }

            return gcvSTATUS_OK;

        }
        else if(ntoken->poolString == PP->keyword->lpara)
        {
            ++locallevel;
        }
        else if(ntoken->poolString == PP->keyword->rpara)
        {
            --locallevel;
        }

        if((*Head) == gcvNULL)
        {
            *End = *Head = ntoken;
        }
        else
        {
            (*End)->inputStream.base.node.prev = (void*)ntoken;

            ntoken->inputStream.base.node.prev = gcvNULL;

            ntoken->inputStream.base.node.next = (void*)(*End);

            *End = ntoken;
        }

        if(*IS)
        {
            status = (*IS)->GetToken(PP, IS, &ntoken, !ppvICareWhiteSpace);            ppmCheckOK();
        }
        else
        {
            ppmCheckFuncOk(
                ppoTOKEN_Destroy(PP, ntoken)
                );

            ppoPREPROCESSOR_Report(PP,clvREPORT_ERROR, "unexpected end of file.");

            return gcvSTATUS_INVALID_DATA;
        }
    }/*while(1)*/
}

