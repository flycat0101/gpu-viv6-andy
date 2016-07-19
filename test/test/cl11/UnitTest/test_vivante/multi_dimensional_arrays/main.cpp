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


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#include "multidimensionalarraystestrunner.h"
#include "roundingmode.h"

#include "test2dint.h"
#include "test2dint2.h"
#include "test2dint3.h"
#include "test2dint4.h"
#include "test2dint8.h"
#include "test2dint16.h"

#include "test2duint.h"
#include "test2duint2.h"
#include "test2duint3.h"
#include "test2duint4.h"
#include "test2duint8.h"
#include "test2duint16.h"

#include "test2dchar.h"
#include "test2dchar2.h"
#include "test2dchar3.h"
#include "test2dchar4.h"
#include "test2dchar8.h"
#include "test2dchar16.h"

#include "test2duchar.h"
#include "test2duchar2.h"
#include "test2duchar3.h"
#include "test2duchar4.h"
#include "test2duchar8.h"
#include "test2duchar16.h"

#include "test2dshort.h"
#include "test2dshort2.h"
#include "test2dshort3.h"
#include "test2dshort4.h"
#include "test2dshort8.h"
#include "test2dshort16.h"

#include "test2dushort.h"
#include "test2dushort2.h"
#include "test2dushort3.h"
#include "test2dushort4.h"
#include "test2dushort8.h"
#include "test2dushort16.h"

#include "test2dfloat.h"
#include "test2dfloat2.h"
#include "test2dfloat3.h"
#include "test2dfloat4.h"
#include "test2dfloat8.h"
#include "test2dfloat16.h"

#include "test3dint.h"
#include "test3dint2.h"
#include "test3dint3.h"
#include "test3dint4.h"
#include "test3dint8.h"
#include "test3dint16.h"

#include "test3duint.h"
#include "test3duint2.h"
#include "test3duint3.h"
#include "test3duint4.h"
#include "test3duint8.h"
#include "test3duint16.h"

#include "test3dshort.h"
#include "test3dshort2.h"
#include "test3dshort3.h"
#include "test3dshort4.h"
#include "test3dshort8.h"
#include "test3dshort16.h"

#include "test3dushort.h"
#include "test3dushort2.h"
#include "test3dushort3.h"
#include "test3dushort4.h"
#include "test3dushort8.h"
#include "test3dushort16.h"

#include "test3dchar.h"
#include "test3dchar2.h"
#include "test3dchar3.h"
#include "test3dchar4.h"
#include "test3dchar8.h"
#include "test3dchar16.h"

#include "test3duchar.h"
#include "test3duchar2.h"
#include "test3duchar3.h"
#include "test3duchar4.h"
#include "test3duchar8.h"
#include "test3duchar16.h"

#include "test3dfloat.h"
#include "test3dfloat2.h"
#include "test3dfloat3.h"
#include "test3dfloat4.h"
#include "test3dfloat8.h"
#include "test3dfloat16.h"

const size_t sizeX = 4;
const size_t sizeY = 4;
const size_t sizeZ = 4;

int main(int argc, const char *argv[]) {

    if (!initializeCL()) {
        std::cerr << getErrorMessage() << std::endl;
        exit(0);
    }

    addTest(new Test2DInt(sizeX,sizeY));
    addTest(new Test2DInt2(sizeX,sizeY));
    addTest(new Test2DInt3(sizeX,sizeY));
    addTest(new Test2DInt4(sizeX,sizeY));
    addTest(new Test2DInt8(sizeX,sizeY));
    addTest(new Test2DInt16(sizeX,sizeY));

    addTest(new Test2DUInt(sizeX,sizeY));
    addTest(new Test2DUInt2(sizeX,sizeY));
    addTest(new Test2DUInt3(sizeX,sizeY));
    addTest(new Test2DUInt4(sizeX,sizeY));
    addTest(new Test2DUInt8(sizeX,sizeY));
    addTest(new Test2DUInt16(sizeX,sizeY));

    addTest(new Test2DChar(sizeX,sizeY));
    addTest(new Test2DChar2(sizeX,sizeY));
    addTest(new Test2DChar3(sizeX,sizeY));
    addTest(new Test2DChar4(sizeX,sizeY));
    addTest(new Test2DChar8(sizeX,sizeY));
    addTest(new Test2DChar16(sizeX,sizeY));

    addTest(new Test2DUChar(sizeX,sizeY));
    addTest(new Test2DUChar2(sizeX,sizeY));
    addTest(new Test2DUChar3(sizeX,sizeY));
    addTest(new Test2DUChar4(sizeX,sizeY));
    addTest(new Test2DUChar8(sizeX,sizeY));
    addTest(new Test2DUChar16(sizeX,sizeY));

    addTest(new Test2DShort(sizeX,sizeY));
    addTest(new Test2DShort2(sizeX,sizeY));
    addTest(new Test2DShort3(sizeX,sizeY));
    addTest(new Test2DShort4(sizeX,sizeY));
    addTest(new Test2DShort8(sizeX,sizeY));
    addTest(new Test2DShort16(sizeX,sizeY));

    addTest(new Test2DUShort(sizeX,sizeY));
    addTest(new Test2DUShort2(sizeX,sizeY));
    addTest(new Test2DUShort3(sizeX,sizeY));
    addTest(new Test2DUShort4(sizeX,sizeY));
    addTest(new Test2DUShort8(sizeX,sizeY));
    addTest(new Test2DUShort16(sizeX,sizeY));

    addTest(new Test2DFloat(sizeX,sizeY));
    addTest(new Test2DFloat2(sizeX,sizeY));
    addTest(new Test2DFloat3(sizeX,sizeY));
    addTest(new Test2DFloat4(sizeX,sizeY));
    addTest(new Test2DFloat8(sizeX,sizeY));
    addTest(new Test2DFloat16(sizeX,sizeY));

    addTest(new Test3DInt(sizeX,sizeY,sizeZ));
    addTest(new Test3DInt2(sizeX,sizeY,sizeZ));
    addTest(new Test3DInt3(sizeX,sizeY,sizeZ));
    addTest(new Test3DInt4(sizeX,sizeY,sizeZ));
    addTest(new Test3DInt8(sizeX,sizeY,sizeZ));
    addTest(new Test3DInt16(sizeX,sizeY,sizeZ));

    addTest(new Test3DUInt(sizeX,sizeY,sizeZ));
    addTest(new Test3DUInt2(sizeX,sizeY,sizeZ));
    addTest(new Test3DUInt3(sizeX,sizeY,sizeZ));
    addTest(new Test3DUInt4(sizeX,sizeY,sizeZ));
    addTest(new Test3DUInt8(sizeX,sizeY,sizeZ));
    addTest(new Test3DUInt16(sizeX,sizeY,sizeZ));

    addTest(new Test3DShort(sizeX,sizeY,sizeZ));
    addTest(new Test3DShort2(sizeX,sizeY,sizeZ));
    addTest(new Test3DShort3(sizeX,sizeY,sizeZ));
    addTest(new Test3DShort4(sizeX,sizeY,sizeZ));
    addTest(new Test3DShort8(sizeX,sizeY,sizeZ));
    addTest(new Test3DShort16(sizeX,sizeY,sizeZ));

    addTest(new Test3DUShort(sizeX,sizeY,sizeZ));
    addTest(new Test3DUShort2(sizeX,sizeY,sizeZ));
    addTest(new Test3DUShort3(sizeX,sizeY,sizeZ));
    addTest(new Test3DUShort4(sizeX,sizeY,sizeZ));
    addTest(new Test3DUShort8(sizeX,sizeY,sizeZ));
    addTest(new Test3DUShort16(sizeX,sizeY,sizeZ));

    addTest(new Test3DChar(sizeX,sizeY,sizeZ));
    addTest(new Test3DChar2(sizeX,sizeY,sizeZ));
    addTest(new Test3DChar3(sizeX,sizeY,sizeZ));
    addTest(new Test3DChar4(sizeX,sizeY,sizeZ));
    addTest(new Test3DChar8(sizeX,sizeY,sizeZ));
    addTest(new Test3DChar16(sizeX,sizeY,sizeZ));

    addTest(new Test3DUChar(sizeX,sizeY,sizeZ));
    addTest(new Test3DUChar2(sizeX,sizeY,sizeZ));
    addTest(new Test3DUChar3(sizeX,sizeY,sizeZ));
    addTest(new Test3DUChar4(sizeX,sizeY,sizeZ));
    addTest(new Test3DUChar8(sizeX,sizeY,sizeZ));
    addTest(new Test3DUChar16(sizeX,sizeY,sizeZ));

    addTest(new Test3DFloat(sizeX,sizeY,sizeZ));
    addTest(new Test3DFloat2(sizeX,sizeY,sizeZ));
    addTest(new Test3DFloat3(sizeX,sizeY,sizeZ));
    addTest(new Test3DFloat4(sizeX,sizeY,sizeZ));
    addTest(new Test3DFloat8(sizeX,sizeY,sizeZ));
    addTest(new Test3DFloat16(sizeX,sizeY,sizeZ));


    int error = runTests(argc, argv);

    char a;
    scanf(&a);

    cleanTests();

    if (!releaseCL()) {
        std::cerr << getErrorMessage() << std::endl;
        exit(0);
    }

    return 0;
}
