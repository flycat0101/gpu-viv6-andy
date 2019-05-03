/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <galUtil.h>
#include "gutsystem.h"

/* default log path */
char        g_logPath[MAX_BUFFER_SIZE + 1]     = "result"; /* The path of the log file. */
char        g_bmpPath[MAX_BUFFER_SIZE + 1]     = "result"; /* The path of the dumpped bmp files. */
char        g_resultPath[MAX_BUFFER_SIZE + 1]  = "result"; /* The path of the dumpped data file. */
char        g_errorPath[MAX_BUFFER_SIZE + 1]   = "result"; /* The path of the error files. */
char        g_perfPath[MAX_BUFFER_SIZE + 1]   = "result"; /* The path of the error files. */

#if defined(_WIN32)

void sysOutput(const char *format, ...)
{
#ifndef UNDER_CE
    char    *buffer;
    int     len;
#endif

    va_list args;

    va_start(args, format);

    vprintf(format, args);
#ifndef UNDER_CE
    len = _vscprintf(format, args) + 1 + 1;
    buffer = (char *)malloc(len * sizeof(char));
    vsprintf(buffer, format, args);

    OutputDebugStringA(buffer);

    free(buffer);
#endif
    va_end(args);
}

gctHANDLE sysLoadModule(const gctSTRING modName)
{
#ifndef UNDER_CE
    return LoadLibraryA(modName);
#else
    TCHAR    temp[MAX_BUFFER_SIZE + 1];

    mbstowcs(temp, modName, MAX_BUFFER_SIZE + 1);
    return LoadLibrary(temp);

#endif
}

gctBOOL sysUnloadModule(const gctHANDLE mod)
{
    return FreeLibrary(mod);
}

gctPOINTER sysGetProcAddress(const gctHANDLE mod, gctSTRING ProName)
{
#ifndef UNDER_CE
    return GetProcAddress(mod, ProName);
#else
    TCHAR    temp[MAX_BUFFER_SIZE + 1];

    mbstowcs(temp, ProName, MAX_BUFFER_SIZE + 1);
    return GetProcAddress(mod, temp);

#endif
}

gctUINT32 sysGetModuleName(const gctHANDLE mod, gctSTRING str, gctUINT32 size)
{
#ifndef UNDER_CE
    return GetModuleFileNameA(mod, str, size);
#else
    TCHAR    temp[MAX_BUFFER_SIZE + 1];
    gctUINT len;

    if (size > MAX_BUFFER_SIZE + 1)
        return 0;

    len = GetModuleFileName(mod, temp, size);
    wcstombs(str, temp, len);

    return len;
#endif
}

#ifdef UNDER_CE
DLL_API char CurrentModulePath[MAX_BUFFER_SIZE];
#endif
gctBOOL sysSetupLog(const gctSTRING CaseName)
{
    char path[MAX_BUFFER_SIZE + 1];
    char * pos;

    memset(path, 0, sizeof(char) * (MAX_BUFFER_SIZE + 1));
    // Get the module path.
    if (g_caseDll)
        sysGetModuleName(g_caseDll, path, MAX_BUFFER_SIZE);
    else
        return gcvFALSE;

    // Find the last path seperator.
    pos = strrchr(path, '\\');
    *(pos + 1) = '\0';

    // Concatenate the file name.
    strcpy(g_errorPath, path);
    strcpy(g_resultPath, path);
    strcpy(g_logPath, path);
    strcpy(g_perfPath, path);
    strcpy(g_bmpPath, path);
#ifdef UNDER_CE
    strcpy(CurrentModulePath, path);
#endif

    if (strcmp(g_errorPath, "") != 0) strcat(g_errorPath, "\\");
    strcat(g_errorPath, CaseName);
    strcat(g_errorPath, ".err");
    GalInitializeOutput(GalOutputType_Error, g_errorPath);

    if (strcmp(g_resultPath, "") != 0) strcat(g_resultPath, "\\");
    strcat(g_resultPath, CaseName);
    strcat(g_resultPath, ".rlt");
    GalInitializeOutput(GalOutputType_Result, g_resultPath);

    if (strcmp(g_logPath, "") != 0) strcat(g_logPath, "\\");
    strcat(g_logPath, CaseName);
    strcat(g_logPath, ".log");
    GalInitializeOutput(GalOutputType_Log, g_logPath);

    if (strcmp(g_perfPath, "") != 0) strcat(g_perfPath, "\\");
    strcat(g_perfPath, CaseName);
    strcat(g_perfPath, ".perf");
    GalInitializeOutput(GalOutputType_Perf, g_perfPath);

    if (strcmp(g_bmpPath, "") != 0) strcat(g_bmpPath, "\\");

    return gcvTRUE;
}
#if defined(UNDER_CE) && (UNDER_CE < 600)
#define MAX_NUM_OF_PARAMETERS    20
int main(int argc, char *argv[]);
int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
    char *mbargv[MAX_NUM_OF_PARAMETERS];
    int i;
    int result = -1;

    if (argc > MAX_NUM_OF_PARAMETERS)
    {
        sysOutput("The parameters are too much!!!");
        return result;
    }

    // convert the string to mb
    for (i = 0; i < argc; i++)
    {
        int len = wcslen(argv[i]);

        mbargv[i] = (char*)malloc(len + 1);
        if (!mbargv[i])
        {
            sysOutput("Out of memory!!!");
            result = -2;
            goto EXIT;
        }

        wcstombs(mbargv[i], argv[i], len);
        mbargv[i][len] = '\0';
    }

    result = main(argc, mbargv);

EXIT:
    // free the mb string
    for (i = 0; i < argc; i++)
    {
        if (mbargv[i])
        {
            free(mbargv[i]);
        }
    }

    return result;
}
#endif

#elif defined(LINUX) || defined(__QNXNTO__)
#include <stdarg.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>

void sysOutput(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

gctHANDLE sysLoadModule(const gctSTRING modName)
{
    return dlopen(modName, RTLD_LAZY);
}

gctBOOL sysUnloadModule(const gctHANDLE mod)
{
    return dlclose(mod);
}

gctPOINTER sysGetProcAddress(const gctHANDLE mod, gctSTRING ProName)
{
    return dlsym(mod, ProName);
}

gctUINT32 sysGetModuleName(const gctHANDLE mod, gctSTRING str, gctUINT32 size)
{
    return 0;
}

gctBOOL sysSetupLog(const gctSTRING CaseName)
{
    /* Setup log file objects. */
    mkdir(g_errorPath, 0755);

    if (strcmp(g_errorPath, "") != 0) strcat(g_errorPath, "/");
    strcat(g_errorPath, CaseName);
    strcat(g_errorPath, ".err");
    GalInitializeOutput(GalOutputType_Error, g_errorPath);

    mkdir(g_resultPath, 0755);

    if (strcmp(g_resultPath, "") != 0) strcat(g_resultPath, "/");
    strcat(g_resultPath, CaseName);
    strcat(g_resultPath, ".rlt");
    GalInitializeOutput(GalOutputType_Result, g_resultPath);

    mkdir(g_logPath, 0755);

    if (strcmp(g_logPath, "") != 0) strcat(g_logPath, "/");
    strcat(g_logPath, CaseName);
    strcat(g_logPath, ".log");
    GalInitializeOutput(GalOutputType_Log, g_logPath);

    mkdir(g_perfPath, 0755);

    if (strcmp(g_perfPath, "") != 0) strcat(g_perfPath, "/");
    strcat(g_perfPath, CaseName);
    strcat(g_perfPath, ".perf");
    GalInitializeOutput(GalOutputType_Perf, g_perfPath);

    mkdir(g_bmpPath, 0755);

    if (strcmp(g_bmpPath, "") != 0) strcat(g_bmpPath, "/");

    return gcvTRUE;
}

#else
#error unsupported platform
#endif
