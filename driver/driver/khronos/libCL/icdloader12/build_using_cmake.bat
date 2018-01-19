@REM #########################################################################
@REM #
@REM #  Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
@REM #
@REM #  The material in this file is confidential and contains trade secrets
@REM #  of Vivante Corporation. This is proprietary information owned by
@REM #  Vivante Corporation. No part of this work may be disclosed,
@REM #  reproduced, copied, transmitted, or used in any way for any purpose,
@REM #  without the express written permission of Vivante Corporation.
@REM #
@REM #########################################################################


call "%VS90COMNTOOLS%/vsvars32.bat"

set BUILD_DIR=build
set BIN_DIR=bin

mkdir %BUILD_DIR%
cd %BUILD_DIR%
cmake -G "NMake Makefiles" ../
nmake
cd ..

