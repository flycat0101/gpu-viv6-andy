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


#ifndef __gc_glsl_preprocessor_int_h_
#define __gc_glsl_preprocessor_int_h_

#include "gc_glsl_preprocessor.h"

#define _ppd_HALTI_SUPPORT  1

/******************************************************************************\
|****************************** Macro Definition ******************************|
\******************************************************************************/
#define ppvICareWhiteSpace          1
#define ppvBINARY_OP                2
#define ppvUNARY_OP                 1
#define ppmSTATE_0_CONSTRUCTED      0
#define ppmSTATE_1_RESETED          1
#define ppmSTATE_2_SRCSETTED        2
#define ppmSTATE_3_PARSED           3
#define ppmSTATE_4_DESTROIED        4

#define ppmCheckOK() \
do\
{\
    if(status != gcvSTATUS_OK) \
    {\
        gcmFOOTER(); \
        return status; \
    }\
}\
while(gcvFALSE)

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

typedef enum _ppeIFSECTION_TYPE
{
    ppvIFSECTION_NONE       = 0,
    ppvIFSECTION_IF         = 1,
    ppvIFSECTION_ELSE       = 2,
    ppvIFSECTION_ELIF       = 3,
    ppvIFSECTION_ENDIF      = 4
}ppeIFSECTION_TYPE;

/******************************************************************************\
|****************************** 1 : Base Object *******************************|
\******************************************************************************/
typedef enum _ppeOBJECT_TYPE
{
    /*00*/    ppvOBJ_UNKNOWN                    =    0,
    /*01*/    ppvOBJ_MEMORY_MANAGER             =    gcmCC('M','M',' ','\0'),
    /*02*/    ppvOBJ_STRING_MANAGER             =    gcmCC('S','M',' ','\0'),
    /*03*/    ppvOBJ_MACRO_MANAGER              =    gcmCC('M','A','M','\0'),
    /*04*/    ppvOBJ_TOKEN                      =    gcmCC('T','O','K','\0'),
    /*05*/    ppvOBJ_PREPROCESSOR               =    gcmCC('P','P',' ','\0'),
    /*06*/    ppvOBJ_HIDE_SET                   =    gcmCC('H','S',' ','\0'),
    /*07*/    ppvOBJ_BYTE_INPUT_STREAM          =    gcmCC('B','I','S','\0'),
    /*08*/    ppvOBJ_STRING_MANAGER_IR          =    gcmCC('S','M','I','\0'),
    /*09*/    ppvOBJ_MACRO_SYMBOL               =    gcmCC('M','S',' ','\0'),
    /*10*/    ppvOBJ_STRING                     =    gcmCC('S','T','R','\0'),
    /*11*/    ppvOBJ_MEMORY_MANAGER_IR          =    gcmCC('M','M','I','\0'),
    /*12*/    ppvOBJ_KEYWORD                    =    gcmCC('K','W',' ','\0'),
    /*13*/    ppvOBJ_ARRAY                      =    gcmCC('A','R','R','\0')
} ppeOBJECT_TYPE;

gceSTATUS
ppeOBJECT_TypeString(
    IN ppeOBJECT_TYPE       TypeEnum,
    OUT gctCONST_STRING*    TypeString
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
    /*00*/    slsDLINK_NODE         node;
    /*01*/    ppeOBJECT_TYPE        type;
    /*02*/    gctCONST_STRING       file;
    /*03*/    gctINT                line;
    /*04*/    gctCONST_STRING       info;
};

gceSTATUS
ppoBASE_Dump(
    IN ppoPREPROCESSOR      PP,
    IN ppoBASE              Base
    );

gceSTATUS
ppoBASE_Init(
    IN ppoPREPROCESSOR      PP,
    IN OUT ppoBASE          YourBase,
    IN gctCONST_STRING      File,
    IN gctUINT              Line,
    IN gctCONST_STRING      MoreInfo,
    IN ppeOBJECT_TYPE       Type
    );


/******************************************************************************\
|**************************** 3 : Hide Set Object *****************************|
\******************************************************************************/

struct _ppoHIDE_SET
{
    struct _ppoBASE         base;
    sltPOOL_STRING          macroName;
};

gceSTATUS
ppoHIDE_SET_Dump(
    IN ppoPREPROCESSOR      PP,
    IN ppoHIDE_SET          HS
    );

gceSTATUS
ppoHIDE_SET_Construct(
    IN ppoPREPROCESSOR      PP,
    IN gctCONST_STRING      File,
    IN gctUINT              Line,
    IN gctCONST_STRING      MoreInfo,
    IN gctSTRING            MacroName,
    OUT ppoHIDE_SET*        New
    );

gceSTATUS
ppoHIDE_SET_Destroy(
    IN ppoPREPROCESSOR      PP,
    IN ppoHIDE_SET          HS
    );

gceSTATUS
ppoHIDE_SET_LIST_ContainSelf(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             Token,
    OUT gctBOOL*            YesOrNo
    );

gceSTATUS
ppoHIDE_SET_LIST_Append(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             TarToken,
    IN ppoTOKEN             SrcToken
    );

gceSTATUS
ppoHIDE_SET_LIST_Colon(
    IN ppoPREPROCESSOR      PP,
    IN ppoHIDE_SET          From,
    OUT ppoHIDE_SET*        New
    );

gceSTATUS
ppoHIDE_SET_LIST_AddName(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             Token,
    IN gctSTRING            AddMacroName
    );

gceSTATUS
ppoHIDE_SET_AddHS(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             Token,
    IN gctSTRING            MacName);

/******************************************************************************\
|************************** 4 : Input Stream Object ***************************|
\******************************************************************************/

struct _ppoINPUT_STREAM
{
    struct _ppoBASE         base;
    gceSTATUS               (*GetToken)(
        IN ppoPREPROCESSOR              PP,
        IN OUT ppoINPUT_STREAM*         IS,
        OUT ppoTOKEN*                   Token,
        IN gctBOOL                      ICareWhiteSpace
        );
    gceSTATUS               (*Dump)(
        IN ppoPREPROCESSOR              PP,
        IN ppoINPUT_STREAM              IS
        );
};

gceSTATUS
ppoINPUT_STREAM_Init(
    IN ppoPREPROCESSOR      PP,
    IN ppoINPUT_STREAM      YouCreated,
    IN gctCONST_STRING      File,
    IN gctUINT              Line,
    IN gctCONST_STRING      MoreInfo,
    IN ppeOBJECT_TYPE       Type,
    IN gceSTATUS            (*GetToken)(ppoPREPROCESSOR,    ppoINPUT_STREAM*, ppoTOKEN*,gctBOOL)
    );

gceSTATUS
ppoINPUT_STREAM_UnGetToken(
    IN ppoPREPROCESSOR      PP,
    IN ppoINPUT_STREAM*     Is,
    IN ppoTOKEN             Token
    );


struct _ppoBYTE_INPUT_STREAM
{
    struct _ppoINPUT_STREAM inputStream;
    gctCONST_STRING         src;
    gctINT                  count;
    gctINT                  curpos;
    gctINT                  inputStringNumber;
};

gceSTATUS
ppoBYTE_INPUT_STREAM_Construct(
    IN ppoPREPROCESSOR      PP,
    IN ppoBYTE_INPUT_STREAM Prev,
    IN ppoBYTE_INPUT_STREAM Next,
    IN gctCONST_STRING      File,
    IN gctINT               Line,
    IN gctCONST_STRING      MoreInfo,
    IN gctCONST_STRING      Src,
    IN const gctINT         InputStringNumber,
    IN const gctINT         Count,
    IN ppoBYTE_INPUT_STREAM*Created
    );

gceSTATUS
ppoBYTE_INPUT_STREAM_GetToken(
    IN ppoPREPROCESSOR      PP,
    IN ppoINPUT_STREAM*     Stream,
    IN ppoTOKEN*            Token,
    IN gctBOOL              CareWS);

gceSTATUS
ppoBYTE_INPUT_STREAM_Dump(
    IN ppoPREPROCESSOR      PP,
    IN ppoINPUT_STREAM      IS
    );

gceSTATUS
ppoINPUT_STREAM_Dump(
    IN ppoPREPROCESSOR      PP,
    IN ppoINPUT_STREAM      IS
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
    IN ppoPREPROCESSOR      PP,
    IN ppoMACRO_SYMBOL      Symbol
    );

gceSTATUS
ppoMACRO_SYMBOL_Construct(
    IN ppoPREPROCESSOR      PP,
    IN gctCONST_STRING      File,
    IN gctINT               Line,
    IN gctCONST_STRING      MoreInfo,
    IN gctSTRING            Name,
    IN gctINT               Argc,
    IN ppoTOKEN             Argv,
    IN ppoTOKEN             Rplst,
    OUT ppoMACRO_SYMBOL*    Created
    );

struct _ppoMACRO_MANAGER
{
    /*This inputStream the macro symbol table.*/
    struct _ppoBASE         base;
    ppoMACRO_SYMBOL         ir;
};

/*MACRO MANAGER Creat*/
gceSTATUS
ppoMACRO_MANAGER_Construct(
    IN ppoPREPROCESSOR      PP,
    IN char *               File,
    IN gctINT               Line,
    IN char *               MoreInfo,
    IN ppoMACRO_MANAGER*    Created
    );

gceSTATUS
ppoMACRO_MANAGER_Destroy(
    IN ppoPREPROCESSOR      PP,
    IN ppoMACRO_MANAGER     MM
    );

/*GetMacroSymbol*/
gceSTATUS
ppoMACRO_MANAGER_GetMacroSymbol(
    IN ppoPREPROCESSOR      PP,
    IN ppoMACRO_MANAGER     Macm,
    IN gctSTRING            Name,
    IN ppoMACRO_SYMBOL*     Found);

/*AddMacroSymbol*/
gceSTATUS
ppoMACRO_MANAGER_AddMacroSymbol(
    IN ppoPREPROCESSOR      PP,
    IN ppoMACRO_MANAGER     Macm,
    IN ppoMACRO_SYMBOL      Ms
    );

gceSTATUS
ppoMACRO_MANAGER_DestroyMacroSymbol(
    IN ppoPREPROCESSOR      PP,
    IN ppoMACRO_MANAGER     Macm,
    IN ppoMACRO_SYMBOL      Ms
    );

gceSTATUS
ppoMACRO_MANAGER_Dump(
    IN ppoPREPROCESSOR      PP,
    IN ppoMACRO_MANAGER     MM
    );

/******************************************************************************\
|*************************** 6 : PP Object **************************|
\******************************************************************************/
struct _ppsKEYWORD
{
    struct _ppoBASE         base;
    gctSTRING               sharp;      /*00 # */
    gctSTRING               define;     /*01 define */
    gctSTRING               undef;      /*02 undef */
    gctSTRING               if_;        /*03 if */
    gctSTRING               ifdef;      /*04 ifdef */
    gctSTRING               ifndef;     /*05 ifndef */
    gctSTRING               else_;      /*06 else */
    gctSTRING               elif;       /*07 elif */
    gctSTRING               endif;      /*08 endif */
    gctSTRING               error;      /*09 error */
    gctSTRING               pragma;     /*10 pragma */
    gctSTRING               extension;  /*11 extension */
    gctSTRING               version;    /*12 version */
    gctSTRING               line;       /*13 line */
    gctSTRING               lpara;      /*14 ( */
    gctSTRING               rpara;      /*15 ) */
    gctSTRING               newline;    /*16 \n */
    gctSTRING               defined;    /*17 defined */
    gctSTRING               minus;      /*18 - */
    gctSTRING               plus;       /*19 + */
    gctSTRING               lor;        /*20 || */
    gctSTRING               land;       /*21 && */
    gctSTRING               bor;        /*22 | */
    gctSTRING               band;       /*23 & */
    gctSTRING               equal;      /*24 == */
    gctSTRING               not_equal;  /*25 != */
    gctSTRING               more;       /*26 > */
    gctSTRING               less;       /*27 < */
    gctSTRING               more_equal; /*28 >= */
    gctSTRING               less_equal; /*29 <= */
    gctSTRING               lshift;     /*30 << */
    gctSTRING               rshift;     /*31 >> */
    gctSTRING               mul;        /*32 * */
    gctSTRING               div;        /*33 / */
    gctSTRING               perc;       /*34 % */
    gctSTRING               positive;   /*35 + */
    gctSTRING               negative;   /*36 - */
    gctSTRING               banti;      /*37 ~ */
    gctSTRING               lanti;      /*38 ! */
    gctSTRING               bex;        /*39 ^ */
    gctSTRING               eof;        /*40 EOF */
    gctSTRING               ws;         /*41 WhiteSpace */
    gctSTRING               comma;      /*42 , */
    gctSTRING               version_100;/*43 version 100 */
    gctSTRING               colon;      /*44 : */
    gctSTRING               require;    /*45 require */
    gctSTRING               enable;     /*46 enable */
    gctSTRING               warn;       /*47 warn */
    gctSTRING               disable;    /*48 diable */
    gctSTRING               _line_;     /*49 __LINE__ */
    gctSTRING               _file_;     /*50 __FILE__ */
    gctSTRING               _version_;  /*51 __VERSION__ */
    gctSTRING               gl_es;      /*52 GL_ES */
    gctSTRING               gl_;        /*53 GL_ */
    gctSTRING               all;        /*54 all */
    gctSTRING               STDGL;      /*55 STDGL */
    gctSTRING               debug;      /*56 debug */
    gctSTRING               optimize;   /*57 optimize */
    gctSTRING               nul_str;    /*58 nulstr */
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
    struct _ppoBASE             base;
    sloCOMPILER                 compiler;
    gctSTRING                   extensionString;
    gctCONST_STRING*            strings;
    gctUINT*                    lens;
    gctUINT                     count;
    gctBOOL                     otherStatementHasAlreadyAppeared;
    gctBOOL                     versionStatementHasAlreadyAppeared;
    ppoMACRO_MANAGER            macroManager;
    ppoINPUT_STREAM             inputStream;
    ppsKEYWORD                  keyword;
    gctINT                      currentSourceFileStringNumber;
    gctINT                      currentSourceFileLineNumber;
    gctSTRING**                 operators;
    ppoTOKEN                    outputTokenStreamHead;
    ppoTOKEN                    outputTokenStreamEnd;
    ppoBYTE_INPUT_STREAM        lastGetcharPhase0IsFromThisBis;
    gctBOOL                     iAmFollowingAComment;
    gctBOOL                     doWeInValidArea;
    gctBOOL                     dirty;
    gctUINT                     version;
    gctBOOL                     toLineEnd;
    gctINT                      skipLine;
    gctBOOL                     nonpreprocessorStatementHasAlreadyAppeared;

    /*
    ** to skip undefined identifiers error,
    ** such as: #if 1 || AA  and # if 0 && AA
    */
    gctBOOL                     skipOPError;
};

gceSTATUS
ppoPREPROCESSOR_Construct(
    IN sloCOMPILER          Compiler,
    IN ppoPREPROCESSOR*     PP
    );

gceSTATUS
ppoPREPROCESSOR_Destroy(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_Reset(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_SetSourceStrings(
    IN ppoPREPROCESSOR      PP,
    IN gctCONST_STRING*     Strings,
    IN gctUINT_PTR          Lens,
    IN gctUINT              Count
    );

gceSTATUS
ppoPREPROCESSOR_Parse(
    IN ppoPREPROCESSOR      PP,
    IN char*                Buffer,
    IN gctUINT              Max,
    IN gctINT*              WriteInNumber
    );

gceSTATUS
ppoPREPROCESSOR_AddToOutputStreamOfPP(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             Token
    );

gceSTATUS
ppoPREPROCESSOR_PushOntoCurrentInputStreamOfPP (
    IN ppoPREPROCESSOR      PP,
    IN ppoINPUT_STREAM      Is
    );

gceSTATUS
ppoPREPROCESSOR_DumpOutputStream(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_DumpInputStream(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_ShiftOverEndOfLine(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_PreprocessingFile(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_PassEmptyLine(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_Group(
    IN ppoPREPROCESSOR      PP,
    IN ppeIFSECTION_TYPE    IfSectionType
    );

gceSTATUS
ppoPREPROCESSOR_GroupPart(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_IfSection(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             CurrentToken
    );

gceSTATUS
ppoPREPROCESSOR_Defined(
    IN ppoPREPROCESSOR      PP,
    IN gctSTRING*           Return
    );

gceSTATUS
ppoPREPROCESSOR_TextLine(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_ControlLine(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             CurrentToken
    );

gceSTATUS
ppoPREPROCESSOR_Version(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_Line(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_Extension(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_Error(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_Pragma(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_Undef(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_Define(
    IN ppoPREPROCESSOR      PP
    );

gceSTATUS
ppoPREPROCESSOR_Define_BufferArgs(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN*            args,
    IN gctINT*              argc
    );

gceSTATUS
ppoPREPROCESSOR_ArgsMacroExpand(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN*            HeadIn,
    IN ppoTOKEN*            HeadOut,
    IN ppoTOKEN*            EndOut
    );

gceSTATUS
ppoPREPROCESSOR_BufferActualArgs(
    IN ppoPREPROCESSOR      PP,
    IN ppoINPUT_STREAM*     IS,
    IN ppoTOKEN*            Head,
    IN ppoTOKEN*            End
    );

gceSTATUS
ppoPREPROCESSOR_Define_BufferReplacementList(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN*            RList
    );

gceSTATUS
ppoPREPROCESSOR_MatchDoubleToken(
    IN ppoPREPROCESSOR      PP,
    IN gctSTRING            NotWSStr1,
    IN gctSTRING            NotWSStr2,
    IN gctBOOL*             Match
    );

gceSTATUS
ppoPREPROCESSOR_IsOpTokenInThisLevel(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             Token,
    IN gctINT               Level,
    IN gctBOOL*             Result
    );

gceSTATUS
ppoPREPROCESSOR_GuardTokenOfThisLevel(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             Token,
    IN gctSTRING            OptGuarder,
    IN gctINT               Level,
    IN gctBOOL*             Result
    );

gceSTATUS
ppoPREPROCESSOR_Eval_GetToken(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN*            Token,
    IN gctBOOL              ICareWhiteSpace
    );

gceSTATUS
ppoPREPROCESSOR_Eval(
    IN ppoPREPROCESSOR      PP,
    IN gctSTRING            OptGuarder,
    IN gctINT               Level,
    IN gctBOOL              EvaluateLine,
    IN gctBOOL*             MeetStringNum,
    IN gctINT*              Result
    );

gceSTATUS
ppoPREPROCESSOR_EvalInt(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             Token,
    IN gctINT*              Result
    );

gceSTATUS
ppoPREPROCESSOR_Report(
    IN ppoPREPROCESSOR      PP,
    IN sleREPORT_TYPE       Type,
    IN gctCONST_STRING      Message,
    ...
    );

gctINT
ppoPREPROCESSOR_Pow(
    IN gctINT               x,
    IN gctINT               y
    );

gceSTATUS
ppoPREPROCESSOR_ToEOL(
    IN ppoPREPROCESSOR      PP
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
    IN ppoPREPROCESSOR      PP,
    IN char                 c,
    IN gctSTRING            cb,
    IN gctINT*              pcblen
    );

/*Single Punctuator*/
gctBOOL
ppoPREPROCESSOR_isnspunc(char c);

/*Punctuator*/
gctBOOL
ppoPREPROCESSOR_ispunc(char c);

gceSTATUS
ppoPREPROCESSOR_InitExtensionTable(
    IN ppoPREPROCESSOR      PP
    );

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
    struct _ppoINPUT_STREAM inputStream;
    ppeTokenType            type;
    ppoHIDE_SET             hideSet;
    sltPOOL_STRING          poolString;
    gctINT                  srcFileString;
    gctINT                  srcFileLine;
    gctBOOL                 hasLeadingWS;
    gctBOOL                 hasTrailingControl;
};

gceSTATUS
ppoTOKEN_Construct(
    IN ppoPREPROCESSOR      PP,
    IN gctCONST_STRING      File,
    IN gctINT               Line,
    IN gctCONST_STRING      MoreInfo,
    IN ppoTOKEN*            Created
    );

gceSTATUS
ppoTOKEN_Destroy(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN_STREAM      Token
    );

gceSTATUS
ppoTOKEN_GetToken(
    IN ppoPREPROCESSOR      PP,
    IN ppoINPUT_STREAM*     IS,
    OUT ppoTOKEN*           Result,
    IN gctBOOL              ICareWhiteSpace
    );

gceSTATUS
ppoTOKEN_Colon(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             Token,
    IN gctCONST_STRING      File,
    IN gctINT               Line,
    IN gctCONST_STRING      MoreInfo,
    IN ppoTOKEN*            New
    );

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
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN_STREAM      TokenStream
    );

gceSTATUS
ppoTOKEN_STREAM_FindID(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN             TokenList,
    IN gctSTRING            ID,
    OUT gctBOOL*            Result
    );

gceSTATUS
ppoTOKEN_FindPoolString(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN_STREAM      TokenStream,
    IN gctSTRING            FindWhat,
    IN ppoTOKEN*            Result
    );

gceSTATUS
ppoTOKEN_STREAM_Colon(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN_STREAM      SrcTLst,
    IN gctCONST_STRING      File,
    IN gctINT               Line,
    IN gctCONST_STRING      MoreInfo,
    IN ppoTOKEN_STREAM*     New
    );

gceSTATUS
ppoTOKEN_STREAM_Find(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN_STREAM      TokenStream,
    IN ppoTOKEN             Findwhat,
    OUT gctBOOL*            Result
    );

gceSTATUS
ppoTOKEN_STREAM_Destroy(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN_STREAM      Token
    );

gceSTATUS
ppoTOKEN_STREAM_Dump(
    IN ppoPREPROCESSOR      PP,
    IN ppoTOKEN_STREAM      TS
    );


gceSTATUS
sloCOMPILER_SetStringNumber(
    IN sloCOMPILER          compiler,
    IN gctINT               String
    );

gceSTATUS
sloCOMPILER_SetLineNumber(
    IN sloCOMPILER          compiler,
    IN gctINT               Line
    );

#include "gc_glsl_macro_expand.h"

#endif /* __gc_glsl_preprocessor_int_h_ */
