/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_hal_user_hardware_thirdparty_h_
#define __gc_hal_user_hardware_thirdparty_h_

#ifdef __cplusplus
extern "C" {
#endif


#define gcmTPALIGN(data, align) \
    ((data + (align - 1)) & ~(align - 1))

#define _gcmTPGETSIZE(reg, field) \
    (reg##_##field##_End - reg##_##field##_Start + 1)

#define _gcmTPGETMASK(reg, field) \
    (_gcmTPGETSIZE(reg, field) == 32 ? ~0 : (~(~0 << _gcmTPGETSIZE(reg, field))))

#define gcmTPSETMASKEDVALUE(reg, field, value) \
(\
    ((gctUINT)\
        ((~((~reg##_##field##_##value & _gcmTPGETMASK(reg, field)) \
            << reg##_##field##_Start)) \
        & (~(reg##_MASK_##field##_MASKED << reg##_MASK_##field##_Start))) \
    ) \
)

#define gcmTPSETVALUE(data, reg, field, value) \
(\
     (((gctUINT) (data)) & \
      ~(_gcmTPGETMASK(reg, field) << reg##_##field##_Start)) \
     | \
     ((reg##_##field##_##value & _gcmTPGETMASK(reg, field)) \
      << reg##_##field##_Start)\
)

#define gcmTPSETFIELD(data, reg, field, value) \
(\
     (((gctUINT) (data)) & \
      ~(_gcmTPGETMASK(reg, field) << reg##_##field##_Start)) \
     | \
     ((value & _gcmTPGETMASK(reg, field)) \
      << reg##_##field##_Start)\
)

typedef enum
{
    gcvTP_CMD_UNIFIED,
    gcvTP_CMD_NO_COMPRESSION,
    gcvTP_CMD_HAS_COMPRESSION,
    gcvTP_CMD_SRC_COMPRESSION,
    gcvTP_CMD_DST_COMPRESSION,
}
gceTPCMDTYPE;

typedef enum
{
    gcvTP_V10 = 0x10,
    gcvTP_V11 = 0x11,
}
gceTPTYPE;

gceSTATUS
gcoTPHARDWARE_CheckSurface(
    IN gctUINT Addr,
    IN gctUINT StatusAddr,
    IN gceSURF_FORMAT Format,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctUINT Stride,
    IN gceSURF_ROTATION Rotation,
    IN gctUINT Tiling,
    IN gceTPTYPE TPCType
    );

gceSTATUS
gcoTPHARDWARE_QueryStateCmdLen(
    IN gceTPCMDTYPE CmdType,
    IN gctUINT Num,
    IN gceTPTYPE TPCType,
    OUT gctUINT* StateCmdLen
    );

/***************************************/
/* TPC V1.0 */
gceSTATUS
gcoTPHARDWARE_EnableTPCCompression_V10(
    IN gcoHARDWARE Hardware,
    IN gctUINT Enable
    );

gceSTATUS
gcoTPHARDWARE_StartTPCCompression_V10(
    IN gcoHARDWARE Hardware,
    IN gctUINT SrcCompressed,
    IN gctUINT DstCompressed
    );

gceSTATUS
gcoTPHARDWARE_EndTPCCompression_V10(
    IN gcoHARDWARE Hardware
    );

gceSTATUS
gcoTPHARDWARE_SetSrcTPCCompression_V10(
    IN gcoHARDWARE Hardware,
    IN gctUINT Enable,
    IN gctUINT Index,
    IN gctUINT SrcAddr,
    IN gctUINT SrcStatusAddr,
    IN gceSURF_FORMAT SrcFormat,
    IN gctUINT SrcWidth,
    IN gctUINT SrcHeight,
    IN gctUINT SrcStride,
    IN gceSURF_ROTATION SrcRotation
    );

gceSTATUS
gcoTPHARDWARE_SetDstTPCCompression_V10(
    IN gcoHARDWARE Hardware,
    IN gctUINT Enable,
    IN gctUINT DstAddr,
    IN gctUINT DstStatusAddr,
    IN gceSURF_FORMAT DstFormat,
    IN gctUINT DstWidth,
    IN gctUINT DstHeight,
    IN gctUINT DstStride,
    IN gceSURF_ROTATION DstRotation
    );


/* TPC V1.1 */
gceSTATUS
gcoTPHARDWARE_EnableTPCCompression_V11(
    IN gcoHARDWARE Hardware,
    IN gctUINT Enable
    );

gceSTATUS
gcoTPHARDWARE_StartTPCCompression_V11(
    IN gcoHARDWARE Hardware,
    IN gctUINT SrcCompressed,
    IN gctUINT DstCompressed
    );

gceSTATUS
gcoTPHARDWARE_EndTPCCompression_V11(
    IN gcoHARDWARE Hardware
    );

gceSTATUS
gcoTPHARDWARE_SetSrcTPCCompression_V11(
    IN gcoHARDWARE Hardware,
    IN gctUINT Enable,
    IN gctUINT Index,
    IN gctUINT SrcAddr,
    IN gctUINT SrcStatusAddr,
    IN gceSURF_FORMAT SrcFormat,
    IN gctUINT SrcWidth,
    IN gctUINT SrcHeight,
    IN gctUINT SrcStride,
    IN gceSURF_ROTATION SrcRotation
    );

gceSTATUS
gcoTPHARDWARE_SetDstTPCCompression_V11(
    IN gcoHARDWARE Hardware,
    IN gctUINT Enable,
    IN gctUINT DstAddr,
    IN gctUINT DstStatusAddr,
    IN gceSURF_FORMAT DstFormat,
    IN gctUINT DstWidth,
    IN gctUINT DstHeight,
    IN gctUINT DstStride,
    IN gceSURF_ROTATION DstRotation
    );

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_user_hardware_thirdparty_h_ */

