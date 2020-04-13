@REM #########################################################################
@REM #
@REM #  Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
@REM #
@REM #  The material in this file is confidential and contains trade secrets
@REM #  of Vivante Corporation. This is proprietary information owned by
@REM #  Vivante Corporation. No part of this work may be disclosed,
@REM #  reproduced, copied, transmitted, or used in any way for any purpose,
@REM #  without the express written permission of Vivante Corporation.
@REM #
@REM #########################################################################


@if "%_echo%"=="" echo off

if "%_TARGETPLATROOT%"=="" (
    echo.
	echo ERROR: not found %%_TARGETPLATROOT%%.
	echo Please execute this tool in CESHELL command window.
	echo.
	goto :eof
)

if "%VIVANTE_SDK_DIR%"=="" (
    echo.
    echo Warning: not found %%VIVANTE_SDK_DIR%%, use default setting:
	echo set VIVANTE_SDK_DIR=%%AQROOT%%\VIVANTE_SDK
	echo.
    set VIVANTE_SDK_DIR=%AQROOT%\VIVANTE_SDK
)

set VIVANTE_SDK_INC=%VIVANTE_SDK_DIR%\inc
set VIVANTE_SDK_LIB=%VIVANTE_SDK_DIR%\lib
set VIVANTE_SDK_BIN=%VIVANTE_SDK_DIR%\bin

if not exist %VIVANTE_SDK_INC% mkdir %VIVANTE_SDK_INC%
if not exist %VIVANTE_SDK_LIB% mkdir %VIVANTE_SDK_LIB%
if not exist %VIVANTE_SDK_BIN% mkdir %VIVANTE_SDK_BIN%

echo.
echo Create Vivante Driver SDK ...
echo.

if not exist %VIVANTE_SDK_INC%\HAL mkdir %VIVANTE_SDK_INC%\HAL
echo copy gc_hal.h ...
copy /y %AQROOT%\hal\inc\gc_hal.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_tpes.h ...
copy /y %AQROOT%\hal\inc\shared\gc_hal_types.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_enum.h ...
copy /y %AQROOT%\hal\inc\gc_hal_enum.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_options.h ...
copy /y %AQROOT%\hal\inc\gc_hal_options.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_base.h ...
copy /y %AQROOT%\hal\inc\gc_hal_base.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_raster.h ...
copy /y %AQROOT%\hal\inc\gc_hal_raster.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_engine.h ...
copy /y %AQROOT%\hal\inc\gc_hal_engine.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_dump.h ...
copy /y %AQROOT%\hal\inc\gc_hal_dump.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_statistics.h ...
copy /y %AQROOT%\hal\inc\gc_hal_statistics.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_mem.h ...
copy /y %AQROOT%\hal\inc\gc_hal_mem.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_driver.h ...
copy /y %AQROOT%\hal\inc\shared\gc_hal_driver.h %VIVANTE_SDK_DIR%\inc\HAL\
if exist %AQROOT%\hal\inc\gc_hal_cl.h (
    echo copy gc_hal_cl.h ...
    copy /y %AQROOT%\hal\inc\gc_hal_cl.h %VIVANTE_SDK_DIR%\inc\HAL\
)
echo copy gc_hal_profiler.h ...
copy /y %AQROOT%\hal\inc\gc_hal_profiler.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_driver_vg.h ...
copy /y %AQROOT%\hal\inc\shared\gc_hal_driver_vg.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_engine_vg.h ...
copy /y %AQROOT%\hal\inc\gc_hal_engine_vg.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_vg.h ...
copy /y %AQROOT%\hal\inc\gc_hal_vg.h %VIVANTE_SDK_DIR%\inc\HAL\
echo copy gc_hal_version.h ...
copy /y %AQROOT%\hal\inc\gc_hal_version.h %VIVANTE_SDK_DIR%\inc\HAL\

echo copy KHR header files ...
if not exist %VIVANTE_SDK_INC%\KHR mkdir %VIVANTE_SDK_INC%\KHR
copy /y %AQROOT%\sdk\inc\KHR\*.h %VIVANTE_SDK_DIR%\inc\KHR\

echo copy EGL header files ...
if not exist %VIVANTE_SDK_INC%\EGL mkdir %VIVANTE_SDK_INC%\EGL
copy /y %AQROOT%\sdk\inc\EGL\*.h %VIVANTE_SDK_DIR%\inc\EGL\

echo copy GLES header files ...
if not exist %VIVANTE_SDK_INC%\GLES mkdir %VIVANTE_SDK_INC%\GLES
copy /y %AQROOT%\sdk\inc\GLES\*.h %VIVANTE_SDK_DIR%\inc\GLES\

echo copy GLES2 header files ...
if not exist %VIVANTE_SDK_INC%\GLES2 mkdir %VIVANTE_SDK_INC%\GLES2
copy /y %AQROOT%\sdk\inc\GLES2\*.h %VIVANTE_SDK_DIR%\inc\GLES2\

echo copy GLES2 header files ...
if not exist %VIVANTE_SDK_INC%\GLES3 mkdir %VIVANTE_SDK_INC%\GLES3
copy /y %AQROOT%\sdk\inc\GLES3\*.h %VIVANTE_SDK_DIR%\inc\GLES3\

if exist %AQROOT%\sdk\inc\CL (
    echo copy CL header files ...
    if not exist %VIVANTE_SDK_INC%\CL mkdir %VIVANTE_SDK_INC%\CL
    copy /y %AQROOT%\sdk\inc\CL\*.h %VIVANTE_SDK_DIR%\inc\CL\
    copy /y %AQROOT%\sdk\inc\CL\*.hpp %VIVANTE_SDK_DIR%\inc\CL\
    copy /y %AQROOT%\sdk\inc\CL\*.txt %VIVANTE_SDK_DIR%\inc\CL\
)

echo copy VG header files ...
if not exist %VIVANTE_SDK_INC%\VG mkdir %VIVANTE_SDK_INC%\VG
copy /y %AQROOT%\sdk\inc\VG\*.h %VIVANTE_SDK_DIR%\inc\VG\

echo copy VDK header files ...
copy /y %AQROOT%\sdk\inc\gc_vdk_types.h %VIVANTE_SDK_DIR%\inc\gc_vdk_types.h
copy /y %AQROOT%\sdk\inc\gc_vdk.h %VIVANTE_SDK_DIR%\inc\gc_vdk.h

set CELIBPATH=%_TARGETPLATROOT%\lib\%_TGTCPU%\%WINCEDEBUG%
if "%_WINCEOSVER%" GEQ "700" set CELIBPATH=%SG_OUTPUT_ROOT%\platform\%_TGTPLAT%\lib\%_TGTCPU%\%WINCEDEBUG%
echo.
echo Entering %CELIBPATH%\ ...
echo.
cd /d %CELIBPATH%\
echo copy libGALCore.lib ...
copy /y libGALCore.lib %VIVANTE_SDK_LIB%\

echo copy libGAL.lib ...
copy /y libGAL.lib %VIVANTE_SDK_LIB%\

echo copy k_libGAL.lib ...
copy /y k_libGAL.lib %VIVANTE_SDK_LIB%\

if exist %CELIBPATH%\GC2D_DDGPE_Accelerator.lib (
echo.
echo copy GC2D_DDGPE_Accelerator.lib
copy /y GC2D_DDGPE_Accelerator.lib %VIVANTE_SDK_LIB%\
echo.
)

if exist %CELIBPATH%\GC2D_GPE_Accelerator.lib (
echo.
echo copy GC2D_GPE_Accelerator.lib
copy /y GC2D_GPE_Accelerator.lib %VIVANTE_SDK_LIB%\
echo.
)

echo copy libEGL.lib ...
copy /y libEGL.lib %VIVANTE_SDK_LIB%\

echo copy libGLESv1_CM.lib ...
copy /y libGLESv1_CM.lib %VIVANTE_SDK_LIB%\

echo copy libGLESv1_CL.lib ...
copy /y libGLESv1_CL.lib %VIVANTE_SDK_LIB%\

echo copy libGLES_CM.lib ...
copy /y libGLES_CM.lib %VIVANTE_SDK_LIB%\

echo copy libGLES_CL.lib ...
copy /y libGLES_CL.lib %VIVANTE_SDK_LIB%\

echo copy libGLSLC.lib ...
copy /y libGLSLC.lib %VIVANTE_SDK_LIB%\

echo copy libVSC.lib ...
copy /y libVSC.lib %VIVANTE_SDK_LIB%\

echo copy libGLESv2.lib ...
copy /y libGLESv2.lib %VIVANTE_SDK_LIB%\

echo copy libOpenVG.lib ...
copy /y libOpenVG.lib %VIVANTE_SDK_LIB%\

if exist %CELIBPATH%\libCLC.lib (
    echo copy libCLC.lib ...
    copy /y libCLC.lib %VIVANTE_SDK_LIB%\
)

if exist %CELIBPATH%\libLLVM_viv.lib (
    echo copy libCLC.lib ...
    copy /y libLLVM_viv.lib %VIVANTE_SDK_LIB%\
)

if exist %CELIBPATH%\libOpenCL.lib (
    echo copy libOpenCL.lib ...
    copy /y libOpenCL.lib %VIVANTE_SDK_LIB%\
)

if exist %CELIBPATH%\libVivanteOpenCL.lib (
    echo copy libVivanteOpenCL.lib ...
    copy /y libVivanteOpenCL.lib %VIVANTE_SDK_LIB%\
)

if exist %CELIBPATH%\libVDK.lib (
	echo copy libVDK.lib ...
	copy /y libVDK.lib %VIVANTE_SDK_LIB%\
)

set CETGTPATH=%_TARGETPLATROOT%\target\%_TGTCPU%\%WINCEDEBUG%
if "%_WINCEOSVER%" GEQ "700" set CETGTPATH=%SG_OUTPUT_ROOT%\platform\%_TGTPLAT%\target\%_TGTCPU%\%WINCEDEBUG%
echo.
echo Entering %CETGTPATH%\ ...
echo.
cd /d %CETGTPATH%\

echo copy libGALCore.dll ...
copy /y libGALCore.dll %VIVANTE_SDK_BIN%\

echo copy libGAL.dll ...
copy /y libGAL.dll %VIVANTE_SDK_BIN%\

echo copy k_libGAL.dll ...
copy /y k_libGAL.dll %VIVANTE_SDK_BIN%\

echo copy libEGL.dll ...
copy /y libEGL.dll %VIVANTE_SDK_BIN%\

echo copy libGLESv1_CM.dll ...
copy /y libGLESv1_CM.dll %VIVANTE_SDK_BIN%\

echo copy libGLESv1_CL.dll ...
copy /y libGLESv1_CL.dll %VIVANTE_SDK_BIN%\

echo copy libGLES_CM.dll ...
copy /y libGLES_CM.dll %VIVANTE_SDK_BIN%\

echo copy libGLES_CL.dll ...
copy /y libGLES_CL.dll %VIVANTE_SDK_BIN%\

echo copy libGLSLC.dll ...
copy /y libGLSLC.dll %VIVANTE_SDK_BIN%\

echo copy libVSC.dll ...
copy /y libVSC.dll %VIVANTE_SDK_BIN%\

echo copy libGLESv2.dll ...
copy /y libGLESv2.dll %VIVANTE_SDK_BIN%\

echo copy libOpenVG.dll ...
copy /y libOpenVG.dll %VIVANTE_SDK_BIN%\

if exist %CETGTPATH%\libCLC.dll (
    echo copy libCLC.dll ...
    copy /y libCLC.dll %VIVANTE_SDK_BIN%\
)

if exist %CETGTPATH%\libLLVM_viv.dll (
    echo copy libCLC.dll ...
    copy /y libLLVM_viv.dll %VIVANTE_SDK_BIN%\
)

if exist %CETGTPATH%\libOpenCL.dll (
    echo copy libOpenCL.dll ...
    copy /y libOpenCL.dll %VIVANTE_SDK_BIN%\
)

if exist %CETGTPATH%\libVivanteOpenCL.dll (
    echo copy libVivanteOpenCL.dll ...
    copy /y libVivanteOpenCL.dll %VIVANTE_SDK_BIN%\
)
if exist %CETGTPATH%\libVDK.dll (
	echo copy libVDK.dll ...
	copy /y libVDK.dll %VIVANTE_SDK_BIN%\
)

echo.
echo  Vivante Driver SDK file list:
echo.
cd /d %VIVANTE_SDK_DIR%
tree /A /F

:eof
