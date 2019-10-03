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


#ifndef __gc_vk_pipeline_h__
#define __gc_vk_pipeline_h__

enum __VK_FACE
{
    __VK_FACE_FRONT = 0,
    __VK_FACE_BACK  = 1,
    __VK_FACE_NUMBER = 2,
};

typedef struct
{
    SHADER_HANDLE vscHandle;
    gcsATOM_PTR   refCount;
} vkShader, *vkShader_HANDLE;

typedef struct __vkShaderModuleRec
{
    __vkObject obj; /* Must be the first field */

    /* ShaderModule specific fields */
    size_t codeSize;
    uint32_t* pCode;
    void * funcTable;

} __vkShaderModule;

typedef struct __vkShaderRec
{
    __vkObject obj; /* Must be the first field */

    /* Shader specific fields */

    __vkShaderModule *shaderMod;

} __vkShader;

typedef struct __VK_ATTRIB_ALIGN(8) __vkPipelineCachePublicHeadRec
{
    uint32_t headBytes;
    uint32_t version;
    uint32_t vendorID;
    uint32_t deviceID;
    uint8_t  UUID[VK_UUID_SIZE];
} __vkPipelineCachePublicHead;

typedef struct __VK_ATTRIB_ALIGN(8) __vkPipelineCachePrivateHeadRec
{
    uint32_t headBytes;
    uint32_t bigEndian;
    uint32_t osInfo;
    uint32_t patchID;       /* App name when initialize the cache,
                            ** can be moved to be per-module attrib later.
                            */
    uint64_t totalBytes;    /* Total size of the pipeline cache data */
    uint32_t numModules;    /* number of modules contained in this cache */
    uint32_t version;       /* Vendor private version control */
    uint8_t  drvVersion[32];
} __vkPipelineCachePrivateHead;

typedef struct __VK_ATTRIB_ALIGN(8) __vkModuleCacheHeadRec
{
    uint32_t headBytes;
    uint32_t magic;
    uint32_t stage;
    uint32_t patchCase;     /* Patch case depends on non-shader states. */
    uint32_t binBytes;      /* Original binary size from compiler */
    uint32_t alignBytes;    /* Aligned binary size for save */
    uint8_t  hashKey[16];   /* MD5 digest of the shader */
} __vkModuleCacheHead;

typedef struct __vkModuleCacheEntryRec
{
    __vkModuleCacheHead head;
    vkShader_HANDLE     handle;
} __vkModuleCacheEntry;


typedef struct __vkPipelineCacheRec
{
    __vkObject obj; /* Must be the first field */

    gctPOINTER mutex;

    /* PipelineCache specific fields */
    VkAllocationCallbacks memCb;

    __vkPipelineCachePublicHead  *publicHead;
    __vkPipelineCachePrivateHead *privateHead;

    uint32_t numModules;
    size_t   totalBytes;
    __vkUtilsHash *moduleHash;
} __vkPipelineCache;

typedef struct __vkPipelineLayoutRec
{
    __vkObject obj; /* Must be the first field */

    uint32_t descSetLayoutCount;
    __vkDescriptorSetLayout **descSetLayout;

    uint32_t *dynamic_index;
    uint32_t total_dynamic_index;

    uint32_t pushConstantRangeCount;
    VkPushConstantRange*  pushConstantRanges;

} __vkPipelineLayout;

#define __VK_MAX_VB_BINDINGS    32
#define __VK_MAX_VERTEX_ATTRIBS 32

enum __VK_DYNAMIC_STATES
{
    __VK_DYNAMIC_STATE_VIEWPORT_BIT                  = (1 << 0),
    __VK_DYNAMIC_STATE_SCISSOR_BIT                   = (1 << 1),
    __VK_DYNAMIC_STATE_LINE_WIDTH_BIT                = (1 << 2),
    __VK_DYNAMIC_STATE_DEPTH_BIAS_BIT                = (1 << 3),
    __VK_DYNAMIC_STATE_BLEND_CONSTANTS_BIT           = (1 << 4),
    __VK_DYNAMIC_STATE_DEPTH_BOUNDS_BIT              = (1 << 5),
    __VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK_BIT      = (1 << 6),
    __VK_DYNAMIC_STATE_STENCIL_WRITE_MASK_BIT        = (1 << 7),
    __VK_DYNAMIC_STATE_STENCIL_REFERENCE_BIT         = (1 << 8),

};

#define __VK_DYNAMIC_STATE_STENCIL_BITS \
    (__VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK_BIT |\
     __VK_DYNAMIC_STATE_STENCIL_WRITE_MASK_BIT   |\
     __VK_DYNAMIC_STATE_STENCIL_REFERENCE_BIT)


#define __VK_MAX_VIEWPORTS 16
#define __VK_MAX_SCISSORS 16

typedef struct __vkDynamicViewportStateRec
{
    uint32_t viewportCount;
    VkViewport viewports[__VK_MAX_VIEWPORTS];
}__vkDynamicViewportState;

typedef struct __vkDynamicScissorStateRec
{
    uint32_t scissorCount;
    VkRect2D scissors[__VK_MAX_SCISSORS];
} __vkDynamicScissorState;

typedef struct __vkDynamicDepthBiasStateRec
{
    float   depthBiasConstantFactor;
    float   depthBiasClamp;
    float   depthBiasSlopeFactor;
} __vkDynamicDepthBiasState;

typedef struct __vkDynamicStencilStateRec
{
    uint32_t compareMask[__VK_FACE_NUMBER];
    uint32_t reference[__VK_FACE_NUMBER];
    uint32_t writeMask[__VK_FACE_NUMBER];
} __vkDynamicStencilState;

typedef struct __vkDynamicColorBlendStateRec
{
    float blendConstants[4];
} __vkDynamicColorBlendState;

typedef struct __vkDynamicLineWidthStateRec
{
    float width;
}__vkDynamicLineWidthState;


#define __VK_PIPELINE_CMDBUFFER_MAXSIZE     512
#define __VK_MAX_VERTEX_BUFFER_BINDINGS     32

enum
{
    __VK_PIPELINE_TYPE_INVALID  = 0,
    __VK_PIPELINE_TYPE_GRAPHICS = 1,
    __VK_PIPELINE_TYPE_COMPUTE  = 2,
};

typedef struct __vkPipelineRec
{
    __vkObject obj; /* Must be the first field */

    uint32_t type;

    __vkDevContext *devCtx;

    VkAllocationCallbacks memCb;

    __vkPipelineCache *cache;

    VkPipelineCreateFlags flags;

    __vkPipelineLayout *pipelineLayout;
    __vkRenderPass *renderPass;
    uint32_t subPass;

    VkPrimitiveTopology topology;
    VkBool32 primitiveRestartEnable;

    VkBool32 msaaEnabled;

    VkFlags dynamicStates;
    __vkDynamicScissorState scissorState;
    __vkDynamicViewportState viewportState;

    VkBool32 rasterDiscard;
    VkBool32 depthBiasEnable;
    VkFrontFace frontFace;

    VkPipelineDepthStencilStateCreateInfo dsInfo;

    uint32_t blendAttachmentCount;
    VkPipelineColorBlendAttachmentState *blendAttachments;

    uint32_t patchControlPoints;

    void *chipPriv;

} __vkPipeline;

typedef VkResult (*VK_DRAW_INDEXED_FUNC)(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
typedef struct __vkSplitPatchListParamsRec
{
    uint32_t indexSize;
    uint32_t indexBufferSize;
    uint32_t bytesPerPatch;
    uint32_t splitBytesPerPatch;
    uint32_t alignBytes;
}__vkSplitPathListParams;


#endif /* __gc_vk_pipeline_h__ */


