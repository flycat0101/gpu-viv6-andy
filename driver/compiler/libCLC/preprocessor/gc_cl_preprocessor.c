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
#include <stdio.h>

/*******************************************************************************
**
**    Header File Related Function
**
*/
gceSTATUS
ppoPREPROCESSOR_AddHeaderFilePathToList(
    IN ppoPREPROCESSOR PP,
    IN gctSTRING   Path
    )
{
    gceSTATUS status = gcvSTATUS_INVALID_DATA;
    ppoHEADERFILEPATH headerFilePathNode = gcvNULL;
    ppoHEADERFILEPATH headerFilePathtemp = gcvNULL;

    /* Verify the arguments. */
    gcmASSERT(PP && Path);

    status = cloCOMPILER_ZeroMemoryAllocate(
                                            PP->compiler,
                                            sizeof(struct _ppoHEADERFILEPATH),
                                            (gctPOINTER *) &headerFilePathNode
                                            );
    if (gcmIS_ERROR(status))
    {
        return status;
    }
    headerFilePathNode->headerFilePath = Path;

    if (PP->headerFilePathList)
    {
        headerFilePathtemp = PP->headerFilePathList;

        while (headerFilePathtemp->base.node.next != gcvNULL)
        {
            headerFilePathtemp = (void*)headerFilePathtemp->base.node.next;
        }
        headerFilePathtemp->base.node.next = (void*)headerFilePathNode;
        headerFilePathNode->base.node.prev = (void*)(headerFilePathtemp);
        headerFilePathNode->base.node.next = gcvNULL;
        headerFilePathtemp = headerFilePathNode;
    }
    else
    {
        PP->headerFilePathList = headerFilePathNode;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
ppoPREPROCESSOR_FreeHeaderFilePathList(
    IN ppoPREPROCESSOR PP
    )
{
    gceSTATUS status = gcvSTATUS_INVALID_DATA;
    ppoHEADERFILEPATH headerFilePathNode = gcvNULL;

    if (PP && PP->headerFilePathList != gcvNULL)
    {
        /* Destroy header file path */
        headerFilePathNode = PP->headerFilePathList;
        while (headerFilePathNode != gcvNULL)
        {
            status = cloCOMPILER_Free(PP->compiler, headerFilePathNode->headerFilePath);
            if (status != gcvSTATUS_OK) return status;
            headerFilePathNode->headerFilePath = gcvNULL;
            headerFilePathNode = (void*)headerFilePathNode->base.node.next;
        }

        /* Destroy headerFilePathList */
        status = cloCOMPILER_Free(PP->compiler, PP->headerFilePathList);
        if (status != gcvSTATUS_OK) return status;
        PP->headerFilePathList = gcvNULL;
    }

    return gcvSTATUS_OK;
}

gceSTATUS ppoPREPROCESSOR_SetDebug(
    IN cloCOMPILER        Compiler,
    IN gctUINT            On
    )
{
    return gcvSTATUS_OK;
}
gceSTATUS ppoPREPROCESSOR_SetOptimize(
    IN cloCOMPILER        Compiler,
    IN gctUINT            On
    )
{
    return gcvSTATUS_OK;
}
gceSTATUS ppoPREPROCESSOR_SetVersion(
    IN cloCOMPILER        Compiler,
    IN gctUINT            Version
    )
{
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**    ppoPREPROCESSOR_PushOntoCurrentInputStreamOfPP
**
*/
gceSTATUS
ppoPREPROCESSOR_PushOntoCurrentInputStreamOfPP (ppoPREPROCESSOR PP,
                                                ppoINPUT_STREAM IS)
{
    gcmASSERT(PP && IS);

    if (PP->inputStream)
    {
        IS->base.node.prev = (void*)PP->inputStream;

        PP->inputStream->base.node.next = (void*)IS;

        PP->inputStream = IS;

        IS->base.node.next = gcvNULL;
    }
    else
    {
        PP->inputStream = IS;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
ppoWriteBufferToFile(IN ppoPREPROCESSOR PP)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (PP->ppLogFile == gcvNULL)
    {
#define _FILENAMEMAX 1024
        gctCHAR fullFileName[_FILENAMEMAX+1];
        gctCHAR fileName[64];
        gctUINT offset = 0;
        gctUINT64 time;

        gcmVERIFY_OK(vscGetTemporaryDir(fullFileName));
#if _WIN32
        gcmVERIFY_OK(gcoOS_StrCatSafe(fullFileName, _FILENAMEMAX, "\\"));
#else
        gcmVERIFY_OK(gcoOS_StrCatSafe(fullFileName, _FILENAMEMAX, "/"));
#endif
        gcoOS_GetTime(&time);
        gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName, gcmSIZEOF(fileName), &offset, "viv_cl_%lld.log", (gctUINT64)time));
        gcmVERIFY_OK(gcoOS_StrCatSafe(fullFileName, _FILENAMEMAX, fileName));
        gcmVERIFY_OK(gcoOS_Open(gcvNULL, fullFileName, gcvFILE_APPENDTEXT, &PP->ppLogFile));
        if (PP->ppLogFile == gcvNULL)
        {
            gcoOS_ZeroMemory(PP->logBuffer, 1024);
            PP->logCurrentSize = 0;
            return gcvSTATUS_INVALID_DATA;
        }
    }

    gcoOS_Write(gcvNULL, PP->ppLogFile, gcoOS_StrLen(PP->logBuffer, gcvNULL), PP->logBuffer);
    gcoOS_ZeroMemory(PP->logBuffer, 1024);
    PP->logCurrentSize = 0;

    return status;
}

/*******************************************************************************
**
**    ppoPREPROCESSOR_AddToOutputStreamOfPP
**
*/
gceSTATUS
ppoPREPROCESSOR_AddToOutputStreamOfPP(ppoPREPROCESSOR PP,
                                      ppoTOKEN Token)
{
    ppoTOKEN ntoken = gcvNULL;
    gceSTATUS status = gcvSTATUS_INVALID_ARGUMENT;
    gctSIZE_T len = 0;

    gcmASSERT(PP && Token);

    status = ppoTOKEN_Colon(
        PP,
        Token,
        __FILE__,
        __LINE__,
        "Dump for adding this token to the output of cpp.",
        &ntoken);                                                                ppmCheckOK();

    ntoken->srcFileString = PP->currentSourceFileStringNumber;

    ntoken->srcFileLine = PP->currentSourceFileLineNumber;

    if ((PP->outputTokenStreamEnd == PP->outputTokenStreamHead) &&
        (PP->outputTokenStreamEnd == gcvNULL))
    {
        /*empty list*/

        PP->outputTokenStreamEnd = PP->outputTokenStreamHead = ntoken;

        ntoken->inputStream.base.node.prev = gcvNULL;

        ntoken->inputStream.base.node.next = gcvNULL;
    }
    else
    {
        /*not empty list, link the ntoken to the prev of the end one.*/

        ntoken->inputStream.base.node.next = (void*)PP->outputTokenStreamEnd;

        ntoken->inputStream.base.node.prev = gcvNULL;

        PP->outputTokenStreamEnd->inputStream.base.node.prev = (void*)ntoken;

        PP->outputTokenStreamEnd = ntoken;
    }

    if (gcmOPT_DUMP_PPEDSTR2FILE())
    {
        gcoOS_StrLen(ntoken->poolString, &len);
        if (((PP->logCurrentSize + len) > (_cldBUFFER_MAX - 5)))
        {
            ppoWriteBufferToFile(PP);
        }

        if (PP->ppLineNumber && PP->ppLineNumber < PP->currentSourceFileLineNumber)
        {
            gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, "\n");
            if (ntoken->hasLeadingWS)
            {
                gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, " "));
                PP->logCurrentSize = PP->logCurrentSize + 2;
            }

            gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer,
                                          _cldBUFFER_MAX,
                                          ntoken->poolString));
            PP->logCurrentSize = PP->logCurrentSize + len + 1;

            if (ntoken->hasTrailingControl)
            {
                gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, " "));
                PP->logCurrentSize = PP->logCurrentSize + 2;
            }
        }
        else
        {
            if (ntoken->hasLeadingWS)
            {
                gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, " "));
                PP->logCurrentSize = PP->logCurrentSize + 2;
            }

            status = gcoOS_StrCatSafe(PP->logBuffer,
                                      _cldBUFFER_MAX,
                                      ntoken->poolString);
            PP->logCurrentSize = PP->logCurrentSize + len + 1;

            if (ntoken->hasTrailingControl)
            {
                gcmVERIFY_OK(gcoOS_StrCatSafe(PP->logBuffer, _cldBUFFER_MAX, " "));
                PP->logCurrentSize = PP->logCurrentSize + 2;
            }
        }
        PP->ppLineNumber = PP->currentSourceFileLineNumber;
    }

    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**    ppoPREPROCESSOR_DumpOutputStream
**
*/
gceSTATUS
ppoPREPROCESSOR_DumpOutputStream(ppoPREPROCESSOR PP)
{
    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    if (PP->outputTokenStreamHead)
    {
        return ppoTOKEN_STREAM_Dump(PP, PP->outputTokenStreamHead);
    }

    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**    ppoPREPROCESSOR_Report
**
*/
gceSTATUS
ppoPREPROCESSOR_Report(ppoPREPROCESSOR PP,
                       cleREPORT_TYPE Type,
                       gctCONST_STRING Message,
                       ...)
{
    gceSTATUS    status;
    gctARGUMENTS arguments;

    gcmARGUMENTS_START(arguments, Message);
    status = cloCOMPILER_VReport(
        PP->compiler,
        PP->currentSourceFileLineNumber,
        PP->currentSourceFileStringNumber,
        Type,
        Message,
        arguments);
    gcmARGUMENTS_END(arguments);

    return status;
}
/*
x * x ...  x * x
----------------
total = y
*/
gctINT ppoPREPROCESSOR_Pow(gctINT x, gctINT y)
{
    gctINT rt    = 1;

    gctINT i    = 0;

    gcmASSERT( x >= 0 && y >= 0 );

    for ( i=1; i<=y; i++ )
    {
        rt = rt * x;
    }

    return rt;
}




gceSTATUS
ppoPREPROCESSOR_Construct_InitKeyword(cloCOMPILER Compiler, ppoPREPROCESSOR    *PP);

gceSTATUS
ppoPREPROCESSOR_Construct_InitOperator(cloCOMPILER Compiler, ppoPREPROCESSOR    *PP);


/*******************************************************************************
**
**    ppoPREPROCESSOR_Construct
**
**        Allocate memory, set undirty.
**
*/
gceSTATUS
ppoPREPROCESSOR_Construct(
cloCOMPILER Compiler,
ppoPREPROCESSOR    *PP,
gctBOOL UseNewPP
)
{

    gceSTATUS status = gcvSTATUS_INVALID_DATA;
    gctPOINTER pointer = gcvNULL;

    do
    {
        /*preprocessor*/
        status = cloCOMPILER_Allocate(Compiler,
                          sizeof(struct _ppoPREPROCESSOR),
                          (gctPOINTER *)PP);
        if (status != gcvSTATUS_OK) break;

        gcoOS_MemFill(*PP,
                      0,/*set value*/
                      sizeof(struct _ppoPREPROCESSOR));
        (*PP)->compiler = Compiler;

        (*PP)->base.file = __FILE__;
        (*PP)->base.info = "Created in ppoPREPROCESSOR_Construct";
        (*PP)->base.line = __LINE__;
        (*PP)->base.node.next = gcvNULL;
        (*PP)->base.node.prev = gcvNULL;
        (*PP)->base.type = ppvOBJ_PREPROCESSOR;
        (*PP)->useNewPP = UseNewPP;

        if ((*PP)->useNewPP)
        {
            /* TODO: Construct extension string. */
            /*_ConstructExtensionString(Compiler, PP);*/

            /* Keywords. */
            gcmONERROR(
                cloCOMPILER_Allocate(
                    Compiler,
                    sizeof (struct _ppsKEYWORD),
                    &pointer
                    ));

            (*PP)->keyword = pointer;

            (*PP)->macroString = gcvNULL;
            (*PP)->macroStringSize = 0;

            gcmONERROR(
                ppoPREPROCESSOR_Construct_InitKeyword(Compiler, PP));

            /* Operators. */
            gcmONERROR(
                cloCOMPILER_Allocate(
                    (*PP)->compiler,
                    sizeof(gctSTRING*)*12,
                    &pointer
                    ) );
            (*PP)->operators = pointer;

            gcoOS_MemFill(
                (*PP)->operators,
                (gctUINT8)0,
                sizeof(gctSTRING*)*12
                );

            gcmONERROR(
                ppoPREPROCESSOR_Construct_InitOperator(Compiler, PP));
        }
        return gcvSTATUS_OK;
    }
    while(gcvFALSE);

OnError:
    cloCOMPILER_Report(Compiler,
               1,
               0,
               clvREPORT_FATAL_ERROR,
               "Failed to start preprocessing.");
    return status;
}


gceSTATUS
ppoPREPROCESSOR_Construct_InitKeyword(cloCOMPILER        Compiler,
                                      ppoPREPROCESSOR    *PP)
{

    gceSTATUS status = gcvSTATUS_INVALID_DATA;

    do
    {
        /*00    #*/
        status = cloCOMPILER_AllocatePoolString(
            (*PP)->compiler,
            "#",
            &((*PP)->keyword->sharp)
            );


        if (status != gcvSTATUS_OK) break;


        /*01    define*/
        status = cloCOMPILER_AllocatePoolString(
            (*PP)->compiler,
            "define",
            &((*PP)->keyword->define)
            );

        if (status != gcvSTATUS_OK) break;

        /*02    undef*/
        status = cloCOMPILER_AllocatePoolString(
            (*PP)->compiler,
            "undef",
            &((*PP)->keyword->undef));

        if (status != gcvSTATUS_OK) break;

        /*03    if*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "if",
            &((*PP)->keyword->if_));

        if (status != gcvSTATUS_OK) break;

        /*04    ifdef*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "ifdef",
            &((*PP)->keyword->ifdef));

        if (status != gcvSTATUS_OK) break;

        /*05    ifndef*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "ifndef",
            &((*PP)->keyword->ifndef));

        if (status != gcvSTATUS_OK) break;

        /*06    ifndef*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "else",
            &((*PP)->keyword->else_));

        if (status != gcvSTATUS_OK) break;

        /*07    ifndef*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "elif",
            &((*PP)->keyword->elif));

        if (status != gcvSTATUS_OK) break;

        /*08    ifndef*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "endif",
            &((*PP)->keyword->endif));

        if (status != gcvSTATUS_OK) break;

        /*09    ifndef*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "error",
            &((*PP)->keyword->error));

        if (status != gcvSTATUS_OK) break;

        /*10    pragma*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "pragma",
            &((*PP)->keyword->pragma));

        if (status != gcvSTATUS_OK) break;

        /*11    ifndef*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "extension",
            &((*PP)->keyword->extension));

        if (status != gcvSTATUS_OK) break;

        /*12    version*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "version",
            &((*PP)->keyword->version));

        if (status != gcvSTATUS_OK) break;

        /*13    line*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "line",
            &((*PP)->keyword->line));

        if (status != gcvSTATUS_OK) break;

        /*14    lpara*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "(",
            &((*PP)->keyword->lpara ));

        if (status != gcvSTATUS_OK) break;

        /*15    rpapa*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            ")",
            &((*PP)->keyword->rpara));

        if (status != gcvSTATUS_OK) break;

        /*16    newline*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "\n",
            &((*PP)->keyword->newline ));

        if (status != gcvSTATUS_OK) break;

        /*17    defined*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "defined",
            &((*PP)->keyword->defined));

        if (status != gcvSTATUS_OK) break;

        /*18    minus*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "-",
            &((*PP)->keyword->minus));

        if (status != gcvSTATUS_OK) break;

        /*19    plus*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "+",
            &((*PP)->keyword->plus));

        if (status != gcvSTATUS_OK) break;

        /*20    ||*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "||",
            &((*PP)->keyword->lor));

        if (status != gcvSTATUS_OK) break;

        /*21    &&*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "&&",
            &((*PP)->keyword->land));

        if (status != gcvSTATUS_OK) break;

        /*22    |*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "|",
            &((*PP)->keyword->bor));

        if (status != gcvSTATUS_OK) break;

        /*23    &*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "&",
            &((*PP)->keyword->band ));

        if (status != gcvSTATUS_OK) break;

        /*24    ==*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "==",
            &((*PP)->keyword->equal));

        if (status != gcvSTATUS_OK) break;

        /*25    !=*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "!=",
            &((*PP)->keyword->not_equal ));

        if (status != gcvSTATUS_OK) break;

        /*26    >*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            ">",
            &((*PP)->keyword->more));


        if (status != gcvSTATUS_OK) break;

        /*27    <*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "<",
            &((*PP)->keyword->less));

        if (status != gcvSTATUS_OK) break;

        /*28    >=*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            ">=",
            &((*PP)->keyword->more_equal ));

        if (status != gcvSTATUS_OK) break;

        /*29    <=*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "<=",
            &((*PP)->keyword->less_equal ));

        if (status != gcvSTATUS_OK) break;

        /*30    <<*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "<<",
            &((*PP)->keyword->lshift ));

        if (status != gcvSTATUS_OK) break;

        /*31    >>*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            ">>",
            &((*PP)->keyword->rshift));

        if (status != gcvSTATUS_OK) break;

        /*32    **/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "*",
            &((*PP)->keyword->mul));

        if (status != gcvSTATUS_OK) break;

        /*33    / */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "/",
            &((*PP)->keyword->div));

        if (status != gcvSTATUS_OK) break;

        /*34    %*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "%",
            &((*PP)->keyword->perc));

        if (status != gcvSTATUS_OK) break;

        /*35    +*/
        (*PP)->keyword->positive = (*PP)->keyword->plus;
        /*36    -*/
        (*PP)->keyword->negative = (*PP)->keyword->minus;
        /*37    ~*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "~",
            &((*PP)->keyword->banti ));

        if (status != gcvSTATUS_OK) break;

        /*38    !*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "!",
            &((*PP)->keyword->lanti));

        if (status != gcvSTATUS_OK) break;

        /*39    ^*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "^",
            &((*PP)->keyword->bex));

        if (status != gcvSTATUS_OK) break;

        /*40    EOF*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "<EOF>",
            &((*PP)->keyword->eof));

        if (status != gcvSTATUS_OK) break;

        /*41    WS */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "<WS>",
            &((*PP)->keyword->ws));

        if (status != gcvSTATUS_OK) break;

        /*42    , */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            ",",
            &((*PP)->keyword->comma));

        if (status != gcvSTATUS_OK) break;

        /*43    version */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "100",
            &((*PP)->keyword->version_100));

        if (status != gcvSTATUS_OK) break;

        /*44    :    */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            ":",
            &((*PP)->keyword->colon));

        if (status != gcvSTATUS_OK) break;

        /*45    require    */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "require",
            &((*PP)->keyword->require));

        if (status != gcvSTATUS_OK) break;

        /*46    enable    */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "enable",
            &((*PP)->keyword->enable));

        if (status != gcvSTATUS_OK) break;

        /*47    warn    */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "warn",
            &((*PP)->keyword->warn));

        if (status != gcvSTATUS_OK) break;

        /*48    disable    */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "disable",
            &((*PP)->keyword->disable));

        if (status != gcvSTATUS_OK) break;

        /*50    __LINE__*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "__LINE__",
            &((*PP)->keyword->_line_));

        if (status != gcvSTATUS_OK) break;

        /*51    __FILE__*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "__FILE__",
            &((*PP)->keyword->_file_));

        if (status != gcvSTATUS_OK) break;

        /*52    __VERSION__*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "__VERSION__",
            &((*PP)->keyword->_version_));

        if (status != gcvSTATUS_OK) break;

        /*53    GL_ES*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "GL_ES",
            &((*PP)->keyword->gl_es));

        if (status != gcvSTATUS_OK) break;

        /*54    GL_*/
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "GL_",
            &((*PP)->keyword->gl_));

        if (status != gcvSTATUS_OK) break;

        /*55    all        */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "all",
            &((*PP)->keyword->all));

        /*56    include        */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "include",
            &((*PP)->keyword->include));

        /*57    nul_str     */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
                                       "",
                                       &((*PP)->keyword->nul_str));

        if (status != gcvSTATUS_OK) break;

        return gcvSTATUS_OK;

    }
    while(gcvFALSE);

    cloCOMPILER_Report(
        Compiler,
        1,
        0,
        clvREPORT_FATAL_ERROR,
        "Failed to start preprocessing.");

    return status;

}

gceSTATUS
ppoPREPROCESSOR_Construct_InitOperator(cloCOMPILER        Compiler,
                                       ppoPREPROCESSOR    *PP)
{

    gceSTATUS status = gcvSTATUS_INVALID_DATA;

    do
    {
        /*(*PP)->operators 00:    Total: 3, Detail: 2, ||, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[0])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[0][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[0][1] = (*PP)->keyword->lor;

        (*PP)->operators[0][2] = gcvNULL;

        /*(*PP)->operators 01:    Total: 3, Detail: 2, &&, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[1])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[1][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[1][1] = (*PP)->keyword->land;

        (*PP)->operators[1][2] = gcvNULL;

        /*(*PP)->operators 02:    Total: 3, Detail: 2, |, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[2])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[2][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[2][1] = (*PP)->keyword->bor;

        (*PP)->operators[2][2] = gcvNULL;

        /*(*PP)->operators 03:    Total: 3, Detail: 2, ^, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[3])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[3][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[3][1] = (*PP)->keyword->bex;

        (*PP)->operators[3][2] = gcvNULL;

        /*(*PP)->operators 04:    Total: 3, Detail: 2, &, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[4])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[4][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[4][1] = (*PP)->keyword->band;

        (*PP)->operators[4][2] = gcvNULL;

        /*(*PP)->operators 05:    Total: 4, Detail: 2, ==, !=, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*4,
            (gctPOINTER*)&((*PP)->operators[5])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[5][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[5][1] = (*PP)->keyword->equal;

        (*PP)->operators[5][2] = (*PP)->keyword->not_equal;

        (*PP)->operators[5][3] = gcvNULL;

        /*(*PP)->operators 06:    Total: 6, Detail: 2, >, <, >=, <=, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*6,
            (gctPOINTER*)&((*PP)->operators[6])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[6][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[6][1] = (*PP)->keyword->less;

        (*PP)->operators[6][2] = (*PP)->keyword->more;

        (*PP)->operators[6][3] = (*PP)->keyword->less_equal;

        (*PP)->operators[6][4] = (*PP)->keyword->more_equal;

        (*PP)->operators[6][5] = gcvNULL;

        /*(*PP)->operators 07:    Total: 4, Detail: 2, <<, >>, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*4,
            (gctPOINTER*)&((*PP)->operators[7])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[7][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[7][1] = (*PP)->keyword->lshift;

        (*PP)->operators[7][2] = (*PP)->keyword->rshift;

        (*PP)->operators[7][3] = gcvNULL;

        /*(*PP)->operators 08:    Total: 4, Detail: 2, +, -, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*4,
            (gctPOINTER*)&((*PP)->operators[8])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[8][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[8][1] = (*PP)->keyword->plus;

        (*PP)->operators[8][2] = (*PP)->keyword->minus;

        (*PP)->operators[8][3] = gcvNULL;

        /*(*PP)->operators 09:    Total: 5, Detail: 2, *, /, %, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*5,
            (gctPOINTER*)&((*PP)->operators[9])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[9][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[9][1] = (*PP)->keyword->mul;

        (*PP)->operators[9][2] = (*PP)->keyword->div;

        (*PP)->operators[9][3] = (*PP)->keyword->perc;

        (*PP)->operators[9][4] = gcvNULL;

        /*(*PP)->operators 10:    Total: 8, Detail: 1, +, -, ~, !, defined, 0*/

        status    = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*8,
            (gctPOINTER*)&((*PP)->operators[10])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[10][0] = (void*)ppvUNARY_OP;

        (*PP)->operators[10][1] = (*PP)->keyword->plus;

        (*PP)->operators[10][2] = (*PP)->keyword->minus;

        (*PP)->operators[10][3] = (*PP)->keyword->div;

        (*PP)->operators[10][4] = (*PP)->keyword->banti;

        (*PP)->operators[10][5] = (*PP)->keyword->lanti;

        (*PP)->operators[10][6] = (*PP)->keyword->defined;

        (*PP)->operators[10][7] = gcvNULL;

        /*(*PP)->operators 11:    Total: 0. This inputStream just a guarder.*/

        (*PP)->operators[11] = gcvNULL;

        return gcvSTATUS_OK;

    }while(gcvFALSE);

    cloCOMPILER_Report(
        Compiler,
        1,
        0,
        clvREPORT_FATAL_ERROR,
        "Failed to start preprocessing.");

    return status;

}




/*******************************************************************************
**
**    ppoPREPROCESSOR_Destroy
**
**        Release memory used by internal elements of *PP*, and *PP*.
**
*/
gceSTATUS
ppoPREPROCESSOR_Destroy (ppoPREPROCESSOR PP)
{

    gceSTATUS status = gcvSTATUS_INVALID_DATA;
    cloCOMPILER    compiler = gcvNULL;
    gctUINT i;

    gcmASSERT(PP->base.type == ppvOBJ_PREPROCESSOR);
    compiler = PP->compiler;

    do
    {
        ppoPREPROCESSOR_Reset(PP);

        if (PP->useNewPP)
        {
            if (PP->extensionString)
            {
                gcmONERROR(cloCOMPILER_Free(PP->compiler, (gctPOINTER)PP->extensionString));
                PP->extensionString = gcvNULL;
            }
            if (PP->macroString)
            {
                gcmONERROR(cloCOMPILER_Free(PP->compiler, (gctPOINTER)PP->macroString));
                PP->macroString = gcvNULL;
            }
            /*release operators of every level*/
            i = 0;
            while (PP->operators[i] != gcvNULL)
            {
                gcmONERROR(cloCOMPILER_Free(
                    compiler,
                    PP->operators[i]
                    ));
                i++;
            }

            gcmONERROR(cloCOMPILER_Free(
                    compiler,
                    PP->operators
                    ));

            /*total number of group of operator, is 12.*/
            gcmASSERT(i == 11);

            /*release output stream*/
            gcmONERROR(ppoTOKEN_STREAM_Destroy(
                PP,
                PP->outputTokenStreamHead
                ));

            /*release keyword*/
            gcmONERROR(cloCOMPILER_Free(
                compiler,
                PP->keyword
                ));

            /* release header file path list */
            gcmONERROR(ppoPREPROCESSOR_FreeHeaderFilePathList(PP));
        }

        /*PP*/
        status = cloCOMPILER_Free(compiler,
                      PP);
        if (status != gcvSTATUS_OK) break;

        return gcvSTATUS_OK;
    }
    while(gcvFALSE);

OnError:
    cloCOMPILER_Report(compiler,
               0,
               0,
               clvREPORT_INTERNAL_ERROR,
               "Error in destroy preprocessor.");
    return status;
}



/*******************************************************************************
**
**    ppoPREPROCESSOR_Dump
**
**        Dump.
**
*/
gceSTATUS    ppoPREPROCESSOR_Dump(ppoPREPROCESSOR PP)
{
    gceSTATUS status = gcvSTATUS_INVALID_ARGUMENT;

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);


    do
    {

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<PP>"
            );
        if (status != gcvSTATUS_OK) break;

        status = ppoBASE_Dump(
            PP,
            (ppoBASE)PP
            );
        if (status != gcvSTATUS_OK) break;

        status = ppoMACRO_MANAGER_Dump(
            PP,
            PP->macroManager
            );
        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<TheInputStack>"
            );
        if (status != gcvSTATUS_OK) break;

        if(PP->inputStream)
        {
            status = ppoINPUT_STREAM_Dump(
                PP,
                PP->inputStream
                );
            if (status != gcvSTATUS_OK) break;
        }

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "</TheInputStack>"
            );
        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<LastTokenString no=\"%d\" />",PP->currentSourceFileStringNumber
            );
        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<LastTokenLine no=\"%d\" />",
            PP->currentSourceFileLineNumber
            );
        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<OutputList>"
            );
        if (status != gcvSTATUS_OK) break;

        if(PP->outputTokenStreamHead)
        {
            status = ppoTOKEN_STREAM_Dump(
                PP,
                PP->outputTokenStreamHead
                );
            if (status != gcvSTATUS_OK) break;
        }

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<OutputList>"
            );
        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<Version version=\"%d\" />",
            PP->version
            );
        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<Legal doWeInValidArea=\"%d\" />", PP->doWeInValidArea
            );
        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<HasAnyStatementOtherThanVersionStatementHaveAppeared appeared=\"%d\" />",
            PP->otherStatementHasAlreadyAppeared
            );
        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "<HasVersionStatementHaveAppeared appeared=\"%d\" />",
            PP->versionStatementHasAlreadyAppeared
            );
        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_Dump(
            PP->compiler,
            clvDUMP_PREPROCESSOR,
            "</PP>"
            );
        if (status != gcvSTATUS_OK) break;

        return gcvSTATUS_OK;
    }
    while(gcvFALSE);

    cloCOMPILER_Report(
            PP->compiler,
            0,
            0,
            clvREPORT_FATAL_ERROR,
            "Error in dumping preprocessor."
            );

    return status;
}

/*******************************************************************************
**
**    ppoPREPROCESSOR_Parse
**
**        Parse the strings just set.
**
**
*/
gceSTATUS    ppoPREPROCESSOR_Parse(cloPREPROCESSOR        PP,
                                  char                    *Buffer,
                                  gctUINT                Max,
                                  gctINT                *WriteInNumber)
{
    gceSTATUS            status    = gcvSTATUS_INVALID_ARGUMENT;
    ppoTOKEN            ntoken    = gcvNULL;
    gctSIZE_T            len        = 0;

    /*legal argument?*/
    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    *WriteInNumber = 0;

    /*end of input?*/
    if(PP->inputStream == gcvNULL)
    {
        /* flush the remaining log buffer to file and close the file */
        if (gcmOPT_DUMP_PPEDSTR2FILE())
        {
            if (PP->logBuffer[0] != '\0')
                ppoWriteBufferToFile(PP);
            if (PP->ppLogFile)
            {
                gcoOS_Close(gcvNULL, PP->ppLogFile);
                PP->ppLogFile = gcvNULL;
            }
        }
        return gcvSTATUS_OK;
    }

    do
    {
        if (PP->outputTokenStreamHead == gcvNULL)
        {
            status = PP->inputStream->GetToken(
                PP,
                &(PP->inputStream),
                &ntoken,
                !ppvICareWhiteSpace);
            if (status != gcvSTATUS_OK) break;

            if (ntoken->type == ppvTokenType_EOF)
            {
                /* flush the remaining log buffer to file and close the file */
                if (gcmOPT_DUMP_PPEDSTR2FILE())
                {
                    if (PP->logBuffer[0] != '\0')
                        ppoWriteBufferToFile(PP);
                    if (PP->ppLogFile)
                    {
                        gcoOS_Close(gcvNULL, PP->ppLogFile);
                        PP->ppLogFile = gcvNULL;
                    }
                }
                status = ppoTOKEN_Destroy(PP, ntoken);
                return status;
            }

            status = ppoINPUT_STREAM_UnGetToken(
                PP,
                &(PP->inputStream),
                ntoken);
            if (status != gcvSTATUS_OK) break;

            status = ppoTOKEN_Destroy(PP, ntoken);
            if (status != gcvSTATUS_OK) break;

            status = ppoPREPROCESSOR_PreprocessingFile(PP);

            if (status != gcvSTATUS_OK) break;

            if(PP->outputTokenStreamHead == gcvNULL)
            {
                *WriteInNumber = -1;
                return gcvSTATUS_OK;
            }
        }

        gcmASSERT( PP->outputTokenStreamHead != gcvNULL );

        /*copy a string to *Buffer*.*/
        len = gcoOS_StrLen(PP->outputTokenStreamHead->poolString, gcvNULL);
        len += PP->outputTokenStreamHead->hasLeadingWS ? 1 : 0;
        len += PP->outputTokenStreamHead->hasTrailingControl ? 1 : 0;

        if(len >= Max)
        {
            cloCOMPILER_Report(
                PP->compiler,
                PP->outputTokenStreamHead->srcFileLine,
                PP->outputTokenStreamHead->srcFileString,
                clvREPORT_ERROR,
                "The token is too long for compiler : %s,"
                "max length : %u",
                PP->outputTokenStreamHead->poolString,
                Max);
            status = gcvSTATUS_INVALID_DATA;
            break;
        }
        else
        {
            *WriteInNumber = len;
        }

        if (PP->outputTokenStreamHead->hasLeadingWS)
        {
           status = gcoOS_StrCopySafe(Buffer, Max, " ");
           status = gcoOS_StrCatSafe(Buffer,
                                       Max - 1,
                                       PP->outputTokenStreamHead->poolString);
        }
        else
        {
           status = gcoOS_StrCopySafe(Buffer,
                                        Max,
                                        PP->outputTokenStreamHead->poolString);
        }

        if (PP->outputTokenStreamHead->srcFileString == 0)
        {
            cloCOMPILER_SetIsMainFile(PP->compiler, gcvTRUE);
        }
        else
        {
            cloCOMPILER_SetIsMainFile(PP->compiler, gcvFALSE);
        }

        if (PP->outputTokenStreamHead->hasTrailingControl)
        {
           status = gcoOS_StrCatSafe(Buffer, Max, " ");
        }

        if (status != gcvSTATUS_OK) break;

        /*set position*/
        status = cloCOMPILER_SetCurrentStringNo(
            PP->compiler,
            (gctUINT)PP->outputTokenStreamHead->srcFileString
            );

        if (status != gcvSTATUS_OK) break;

        status = cloCOMPILER_SetCurrentLineNo(
            PP->compiler,
            PP->outputTokenStreamHead->srcFileLine
            );

        if (status != gcvSTATUS_OK) break;

        /*remove head*/
        if(PP->outputTokenStreamHead == PP->outputTokenStreamEnd)
        {
            status = ppoTOKEN_Destroy(
                PP,
                PP->outputTokenStreamHead
                );
            if (status != gcvSTATUS_OK) break;

            PP->outputTokenStreamHead = PP->outputTokenStreamEnd = gcvNULL;
        }
        else
        {
            ppoTOKEN prev = (void*)(PP->outputTokenStreamHead->inputStream.base.node.prev);

            status = ppoTOKEN_Destroy(
                PP,
                PP->outputTokenStreamHead
                );
            if (status != gcvSTATUS_OK) break;

            PP->outputTokenStreamHead = prev;
        }
        return gcvSTATUS_OK;
    }
    while(gcvFALSE);

    cloCOMPILER_Report(
        PP->compiler,
        PP->currentSourceFileLineNumber,
        PP->currentSourceFileStringNumber,
        clvREPORT_ERROR,
        "Error in parsing.");

    return status;
}





/*******************************************************************************
**
**    ppoPREPROCESSOR_Reset
**
**        1. reset states.
**
*/

gceSTATUS
ppoPREPROCESSOR_Reset (
ppoPREPROCESSOR PP
)
{
    gceSTATUS status = gcvSTATUS_INVALID_DATA;

    do {
        if (PP->useNewPP)
        {
            if(PP->strings)
            {
                status = cloCOMPILER_Free(PP->compiler, (gctPOINTER)PP->strings);
                if (status != gcvSTATUS_OK) break;
            }

            PP->strings = gcvNULL;

            /*lens*/
            if(PP->lens)
            {
                status = cloCOMPILER_Free(PP->compiler, PP->lens);
                if (status != gcvSTATUS_OK) break;
            }

            PP->lens = gcvNULL;
        }
        else
        {
            /* Free strings */
            if (PP->ppedCount > 0) {
                gctUINT i;

                for (i=0; i < PP->ppedCount; i++) {
                    gcmVERIFY_OK(gcoOS_Free(gcvNULL, PP->ppedStrings[i]));
                }
           }
           PP->ppedCount = 0;

           if(PP->ppedStrings) {
               status = cloCOMPILER_Free(PP->compiler, (gctPOINTER)PP->ppedStrings);
               if (status != gcvSTATUS_OK) break;
           }
           PP->ppedStrings = gcvNULL;
       }

        /*macro manager*/
        if(PP->macroManager)
        {
            status = ppoMACRO_MANAGER_Destroy(
                PP,
                PP->macroManager
                );
            if (status != gcvSTATUS_OK) break;
        }

        PP->macroManager = gcvNULL;

        /*inputstream*/
        while(PP->inputStream)
        {
            ppoINPUT_STREAM tmp;
            ppoBYTE_INPUT_STREAM tmpBis;

            tmp = PP->inputStream;
            tmpBis = (ppoBYTE_INPUT_STREAM)tmp;
            /* free the string from the header file, whose inputStringNumber has been set to -1 */
            if (tmpBis->inputStringNumber == -1)
            {
                status = cloCOMPILER_Free(PP->compiler,
                                          (gctPOINTER)tmpBis->src);
            }
            if (status != gcvSTATUS_OK) break;
            PP->inputStream = (ppoINPUT_STREAM)PP->inputStream->base.node.prev;

            status = cloCOMPILER_Free(PP->compiler,
                                      tmp);
            if (status != gcvSTATUS_OK) break;
        }

        /*release output stream*/

        if(PP->outputTokenStreamHead)
        {
            status = ppoTOKEN_STREAM_Destroy(PP, PP->outputTokenStreamHead);

            if (status != gcvSTATUS_OK) break;
        }

        PP->outputTokenStreamHead = gcvNULL;

        PP->outputTokenStreamEnd = gcvNULL;

        /*reset debug mode*/

        status = cloCOMPILER_SetDebug( PP->compiler, gcvFALSE );

        if (status != gcvSTATUS_OK) break;

        /*reset optimize mode*/

        status = cloCOMPILER_SetOptimize( PP->compiler, gcvTRUE );

        if (status != gcvSTATUS_OK) break;

        /*reset compiler version*/

        status = cloCOMPILER_SetVersion( PP->compiler, 100 );

        PP->version = 100;

        if (status != gcvSTATUS_OK) break;

        /*reset states for version parsing.*/

        PP->otherStatementHasAlreadyAppeared = gcvFALSE;

        PP->versionStatementHasAlreadyAppeared = gcvFALSE;

        /*reset the file string number and the file line number*/

        PP->currentSourceFileStringNumber = 0;

        PP->currentSourceFileLineNumber = 1;

        /*reset the state maintained to restore chars read from input stream*/

        PP->lastGetcharPhase0IsFromThisBis = gcvNULL;

        /*reset the state to handle the line-continue following a comment.*/

        PP->iAmFollowingAComment = gcvFALSE;


        /*reset the state maintianed to parse the #if etc.*/

        PP->doWeInValidArea = gcvTRUE;

        PP->toLineEnd = gcvFALSE;
        PP->skipLine = -1;
        PP->skipOPError = gcvFALSE;

        /*apiState*/


        return gcvSTATUS_OK;

    }
    while(gcvFALSE);

    cloCOMPILER_Report(PP->compiler,
               1,
               0,
               clvREPORT_INTERNAL_ERROR,
               "Failed in resetting.");

    return status;
}

typedef gctCONST_STRING (*GETVALUE_FPTR)(void);

typedef struct _clsPREDEFINED_MACRO
{
    gctSTRING        str;
    gctSTRING        rplStr;
    GETVALUE_FPTR    fptr;     /* function pointer to get the macro value string dynamically */
    gctBOOL          checkFeature; /* If it is FALSE, then it is always enabled. */
}
clsPREDEFINED_MACRO;

/* Some are copied from InitializePredefinedMacros in llvm pp.
    TODO: any other predefined macros? */
static clsPREDEFINED_MACRO _PredefinedMacros[] =
{
    {"__OPENCL_VERSION__", "110", gcvNULL, gcvFALSE},
    {"CL_VERSION_1_0", "100", gcvNULL, gcvFALSE},
    {"CL_VERSION_1_1", "110", gcvNULL, gcvFALSE},
    {"CL_VERSION_1_2", "120", gcvNULL, gcvFALSE},
    {"__OPENCL_C_VERSION__", "120", gcvNULL, gcvFALSE},
    {"__IMAGE_SUPPORT__", "1", gcvNULL, gcvFALSE},
    {"__ENDIAN_LITTLE__", "1", gcvNULL, gcvFALSE},
    {"__kernel_exec(X, typen)", "__kernel __attribute__((work_group_size_hint(X, 1, 1))) \\\n__attribute__((vec_type_hint(typen)))", gcvNULL, gcvFALSE},
};

#define _cldMaxPredefinedMacroNameLen 64   /*Hard coded maximum predefined macro name length */
#define _cldPredefinedMacroCount  (sizeof(_PredefinedMacros) / sizeof(clsPREDEFINED_MACRO))
#define _cldMessageBufferLen  (_cldMaxPredefinedMacroNameLen + 64)

static gceSTATUS
ppoPREPROCESSOR_addMacroDef_Str(
ppoPREPROCESSOR  PP,
gctSTRING  MacroStr,
gctSTRING  ValueStr
)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER pointer = gcvNULL;
    gctSIZE_T length = 0;
    gctSIZE_T bufSize = 0; /* initial size */
    gctSIZE_T reqStrSize = 0;
    gctBOOL needToRealloc = gcvFALSE;

    length = 8 + gcoOS_StrLen(MacroStr, gcvNULL) + 1 + gcoOS_StrLen(ValueStr, gcvNULL) + 1 + 1;
    if (PP->macroString)
    {
        reqStrSize = gcoOS_StrLen(PP->macroString, gcvNULL) + length;
        bufSize = PP->macroStringSize;
    }
    else
    {
        reqStrSize = length;
        bufSize = 1024; /* initial size */
    }
    if (reqStrSize > bufSize)
    {
        /* need to enlarge the size of macroStr to contain longer string. */
        for (; bufSize < reqStrSize; bufSize = bufSize * 2)
        {
            /* do nothing */
        }
        needToRealloc = gcvTRUE;
    }
    if (PP->macroString == gcvNULL)
    {
        gcmONERROR(cloCOMPILER_Allocate(PP->compiler, bufSize, &pointer));
        PP->macroString = pointer;
        PP->macroString[0] = '\0';
        PP->macroStringSize = bufSize;
    }
    else if (needToRealloc)
    {
        gctSTRING tempStr = PP->macroString;
        gcmONERROR(cloCOMPILER_Allocate(PP->compiler, bufSize, &pointer));
        PP->macroString = pointer;
        PP->macroStringSize = bufSize;
        gcoOS_MemCopy(PP->macroString, tempStr, gcoOS_StrLen(tempStr, gcvNULL) + 1);
        cloCOMPILER_Free(PP->compiler, tempStr);
    }

    /* add "#define MacroStr ValueStr" to the macroString of PP */
    gcoOS_StrCatSafe(PP->macroString, bufSize, "#define ");
    gcoOS_StrCatSafe(PP->macroString, bufSize, MacroStr);
    gcoOS_StrCatSafe(PP->macroString, bufSize, " ");
    gcoOS_StrCatSafe(PP->macroString, bufSize, ValueStr);
    gcoOS_StrCatSafe(PP->macroString, bufSize, "\n");
OnError:
    return status;
}

gceSTATUS
ppoPREPROCESSOR_addMacroDef_Int(
ppoPREPROCESSOR  PP,
gctSTRING  MacroStr,
gctSTRING  ValueStr
)
{
    ppoMACRO_SYMBOL ms = gcvNULL;
    gctSTRING msName   = gcvNULL;
    gctCHAR messageBuffer[_cldMessageBufferLen];
    gctUINT offset = 0;
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    ppoTOKEN rplst = gcvNULL;
    gctSTRING rplStr = gcvNULL;

    gcmONERROR(cloCOMPILER_AllocatePoolString(PP->compiler,
                                                MacroStr,
                                                &msName));

    if(ValueStr)
    {
        gcmONERROR(cloCOMPILER_AllocatePoolString(PP->compiler,
                                                    ValueStr,
                                                    &rplStr));

        gcmONERROR(ppoTOKEN_Construct(PP, __FILE__, __LINE__, "Creat for CLC.", &rplst));
        rplst->hideSet       = gcvNULL;
        rplst->poolString    = rplStr;
        rplst->type          = ppvTokenType_INT;
    }
    else
    {
        rplst = gcvNULL;
    }

    offset = 0;
    gcoOS_PrintStrSafe(messageBuffer,
                        gcmSIZEOF(messageBuffer),
                        &offset,
                        "ppoPREPROCESSOR_Construct :add %s into macro symbol.",
                        msName);

    gcmONERROR(ppoMACRO_SYMBOL_Construct(PP,
                                        __FILE__,
                                        __LINE__,
                                            messageBuffer,
                                            msName,
                                            0,/*argc*/
                                            gcvNULL,/*argv*/
                                            rplst,/*replacement-list*/
                                            &ms));

    if (ms == gcvNULL)
    {
        gcmASSERT(ms);
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    gcmONERROR(ppoMACRO_MANAGER_AddMacroSymbol(PP,
                                                (PP)->macroManager,
                                                ms));
    return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(cloCOMPILER_Report(
        PP->compiler,
        1,
        0,
        clvREPORT_FATAL_ERROR,
        "Failed in preprocessing."));

    return status;
}

#define _cldFILENAME_MAX       1024

static gceSTATUS
ppoPREPROCESSOR_SetSourceStrings_New(
ppoPREPROCESSOR  PP,
gctCONST_STRING* Strings,
gctUINT          Count,
gctCONST_STRING  Options
)
{
    gceSTATUS status = gcvSTATUS_COMPILER_FE_PREPROCESSOR_ERROR;
    gctINT i;
    ppoINPUT_STREAM tmpbis = gcvNULL;
    ppoBYTE_INPUT_STREAM tmpbisCreated = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gctSTRING fileNamePath = gcvNULL;

    /*check argument*/
    gcmASSERT(PP);
    gcmASSERT(ppvOBJ_PREPROCESSOR == PP->base.type);
    gcmASSERT(Strings);

    gcmONERROR(ppoPREPROCESSOR_Reset(PP));

    /*macro manager*/
    gcmONERROR(ppoMACRO_MANAGER_Construct(PP,
                               __FILE__,
                               __LINE__,
                               "ppoPREPROCESSOR_Construct : Create.",
                               &((PP)->macroManager)));

    /* pre-defined macros */
    do
    {
        for(i = 0; i < (signed)_cldPredefinedMacroCount; i++)
        {
           gctSTRING valueStr;

           if (_PredefinedMacros[i].checkFeature &&
               gcoOS_StrStr(PP->extensionString, _PredefinedMacros[i].str, gcvNULL) == gcvSTATUS_FALSE)
           {
               continue;
           }
           valueStr = (gctSTRING) (_PredefinedMacros[i].fptr ? _PredefinedMacros[i].fptr()
                                                : _PredefinedMacros[i].rplStr);
           status = ppoPREPROCESSOR_addMacroDef_Str(PP, _PredefinedMacros[i].str, valueStr);
        }
    }
    while(gcvFALSE);

    /* Save current path "." to header file list */
    status = cloCOMPILER_ZeroMemoryAllocate(
                                    PP->compiler,
                                    2,
                                    (gctPOINTER *)&pointer
                                    );
    if (gcmIS_ERROR(status)) return status;
    fileNamePath = (gctCHAR *)pointer;

    gcmVERIFY_OK(gcoOS_StrCopySafe(fileNamePath, 2, "."));
    ppoPREPROCESSOR_AddHeaderFilePathToList(PP, fileNamePath);
    if (gcmIS_ERROR(status))
    {
        return status;
    }

    /* parse build options */
    if (Options != gcvNULL) {
        gctSTRING pos = gcvNULL;
        gcoOS_StrStr(Options, "-", &pos);

        while (pos)
        {
            pos++;
            if (!pos)
                break;

            if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "I", 1))
            {
                gctUINT pathLen = 0;
                gctSTRING path;
                gctSTRING tmpPos;

                pos += 1;
                /* skip whitespace if any */
                for(; pos && *pos != '\0'; pos++)
                {
                    if (*pos != ' ')
                        break;
                }
                /* get the lengh of path string */
                for(tmpPos = pos; tmpPos && *tmpPos != '\0'; tmpPos++)
                {
                    if (*tmpPos == ' ')
                        break;
                    pathLen++;
                }

                if (pathLen > _cldFILENAME_MAX)
                {
                    return gcvSTATUS_INVALID_DATA;
                }

                status = cloCOMPILER_ZeroMemoryAllocate(
                                                        PP->compiler,
                                                        pathLen + 1,
                                                        &pointer
                                                        );
                if (gcmIS_ERROR(status))
                {
                    return status;
                }
                path = pointer;

                gcoOS_StrCopySafe(path, pathLen + 1, pos);
                ppoPREPROCESSOR_AddHeaderFilePathToList(PP, path);
                if (gcmIS_ERROR(status))
                {
                    return status;
                }
                pos = tmpPos;
            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "D", 1))
            {
                /* add macro definition */
                gctSTRING valueStr = gcvNULL;
                gctSTRING macroStr = gcvNULL;
                gctBOOL hasVauleStr = gcvFALSE;
                gctSIZE_T len;
                gctINT j = 0;

                pos++;
                len = gcoOS_StrLen(pos, gcvNULL);
                /* remove whitespaces if any */
                for(; pos; pos++)
                {
                    if (*pos == ' ')
                        continue;
                    else
                    {
                        status = cloCOMPILER_ZeroMemoryAllocate(
                                                                PP->compiler,
                                                                len + 1,
                                                                &pointer
                                                                );
                        if (gcmIS_ERROR(status))
                        {
                            return status;
                        }
                        macroStr = pointer;
                        gcoOS_StrCopySafe(macroStr, len + 1, pos);
                        break;
                    }
                }

                /* get macro name string and value string */
                for(i = 0; *(pos + i) != '\0'; i++)
                {
                    gctCHAR currentChar = *(pos + i);

                    /* find next whitespace */
                    if (currentChar == ' ')
                    {
                        if (hasVauleStr)
                        {
                            *(valueStr + i - j - 1) = '\0';
                        }
                        else
                        {
                            *(macroStr + i) = '\0';
                        }
                        break;
                    }

                    if (currentChar == '=')
                    {
                        hasVauleStr = gcvTRUE;
                        j = i; /* save the index of "=" */
                        *(macroStr + i) = '\0';
                        len = gcoOS_StrLen(pos+i+1, gcvNULL);
                        status = cloCOMPILER_ZeroMemoryAllocate(
                                                                PP->compiler,
                                                                len + 1,
                                                                &pointer
                                                                );
                        if (gcmIS_ERROR(status))
                        {
                            return status;
                        }
                        valueStr = pointer;
                        gcoOS_StrCopySafe(valueStr, len+1, pos+i+1);
                    }
                }

                if (!hasVauleStr)
                {
                    status = ppoPREPROCESSOR_addMacroDef_Int(PP, macroStr, "1");
                }
                else
                {
                    status = ppoPREPROCESSOR_addMacroDef_Str(PP, macroStr, valueStr);
                }
                if(gcmIS_ERROR(status)) return status;

                /* set pos to next word */
                len = gcoOS_StrLen(macroStr, gcvNULL);
                pos += len;
                if (hasVauleStr)
                {
                    len = gcoOS_StrLen(valueStr, gcvNULL) + 1; /* consider the length of "=" too */
                    pos += len;
                }

                if (macroStr)
                {
                    gcmVERIFY_OK(cloCOMPILER_Free(PP->compiler, macroStr));
                    macroStr = gcvNULL;
                }
                if (hasVauleStr)
                {
                    gcmVERIFY_OK(cloCOMPILER_Free(PP->compiler, valueStr));
                    valueStr = gcvNULL;
                }

            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "cl-viv-vx-extension", sizeof("cl-viv-vx-extension")-1))
            {
                char* env = gcvNULL;
                status = cloCOMPILER_EnableExtension(PP->compiler,
                                                     clvEXTENSION_VIV_VX,
                                                     gcvTRUE);
                if(gcmIS_ERROR(status)) return status;

                status = ppoPREPROCESSOR_addMacroDef_Int(PP, "_VIV_VX_EXTENSION", "1");
                if(gcmIS_ERROR(status)) return status;

                /* Add <SDK_DIR> to header file search path */
                gcoOS_GetEnv(gcvNULL, "VIVANTE_SDK_DIR", &env);
                if(env)
                {
                    gctUINT len;
                    gctSTRING path, path1;

                    len = gcoOS_StrLen(env, gcvNULL);

                    status = cloCOMPILER_ZeroMemoryAllocate(
                                                            PP->compiler,
                                                            _cldFILENAME_MAX,
                                                            &pointer
                                                            );
                    if (gcmIS_ERROR(status))
                    {
                        return status;
                    }
                    path = pointer;
                    gcoOS_StrCopySafe(path, len + 1, env);
                    gcoOS_StrCatSafe(path, _cldFILENAME_MAX, "/include/CL/");

                    status = ppoPREPROCESSOR_AddHeaderFilePathToList(PP, path);
                    if (gcmIS_ERROR(status))
                    {
                        return status;
                    }

                    len = gcoOS_StrLen(env, gcvNULL);

                    status = cloCOMPILER_ZeroMemoryAllocate(
                                                            PP->compiler,
                                                            _cldFILENAME_MAX,
                                                            &pointer
                                                            );
                    if (gcmIS_ERROR(status))
                    {
                        return status;
                    }
                    path1 = pointer;
                    gcoOS_StrCopySafe(path1, len + 1, env);
                    gcoOS_StrCatSafe(path1, _cldFILENAME_MAX, "/inc/CL/");

                    status = ppoPREPROCESSOR_AddHeaderFilePathToList(PP, path1);
                    if (gcmIS_ERROR(status))
                    {
                        return status;
                    }
                }
                pos += (sizeof("cl-viv-vx-extension")-1);
            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "cl-fast-relaxed-math", sizeof("cl-fast-relaxed-math")-1))
            {
                status = cloCOMPILER_SetFpConfig(PP->compiler,
                                                 cldFpFAST_RELAXED_MATH);
                if(gcmIS_ERROR(status)) return status;
                status = ppoPREPROCESSOR_addMacroDef_Int(PP, "__FAST_RELAXED_MATH__", "1");
                if(gcmIS_ERROR(status)) return status;
                pos += (sizeof("cl-fast-relaxed-math")-1);
            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "cl-viv-gcsl-driver-image", sizeof("cl-viv-gcsl-driver-image")-1))
            {
                status = cloCOMPILER_SetGcslDriverImage(PP->compiler);
                if(gcmIS_ERROR(status)) return status;
                pos += (sizeof("cl-viv-gcsl-driver-image")-1);
            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "cl-viv-packed-basic-type", sizeof("cl-viv-packed-basic-type")-1))
            {
                status = cloCOMPILER_SetBasicTypePacked(PP->compiler);
                if(gcmIS_ERROR(status)) return status;
                pos += (sizeof("cl-viv-packed-basic-type")-1);
            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "cl-viv-longulong-patch", sizeof("cl-viv-longulong-patch")-1))
            {
                status = cloCOMPILER_SetLongUlongPatch(PP->compiler);
                if(gcmIS_ERROR(status)) return status;
                pos += (sizeof("cl-viv-longulong-patch")-1);
            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "cl-finite-math-only", sizeof("cl-finite-math-only")-1))
            {
                status = cloCOMPILER_SetFpConfig(PP->compiler,
                                                 cldFpFINITE_MATH_ONLY);
                if(gcmIS_ERROR(status)) return status;
                pos += (sizeof("cl-finite-math-only")-1);
            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "cl-viv-vx-image-array-maxlevel=", sizeof("cl-viv-vx-image-array-maxlevel=")-1))
            {
                gctUINT len1 = 0, len2 = 0;
                gctSTRING maxLevel;
                gctSTRING temPos;
                pos += 31;
                for(; pos; pos++)
                {
                    if (*pos == ' ')
                        continue;
                    else
                        len1 = gcoOS_StrLen(pos, gcvNULL);
                    break;
                }
                temPos = pos;
                for(; pos; pos++)
                {
                    if (*pos == '\0' || *pos == '-')
                        break;
                    else if (*pos != ' ')
                        continue;
                    else
                        break;
                }
                len2 = gcoOS_StrLen(pos, gcvNULL);

                status = cloCOMPILER_ZeroMemoryAllocate(
                                                        PP->compiler,
                                                        (len1-len2) + 1,
                                                        &pointer
                                                        );
                if (gcmIS_ERROR(status))
                {
                    return status;
                }
                maxLevel = pointer;
                gcoOS_StrCopySafe(maxLevel, (len1-len2) + 1, temPos);
                status = cloCOMPILER_SetImageArrayMaxLevel(PP->compiler, maxLevel);
                if(gcmIS_ERROR(status))
                {
                    cloCOMPILER_Report(PP->compiler,
                                       0,
                                       0,
                                       clvREPORT_ERROR,
                                       "unrecognized image array max level \"%s\" specified in option cl-viv-vx-image-array-maxlevel",
                                       maxLevel);
                    if (maxLevel)
                    {
                        gcmVERIFY_OK(cloCOMPILER_Free(PP->compiler, maxLevel));
                        maxLevel = gcvNULL;
                    }
                    return gcvSTATUS_INTERFACE_ERROR;
                }
                if (maxLevel)
                {
                    gcmVERIFY_OK(cloCOMPILER_Free(PP->compiler, maxLevel));
                    maxLevel = gcvNULL;
                }
            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "cl-finite-math-only", sizeof("cl-finite-math-only")-1))
            {
                status = cloCOMPILER_SetFpConfig(PP->compiler,
                                                 cldFpFINITE_MATH_ONLY);
                if(gcmIS_ERROR(status)) return status;
                pos += (sizeof("cl-finite-math-only")-1);
            }
            else if (gcvSTATUS_OK == gcoOS_StrNCmp(pos, "cl-std=", sizeof("cl-std=")-1))
            {
                gctSIZE_T len1 = 0, len2 = 0;
                gctSTRING languageVersion;
                gctSTRING temPos;

                /* Interception versionLanguage from option string */
                pos += 7;
                for(; pos; pos++)
                {
                    if (*pos == ' ')
                        continue;
                    else
                        len1 = gcoOS_StrLen(pos, gcvNULL);
                    break;
                }
                temPos = pos;
                for(; pos; pos++)
                {
                    if (*pos == '\0' || *pos == '-')
                        break;
                    else if (*pos != ' ')
                        continue;
                    else
                        break;
                }
                len2 = gcoOS_StrLen(pos, gcvNULL);

                status = cloCOMPILER_ZeroMemoryAllocate(
                                                        PP->compiler,
                                                        (len1 - len2) + 1,
                                                        &pointer
                                                        );
                if (gcmIS_ERROR(status))
                {
                    return status;
                }
                languageVersion = pointer;
                gcoOS_StrCopySafe(languageVersion, (len1 - len2) + 1, temPos);

                status = cloCOMPILER_SetLanguageVersion(PP->compiler, languageVersion);
                if(gcmIS_ERROR(status))
                {
                    cloCOMPILER_Report(PP->compiler,
                                       0,
                                       0,
                                       clvREPORT_ERROR,
                                       "unrecognized language version \"%s\" specified in option cl-std",
                                       languageVersion);
                    if (languageVersion)
                    {
                        gcmVERIFY_OK(cloCOMPILER_Free(PP->compiler, languageVersion));
                        languageVersion = gcvNULL;
                    }
                    return gcvSTATUS_INTERFACE_ERROR;
                }

                if (languageVersion)
                {
                    gcmVERIFY_OK(cloCOMPILER_Free(PP->compiler, languageVersion));
                    languageVersion = gcvNULL;
                }
            }
            else
            {
                /* skip other options */
            }
            gcoOS_StrStr(pos, "-", &pos);
        }
    }

    /*count*/
    PP->count = Count;

    /*lens*/
    gcmONERROR(cloCOMPILER_Allocate(
        PP->compiler,
        Count * sizeof(gctUINT),
        &pointer
        ));

    PP->lens = pointer;

    for (i = 0; i < (signed)Count; i++)
    {
        PP->lens[i] = (gctUINT) gcoOS_StrLen(Strings[i], gcvNULL);
    }

    /*strings*/
    gcmONERROR(cloCOMPILER_Allocate(
        PP->compiler,
        Count * sizeof(gctCONST_STRING),
        &pointer));

    PP->strings = pointer;

    /*strings[i]*/
    for( i = 0; i < (signed)Count; i++ )
    {
        PP->strings[i] = Strings[i];
    }

    for( i = Count-1; i >= 0; --i )
    {
        if( PP->lens != 0 )
        {
            gcmONERROR(ppoBYTE_INPUT_STREAM_Construct(
                PP,
                gcvNULL,/*prev node*/
                gcvNULL,/*next node*/
                __FILE__,
                __LINE__,
                "ppoPREPROCESSOR_SetSourceStrings : Creat to init CPP input stream",
                PP->strings[i],
                i,/*file string number, used to report debug infomation.*/
                PP->lens[i],
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
            }/*if( gcvNULL == PP->inputStream )*/
        }
        else/*if( PP->lens > 0 )*/
        {
            gcmONERROR(ppoPREPROCESSOR_Report(
                PP,
                clvREPORT_WARN,
                "file string : %u's length is zero",
                i));
        }/*if( PP->lens > 0 )*/
    }/*for*/

    if (PP->macroString)
    {
        gcmONERROR(ppoBYTE_INPUT_STREAM_Construct(
                   PP,
                   gcvNULL,/*prev node*/
                   gcvNULL,/*next node*/
                   __FILE__,
                   __LINE__,
                   "ppoPREPROCESSOR_SetSourceStrings : Creat to init CPP input stream",
                   PP->macroString,
                   -2, /* input string number is -2 for macroString */
                   (gctUINT) gcoOS_StrLen(PP->macroString, gcvNULL),
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
        }/*if( gcvNULL == PP->inputStream )*/
    }

    return gcvSTATUS_OK;

OnError:
    gcmVERIFY_OK(cloCOMPILER_Report(
        PP->compiler,
        1,
        0,
        clvREPORT_FATAL_ERROR,
        "Failed in preprocessing."));

    return status;
}

/*******************************************************************************
**
**    ppoPREPROCESSOR_SetSourceStrings
**
**        Buffer source strings and lens, then set Dirty in *PP*
**
*/

gceSTATUS
ppoPREPROCESSOR_SetSourceStrings(
ppoPREPROCESSOR    PP,
gctCONST_STRING*   Strings,
gctUINT            Count,
gctCONST_STRING    Options
)
{
    gceSTATUS status = gcvSTATUS_INVALID_ARGUMENT;

    /*check argument*/
    gcmASSERT(PP);
    gcmASSERT(ppvOBJ_PREPROCESSOR == PP->base.type);
    gcmASSERT(Strings);

    do {
        if (PP->useNewPP)
        {
            ppoPREPROCESSOR_SetSourceStrings_New(PP, Strings, Count, Options);
            return gcvSTATUS_OK;
        }
        gcmONERROR(ppoPREPROCESSOR_Reset(PP));

        /*count*/
        PP->count = Count;
        PP->lens = gcvNULL;

        /*strings*/
        PP->strings = Strings;

        PP->ppedCount = 0;
        gcmONERROR(cloCOMPILER_Allocate(PP->compiler,
                            sizeof(gctSTRING*)*Count,
                            (void*)&PP->ppedStrings));

        gcoOS_MemFill(PP->ppedStrings,
                         (gctUINT8)0,
                         sizeof(gctSTRING*)*Count);
        return gcvSTATUS_OK;
    }
    while(gcvFALSE);
OnError:
    cloCOMPILER_Report(PP->compiler,
               1,
               0,
               clvREPORT_FATAL_ERROR,
               "Failed in preprocessing.");
    return status;
}

