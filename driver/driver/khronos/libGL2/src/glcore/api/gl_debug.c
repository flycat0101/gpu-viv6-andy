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


#include "gc_gl_debug.h"

#if (defined(_DEBUG) || defined(DEBUG))
#include "gl_constantname.c"

/*counters*/
GLuint  gdbg_drawCount      = 0;
GLuint  gdbg_rasterCount    = 0;
GLuint  gdbg_drawImmeCount  = 0;
GLuint  gdbg_drawArrayCount = 0;
GLuint  gdbg_drawDlistCount = 0;
GLuint  gdbg_frameCount     = 0;
GLuint  gdbg_frameCountTarget   = 0;

GLuint  gdbg_drawTarget     = 0;
GLuint  gdbg_skipTarget     = 0;
GLuint  gdbg_nameTarget     = 0;

GLuint  gdbg_logFrameDrawCount  = GL_FALSE;
GLuint  gdbg_logFrameStartIndex = 0x9f;
GLuint  gdbg_logFrameEndIndex   = 0xb8;
FILE*   gdbg_logFrameFile   = NULL;

/*static variables*/
static GLuint       dbg_functionDepth = 0;
static GLubyte      dbg_sbuffer[1024];

/*log, msg fitlers*/
GLuint dbg_logAPIFilter = 0;
GLuint dbg_bLogAPIToFile= GL_FALSE;

static GLuint dbg_bWGLApi   = 0;
static FILE*  dbg_logFile   = NULL;

/*basic message functions*/
GLvoid dbgPrint(DWORD level, GLbyte *szFormat, ... )
{
    va_list va;
    GLbyte buf[200];

    va_start(va, szFormat);
    wvsprintf(buf, szFormat, va);
    OutputDebugString(buf);
    va_end(va);

}

GLvoid dbgMessage(GLbyte *szFormat, ... )
{
    va_list va;
    GLbyte buf[200];

    OutputDebugString("GL Message:");
    va_start(va, szFormat);
    wvsprintf(buf, szFormat, va);
    OutputDebugString(buf);
    va_end(va);

}

GLvoid dbgError(GLbyte *szFormat, ... )
{
    va_list va;
    GLbyte buf[200];

    OutputDebugString("GL Error:");
    va_start(va, szFormat);
    wvsprintf(buf, szFormat, va);
    OutputDebugString(buf);
    va_end(va);

}

GLvoid dbgLogFileOpen()
{
    static GLint count = 0;
    if(dbg_logFile == NULL)
    {
        GLbyte szFileName[256];
        sprintf(szFileName, "gllog%04x.txt", count);

        dbg_logFile = fopen(szFileName, "wt");
        count++;
    }
}

GLvoid dbgLogFileClose()
{
    if(dbg_logFile)
    {
        fclose(dbg_logFile);
        dbg_logFile = NULL;
    }
}

GLvoid dbgLogFilePrintf(GLbyte *buf)
{
    if(dbg_logFile)
    {
        fprintf(dbg_logFile, buf);
    }
}

GLvoid dbgLogFileFlush()
{
    if(dbg_logFile)
        fflush(dbg_logFile);
}

/*log functions*/
GLvoid dbgLogFullApi(GLbyte *szAPI, ...)
{
    va_list     args;
    GLuint      dataType;
    GLint       iVal;
    GLuint      uVal;
    GLfloat     fVal;
    GLdouble    dVal;
    GLvoid *    pVal;
    GLuint      pos;
    const GLbyte* szName;
    GLuint      first=1;

    /*filter check*/
    if(0 == (dbg_logAPIFilter & (GL_DBBUG_PRINT_API | GL_DBBUG_PRINT_API_PARAM)))
        return;

    pos = 0;
    sprintf(dbg_sbuffer, "%s(", szAPI);
    pos = strlen(dbg_sbuffer);
    va_start(args, szAPI);

    if(dbg_bLogAPIToFile)
        dbgLogFileOpen();

    /*filter check*/
    if(0 == (dbg_logAPIFilter & GL_DBBUG_PRINT_API_PARAM))
    {
        va_end(args);

        if(dbg_bLogAPIToFile)
        {
            fprintf(dbg_logFile, szAPI);
            fprintf(dbg_logFile, "\r\n");
        }
        else
        {
            OutputDebugString(szAPI);
            OutputDebugString("\r\n");
        }
        return;
    }

    do
    {
        dataType = (GLuint)va_arg(args, GLuint);

        if(first)
        {
            first = 0;
        }
        else
        {
            if(DT_GLnull != dataType)
            {
                dbg_sbuffer[pos]=',';
                pos++;
                dbg_sbuffer[pos]=' ';
                pos++;
                dbg_sbuffer[pos]='\0';
            }
        }

        switch(dataType)
        {
        case DT_GLenum:
            uVal    = (GLenum)va_arg(args, GLenum);
            if(GL_FALSE == dbg_bWGLApi)
                szName  = dbg_GetGLenumName(uVal);
            else
                szName  = dbg_WGLGetGLenumName(uVal);

            if(szName)
            {
                sprintf(&dbg_sbuffer[pos], "%s", szName);
            }
            else
            {
                sprintf(&dbg_sbuffer[pos], "0x%04x", uVal);
            }


            break;
        case DT_GLboolean:
            uVal = (GLuint)va_arg(args, GLboolean);
            if(uVal)
            {
                sprintf(&dbg_sbuffer[pos], "GL_TRUE");
            }
            else
            {
                sprintf(&dbg_sbuffer[pos], "GL_FALSE");
            }

            break;
        case DT_GLbitfield:
            uVal = (GLuint)va_arg(args, GLbitfield);
            sprintf(&dbg_sbuffer[pos], "0x%08x", uVal);
            break;
        case DT_GLbyte:
        case DT_GLubyte:
            uVal = (GLuint)va_arg(args, GLbyte);
            sprintf(&dbg_sbuffer[pos], "0x%02x", uVal);
            break;
        case DT_GLshort:
            iVal = (GLint)va_arg(args, GLshort);
            sprintf(&dbg_sbuffer[pos], "%d", iVal);
            break;
        case DT_GLsizei:
            uVal = (GLuint)va_arg(args, GLsizei);
            sprintf(&dbg_sbuffer[pos], "0x%08x", uVal);
            break;
        case DT_GLushort:
            uVal = (GLuint)va_arg(args, GLushort);
            sprintf(&dbg_sbuffer[pos], "0x%x", uVal);
            break;
        case DT_GLfloat:
        case DT_GLclampf:
            fVal = (GLfloat)va_arg(args, GLdouble);
            sprintf(&dbg_sbuffer[pos], "%f", fVal);
            break;
        case DT_GLdouble:
        case DT_GLclampd:
            dVal = (GLdouble)va_arg(args, GLdouble);
            sprintf(&dbg_sbuffer[pos], "%lf", dVal);
            break;
        case DT_GLint:
            iVal = (GLint)va_arg(args, GLint);
            sprintf(&dbg_sbuffer[pos], "%d", iVal);
            break;
        case DT_GLuint:
            uVal = (GLuint)va_arg(args, GLuint);
            sprintf(&dbg_sbuffer[pos], "0x%x", uVal);
            break;
        case DT_GLenum_ptr:
        case DT_GLboolean_ptr:
        case DT_GLbyte_ptr:
        case DT_GLsizei_ptr:
        case DT_GLshort_ptr:
        case DT_GLubyte_ptr:
        case DT_GLushort_ptr:
        case DT_GLfloat_ptr:
        case DT_GLclampf_ptr:
        case DT_GLdouble_ptr:
        case DT_GLclampd_ptr:
        case DT_GLvoid_ptr:
        case DT_GLint_ptr:
        case DT_GLuint_ptr:
            pVal = (GLvoid*)va_arg(args, GLvoid*);
            uVal = (DWORD)(ULONG_PTR)pVal;
            sprintf(&dbg_sbuffer[pos], "0x%08x", uVal);
            break;
        case DT_GLnull:
            dataType = DT_GLnull;
            break;
        default:
            dataType = DT_GLnull;
            break;
        }
        pos = strlen(dbg_sbuffer);
    }while(dataType != DT_GLnull);
    va_end(args);

    /*end bracket*/
    dbg_sbuffer[pos]=')';
    pos++;
    dbg_sbuffer[pos]=';';
    pos++;
    dbg_sbuffer[pos]='\0';

    if(dbg_bLogAPIToFile)
    {
        GLuint  closeLog = 0;

        fprintf(dbg_logFile, dbg_sbuffer);
        fprintf(dbg_logFile, "\r\n");

        if(dbg_logAPIFilter & GL_DBBUG_PRINT_API_FLUSH)
            fflush(dbg_logFile);

        if(closeLog)
            dbgLogFileClose();
    }
    else
    {
        OutputDebugString(dbg_sbuffer);
        OutputDebugString("\r\n");
    }
}

GLvoid dbgLogFullApiBegin()
{
    GLbyte  szTemp[32];
    GLuint   i;
    dbg_functionDepth++;

    for(i = 0; i < dbg_functionDepth; i++)
        szTemp[i] = '\t';
    szTemp[i] = '\0';

    if(dbg_bLogAPIToFile)
        fprintf(dbg_logFile, szTemp);
    else
        OutputDebugString(szTemp);
}

GLvoid dbgLogFullApiEnd()
{
    if(0 == (dbg_logAPIFilter & (GL_DBBUG_PRINT_API | GL_DBBUG_PRINT_API_PARAM)))
        return;

    dbg_functionDepth--;
}

GLvoid dbgLogFullApiRet(GLbyte *szFormat, ... )
{
    va_list va;
    GLbyte buf[200];

    if(0 == (dbg_logAPIFilter & (GL_DBBUG_PRINT_API | GL_DBBUG_PRINT_API_PARAM)))
        return;
    va_start(va, szFormat);
    wvsprintf(buf, szFormat, va);
    va_end(va);

    if(dbg_bLogAPIToFile)
        fprintf(dbg_logFile, buf);
    else
        OutputDebugString(buf);
}

GLvoid dbgWGLLogAPIBegin()
{
    dbg_bWGLApi = GL_TRUE;
}

GLvoid dbgWGLLogAPIEnd()
{
    dbg_bWGLApi = GL_FALSE;
}

GLvoid dbgGCState(__GLcontext* gc)
{

}

/*dump functions*/

const GLbyte * dbgQueryTexFmt(__GLdeviceFormat uFmt)
{
    GL_ASSERT(uFmt<__GL_DEVFMT_MAX);
    return dbg_texInternalFmtConstNames[uFmt].name;
}


#endif