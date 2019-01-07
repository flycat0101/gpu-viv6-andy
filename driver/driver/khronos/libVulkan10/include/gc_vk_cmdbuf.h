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


#ifndef __gc_vk_cmdbuf_h__
#define __gc_vk_cmdbuf_h__

#define __VK_MAX_COMMITS 128
#define __VK_MAX_COMMITS_POOL_COUNT 4

#define __VK_COMMANDBUFFER_SIZE             gcdCMD_BUFFER_SIZE  /* 128KB */
#define __VK_STATEBUFFER_SIZE               (__VK_COMMANDBUFFER_SIZE - 0x800) /* Command buffer size reserves some space for kernel */
#define __VK_STATEPOOL_BLOCK_NUM            256     /* CMDPOOL size is 256 * 128 KB = 32 MB */
#define __VK_STATEPOOL_SIZE                 (__VK_STATEBUFFER_SIZE * __VK_STATEPOOL_BLOCK_NUM)

#define __VK_PIPE_SELECT_DMA_SIZE \
    (2 * sizeof(uint32_t))

#if __VK_ENABLETS
#define __VK_2D_FLUSH_PIPE_DMA_SIZE(a) \
    ((((!a->database->PE_TILE_CACHE_FLUSH_FIX) && a->database->REG_BltEngine) ? 8 : 4) * sizeof(uint32_t))
#else
#define __VK_2D_FLUSH_PIPE_DMA_SIZE(a) \
    (2 * sizeof(uint32_t))
#endif

#if __VK_ENABLETS
#define __VK_3D_FLUSH_PIPE_DMA_SIZE(a) \
    ((((!a->database->PE_TILE_CACHE_FLUSH_FIX) && a->database->REG_BltEngine) ? 12 : 8) * sizeof(uint32_t))
#else
#define __VK_3D_FLUSH_PIPE_DMA_SIZE(a) \
    (6 * sizeof(uint32_t))
#endif

#define __VK_CMD_BEGIN_STATE_BATCH(a, b, c, d) \
    { \
        *(a)++ \
            = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE, LOAD_STATE) \
            | gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, (b))        \
            | gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT, (c))        \
            | gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, FLOAT, (d));       \
    }

#define __VK_CMD_HW_FENCE_WAIT(_Delay_) \
    (uint32_t)0x78000000 \
        | \
        ((_Delay_) & 0xFFFF) \

#define __VK_CMD_HW_NOP() \
    (uint32_t) 0x18C0FFEE

typedef enum
{
    __VK_CMDBUF_STATE_FREE = 0,
    __VK_CMDBUF_STATE_RECORDING,
    __VK_CMDBUF_STATE_EXECUTABLE,
    __VK_CMDBUF_STATE_EXECUTION_PENDING,
    __VK_CMDBUF_STATE_RESET_REQUIRED,
    __VK_CMDBUF_STATE_STATE_COUNT
} __vkCommandBufferState;


typedef struct __vkSecondaryCmdBuferInfoRec
{
    /* Seondary renderPass handle and subPass index */
    VkRenderPass renderPass;
    uint32_t subPass;

    /* Pointer to last primary buffer that submitted this secondary buffer */
    VkCommandBuffer prevPrimarySubmission;
} __vkSecondaryCmdBuferInfo;

typedef struct __vkCmdExecuteCommandsInfoRec
{
    VkCommandBuffer primary;        // Primary command buffer
    VkCommandBuffer secondary;      // Secondary command buffer
    uint32_t stateBufferListIndex;  // Primary state buffer list index where secondary is inserted
    uint32_t stateBufferOffset;     // Primary state buffer byte offset where secondary is inserted
    uint32_t stateBufferPipe;       // Primary state buffer current pipe;

    struct __vkCmdExecuteCommandsInfoRec *next;
} __vkCmdExecuteCommandsInfo;

typedef struct __vkStateBufferRec
{
    uint8_t *bufStart;          // Beginning of state buffer
    uint32_t bufSize;
    uint32_t bufOffset;
    uint32_t bufPipe;
    uint32_t secBufCount;       // Count of secondary buffers inserted in this state buffer
    struct __vkStateBufferRec *next;
} __vkStateBuffer;

typedef struct __vkCommandPoolRec
{
    __vkObject obj; /* Must be the first field */

    uint32_t threadId;          // Current owner of command pool
    uint32_t queueFamilyIndex;

    VkAllocationCallbacks memCb;

    VkCommandPoolCreateFlags flags;

    /* CommandPool specific fields */
    uint32_t numOfCmdBuffers;           // Number of command buffers
    uint32_t numOfStateBuffers;         // Number of state buffers
    uint32_t sizeOfEachStateBuffer;     // Size of each state buffer

    VkCommandBuffer vkCmdBufferList;
} __vkCommandPool;

#define __VK_MAX_DESCRIPTOR_SETS            4
#define __VK_MAX_PUSHCONST_SIZE_IN_UINT     32
#define __VK_MAX_UNIFORM_BUFFER_DYNAMIC     8
#define __VK_MAX_STORAGE_BUFFER_DYNAMIC     4

typedef struct __vkCmdBindDescSetInfoRec
{
    __vkDescriptorSet *descSets[__VK_MAX_DESCRIPTOR_SETS];
    uint32_t  dynamicOffsets[__VK_MAX_DESCRIPTOR_SETS][__VK_MAX_UNIFORM_BUFFER_DYNAMIC + __VK_MAX_STORAGE_BUFFER_DYNAMIC];
    uint32_t  dirtyMask;
    uint32_t  patchMask;
}__vkCmdBindDescSetInfo;

enum
{
    __VK_CMDBUF_BINDNIGINFO_PIPELINE_GRAPHICS_DIRTY = 1 << 0,
    __VK_CMDBUF_BINDINGINFO_PIPELINE_COMPUTE_DIRTY  = 1 << 1,
};

typedef struct __vkRenderPassInfoRec
{
    __vkRenderPass * rdp;
    __vkRenderSubPassInfo *subPassInfo;
    __vkFramebuffer *fb;
    uint32_t subPassIndex;
    struct __vkRenderPassInfoRec *next;
} __vkRenderPassInfo;

typedef union
{
    struct
    {
        VkBool32 indexDraw;
        VkBool32 indirectDraw;
        /* direct draw only */
        uint32_t firstVertex;
        uint32_t vertexCount;
        uint32_t firstInstance;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t indexCount;
        /* indirect draw only */
        VkBuffer buffer;
        VkDeviceSize offset;
        uint32_t count;
        uint32_t stride;
    } draw;

    struct
    {
        VkBool32 indirectCompute;
        /* direct compute */
        uint32_t x;
        uint32_t y;
        uint32_t z;
        /* indirect compute */
        VkBuffer buffer;
        VkDeviceSize offset;
    } compute;
} __vkDrawComputeCmdParams;

typedef struct __vkCmdBindInfoRec
{
    struct
    {
        __vkPipeline * graphics;
        __vkPipeline * compute;
        uint32_t dirty;
        VkPipelineBindPoint activePipeline;
    } pipeline;

    struct
    {
        __vkCmdBindDescSetInfo graphics;
        __vkCmdBindDescSetInfo compute;
    } bindDescSet;

    struct
    {
        __vkRenderPass * rdp;
        __vkRenderSubPassInfo *subPass;
        __vkFramebuffer *fb;
        VkSubpassContents subPassContent;
        VkBool32 dirty;
    } renderPass;

    struct
    {
        __vkDynamicViewportState viewport;
        __vkDynamicScissorState scissor;
        __vkDynamicStencilState stencil;
        __vkDynamicDepthBiasState depthBias;
        __vkDynamicColorBlendState blend;
        __vkDynamicLineWidthState  lineWidth;
        uint32_t dirtyMask;
    } dynamicStates;

    struct
    {
        uint32_t firstIndex;
        VkBuffer buffer;
        VkDeviceSize offset;
        VkIndexType indexType;
        VkBool32 dirty;
    } indexBuffer;

    struct
    {
        uint32_t firstBinding;
        uint32_t bindingCount;
        VkBuffer buffers[__VK_MAX_VERTEX_BUFFER_BINDINGS];
        VkDeviceSize offsets[__VK_MAX_VERTEX_BUFFER_BINDINGS];
        uint32_t firstInstance;
        uint32_t dirtyBits;
    } vertexBuffers;

    struct
    {
        uint32_t  values[__VK_MAX_PUSHCONST_SIZE_IN_UINT];
        uint32_t dirtyMask;
    } pushConstants;

    VkBool32 oqEnable;
} __vkCmdBindInfo;

typedef struct __vkScratchMemRec
{
    __vkDeviceMemory *memory;
    struct __vkScratchMemRec *next;
} __vkScratchMem;

enum
{
    __VK_CMDUBF_ACTIVE_PIPELINE_NONE    = 0,
    __VK_CMDBUF_ACTIVE_PIPELINE_DRAW    = 1,
    __VK_CMDBUF_ACTIVE_PIPELINE_COMPUTE = 2,
};

#define __VK_CMDBUF_SCRATCH_BUFFER_SIZE 2048

typedef struct __vkCommandBufferRec
{
    __vkObject obj; /* Must be the first field */

    __vkDevContext *devCtx;

    uint32_t threadId;

    /* GCHAL Interface (needed when command buffers recorded from different threads) */
    gcoHAL hal;

    /* Back pointer to the commandPool */
    VkCommandPool commandPool;

    /* CommandBuffer specific fields */
    uint32_t bufWriteRequestCount;

    VkBool32 using2D;
    VkBool32 using3D;

    /* Primary / Secondary flag */
    VkCommandBufferLevel level;

    /* Command buffer state */
    __vkCommandBufferState state;

    /* Usage flags set at begin record */
    uint32_t usage;

    /* Internal state buffer list */
    __vkStateBuffer *stateBufferList;
    __vkStateBuffer *stateBufferTail;
    uint32_t lastStateBufferIndex;

    /* Internal command buffer execute commands info list */
    __vkCmdExecuteCommandsInfo *executeList;    // Only valid for primary buffers
    __vkCmdExecuteCommandsInfo *executeTail;

    __vkSecondaryCmdBuferInfo secondaryInfo;    // Only valid for secondary buffers

    __vkCmdBindInfo bindInfo;
    /* Scratch command buffer server for one operation (draw/compute/copy/resolve) command accumulation */
    uint32_t scratchCmdBuffer[__VK_CMDBUF_SCRATCH_BUFFER_SIZE];
    uint32_t curScrachBufIndex;

    __vkScratchMem *scratchHead;

    __vkRenderPassInfo *renderPassInfo;

#if __VK_RESOURCE_INFO
#if __VK_RESOURCE_SAVE_TGA
    VkBool32 skipSaveTGA;
#endif
    __vkCmdResNode *inputs;
    __vkCmdResNode *outputs;
#endif

    struct __vkCommandBufferRec *next;

    void *chipPriv;

    /* Error status */
    VkResult result;
    /* sequence ID, only for draw and compute operation */
    uint32_t sequenceID;

    gceMULTI_GPU_RENDERING_MODE gpuRenderingMode;

} __vkCommandBuffer;

typedef struct __vk_CommitInfoRec
{
    uint8_t *stateStart;
    uint32_t stateSize;
    uint32_t statePipe;
} __vk_CommitInfo;

void __vk_CmdAquireBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t count,
    uint32_t **cmdBuffer
    );

void __vk_CmdReleaseBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t count
    );

gcmINLINE static void
__vkCmdLoadSingleHWState(
    uint32_t **states,
    uint32_t address,
    VkBool32 fixedPoint,
    const uint32_t data
    )
{
    {*(*states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((address)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) ((1)) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) ((fixedPoint)) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));};

    *(*states)++ = data;
}

gcmINLINE static void
__vkCmdLoadBatchHWStates(
    uint32_t **states,
    uint32_t address,
    VkBool32 fixedPoint,
    uint32_t count,
    const void *data
    )
{
    {*(*states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | (((gctUINT32) ((gctUINT32) ((address)) & ((gctUINT32) ((((1 ?
 15:0) - (0 ?
 15:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:0) - (0 ?
 15:0) + 1))))))) << (0 ?
 15:0))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | (((gctUINT32) ((gctUINT32) ((count)) & ((gctUINT32) ((((1 ?
 25:16) - (0 ?
 25:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 25:16) - (0 ?
 25:16) + 1))))))) << (0 ?
 25:16))) | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:26) - (0 ?
 26:26) + 1))))))) << (0 ?
 26:26))) | (((gctUINT32) ((gctUINT32) ((fixedPoint)) & ((gctUINT32) ((((1 ?
 26:26) - (0 ?
 26:26) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:26) - (0 ? 26:26) + 1))))))) << (0 ? 26:26)));};


    gcoOS_MemCopy(*states, data, count * sizeof(uint32_t));
    *states += count;

    if (count % 2 == 0)
    {
        *(*states)++ = 0xdeadbeef;
    }
}

gcmINLINE static void
__vkCmdLoadFlush2DHWStates(
#if __VK_ENABLETS
    VkBool32 bltTileCache,
#endif
    uint32_t **states
    )
{
#if __VK_ENABLETS
    /* Flush tile cache */
    if (bltTileCache)
    {
        __vkCmdLoadSingleHWState(states, 0x502E, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );

        __vkCmdLoadSingleHWState(states, 0x502B, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );

        __vkCmdLoadSingleHWState(states, 0x502E, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
    }
    else
    {
        __vkCmdLoadSingleHWState(states, 0x0594, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
    }
#endif

    /* Flush pipe. */
    __vkCmdLoadSingleHWState(states, 0x0E03, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
        );
}

gcmINLINE static void
__vkCmdLoadFlush3DHWStates(
#if __VK_ENABLETS
    VkBool32 bltTileCache,
#endif
    uint32_t **states
    )
{
#if __VK_ENABLETS
    /* Flush tile cache */
    if (bltTileCache)
    {
        __vkCmdLoadSingleHWState(states, 0x502E, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );

        __vkCmdLoadSingleHWState(states, 0x502B, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );

        __vkCmdLoadSingleHWState(states, 0x502E, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
    }
    else
    {
        __vkCmdLoadSingleHWState(states, 0x0594, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
            );
    }
#endif

    /* Flush pipe. */
    __vkCmdLoadSingleHWState(states, 0x0E03, VK_FALSE,
        ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 0:0) - (0 ?
 0:0) + 1))))))) << (0 ?
 0:0))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 0:0) - (0 ?
 0:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 0:0) - (0 ? 0:0) + 1))))))) << (0 ? 0:0)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:1) - (0 ?
 1:1) + 1))))))) << (0 ?
 1:1))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 1:1) - (0 ?
 1:1) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:1) - (0 ? 1:1) + 1))))))) << (0 ? 1:1)))
        | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:2) - (0 ?
 2:2) + 1))))))) << (0 ?
 2:2))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 2:2) - (0 ?
 2:2) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:2) - (0 ? 2:2) + 1))))))) << (0 ? 2:2)))
        );

#if __VK_ENABLETS
    if (bltTileCache)
    {
        /* Insert a semaphore stall. */
        __vkCmdLoadSingleHWState(states, 0x0E02, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
            );

        *(*states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
        *(*states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x10 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));
    }
    else
#endif
    {
        /* Insert a semaphore stall. */
        __vkCmdLoadSingleHWState(states, 0x0E02, VK_FALSE,
            ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)))
            );

        *(*states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 31:27) - (0 ?
 31:27) + 1))))))) << (0 ?
 31:27))) | (((gctUINT32) (0x09 & ((gctUINT32) ((((1 ?
 31:27) - (0 ?
 31:27) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 31:27) - (0 ? 31:27) + 1))))))) << (0 ? 31:27)));
        *(*states)++ = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:0) - (0 ?
 4:0) + 1))))))) << (0 ?
 4:0))) | (((gctUINT32) (0x01 & ((gctUINT32) ((((1 ?
 4:0) - (0 ?
 4:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:0) - (0 ? 4:0) + 1))))))) << (0 ? 4:0)))
            | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:8) - (0 ?
 12:8) + 1))))))) << (0 ?
 12:8))) | (((gctUINT32) (0x07 & ((gctUINT32) ((((1 ?
 12:8) - (0 ?
 12:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:8) - (0 ? 12:8) + 1))))))) << (0 ? 12:8)));
    }
}

void
__vk_CmdAquireBufferAndLoadHWStates(
    VkCommandBuffer commandBuffer,
    uint32_t address,
    VkBool32 fixedPoint,
    uint32_t count,
    void *data
    );

VkResult __vk_CommitSubmitFence(VkQueue queue, VkFence fence);

VkResult __vk_InsertSemaphoreWaits(
    VkQueue queue,
    const VkSemaphore* pSemaphores,
    uint32_t semaphoreCount
    );

VkResult
__vk_CommitStateBuffers(
    VkQueue queue,
    __vk_CommitInfo** pCommits,
    uint32_t curPoolIndex,
    uint32_t commitCount
    );

__vkScratchMem* __vkGetScratchMem(
    __vkCommandBuffer *pCmdBuf,
    VkDeviceSize size
    );
#endif /* __gc_vk_cmdbuf_h__ */


