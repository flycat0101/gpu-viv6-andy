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


#include "gc_spirv_disassemble.h"

#define spvPRINT                    gcmPRINT
#define SPV_DUMP_MAX_SIZE           2048

#define SpvDump_Is_ValidId(Id)      ((Id) != SPV_INVALID_ID)
#define SpvDump_Goto_OnError()      {goto OnError;}

// used to identify the extended instruction library imported when printing
typedef enum ExtInstSet {
    GLSL450Inst,
    OpenCLExtInst,
}ExtInstSet;

gctSTRING __SpvKernelEnqueueFlagsToString(gctUINT flag)
{
    switch (flag)
    {
    case 0:  return "NoWait";
    case 1:  return "WaitKernel";
    case 2:  return "WaitWorkGroup";

    default: return "Unsupported  kernel enqueue flag";
    }
}

gctSTRING __SpvGroupOperationToString(gctUINT gop)
{

    switch (gop)
    {
    case 0:  return "Reduce";
    case 1:  return "InclusiveScan";
    case 2:  return "ExclusiveScan";

    default: return "Unsupported group operation";
    }
}

gctSTRING __SpvScopeToString(gctUINT mem)
{
    switch (mem) {
    case 0:  return "CrossDevice";
    case 1:  return "Device";
    case 2:  return "Workgroup";
    case 3:  return "Subgroup";
    case 4:  return "Invocation";

    default: return "Unsupported scope";
    }
}

gctSTRING __SpvFuncParamAttrToString(gctUINT attr)
{
    switch (attr) {
    case 0:  return "Zext";
    case 1:  return "Sext";
    case 2:  return "ByVal";
    case 3:  return "Sret";
    case 4:  return "NoAlias";
    case 5:  return "NoCapture";
    case 6:  return "NoWrite";
    case 7:  return "NoReadWrite";

    default: return "Unsupported function parameter";
    }
}

gctSTRING __SpvLinkageTypeToString(gctUINT type)
{
    switch (type) {
    case 0:  return "Export";
    case 1:  return "Import";

    default: return "Unsupported linkage type";
    }
}

gctSTRING __SpvFPRoundingModeToString(gctUINT mode)
{
    switch (mode) {
    case 0:  return "RTE";
    case 1:  return "RTZ";
    case 2:  return "RTP";
    case 3:  return "RTN";

    default: return "Unsupported FP rounding mode";
    }
}

gctSTRING __SpvImageChannelDataTypeToString(gctUINT type)
{
    switch (type)
    {
    case 0: return "SnormInt8";
    case 1: return "SnormInt16";
    case 2: return "UnormInt8";
    case 3: return "UnormInt16";
    case 4: return "UnormShort565";
    case 5: return "UnormShort555";
    case 6: return "UnormInt101010";
    case 7: return "SignedInt8";
    case 8: return "SignedInt16";
    case 9: return "SignedInt32";
    case 10: return "UnsignedInt8";
    case 11: return "UnsignedInt16";
    case 12: return "UnsignedInt32";
    case 13: return "HalfFloat";
    case 14: return "Float";
    case 15: return "UnormInt24";
    case 16: return "UnormInt101010_2";

    default:
        return "Unsupported image channel datatype";
    }
}

gctSTRING __SpvImageChannelOrderToString(gctUINT format)
{
    switch (format) {
    case 0:  return "R";
    case 1:  return "A";
    case 2:  return "RG";
    case 3:  return "RA";
    case 4:  return "RGB";
    case 5:  return "RGBA";
    case 6:  return "BGRA";
    case 7:  return "ARGB";
    case 8:  return "Intensity";
    case 9:  return "Luminance";
    case 10: return "Rx";
    case 11: return "RGx";
    case 12: return "RGBx";
    case 13: return "Depth";
    case 14: return "DepthStencil";
    case 15: return "sRGB";
    case 16: return "sRGBx";
    case 17: return "sRGBA";
    case 18: return "sBGRA";

    default:
        return "Unsupported image channel order";
    }
}

gctSTRING __SpvSamplerFilterModeToString(gctUINT mode)
{
    switch (mode) {
    case 0: return "Nearest";
    case 1: return "Linear";

    default: return "Unsupported sampler filter mode";
    }
}

gctSTRING __SpvSamplerAddressingModeToString(gctUINT mode)
{
    switch (mode) {
    case 0:  return "None";
    case 1:  return "ClampToEdge";
    case 2:  return "Clamp";
    case 3:  return "Repeat";
    case 4:  return "RepeatMirrored";

    default: return "Unsupported sampler addressing mode";
    }
}

gctSTRING __SpvStorageClassToString(gctUINT StorageClass)
{
    switch (StorageClass) {
    case 0:  return "UniformConstant";
    case 1:  return "Input";
    case 2:  return "Uniform";
    case 3:  return "Output";
    case 4:  return "Workgroup";
    case 5:  return "CrossWorkgroup";
    case 6:  return "Private";
    case 7:  return "Function";
    case 8:  return "Generic";
    case 9:  return "PushConstant";
    case 10: return "AtomicCounter";
    case 11: return "Image";
    case 12: return "StorageBuffer";

    default: return "Unsupported storage class";
    }
}

gctSTRING __SpvDecorationToString(gctUINT decoration)
{
    switch (decoration) {
    case 0:  return "RelaxedPrecision";
    case 1:  return "SpecId";
    case 2:  return "Block";
    case 3:  return "BufferBlock";
    case 4:  return "RowMajor";
    case 5:  return "ColMajor";
    case 6:  return "ArrayStride";
    case 7:  return "MatrixStride";
    case 8:  return "GLSLShared";
    case 9:  return "GLSLPacked";
    case 10: return "CPacked";
    case 11: return "BuiltIn";
    case 12: return "No exist decoration";
    case 13: return "NoPerspective";
    case 14: return "Flat";
    case 15: return "Patch";
    case 16: return "Centroid";
    case 17: return "Sample";
    case 18: return "Invariant";
    case 19: return "Restrict";
    case 20: return "Aliased";
    case 21: return "Volatile";
    case 22: return "Constant";
    case 23: return "Coherent";
    case 24: return "NonWritable";
    case 25: return "NonReadable";
    case 26: return "Uniform";
    case 27: return "No exist decoration";
    case 28: return "SaturatedConversion";
    case 29: return "Stream";
    case 30: return "Location";
    case 31: return "Component";
    case 32: return "Index";
    case 33: return "Binding";
    case 34: return "DescriptorSet";
    case 35: return "Offset";
    case 36: return "XfbBuffer";
    case 37: return "XfbStride";
    case 38: return "FuncParamAttr";
    case 39: return "FP Rounding Mode";
    case 40: return "FP Fast Math Mode";
    case 41: return "Linkage Attributes";
    case 42: return "NoContraction";
    case 43: return "InputAttachmentIndex";
    case 44: return "Alignment";
    case 45: return "MaxByteOffset";
    case 46: return "AlignmentId";
    case 47: return "MaxByteOffsetId";

    default:  return "Unsupported decoration";
    }
}

gctSTRING __SpvSourceToString(gctUINT source)
{
    switch (source) {
    case 0:  return "Unknown";
    case 1:  return "ESSL";
    case 2:  return "GLSL";
    case 3:  return "OpenCL_C";
    case 4:  return "OpenCL_CPP";

    default: return "Unsupported source type";
    }
}

gctSTRING __SpvExecutionModelToString(gctUINT model)
{
    switch (model) {
    case 0:  return "Vertex";
    case 1:  return "TessellationControl";
    case 2:  return "TessellationEvaluation";
    case 3:  return "Geometry";
    case 4:  return "Fragment";
    case 5:  return "GLCompute";
    case 6:  return "Kernel";

    default: return "Unsupported execution model";
    }
}

gctSTRING __SpvExecutionModeToString(gctUINT mode)
{
    switch (mode) {
    case 0:  return "Invocations";
    case 1:  return "SpacingEqual";
    case 2:  return "SpacingFractionalEven";
    case 3:  return "SpacingFractionalOdd";
    case 4:  return "VertexOrderCw";
    case 5:  return "VertexOrderCcw";
    case 6:  return "PixelCenterInteger";
    case 7:  return "OriginUpperLeft";
    case 8:  return "OriginLowerLeft";
    case 9:  return "EarlyFragmentTests";
    case 10: return "PointMode";
    case 11: return "Xfb";
    case 12: return "DepthReplacing";
    case 13: return "No exist execution mode";
    case 14: return "DepthGreater";
    case 15: return "DepthLess";
    case 16: return "DepthUnchanged";
    case 17: return "LocalSize";
    case 18: return "LocalSizeHint";
    case 19: return "InputPoints";
    case 20: return "InputLines";
    case 21: return "InputLinesAdjacency";
    case 22: return "Triangles";
    case 23: return "InputTrianglesAdjacency";
    case 24: return "Quads";
    case 25: return "Isolines";
    case 26: return "OutputVertices";
    case 27: return "OutputPoints";
    case 28: return "OutputLineStrip";
    case 29: return "OutputTriangleStrip";
    case 30: return "VecTypeHint";
    case 31: return "ContractionOff";
    case 32: return "No exist execution mode";
    case 33: return "Initializer";
    case 34: return "Finalizer";
    case 35: return "SubgroupSize";
    case 36: return "SubgroupsPerWorkgroup";
    case 37: return "SubgroupsPerWorkgroupId";
    case 38: return "LocalSizeId";
    case 39: return "LocalSizeHintId";

    default: return "Unsupported execution mode";
    }
}

gctSTRING __SpvMemoryToString(gctUINT mem)
{
    switch (mem) {
    case 0:  return "Simple";
    case 1:  return "GLSL450";
    case 2:  return "OpenCL";

    default: return "Unsupported memory mode";
    }
}

gctSTRING __SpvCapabilityToString(gctUINT info)
{
    switch (info)
    {
    /* Basic capabilities. */
    case 0:  return "Matrix";
    case 1:  return "Shader";
    case 2:  return "Geometry";
    case 3:  return "Tessellation";
    case 4:  return "Addresses";
    case 5:  return "Linkage";
    case 6:  return "Kernel";
    case 7:  return "Vector16";
    case 8:  return "Float16Buffer";
    case 9:  return "Float16";
    case 10: return "Float64";
    case 11: return "Int64";
    case 12: return "Int64Atomics";
    case 13: return "ImageBasic";
    case 14: return "ImageReadWrite";
    case 15: return "ImageMipmap";
    case 16: return "No exist capability";
    case 17: return "Pipes";
    case 18: return "Groups";
    case 19: return "DeviceEnqueue";
    case 20: return "LiteralSampler";
    case 21: return "AtomicStorage";
    case 22: return "Int16";
    case 23: return "TessellationPointSize";
    case 24: return "GeometryPointSize";
    case 25: return "ImageGatherExtended";
    case 26: return "No exist capability";
    case 27: return "StorageImageMultisample";
    case 28: return "UniformBufferArrayDynamicIndexing";
    case 29: return "SampledImageArrayDynamicIndexing";
    case 30: return "StorageBufferArrayDynamicIndexing";
    case 31: return "StorageImageArrayDynamicIndexing";
    case 32: return "ClipDistance";
    case 33: return "CullDistance";
    case 34: return "ImageCubeArray";
    case 35: return "SampleRateShading";
    case 36: return "ImageRect";
    case 37: return "SampledRect";
    case 38: return "GenericPointer";
    case 39: return "Int8";
    case 40: return "InputAttachment";
    case 41: return "SparseResidency";
    case 42: return "MinLod";
    case 43: return "Sampled1D";
    case 44: return "Image1D";
    case 45: return "SampledCubeArray";
    case 46: return "SampledBuffer";
    case 47: return "ImageBuffer";
    case 48: return "ImageMSArray";
    case 49: return "StorageImageExtendedFormats";
    case 50: return "ImageQuery";
    case 51: return "DerivativeControl";
    case 52: return "InterpolationFunction";
    case 53: return "TransformFeedback";
    case 54: return "GeometryStreams";
    case 55: return "StorageImageReadWithoutFormat";
    case 56: return "StorageImageWriteWithoutFormat";
    case 57: return "MultiViewport";
    case 58: return "SubGroupDispatch";
    case 59: return "NamedBarrier";
    case 60: return "PipeStorage";
    case 61: return "GroupNonUniform";
    case 62: return "GroupNonUniformVote";
    case 63: return "GroupNonUniformArithmetic";
    case 64: return "GroupNonUniformBallot";
    case 65: return "GroupNonUniformShuffle";
    case 66: return "GroupNonUniformShuffleRelative";
    case 67: return "GroupNonUniformClustered";
    case 68: return "GroupNonUniformQuad";

    /* Extension capabilities. */
    case 4437:  return "DeviceGroup";
    case 4441:  return "VariablePointersStorageBuffer";

    /* Unsupport capabilities. */
    default: return "Unsupported capability";
    }
}

gctSTRING __SpvAddressingToString(gctUINT addr)
{
    switch (addr) {
    case 0:  return "Logical";
    case 1:  return "Physical32";
    case 2:  return "Physical64";

    default: return "Unsupported addressing mode";
    }
}

gctSTRING __SpvBuiltInToString(gctUINT builtIn)
{
    switch (builtIn) {
    case 0:  return "Position";
    case 1:  return "PointSize";
    case 2:  return "No exist builtin";
    case 3:  return "ClipDistance";
    case 4:  return "CullDistance";
    case 5:  return "VertexId";
    case 6:  return "InstanceId";
    case 7:  return "PrimitiveId";
    case 8:  return "InvocationId";
    case 9:  return "Layer";
    case 10: return "ViewportIndex";
    case 11: return "TessLevelOuter";
    case 12: return "TessLevelInner";
    case 13: return "TessCoord";
    case 14: return "PatchVertices";
    case 15: return "FragCoord";
    case 16: return "PointCoord";
    case 17: return "FrontFacing";
    case 18: return "SampleId";
    case 19: return "SamplePosition";
    case 20: return "SampleMask";
    case 21: return "No exist builtin";
    case 22: return "FragDepth";
    case 23: return "HelperInvocation";
    case 24: return "NumWorkgroups";
    case 25: return "WorkgroupSize";
    case 26: return "WorkgroupId";
    case 27: return "LocalInvocationId";
    case 28: return "GlobalInvocationId";
    case 29: return "LocalInvocationIndex";
    case 30: return "WorkDim";
    case 31: return "GlobalSize";
    case 32: return "EnqueuedWorkgroupSize";
    case 33: return "GlobalOffset";
    case 34: return "GlobalLinearId";
    case 35: return "No exist builtin";
    case 36: return "SubgroupSize";
    case 37: return "SubgroupMaxSize";
    case 38: return "NumSubgroups";
    case 39: return "NumEnqueuedSubgroups";
    case 40: return "SubgroupId";
    case 41: return "SubgroupLocalInvocationId";
    case 42: return "VertexIndex";                 // TBD: put next to VertexId?
    case 43: return "InstanceIndex";               // TBD: put next to InstanceId?
    case 4438: return "DeviceIndex";
    case 4440: return "ViewIndex";

    default: return "Unsupported builtin";
    }
}

gctSTRING __SpvAccessQualifierToString(gctUINT attr)
{
    switch (attr) {
    case 0:  return "ReadOnly";
    case 1:  return "WriteOnly";
    case 2:  return "ReadWrite";

    default: return "Unsupported access qualifier";
    }
}

gctSTRING __SpvImageFormatToString(gctUINT format)
{
    switch (format) {
    case  0: return "Unknown";

        // ES/Desktop float
    case  1: return "Rgba32f";
    case  2: return "Rgba16f";
    case  3: return "R32f";
    case  4: return "Rgba8";
    case  5: return "Rgba8Snorm";

        // Desktop float
    case  6: return "Rg32f";
    case  7: return "Rg16f";
    case  8: return "R11fG11fB10f";
    case  9: return "R16f";
    case 10: return "Rgba16";
    case 11: return "Rgb10A2";
    case 12: return "Rg16";
    case 13: return "Rg8";
    case 14: return "R16";
    case 15: return "R8";
    case 16: return "Rgba16Snorm";
    case 17: return "Rg16Snorm";
    case 18: return "Rg8Snorm";
    case 19: return "R16Snorm";
    case 20: return "R8Snorm";

        // ES/Desktop int
    case 21: return "Rgba32i";
    case 22: return "Rgba16i";
    case 23: return "Rgba8i";
    case 24: return "R32i";

        // Desktop int
    case 25: return "Rg32i";
    case 26: return "Rg16i";
    case 27: return "Rg8i";
    case 28: return "R16i";
    case 29: return "R8i";

        // ES/Desktop uint
    case 30: return "Rgba32ui";
    case 31: return "Rgba16ui";
    case 32: return "Rgba8ui";
    case 33: return "R32ui";

        // Desktop uint
    case 34: return "Rgb10a2ui";
    case 35: return "Rg32ui";
    case 36: return "Rg16ui";
    case 37: return "Rg8ui";
    case 38: return "R16ui";
    case 39: return "R8ui";

    default:
        return "Unsupported image format";
    }
}

gctSTRING __SpvDimensionToString(gctUINT dim)
{
    switch (dim) {
    case 0:  return "1D";
    case 1:  return "2D";
    case 2:  return "3D";
    case 3:  return "Cube";
    case 4:  return "Rect";
    case 5:  return "Buffer";
    case 6:  return "SubpassData";

    default: return "Unsupported dimension";
    }
}

gctSTRING __SpvOpcodeToString(SpvOp opCode)
{
    switch (opCode) {
    case 0:   return "OpNop";
    case 1:   return "OpUndef";
    case 2:   return "OpSourceContinued";
    case 3:   return "OpSource";
    case 4:   return "OpSourceExtension";
    case 5:   return "OpName";
    case 6:   return "OpMemberName";
    case 7:   return "OpString";
    case 8:   return "OpLine";
    case 10:  return "OpExtension";
    case 11:  return "OpExtInstImport";
    case 12:  return "OpExtInst";
    case 14:  return "OpMemoryModel";
    case 15:  return "OpEntryPoint";
    case 16:  return "OpExecutionMode";
    case 17:  return "OpCapability";
    case 19:  return "OpTypeVoid";
    case 20:  return "OpTypeBool";
    case 21:  return "OpTypeInt";
    case 22:  return "OpTypeFloat";
    case 23:  return "OpTypeVector";
    case 24:  return "OpTypeMatrix";
    case 25:  return "OpTypeImage";
    case 26:  return "OpTypeSampler";
    case 27:  return "OpTypeSampledImage";
    case 28:  return "OpTypeArray";
    case 29:  return "OpTypeRuntimeArray";
    case 30:  return "OpTypeStruct";
    case 31:  return "OpTypeOpaque";
    case 32:  return "OpTypePointer";
    case 33:  return "OpTypeFunction";
    case 34:  return "OpTypeEvent";
    case 35:  return "OpTypeDeviceEvent";
    case 36:  return "OpTypeReserveId";
    case 37:  return "OpTypeQueue";
    case 38:  return "OpTypePipe";
    case 39:  return "OpTypeForwardPointer";
    case 41:  return "OpConstantTrue";
    case 42:  return "OpConstantFalse";
    case 43:  return "OpConstant";
    case 44:  return "OpConstantComposite";
    case 45:  return "OpConstantSampler";
    case 46:  return "OpConstantNull";
    case 48:  return "OpSpecConstantTrue";
    case 49:  return "OpSpecConstantFalse";
    case 50:  return "OpSpecConstant";
    case 51:  return "OpSpecConstantComposite";
    case 52:  return "OpSpecConstantOp";
    case 54:  return "OpFunction";
    case 55:  return "OpFunctionParameter";
    case 56:  return "OpFunctionEnd";
    case 57:  return "OpFunctionCall";
    case 59:  return "OpVariable";
    case 60:  return "OpImageTexelPointer";
    case 61:  return "OpLoad";
    case 62:  return "OpStore";
    case 63:  return "OpCopyMemory";
    case 64:  return "OpCopyMemorySized";
    case 65:  return "OpAccessChain";
    case 66:  return "OpInBoundsAccessChain";
    case 67:  return "OpPtrAccessChain";
    case 68:  return "OpArrayLength";
    case 69:  return "OpGenericPtrMemSemantics";
    case 70:  return "OpInBoundsPtrAccessChain";
    case 71:  return "OpDecorate";
    case 72:  return "OpMemberDecorate";
    case 73:  return "OpDecorationGroup";
    case 74:  return "OpGroupDecorate";
    case 75:  return "OpGroupMemberDecorate";
    case 77:  return "OpVectorExtractDynamic";
    case 78:  return "OpVectorInsertDynamic";
    case 79:  return "OpVectorShuffle";
    case 80:  return "OpCompositeConstruct";
    case 81:  return "OpCompositeExtract";
    case 82:  return "OpCompositeInsert";
    case 83:  return "OpCopyObject";
    case 84:  return "OpTranspose";
    case 86:  return "OpSampledImage";
    case 87:  return "OpImageSampleImplicitLod";
    case 88:  return "OpImageSampleExplicitLod";
    case 89:  return "OpImageSampleDrefImplicitLod";
    case 90:  return "OpImageSampleDrefExplicitLod";
    case 91:  return "OpImageSampleProjImplicitLod";
    case 92:  return "OpImageSampleProjExplicitLod";
    case 93:  return "OpImageSampleProjDrefImplicitLod";
    case 94:  return "OpImageSampleProjDrefExplicitLod";
    case 95:  return "OpImageFetch";
    case 96:  return "OpImageGather";
    case 97:  return "OpImageDrefGather";
    case 98:  return "OpImageRead";
    case 99:  return "OpImageWrite";
    case 100: return "OpImage";
    case 101: return "OpImageQueryFormat";
    case 102: return "OpImageQueryOrder";
    case 103: return "OpImageQuerySizeLod";
    case 104: return "OpImageQuerySize";
    case 105: return "OpImageQueryLod";
    case 106: return "OpImageQueryLevels";
    case 107: return "OpImageQuerySamples";
    case 109: return "OpConvertFToU";
    case 110: return "OpConvertFToS";
    case 111: return "OpConvertSToF";
    case 112: return "OpConvertUToF";
    case 113: return "OpUConvert";
    case 114: return "OpSConvert";
    case 115: return "OpFConvert";
    case 116: return "OpQuantizeToF16";
    case 117: return "OpConvertPtrToU";
    case 118: return "OpSatConvertSToU";
    case 119: return "OpSatConvertUToS";
    case 120: return "OpConvertUToPtr";
    case 121: return "OpPtrCastToGeneric";
    case 122: return "OpGenericCastToPtr";
    case 123: return "OpGenericCastToPtrExplicit";
    case 124: return "OpBitcast";
    case 126: return "OpSNegate";
    case 127: return "OpFNegate";
    case 128: return "OpIAdd";
    case 129: return "OpFAdd";
    case 130: return "OpISub";
    case 131: return "OpFSub";
    case 132: return "OpIMul";
    case 133: return "OpFMul";
    case 134: return "OpUDiv";
    case 135: return "OpSDiv";
    case 136: return "OpFDiv";
    case 137: return "OpUMod";
    case 138: return "OpSRem";
    case 139: return "OpSMod";
    case 140: return "OpFRem";
    case 141: return "OpFMod";
    case 142: return "OpVectorTimesScalar";
    case 143: return "OpMatrixTimesScalar";
    case 144: return "OpVectorTimesMatrix";
    case 145: return "OpMatrixTimesVector";
    case 146: return "OpMatrixTimesMatrix";
    case 147: return "OpOuterProduct";
    case 148: return "OpDot";
    case 149: return "OpIAddCarry";
    case 150: return "OpISubBorrow";
    case 151: return "OpUMulExtended";
    case 152: return "OpSMulExtended";
    case 154: return "OpAny";
    case 155: return "OpAll";
    case 156: return "OpIsNan";
    case 157: return "OpIsInf";
    case 158: return "OpIsFinite";
    case 159: return "OpIsNormal";
    case 160: return "OpSignBitSet";
    case 161: return "OpLessOrGreater";
    case 162: return "OpOrdered";
    case 163: return "OpUnordered";
    case 164: return "OpLogicalEqual";
    case 165: return "OpLogicalNotEqual";
    case 166: return "OpLogicalOr";
    case 167: return "OpLogicalAnd";
    case 168: return "OpLogicalNot";
    case 169: return "OpSelect";
    case 170: return "OpIEqual";
    case 171: return "OpINotEqual";
    case 172: return "OpUGreaterThan";
    case 173: return "OpSGreaterThan";
    case 174: return "OpUGreaterThanEqual";
    case 175: return "OpSGreaterThanEqual";
    case 176: return "OpULessThan";
    case 177: return "OpSLessThan";
    case 178: return "OpULessThanEqual";
    case 179: return "OpSLessThanEqual";
    case 180: return "OpFOrdEqual";
    case 181: return "OpFUnordEqual";
    case 182: return "OpFOrdNotEqual";
    case 183: return "OpFUnordNotEqual";
    case 184: return "OpFOrdLessThan";
    case 185: return "OpFUnordLessThan";
    case 186: return "OpFOrdGreaterThan";
    case 187: return "OpFUnordGreaterThan";
    case 188: return "OpFOrdLessThanEqual";
    case 189: return "OpFUnordLessThanEqual";
    case 190: return "OpFOrdGreaterThanEqual";
    case 191: return "OpFUnordGreaterThanEqual";
    case 194: return "OpShiftRightLogical";
    case 195: return "OpShiftRightArithmetic";
    case 196: return "OpShiftLeftLogical";
    case 197: return "OpBitwiseOr";
    case 198: return "OpBitwiseXor";
    case 199: return "OpBitwiseAnd";
    case 200: return "OpNot";
    case 201: return "OpBitFieldInsert";
    case 202: return "OpBitFieldSExtract";
    case 203: return "OpBitFieldUExtract";
    case 204: return "OpBitReverse";
    case 205: return "OpBitCount";
    case 207: return "OpDPdx";
    case 208: return "OpDPdy";
    case 209: return "OpFwidth";
    case 210: return "OpDPdxFine";
    case 211: return "OpDPdyFine";
    case 212: return "OpFwidthFine";
    case 213: return "OpDPdxCoarse";
    case 214: return "OpDPdyCoarse";
    case 215: return "OpFwidthCoarse";
    case 218: return "OpEmitVertex";
    case 219: return "OpEndPrimitive";
    case 220: return "OpEmitStreamVertex";
    case 221: return "OpEndStreamPrimitive";
    case 224: return "OpControlBarrier";
    case 225: return "OpMemoryBarrier";
    case 227: return "OpAtomicLoad";
    case 228: return "OpAtomicStore";
    case 229: return "OpAtomicExchange";
    case 230: return "OpAtomicCompareExchange";
    case 231: return "OpAtomicCompareExchangeWeak";
    case 232: return "OpAtomicIIncrement";
    case 233: return "OpAtomicIDecrement";
    case 234: return "OpAtomicIAdd";
    case 235: return "OpAtomicISub";
    case 236: return "OpAtomicSMin";
    case 237: return "OpAtomicUMin";
    case 238: return "OpAtomicSMax";
    case 239: return "OpAtomicUMax";
    case 240: return "OpAtomicAnd";
    case 241: return "OpAtomicOr";
    case 242: return "OpAtomicXor";
    case 245: return "OpPhi";
    case 246: return "OpLoopMerge";
    case 247: return "OpSelectionMerge";
    case 248: return "OpLabel";
    case 249: return "OpBranch";
    case 250: return "OpBranchConditional";
    case 251: return "OpSwitch";
    case 252: return "OpKill";
    case 253: return "OpReturn";
    case 254: return "OpReturnValue";
    case 255: return "OpUnreachable";
    case 256: return "OpLifetimeStart";
    case 257: return "OpLifetimeStop";
    case 259: return "OpGroupAsyncCopy";
    case 260: return "OpGroupWaitEvents";
    case 261: return "OpGroupAll";
    case 262: return "OpGroupAny";
    case 263: return "OpGroupBroadcast";
    case 264: return "OpGroupIAdd";
    case 265: return "OpGroupFAdd";
    case 266: return "OpGroupFMin";
    case 267: return "OpGroupUMin";
    case 268: return "OpGroupSMin";
    case 269: return "OpGroupFMax";
    case 270: return "OpGroupUMax";
    case 271: return "OpGroupSMax";
    case 274: return "OpReadPipe";
    case 275: return "OpWritePipe";
    case 276: return "OpReservedReadPipe";
    case 277: return "OpReservedWritePipe";
    case 278: return "OpReserveReadPipePackets";
    case 279: return "OpReserveWritePipePackets";
    case 280: return "OpCommitReadPipe";
    case 281: return "OpCommitWritePipe";
    case 282: return "OpIsValidReserveId";
    case 283: return "OpGetNumPipePackets";
    case 284: return "OpGetMaxPipePackets";
    case 285: return "OpGroupReserveReadPipePackets";
    case 286: return "OpGroupReserveWritePipePackets";
    case 287: return "OpGroupCommitReadPipe";
    case 288: return "OpGroupCommitWritePipe";
    case 291: return "OpEnqueueMarker";
    case 292: return "OpEnqueueKernel";
    case 293: return "OpGetKernelNDrangeSubGroupCount";
    case 294: return "OpGetKernelNDrangeMaxSubGroupSize";
    case 295: return "OpGetKernelWorkGroupSize";
    case 296: return "OpGetKernelPreferredWorkGroupSizeMultiple";
    case 297: return "OpRetainEvent";
    case 298: return "OpReleaseEvent";
    case 299: return "OpCreateUserEvent";
    case 300: return "OpIsValidEvent";
    case 301: return "OpSetUserEventStatus";
    case 302: return "OpCaptureEventProfilingInfo";
    case 303: return "OpGetDefaultQueue";
    case 304: return "OpBuildNDRange";
    case 305: return "OpImageSparseSampleImplicitLod";
    case 306: return "OpImageSparseSampleExplicitLod";
    case 307: return "OpImageSparseSampleDrefImplicitLod";
    case 308: return "OpImageSparseSampleDrefExplicitLod";
    case 309: return "OpImageSparseSampleProjImplicitLod";
    case 310: return "OpImageSparseSampleProjExplicitLod";
    case 311: return "OpImageSparseSampleProjDrefImplicitLod";
    case 312: return "OpImageSparseSampleProjDrefExplicitLod";
    case 313: return "OpImageSparseFetch";
    case 314: return "OpImageSparseGather";
    case 315: return "OpImageSparseDrefGather";
    case 316: return "OpImageSparseTexelsResident";
    case 317: return "OpNoLine";
    case 318: return "OpAtomicFlagTestAndSet";
    case 319: return "OpAtomicFlagClear";
    case 320: return "OpImageSparseRead";
    case 321: return "OpSizeOf";
    case 322: return "OpTypePipeStorage";
    case 323: return "OpConstantPipeStorage";
    case 324: return "OpCreatePipeFromPipeStorage";
    case 325: return "OpGetKernelLocalSizeForSubgroupCount";
    case 326: return "OpGetKernelMaxNumSubgroups";
    case 327: return "OpTypeNamedBarrier";
    case 328: return "OpNamedBarrierInitialize";
    case 329: return "OpMemoryNamedBarrier";
    case 330: return "OpModuleProcessed";
    case 331: return "OpExecutionModeId";
    case 332: return "OpDecorateId";
    case 333: return "OpGroupNonUniformElect";

    default:
        return "Unsupported opcode";
    }
}

gceSTATUS __SpvDumpCheckId(SpvId id)
{
    return gcvSTATUS_OK;
}

gceSTATUS __SpvDumpCheckOpCode(SpvOp opCode)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (opCode < SpvOpNop || opCode > SpvOpGroupNonUniformElect)
        status = gcvSTATUS_INVALID_ARGUMENT;

    return status;
}

gctSTRING __SpvDumpTypeId(SpvId typeId)
{
    static gctCHAR line[SPV_DUMP_MAX_SIZE];
    gctUINT offset = 0;

    gcoOS_ZeroMemory(line, SPV_DUMP_MAX_SIZE * gcmSIZEOF(gctCHAR));

    if (SpvDump_Is_ValidId(typeId))
    {
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%12d ", typeId);
    }
    else
    {
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "             ");
    }

    return line;
}

gctSTRING __SpvDumpResultId(SpvId resultId)
{
    static gctCHAR line[SPV_DUMP_MAX_SIZE];
    gctUINT offset = 0;

    gcoOS_ZeroMemory(line, SPV_DUMP_MAX_SIZE * gcmSIZEOF(gctCHAR));

    if (SpvDump_Is_ValidId(resultId))
    {
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%16d:", resultId);
    }
    else
    {
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "                 ");
    }

    return line;
}

gctSTRING __SpvDumpString(
    IN gctUINT * stream,
    IN gctUINT word,
    INOUT gctUINT * charCount)
{
    static gctCHAR line[SPV_DUMP_MAX_SIZE];
    gctUINT startWord = word;
    gctUINT offset = 0;
    gctSTRING wordString;
    gctBOOL done = gcvFALSE;
    gctUINT i;

    gcoOS_ZeroMemory(line, SPV_DUMP_MAX_SIZE * gcmSIZEOF(gctCHAR));

    gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, " \t\t\t\t\t\t\t\t\t\t\t\t\t ");

    gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "\"");

    do {
        gctUINT content = stream[word];
        wordString = (gctSTRING)&content;

        for (i = 0; i < 4; ++i)
        {
            if (*wordString == 0)
            {
                done = gcvTRUE;
                break;
            }
            if (offset < SPV_DUMP_MAX_SIZE - 2)
            {
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%c", *(wordString++));
            }
            else
            {
                spvPRINT("%s", line);
                offset = 0;
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%c", *(wordString++));
            }
        }
        ++word;
    } while (!done);

    gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "\"");

    if (charCount)
    {
        *charCount = word - startWord;
    }

    spvPRINT(line);

    return line;
}

gctSTRING __SpvDumpId(SpvId Id)
{
    static gctCHAR line[SPV_DUMP_MAX_SIZE];
    gctUINT offset = 0;

    gcoOS_ZeroMemory(line, SPV_DUMP_MAX_SIZE * gcmSIZEOF(gctCHAR));

    gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%d", Id);

    return line;
}

gctSTRING __SpvDumpIds(SpvId *Ids, gctUINT numOperands)
{
    static gctCHAR line[SPV_DUMP_MAX_SIZE];
    gctUINT offset = 0;
    gctUINT word = 0;
    gctUINT i;

    gcoOS_ZeroMemory(line, SPV_DUMP_MAX_SIZE * gcmSIZEOF(gctCHAR));

    for (i = 0; i < numOperands; i++)
    {
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%d ", Ids[word]);
        word++;
    }

    return line;
}

gctSTRING __SpvDumpGeneralOperand(
    SpvOperandClass operandClass,
    gctUINT * stream,
    gctUINT word)
{
    gctSTRING dumpString = gcvNULL;
    gctUINT dumpId = stream[word];

    switch (operandClass)
    {
    case OperandAddressing:         dumpString = __SpvAddressingToString(dumpId); break;
    case OperandCapability:         dumpString = __SpvCapabilityToString(dumpId); break;
    case OperandMemory:             dumpString = __SpvMemoryToString(dumpId); break;
    case OperandExecutionModel:     dumpString = __SpvExecutionModelToString(dumpId); break;
    case OperandExecutionMode:      dumpString = __SpvExecutionModeToString(dumpId); break;
    case OperandSource:             dumpString = __SpvSourceToString(dumpId); break;
    case OperandDecoration:         dumpString = __SpvDecorationToString(dumpId); break;
    case OperandStorage:            dumpString = __SpvStorageClassToString(dumpId); break;
    case OperandDimensionality:     dumpString = __SpvDimensionToString(dumpId); break;
    case OperandSamplerAddressingMode:  dumpString = __SpvSamplerAddressingModeToString(dumpId); break;
    case OperandSamplerFilterMode:  dumpString = __SpvSamplerFilterModeToString(dumpId); break;
    case OperandSamplerImageFormat: dumpString = __SpvImageFormatToString(dumpId); break;
    case OperandImageChannelOrder:  dumpString = __SpvImageChannelOrderToString(dumpId); break;
    case OperandImageChannelDataType: dumpString = __SpvImageChannelDataTypeToString(dumpId); break;
    case OperandFPRoundingMode:     dumpString = __SpvFPRoundingModeToString(dumpId); break;
    case OperandLinkageType:        dumpString = __SpvLinkageTypeToString(dumpId); break;
    case OperandFuncParamAttr:      dumpString = __SpvFuncParamAttrToString(dumpId); break;
    case OperandAccessQualifier:    dumpString = __SpvAccessQualifierToString(dumpId); break;
    case OperandBuiltIn:            dumpString = __SpvBuiltInToString(dumpId); break;
    case OperandScope:              dumpString = __SpvScopeToString(dumpId); break;
    case OperandGroupOperation:     dumpString = __SpvGroupOperationToString(dumpId); break;
    case OperandKernelEnqueueFlags: dumpString = __SpvKernelEnqueueFlagsToString(dumpId); break;
    case OperandOpcode:             dumpString = __SpvOpcodeToString(dumpId); break;
    default: break;
    }

    return dumpString;
}

void __SpvDumpImageOperandMask(
    gctCHAR*Line,
    gctUINT*Offset,
    gctUINT Mask)
{
    gctUINT i;
    gctSTRING masks[SPV_MAX_IMAGE_OPERAND_MASK] = {"Bias", "Lod", "Grad", "ConstOffset", "Offset", "ConstOffsets", "Sample", "MinLod"};

    if (Mask == 0)
    {
        gcoOS_PrintStrSafe(Line,
                            SPV_DUMP_MAX_SIZE - 1,
                            Offset,
                            "None ");
    }

    for (i = 0; i < SPV_MAX_IMAGE_OPERAND_MASK; i++)
    {
        if (Mask & (1 << i))
        {
            gcoOS_PrintStrSafe(Line,
                               SPV_DUMP_MAX_SIZE - 1,
                               Offset,
                               "%s ",
                               masks[i]);
        }
    }
}

void __SpvDumpFPFastMathMask(
    gctCHAR*Line,
    gctUINT*Offset,
    gctUINT Mask)
{
    gctUINT i;
    gctSTRING masks[5] = {"NotNaN", "NotInf", "NSZ", "AllowRecip", "Fast"};

    if (Mask == 0)
    {
        gcoOS_PrintStrSafe(Line,
                           SPV_DUMP_MAX_SIZE - 1,
                           Offset,
                           "None ");
    }

    for (i = 0; i < sizeof(masks) / sizeof(gctSTRING); i++)
    {
        if (Mask & (1 << i))
        {
            gcoOS_PrintStrSafe(Line,
                               SPV_DUMP_MAX_SIZE - 1,
                               Offset,
                               "%s ",
                               masks[i]);
        }
    }
}

void __SpvDumpSelectMask(
    gctCHAR*Line,
    gctUINT*Offset,
    gctUINT Mask)
{
    gctUINT i;
    gctSTRING masks[2] = {"Flatten", "DontFlatten"};

    if (Mask == 0)
    {
        gcoOS_PrintStrSafe(Line,
                           SPV_DUMP_MAX_SIZE - 1,
                           Offset,
                           "None ");
    }

    for (i = 0; i < sizeof(masks) / sizeof(gctSTRING); i++)
    {
        if (Mask & (1 << i))
        {
            gcoOS_PrintStrSafe(Line,
                               SPV_DUMP_MAX_SIZE - 1,
                               Offset,
                               "%s ",
                               masks[i]);
        }
    }
}

void __SpvDumpLoopMask(
    gctCHAR*Line,
    gctUINT*Offset,
    gctUINT Mask)
{
    gctUINT i;
    gctSTRING masks[2] = {"Unroll", "DontUnroll"};

    if (Mask == 0)
    {
        gcoOS_PrintStrSafe(Line,
                           SPV_DUMP_MAX_SIZE - 1,
                           Offset,
                           "None ");
    }

    for (i = 0; i < sizeof(masks) / sizeof(gctSTRING); i++)
    {
        if (Mask & (1 << i))
        {
            gcoOS_PrintStrSafe(Line,
                               SPV_DUMP_MAX_SIZE - 1,
                               Offset,
                               "%s ",
                               masks[i]);
        }
    }
}

void __SpvDumpFunctionControlMask(
    gctCHAR*Line,
    gctUINT*Offset,
    gctUINT Mask)
{
    gctUINT i;
    gctSTRING masks[4] = {"Inline", "DontInline", "Pure", "Const"};

    if (Mask == 0)
    {
        gcoOS_PrintStrSafe(Line,
                           SPV_DUMP_MAX_SIZE - 1,
                           Offset,
                           "None ");
    }

    for (i = 0; i < sizeof(masks) / sizeof(gctSTRING); i++)
    {
        if (Mask & (1 << i))
        {
            gcoOS_PrintStrSafe(Line,
                               SPV_DUMP_MAX_SIZE - 1,
                               Offset,
                               "%s ",
                               masks[i]);
        }
    }
}

void __SpvDumpMemoryAccessMask(
    gctCHAR*Line,
    gctUINT*Offset,
    gctUINT Mask)
{
    gctUINT i;
    gctSTRING masks[3] = {"Volatile", "Aligned", "Nontemporal"};

    if (Mask == 0)
    {
        gcoOS_PrintStrSafe(Line,
                           SPV_DUMP_MAX_SIZE - 1,
                           Offset,
                           "None ");
    }

    for (i = 0; i < sizeof(masks) / sizeof(gctSTRING); i++)
    {
        if (Mask & (1 << i))
        {
            gcoOS_PrintStrSafe(Line,
                               SPV_DUMP_MAX_SIZE - 1,
                               Offset,
                               "%s ",
                               masks[i]);
        }
    }
}

void __SpvDumpKernelProfilingInfoMask(
    gctCHAR*Line,
    gctUINT*Offset,
    gctUINT Mask)
{
    gctUINT i;
    gctSTRING masks[1] = {"CmdExecTime"};

    if (Mask == 0)
    {
        gcoOS_PrintStrSafe(Line,
                           SPV_DUMP_MAX_SIZE - 1,
                           Offset,
                           "None ");
    }

    for (i = 0; i < sizeof(masks) / sizeof(gctSTRING); i++)
    {
        if (Mask & (1 << i))
        {
            gcoOS_PrintStrSafe(Line,
                               SPV_DUMP_MAX_SIZE - 1,
                               Offset,
                               "%s ",
                               masks[i]);
        }
    }
}

gceSTATUS __SpvDumpValidator(
    gctUINT * stream,
    gctUINT sizeInByte)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT word = 0;
    gctUINT size = sizeInByte / 4;

    if (size < 4)
    {
        return gcvSTATUS_INVALID_DATA;
    }

    if (stream[word++] != SpvMagicNumber)
    {
        return gcvSTATUS_INVALID_DATA;
    }

    spvPRINT("// Module Version %x", stream[word++]);
    spvPRINT("// Generated by (magic number): %x", stream[word++]);
    spvPRINT("// Id's are bound by %d", stream[word++]);
    spvPRINT("\n");

    if (stream[word] != 0)
    {
        return gcvSTATUS_INVALID_DATA;
    }

    return status;
}

gceSTATUS __SpvDumpLine(
    SpvId resultId,
    SpvId typeId,
    SpvOp opCode,
    gctUINT * stream,
    gctUINT numOperands)
{
    gceSTATUS status = gcvSTATUS_OK;
    gceSTATUS needPrint = gcvSTATUS_TRUE;
    gctCHAR line[SPV_DUMP_MAX_SIZE] = { 0 };
    gctUINT offset = 0;
    gctUINT word = 0;
    gctUINT operandNum = __SpvGetOperandNumFromOpCode(opCode);
    gctUINT i;
    gctUINT printCacheSize = SPV_DUMP_MAX_SIZE * 3 / 4;

    gcmONERROR(__SpvDumpCheckId(resultId));
    gcmONERROR(__SpvDumpCheckId(typeId));
    gcmONERROR(__SpvDumpCheckOpCode(opCode));

    /* result: */
    gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvDumpResultId(resultId));

    /* type: */
    gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvDumpTypeId(typeId));

    /* opCode */
    gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvOpcodeToString(opCode));

    /* special handle Image */
    if (opCode == SpvOpTypeImage)
    {
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvDumpId(stream[word++]));
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvDimensionToString(stream[word++]));
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", SpvDump_Is_ValidId(stream[word++]) ? "depth" : "");
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", SpvDump_Is_ValidId(stream[word++]) ? "array" : "");
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", SpvDump_Is_ValidId(stream[word++]) ? "multi-sampled" : "");
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", stream[word] == 0 ? "runtime" : (stream[word] == 1 ? "sampled" : "nonsampled"));
        word++;
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "format:%s ", __SpvImageFormatToString(stream[word++]));

        if (numOperands == 8)
        {
            gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvAccessQualifierToString(stream[word++]));
        }

        SpvDump_Goto_OnError();
    }

#define __PRINT_BUFFER_CACHE__          do { if ((offset) >= (printCacheSize)) { spvPRINT("%s", line); offset = 0; }; } while (0)

    /* handle all operands */
    for (i = 0; i < operandNum && numOperands > 0; i++)
    {
        SpvOperandClass operandClass = __SpvGetOperandClassFromOpCode(opCode, i);
        needPrint = gcvSTATUS_TRUE;

        __PRINT_BUFFER_CACHE__;

        switch (operandClass)
        {
        case OperandId:
        /* Scope&MemorySemantics is an <id> of a 32-bit integer scalar, we can't get the constant value when dummping the binary.  */
        case OperandScope:
        case OperandMemorySemantics:
            gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvDumpId(stream[word++]));
            --numOperands;
            break;

        case OperandVariableIds:
            {
                gctUINT i, j;
                for (i = 0, j = 0; i < numOperands; i++, j++)
                {
                    if (opCode == SpvOpEntryPoint)
                    {
                        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
                    }
                    gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%d ", stream[word + j]);
                    __PRINT_BUFFER_CACHE__;
                }
            }
            SpvDump_Goto_OnError();
            break;

        case OperandOptionalLiteral:
        case OperandVariableLiterals:
            if ((opCode == SpvOpDecorate && stream[word - 1] == SpvDecorationBuiltIn) ||
                (opCode == SpvOpMemberDecorate && stream[word - 1] == SpvDecorationBuiltIn))
            {
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvBuiltInToString(stream[word++]));
                --numOperands;
                i++;
            }

            {
                gctUINT i, j;
                for (i = 0, j = 0; i < numOperands; i++, j++)
                {
                    gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%d ", stream[word + j]);
                    __PRINT_BUFFER_CACHE__;
                }
            }

            SpvDump_Goto_OnError();
            break;

        case OperandVariableIdLiteral:
            while (numOperands > 0)
            {
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "\n");
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s", __SpvDumpResultId(0));
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s", __SpvDumpTypeId(0));
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "    ");
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "    Type %s, ", __SpvDumpId(stream[word++]));
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "member %s", __SpvDumpId(stream[word++]));

                __PRINT_BUFFER_CACHE__;

                numOperands -= 2;
            }
            break;

        case OperandVariableLiteralId:
            while (numOperands > 0)
            {
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "\n");
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s", __SpvDumpResultId(0));
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s", __SpvDumpTypeId(0));
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "    ");
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "    case %s: ", __SpvDumpId(stream[word++]));
                gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s", __SpvDumpId(stream[word++]));

                __PRINT_BUFFER_CACHE__;

                numOperands -= 2;
            }
            SpvDump_Goto_OnError();
            break;

        case OperandLiteralNumber:
            gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvDumpId(stream[word++]));

            --numOperands;

            if (opCode == SpvOpExtInst)
            {
                ExtInstSet extInstSet = GLSL450Inst;
                gctUINT entryPoint = stream[word - 1];

                if (gcoOS_MemCmp("OpenCL", (gctSTRING)(&stream[word - 2]), 6) == gcvSTATUS_OK)
                {
                    extInstSet = OpenCLExtInst;
                }

                if (entryPoint == GLSL450Inst)
                {
                }
            }
            break;

        case OperandOptionalLiteralString:
        case OperandLiteralString:
            {
                gctUINT costWord = 0;
                spvPRINT("%s", line);
                line[0] = '\0';
                offset = 0;
                __SpvDumpString(stream, word, &costWord);
                needPrint = gcvSTATUS_FALSE;

                word += costWord;
                numOperands -= costWord;
            }
            break;

        case OperandImageOperands:
            __SpvDumpImageOperandMask(line, &offset, stream[word]);
            word++;
            --numOperands;
            break;

        case OperandFPFastMath:
            __SpvDumpFPFastMathMask(line, &offset, stream[word]);
            word++;
            --numOperands;
            break;

        case OperandSelect:
            __SpvDumpSelectMask(line, &offset, stream[word]);
            word++;
            --numOperands;
            break;

        case OperandLoop:
            __SpvDumpLoopMask(line, &offset, stream[word]);
            word++;
            --numOperands;
            break;

        case OperandFunction:
            __SpvDumpFunctionControlMask(line, &offset, stream[word]);
            word++;
            --numOperands;
            break;

        case OperandMemoryAccess:
            __SpvDumpMemoryAccessMask(line, &offset, stream[word]);
            word++;
            --numOperands;
            break;

        case OperandKernelProfilingInfo:
            __SpvDumpKernelProfilingInfoMask(line, &offset, stream[word]);
            word++;
            --numOperands;
            break;

        default:
            gcmASSERT(operandClass >= OperandSource && operandClass < OperandOpcode);

            gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "%s ", __SpvDumpGeneralOperand(operandClass, stream, word));

            word++;
            --numOperands;
            break;
        }
    }

OnError:
    if (needPrint == gcvSTATUS_TRUE)
    {
        spvPRINT("%s", line);
    }
    return status;
}

gceSTATUS __SpvDumpSpecConstant(
    VkSpecializationInfo        *specInfo)
{
    gctUINT         i;
    gctUINT8        *data;
    gctCHAR         line[SPV_DUMP_MAX_SIZE] = { 0 };
    gctUINT         offset = 0;

    if (specInfo == gcvNULL || specInfo->mapEntryCount == 0)
    {
        return gcvSTATUS_OK;
    }

    data = (gctUINT8 *)specInfo->pData;

    for (i = 0; i < specInfo->mapEntryCount; i++)
    {
        VkSpecializationMapEntry    *mapEntry = (VkSpecializationMapEntry *)(&specInfo->pMapEntries[i]);
        gctUINT8                    *specData = data + mapEntry->offset;
        gctUINT32                   constValue = 0;
        gctINT32                    size = (gctINT32)mapEntry->size;

        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "SpecId(%d): ", mapEntry->constantID);

        while (size > 0)
        {
            if (size >= 4)
            {
                gcoOS_MemCopy(&constValue, specData, 4);
                size -= 4;
            }
            else
            {
                gcoOS_MemCopy(&constValue, specData, size);
                size = 0;
            }

            gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "0x%x ", constValue);
        }
        gcoOS_PrintStrSafe(line, SPV_DUMP_MAX_SIZE - 1, &offset, "\n");
    }

    spvPRINT("%s", line);

    return gcvSTATUS_OK;
}

gceSTATUS __SpvDumpSpriv(
    gctUINT * stream,
    gctUINT sizeInByte)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT word = 5;
    gctUINT size = sizeInByte / 4;

    if (stream == gcvNULL)
    {
        return gcvSTATUS_INVALID_DATA;
    }

    status = __SpvDumpValidator(stream, sizeInByte);
    if (status != gcvSTATUS_OK)
    {
        return status;
    }

    while (word < size)
    {
        gctUINT firstWord = stream[word];
        gctUINT wordCount = firstWord >> SpvWordCountShift;
        SpvOp opCode = (SpvOp)(firstWord & SpvOpCodeMask);
        gctUINT nextInst = word + wordCount;
        gctUINT numOperands = 0;
        SpvId resultId = 0, typeId = 0;

        ++word;

        /* Presence of full instruction */
        if (nextInst > size)
        {
            return gcvSTATUS_INVALID_DATA;
        }

        /* Base for computing number of operands; will be updated as more is learned */
        numOperands = wordCount - 1;

        if (__SpvOpCodeHasType(opCode))
        {
            typeId = stream[word++];
            --numOperands;
        }

        /* Result <id> */
        if (__SpvOpCodeHasResult(opCode))
        {
            resultId = stream[word++];
            --numOperands;
        }

        __SpvDumpLine(resultId, typeId, opCode, &stream[word], numOperands);

        word = nextInst;
    }

    return status;
}
