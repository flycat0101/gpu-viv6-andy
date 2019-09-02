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


#ifndef __GC_VX_BINARY_H__
#define __GC_VX_BINARY_H__

#define SH_COMMAND_ALIGN_SIZE             256
#define VX_MAX_LAYER_NAME_LENGTH          64
#define VX_MAX_NAME_LEGTH                 64
#define NN_MAX_DIMENSION                  4
#define VX_MAX_NN_INOUT_PARAM_COUNT       1024
#define VX_MAX_SH_OPERATION_STATE_SIZE    0xE00
#define VX_MAX_NNTP_OPERATION_STATE_SIZE  0x200
#define VX_MAX_INITIALIZE_COMMAND_SIZE    0xC00
#define VX_MAX_SC_OPERATION_STATE_SIZE    0x400
#define VX_MAX_WAIT_STATE_SIZE            0x200

enum
{
    VX_BINARY_PATCH_TRANSFORMATION_ORIGINAL,
    VX_BINARY_PATCH_TRANSFORMATION_RIGHT_SHIFT_6,
};

enum
{
    VX_BINARY_PATCH_TYPE_STATE     = 0, /* patch state stream */
    VX_BINARY_PATCH_TYPE_COMMAND   = 1, /* patch SH/NN/TP command */
};

enum
{
    VX_BINARY_BUFFER_FORMAT_FP32   = 0, /* A float type of buffer data */
    VX_BINARY_BUFFER_FORMAT_FP16   = 1, /* A half float type of buffer data */
    VX_BINARY_BUFFER_FORMAT_UINT8  = 2, /* A 8 bit unsigned integer type of buffer data */
    VX_BINARY_BUFFER_FORMAT_INT8   = 3, /* A 8 bit signed integer type of buffer data */
    VX_BINARY_BUFFER_FORMAT_UINT16 = 4, /* A 16 bit unsigned integer type of buffer data */
    VX_BINARY_BUFFER_FORMAT_INT16  = 5, /* A 16 signed integer type of buffer data */
};

enum
{
    VX_BINARY_BUFFER_TYPE_TENSOR = 0, /* A tensor data type of buffer data */
    VX_BINARY_BUFFER_TYPE_IMAGE  = 1, /* A image data type of buffer data */
    VX_BINARY_BUFFER_TYPE_ARRAY  = 2, /* A array data type of buffer data */
    VX_BINARY_BUFFER_TYPE_SCALAR = 3, /* A scalar data type of buffer data */
    VX_BINARY_BUFFER_TYPE_MAX,
};

enum
{
    VX_BINARY_BUFFER_QUANT_FORMAT_NONE  = 0, /* Not quantized format */
    VX_BINARY_BUFFER_QUANT_FORMAT_DFP   = 1, /* The data is quantized with dynamic fixed point */
    VX_BINARY_BUFFER_QUANT_FORMAT_ASYMM = 2, /* The data is quantized with TF asymmetric format */
};

enum
{
    VX_BINARY_SOURCE_COMMAND              = 0, /* SH instrction/NN command /TP command buffer */
    VX_BINARY_SOURCE_INPUT                = 1, /* graph input buffer */
    VX_BINARY_SOURCE_OUTPUT               = 2, /* graph output buffer */
    VX_BINARY_SOURCE_MEMORY_POOL          = 3, /* virtual pool buffer */
    VX_BINARY_SOURCE_AXI_SRAM             = 4, /* AXI SRAM memory */
    VX_BINARY_SOURCE_MISC_DYNAMIC_GENERIC = 5, /* variable length of buffer not any of above */
    VX_BINARY_SOURCE_MISC_DYNAMIC_INPUT   = 6, /* source is misc dynamic input data */
    VX_BINARY_SOURCE_MISC_DYNAMIC_OUTPUT  = 7, /* source is misc dynamic output data */
    VX_BINARY_SOURCE_VIP_SRAM             = 8, /* VIP SRAM memory */
    VX_BINARY_SOURCE_MAX,
};

enum
{
    VX_BINARY_DATA_FORMAT_FP32   = 0, /* A float type of buffer data */
    VX_BINARY_DATA_FORMAT_FP16   = 1, /* A half float type of buffer data */
    VX_BINARY_DATA_FORMAT_FP64   = 2, /* A double float type of  data */
    VX_BINARY_DATA_FORMAT_UINT8  = 3, /* A 8 bit unsigned integer type of buffer data */
    VX_BINARY_DATA_FORMAT_INT8   = 4, /* A 8 bit signed integer type of buffer data */
    VX_BINARY_DATA_FORMAT_UINT16 = 5, /* A 16 bit unsigned integer type of buffer data */
    VX_BINARY_DATA_FORMAT_INT16  = 6, /* A 16 signed integer type of buffer data */
    VX_BINARY_DATA_FORMAT_CHAR   = 7, /* A char type of  data */
    VX_BINARY_DATA_FORMAT_INT32  = 8, /* A 32 bit signed integer type of  data */
    VX_BINARY_DATA_FORMAT_UINT32 = 9, /* A 32 bit unsigned integer type of  data */
    VX_BINARY_DATA_FORMAT_INT64  = 10,/* A 64 bit signed integer type of  data */
    VX_BINARY_DATA_FORMAT_UINT64 = 11,/* A 64 bit unsigned integer type of  data */
};

enum
{
    VX_BINARY_LOAD_STATUS_NONE = 0, /* not binary */
    VX_BINARY_LOAD_STATUS_INIT = 1, /* binary init */
    VX_BINARY_LOAD_STATUS_RUN = 2, /* binary running */
};

typedef enum _vx_binary_sw_operation_type_e
{
    VX_BINARY_SW_OPERATION_NONE        = 0,
    VX_BINARY_SW_OPERATION_RPN         = 1,
}
vx_binary_sw_operation_type_e;

typedef enum _vx_binary_operation_target_e
{
    VX_BINARY_OPERATION_TYPE_NONE = 0, /* invalid operation */
    VX_BINARY_OPERATION_TYPE_SH   = 1, /* PPU operation */
    VX_BINARY_OPERATION_TYPE_NN   = 2, /* NN operation */
    VX_BINARY_OPERATION_TYPE_TP   = 3, /* TP operation */
    VX_BINARY_OPERATION_TYPE_SW   = 4, /* CPU operation */
    VX_BINARY_OPERATION_TYPE_SC   = 5, /* Scaler operation */
    VX_BINARY_OPERATION_TYPE_END  = 0xFFFE, /* end of commands */
    VX_BINARY_OPERATION_TYPE_INIT = 0xFFFF, /* global init command. added in v1.1 */
}
vx_binary_operation_target_e;

typedef struct _vx_binary_segment_base_s
{
    vx_uint32           segmentType;
}
vx_binary_segment_base_s;

/**************CPU Operation Layer Define Start**********/
typedef struct _vx_binary_layer_buffer_s
{
    vx_uint8_ptr                            buffer;
    vx_uint32                               dimCount;
    vx_uint32                               dims[NN_MAX_DIMENSION];
    vx_enum                                 dataFormat;
    vx_enum                                 dataType;
    vx_enum                                 quantFormat;
    vx_int32                                fixPointZeroPoint;
    vx_float32                              tfScale;
}
vx_binary_layer_buffer_s, *vx_binary_layer_buffer;

typedef struct _vx_binary_nn_layer_RPN_s
{
    vx_binary_segment_base_s                base;

    vx_binary_layer_buffer_s                score;
    vx_binary_layer_buffer_s                bbox;
    vx_binary_layer_buffer_s                anchor;
    vx_binary_layer_buffer_s                img_info;
    vx_binary_layer_buffer_s                roi_output;
    vx_binary_layer_buffer_s                score_output;
    vx_binary_layer_buffer_s                feature_stride;
    vx_binary_layer_buffer_s                min_size;
    vx_binary_layer_buffer_s                pre_nms_topn;
    vx_binary_layer_buffer_s                post_nms_topn;
    vx_binary_layer_buffer_s                nms_thresh;
}
vx_binary_nn_layer_RPN_s;



/**************CPU Operation Layer Define End**********/

typedef struct _vx_binary_feature_database_s
{
    vx_uint32 hi_reorder_fix:1;  /* gcFEATURE_BIT_HI_REORDER_FIX */
    vx_uint32 ocb_counter:1;     /* gcFEATURE_BIT_OCB_COUNTER */
    vx_uint32 reserved:30;       /* reserved bits */

    vx_uint32 num_pixel_pipes;   /* gcFEATURE_VALUE_NumPixelPipes */

    vx_uint32 vsi_reserved[14];  /* reserved for  verisilicon */

    vx_uint32 customer_reserved[48]; /* reserved for customer */
}
vx_binary_feature_database_s;

typedef struct _vx_binary_segment_s
{
    vx_binary_segment_base_s                *base;

    vx_uint32                               statesStartPos;
    vx_uint32                               statesSize;
    vx_uint32                               startOperationIndex;
    vx_uint32                               endOperationIndex;

    vx_uint32                               layerId;
    vx_bool                                 isSWSegment;

    /* nn layer dump */
    vx_uint8_ptr                            outputlogical[VX_MAX_OPERTAION_INPUTS_OUTPUTS];
    vx_uint32                               outputSize[VX_MAX_OPERTAION_INPUTS_OUTPUTS];
    vx_uint32                               outputCount;
}
vx_binary_segment_s;

typedef struct _vx_binary_header_s
{
    char                                    magic[4]; /* Magic no: 'V', 'P', 'M', 'N'. */
    vx_uint32                               version;
    vx_uint32                               target;
    char                                    networkName[VX_MAX_NAME_LEGTH];
    vx_uint32                               layerCount;
    vx_uint32                               operationCount;
    vx_uint32                               inputCount;
    vx_uint32                               outputCount;
    vx_binary_feature_database_s            featureDB;
}
vx_binary_header_s;

typedef struct _vx_binary_memory_pool_info_s
{
    vx_uint32                               alignedSize;
    vx_uint32                               alignement;
    vx_uint32                               memoryPoolBase;
}
vx_binary_memory_pool_info_s;

typedef struct _vx_binary_axi_sram_info_s
{
    vx_uint32                               sramBase;
    vx_uint32                               sramSize;
}
vx_binary_axi_sram_info_s;

typedef struct _vx_binary_input_table_entrance
{
    vx_uint32                               inputInfoOffset;
    vx_uint32                               inputInfoBytes;
}
vx_binary_input_table_entrance_s;

typedef struct _vx_binary_output_table_entrance
{
    vx_uint32                               outputInfoOffset;
    vx_uint32                               outputInfoBytes;
}
vx_binary_output_table_entrance_s;

typedef struct _vx_binary_layers_table_entrance
{
    vx_uint32                               layersInfoOffset;
    vx_uint32                               layersInfoBytes;
}
vx_binary_layers_table_entrance_s;

typedef struct _vx_binary_operations_table_entrance
{
    vx_uint32                               operationsOffset;
    vx_uint32                               operationsBytes;
}
vx_binary_operations_table_entrance_s;

typedef struct _vx_binary_loading_config_table_entrance
{
    vx_uint32                               loadingConfigDataTableOffset;
    vx_uint32                               loadingConfigDataTableBytes;
}
vx_binary_loading_config_table_entrance_s;

typedef struct _vx_binary_loading_config_data_entrance
{
    vx_uint32                               loadingConfigDataOffset;
    vx_uint32                               loadingConfigDataBytes;
}
vx_binary_loading_config_data_entrance_s;

typedef struct _vx_binary_nn_operation_table_entrance
{
    vx_uint32                               nnOperationDataOffset;
    vx_uint32                               nnOperationDataBytes;
}
vx_binary_nn_operation_table_entrance_s;

typedef struct _vx_binary_tf_operation_table_entrance
{
    vx_uint32                               tpOperationDataOffset;
    vx_uint32                               tpOperationDataBytes;
}
vx_binary_tf_operation_table_entrance_s;

typedef struct _vx_binary_sh_operation_table_entrance
{
    vx_uint32                               shaderOperationDataOffset;
    vx_uint32                               shaderOperationDataBytes;
}
vx_binary_sh_operation_table_entrance_s;

typedef struct _vx_binary_patch_table_entrance
{
    vx_uint32                               patchDataOffset;
    vx_uint32                               patchDataBytes;
}
vx_binary_patch_table_entrance_s;

typedef struct _vx_binary_layer_parameter_entrance_s
{
    vx_uint32                               layerParamOffset;
    vx_uint32                               layerParamBytes;
}
vx_binary_layer_parameter_entrance_s;

typedef struct _vx_binary_sw_operation_table_entrance
{
    vx_uint32                               swOperationDataOffset;
    vx_uint32                               swOperationDataBytes;
}
vx_binary_sw_operation_table_entrance_s;

typedef struct _vx_binary_entrance_info_s
{
    vx_binary_input_table_entrance_s          inputEntr;
    vx_binary_output_table_entrance_s         outputEntr;
    vx_binary_layers_table_entrance_s         layersEntr;
    vx_binary_operations_table_entrance_s     operationsEntr;
    vx_binary_loading_config_table_entrance_s loadingConfigDataTableEntr;
    vx_binary_loading_config_data_entrance_s  loadingConfigDataEntr;
    vx_binary_nn_operation_table_entrance_s   nnOperationsEntr;
    vx_binary_tf_operation_table_entrance_s   tpOperationsEntr;
    vx_binary_sh_operation_table_entrance_s   shOperationsEntr;
    vx_binary_patch_table_entrance_s          patchsEntr;
    vx_binary_layer_parameter_entrance_s      layerParamEntr;
    vx_binary_sw_operation_table_entrance_s   swOperationsEntr;
}
vx_binary_entrance_info_s;

typedef struct _vx_binary_layer_info_s
{
    vx_char                                layerName[VX_MAX_LAYER_NAME_LENGTH];
    vx_uint32                              layerId;
    vx_uint32                              operationCount;
}
vx_binary_layers_info_s;

typedef struct _vx_binary_operation_info_s
{
    vx_enum                                operationType;
    vx_uint32                              operationIndex;
    vx_uint32                              layerId;
    vx_uint32                              stateLCDTIndex;
    vx_uint32                              indexOfFirstPatch;
    vx_uint32                              counterOfPatches;
}
vx_binary_operation_info_s;

typedef struct _vx_binary_nn_operation_info_s
{
    vx_uint8                               nnCmd[NNE_COMMAND_SIZE];
}
vx_binary_nn_operation_info_s;

typedef struct _vx_binary_tp_operation_info_s
{
    vx_uint8                               tpCmd[TP_COMMAND_SIZE];
}
vx_binary_tp_operation_info_s;

typedef struct _vx_binary_sh_operation_info_s
{
    vx_uint32                              instructionCmdLCDTIndex;
}
vx_binary_sh_operation_info_s;

typedef struct _vx_binary_sw_operation_info_s
{
    vx_uint32                              swOperationType;
}
vx_binary_sw_operation_info_s;

typedef struct _vx_binary_patch_info_s
{
    vx_uint32                              type;
    vx_uint32                              offset;
    vx_uint32                              sourceType;
    vx_int32                               index;
    vx_uint32                              originalBaseAddress;
    vx_uint32                              transformation;
}
vx_binary_patch_info_s;

typedef struct _vx_binary_loadingdata_table_info_s
{
    vx_uint32                               loadingDataOffset;
    vx_uint32                               loadingDataSize;
}
vx_binary_loadingdata_table_info_s;

typedef struct _vx_binary_input_output_info_s
{
    vx_uint32                               dimCount;
    vx_uint32                               dims[NN_MAX_DIMENSION];
    vx_enum                                 dataFormat;
    vx_enum                                 dataType;
    vx_enum                                 quantFormat;
    vx_int32                                fixedPointPos;
    vx_float32                              tfScale;
    vx_int32                                tfZeroPoint;
}
vx_binary_input_output_info_s;

typedef struct _vx_binary_layer_parameter_s
{
    vx_char                                 paramName[16];
    vx_uint32                               dimCount;
    vx_uint32                               dims[NN_MAX_DIMENSION];
    vx_enum                                 dataFormat;
    vx_enum                                 dataType;
    vx_enum                                 quantFormat;
    vx_int32                                fixPointZeroPoint;
    vx_float32                              tfScale;
    vx_int32                                index;
    vx_uint32                               addressOffset;
    vx_uint32                               sourceType;
}
vx_binary_layer_parameter_s;

typedef struct _vx_binary_input_node_info_s
{
    vx_node                                 node;
    vx_uint32                               inputPhysical[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_uint32                               paramIndex[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_uint32                               count;
}
vx_binary_input_node_info_s;

typedef struct _vx_binary_save_s
{
    FILE *                                   binarySaveFile;

    vx_uint32                                inputParamCount;
    vx_uint32                                outputParamCount;
    vx_uint32                                inputPhysicalEntry[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_uint32                                outputPhysicalEntry[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_uint32                                inputInPatchedBinOffset[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_uint32                                inputInPatchedPhysical[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_uint32                                inputInPatchedIndex[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_uint32                                inputInPatchedNum;
    vx_binary_input_output_info_s            inputInfo[VX_MAX_NN_INOUT_PARAM_COUNT];

    vx_reference                             inputEntry[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_reference                             outputEntry[VX_MAX_NN_INOUT_PARAM_COUNT];

    struct
    {
        vx_uint32                            size;
        vx_uint32                            tensorPhysical;
        vx_uint32                            LCDTIndex;
    } tempTensorsPhysical[VX_MAX_TEMP_TENSORS];
    vx_uint32                                numberOfTempTensorInfo;

    /* operation data info */
    vx_uint32                                operationCount;
    vx_uint32                                currOperationIndex;
    vx_uint32                                currOperationOffset;
    vx_uint32                                lastOperation0ffset;
    vx_uint64                                *operationCmdPhysical;
    vx_uint32                                *operationOffset;

    /* init operation data info */
    vx_uint32                                initOperationCount;
    /* end operation data info */
    vx_uint32                                endOperationCount;

    /* nn operation data info */
    vx_uint32                                nnOperationCount;
    vx_uint32                                currNNOperationIndex;
    vx_uint32                                currNNOperationOffset;
    /* tp operation data info */
    vx_uint32                                tpOperationCount;
    vx_uint32                                currTPOperationIndex;
    vx_uint32                                currTPOperationOffset;
    /* shader operation data info */
    vx_uint32                                shOperationCount;
    vx_uint32                                currSHOperationIndex;
    vx_uint32                                currSHOperationOffset;
    /* scaler operation data info */
    vx_uint32                                scOperationCount;
    vx_uint32                                scOperationOffset;
    /* sw operation data info */
    vx_uint32                                swOperationCount;
    vx_uint32                                swOperationOffset;
    vx_uint32                                currSWOperationIndex;
    vx_uint32                                currSWOperationOffset;

    /* patch data info */
    vx_uint32                                patchCount;
    vx_uint32                                currPatchIndex;
    vx_uint32                                currPatchOffset;
    vx_uint32                                *patchNNTPCmdPhysical;
    vx_uint32                                *patchNNTPCmdOffset;
    vx_uint32                                 patchNNTPCmdCount;

    /* loading data table info */
    vx_uint32                                currLoadingDataTableIndex;
    vx_uint32                                currLoadingDataTableOffset;

    /* loading data info*/
    vx_uint32                                loadingDataTotalBytes;
    vx_uint32                                loadingDataCount;
    vx_uint32                                loadingDataStartOffset;
    vx_uint32                                currLoadingDataOffset;

    /* layer parameters table */
    vx_uint32                                layerParamCount;
    vx_uint32                                currLayerParamOffset;
    vx_uint32                                currLayerParamIndex;

    vx_char                                  binaryFileName[256];

    vx_binary_input_node_info_s              inputNode[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_uint32                                inputNodeCount;

    vx_uint32                                savedOperationCount;

    vx_binary_header_s                       headerInfo;
    vx_binary_entrance_info_s                entrancesInfo;
    vx_uint32                                entryTablePos;

    vx_reference                             inputTableRef[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_reference                             outputTableRef[VX_MAX_NN_INOUT_PARAM_COUNT];
    vx_uint32                                inputTableRefCount;
    vx_uint32                                outputTableRefCount;

    vx_uint32                                *NNTPDataCmdPhysical;
    vx_uint32                                *NNTPDataOffset;
    vx_uint32                                NNTPDataCount;

    vx_uint8                                 waitCommands[VX_MAX_WAIT_STATE_SIZE];
    vx_uint32                                waitCommandsSize;

    vx_uint8                                 endCommands[VX_MAX_INITIALIZE_COMMAND_SIZE];
    vx_uint32                                endCommandsSize;

    vx_uint32                                generateNBGToMemory;
    vx_ptr                                   NBGBuffer;
    vx_uint32                                *NBGSize;
}
vx_binary_save_s, *vx_binary_save;

typedef struct _vx_binary_allocation
{
    vx_uint8_ptr                            logical;
    vx_uint32                               physical;
    gcsSURF_NODE_PTR                        nodePtr;
} vx_binary_allocation_s;

typedef struct _vx_binary_memory
{
    vx_binary_allocation_s                  pool;        /* Memory pool for GPU to use */
    vx_binary_allocation_s                  nntpCmdBuff; /*nn and tp command buffer on GPU*/
    vx_binary_allocation_s                  shCmdBuff;   /*shader instruction command buffer on GPU*/
    gctPOINTER                              statesBuff; /*states buffer on system*/
    vx_uint32                               statesSize;
} vx_binaryLoad_memory_s;

typedef struct _vx_binary_entry
{
    vx_uint32                               offset;
    vx_uint32                               size;
} vx_binary_entry_s;

typedef struct _vx_binary_io_patch_info
{
    vx_uint32                               count;
    vx_uint32                               number;
    vx_uint32                               **references;  /* The address of references in the command buffer. */
    vx_uint32                               *offsets;     /* The offset to the buffer base for each command. */
} vx_binary_io_patch_info_s;

typedef struct _vx_binary_fixed
{
    vx_binary_header_s                      header;
    vx_binary_memory_pool_info_s            poolTable;
    vx_binary_axi_sram_info_s               axiSramTable;
    vx_binary_entry_s                       layerTable;
    vx_binary_entry_s                       opeartionTable;
    vx_binary_entry_s                       LCDTable;
    vx_binary_entry_s                       LCD;
    vx_binary_entry_s                       nnOpDataTable;
    vx_binary_entry_s                       tpOpDataTable;
    vx_binary_entry_s                       shOpDataTable;
    vx_binary_entry_s                       inputTable;
    vx_binary_entry_s                       outputTable;
    vx_binary_entry_s                       patchDataTable;
    vx_binary_entry_s                       layerParamTable;
    vx_binary_entry_s                       swOpDataTable;
} vx_binary_fixed_s;

typedef struct _vxnne_nbg_operation_s
{
    vxnne_operation_s base;
}
vxnne_nbg_operation_s, *vxnne_nbg_operation;

typedef struct _vxnne_nbg_layer_s
{
    vxnne_layer_s                            base;
    vxnne_operation                          operations[1];
    vxnne_nbg_operation_s                    NBGOperation;
}
vxnne_nbg_layer_s, *vxnne_nbg_layer;

typedef struct _vx_binary_loader
{
    vx_kernel                               kernel;
    vx_context                              context;
    /* Fixed part of the bin. */
    vx_binary_fixed_s                       fixed;

    /* Dynamic data part of the bin. */
    vx_binary_input_output_info_s           *inputs;
    vx_binary_input_output_info_s           *outputs;
    vx_binary_layers_info_s                 *layersInfo;
    vx_binary_entry_s                       *LCDT;
    vx_binary_allocation_s                  LCD;
    vx_binary_nn_operation_info_s           *nnOpsData;
    vx_binary_tp_operation_info_s           *tpOpsData;
    vx_binary_sh_operation_info_s           *shOpsData;
    vx_binary_patch_info_s                  *patchData;
    vx_binary_operation_info_s              *operations;
    vx_binary_layer_parameter_s             *layerParam;
    vx_binary_sw_operation_info_s           *swOpsData;

    vx_uint32                               nInputs;
    vx_uint32                               nOutputs;
    vx_uint32                               nLayersInfo;
    vx_uint32                               nOperations;
    vx_uint32                               nLCDT;
    vx_uint32                               nNnOps;
    vx_uint32                               nTpOps;
    vx_uint32                               nShOps;
    vx_uint32                               nSwOps;
    vx_uint32                               nPdEntries;
    vx_uint32                               nlayerParam;

    gctPOINTER                              binaryBuffer; /* read binary file buffer */
    gctFILE                                 dFile;/* binary file descriptor */

    vx_binary_segment_s                     *segments;
    vx_uint32                               segmentsCount;

    vx_binary_io_patch_info_s               *inputPatch;
    vx_binary_io_patch_info_s               *outputPatch;

    vx_uint32                               status;
} vx_binary_loader_s;

typedef struct _vx_binary_reader
{
    vx_binary_loader_s                      *binLoad;
    vx_uint32                               offset;
    vx_uint32                               size;
    vx_char                                 *data;
    vx_char                                 *currentData;
} vx_binary_reader_s;

/* graph binary functions */
VX_INTERNAL_API vx_status vxoBinaryGraph_WrapNBGKernel(
    vx_node node,
    vx_binary_loader_s *binLoad
    );

VX_INTERNAL_API vx_enum vxoBinaryGraph_ConvertToOVXDataType(
    vx_enum dataType
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_GenerateStatesBuffer(
    vx_node node,
    vx_binary_loader_s *binLoad
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_ReleaseStatesBuffer(
    vx_node node
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_Run(
    vx_node node,
    vx_binary_loader_s *binLoad
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_LoadFile(
    vx_context context,
    vx_binary_loader_s **binLoad,
    gctCONST_STRING fileName
    );

VX_INTERNAL_API vx_bool vxoBinaryGraph_HasBinaryInGraph(
    vx_graph graph
);

VX_INTERNAL_API vx_status vxoBinaryGraph_SetParameter(
    vx_node node,
    vx_uint32 index
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_UpdataIOPhsicalTable(
    vx_node node,
    vx_uint32 index
);

VX_INTERNAL_API vx_status vxoBinaryGraph_ReleaseFile(
    vx_binary_loader_s *binLoad
    );

VX_INTERNAL_API void vxoBinaryGraph_SaveTPNNOperation(
    vx_node node,
    vx_uint8_ptr cmdLogicalAddress,
    vx_uint32 cmdPhysicalAddress,
    vx_uint32 cmdSize,
    vx_binary_operation_target_e cmdType,
    vx_uint8_ptr ksDataLogical,
    vx_uint32  ksDataPhysical,
    vx_uint32 ksDataSize,
    vxnne_tensor_info input,
    vxnne_tensor_info output,
    gctUINT32 inputPhyAddr,
    gctUINT32 outputPhyAddr
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveShaderOperation(
    vx_node  node,
    vxnne_operation operation,
    vx_shader kernelShader,
    vx_reference *shParameter,
    vx_uint32 shParamNum,
    gctUINT8_PTR stateBuffer,
    gctUINT stateSize,
    vx_uint32 batchIndex
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveEndOperation(
    vx_graph graph,
    vx_uint8_ptr stateBuffer,
    vx_uint32 stateSize
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveSWOperation(
    vxnne_operation operation
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveScalerOperation(
    vxnne_operation operation,
    gctUINT8_PTR stateBuffer,
    gctUINT32 stateSize
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_ReSaveNNTPCommand(
    vxnne_operation operation,
    vx_uint32 cmdPhysical,
    vx_uint32 offset,
    vx_uint32 value
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_ReSaveInputAndPatchTable(
    vx_graph graph
    );


VX_INTERNAL_API vx_status vxoBinaryGraph_SaveBinaryEntrance(
    vx_graph graph
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_GetGraphInputOutput(
    vx_graph graph
    );

VX_INTERNAL_API vx_status vxoBinaryGraph_SaveNNTPStates(
    vx_node node,
    vx_uint32 cmdPhysical,
    gctUINT8 *captureBuffer,
    vx_uint32 actualSize
    );

VX_INTERNAL_API void vxoBinaryGraph_CacheOrImport(
    vx_graph graph
    );
VX_INTERNAL_API void vxoBinaryGraph_ReleaseCache(
    vx_graph graph
    );

#endif

