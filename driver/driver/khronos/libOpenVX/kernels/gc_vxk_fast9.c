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


#include <gc_vxk_common.h>

#define F9C_STRENGTH 0
#define F9C_NONMAX   1

gcoVX_Index indexs_halfevis[]                = {
        /* index, num, shift0, shift1, mask0, mask1 */
        {    3, 4 * 4, {(vx_uint32)FV4(2*8,3*8,4*8,(2+16)*8), (vx_uint32)FV4((3+16)*8,(4+16)*8,0, 0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {    4, 4 * 4, {(vx_uint32)FV4(8,5*8,(1+16)*8,(5+16)*8), FV4(0,0,0,0), FV4(8,8,8,8), FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {    5, 4 * 4, {(vx_uint32)FV4(0,6*8,16*8,(6+16)*8), FV4(0,6*8,0,0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {    6, 4 * 4, {FV(3 * 8), FV(3 * 8), FV(8), FV(8)       }  }, /* p */

        {    7, 4 * 4, {(vx_uint32)FV4(3*8,4*8,5*8,(3+16)*8), FV4((4+16)*8,(5+16)*8,0, 0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {    8, 4 * 4, {(vx_uint32)FV4(2*8,6*8,(2+16)*8,(6+16)*8), FV4(0,0,0,0), FV4(8,8,8,8), FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {    9, 4 * 4, {(vx_uint32)FV4(8,7*8,(16+1)*8,(7+16)*8), FV4(8,7*8,0,0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   10, 4 * 4, {FV(4 * 8), FV(4 * 8), FV(8), FV(8)       }  }, /* p */

        {   11, 4 * 4, {(vx_uint32)FV4(4*8,5*8,6*8,(4+16)*8), FV4((5+16)*8,(6+16)*8,0, 0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   12, 4 * 4, {(vx_uint32)FV4(3*8,7*8,(3+16)*8,(7+16)*8), FV4(0,0,0,0), FV4(8,8,8,8), FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   13, 4 * 4, {(vx_uint32)FV4(2*8,8*8,(16+2)*8,(8+16)*8), FV4(2*8,8*8,0,0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   14, 4 * 4, {FV(5 * 8), FV(5 * 8), FV(8), FV(8)       }  }, /* p */

        {   15, 4 * 4, {(vx_uint32)FV4(5*8,6*8,7*8,(5+16)*8), FV4((6+16)*8,(7+16)*8,0, 0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   16, 4 * 4, {(vx_uint32)FV4(4*8,8*8,(4+16)*8,(8+16)*8), FV4(0,0,0,0), FV4(8,8,8,8), FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   17, 4 * 4, {(vx_uint32)FV4(3*8,9*8,(16+3)*8,(9+16)*8), FV4(3*8,9*8,0,0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   18, 4 * 4, {FV(6 * 8), FV(6 * 8), FV(8), FV(8)       }  }, /* p */

        {   19, 4 * 4, {(vx_uint32)FV4(6*8,7*8,8*8,(6+16)*8), FV4((7+16)*8,(8+16)*8,0, 0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   20, 4 * 4, {(vx_uint32)FV4(5*8,9*8,(5+16)*8,(9+16)*8), FV4(0,0,0,0), FV4(8,8,8,8), FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   21, 4 * 4, {(vx_uint32)FV4(4*8,10*8,(16+4)*8,(10+16)*8), FV4(4*8,10*8,0,0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   22, 4 * 4, {FV(7 * 8), FV(7 * 8), FV(8), FV(8)       }  }, /* p */

        {   23, 4 * 4, {(vx_uint32)FV4(7*8,8*8,9*8,(7+16)*8), FV4((8+16)*8,(9+16)*8,0, 0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   24, 4 * 4, {(vx_uint32)FV4(6*8,10*8,(6+16)*8,(10+16)*8), FV4(0,0,0,0), FV4(8,8,8,8), FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   25, 4 * 4, {(vx_uint32)FV4(5*8,11*8,(16+5)*8,(11+16)*8), FV4(5*8,11*8,0,0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   26, 4 * 4, {FV(8 * 8), FV(8 * 8), FV(8), FV(8)       }  }, /* p */

        {   27, 4 * 4, {(vx_uint32)FV4(8*8,9*8,10*8,(8+16)*8), FV4((9+16)*8,(10+16)*8,0, 0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   28, 4 * 4, {(vx_uint32)FV4(7*8,11*8,(7+16)*8,(11+16)*8), FV4(0,0,0,0), FV4(8,8,8,8), FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   29, 4 * 4, {(vx_uint32)FV4(6*8,12*8,(16+6)*8,(12+16)*8), FV4(6*8,12*8,0,0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   30, 4 * 4, {FV(9 * 8), FV(9 * 8), FV(8), FV(8)       }  }, /* p */

        {   31, 4 * 4, {(vx_uint32)FV4(9*8,10*8,11*8,(9+16)*8), FV4((10+16)*8,(11+16)*8,0, 0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   32, 4 * 4, {(vx_uint32)FV4(8*8,12*8,(8+16)*8,(12+16)*8), FV4(0,0,0,0), FV4(8,8,8,8), FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   33, 4 * 4, {(vx_uint32)FV4(7*8,13*8,(16+7)*8,(13+16)*8), FV4(7*8,13*8,0,0), FV4(8,8,8,8), FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   34, 4 * 4, {FV(10 * 8), FV(10 * 8), FV(8), FV(8)       }  }, /* p */

        {   35, 4 * 4, {             0, FV2(255), 0, 0    }  }, /* a, b, c, 0 */
        {   36, 4 * 4, {        FV2(1), FV2(1), 0, FV2(1)    }  }, /* constant */
        {   37, 4 * 4, {  FV4(0,1,2,3), FV4(4,5,6,7), FV4(9,9,9,9), FV4(9,9,9,9)   }  }, /*  */
        {   38, 4 * 4, {FV4(8,9,10,11),FV4(12,13,14,15), FV4(9,9,9,9), FV4(9,9,9,9)   }  }, /*  */
        {   39, 4 * 4, {  0x01ff01ff, 0x01ff01ff, 0x01ff01ff, 0x01ff01ff   }  }, /*  */
        /* 40 - 41 sort */
        {   40, 4 * 4, {(vx_uint32)FV4(16,2*16,(8+1)*16,(8+5)*16), FV4(7*16,(8+7)*16,(8+3)*16, 5*16), FV4(16,16,16,16),FV4(16,16,16,16)}  }, /* 1, 2, 3, 4, 5, 6, 7, 8 */
        {   41, 4 * 4, {(vx_uint32)FV4(4*16,3*16,(8+2)*16,(8+6)*16), FV4(6*16,(8+4)*16,8*16, 0), FV4(16,16,16,16),FV4(16,16,16,16)}  }, /* 1, 2, 3, 4, 5, 6, 7, 8 */
    };

gcoVX_Index indexs[]                = {
        /* index, num, shift0, shift1, mask0, mask1 */
        {    3, 4 * 4, {(vx_uint32)FV4(2*8,3*8,4*8,(2+16)*8), (vx_uint32)FV4((3+16)*8,(4+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {    4, 4 * 4, {(vx_uint32)FV4(8,5*8,(1+16)*8,(5+16)*8), (vx_uint32)FV4(0,0,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {    5, 4 * 4, {(vx_uint32)FV4(0,6*8,16*8,(6+16)*8), (vx_uint32)FV4(0,6*8,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {    6, 4 * 4, {           FV(3 * 8), FV(3 * 8), FV(8), FV(8)       }  }, /* p */

        {    7, 4 * 4, {(vx_uint32)FV4(3*8,4*8,5*8,(3+16)*8), (vx_uint32)FV4((4+16)*8,(5+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {    8, 4 * 4, {(vx_uint32)FV4(2*8,6*8,(2+16)*8,(6+16)*8),(vx_uint32)FV4(0,0,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {    9, 4 * 4, {(vx_uint32)FV4(8,7*8,(16+1)*8,(7+16)*8), (vx_uint32)FV4(8,7*8,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   10, 4 * 4, {           FV(4 * 8), FV(4 * 8), FV(8), FV(8)       }  }, /* p */

        {   11, 4 * 4, {(vx_uint32)FV4(4*8,5*8,6*8,(4+16)*8), (vx_uint32)FV4((5+16)*8,(6+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   12, 4 * 4, {(vx_uint32)FV4(3*8,7*8,(3+16)*8,(7+16)*8),(vx_uint32)FV4(0,0,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   13, 4 * 4, {(vx_uint32)FV4(2*8,8*8,(16+2)*8,(8+16)*8),(vx_uint32)FV4(2*8,8*8,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   14, 4 * 4, {           FV(5 * 8), FV(5 * 8), FV(8), FV(8)       }  }, /* p */

        {   15, 4 * 4, {(vx_uint32)FV4(5*8,6*8,7*8,(5+16)*8), (vx_uint32)FV4((6+16)*8,(7+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   16, 4 * 4, {(vx_uint32)FV4(4*8,8*8,(4+16)*8,(8+16)*8),(vx_uint32)FV4(0,0,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   17, 4 * 4, {(vx_uint32)FV4(3*8,9*8,(16+3)*8,(9+16)*8),(vx_uint32)FV4(3*8,9*8,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   18, 4 * 4, {           FV(6 * 8), FV(6 * 8), FV(8), FV(8)       }  }, /* p */

        {   19, 4 * 4, {(vx_uint32)FV4(6*8,7*8,8*8,(6+16)*8), (vx_uint32)FV4((7+16)*8,(8+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   20, 4 * 4, {(vx_uint32)FV4(5*8,9*8,(5+16)*8,(9+16)*8), (vx_uint32)FV4(0,0,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   21, 4 * 4, {(vx_uint32)FV4(4*8,10*8,(16+4)*8,(10+16)*8), (vx_uint32)FV4(4*8,10*8,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   22, 4 * 4, {           FV(7 * 8), FV(7 * 8), FV(8), FV(8)       }  }, /* p */
        {   23, 4 * 4, {(vx_uint32)FV4(7*8,8*8,9*8,(7+16)*8), (vx_uint32)FV4((8+16)*8,(9+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   24, 4 * 4, {(vx_uint32)FV4(6*8,10*8,(6+16)*8,(10+16)*8), (vx_uint32)FV4(0,0,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   25, 4 * 4, {(vx_uint32)FV4(5*8,11*8,(16+5)*8,(11+16)*8), (vx_uint32)FV4(5*8,11*8,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   26, 4 * 4, {           FV(8 * 8), FV(8 * 8), FV(8), FV(8)       }  }, /* p */

        {   27, 4 * 4, {(vx_uint32)FV4(8*8,9*8,10*8,(8+16)*8), (vx_uint32)FV4((9+16)*8,(10+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   28, 4 * 4, {(vx_uint32)FV4(7*8,11*8,(7+16)*8,(11+16)*8), (vx_uint32)FV4(0,0,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   29, 4 * 4, {(vx_uint32)FV4(6*8,12*8,(16+6)*8,(12+16)*8), (vx_uint32)FV4(6*8,12*8,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   30, 4 * 4, {           FV(9 * 8), FV(9 * 8), FV(8), FV(8)       }  }, /* p */

        {   31, 4 * 4, {(vx_uint32)FV4(9*8,10*8,11*8,(9+16)*8), (vx_uint32)FV4((10+16)*8,(11+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   32, 4 * 4, {(vx_uint32)FV4(8*8,12*8,(8+16)*8,(12+16)*8), (vx_uint32)FV4(0,0,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   33, 4 * 4, {(vx_uint32)FV4(7*8,13*8,(16+7)*8,(13+16)*8), (vx_uint32)FV4(7*8,13*8,0,0), (vx_uint32)FV4(8,8,8,8), (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   34, 4 * 4, {           FV(10 * 8), FV(10 * 8), FV(8), FV(8)       }  }, /* p */

        {   35, 4 * 4, {             0, FV2(255), 0, 0    }  }, /* a, b, c, 0 */
        {   36, 4 * 4, {        FV2(1), FV2(1), 0, FV2(1)    }  }, /* constant */
        {   37, 4 * 4, {  FV4(0,1,2,3), FV4(4,5,6,7), FV4(9,9,9,9), FV4(9,9,9,9)   }  }, /*  */
        {   38, 4 * 4, {FV4(8,9,10,11),FV4(12,13,14,15), FV4(9,9,9,9), FV4(9,9,9,9)   }  }, /*  */
        {   39, 4 * 4, {  0x01ff01ff, 0x01ff01ff, 0x01ff01ff, 0x01ff01ff   }  }, /*  */
        /* 40 - 41 sort */
        {   40, 4 * 4, {(vx_uint32)FV4(16,2*16,(8+1)*16,(8+5)*16), (vx_uint32)FV4(7*16,(8+7)*16,(8+3)*16, 5*16), (vx_uint32)FV4(16,16,16,16),(vx_uint32)FV4(16,16,16,16)}  }, /* 1, 2, 3, 4, 5, 6, 7, 8 */
        {   41, 4 * 4, {(vx_uint32)FV4(4*16,3*16,(8+2)*16,(8+6)*16), (vx_uint32)FV4(6*16,(8+4)*16,8*16, 0), (vx_uint32)FV4(16,16,16,16),(vx_uint32)FV4(16,16,16,16)}  }, /* 1, 2, 3, 4, 5, 6, 7, 8 */

        {   42, 4 * 4, {(vx_uint32)FV4(3*8,7*8,11*8, 15*8), (vx_uint32)FV4(0, 0, 0, 0), (vx_uint32)FV4(8*4,8*4,8*4,8*4),(vx_uint32)FV4(0, 0, 0, 0)}  }, /* 4, 5, 6, 7, 8, 9, a, b, c, d, e, f, 0, 1, 2, 3 */
        {   43, 4 * 4, {(vx_uint32)FV4(6*8,10*8,14*8, 18*8), (vx_uint32)FV4(0, 0, 0, 0), (vx_uint32)FV4(8*4,8*4,8*4,8*4),(vx_uint32)FV4(0, 0, 0, 0)}  }, /* 7, 8, 9, a, b, c, d, e, f, 0, 1, 2, 3, 4, 5, 6 */
        {   44, 4 * 4, {(vx_uint32)FV4(8*8,12*8,0, 4*8), (vx_uint32)FV4(0, 0, 0, 0), (vx_uint32)FV4(8*4,8*4,8*4,8*4),(vx_uint32)FV4(0, 0, 0, 0)}  }, /* 7, 8, 9, a, b, c, d, e, f, 0, 1, 2, 3, 4, 5, 6 */
    };

vx_status vxViv_Fast9Corners_Strength(vx_node node, vx_image src, vx_scalar t, vx_scalar do_nonmax, vx_image output)
{
    vx_status status                    = VX_SUCCESS;
    vx_uint32                         i = 0, height = 0;
    vx_float32 toleranceValue;
    vx_bool isDoNonmax;
    gcoVX_Index *indexData = gcvNULL;
    vx_uint32 indexNum = 0;

    gcoVX_Kernel_Context * kernelContext = gcvNULL;

#if gcdVX_OPTIMIZER
    if (node && node->kernelContext)
    {
        kernelContext = (gcoVX_Kernel_Context *) node->kernelContext;
    }
    else
#endif
    {
        if (node->kernelContext == VX_NULL)
        {
            /* Allocate a local copy for old flow. */
            node->kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));
        }
        kernelContext = (gcoVX_Kernel_Context *)node->kernelContext;
        kernelContext->objects_num = 0;
        kernelContext->uniform_num = 0;
    }

    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noIAdd || node->base.context->evisNoInst.noFilter || node->base.context->evisNoInst.noAbsDiff)
    {
        indexNum = sizeof(indexs_halfevis)/sizeof(indexs_halfevis[0]);
        indexData = (gcoVX_Index *)vxAllocate(sizeof(gcoVX_Index) * indexNum);
        vxMemCopy(indexData, indexs_halfevis, sizeof(gcoVX_Index) * indexNum);
    }
    else
    {
        indexNum = sizeof(indexs)/sizeof(indexs[0]);
        indexData = (gcoVX_Index *)vxAllocate(sizeof(gcoVX_Index) * indexNum);
        vxMemCopy(indexData, indexs, sizeof(gcoVX_Index) * indexNum);
    }

    vxReadScalarValue(t, &toleranceValue);
    indexData[32].bin[0] = indexData[32].bin[2] = FV2((vx_int8)toleranceValue);
    vxReadScalarValue(do_nonmax, &isDoNonmax);

    vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    kernelContext->params.step                 = F9C_STRENGTH;
    kernelContext->params.kernel               = gcvVX_KERNEL_FAST_CORNERS;
#if gcdVX_OPTIMIZER
    kernelContext->borders                     = VX_BORDER_UNDEFINED;
#else
    kernelContext->params.borders              = VX_BORDER_UNDEFINED;
#endif
    kernelContext->params.row                  = (vx_int8)toleranceValue;;
    kernelContext->params.xstep                = 8;
    kernelContext->params.ystep                = height;
    kernelContext->params.volume               = height; /* save height to check border */
    kernelContext->uniform_num                 = indexNum;
    kernelContext->params.constant_value       = isDoNonmax;

    for(i = 0; i < kernelContext->uniform_num; i++)
    {
        gcoOS_MemCopy(&kernelContext->uniforms[i].uniform, indexData[i].bin, sizeof(indexData[i].bin));
        kernelContext->uniforms[i].num = indexData[i].num;
        kernelContext->uniforms[i].index = indexData[i].index;
    }

    {
        vx_uint32 bin[16];

        /* DP16x1 */
        bin[0] = 0x55555555;
        bin[1] = 0x55550000;
        bin[2] = 0x76543210;
        bin[3] = 0x76543210;

        bin[4] = 0xaaaaaaaa;
        bin[5] = 0x76543210;
        bin[6] = 0xfedcba98;
        if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noIAdd || node->base.context->evisNoInst.noFilter || node->base.context->evisNoInst.noAbsDiff)
        {
            bin[7] = 0x00006300;
        }
        else
        {
            bin[7] = 0x00006600;
        }

        bin[8] =  0x00020001;
        bin[9] =  0x00080004;
        bin[10] = 0x00200010;
        bin[11] = 0x00800040;

        bin[12] = 0x02000100;
        bin[13] = 0x08000400;
        bin[14] = 0x20001000;
        bin[15] = 0x80004000;

        gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
        kernelContext->uniforms[kernelContext->uniform_num].num         = 16 * 4;
        if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noIAdd || node->base.context->evisNoInst.noFilter || node->base.context->evisNoInst.noAbsDiff)
        {
            kernelContext->uniforms[kernelContext->uniform_num].index       = 42;
        }
        else
        {
            kernelContext->uniforms[kernelContext->uniform_num].index       = 45;
        }

        kernelContext->uniform_num ++;
    }

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

    vxFree(indexData);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

vx_status vxViv_Fast9Corners_NonMax(vx_node node, vx_image src, vx_scalar t, vx_scalar do_nonmax, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;
    vx_float32 toleranceValue;
    vx_bool isDoNonmax;
    gcoVX_Index indexs[] = {
        /* index, num, shift0, shift1, mask0, mask1 */
        {    3, 4 * 4, {       FV4(8,8,8,8), 0, FV4(8,8,8,8), 0       }  }, /* pixel p(x,y) */
        {    4, 4 * 4, {       FV4(0,0,0,0), 0, FV4(8,0,0,0), 0       }  }, /* pixel p(x-1,y) */
        {    5, 4 * 4, {     FV4(2*8,0,0,0), 0, FV4(8,0,0,0), 0       }  }, /* pixel p(x+1,y) */
        {    6, 4 * 4, {              FV(1), FV(1), FV(1), FV(1)       }  }, /* pixel p(x+1,y) */
        {    7, 4 * 4, {     FV4(0,4*8,0,0), 0, FV4(2*8,2*8,0,0), 0       }  }, /* u32->u16 */
    };
    gcoVX_Kernel_Context * kernelContext = gcvNULL;

#if gcdVX_OPTIMIZER
    if (node && node->kernelContext)
    {
        kernelContext = (gcoVX_Kernel_Context *) node->kernelContext;
    }
    else
#endif
    {
        if (node->kernelContext == VX_NULL)
        {
            /* Allocate a local copy for old flow. */
            node->kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));
        }
        kernelContext = (gcoVX_Kernel_Context *)node->kernelContext;
        kernelContext->objects_num = 0;
        kernelContext->uniform_num = 0;
    }

    vxReadScalarValue(t, &toleranceValue);
    vxReadScalarValue(do_nonmax, &isDoNonmax);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    kernelContext->params.step     = F9C_NONMAX;
    kernelContext->params.kernel   = gcvVX_KERNEL_FAST_CORNERS;
#if gcdVX_OPTIMIZER
    kernelContext->borders         = VX_BORDER_UNDEFINED;
#else
    kernelContext->params.borders  = VX_BORDER_UNDEFINED;
#endif
    kernelContext->params.row      = (vx_uint8)toleranceValue;;
    kernelContext->params.col      = isDoNonmax;;
    kernelContext->params.xstep    = 1;
    kernelContext->uniform_num     = sizeof(indexs)/sizeof(indexs[0]);

    for(i = 0; i < kernelContext->uniform_num; i++)
    {
        gcoOS_MemCopy(&kernelContext->uniforms[i].uniform, indexs[i].bin, sizeof(indexs[i].bin));
        kernelContext->uniforms[i].num = indexs[i].num;
        kernelContext->uniforms[i].index = indexs[i].index;
    }

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

