@REM #########################################################################
@REM #
@REM #  Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
@REM #  All Rights Reserved.
@REM #
@REM #  Permission is hereby granted, free of charge, to any person obtaining
@REM #  a copy of this software and associated documentation files (the
@REM #  'Software'), to deal in the Software without restriction, including
@REM #  without limitation the rights to use, copy, modify, merge, publish,
@REM #  distribute, sub license, and/or sell copies of the Software, and to
@REM #  permit persons to whom the Software is furnished to do so, subject
@REM #  to the following conditions:
@REM #
@REM #  The above copyright notice and this permission notice (including the
@REM #  next paragraph) shall be included in all copies or substantial
@REM #  portions of the Software.
@REM #
@REM #  THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
@REM #  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
@REM #  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
@REM #  IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
@REM #  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
@REM #  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
@REM #  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
@REM #
@REM #########################################################################


@echo off
set ARGV=320_240_A8R8G8B8_VIRTUAL 320_240_A8R8G8B8_DEFAULT 320_240_A1R5G5B5_VIRTUAL 320_240_A4R4G4B4_VIRTUAL 320_240_R5G6B5_SYSTEM 640_480_A8R8G8B8_DEFAULT
set ARGV=%ARGV% 640_480_R8G8B8A8_VIRTUAL 640_480_R5G5B5A1_VIRTUAL 640_480_B8G8R8A8_VIRTUAL 640_480_B5G6R5_VIRTUAL 640_480_A4B4G4R4_VIRTUAL 600_400_B4G4R4A4_DEFAULT

for %%i in (%ARGV%) do (
	echo %%i
	mkdir %%i\bmp %%i\err %%i\log %%i\rlt
	call run.bat %%i.ini
	mv *.bmp %%i\bmp
	mv *.log %%i\log
	mv *.rlt %%i\rlt
	mv *.err %%i\err
)

:end
