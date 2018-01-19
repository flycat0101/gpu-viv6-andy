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


#include "gc_cl_preprocessor_int.h"

/*******************************************************************************
**
**    ppoBASE_Dump
**
**        To dump the base class.
**
**
**
**
*/
gceSTATUS    ppoBASE_Dump(
    ppoPREPROCESSOR PP,
    ppoBASE            Base
    )
{
    return cloCOMPILER_Dump(
        PP->compiler,
        clvDUMP_PREPROCESSOR,
        "<BaseClass file=\"%s\" line=\"%d\" infomation=\"%s\" />",
        Base->file,
        Base->line,
        Base->info
        );
}

/*******************************************************************************
**
**    ppoBASE_Init
**
**        Init(Reset), not alloc a base class.
**
*/

gceSTATUS
ppoBASE_Init(
    /*00*/ IN        ppoPREPROCESSOR        PP,
    /*01*/ IN OUT    ppoBASE                YourBase,
    /*02*/ IN        gctCONST_STRING        File,
    /*03*/ IN        gctUINT                Line,
    /*04*/ IN        gctCONST_STRING        MoreInfo,
    /*05*/ IN        ppeOBJECT_TYPE        Type
    )
{
    gcmASSERT(
        /*00*/    PP
        /*01*/    && YourBase
        /*02*/    && File
        /*03*/    && Line
        /*04*/    && MoreInfo
        /*05*/    && Type
        );

    gcoOS_MemFill(YourBase, 0, sizeof(struct _ppoBASE));

    /*00    node*/

    /*01*/    YourBase->type    =    Type;

    /*02*/    YourBase->file    =    File;

    /*03*/    YourBase->line    =    Line;

    /*04*/    YourBase->info    =    MoreInfo;

    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**    ppeOBJECT_TypeString
**
**        Return the name string of a type.
**
*/
gceSTATUS
ppeOBJECT_TypeString(
    IN    ppeOBJECT_TYPE        TypeEnum,
    OUT    gctCONST_STRING*    TypeString
    )
{
    switch (TypeEnum)
    {
    case ppvOBJ_UNKNOWN:
        *TypeString = "Object Type : Unknown";
        break;
    case ppvOBJ_MACRO_MANAGER:
        *TypeString = "Object Type : Macro Manager";
        break;
    case ppvOBJ_TOKEN:
        *TypeString = "Object Type : Token";
        break;
    case ppvOBJ_PREPROCESSOR:
        *TypeString = "Object Type : PP";
        break;
    case ppvOBJ_HIDE_SET:
        *TypeString = "Object Type : Hide Set";
        break;
    case ppvOBJ_BYTE_INPUT_STREAM:
        *TypeString = "Object Type : Byte Input Stream";
        break;
    case ppvOBJ_STRING_MANAGER_IR:
        *TypeString = "Object Type : String Manager";
        break;
    case ppvOBJ_MACRO_SYMBOL:
        *TypeString = "Object Type : Macro Symbol";
        break;
    default:
        {
            *TypeString = gcvNULL;
            return gcvSTATUS_INVALID_DATA;
        }
    }
    return gcvSTATUS_OK;
}





