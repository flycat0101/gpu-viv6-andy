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


#ifndef __GUT_SYSTEM_H__
#define __GUT_SYSTEM_H__

/* system related functions */
void sysOutput(const char *format, ...);
const char * sysGetCmd();
gctHANDLE sysLoadModule(const gctSTRING modName);
gctBOOL sysUnloadModule(const gctHANDLE mod);
gctPOINTER sysGetProcAddress(const gctHANDLE mod, gctSTRING ProName);
gctUINT32 sysGetModuleName(const gctHANDLE mod, gctSTRING str, gctUINT32 size);
gctBOOL sysSetupLog(const gctSTRING CaseName);

/* default log path */
extern char g_logPath[MAX_BUFFER_SIZE + 1]; /* The path of the log file. */
extern char g_bmpPath[MAX_BUFFER_SIZE + 1]; /* The path of the dumpped bmp files. */
extern char g_resultPath[MAX_BUFFER_SIZE + 1]; /* The path of the dumpped data file. */
extern char g_errorPath[MAX_BUFFER_SIZE + 1]; /* The path of the error files. */

extern gctHANDLE   g_caseDll;

#define PREFIX "gal"

#if defined(WIN32)
#define SURFFIX ".dll"
#elif defined(LINUX) || defined(__QNXNTO__)
#define SURFFIX ".so"
#else
#error unsupported platform
#endif


#endif /* __GUT_SYSTEM_H__ */
