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


#ifndef __gc_halti5_chip_h__
#define __gc_halti5_chip_h__

#define BLT_SWIZZLE_EX(r, g, b, a) \
    gcmSETFIELDVALUE(0, GCREG_BLT_SWIZZLE_EX_SRC_SWIZZLE, RED, r) |\
    gcmSETFIELDVALUE(0, GCREG_BLT_SWIZZLE_EX_SRC_SWIZZLE, GREEN, g) |\
    gcmSETFIELDVALUE(0, GCREG_BLT_SWIZZLE_EX_SRC_SWIZZLE, BLUE, b) |\
    gcmSETFIELDVALUE(0, GCREG_BLT_SWIZZLE_EX_SRC_SWIZZLE, ALPHA, a)


#define __VK_STALL_FE(a, b) \
    *(*a)++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL); \
    *(*a)++ = (b); \

#define __VK_STALL_RA(a, b) \
    __vkCmdLoadSingleHWState(a, 0x0F00, VK_FALSE, (b));\

#define __VK_ENABLE3DCORE(a, b) \
    *(*a)++ = gcmSETFIELDVALUE(0, GCCMD_CHIP_ENABLE_COMMAND, OPCODE, CHIP_ENABLE) \
            | (b); \
    *(*a)++ = 0; \

/* Check if any tileStatus enable*/
#define __VK_AnyTSEnable(tsResource, pRange, anyTsEnable) \
{\
    uint32_t i = pRange->baseMipLevel;\
    uint32_t j = pRange->baseArrayLayer;\
    for (; i < (pRange->baseMipLevel + pRange->levelCount); i++)\
    {\
        for (; j < (pRange->baseArrayLayer + pRange->layerCount); j++)\
        {\
            if ((tsResource) && (tsResource->tileStatusDisable[i][j] == VK_FALSE))\
            {\
                *anyTsEnable = VK_TRUE;\
                break;\
            }\
        }\
    }\
}\

/* Check whether the tileStatus can enable or not, there are three cases:
** 1)tileStatus has been disabled;
** 2)For multi-slice, if fcValue is different, tileStatus should be disabled;
** 3)For multi-slice, if fcValueUpper is different, tileStatus should be disabled.
*/
#define __VK_CanTSEnable(tsResource, pRange, canTsEnable) \
{\
    if (tsResource)\
    {\
        uint32_t i = pRange->baseMipLevel;\
        uint32_t j = pRange->baseArrayLayer;\
        for (; i < (pRange->baseMipLevel + pRange->levelCount); i++)\
        {\
            for (j = pRange->baseArrayLayer; j < (pRange->layerCount + pRange->baseArrayLayer); j++)\
            {\
                if (tsResource->tileStatusDisable[i][j] == VK_TRUE)\
                {\
                    *canTsEnable = VK_FALSE;\
                    break;\
                }\
                if (tsResource->fcValue[i][j] != tsResource->fcValue[pRange->baseMipLevel][pRange->baseArrayLayer])\
                {\
                    *canTsEnable = VK_FALSE;\
                    break;\
                }\
                if (tsResource->fcValueUpper[i][j] != tsResource->fcValueUpper[pRange->baseMipLevel][pRange->baseArrayLayer])\
                {\
                    *canTsEnable = VK_FALSE;\
                    break;\
                }\
            }\
        }\
    }\
}\

#define TX_COMP_RED_SWIZZLE_SHIFT    0
#define TX_COMP_GREEN_SWIZZLE_SHIFT  3
#define TX_COMP_BLUE_SWIZZLE_SHIFT   6
#define TX_COMP_ALPHA_SWIZZLE_SHIFT  9

#define TX_FORMAT_OLD_SHIFT            0
#define TX_FORMAT_NEW_SHIFT            8
#define TX_FORMAT_COLOR_SWIZZLE_SHIFT  16
#define TX_FORMAT_SRGB_SHIFT           24
#define TX_FORMAT_FAST_FILTER_SHIFT    31


#define TX_COMP_SWIZZLE(r, g, b, a) \
    {r, g, b ,a }

#define TX_FORMAT(oldFormat, newFormat, color_swizzle, srgb, fast_filter) \
    (oldFormat << TX_FORMAT_OLD_SHIFT)                | \
    (newFormat << TX_FORMAT_NEW_SHIFT)                | \
    (color_swizzle << TX_FORMAT_COLOR_SWIZZLE_SHIFT)  | \
    (srgb << TX_FORMAT_SRGB_SHIFT)                     | \
    (fast_filter << TX_FORMAT_FAST_FILTER_SHIFT)


enum
{
    SWZL_USE_RED   = 0x0,
    SWZL_USE_GREEN = 0x1,
    SWZL_USE_BLUE  = 0x2,
    SWZL_USE_ALPHA = 0x3,
    SWZL_USE_ZERO  = 0x4,
    SWZL_USE_ONE   = 0x5,
};

typedef enum
{
    SWIZZLE_INVALID       = 0,
    SWIZZLE_RED           = 1,
    SWIZZLE_GREEN         = 2,
    SWIZZLE_BLUE          = 3,
    SWIZZLE_ALPHA         = 4,
    SWIZZLE_ZERO          = 5,
    SWIZZLE_ONE           = 6,
}SwizzleComponent;

typedef enum
{
    HW_RESOURCEVIEW_USAGE_TX        = 1 << 0,
    HW_RESOURCEVIEW_USAGE_SH        = 1 << 1,
    HW_RESOURCEVIEW_USAGE_DEPTH     = 1 << 2,
    HW_RESOURCEVIEW_USAGE_COLOR     = 1 << 3,
}HwResourceViewUsage;

typedef enum
{
    HW_CACHE_TEXTURE_L2       = 1 << 0,
    HW_CACHE_TEXUTRE_L1       = 1 << 1,
    HW_CACHE_TEXTURE_DESC     = 1 << 2,
    HW_CACHE_VST_L2           = 1 << 3,
    HW_CACHE_VST_L1           = 1 << 4,
    HW_CACHE_VST_DESC         = 1 << 5,
    HW_CACHE_SH_L1            = 1 << 6,
    HW_CACHE_DEPTH            = 1 << 7,
    HW_CACHE_COLOR            = 1 << 8,
    HW_CACHE_TS               = 1 << 9,
    HW_CACHE_INSTRUCTION      = 1 << 10,
    HW_CACHE_TFB              = 1 << 11,
    HW_CACHE_L2               = 1 << 12,
    HW_CACHE_VERTEX_DATA      = 1 << 13, /* for multicluster */

    /* combined mask */
    HW_CACHE_TEXTURE_DATA     = HW_CACHE_TEXTURE_L2 | HW_CACHE_TEXUTRE_L1,
    HW_CACHE_TEXTURE_ALL      = HW_CACHE_TEXTURE_DATA | HW_CACHE_TEXTURE_DESC,
    HW_CACHE_VST_DATA         = HW_CACHE_VST_L2 | HW_CACHE_VST_L1,
    HW_CACHE_VST_ALL          = HW_CACHE_VST_DATA | HW_CACHE_VST_DESC,
    HW_CACHE_TX_DESC          = HW_CACHE_TEXTURE_DESC | HW_CACHE_VST_DESC,
    HW_CACHE_WRITABLE         = HW_CACHE_SH_L1 | HW_CACHE_DEPTH | HW_CACHE_COLOR | HW_CACHE_TS | HW_CACHE_TFB,
    HW_CACHE_ALL              = ~0, /* must be ~0 as core layer code which doesn't HwCacheMask definition */
    HW_CACHE_NONE             = 0,
}HwCacheMask;

typedef struct
{
    const VkVertexInputAttributeDescription *sortedAttributeDescPtr;
    uint32_t hwFetchSize;
    VkBool32 hwFetchBreak;
    uint32_t hwDataType;
    uint32_t hwSize;
    uint32_t hwNormalized;
    VkBool32 integer;
    VkBool32 is16Bit;
} HwVertexAttribDesc;

typedef struct
{
    uint32_t hwFormat;
    VkBool32 hwSrgb;
    VkBool32 hwSaturation;
    uint32_t hwOutputMode;
}HwPEDesc;

typedef struct
{
    uint32_t        hwFormat;
    uint32_t        pixelSize;
    uint32_t        bltSwizzleEx;
    uint32_t        downSampleMode;
    VkBool32        sRGB;
    VkBool32        fakeFormat;
} HwBLTDesc;

typedef struct
{
    uint32_t        hwFormat;
    uint32_t        pixelSize;
    uint32_t        downSampleMode;
    VkBool32        fakeFormat;
    VkBool32        flipRB;
} HwRsDesc;

typedef struct __vkFormatToHWTxFmtInfo
{
    uint32_t vkFormat;
    int32_t  hwFormat;
    uint32_t hwSwizzles[4];
} __vkFormatToHwTxFmtInfo;

typedef struct
{
    VkDeviceMemory descriptor;
    VkBool32 sRGB;
    VkBool32 fast_filter;
    VkBool32 sampleStencil;
    VkBool32 msaaImage;
    VkBool32 isCubmap;
    VkFlags  invalidBorderColorMask;
    VkBool32 hasTileStatus;

    /* Needed if texelFetch need to be implemented using texelLoad */
    uint32_t baseWidth;
    uint32_t baseHeight;
    uint32_t baseDepth;
    uint32_t baseSlice;
    struct
    {
        uint32_t hwSamplerMode_p1;
        uint32_t hwSampleWH;
        uint32_t hwSampleLogWH_p1;
        uint32_t hwSampler3D_p1;
        uint32_t hwSamplerModeEx;
        uint32_t hwBaseLOD_p1;
        uint32_t hwSliceSize;
        uint32_t hwTxConfig2;
        uint32_t hwTxConfig3;
        uint32_t hwSamplerLinearStride;
        uint32_t hwTxASTCEx;
        uint32_t hwSamplerLodAddresses[14];
    } halti2;
    struct
    {
        uint32_t hwTxSizeExt;
        uint32_t hwSamplerVolumeExt;
    } halti3;

} HwTxDesc;

typedef struct
{
    uint32_t imageInfo[4];

    /* Needed if texelFetch need to be implemented using texelLoad
    for 128 bpp texelbuffer use img to do*/
    uint32_t baseWidth;
    uint32_t baseHeight;
    uint32_t baseDepth;
    uint32_t baseSlice;
} HwImgDesc;


typedef union
{
    struct
    {
        uint32_t hwSamplerCtrl0;
        uint32_t hwSamplerCtrl1;
        uint32_t hwSamplerLodMinMax;
        uint32_t hwSamplerLodBias;
        uint32_t hwSamplerAnisVal;
    } halti5;
    struct
    {
        uint32_t hwSamplerMode_p0;
        uint32_t hwSamplerLOD;
        uint32_t hwBaseLOD_p0;
        uint32_t hwSampler3D_p0;
        uint32_t anisoLog;
    } halti2;
    struct
    {
        uint32_t hwSamplerMode_p0;
        uint32_t hwSamplerLOD;
        uint32_t hwSamplerLODBias;
        uint32_t hwBaseLOD_p0;
        uint32_t hwSampler3D_p0;
        uint32_t anisoLog;
    } halti3;

} HwSamplerDesc;

typedef uint16_t halti5_patch_key;

typedef struct
{
    HwSamplerDesc samplerDesc;
    halti5_patch_key patchKey;
} halti5_sampler;

typedef struct
{
    HwTxDesc txDesc[__VK_MAX_PARTS];
    HwPEDesc peDesc;
    HwImgDesc imgDesc[__VK_MAX_PARTS];
    uint32_t patchFormat;
    halti5_patch_key patchKey;
    HwResourceViewUsage usedUsageMask;
    HwSamplerDesc samplerDesc;
    VkBool32 isCompatiableImage;
} halti5_imageView;

typedef struct
{
    HwImgDesc imgDesc[__VK_MAX_PARTS];
    HwTxDesc txDesc[__VK_MAX_PARTS];
    HwSamplerDesc samplerDesc;
    uint32_t  patchFormat;
    halti5_patch_key patchKey;
    HwResourceViewUsage usedUsageMask;
} halti5_bufferView;

typedef struct
{
    /* clusterAliveMask is an enabled physical mask, may not all clusters are enabled by user.*/
    uint32_t clusterAliveMask;
    uint32_t clusterAliveCount;
    uint32_t clusterMinID;
    uint32_t clusterMaxID;
}halti5_clusterInfo;

enum
{
    HALTI5_BLIT_2D_UNORM_FLOAT = 0,
    HALTI5_BLIT_2D_UNORM_FLOAT_HWDOUBLEROUNDING,
    HALTI5_BLIT_2D_UNORM_TO_PACK16,
    HALTI5_BLIT_2D_SINT,
    HALTI5_BLIT_2D_UINT,
    HALTI5_BLIT_2D_UINT_TO_A2B10G10R10_PACK,

    HALTI5_BLIT_2D_SFLOAT_DOWNSAMPLE,
    HALTI5_BLIT_2D_SINT_DOWNSAMPLE,
    HALTI5_BLIT_2D_UINT_DOWNSAMPLE,

    HALTI5_BLIT_3D_UNORM_FLOAT,
    HALTI5_BLIT_3D_UNORM_TO_PACK16,
    HALTI5_BLIT_3D_SINT,
    HALTI5_BLIT_3D_UINT,
    HALTI5_BLIT_3D_UINT_TO_A2B10G10R10_PACK,

    HALTI5_BLIT_2LAYERS_IMG_TO_BUF,
    HALTI5_BLIT_BUF_TO_2LAYERS_IMG,
    HALTI5_BLIT_COPY_2D_UNORM_FLOAT,
    HALTI5_BLIT_COPY_2D_UNORM_TO_A4R4G4B4,
    HALTI5_BLIT_COPY_2D_SINT,
    HALTI5_BLIT_COPY_2D_UINT,
    HALTI5_BLIT_COPY_OQ_QUERY_POOL,

    HALTI3_CLEAR_2D_UINT,
    HALTI3_CLEAR_TO_2LAYERS_IMG,
    HALTI3_BLIT_BUFFER_2D,

    HALTI5_BLIT_NUM,
};

enum
{
    __CHANNEL_MASK_R    = 0x1,
    __CHANNEL_MASK_G    = 0x2,
    __CHANNEL_MASK_B    = 0x4,
    __CHANNEL_MASK_A    = 0x8,
    __CHANNEL_MASK_RGB  = __CHANNEL_MASK_R | __CHANNEL_MASK_G | __CHANNEL_MASK_B,
    __CHANNEL_MASK_GBA  = __CHANNEL_MASK_G | __CHANNEL_MASK_B | __CHANNEL_MASK_A,
    __CHANNEL_MASK_RGBA = __CHANNEL_MASK_RGB | __CHANNEL_MASK_A,
};

enum
{
    __PACK_FORMAT_INVALID      = 0,
    __PACK_FORMAT_A4R4G4B4     = 1,
    __PACK_FORMAT_B4G4R4A4     = 2,
    __PACK_FORMAT_R5G6B5       = 3,
    __PACK_FORMAT_A1R5G5B5     = 4,
    __PACK_FORMAT_A2B10G10R10  = 5,
    __PACK_FORMAT_R32G32B32A32 = 6,
    __PACK_FORMAT_R16G16B16A16 = 7,
    __PACK_FORMAT_R32G32       = 8,

};

#define HALTI5_INSTANCE_CMD_BUFSIZE 10
typedef struct
{
    __vkDevContext *devCtx;
    PROGRAM_EXECUTABLE_PROFILE pep;
    VSC_HW_PIPELINE_SHADERS_STATES hwStates;
    __vkUtilsHashObject *ownerCacheObj;
    uint32_t instanceCmd[HALTI5_INSTANCE_CMD_BUFSIZE];
    uint32_t curInstanceCmdIndex;
    VkBool32 memoryWrite;
} halti5_vscprogram_instance;

typedef struct __vkComputeBlitParams
{
    VkBool32   rawCopy;
    VkBool3D   reverse;
    VkBool32   srcSRGB;
    VkBool32   dstSRGB;
    uint32_t   srcFormat;   /* tex format finally used to program compute blit */
    uint32_t   dstFormat;   /* img format finally used to program compute blit */
    uint32_t   srcParts;
    uint32_t   dstParts;
    const VkComponentMapping *txSwizzles;
    VkBool32   flushTex;

    VkOffset3D srcOffset;
    VkOffset3D dstOffset;
    VkExtent3D srcExtent;
    VkExtent3D dstExtent;
    VkExtent3D srcSize;
    uint32_t   packFormat;
    uint32_t   channelWriteMask;    /* channel level write mask */
    uint32_t   uClearValue0[4];
    uint32_t   uClearValue1[4];
} __vkComputeBlitParams;

typedef struct halti5_vscprogram_blit
{
    VkBool32                        inited;
    uint32_t                        kind;

    PROGRAM_EXECUTABLE_PROFILE      pep;
    VSC_HW_PIPELINE_SHADERS_STATES  hwStates;

    PROG_VK_COMBINED_TEX_SAMPLER_TABLE_ENTRY    *srcTexEntry[2];
    PROG_VK_STORAGE_TABLE_ENTRY                 *srcImgEntry[2];
    PROG_VK_STORAGE_TABLE_ENTRY                 *dstImgEntry[2];
    PROG_VK_UNIFORM_BUFFER_TABLE_ENTRY          *constEntry;

    VkResult (*program_src)(
        IN  __vkCommandBuffer *pCmdBuf,
        IN  struct halti5_vscprogram_blit *blitProg,
        IN  uint32_t **states,
        IN  __vkBlitRes *srcRes,
        IN  __vkBlitRes *dstRes,
        IN  VkFilter filter,
        OUT __vkComputeBlitParams *params
    );

    VkResult (*program_dst)(
        IN  __vkCommandBuffer *pCmdBuf,
        IN  struct halti5_vscprogram_blit *blitProg,
        IN  uint32_t **states,
        IN  __vkBlitRes *srcRes,
        IN  __vkBlitRes *dstRes,
        OUT __vkComputeBlitParams *params
    );

    VkResult (*program_const)(
        IN __vkCommandBuffer *pCmdBuf,
        IN struct halti5_vscprogram_blit *blitProg,
        IN uint32_t **states,
        IN __vkComputeBlitParams *params
    );

} halti5_vscprogram_blit;

typedef struct halti5_tweak_handler
{
    const char reversedName[__VK_MAX_NAME_LENGTH];
    uint32_t index;

    VkBool32 (* match)(
        __vkDevContext *devCtx,
        __vkPipeline *pip,
        void *createInfo
        );

    VkResult (* tweak)(
        __vkDevContext *devCtx,
        __vkPipeline *pip,
        void *createInfo,
        struct halti5_tweak_handler *handler
        );

    VkResult (*collect)(
        __vkDevContext *devCtx,
        __vkPipeline *pip,
        struct halti5_tweak_handler *handler
        );

    VkResult (* set)(
        __vkCommandBuffer *cmdBuf,
        __vkDrawComputeCmdParams *cmdParams,
        struct halti5_tweak_handler *handler
        );

    VkResult (* cleanup)(
        __vkDevContext *devCtx,
        struct halti5_tweak_handler *handler
        );

    VkResult (* copy)(
        __vkCommandBuffer *cmd,
        __vkDevContext *devCtx,
        __vkPipeline *pip,
        __vkImage *srcImg,
        __vkBuffer *dstBuf,
        struct halti5_tweak_handler *handler
        );

    void *privateData;
} halti5_tweak_handler;

VkResult halti5_tweak_detect(
    __vkDevContext *devCtx
    );

#define HALTI5_INIT_CMD_SIZE_INUINT 12

typedef struct
{
    /* Msaa tables */
    gctUINT32       sampleCoords2;
    gctUINT32       sampleCoords4[3];
    gcsCENTROIDS    centroids2;
    gcsCENTROIDS    centroids4[3];
    gctFLOAT        sampleLocations[4][4];

    SHADER_HANDLE   patchLib;

    halti5_vscprogram_blit blitProgs[HALTI5_BLIT_NUM];

    uint32_t initialCmds[HALTI5_INIT_CMD_SIZE_INUINT];
    uint32_t curInitCmdIndex;

    halti5_tweak_handler **ppTweakHandlers;
    uint32_t tweakHandleCount;
    halti5_clusterInfo clusterInfo;

    struct
    {
    VkResult (*helper_convertHwTxDesc)(
        __vkDevContext *devCtx,
        __vkImageView *imgv,
        __vkBufferView *bufv,
        HwTxDesc *hwTxDesc
        );

    void (*helper_convertHwSampler)(
        __vkDevContext *devCtx,
        const VkSamplerCreateInfo *createInfo,
        HwSamplerDesc *hwSamplerDesc
        );

    void (*helper_setSamplerStates)(
        __vkCommandBuffer *cmdBuf,
        uint32_t **commandBuffer,
        __vkImageView *imgv,
        uint32_t txHwRegisterIdx,
        HwTxDesc *txDesc,
        HwSamplerDesc *samplerDesc,
        uint32_t borderColorIdx,
        uint32_t hwSamplerNo,
        uint32_t shaderConfigData
        );

    VkResult (*pip_emit_vsinput)(
        __vkDevContext *devCtx,
        __vkPipeline *pip,
        const VkGraphicsPipelineCreateInfo * info
        );
    } minorTable;

} halti5_module;

enum
{
    HALTI5_GFXPIPELINE_SWITCH_CACHEMODE_DIRTY          = 1 << 0,
    HALTI5_GFXPIPELINE_SWITCH_SINGLE_PE_DIRTY          = 1 << 1,
    HALTI5_GFXPIPELINE_SWITCH_STENCIL_MODE_DIRTY       = 1 << 2,
    HALTI5_GFXPIPELINE_SWITCH_PE_DEPTH_DIRTY           = 1 << 3,
    HALTI5_GFXPIPELINE_SWITCH_EARLYZ_DIRTY             = 1 << 4,
    HALTI5_GFXPIPELINE_SWITCH_DEPTH_COMPAREOP_DIRTY    = 1 << 5,
    HALTI5_GFXPIPELINE_SWITCH_DESTINATION_READ_DIRTY   = 1 << 6,
    HALTI5_GFXPIPELINE_SWITCH_UNIFIED_RESOURCE_DIRTY   = 1 << 7,

    HALTI5_GFXPIPELINE_SWITCH_ALL_DIRTY                = 0xFFFFFFFF,
};


typedef struct
{
    int32_t  gfxPipelineSwitchDirtyMask;
    uint32_t memoryConfig;
    uint32_t memoryConfigMRT[gcdMAX_DRAW_BUFFERS];
#if __VK_ENABLETS
    VkBool32 texHasTileStatus[gcdTXDESCRIPTORS];
    int32_t texTileStatusSlotIndex[gcdTXDESCRIPTORS];
    uint32_t textureControlAddrReg[gcdTXDESCRIPTORS];
    __vkImageView * imgvWithTS[gcdTXDESCRIPTORS];
    int32_t texTileStatusSlotUser[gcdSAMPLER_TS];
    int32_t texTileStatusSlotDirty;
    int32_t txDescDirty;
    VkBool32 rt0TSEnable;
#endif
    HwResourceViewUsage newResourceViewUsageMask; /* It's a per-draw mask */
} halti5_commandBuffer;

enum halti_patch_type
{
    HALTI5_PATCH_PE_EXTRA_OUTPUT         = 0,
    HALTI5_PATCH_TX_EXTRA_INPUT          = 1,
    HALTI5_PATCH_REPLACE_TXLD_WITH_IMGLD = 2,
    HALTI5_PATCH_UNORMALIZED_SAMPLER     = 3,
    HALTI5_PATCH_TX_GATHER               = 4,
    HALTI5_PATCH_TX_EXTRA_INPUT_GRAD     = 5,
    HALTI5_PATCH_TX_GATHER_PCF           = 6,
};

enum
{
    HALTI5_PATCH_PE_EXTRA_OUTPUT_BIT          = 1 << HALTI5_PATCH_PE_EXTRA_OUTPUT,
    HALTI5_PATCH_TX_EXTRA_INPUT_BIT           = 1 << HALTI5_PATCH_TX_EXTRA_INPUT,
    HALTI5_PATCH_REPLACE_TXLD_WITH_IMGLD_BIT  = 1 << HALTI5_PATCH_REPLACE_TXLD_WITH_IMGLD,
    HALTI5_PATCH_UNORMALIZED_SAMPLER_BIT      = 1 << HALTI5_PATCH_UNORMALIZED_SAMPLER,
    HALTI5_PATCH_TX_GATHER_BIT                = 1 << HALTI5_PATCH_TX_GATHER,
    HALTI5_PATCH_TX_EXTRA_INPUT_GRAD_BIT      = 1 << HALTI5_PATCH_TX_EXTRA_INPUT_GRAD,
    HALTI5_PATCH_TX_GATHER_PCF_BIT            = 1 << HALTI5_PATCH_TX_GATHER_PCF,

    HALTI5_PATCHKEY_ALL_BITS                = 0xFFFF,
};

typedef struct
{
    VkShaderStageFlags patchStages;
    uint32_t patchFormat;
    uint32_t binding;
    uint32_t arrayIndex;
    SwizzleComponent swizzles[4];
    VkImageViewType viewType;
    VkCompareOp   compareOp;
} halti5_patch_info;

typedef struct
{
    halti5_patch_key  *patchKeys;  /* an array of key */
    halti5_patch_info  *patchInfos;  /* an array of patch info */
    uint32_t  numofEntries; /* how many entries we allocate for patchMasks and patchInfos */
    uint32_t  numofKeys; /* how many entries which really have patch information */
} halti5_descriptorSet;

typedef struct
{
    halti5_vscprogram_instance *masterInstance;
    halti5_vscprogram_instance *curInstance;
    __vkUtilsHash *vscProgInstanceHash;

    VSC_PROGRAM_RESOURCE_LAYOUT *vscResLayout;
    SHADER_HANDLE  vscShaderArray[VSC_MAX_SHADER_STAGE_COUNT];

    uint32_t cmdBuffer[__VK_PIPELINE_CMDBUFFER_MAXSIZE];
    uint32_t curCmdIndex;
    VkBool32 *separateBindingProgramed;
    uint32_t countOfseparateBinding;

    VkBool32 fastFilterDisable;

    VkBool32 vanilla; /* plain pipeline and we don't need patch it */
    /* patch key for each descriptor set */
    halti5_patch_key *patchKeys[__VK_MAX_DESCRIPTOR_SETS];
    /* number of patch key for each descriptor set */
    uint32_t patchKeyCount[__VK_MAX_DESCRIPTOR_SETS];
    /* number of valid key array */
    uint32_t keyCount;

    VSC_PROG_LIB_LINK_ENTRY *linkEntries;
    uint32_t linkEntryCount;

    halti5_tweak_handler *tweakHandler;
} halti5_pipeline;

enum HwCacheMode
{
    CHIP_CACHEMODE_64   = 0,
    CHIP_CACHEMODE_128  = 1,
    CHIP_CACHEMODE_256  = 2,
};

typedef struct
{
    uint32_t hwRegNo;
    uint32_t hwRegCount;
    uint32_t hwRegAddress;
    VkBool32 bUsed;
} halti5_priv_const;

typedef struct
{
    /* MUST be the first member */
    halti5_pipeline chipPipeline;

    uint32_t instancedVertexBindingMask;
    uint32_t instancedVertexBindingStride[__VK_MAX_VERTEX_BUFFER_BINDINGS];

    VkBool32 subSampleZUsedInPS;
    VkBool32 sampleMaskInPos;

    VkBool32 colorPipeEnable;

    VkBool32 hasOne8bitCb;
    VkBool32 has16orLessBppImage;
    VkBool32 allColorWriteOff;
    VkBool32 anyPartialColorWrite;
    VkBool32 anyBlendEnabled;

    struct
    {
        uint32_t mask;
        uint32_t count;
        struct
        {
            uint32_t format;
            uint32_t locations[__VK_MAX_PARTS];
            uint32_t partCount;
        } outputs[__VK_MAX_RENDER_TARGETS];
    } patchOutput;

    halti5_priv_const baseInstance;
    halti5_priv_const sampleLocation;
    halti5_priv_const ehableMultiSampleBuffers;
    halti5_priv_const useViewIndex;

    VkBool32 depthOnly;
    VkBool32 peDepth;
    VkBool32 earlyZ;
    uint32_t stencilMode;
    VkCompareOp depthCompareOp;
    VkBool32 destinationRead;
    VkBool32 singlePEpipe;
    uint32_t raControlEx;
    enum HwCacheMode hwCacheMode;

    uint32_t regDepthConfig;
    uint32_t regRAControl;

    uint32_t psOutCntl4to7;

} halti5_graphicsPipeline;

typedef struct
{
    /* MUST be the first member */
    halti5_pipeline chipPipeline;

    halti5_priv_const numberOfWorkGroup;
} halti5_computePipeline;

/* Halti5 Chip function prototypes */
VkResult halti5_initializeChipModule(
    VkDevice device
    );

VkResult halti5_finalizeChipModule(
    VkDevice device
    );

VkResult halti5_createGraphicsPipeline(
    VkDevice device,
    const VkGraphicsPipelineCreateInfo *info,
    VkPipeline pipeline
    );

VkResult halti5_createComputePipeline(
    VkDevice device,
    const VkComputePipelineCreateInfo *info,
    VkPipeline pipeline
    );

VkResult halti5_pip_emit_vsinput(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo * info
    );

VkResult halti5_helper_convert_VertexAttribDesc(
    __vkDevContext *devCtx,
    uint32_t count,
    HwVertexAttribDesc *hwVertxAttribDesc
    );

VkResult __vkComputeClearVal(
    IN uint32_t clearImgFormat,
    IN VkImageAspectFlags aspectMask,
    IN VkClearValue *vkClearValue,
    IN uint32_t partIndex,
    OUT uint32_t *pClearVals,
    OUT uint32_t *pClearBitMasks,
    OUT uint32_t *pClearByteMasks
    );

uint32_t halti5_helper_convertHwImgSwizzle(
    VkComponentSwizzle swizzle
    );

uint32_t halti5_helper_convertHwTxSwizzle(
    const __vkFormatInfo * residentFormatInfo,
    VkComponentSwizzle swizzle,
    uint32_t currentSwizzle,
    const uint32_t hwComponentSwizzle[]
    );

VkResult halti5_clearImage(
    VkCommandBuffer cmdBuf,
    VkImage image,
    VkImageSubresource *subResource,
    VkClearValue *clearValue,
    VkRect2D *rect
    );

VkResult halti5_copyImage(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    VkBool32 rawCopy,
    VkFilter filter,
    VkBool32 oldPath
    );

VkResult halti5_fillBuffer(
    VkCommandBuffer cmdBuf,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkDeviceSize size,
    uint32_t data
    );

VkResult halti5_copyBuffer(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    uint64_t copySize
    );

VkResult halti5_updateBuffer(
    VkCommandBuffer cmdBuf,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkDeviceSize size,
    const uint32_t* pData
    );

VkResult halti5_computeBlit(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    VkBool32 rawCopy,
    VkBool3D *reverse,
    VkFilter filter
    );

VkResult halti5_computeCopyOQQueryPool(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes
    );

VkResult halti5_draw(
    VkCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance
    );

VkResult halti5_drawIndexed(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance
    );

VkResult halti5_drawDirect(
    VkCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance
    );

VkResult halti5_drawIndexedDirect(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance
    );

VkResult halti5_splitDrawIndexed(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance
    );

VkResult halti5_drawIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t count,
    uint32_t stride
    );

VkResult halti5_drawIndexedIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t count,
    uint32_t stride
    );

VkResult halti5_dispatch(
    VkCommandBuffer commandBuffer,
    uint32_t baseX,
    uint32_t baseY,
    uint32_t baseZ,
    uint32_t x,
    uint32_t y,
    uint32_t z
    );

VkResult halti5_dispatchIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset
    );

VkResult halti5_createImageView(
    VkDevice device,
    VkImageView imageView
    );

VkResult halti5_destroyImageView(
    VkDevice device,
    VkImageView imageView
    );

VkResult halti5_destroyBufferView(
    VkDevice device,
    VkBufferView bufferView
    );

VkResult halti5_createBufferView(
    VkDevice device,
    VkBufferView bufferView
    );

VkResult halti5_helper_convertHwTxDesc(
    __vkDevContext *devCtx,
    __vkImageView *imgv,
    __vkBufferView *bufv,
    HwTxDesc *hwTxDesc
    );

void halti5_helper_convertHwSampler(
    __vkDevContext *devCtx,
    const VkSamplerCreateInfo *createInfo,
    HwSamplerDesc *hwSamplerDesc
    );

void halti5_helper_setSamplerStates(
    __vkCommandBuffer *cmdBuf,
    uint32_t **commandBuffer,
    __vkImageView *imgv,
    uint32_t txHwRegisterIdx,
    HwTxDesc *txDesc,
    HwSamplerDesc *samplerDesc,
    uint32_t borderColorIdx,
    uint32_t hwSamplerNo,
    uint32_t shaderConfigData
    );

VkResult halti5_setViewport(
    __vkCommandBuffer *cmdBuf
    );

VkResult halti5_setScissor(
    __vkCommandBuffer *cmdBuf
    );

VkResult halti5_setDepthBias(
    __vkCommandBuffer *cmdBuf
    );

VkResult halti5_setStencilStates(
    __vkCommandBuffer *cmdBuf
    );

VkResult halti5_setBlendConstants(
    __vkCommandBuffer *cmdBuf
    );

VkResult halti5_setLineWidth(
    __vkCommandBuffer *cmdBuf
    );

VkResult halti5_setPsOutputMode(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline *pip
    );

VkResult halti5_setRenderTargets(
    __vkCommandBuffer *cmdBuf
    );

VkResult halti5_setDesriptorSets(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline *pip,
    __vkCmdBindDescSetInfo *cmdDescSetInfo
    );

VkResult halti5_setIndexBuffer(
    __vkCommandBuffer *cmdBuf
    );

VkResult halti5_setVertexBuffers(
    __vkCommandBuffer *cmdBuf
    );

VkResult halti5_setTxTileStatus(
    __vkCommandBuffer *cmdBuf,
    __vkCmdBindDescSetInfo *descSetInfo
    );

VkResult halti5_processQueryRequest(
    VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t query,
    VkBool32 beginOQ
    );

/* Helper functions */
VkResult halti5_helper_set_viewport(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    VkViewport *viewport
    );

VkResult halti5_helper_set_depthBias(
    __vkDevContext *devCtx,
    uint32_t depthFormat,
    uint32_t **commandBuffer,
    VkBool32 depthBiasEnable,
    __vkDynamicDepthBiasState *depthBiasState
    );

VkResult halti5_helper_set_stencilStates(
    __vkDevContext *devCtx,
    VkFrontFace frontFace,
    uint32_t **commandBuffer,
    __vkDynamicStencilState *stencilState,
    uint32_t stencilMode
    );

VkResult halti5_helper_set_blendConstants(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    __vkDynamicColorBlendState *blendState,
    uint32_t attachmentCount
    );

VkResult halti5_helper_set_linewidth(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    float lineWidth
    );

VkResult halti5_helper_set_psOutputMode(
    __vkDevContext *devCtx,
    uint32_t **commandBuffer,
    uint32_t oldPsOutCntl4to7,
    uint32_t newPsOutCntl4to7
    );

VkResult halti5_helper_convertHwPEDesc(
    __vkDevContext *devCtx,
    uint32_t vkFormat,
    VkBool32 is16Bit,
    HwPEDesc *hwPEDesc
    );

VkResult halti5_createSampler(
    VkDevice device,
    VkSampler sampler
    );

VkResult halti5_beginCommandBuffer(
    VkCommandBuffer commandBuffer
    );

VkResult halti5_endCommandBuffer(
    VkCommandBuffer commandBuffer
    );

VkResult halti5_setPushConstants(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline *pip
    );

VkResult halti5_setMultiGPURenderingMode(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline *pip
    );

VkResult halti5_destroyPipeline(
    VkDevice device,
    VkPipeline pipeline
    );

VkResult halti5_allocDescriptorSet(
    VkDevice device,
    VkDescriptorSet descriptorSet
    );


VkResult halti5_freeDescriptorSet(
    VkDevice device,
    VkDescriptorSet descriptorSet
    );

VkResult halti5_updateDescriptorSet(
    VkDevice device,
    VkDescriptorSet descriptorSet
    );

const char * halti5_helper_patchFuc(
    uint32_t patchFormat,
    uint32_t patchType,
    VkImageViewType viewType,
    VSC_RES_OP_BIT *pOpTypeBits,
    VSC_RES_ACT_BIT *pActBits,
    uint32_t *pSubType
    );

VkResult halti5_bindDescriptors(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    uint32_t firstSet,
    uint32_t setCount
    );

VkResult halti5_patch_pipeline(
    __vkPipeline *pip,
    __vkCmdBindDescSetInfo *descSetInfo,
    VkBool32 *instanceSwitched
    );

void halti5_pipelineBarrier(
    VkCommandBuffer commandBuffer,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags destStageMask,
    VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers
    );


void halti5_setEvent(
    VkCommandBuffer commandBuffer,
    VkEvent event,
    VkPipelineStageFlags stageMask,
    VkBool32 signal
    );

void halti5_waitEvents(
    VkCommandBuffer commandBuffer,
    uint32_t eventCount,
    const VkEvent* pEvents,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags destStageMask,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers
    );

VkResult halti5_allocateCommandBuffer(
    VkDevice device,
    VkCommandBuffer commandBuffer
    );

VkResult halti5_freeCommandBuffer(
    VkDevice device,
    VkCommandBuffer commandBuffer
    );

void halti5_bindPipeline(
    VkCommandBuffer commandBuffer,
    VkPipeline oldPipeline,
    VkPipeline newPipeline
    );


const __vkFormatToHwTxFmtInfo* halti5_helper_convertHwTxInfo(
    __vkDevContext *devCtx,
    uint32_t vkFormat
    );

VkResult halti5_helper_convertHwImgDesc(
    __vkDevContext *devCtx,
    __vkImageView *imgv,
    __vkBufferView *bufv,
    VkExtent3D *userSize,
    HwImgDesc *hwImgDesc
    );

void halti5_helper_configMSAA(
    __vkImage *img,
    uint32_t *msaa,
    uint32_t *cacheMode
    );

void halti5_helper_configTiling(
    __vkImage *img,
    uint32_t *tiling,
    uint32_t *superTile
    );

VkResult halti5_helper_convertHwBltDesc(
    VkBool32 isSource,
    uint32_t vkFormat,
    HwBLTDesc *hwBltDesc
    );

VkResult halti5_decompressTileStatus(
    __vkCommandBuffer *cmdBuf,
    uint32_t **commandBuffer,
    __vkImage *img,
    VkImageSubresourceRange* pRanges
    );

VkResult halti5_setRtTileStatus(
    __vkCommandBuffer *cmdBuf,
    uint32_t **commandBuffer,
    __vkImage *img,
    VkImageSubresourceRange* pRanges,
    uint32_t hwRtIndex
    );

#define TX_HW_DESCRIPTOR_MEM_SIZE   256

__VK_INLINE uint32_t __vk_UtilLog2inXdot8(
    uint32_t val
    )
{
    uint32_t ret = 0;
    uint32_t scale = (1 << 8);

    if (val <= 1 )
    {
        return 0;
    }

    if (!(val & 0xFF))
    {
        val >>= 8;
        ret += 8 * scale;
    }
    if (!(val & 0xF))
    {
        val >>= 4;
        ret += 4 * scale;
    }
    if (!(val & 0x3))
    {
        val >>= 2;
        ret += 2 * scale;
    }
    if (!(val & 0x1))
    {
        val >>= 1;
        ret += scale;
    }

    if (val > 1)
    {
        ret += (uint32_t)(gcoMATH_Log2((gctFLOAT)val) * scale);
    }

    return ret;
}

/*
* convert a input floating point number to an fixed point
* n.m format. The value will be clamped if out-of-range.
*/
__VK_INLINE int32_t _Float2SignedFixed(
    float x,
    int8_t truncBits,
    int8_t fracBits
    )
{
    int32_t outInteger;

    uint32_t in = *((uint32_t*)&x);

    const uint16_t fixedMask = (1 << (truncBits + fracBits)) - 1;
    const uint16_t fixedMax  = (1 << (truncBits + fracBits - 1)) - 1;
    const uint16_t fixedMin  = 1 << (truncBits + fracBits - 1);

    VkBool32   signIn = (in & 0x80000000) ? VK_TRUE : VK_FALSE;
    /* minus 7F to get signed exponent */
    int16_t  expIn  = ((in >> 23) & 0xFF) - 0x7F;
    /* There is implicit "1" before the 23 mantissa bits */
    uint32_t manIn  = (in & 0x7FFFFF) | 0x800000;

    if (expIn < -fracBits)
    {
        outInteger = 0;
    }
    else if (expIn >= truncBits - 1)
    {
        outInteger = signIn ? fixedMin : fixedMax;
    }
    else
    {
        int32_t manShift = 23 - (expIn + fracBits);
        outInteger = manIn >> manShift;
        if (signIn)
        {
            outInteger = (~outInteger + 1) & fixedMask;
        }
    }

    return outInteger;
}

VkResult halti3_copyBuffer(
    VkCommandBuffer cmdBuf,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    uint64_t copySize
    );

VkResult halti3_fillBuffer(
    VkCommandBuffer cmdBuf,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkDeviceSize size,
    uint32_t data
    );

VkResult halti3_updateBuffer(
    VkCommandBuffer cmdBuf,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkDeviceSize size,
    const uint32_t* pData
    );

VkResult halti2_clearImageWithRS(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageSubresource *subResource,
    VkClearValue *clearValue,
    VkRect2D *rect
    );

VkResult halti2_copyImageWithRS(
    VkCommandBuffer commandBuffer,
    __vkBlitRes *srcRes,
    __vkBlitRes *dstRes,
    VkBool32 rawCopy,
    VkFilter filter,
    VkBool32 oldPath
    );

void halti2_helper_convertHwSampler(
    __vkDevContext *devCtx,
    const VkSamplerCreateInfo *createInfo,
    HwSamplerDesc *hwSamplerDesc
    );

VkResult halti2_helper_convertHwTxDesc(
    __vkDevContext *devCtx,
    __vkImageView *imgv,
    __vkBufferView *bufv,
    HwTxDesc *hwTxDesc
    );

VkResult halti2_pip_emit_vsinput(
    __vkDevContext *devCtx,
    __vkPipeline *pip,
    const VkGraphicsPipelineCreateInfo * info
    );

void halti2_helper_setSamplerStates(
    __vkCommandBuffer *cmdBuf,
    uint32_t **commandBuffer,
    __vkImageView *imgv,
    uint32_t txHwRegisterIdx,
    HwTxDesc *txDesc,
    HwSamplerDesc *samplerDesc,
    uint32_t borderColorIdx,
    uint32_t hwSamplerNo,
    uint32_t shaderConfigData
    );

VkResult halti5_setMultiGpuSync(
    VkDevice device,
    uint32_t** commandBuffer,
    uint32_t * sizeInUint
    );

VkResult halti5_flushCache(
    VkDevice device,
    uint32_t **commandBuffer,
    uint32_t *sizeInUint,
    int32_t cacheMask
    );

VkBool32 halti5_tweakCopy(
    VkCommandBuffer cmdBuf,
    VkImage srcImage,
    VkBuffer destBuffer
    );

VkResult halti5_getQueryResult(
    VkDevice device,
    VkQueryPool queryPool,
    uint32_t queryIndex,
    uint64_t *retValue
    );

VkResult halti5_copyQueryResult(
    VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    VkBuffer dstBuffer,
    uint32_t queryIndex,
    VkDeviceSize dstOffset,
    VkDeviceSize dstStride,
    VkQueryResultFlags flags
    );

uint32_t halti5_computeTileStatusAddr(
    __vkDevContext *devCtx,
    __vkImage *img,
    uint32_t offset
    );

VkResult halti3_helper_convertHwTxDesc(
    __vkDevContext *devCtx,
    __vkImageView *imgv,
    __vkBufferView *bufv,
    HwTxDesc *hwTxDesc
    );

void halti3_helper_convertHwSampler(
    __vkDevContext *devCtx,
    const VkSamplerCreateInfo *createInfo,
    HwSamplerDesc *hwSamplerDesc
    );

void halti3_helper_setSamplerStates(
    __vkCommandBuffer *cmdBuf,
    uint32_t **commandBuffer,
    __vkImageView *imgv,
    uint32_t txHwRegisterIdx,
    HwTxDesc *txDesc,
    HwSamplerDesc *samplerDesc,
    uint32_t borderColorIdx,
    uint32_t hwSamplerNo,
    uint32_t shaderConfigData
    );

VkResult halti5_setDrawID(
    __vkCommandBuffer *cmdBuf,
    __vkPipeline *pip
    );

int32_t halti5_convertLocationToRenderIndex(
    gcsHINT_PTR pHints,
    uint32_t locationIndex
    );

#endif /* __gc_halti5_chip_h__ */


