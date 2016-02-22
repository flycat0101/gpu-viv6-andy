/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CL/opencl.h>

/******************************************************************************\
|* Messages                                                                   *|
\******************************************************************************/
cl_bool _verbose = CL_FALSE;

#define clmVERBOSE(...) { if (_verbose) printf(__VA_ARGS__); }
#define clmINFO(...)    printf(__VA_ARGS__)
#define clmERROR(...)   fprintf (stderr, __VA_ARGS__)

/******************************************************************************\
|* Error Checking                                                             *|
\******************************************************************************/
#define clmCHECKERROR(a, b) checkError(a, b, __FILE__ , __LINE__)

void
checkError(
    cl_int Value,
    cl_int Reference,
    const char* FileName,
    const int LineNumber
    )
{
    if (Reference != Value)
    {
        clmERROR("\n !!! Error # %i at line %i , in file %s !!!\n\n",
                 Value, LineNumber, FileName);
        exit(EXIT_FAILURE);
    }
}

/******************************************************************************\
|* Argument parsing                                                           *|
\******************************************************************************/
void
parseArgs(
    int argc,
    const char **argv,
    cl_int numPrograms,
    cl_int * testCase
    )
{
    int i;

    *testCase = -1;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'v')
            {
                _verbose = CL_TRUE;
            }
            else
            {
                clmERROR("Invalid option (%s).\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            _verbose = CL_TRUE;
            *testCase = atoi(argv[i]);
            if (*testCase >= numPrograms)
            {
                clmERROR("Invalid testcase (%d).  Valid testcase: [0..%d].\n",
                         *testCase, numPrograms - 1);
                exit(EXIT_FAILURE);
            }
        }
    }
}

/******************************************************************************\
|* Extension checking                                                         *|
\******************************************************************************/
cl_int
checkExtension(
    cl_device_id device,
    const char *extensionName,
    bool *supported
    )
{
    cl_int status = CL_SUCCESS;
    cl_int errNum;
    size_t size = 0;
    char *extString;

    *supported = false;

    if ((errNum = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, NULL, &size)))
    {
        clmERROR("Error: failed to determine size of device extensions string at %s:%d (err = %d)\n", __FILE__, __LINE__, errNum);
        return errNum;
    }

    if (0 == size)
    {
        return CL_SUCCESS;
    }

    extString = (char*) malloc(size);
    if (NULL == extString)
    {
        clmERROR("Error: unable to allocate %ld byte buffer for extension string at %s:%d\n", size, __FILE__, __LINE__);
        return CL_OUT_OF_RESOURCES;
    }

    if ((errNum = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, size, extString, NULL)))
    {
        clmERROR("Error: failed to obtain device extensions string at %s:%d (err = %d)\n", __FILE__, __LINE__, errNum);
    }
    else
    {
        if (strstr(extString, extensionName))
        {
            *supported = true;
        }
        else
        {
            clmINFO("Extension \"%s\" is not supported.\n", extensionName);
        }
    }

    free(extString);
    return errNum;
}
