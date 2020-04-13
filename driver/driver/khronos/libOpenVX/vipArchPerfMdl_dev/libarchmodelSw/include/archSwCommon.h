/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/***********************************************************************************
* Copyright:    Verisilicon
* FileName:        archSwCommon.h
* Author:        JinboHuang
* Data:            2019-05-27
* Version:        0.5.00
* Description:    Head file of Common definiton for Arch Model SW Library for Internal
*
***************************************************************************************/
#ifndef _ARCH_SW_COMMON_H_
#define _ARCH_SW_COMMON_H_


#include "archSwType.h"
#include "archSwPerf.h"

#include "nnArchPerf.h"

/* For merge  */
#define USE_NNPERF_EXPORT
#define CACHE_SPACE_ENABLE
#define ALIGNMENT_64B
#define SPLIT_Z_DIMENSION


/*************************************** MACRO definition ************************************/
#define VX_PRINT_BUFFER_COUNT      2048

#define archInfo(...)  archPRINT(ARCH_DEBUG_LEVEL_INFO, __VA_ARGS__)
#define archWarning(...) archPRINT(ARCH_DEBUG_LEVEL_INFO, __VA_ARGS__)
#define archError(...) archPRINT(ARCH_DEBUG_LEVEL_INFO, __VA_ARGS__)

enum {
    ARCH_DEBUG_LEVEL_NONE = 0,
    ARCH_DEBUG_LEVEL_INFO = 1,
    /*VX_DEBUG_LEVEL_ERROR = 2,*/
};

/* Function macros */
#ifndef archmLENGTH_OF
#define archmLENGTH_OF(array)                 (sizeof(array) / sizeof((array)[0]))
#endif


/*************************************** Cost Type definition ************************************/
typedef enum _arch_model_cost_type
{
    ARCH_MODEL_DDR_COST          = 0,
    ARCH_MODEL_VIP_SRAM_COST     = 1,
    ARCH_MODEL_AXI_SRAM_COST     = 2,
    ARCH_MODEL_AXI_BUS_COST      = 3,
    ARCH_MODEL_DDR_KERNEL_COST   = 4,
    ARCH_MODEL_DDR_IN_IMAGE_COST = 5,
}
arch_model_cost_type;

struct _archModelCost_u64
{
    arch_uint64 cycle;
    arch_uint64 bw;
};

/*************************************** TP Command definition ************************************/
typedef enum _arch_tp_cmd_type_e
{
    TP_NONE = 0,
    TP_RESHUFFLE,
    TP_SINGLE_FC,
    TP_MAX_POOLING,
    TP_ACTIVATION,
    TP_LRN,
    TP_TRANSPOSE,
    TP_ROI_POOLING,
    TP_REORG,
    TP_REORG_DEPTH2SPACE,
    TP_REORG_SPACE2DEPTH,
    TP_REORG_SPACE2BATCH,
    TP_REORG_BATCH2SPACE,
    TP_ADD,
    TP_REVERSE,
    TP_UPSAMPLE,
    TP_DILATE_UPSAMPLE,
    TP_DILATE_UPSAMPLE2,
    TP_DILATE_RESHUFFLE,
    TP_BRICK,
    TP_RNN_INTERLEAVE,
    TP_TENSOR_COPY,
    TP_TENSOR_PAD,
    TP_LSTM_RESHUFFLE_INPUT,
    TP_LSTM_STATE_OUT,
    TP_LSTM_RESHUFFLE_STATE_O,
    TP_CMD_NUM,
    TP_ROI_POOLING_STEP_1,
    TP_ROI_POOLING_STEP_2,
    TP_UPSAMPLE_CLIP,
    TP_TENSOR_SQUEEZE,
    TP_TENSOR_PAD_CN,
    TP_TENSOR_COPY4CONCAT,
    TP_TENSOR_STRIDED_SLICE,
    TP_TENSOR_SVDF_MAP,

    TP_TENSOR_COUNT,
}
arch_tp_cmd_type_e;


/*************************************** Struction definition ************************************/

/*#pragma pack(push, 8)*/


/* Cost Information */
typedef struct _archModelCost
{
    arch_float64 cycle;
    arch_float64 bw;
}archModelCost;

/* Split Information. For store the operation after split */
typedef struct _archModelSplitInfo
{
    struct _archModelCost *savedSegmentCost;
    arch_uint32   **savedSIX;
    arch_uint32   **savedSIY;
    arch_uint32   **savedSIZ;
#ifdef CACHE_SPACE_ENABLE
    arch_uint32   **savedCacheSpace;            /* Need to allocate memory */
#endif
    arch_uint32   **savedTrspIvLayerChsIn;
    arch_uint32   **savedTrspIvLayerChsOut;
    struct _archModelCost   **savedCost;
    arch_uint8    **split_array;
    arch_int32    *bestCostSWTilingType;
}archModelSplitInfo;

/* Main Data Struction of Arch Model */
typedef struct _archModelInfo
{
    struct _archHAL_CHIPIDENTITY chipDentity;
    struct _archModelOpInfo    **opInfoArray;
    struct _archModelSplitInfo **splitInfoArray;
    arch_uint32                  totalOpCount;        /* total count, including NN/TP/SH */
    /* add for new */
    arch_uint32             actualCount;
    APMHandle               apm;
    archNN_DATABASE_FEATURE *pArchDataFeature;
} archModelInfo;
/*#pragma pack(pop)*/



/* data type size information */
typedef struct _arch_datatype_size_record_s
{
    arch_int32 type;
    size_t     size;
}
arch_datatype_size_record_s;


/* Arch Model rectangle information */
typedef struct _arch_rectangle_t {
    arch_uint32 start_x;          /*!< \brief The Start X coordinate. */
    arch_uint32 start_y;          /*!< \brief The Start Y coordinate. */
    arch_uint32 end_x;            /*!< \brief The End X coordinate. */
    arch_uint32 end_y;            /*!< \brief The End Y coordinate. */
} arch_rectangle_t;

typedef struct _arch_coordinates2d_t {
    arch_uint32 x;    /*!< \brief The X coordinate. */
    arch_uint32 y;    /*!< \brief The Y coordinate. */
} arch_coordinates2d_t;


/*! \brief The 3D Coordinates structure.
 * \ingroup group_basic_features
 */
typedef struct _arch_coordinates3d_t {
    arch_uint32 x;    /*!< \brief The X coordinate. */
    arch_uint32 y;    /*!< \brief The Y coordinate. */
    arch_uint32 z;    /*!< \brief The Z coordinate. */
} arch_coordinates3d_t;

/*! \brief The keypoint data structure.
 * \ingroup group_basic_features
 */
typedef struct _arch_keypoint_t {
    arch_int32   x;                 /*!< \brief The x coordinate. */
    arch_int32   y;                 /*!< \brief The y coordinate. */
    arch_float32 strength;          /*!< \brief The strength of the keypoint. Its definition is specific to the corner detector. */
    arch_float32 scale;             /*!< \brief Initialized to 0 by corner detectors. */
    arch_float32 orientation;       /*!< \brief Initialized to 0 by corner detectors. */
    arch_int32   tracking_status;   /*!< \brief A zero indicates a lost point. Initialized to 1 by corner detectors. */
    arch_float32 error;             /*!< \brief A tracking method specific error. Initialized to 0 by corner detectors. */
} arch_keypoint_t;


/* for debug assert */


/*************************************** API definition ************************************/


ARCH_INTERNAL_API archSTATUS archAllocateMemory(
    size_t size,
    archPOINTER *pMemory
    );

ARCH_INTERNAL_API void archFreeMemory(
    archPOINTER pMemory
    );

ARCH_INTERNAL_API archPOINTER archAllocateAndZeroMemory(
    size_t size
    );

void archSetDebugLevel(
    arch_int32 level
    );

void archPRINT(
    arch_uint32 level,
    const char *msg,
    ...
    );
const arch_char * archGetLayerName(
    arch_uint32 type
    );

arch_float64 _calcKernelCachePercentageSw(
    arch_uint32 kx,
    arch_uint32 ky,
    arch_uint32 kz,
    arch_uint32 z,
    arch_uint32 cores,
    arch_float64 coef_compress_ratio,
    arch_float64 cache_size_in_pixel,
    arch_bool full_cach_kernel_head_fix,
    arch_bool is_depth_wise
    );

arch_float32 ImageIdealCacheInPixelSw(
    arch_uint32 tile_x,
    arch_uint32 tile_y,
    arch_uint32 kx,
    arch_uint32 ky,
    arch_uint32 kz,
    arch_uint32 x,
    arch_uint32 y,
    arch_int32 xoffset,
    arch_int32 yoffset,
    arch_uint32 sub_x,
    arch_uint32 sub_y,
    arch_uint32 data_size,
    arch_bool image_not_packed_in_sram,
    arch_uint32 equivalent_vip_sram_width_in_byte
    );

void _calcArchModelCacheMode(
    APMHandle apm,
    arch_nn_config *pArchNnConfig,
    arch_perf perf,
    arch_int32 *image_ideal_cache_size_in_pixel
    );

/* this function reshape 1x1 convolution for better hw performance */
void reshapeImageTo16xN(
    arch_nn_config  *pArchNnConfig,
    /*arch_drv_option *pArchOptions,*/
    arch_uint32      src_buf,
    unsigned int      inimageSlice,
    unsigned int     *inImageStride,
    unsigned int   *outImageStride,
    /*unsigned int    &ky,
    unsigned int    &kz,*/
    unsigned int    *SIX,
    unsigned int    *SIY
    );

void updateSingleAllSilbling1X1(
    archModelOpInfo ** OpInfo,
    arch_uint32 index,
    arch_uint32 totalCount
    );

#endif /* _ARCH_SW_COMMON_H_ */
