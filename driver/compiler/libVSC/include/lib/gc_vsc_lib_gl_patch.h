/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
** Protocol for the multi-layer input/output conversion.
** Here are some defined protocol for format conversion. For example,
** gcvSURF_A32B32G32R32UI_4_A8R8G8B8 format.
** We always assume:
** 1, Format name indicate the component sequence.  and we always define
**    them from MSB->LSB. That means
**        A32 is MSB, R32 is LSB,
**        A8  is MSB, B8  is LSB.
** 2, For multiple layers, we always assume that mapping
**         layer0..layerer1..layer2..layer3
**    to LSB component to MSB component. That means,
**         layer0 is R32, layer1 is G32,
**         layer2 is B32, layer3 is A32.
** 3, For every layer, its format is A8R8G8B8, so in recompile library
**    function, always A8 is MSB, B8 is LSB. so in shader,
**    sampleResult.w is MSB, sampleResult. z is LSB.
**    because in shader xyzw is mapping to rgba.
**/

#define TEXLDTYPE_NORMAL        0
#define TEXLDTYPE_PROJ          1
#define TEXLDTYPE_GATHER        2
#define TEXLDTYPE_GATHERPCF     3
#define TEXLDTYPE_FETCHMS       4
#define TEXLDTYPE_U             5

extern gctSTRING gcLibTexFormatConvertHalti2_Header;
extern gctSTRING gcLibTexFormatConvertHalti2_TexCvtUnifiedFunc;
extern gctSTRING gcLibTexFormatConvertHalti2_TexCvtFunc;
extern gctSTRING gcLibTexFormatConvertHalti2_TexCvtFunc1;
extern gctSTRING gcLibTexFormatConvertHalti2_TexPcfCvtFunc;
extern gctSTRING gcLibTexFormatConvertHalti2_OutputCvtUnifiedFunc;
extern gctSTRING gcLibTexFormatConvertHalti2_OutputCvtFunc;
extern gctSTRING gcLibTexFormatConvertHalti2_MainFunc;
extern gctSTRING gcLibTexFormatConvertHalti1_Header;
extern gctSTRING gcLibTexFormatConvertHalti1_SinglePipe_Header;
extern gctSTRING gcLibTexFormatConvertHalti1_TexCvtUnifiedFunc;
extern gctSTRING gcLibTexFormatConvertHalti1_TexCvtFunc0;
extern gctSTRING gcLibTexFormatConvertHalti1_TexCvtFunc1;
extern gctSTRING gcLibTexFormatConvertHalti1_TexCvtFunc_SinglePipe;
extern gctSTRING gcLibTexFormatConvertHalti1_TexPcfCvtFunc;
extern gctSTRING gcLibTexFormatConvertHalti1_OutputCvtUnifiedFunc;
extern gctSTRING gcLibTexFormatConvertHalti1_OutputCvtFunc;
extern gctSTRING gcLibTexFormatConvertHalti1_MainFunc;
extern gctSTRING gcLibTexFormatConvertHalti0_Header;
extern gctSTRING gcLibTexFormatConvertHalti0_TexCvtUnifiedFunc;
extern gctSTRING gcLibTexFormatConvertHalti0_TexCvtFunc;
extern gctSTRING gcLibTexFormatConvertHalti0_TexPcfCvtFunc;
extern gctSTRING gcLibTexFormatConvertHalti0_OutputCvtUnifiedFunc;
extern gctSTRING gcLibTexFormatConvertHalti0_OutputCvtFunc;
extern gctSTRING gcLibTexFormatConvertHalti0_MainFunc;
extern gctSTRING gcLibConvertBlend_Func;

