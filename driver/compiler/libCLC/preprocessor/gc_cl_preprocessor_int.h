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


#ifndef __gc_cl_preprocessor_int_h_
#define    __gc_cl_preprocessor_int_h_

#include "gc_cl_preprocessor.h"

/******************************************************************************\
|****************************** Macro Definition ******************************|
\******************************************************************************/
#define ppvICareWhiteSpace        1

#define ppvBINARY_OP            2

#define ppvUNARY_OP                1

#define ppmCheckOK() \
    do\
{\
    if(status != gcvSTATUS_OK){return status;}\
}\
    while(gcvFALSE)

#define ppmCheckFuncOk(func)\
    do\
{\
    status = func;\
    if(status != gcvSTATUS_OK){return status;}\
}\
    while(gcvFALSE)

#define ppmSTATE_0_CONSTRUCTED    0
#define ppmSTATE_1_RESETED        1
#define ppmSTATE_2_SRCSETTED    2
#define ppmSTATE_3_PARSED        3
#define ppmSTATE_4_DESTROIED    4

#define _cldBUFFER_MAX        1024

/******************************************************************************\
|********************************** Typedefs **********************************|
\******************************************************************************/

/*1*/
typedef struct
_ppoBASE
*ppoBASE;

/*2*/
typedef struct
_ppoBYTE_INPUT_STREAM
*ppoBYTE_INPUT_STREAM;

/*3*/
typedef    struct
_ppoHIDE_SET
*ppoHIDE_SET;

/*4*/
typedef struct
_ppoINPUT_STREAM
*ppoINPUT_STREAM;

/*5*/
typedef struct
_ppoMACRO_MANAGER
*ppoMACRO_MANAGER;

typedef    struct
_ppoMACRO_SYMBOL
*ppoMACRO_SYMBOL;

/*6*/
typedef struct
_ppsKEYWORD
*ppsKEYWORD;

typedef struct
_ppoPREPROCESSOR
*ppoPREPROCESSOR;

/*7*/
typedef struct
_ppoTOKEN
*ppoTOKEN,
*ppoTOKEN_STREAM;

/*8*/
typedef struct
_ppoHEADERFILEPATH
*ppoHEADERFILEPATH;


/******************************************************************************\
|****************************** 1 : Base Object *******************************|
\******************************************************************************/
typedef enum _ppeOBJECT_TYPE
{
    /*00*/    ppvOBJ_UNKNOWN                 =    0,
    /*01*/    ppvOBJ_MEMORY_MANAGER        =    gcmCC('M','M',' ','\0'),
    /*02*/    ppvOBJ_STRING_MANAGER        =    gcmCC('S','M',' ','\0'),
    /*03*/    ppvOBJ_MACRO_MANAGER        =    gcmCC('M','A','M','\0'),
    /*04*/    ppvOBJ_TOKEN                =    gcmCC('T','O','K','\0'),
    /*05*/    ppvOBJ_PREPROCESSOR            =    gcmCC('P','P',' ','\0'),
    /*06*/    ppvOBJ_HIDE_SET                =    gcmCC('H','S',' ','\0'),
    /*07*/    ppvOBJ_BYTE_INPUT_STREAM    =    gcmCC('B','I','S','\0'),
    /*08*/    ppvOBJ_STRING_MANAGER_IR    =    gcmCC('S','M','I','\0'),
    /*09*/    ppvOBJ_MACRO_SYMBOL            =    gcmCC('M','S',' ','\0'),
    /*10*/    ppvOBJ_STRING                =    gcmCC('S','T','R','\0'),
    /*11*/    ppvOBJ_MEMORY_MANAGER_IR    =    gcmCC('M','M','I','\0'),
    /*12*/    ppvOBJ_KEYWORD                =    gcmCC('K','W',' ','\0'),
    /*13*/    ppvOBJ_ARRAY                =    gcmCC('A','R','R','\0')
} ppeOBJECT_TYPE;

gceSTATUS
ppeOBJECT_TypeString(
                     IN    ppeOBJECT_TYPE        TypeEnum,
                     OUT    gctCONST_STRING*    TypeString
                     );


/******************************************************************************\
**
**    _ppoBASE
**
**        This is the basic class contain the debug info and type. It is
**        included in every "object" of preprocessor.
**
**    who depends on this struct?
**
**        1.ppoBASE_Init(),
**        2.ppoPREPROCESSOR_SetSourceStrings().
**
*/
struct    _ppoBASE
{
    /*00*/    slsDLINK_NODE        node;

    /*01*/    ppeOBJECT_TYPE        type;

    /*02*/    gctCONST_STRING        file;

    /*03*/    gctINT                line;

    /*04*/    gctCONST_STRING        info;
};

gceSTATUS
ppoBASE_Dump(
             ppoPREPROCESSOR PP,
             ppoBASE Base
             );


gceSTATUS
ppoBASE_Init(
             /*00*/ IN        ppoPREPROCESSOR        PP,
             /*01*/ IN OUT    ppoBASE                YourBase,
             /*02*/ IN        gctCONST_STRING        File,
             /*03*/ IN        gctUINT                Line,
             /*04*/ IN        gctCONST_STRING        MoreInfo,
             /*05*/ IN        ppeOBJECT_TYPE        Type
             );


/******************************************************************************\
|**************************** 3 : Hide Set Object *****************************|
\******************************************************************************/

struct _ppoHIDE_SET
{
    /*00*/    struct _ppoBASE    base;

    /*01*/    cltPOOL_STRING    macroName;
};

gceSTATUS
ppoHIDE_SET_Dump(
                                IN    ppoPREPROCESSOR    PP,
                                IN    ppoHIDE_SET        HS
                                );

gceSTATUS
ppoHIDE_SET_Construct(
                      IN    ppoPREPROCESSOR        PP,
                      IN    gctCONST_STRING        File,
                      IN    gctUINT                Line,
                      IN    gctCONST_STRING        MoreInfo,
                      IN    gctSTRING            MacroName,
                      OUT    ppoHIDE_SET*        New
                      );

gceSTATUS
ppoHIDE_SET_Destroy(
                    IN    ppoPREPROCESSOR    PP,
                    IN    ppoHIDE_SET        HS
                    );

gceSTATUS
ppoHIDE_SET_LIST_ContainSelf(
                             IN    ppoPREPROCESSOR    PP,
                             IN    ppoTOKEN        Token,
                             OUT    gctBOOL*        YesOrNo
                             );

gceSTATUS    ppoHIDE_SET_LIST_Append(
                                    ppoPREPROCESSOR        PP,
                                    ppoTOKEN    TarToken,
                                    ppoTOKEN    SrcToken);

gceSTATUS
ppoHIDE_SET_LIST_Colon(
                       IN    ppoPREPROCESSOR        PP,
                       IN    ppoHIDE_SET            From,
                       OUT    ppoHIDE_SET*        New);

gceSTATUS
ppoHIDE_SET_LIST_AddName(
                         IN    ppoPREPROCESSOR    PP,
                         IN    ppoTOKEN        Token,
                         IN    gctSTRING        AddMacroName
                         );

gceSTATUS    ppoHIDE_SET_AddHS(
                              ppoPREPROCESSOR        PP,
                              ppoTOKEN    Token,
                              gctSTRING        MacName);


/******************************************************************************\
|************************** 4 : Input Stream Object ***************************|
\******************************************************************************/

struct _ppoINPUT_STREAM
{
    /*00*/    struct _ppoBASE        base;
    /*01*/    gceSTATUS            (*GetToken)(
        IN        ppoPREPROCESSOR        PP,
        IN    OUT    ppoINPUT_STREAM*    IS,
        OUT        ppoTOKEN*            Token,
        IN        gctBOOL                ICareWhiteSpace
        );
    /*02*/    gceSTATUS            (*Dump)(
        IN        ppoPREPROCESSOR        PP,
        IN        ppoINPUT_STREAM        IS
        );
};

gceSTATUS
ppoINPUT_STREAM_Init(
                     /*00*/    ppoPREPROCESSOR            PP,
                     /*01*/    ppoINPUT_STREAM            YouCreated,
                     /*02*/    gctCONST_STRING            File,
                     /*03*/    gctUINT                    Line,
                     /*04*/    gctCONST_STRING            MoreInfo,
                     /*05*/    ppeOBJECT_TYPE            Type,
                     /*06*/    gceSTATUS                (*GetToken)    (ppoPREPROCESSOR,    ppoINPUT_STREAM*, ppoTOKEN*,gctBOOL)
                     );

gceSTATUS
ppoINPUT_STREAM_UnGetToken(
                           ppoPREPROCESSOR        PP,
                           ppoINPUT_STREAM*    Is,
                           ppoTOKEN            Token
                           );


struct _ppoBYTE_INPUT_STREAM
{
    /*00*/    struct _ppoINPUT_STREAM        inputStream;
    /*01*/    gctCONST_STRING                src;
    /*02*/    gctINT                        count;
    /*03*/    gctINT                        curpos;
    /*04*/    gctINT                        inputStringNumber;
};

gceSTATUS
ppoBYTE_INPUT_STREAM_Construct(
                               /*00*/    ppoPREPROCESSOR        PP,
                               /*01*/    ppoBYTE_INPUT_STREAM        Prev,
                               /*02*/    ppoBYTE_INPUT_STREAM        Next,
                               /*03*/    gctCONST_STRING                File,
                               /*04*/    gctINT                        Line,
                               /*05*/    gctCONST_STRING                MoreInfo,
                               /*06*/    gctCONST_STRING                Src,
                               /*07*/    const gctINT                InputStringNumber,
                               /*08*/    const gctINT                Count,
                               /*09*/    ppoBYTE_INPUT_STREAM*        Created
                               );

gceSTATUS
ppoBYTE_INPUT_STREAM_GetToken(
                              ppoPREPROCESSOR        PP,
                              ppoINPUT_STREAM*,
                              ppoTOKEN*,
                              gctBOOL);

gceSTATUS
ppoBYTE_INPUT_STREAM_Dump(
                          ppoPREPROCESSOR PP,
                          ppoINPUT_STREAM IS);

gceSTATUS
ppoINPUT_STREAM_Dump(
                     ppoPREPROCESSOR PP,
                     ppoINPUT_STREAM IS
                     );


/******************************************************************************\
|*************************** 5 : Macro Symbol Object **************************|
\******************************************************************************/
struct _ppoMACRO_SYMBOL
{
    struct _ppoBASE         base;
    gctSTRING               name;
    gctINT                  argc;
    ppoTOKEN                argv;
    ppoTOKEN                replacementList;
    gctBOOL                 undefined;
    gctBOOL                 hasPara;
};

gceSTATUS
ppoMACRO_SYMBOL_Dump(
                     IN    ppoPREPROCESSOR,
                     IN    ppoMACRO_SYMBOL
                     );

gceSTATUS
ppoMACRO_SYMBOL_Construct(
                          IN    ppoPREPROCESSOR        PP,
                          IN    gctCONST_STRING        File,
                          IN    gctINT        Line,
                          IN    gctCONST_STRING        MoreInfo,
                          IN    gctSTRING        Name,
                          IN    gctINT        Argc,
                          IN    ppoTOKEN    Argv,
                          IN    ppoTOKEN    Rplst,
                          OUT ppoMACRO_SYMBOL*    Created
                          );


struct _ppoMACRO_MANAGER
{
    /*This inputStream the macro symbol table.*/
    /*00*/    struct _ppoBASE    base;
    /*01*/    ppoMACRO_SYMBOL    ir;
};

/*MACRO MANAGER Creat*/
gceSTATUS
ppoMACRO_MANAGER_Construct(
                           ppoPREPROCESSOR        PP,
                           char *                File,
                           gctINT                Line,
                           char *                MoreInfo,
                           ppoMACRO_MANAGER*    Created);

gceSTATUS
ppoMACRO_MANAGER_Destroy(
                         ppoPREPROCESSOR        PP,
                         ppoMACRO_MANAGER    MM);

/*GetMacroSymbol*/
gceSTATUS
ppoMACRO_MANAGER_GetMacroSymbol    (
                                 ppoPREPROCESSOR,
                                 ppoMACRO_MANAGER,
                                 gctSTRING,
                                 ppoMACRO_SYMBOL*);

/*AddMacroSymbol*/
gceSTATUS
ppoMACRO_MANAGER_AddMacroSymbol(
                                ppoPREPROCESSOR        PP,
                                ppoMACRO_MANAGER    Macm,
                                ppoMACRO_SYMBOL        Ms);

gceSTATUS
ppoMACRO_MANAGER_Dump(
                      ppoPREPROCESSOR        PP,
                      ppoMACRO_MANAGER        MM);


/******************************************************************************\
|*************************** 6 : PP Object **************************|
\******************************************************************************/
struct _ppsKEYWORD
{
    struct _ppoBASE        base;
    /*00    #        */    gctSTRING sharp;
    /*01    define    */    gctSTRING define;
    /*02    undef    */    gctSTRING undef;
    /*03    if        */    gctSTRING if_;
    /*04    ifdef    */    gctSTRING ifdef;
    /*05    ifndef    */    gctSTRING ifndef;
    /*06    else    */    gctSTRING else_;
    /*07    elif    */    gctSTRING elif;
    /*08    endif    */    gctSTRING endif;
    /*09    error    */    gctSTRING error;
    /*10    pragma    */    gctSTRING pragma;
    /*11    extension*/    gctSTRING extension;
    /*12    version    */    gctSTRING version;
    /*13    line    */    gctSTRING line;
    /*14    (        */    gctSTRING lpara;
    /*15    )        */    gctSTRING rpara;
    /*16    \n        */    gctSTRING newline;
    /*17    defined    */    gctSTRING defined;
    /*18    -        */    gctSTRING minus;
    /*19    +        */    gctSTRING plus;
    /*20    ||        */    gctSTRING lor;
    /*21    &&        */    gctSTRING land;
    /*22    |        */    gctSTRING bor;
    /*23    &        */    gctSTRING band;
    /*24    ==        */    gctSTRING equal;
    /*25    !=        */    gctSTRING not_equal;
    /*26    >        */    gctSTRING more;
    /*27    <        */    gctSTRING less;
    /*28    >=        */    gctSTRING more_equal;
    /*29    <=        */    gctSTRING less_equal;
    /*30    <<        */    gctSTRING lshift;
    /*31    >>        */    gctSTRING rshift;
    /*32    *        */    gctSTRING mul;
    /*33    /        */    gctSTRING div;
    /*34    %        */    gctSTRING perc;
    /*35    +        */    gctSTRING positive;
    /*36    -        */    gctSTRING negative;
    /*37    ~        */    gctSTRING banti;
    /*38    !        */    gctSTRING lanti;
    /*39    ^        */    gctSTRING bex;
    /*40    EOF        */    gctSTRING eof;
    /*41 WhiteSpace    */    gctSTRING ws;
    /*42    ,        */    gctSTRING comma;
    /*43    version number*/    gctSTRING version_100;
    /*44    :        */    gctSTRING colon;
    /*45    require    */    gctSTRING require;
    /*45    enable    */    gctSTRING enable;
    /*45    warn    */    gctSTRING warn;
    /*45    disable    */  gctSTRING disable;
    /*50    __LINE__*/    gctSTRING    _line_;
    /*51    __FILE__*/    gctSTRING    _file_;
    /*52    __VERSION__*/gctSTRING    _version_;
    /*53    GL_ES    */    gctSTRING    gl_es;
    /*54    GL_        */    gctSTRING    gl_;
    /*55    all        */    gctSTRING    all;
    /*56    include    */    gctSTRING    include;
    /*57     nulstr    */    gctSTRING    nul_str;
};

/******************************************************************************\
**
**    struct _ppoPREPROCESSOR
**
**        This is the basic class contain the debug info and type. It is
**        included in every "object" of preprocessor.
**
**    This depends on other:
**
**        gcvNULL
**
**    Other depends on this:
**
**        1.ppoBASE_Init(),
**        2.ppoPREPROCESSOR_SetSourceStrings().
**
**    History:
**
**        2008/08/01, qizhuang.liu.
**
**            add *dirty* member.
**            add ppoPREPROCESSOR_IsDirty().
**
**            This member is used to judge whether the source strings
**            and other states of preprocessor has been setted.
**
*/
/*C : ppoPREPROCESSOR_Construct */
/*D : ...                       */
/*S : ...                       */
/*R                             */
struct _ppoPREPROCESSOR
{
    struct _ppoBASE            base;
    cloCOMPILER                compiler;
    gctCONST_STRING*           strings;
    gctUINT*                   lens;
    gctUINT                    count;
    gctBOOL                    otherStatementHasAlreadyAppeared;
    gctBOOL                    versionStatementHasAlreadyAppeared;
    ppoMACRO_MANAGER           macroManager;
    ppoINPUT_STREAM            inputStream;
    ppsKEYWORD                 keyword;
    gctINT                     currentSourceFileStringNumber;
    gctINT                     currentSourceFileLineNumber;
    gctCHAR                    currentSourceFileStringName[1024];
    gctSTRING**                operators;
    ppoTOKEN                   outputTokenStreamHead;
    ppoTOKEN                   outputTokenStreamEnd;
    ppoBYTE_INPUT_STREAM       lastGetcharPhase0IsFromThisBis;
    gctBOOL                    iAmFollowingAComment;
    gctBOOL                    doWeInValidArea;
    gctBOOL                    dirty;
    gctUINT                    version;
    gctSTRING*                 ppedStrings;
    gctUINT                    ppedCount;
    gctBOOL                    useNewPP;
    gctSTRING                  extensionString;
    ppoHEADERFILEPATH          headerFilePathList;
    gctSTRING                  macroString;
    gctSIZE_T                  macroStringSize;
    gctBOOL                    toLineEnd;
    gctINT                     skipLine;
    gctBOOL                    nonpreprocessorStatementHasAlreadyAppeared;
    gctCHAR                    logBuffer[_cldBUFFER_MAX];
    gctFILE                    ppLogFile;
    gctINT                     ppLineNumber;
    gctUINT                    logCurrentSize;

    /*
    ** to skip undefined identifiers error,
    ** such as: #if 1 || AA  and # if 0 && AA
    */
    gctBOOL                    skipOPError;
};

gceSTATUS
ppoPREPROCESSOR_Construct(
                          cloCOMPILER         Compiler,
                          ppoPREPROCESSOR*    PP,
                          gctBOOL             UseNewPP
                          );

gceSTATUS
ppoPREPROCESSOR_Destroy(
                        ppoPREPROCESSOR    PP
                        );

gceSTATUS
ppoPREPROCESSOR_Reset (
                       ppoPREPROCESSOR PP
                       );

gceSTATUS
ppoPREPROCESSOR_SetSourceStrings(
                                 ppoPREPROCESSOR    PP,
                                 gctCONST_STRING*   Strings,
                                 gctUINT            Count,
                                 gctCONST_STRING    Options
                                 );

gceSTATUS
ppoPREPROCESSOR_Parse(
                      cloPREPROCESSOR PP,
                      char *Buffer,
                      gctUINT Max,
                      gctINT *WriteInNumber);

gceSTATUS
ppoPREPROCESSOR_AddToOutputStreamOfPP(
                                      ppoPREPROCESSOR PP,
                                      ppoTOKEN Token
                                      );

gceSTATUS
ppoPREPROCESSOR_PushOntoCurrentInputStreamOfPP (
    ppoPREPROCESSOR PP,
    ppoINPUT_STREAM Is
    );

gceSTATUS
ppoPREPROCESSOR_DumpOutputStream(
                                 ppoPREPROCESSOR PP
                                 );

gceSTATUS
ppoPREPROCESSOR_DumpInputStream(
                                ppoPREPROCESSOR PP
                                );

gceSTATUS
ppoPREPROCESSOR_ShiftOverEndOfLine(
                                   ppoPREPROCESSOR PP
                                   );

gceSTATUS
ppoPREPROCESSOR_PreprocessingFile(
                                  ppoPREPROCESSOR PP
                                  );

gceSTATUS
ppoPREPROCESSOR_PassEmptyLine(
                              ppoPREPROCESSOR PP
                              );

gceSTATUS
ppoPREPROCESSOR_Group(
                      ppoPREPROCESSOR PP
                      );

gceSTATUS
ppoPREPROCESSOR_GroupPart(
                          ppoPREPROCESSOR PP
                          );

gceSTATUS
ppoPREPROCESSOR_IfSection(
                          ppoPREPROCESSOR PP
                          );

gceSTATUS
ppoPREPROCESSOR_Defined(
                        ppoPREPROCESSOR PP,
                        gctSTRING*        Return
                        );

gceSTATUS
ppoPREPROCESSOR_TextLine(
                         ppoPREPROCESSOR PP
                         );

gceSTATUS
ppoPREPROCESSOR_ControlLine(
                            ppoPREPROCESSOR PP
                            );

gceSTATUS
ppoPREPROCESSOR_Version(
                        ppoPREPROCESSOR PP
                        );

gceSTATUS
ppoPREPROCESSOR_Line(
                     ppoPREPROCESSOR PP
                     );

gceSTATUS
ppoPREPROCESSOR_Error(
                      ppoPREPROCESSOR PP
                      );

gceSTATUS
ppoPREPROCESSOR_Pragma(
                       ppoPREPROCESSOR PP
                       );

gceSTATUS
ppoPREPROCESSOR_Undef(
                      ppoPREPROCESSOR PP
                      );

gceSTATUS
ppoPREPROCESSOR_Define(
                       ppoPREPROCESSOR PP
                       );

gceSTATUS
ppoPREPROCESSOR_Include(
                       ppoPREPROCESSOR PP
                       );

gceSTATUS
ppoPREPROCESSOR_Define_BufferArgs(
                                  ppoPREPROCESSOR PP,
                                  ppoTOKEN*        args,
                                  gctINT*            argc
                                  );

gceSTATUS
ppoPREPROCESSOR_ArgsMacroExpand(
                                ppoPREPROCESSOR PP,
                                ppoTOKEN*        HeadIn,
                                ppoTOKEN*        HeadOut,
                                ppoTOKEN*        EndOut);

gceSTATUS
ppoPREPROCESSOR_BufferActualArgs(
                                 ppoPREPROCESSOR        PP,
                                 ppoINPUT_STREAM*    IS,
                                 ppoTOKEN*            Head,
                                 ppoTOKEN*            End);

gceSTATUS
ppoPREPROCESSOR_Define_BufferReplacementList(
    ppoPREPROCESSOR PP,
    ppoTOKEN*        RList);

gceSTATUS
ppoPREPROCESSOR_MatchDoubleToken(
                                 ppoPREPROCESSOR    PP,
                                 gctSTRING        NotWSStr1,
                                 gctSTRING        NotWSStr2,
                                 gctBOOL*        Match);

gceSTATUS
ppoPREPROCESSOR_IsOpTokenInThisLevel(
    ppoPREPROCESSOR        PP,
    ppoTOKEN            Token,
    gctINT                Level,
    gctBOOL*            Result
    );

gceSTATUS
ppoPREPROCESSOR_GuardTokenOfThisLevel(
                                      ppoPREPROCESSOR    PP,
                                      ppoTOKEN        Token,
                                      gctSTRING        OptGuarder,
                                      gctINT            Level,
                                      gctBOOL*        Result);

gceSTATUS
ppoPREPROCESSOR_Eval(
                     ppoPREPROCESSOR    PP,
                     gctSTRING        OptGuarder,
                     gctINT            Level,
                     gctINT*            Result);

gceSTATUS
ppoPREPROCESSOR_EvalInt(
                        ppoPREPROCESSOR        PP,
                        ppoTOKEN            Token,
                        gctINT*                Result);



gceSTATUS
ppoPREPROCESSOR_Report(
                       ppoPREPROCESSOR PP,
                       cleREPORT_TYPE    Type,
                       gctCONST_STRING Message,
                       ...
                       );

gctINT
ppoPREPROCESSOR_Pow(
                    gctINT x,
                    gctINT y
                    );

gceSTATUS
ppoPREPROCESSOR_ToEOL(
                      ppoPREPROCESSOR PP
                      );


gceSTATUS
ppoPREPROCESSOR_addMacroDef_Int(
                                ppoPREPROCESSOR  PP,
                                gctSTRING  MacroStr,
                                gctSTRING  ValueStr
                               );


/*Alphabet Number, _*/
gctBOOL
ppoPREPROCESSOR_isalnum_(char c);

/*Number*/
gctBOOL
ppoPREPROCESSOR_isnum(char c);

/*HEX Number*/
gctBOOL
ppoPREPROCESSOR_ishexnum(char c);

/*OCT*/
gctBOOL
ppoPREPROCESSOR_isoctnum(char c);

/*Alphabet,_*/
gctBOOL
ppoPREPROCESSOR_isal_(char c);

/*Character Set*/
gctBOOL
ppoPREPROCESSOR_islegalchar(char c);

/*White Space*/
gctBOOL
ppoPREPROCESSOR_isws(char c);

/*Newline*/
gctBOOL
ppoPREPROCESSOR_isnl(char c);


/*Set string*/
gceSTATUS
ppoPREPROCESSOR_setnext(
                        ppoPREPROCESSOR  PP,
                        char    c,
                        gctSTRING    cb,
                        gctINT* pcblen);

/*Single Punctuator*/
gctBOOL
ppoPREPROCESSOR_isnspunc(char c);

/*Punctuator*/
gctBOOL
ppoPREPROCESSOR_ispunc(char c);

/******************************************************************************\
|****************************** 7 : Token Object ******************************|
\******************************************************************************/

typedef enum
{
    ppvTokenType_ERROR,
    ppvTokenType_EOF,
    ppvTokenType_INT,
    ppvTokenType_FLOAT,
    ppvTokenType_ID,
    ppvTokenType_PUNC,
    ppvTokenType_NL,
    ppvTokenType_WS,
    ppvTokenType_NUL,
    ppvTokenType_NOT_IN_LEGAL_CHAR_SET
}
ppeTokenType;

struct _ppoTOKEN
{
    /*00*/    struct _ppoINPUT_STREAM        inputStream;
    /*02*/    ppeTokenType                type;
    /*03*/    ppoHIDE_SET                    hideSet;
    /*04*/    cltPOOL_STRING                poolString;
    /*05*/    gctINT                        srcFileString;
    /*06*/    gctINT                        srcFileLine;
    /*07*/    gctBOOL                       hasLeadingWS;
    /*08*/    gctBOOL                       hasTrailingControl;
};

gceSTATUS
ppoTOKEN_Construct(
                   /*00*/    ppoPREPROCESSOR    PP,
                   /*01*/    gctCONST_STRING    File,
                   /*02*/    gctINT            Line,
                   /*03*/    gctCONST_STRING    MoreInfo,
                   /*04*/    ppoTOKEN*            Created
                   );

gceSTATUS
ppoTOKEN_Destroy(
                 ppoPREPROCESSOR PP,
                 ppoTOKEN_STREAM Token
                 );

gceSTATUS
ppoTOKEN_GetToken(
                  IN    ppoPREPROCESSOR        PP,
                  IN    ppoINPUT_STREAM*    IS,
                  OUT    ppoTOKEN*            Result,
                  IN    gctBOOL                ICareWhiteSpace
                  );

gceSTATUS
ppoTOKEN_Colon(
               ppoPREPROCESSOR        PP,
               ppoTOKEN            Token,
               gctCONST_STRING        File,
               gctINT                Line,
               gctCONST_STRING        MoreInfo,
               ppoTOKEN*            New);

gceSTATUS
ppoTOKEN_ColonTokenList(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             SrcTLst,
    IN gctCONST_STRING      File,
    IN gctINT               Line,
    IN gctCONST_STRING      MoreInfo,
    IN ppoTOKEN*            ColonedHead,
    IN ppoTOKEN             RefID
    );

gceSTATUS
ppoTOKEN_Dump(
              ppoPREPROCESSOR   PP,
              ppoTOKEN_STREAM      TokenStream);

gceSTATUS
ppoTOKEN_STREAM_FindID(
                       IN    ppoPREPROCESSOR        PP,
                       IN    ppoTOKEN            TokenList,
                       IN    gctSTRING            ID,
                       OUT    gctBOOL*            Result);


gceSTATUS
ppoTOKEN_FindPoolString(
                        ppoPREPROCESSOR        PP,
                        ppoTOKEN_STREAM        TokenStream,
                        gctSTRING            FindWhat,
                        ppoTOKEN*            Result
                        );

gceSTATUS
ppoTOKEN_STREAM_Colon(
                      ppoPREPROCESSOR        PP,
                      ppoTOKEN_STREAM        SrcTLst,
                      gctCONST_STRING        File,
                      gctINT                Line,
                      gctCONST_STRING        MoreInfo,
                      ppoTOKEN_STREAM*    New
                      );

gceSTATUS
ppoTOKEN_STREAM_Find(
                     IN    ppoPREPROCESSOR        PP,
                     IN    ppoTOKEN_STREAM        TokenStream,
                     IN    ppoTOKEN            Findwhat,
                     OUT    gctBOOL*            Result
                     );


gceSTATUS
ppoTOKEN_STREAM_Destroy(
                        ppoPREPROCESSOR PP,
                        ppoTOKEN_STREAM Token
                        );

gceSTATUS
ppoTOKEN_STREAM_Dump(
                     ppoPREPROCESSOR PP,
                     ppoTOKEN_STREAM TS
                     );


gceSTATUS
cloCOMPILER_SetStringNumber(
                            cloCOMPILER        compiler,
                            gctINT            String
                            );

gceSTATUS
cloCOMPILER_SetLineNumber(
                          cloCOMPILER Compiler,
                          gctINT Line
                          );

gceSTATUS
cloCOMPILER_SetDebug(
                     cloCOMPILER Compiler,
                     gctBOOL        Debug
                     );

gceSTATUS
cloCOMPILER_SetOptimize(
                        cloCOMPILER Compiler,
                        gctBOOL        Optimize
                        );

gceSTATUS
cloCOMPILER_SetVersion(
                       cloCOMPILER Compiler,
                       gctINT Line
                       );


/******************************************************************************\
|******************************  Header File Path  ****************************|
\******************************************************************************/

struct _ppoHEADERFILEPATH
{
    struct _ppoBASE     base;
    gctSTRING headerFilePath;
};

gceSTATUS
ppoPREPROCESSOR_AddHeaderFilePathToList(
                                        ppoPREPROCESSOR PP,
                                        gctSTRING   Path
                                        );

gceSTATUS
ppoPREPROCESSOR_FreeHeaderFilePathList(
                                       ppoPREPROCESSOR PP
                                       );

gceSTATUS
ppoWriteBufferToFile(ppoPREPROCESSOR PP);

#include "gc_cl_macro_expand.h"

#endif /* __gc_cl_preprocessor_int_h_ */
