/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_cl_builtin_lib_h_
#define __gc_vsc_cl_builtin_lib_h_

#undef _CL_LOCALLY_SET
#ifndef __BUILTIN_SHADER_LENGTH__
#define __BUILTIN_SHADER_LENGTH__ (65535 * 8)
#define _CL_LOCALLY_SET
#endif

extern gctSTRING gcCLLibHeader ;

extern gctSTRING gcCLLibFunc_Extension;

#if CompileIntrinsicLibfromSrc
extern gctSTRING gcCLLibRelational_Funcs_packed;
#else
extern gctSTRING gcCLLibFMA_Func_fmaSupported;

extern gctSTRING gcCLLibASIN_ACOS_Funcs_Common;

extern gctSTRING gcCLLibASIN_Funcs;

extern gctSTRING gcCLLibASIN_Funcs_halti2;

extern gctSTRING gcCLLibASIN_Funcs_halti5;

extern gctSTRING gcCLLibASIN_Funcs_halti5_fmaSupported;

extern gctSTRING gcCLLibACOS_Funcs;

extern gctSTRING gcCLLibACOS_Funcs_halti2;

extern gctSTRING gcCLLibACOS_Funcs_halti5;

extern gctSTRING gcCLLibACOS_Funcs_halti5_fmaSupported;

extern gctSTRING gcCLLibATAN_Funcs;

extern gctSTRING gcCLLibATAN_Funcs_halti2;

extern gctSTRING gcCLLibATAN_Funcs_halti5;

extern gctSTRING gcCLLibATAN_Funcs_halti5_fmaSupported;

extern gctSTRING gcCLLibATAN2_Funcs;

extern gctSTRING gcCLLibATAN2_Funcs_halti2;

extern gctSTRING gcCLLibATAN2_Funcs_halti5;
extern gctSTRING gcCLLibATAN2_Funcs_halti5_fmaSupported;

extern gctSTRING gcCLLibLongMADSAT_Funcs;

extern gctSTRING gcCLLibLongNEXTAFTER_Funcs;
#endif

/*****************************
   image descriptor:  uint8
   s0: base address of image data
   s1: stride of a row of the image
   s2: image size
       [15:0]: width (U16)
       [31:16]: height (U16)
   s3: image properties
       [2:0]: shift (for calculation of bpp)
       [3:3]: multiply (for calculation of bpp)
          0: ONE
          1: THREE
       [5:4]: addressing mode
          0: NONE   // Currently aliases to BORDER0.
          1: BORDER0    (0,0,0,0)
          2: BORDER1    (0,0,0,1) or (0,0,0,1.0), depending on image format.
          3: CLAMP      clamp to edge color  // v60 +
       [9:6]: image format
       [11:10]: tiling mode
          0: LINEAR
          1: TILED
          2: SUPER_TILED
       [12:12]: image type
       [15:14]: component count (packed formats are treated as 1 component)
                0: 4 components
                1: 1 component
                2: 2 components
                3: 3 components
       [18:16]: swizzle_r
       [22:20]: swizzle_g
       [26:24]: swizzle_b
       [30:28]: swizzle_a
   s4: size of an 2-D image in a 2-D image array or slice size of 2-D layer in a 3-D image
   s5: depth of 3-D image or array size of 1-D/2-D array.
   s6: image format
       [0:15]: channel order
       [16:31]: channel data type
****************************/

extern gctSTRING gcCLLibImageQuery_Funcs_UseImgInst;

/* Description of the image header:
    struct _cl_image_header
    {
        int                  width;
        int                  height;
        int                  depth;
        int                  channelDataType;
        int                  channelOrder;
        int                  samplerValue;
        int                  rowPitch;
        int                  slicePitch;
        int                  textureNum;
        int*                 physical;
     };
*/

extern gctSTRING gcCLLibImageQuery_Funcs;

#ifdef _CL_LOCALLY_SET
#undef __BUILTIN_SHADER_LENGTH__
#endif
#endif /* __gc_vsc_cl_builtin_lib_h_ */

