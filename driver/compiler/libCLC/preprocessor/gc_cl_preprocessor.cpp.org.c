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

gceSTATUS ppoPREPROCESSOR_SetDebug(
    IN cloCOMPILER      Compiler,
    IN gctUINT          On
    )
{
    return gcvSTATUS_OK;
}
gceSTATUS ppoPREPROCESSOR_SetOptimize(
    IN cloCOMPILER      Compiler,
    IN gctUINT          On
    )
{
    return gcvSTATUS_OK;
}
gceSTATUS ppoPREPROCESSOR_SetVersion(
    IN cloCOMPILER      Compiler,
    IN gctUINT          Version
    )
{
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  ppoPREPROCESSOR_PushOntoCurrentInputStreamOfPP
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





/*******************************************************************************
**
**  ppoPREPROCESSOR_AddToOutputStreamOfPP
**
*/
gceSTATUS
ppoPREPROCESSOR_AddToOutputStreamOfPP(ppoPREPROCESSOR PP,
                                      ppoTOKEN Token)
{
    ppoTOKEN ntoken = gcvNULL;

    gceSTATUS status = gcvSTATUS_INVALID_ARGUMENT;

    gcmASSERT(PP && Token);

    status = ppoTOKEN_Colon(
        PP,
        Token,
        __FILE__,
        __LINE__,
        "Dump for adding this token to the output of cpp.",
        &ntoken);                                                               ppmCheckOK();

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

    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  ppoPREPROCESSOR_DumpOutputStream
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
**  ppoPREPROCESSOR_Report
**
*/
gceSTATUS
ppoPREPROCESSOR_Report(ppoPREPROCESSOR PP,
                       cleREPORT_TYPE Type,
                       gctCONST_STRING Message,
                       ...)
{
    gceSTATUS status;

    status = cloCOMPILER_VReport(
        PP->compiler,
        PP->currentSourceFileLineNumber,
        PP->currentSourceFileStringNumber,
        Type,
        Message,
        (gctPOINTER) (&Message + 1));

    return status;
}
/*
x * x ...  x * x
----------------
total = y
*/
gctINT ppoPREPROCESSOR_Pow(gctINT x, gctINT y)
{
    gctINT rt   = 1;

    gctINT i    = 0;

    gcmASSERT( x >= 0 && y >= 0 );

    for ( i=1; i<=y; i++ )
    {
        rt = rt * x;
    }

    return rt;
}




gceSTATUS
ppoPREPROCESSOR_Construct_InitKeyword(cloCOMPILER       Compiler,
                                      ppoPREPROCESSOR   *PP);

gceSTATUS
ppoPREPROCESSOR_Construct_InitOperator(cloCOMPILER      Compiler,
                                       ppoPREPROCESSOR  *PP);


/*******************************************************************************
**
**  ppoPREPROCESSOR_Construct
**
**      Allocate memory, set undirty.
**
*/
gceSTATUS
ppoPREPROCESSOR_Construct(cloCOMPILER       Compiler,
                          ppoPREPROCESSOR   *PP)

{

    gceSTATUS status = gcvSTATUS_INVALID_DATA;

    do
    {
        /*preprocessor*/
        status = cloCOMPILER_Allocate(
            Compiler,
            sizeof(struct _ppoPREPROCESSOR),
            (gctPOINTER *)PP
            );

        if (status != gcvSTATUS_OK) break;

        status = gcoOS_MemFill(
            *PP,
            0,/*set value*/
            sizeof(struct _ppoPREPROCESSOR)
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->base.file = __FILE__;
        (*PP)->base.info = "Created in ppoPREPROCESSOR_Construct";
        (*PP)->base.line = __LINE__;
        (*PP)->base.node.next = gcvNULL;
        (*PP)->base.node.prev = gcvNULL;
        (*PP)->base.type = ppvOBJ_PREPROCESSOR;


        (*PP)->compiler = Compiler;

        /*keywords*/

        status = cloCOMPILER_Allocate(
            Compiler,
            sizeof (struct _ppsKEYWORD),
            (gctPOINTER*)&((*PP)->keyword)
            );

        if (status != gcvSTATUS_OK) break;

        status = ppoPREPROCESSOR_Construct_InitKeyword(Compiler, PP);

        /*operators*/

        status  = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING*)*12,
            (void*)&((*PP)->operators)
            );

        if (status != gcvSTATUS_OK) break;

        status = gcoOS_MemFill(
            (*PP)->operators,
            (gctUINT8)0,
            sizeof(gctSTRING*)*12
            );

        if (status != gcvSTATUS_OK) break;

        status = ppoPREPROCESSOR_Construct_InitOperator(Compiler, PP);

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
ppoPREPROCESSOR_Construct_InitKeyword(cloCOMPILER       Compiler,
                                      ppoPREPROCESSOR   *PP)
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

        /*44    :   */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            ":",
            &((*PP)->keyword->colon));

        if (status != gcvSTATUS_OK) break;

        /*45    require */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "require",
            &((*PP)->keyword->require));

        if (status != gcvSTATUS_OK) break;

        /*46    enable  */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "enable",
            &((*PP)->keyword->enable));

        if (status != gcvSTATUS_OK) break;

        /*47    warn    */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "warn",
            &((*PP)->keyword->warn));

        if (status != gcvSTATUS_OK) break;

        /*48    disable */
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

        /*55    all     */
        status = cloCOMPILER_AllocatePoolString((*PP)->compiler,
            "all",
            &((*PP)->keyword->all));

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
ppoPREPROCESSOR_Construct_InitOperator(cloCOMPILER      Compiler,
                                       ppoPREPROCESSOR  *PP)
{

    gceSTATUS status = gcvSTATUS_INVALID_DATA;

    do
    {
        /*(*PP)->operators 00:  Total: 3, Detail: 2, ||, 0*/

        status  = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[0])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[0][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[0][1] = (*PP)->keyword->lor;

        (*PP)->operators[0][2] = gcvNULL;

        /*(*PP)->operators 01:  Total: 3, Detail: 2, &&, 0*/

        status  = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[1])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[1][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[1][1] = (*PP)->keyword->land;

        (*PP)->operators[1][2] = gcvNULL;

        /*(*PP)->operators 02:  Total: 3, Detail: 2, |, 0*/

        status  = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[2])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[2][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[2][1] = (*PP)->keyword->bor;

        (*PP)->operators[2][2] = gcvNULL;

        /*(*PP)->operators 03:  Total: 3, Detail: 2, ^, 0*/

        status  = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[3])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[3][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[3][1] = (*PP)->keyword->bex;

        (*PP)->operators[3][2] = gcvNULL;

        /*(*PP)->operators 04:  Total: 3, Detail: 2, &, 0*/

        status  = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*3,
            (gctPOINTER*)&((*PP)->operators[4])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[4][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[4][1] = (*PP)->keyword->band;

        (*PP)->operators[4][2] = gcvNULL;

        /*(*PP)->operators 05:  Total: 4, Detail: 2, ==, !=, 0*/

        status  = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*4,
            (gctPOINTER*)&((*PP)->operators[5])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[5][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[5][1] = (*PP)->keyword->equal;

        (*PP)->operators[5][2] = (*PP)->keyword->not_equal;

        (*PP)->operators[5][3] = gcvNULL;

        /*(*PP)->operators 06:  Total: 6, Detail: 2, >, <, >=, <=, 0*/

        status  = cloCOMPILER_Allocate(
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

        /*(*PP)->operators 07:  Total: 4, Detail: 2, <<, >>, 0*/

        status  = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*4,
            (gctPOINTER*)&((*PP)->operators[7])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[7][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[7][1] = (*PP)->keyword->lshift;

        (*PP)->operators[7][2] = (*PP)->keyword->rshift;

        (*PP)->operators[7][3] = gcvNULL;

        /*(*PP)->operators 08:  Total: 4, Detail: 2, +, -, 0*/

        status  = cloCOMPILER_Allocate(
            (*PP)->compiler,
            sizeof(gctSTRING)*4,
            (gctPOINTER*)&((*PP)->operators[8])
            );

        if (status != gcvSTATUS_OK) break;

        (*PP)->operators[8][0] = (void*)ppvBINARY_OP;

        (*PP)->operators[8][1] = (*PP)->keyword->plus;

        (*PP)->operators[8][2] = (*PP)->keyword->minus;

        (*PP)->operators[8][3] = gcvNULL;

        /*(*PP)->operators 09:  Total: 5, Detail: 2, *, /, %, 0*/

        status  = cloCOMPILER_Allocate(
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

        /*(*PP)->operators 10:  Total: 8, Detail: 1, +, -, ~, !, defined, 0*/

        status  = cloCOMPILER_Allocate(
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

        /*(*PP)->operators 11:  Total: 0. This inputStream just a guarder.*/

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
**  ppoPREPROCESSOR_Destroy
**
**      Release memory used by internal elements of *PP*, and *PP*.
**
*/
gceSTATUS
ppoPREPROCESSOR_Destroy (ppoPREPROCESSOR PP)
{

    gceSTATUS status = gcvSTATUS_INVALID_DATA;

    cloCOMPILER compiler = gcvNULL;

    gctUINT i = 0;

    gcmASSERT(PP->base.type == ppvOBJ_PREPROCESSOR);

    compiler = PP->compiler;

    do
    {
        ppoPREPROCESSOR_Reset(PP);

        /*release operators of every level*/

        i = 0;

        while (PP->operators[i] != gcvNULL)
        {
            status = cloCOMPILER_Free(
                compiler,
                PP->operators[i]
                );

            if (status != gcvSTATUS_OK) break;

            i++;
        }

        status = cloCOMPILER_Free(
                compiler,
                PP->operators
                );

        if (status != gcvSTATUS_OK) break;

        gcmASSERT(i == 11);/*total number of group of operator, is 12.*/

        /*release output stream*/
        status = ppoTOKEN_STREAM_Destroy(
            PP,
            PP->outputTokenStreamHead
            );
        if (status != gcvSTATUS_OK) break;

        /*release keyword*/
        status = cloCOMPILER_Free(
            compiler,
            PP->keyword
            );
        if (status != gcvSTATUS_OK) break;

        /*PP*/
        status = cloCOMPILER_Free(
            compiler,
            PP);

        if (status != gcvSTATUS_OK) break;

        return gcvSTATUS_OK;
    }
    while(gcvFALSE);

    cloCOMPILER_Report(
        compiler,
        0,
        0,
        clvREPORT_INTERNAL_ERROR,
        "Error in destroy preprocessor."
        );
    return status;
}



/*******************************************************************************
**
**  ppoPREPROCESSOR_Dump
**
**      Dump.
**
*/
gceSTATUS   ppoPREPROCESSOR_Dump(ppoPREPROCESSOR PP)
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
**  ppoPREPROCESSOR_Parse
**
**      Parse the strings just set.
**
**
*/
gceSTATUS   ppoPREPROCESSOR_Parse(cloPREPROCESSOR       PP,
                                  char                  *Buffer,
                                  gctUINT               Max,
                                  gctINT                *WriteInNumber)
{
    gceSTATUS           status  = gcvSTATUS_INVALID_ARGUMENT;

    ppoTOKEN            ntoken  = gcvNULL;

    gctSIZE_T           len     = 0;

    /*legal argument?*/

    gcmASSERT(PP && PP->base.type == ppvOBJ_PREPROCESSOR);

    *WriteInNumber = -1;

    /*end of input?*/

    if(PP->inputStream == gcvNULL)
    {
        *WriteInNumber = -1;
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
                *WriteInNumber = -1;

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

        status = gcoOS_StrLen(PP->outputTokenStreamHead->poolString, &len);

        if (status != gcvSTATUS_OK) break;

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

        status = gcoOS_StrCopySafe(
            Buffer, Max,
            PP->outputTokenStreamHead->poolString
            );
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
**  ppoPREPROCESSOR_Reset
**
**      1. reset states.
**
*/

gceSTATUS   ppoPREPROCESSOR_Reset (ppoPREPROCESSOR PP)
{
    gceSTATUS status = gcvSTATUS_INVALID_DATA;

    do
    {
/** KLC  do not free as these are not allocated ***/
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

            tmp = PP->inputStream;

            PP->inputStream = (ppoINPUT_STREAM)PP->inputStream->base.node.prev;

            status = cloCOMPILER_Free(
                PP->compiler,
                tmp
                );
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

        /*apiState*/


        return gcvSTATUS_OK;

    }
    while(gcvFALSE);

    cloCOMPILER_Report(
        PP->compiler,
        1,
        0,
        clvREPORT_INTERNAL_ERROR,
        "Failed in resetting.");

    return status;
}


/*******************************************************************************
**
**  ppoPREPROCESSOR_SetSourceStrings
**
**      Buffer source strings and lens, then set Dirty in *PP*
**
*/

gceSTATUS
ppoPREPROCESSOR_SetSourceStrings(
                                 ppoPREPROCESSOR    PP,
                                 gctCONST_STRING*   Strings,
                                 gctUINT_PTR        Lens,
                                 gctUINT            Count
                                 )
{
    gceSTATUS status = gcvSTATUS_INVALID_ARGUMENT;

    gctINT i = 0;

    ppoINPUT_STREAM tmpbis = gcvNULL;

    /*check argument*/

    gcmASSERT(PP);

    gcmASSERT(ppvOBJ_PREPROCESSOR == PP->base.type);

    gcmASSERT(Strings);

    do
    {
        status = ppoPREPROCESSOR_Reset(PP);

        if (status != gcvSTATUS_OK) break;

        /*macro manager*/

        ppoMACRO_MANAGER_Construct(
            PP,
            __FILE__,
            __LINE__,
            "ppoPREPROCESSOR_Construct : Create.",
            &((PP)->macroManager)
            );

        if (status != gcvSTATUS_OK) break;

        /* pre-defined ms GL_ES */

        do
        {

            ppoMACRO_SYMBOL ms_gl_es = gcvNULL;

            gctSTRING name_gl_es = gcvNULL;

            status = cloCOMPILER_AllocatePoolString(
                PP->compiler,
                "GL_ES",
                &name_gl_es
                );

            if (status != gcvSTATUS_OK) break;

            ppoMACRO_SYMBOL_Construct(
                PP,
                __FILE__,
                __LINE__,
                "ppoPREPROCESSOR_Construct :add GL_ES into macro symbol.",
                name_gl_es,
                0,/*argc*/
                gcvNULL,/*argv*/
                gcvNULL,/*replacement-list*/
                &ms_gl_es
                );

            if (status != gcvSTATUS_OK) break;

            ppoMACRO_MANAGER_AddMacroSymbol(
                PP,
                (PP)->macroManager,
                ms_gl_es
                );

            if (status != gcvSTATUS_OK) break;
        }
        while(gcvFALSE);

        /* pre-defined ms GL_OES_standard_derivatives */

        do
        {
            ppoMACRO_SYMBOL ms = gcvNULL;

            gctSTRING msName = gcvNULL;

            status = cloCOMPILER_AllocatePoolString(
                PP->compiler,
                "GL_OES_standard_derivatives",
                &msName
                );

            if (status != gcvSTATUS_OK) break;

            ppoMACRO_SYMBOL_Construct(
                PP,
                __FILE__,
                __LINE__,
                "ppoPREPROCESSOR_Construct :add GL_OES_standard_derivatives into macro symbol.",
                msName,
                0,/*argc*/
                gcvNULL,/*argv*/
                gcvNULL,/*replacement-list*/
                &ms
                );

            if (status != gcvSTATUS_OK) break;

            ppoMACRO_MANAGER_AddMacroSymbol(
                PP,
                (PP)->macroManager,
                ms
                );

            if (status != gcvSTATUS_OK) break;

        }
        while(gcvFALSE);

        /*count*/

        PP->count = Count;

        /*lens*/

        status = cloCOMPILER_Allocate(
            PP->compiler,
            Count * sizeof(gctUINT),
            (void*)&(PP->lens)
            );

        if (status != gcvSTATUS_OK) break;

        if (Lens == gcvNULL)
        {

            if (status != gcvSTATUS_OK) break;

            for (i = 0; i < (signed)Count; i++)
            {
                status = gcoOS_StrLen(Strings[i], (gctSIZE_T *)&PP->lens[i]);

                if (status != gcvSTATUS_OK) break;
            }
        }
        else
        {

            status = gcoOS_MemCopy(
                PP->lens,
                Lens,
                Count * sizeof(gctUINT)
                );

            if (status != gcvSTATUS_OK) break;
        }

        /*strings*/

        status = cloCOMPILER_Allocate(
            PP->compiler,
            Count * sizeof(gctCONST_STRING),
            (gctPOINTER*)&(PP->strings));

        if (status != gcvSTATUS_OK) break;

        /*strings[i]*/

        for( i=0 ; i<(signed)Count ; i++ )
        {
            PP->strings[i] = Strings[i];
        }

        for( i=Count-1 ; i>=0 ; --i )
        {

            if( PP->lens != 0 )
            {
                status = ppoBYTE_INPUT_STREAM_Construct(
                    PP,
                    gcvNULL,/*prev node*/
                    gcvNULL,/*next node*/
                    __FILE__,
                    __LINE__,
                    "ppoPREPROCESSOR_SetSourceStrings : Creat to init CPP input stream",
                    PP->strings[i],
                    i,/*file string number, used to report debug infomation.*/
                    PP->lens[i],
                    (ppoBYTE_INPUT_STREAM *)(&tmpbis)/*out*/
                    );

                if (status != gcvSTATUS_OK) break;

                if( gcvNULL == PP->inputStream )
                {

                    PP->inputStream = tmpbis;

                    tmpbis->base.node.prev = gcvNULL;

                    tmpbis->base.node.next = gcvNULL;

                }
                else
                {

                    ppoINPUT_STREAM tmp_is

                        = PP->inputStream;

                    PP->inputStream = tmpbis;

                    tmpbis->base.node.prev = (void*)tmp_is;

                    tmpbis->base.node.next = (void*)gcvNULL;

                    tmp_is->base.node.next = (void*)tmpbis;

                }/*if( gcvNULL == PP->inputStream )*/
            }
            else/*if( PP->lens > 0 )*/
            {
                ppoPREPROCESSOR_Report(
                    PP,
                    clvREPORT_WARN,
                    "file string : %u's length is zero",
                    i);
            }/*if( PP->lens > 0 )*/
        }/*for*/

        return gcvSTATUS_OK;

    }
    while(gcvFALSE);

    cloCOMPILER_Report(
        PP->compiler,
        1,
        0,
        clvREPORT_FATAL_ERROR,
        "Failed in preprocessing.");

    return status;
}
