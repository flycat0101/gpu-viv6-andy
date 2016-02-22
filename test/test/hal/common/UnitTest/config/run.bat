@REM #########################################################################
@REM #
@REM #  Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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


if "%1"=="" goto :usage
set CONFIG=%1

galRunTest2.exe gal2DAlphablendFilterBlit001.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit002.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit003.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit004.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit005.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit006.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit011.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit012.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit013.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit014.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit015.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit016.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit017.dll -c %CONFIG%
galRunTest2.exe gal2DAlphablendFilterBlit018.dll -c %CONFIG%
galRunTest2.exe gal2DAlphaBlending001.dll -c %CONFIG%
galRunTest2.exe gal2DAlphaBlending002.dll -c %CONFIG%
galRunTest2.exe gal2DAlphaBlending003.dll -c %CONFIG%
galRunTest2.exe gal2DAlphaBlending004.dll -c %CONFIG%
galRunTest2.exe gal2DAlphaBlending005.dll -c %CONFIG%
galRunTest2.exe gal2DAlphaBlending006.dll -c %CONFIG%
galRunTest2.exe gal2DBitBlit001.dll -c %CONFIG%
galRunTest2.exe gal2DBlendFactors.dll -c %CONFIG%
galRunTest2.exe gal2DClear001.dll -c %CONFIG%
galRunTest2.exe gal2DClear002.dll -c %CONFIG%
galRunTest2.exe gal2DColorBrush001.dll -c %CONFIG%
galRunTest2.exe gal2DColorBrush002.dll -c %CONFIG%
galRunTest2.exe gal2DColorBrush003.dll -c %CONFIG%
galRunTest2.exe gal2DColorBrush004.dll -c %CONFIG%
galRunTest2.exe gal2DColorKey.dll -c %CONFIG%
galRunTest2.exe gal2DColorSource001.dll -c %CONFIG%
galRunTest2.exe gal2DColorSource002.dll -c %CONFIG%
galRunTest2.exe gal2DColorSource003.dll -c %CONFIG%
galRunTest2.exe gal2DColorSource004.dll -c %CONFIG%
galRunTest2.exe gal2DColorSource005.dll -c %CONFIG%
galRunTest2.exe gal2DColorSource006.dll -c %CONFIG%
galRunTest2.exe gal2DColorSource007.dll -c %CONFIG%
galRunTest2.exe gal2DColorSource008.dll -c %CONFIG%
galRunTest2.exe gal2DColorSource009.dll -c %CONFIG%
galRunTest2.exe gal2DDestColorKeyDFBMode.dll -c %CONFIG%
galRunTest2.exe gal2DDestination001.dll -c %CONFIG%
galRunTest2.exe gal2DDestination002.dll -c %CONFIG%
galRunTest2.exe gal2DDestination003.dll -c %CONFIG%
galRunTest2.exe gal2DDestination004.dll -c %CONFIG%
galRunTest2.exe gal2DDestination005.dll -c %CONFIG%
galRunTest2.exe gal2DDestination006.dll -c %CONFIG%
galRunTest2.exe gal2DDither001.dll -c %CONFIG%
galRunTest2.exe gal2DDither002.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit001.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit002.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit003.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit004.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit005.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit006.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit007.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit008.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit009.dll -c %CONFIG%
galRunTest2.exe gal2DFilterBlit010.dll -c %CONFIG%
galRunTest2.exe gal2DFilterRotateDither.dll -c %CONFIG%
galRunTest2.exe gal2DFormatA8_001.dll -c %CONFIG%
galRunTest2.exe gal2DFormatA8_002.dll -c %CONFIG%
galRunTest2.exe gal2DFormatARGB001.dll -c %CONFIG%
galRunTest2.exe gal2DFormatARGB002.dll -c %CONFIG%
galRunTest2.exe gal2DFormatTiling001.dll -c %CONFIG%
galRunTest2.exe gal2DFormatTiling002.dll -c %CONFIG%
galRunTest2.exe gal2DFormatTiling003.dll -c %CONFIG%
galRunTest2.exe gal2DFormatYUV001.dll -c %CONFIG%
galRunTest2.exe gal2DFormatYUV002.dll -c %CONFIG%
galRunTest2.exe gal2DFormatYUV003.dll -c %CONFIG%
galRunTest2.exe gal2DLine001.dll -c %CONFIG%
galRunTest2.exe gal2DLine002.dll -c %CONFIG%
galRunTest2.exe gal2DLine003.dll -c %CONFIG%
galRunTest2.exe gal2DLine004.dll -c %CONFIG%
galRunTest2.exe gal2DLine005.dll -c %CONFIG%
galRunTest2.exe gal2DLine006.dll -c %CONFIG%
galRunTest2.exe gal2DLine007.dll -c %CONFIG%
galRunTest2.exe gal2DLine008.dll -c %CONFIG%
galRunTest2.exe gal2DLine009.dll -c %CONFIG%
galRunTest2.exe gal2DLine010.dll -c %CONFIG%
galRunTest2.exe gal2DLine011.dll -c %CONFIG%
galRunTest2.exe gal2DMaskedSource.dll -c %CONFIG%
galRunTest2.exe gal2DMaskedSource001.dll -c %CONFIG%
galRunTest2.exe gal2DMaskedString.dll -c %CONFIG%
galRunTest2.exe gal2DMonoBlit001.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeBrush001.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeBrush002.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeBrush003.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeBrush004.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeBrush005.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeSource001.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeSource002.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeSource003.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeSource004.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeSource005.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeSource006.dll -c %CONFIG%
galRunTest2.exe gal2DMonochromeSource007.dll -c %CONFIG%
galRunTest2.exe gal2DMonoSource.dll -c %CONFIG%
galRunTest2.exe gal2DMultiply001.dll -c %CONFIG%
galRunTest2.exe gal2DMultiSourceBlit001.dll -c %CONFIG%
galRunTest2.exe gal2DMultiSourceBlit002.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance001.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance002.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance003.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance004.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance005.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance006.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance007.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance008.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance009.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance010.dll -c %CONFIG%
galRunTest2.exe gal2DPerformance011.dll -c %CONFIG%
galRunTest2.exe gal2DRotation001.dll -c %CONFIG%
galRunTest2.exe gal2DRotation002.dll -c %CONFIG%
galRunTest2.exe gal2DRotation003.dll -c %CONFIG%
galRunTest2.exe gal2DRotation004.dll -c %CONFIG%
galRunTest2.exe gal2DRotation005.dll -c %CONFIG%
galRunTest2.exe gal2DRotation006.dll -c %CONFIG%
galRunTest2.exe gal2DRotation007.dll -c %CONFIG%
galRunTest2.exe gal2DRotation008.dll -c %CONFIG%
galRunTest2.exe gal2DRotation009.dll -c %CONFIG%
galRunTest2.exe gal2DRotation010.dll -c %CONFIG%
galRunTest2.exe gal2DRotation011.dll -c %CONFIG%
galRunTest2.exe gal2DRotation012.dll -c %CONFIG%
galRunTest2.exe gal2DRotation013.dll -c %CONFIG%
galRunTest2.exe gal2DScaleBlit.dll -c %CONFIG%
galRunTest2.exe gal2DSingleColorBrush001.dll -c %CONFIG%
galRunTest2.exe gal2DSingleColorBrush002.dll -c %CONFIG%
galRunTest2.exe gal2DSingleColorBrush003.dll -c %CONFIG%
galRunTest2.exe gal2DSourceColorKeyDFBMode.dll -c %CONFIG%
galRunTest2.exe gal2DStress.dll -c %CONFIG%
galRunTest2.exe gal2DStretchBlit001.dll -c %CONFIG%
galRunTest2.exe gal2DStretchBlit002.dll -c %CONFIG%
galRunTest2.exe gal2DStretchBlit003.dll -c %CONFIG%
galRunTest2.exe gal2DStretchBlit004.dll -c %CONFIG%
galRunTest2.exe gal2DStretchBlit005.dll -c %CONFIG%
galRunTest2.exe gal2DStretchBlit006.dll -c %CONFIG%
galRunTest2.exe gal2DStretchBlit007.dll -c %CONFIG%
galRunTest2.exe gal2DString.dll -c %CONFIG%
galRunTest2.exe gal2DYUV2RGB.dll -c %CONFIG%

:usage
echo usage: run.bat CONFIG
echo .


:end