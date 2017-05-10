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


/**
**  @file
**  Architecture independent capability query functions.
**
*/

#include "gc_hal_user_precomp.h"

#define _GC_OBJ_ZONE        gcvZONE_SURFACE

gcsFORMAT_COMPONENT gcvPIXEL_COMP_XXX8 = {  0, 8 };
gcsFORMAT_COMPONENT gcvPIXEL_COMP_XX8X = {  8, 8 };
gcsFORMAT_COMPONENT gcvPIXEL_COMP_X8XX = { 16, 8 };
gcsFORMAT_COMPONENT gcvPIXEL_COMP_8XXX = { 24, 8 };

/*******************************************************************************
**
**  gcoSURF_ComputeColorMask
**
**  Computes the color pixel mask; for RGB formats alpha channel is not
**  included in the mask.
**
**  INPUT:
**
**      gcsSURF_FORMAT_INFO_PTR Format
**          Pointer to the format information structure.
**
**  OUTPUT:
**
**      gctUINT32_PTR ColorMask
**          Pixel color mask.
*/
gceSTATUS
gcoSURF_ComputeColorMask(
    IN gcsSURF_FORMAT_INFO_PTR Format,
    OUT gctUINT32_PTR ColorMask
    )
{
    gcmHEADER_ARG("Format=0x%x", Format);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Format != gcvNULL);
    gcmDEBUG_VERIFY_ARGUMENT(ColorMask != gcvNULL);

    /* Commpute source pixel mask. */
    if (Format[0].fmtClass == gcvFORMAT_CLASS_RGBA)
    {
        /* Reset the mask. */
        *ColorMask = 0;

        /* Add components. */
        if (Format[0].u.rgba.red.width)
        {
            *ColorMask
                |= ((1 << Format[0].u.rgba.red.width) - 1)
                << Format[0].u.rgba.red.start;
        }

        if (Format[0].u.rgba.green.width)
        {
            *ColorMask
                |= ((1 << Format[0].u.rgba.green.width) - 1)
                << Format[0].u.rgba.green.start;
        }

        if (Format[0].u.rgba.blue.width)
        {
            *ColorMask
                |= ((1 << Format[0].u.rgba.blue.width) - 1)
                << Format[0].u.rgba.blue.start;
        }
    }
    else
    {
        *ColorMask = (1 << Format[0].bitsPerPixel) - 1;
    }

    /* Success. */
    gcmFOOTER_ARG("*ColorMask=%08x", *ColorMask);
    return gcvSTATUS_OK;
}

#undef  _GC_OBJ_ZONE
#define _GC_OBJ_ZONE        gcvZONE_HAL

/*******************************************************************************
**
**  gcoHAL_QueryChipIdentity
**
**  Query the identity of the hardware.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**  OUTPUT:
**
**      gceCHIPMODEL* ChipModel
**          If 'ChipModel' is not gcvNULL, the variable it points to will
**          receive the model of the chip.
**
**      gctUINT32* ChipRevision
**          If 'ChipRevision' is not gcvNULL, the variable it points to will
**          receive the revision of the chip.
**
**      gctUINT32* ChipFeatures
**          If 'ChipFeatures' is not gcvNULL, the variable it points to will
**          receive the feature set of the chip.
**
**      gctUINT32 * ChipMinorFeatures
**          If 'ChipMinorFeatures' is not gcvNULL, the variable it points to
**          will receive the minor feature set of the chip.
**
*/
gceSTATUS gcoHAL_QueryChipIdentity(
    IN gcoHAL Hal,
    OUT gceCHIPMODEL* ChipModel,
    OUT gctUINT32* ChipRevision,
    OUT gctUINT32* ChipFeatures,
    OUT gctUINT32* ChipMinorFeatures
    )
{
    gceSTATUS status;
#if gcdENABLE_VG
    gceHARDWARE_TYPE currentHW = gcvHARDWARE_INVALID;
#endif

    gcmHEADER();
#if gcdENABLE_VG
    gcmGETCURRENTHARDWARE(currentHW);
    if (currentHW == gcvHARDWARE_VG)
    {
        /* Query identity from hardware object. */
        status = gcoVGHARDWARE_QueryChipIdentity(gcvNULL,
                                               ChipModel,
                                               ChipRevision);
    }
    else
#endif
    {
        /* Query identity from hardware object. */
        status = gcoHARDWARE_QueryChipIdentity(
                                           gcvNULL,
                                           ChipModel,
                                           ChipRevision);
    }

    /* Return status. */
    if (gcmIS_SUCCESS(status))
    {
        gcmFOOTER_ARG("status=%d *ChipModel=0x%x *ChipRevision=0x%x", status, gcmOPT_VALUE(ChipModel),
                      gcmOPT_VALUE(ChipRevision));
    }
    else
    {
        gcmFOOTER();
    }

    return status;
}


/*******************************************************************************
**
**  gcoHAL_QueryTiled
**
**  Query the tile sizes.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**  OUTPUT:
**
**      gctINT32 * TileWidth2D
**          Pointer to a variable receiving the width in pixels per 2D tile.  If
**          the 2D is working in linear space, the width will be 1.  If there is
**          no 2D, the width will be 0.
**
**      gctINT32 * TileHeight2D
**          Pointer to a variable receiving the height in pixels per 2D tile.
**          If the 2D is working in linear space, the height will be 1.  If
**          there is no 2D, the height will be 0.
**
**      gctINT32 * TileWidth3D
**          Pointer to a variable receiving the width in pixels per 3D tile.  If
**          the 3D is working in linear space, the width will be 1.  If there is
**          no 3D, the width will be 0.
**
**      gctINT32 * TileHeight3D
**          Pointer to a variable receiving the height in pixels per 3D tile.
**          If the 3D is working in linear space, the height will be 1.  If
**          there is no 3D, the height will be 0.
*/
gceSTATUS
gcoHAL_QueryTiled(
    IN gcoHAL Hal,
    OUT gctINT32 * TileWidth2D,
    OUT gctINT32 * TileHeight2D,
    OUT gctINT32 * TileWidth3D,
    OUT gctINT32 * TileHeight3D
    )
{
    gceSTATUS status;

    gcmHEADER();

    /* Query the tile sizes through gcoHARDWARE. */
    status = gcoHARDWARE_QueryTileSize(TileWidth2D, TileHeight2D,
                                       TileWidth3D, TileHeight3D,
                                       gcvNULL);

    gcmFOOTER_ARG("status=%d *TileWidth2D=%d *TileHeight2D=%d *TileWidth3D=%d "
                  "*TileHeight3D=%d", status, gcmOPT_VALUE(TileWidth2D),
                  gcmOPT_VALUE(TileHeight2D), gcmOPT_VALUE(TileWidth3D),
                  gcmOPT_VALUE(TileHeight3D));
    return status;
}

/*******************************************************************************
**
**  gcoHAL_QueryPowerManagementState
**
**  Query current GPU power state.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**  OUTPUT:
**
**      gceCHIPPOWERSTATE State
**          Power State.
**
*/
gceSTATUS
gcoHAL_QueryPowerManagementState(
    IN gcoHAL Hal,
    OUT gceCHIPPOWERSTATE * State
    )
{
    gcsHAL_INTERFACE iface;
    gceSTATUS status;

    gcmHEADER();

    /* Call kernel API to set power management state. */
    iface.command = gcvHAL_QUERY_POWER_MANAGEMENT_STATE;
    gcmONERROR(gcoHAL_Call(gcvNULL, &iface));

    /* Return state to the caller. */
    *State = iface.u.QueryPowerManagement.state;

    /* Success. */
    gcmFOOTER_ARG("*State=%d", *State);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHAL_IsFeatureAvailable
**
**  Verifies whether the specified feature is available in hardware.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**      gceFEATURE Feature
**          Feature to be verified.
*/
gceSTATUS
gcoHAL_IsFeatureAvailable(
    IN gcoHAL Hal,
    IN gceFEATURE Feature
    )
{
    gceSTATUS status;
#if gcdENABLE_VG
    gceHARDWARE_TYPE currentHW = gcvHARDWARE_INVALID;
#endif

    gcmHEADER_ARG("Feature=%d", Feature);
#if gcdENABLE_VG
    gcmGETCURRENTHARDWARE(currentHW);
    if (currentHW == gcvHARDWARE_VG)
    {
        status = gcoVGHARDWARE_IsFeatureAvailable(gcvNULL, Feature);
    }
    else
#endif
    {
        /* Query support from hardware object. */
        status = gcoHARDWARE_IsFeatureAvailable(gcvNULL, Feature);
    }


    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHAL_IsSwwaNeeded(
    IN gcoHAL Hal,
    IN gceSWWA Swwa
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Swwa=%d", Swwa);

    /* Query support from hardware object. */
    status = gcoHARDWARE_IsSwwaNeeded(gcvNULL, Swwa);

    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  gcoHAL_IsFeatureAvailable1
**
**  Verifies whether the specified feature is available in non-VG hardware.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**      gceFEATURE Feature
**          Feature to be verified.
*/
gceSTATUS
gcoHAL_IsFeatureAvailable1(
    IN gcoHAL Hal,
    IN gceFEATURE Feature
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("Feature=%d", Feature);

    /* Query support from hardware object. */
    status = gcoHARDWARE_IsFeatureAvailable(gcvNULL, Feature);

    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHAL_QueryVideoMemory
**
**  Query the amount of video memory.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to an gcoHAL object.
**
**  OUTPUT:
**
**      gctPHYS_ADDR * InternalAddress
**          Pointer to a variable that will hold the physical address of the
**          internal memory.  If 'InternalAddress' is gcvNULL, no information about
**          the internal memory will be returned.
**
**      gctSIZE_T * InternalSize
**          Pointer to a variable that will hold the size of the internal
**          memory.  'InternalSize' cannot be gcvNULL if 'InternalAddress' is not
**          gcvNULL.
**
**      gctPHYS_ADDR * ExternalAddress
**          Pointer to a variable that will hold the physical address of the
**          external memory.  If 'ExternalAddress' is gcvNULL, no information about
**          the external memory will be returned.
**
**      gctSIZE_T * ExternalSize
**          Pointer to a variable that will hold the size of the external
**          memory.  'ExternalSize' cannot be gcvNULL if 'ExternalAddress' is not
**          gcvNULL.
**
**      gctPHYS_ADDR * ContiguousAddress
**          Pointer to a variable that will hold the physical address of the
**          contiguous memory.  If 'ContiguousAddress' is gcvNULL, no information
**          about the contiguous memory will be returned.
**
**      gctSIZE_T * ContiguousSize
**          Pointer to a variable that will hold the size of the contiguous
**          memory.  'ContiguousSize' cannot be gcvNULL if 'ContiguousAddress' is
**          not gcvNULL.
*/
gceSTATUS
gcoHAL_QueryVideoMemory(
    IN gcoHAL Hal,
    OUT gctPHYS_ADDR * InternalAddress,
    OUT gctSIZE_T * InternalSize,
    OUT gctPHYS_ADDR * ExternalAddress,
    OUT gctSIZE_T * ExternalSize,
    OUT gctPHYS_ADDR * ContiguousAddress,
    OUT gctSIZE_T * ContiguousSize
    )
{
    gceSTATUS status;

    gcmHEADER();

    status = gcoOS_QueryVideoMemory(gcvNULL,
                                    InternalAddress, InternalSize,
                                    ExternalAddress, ExternalSize,
                                    ContiguousAddress, ContiguousSize);

    gcmFOOTER_ARG("status=%d InternalAddress=0x%x InternalSize=%lu "
                  "ExternalAddress=0x%x ExternalSize=%lu ContiguousAddress=0x%x "
                  "ContiguousSize=%lu", status, gcmOPT_POINTER(InternalAddress),
                  gcmOPT_VALUE(InternalSize), gcmOPT_POINTER(ExternalAddress),
                  gcmOPT_VALUE(ExternalSize), gcmOPT_POINTER(ContiguousAddress),
                  gcmOPT_VALUE(ContiguousSize));
    return status;
}

#if gcdENABLE_3D || gcdENABLE_VG
/*******************************************************************************
**
**  gcoHAL_QueryTargetCaps
**
**  Query the render target capabilities.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to gcoHAL object.
**
**  OUTPUT:
**
**      gctUINT * MaxWidth
**          Pointer to a variable receiving the maximum width of a render
**          target.
**
**      gctUINT * MaxHeight
**          Pointer to a variable receiving the maximum height of a render
**          target.
**
**      gctUINT * MultiTargetCount
**          Pointer to a variable receiving the number of render targets.
**
**      gctUINT * MaxSamples
**          Pointer to a variable receiving the maximum number of samples per
**          pixel for MSAA.
*/
gceSTATUS
gcoHAL_QueryTargetCaps(
    IN gcoHAL Hal,
    OUT gctUINT * MaxWidth,
    OUT gctUINT * MaxHeight,
    OUT gctUINT * MultiTargetCount,
    OUT gctUINT * MaxSamples
    )
{
    gceSTATUS status;

#if gcdENABLE_VG
    gceHARDWARE_TYPE currentHW = gcvHARDWARE_INVALID;
#endif
    gcmHEADER();

#if gcdENABLE_VG
    gcmGETCURRENTHARDWARE(currentHW);
    if (currentHW == gcvHARDWARE_VG)
    {
        status = gcoVGHARDWARE_QueryTargetCaps(gcvNULL,
                                         MaxWidth, MaxHeight,
                                         MultiTargetCount,
                                         MaxSamples);
    }
    else
#endif
    {
        status = gcoHARDWARE_QueryTargetCaps(gcvNULL,
                                            MaxWidth, MaxHeight,
                                            MultiTargetCount,
                                            MaxSamples);
    }

    gcmFOOTER_ARG("status=%d *MaxWidth=%u *MaxHeight=%u *MultiTargetCount=%u "
                  "*MaxSamples=%u",
                  status, gcoOS_DebugStatus2Name(status),
                  (MaxWidth         == gcvNULL) ? 0 : *MaxWidth,
                  (MaxHeight        == gcvNULL) ? 0 : *MaxHeight,
                  (MultiTargetCount == gcvNULL) ? 0 : *MultiTargetCount,
                  (MaxSamples       == gcvNULL) ? 0 : *MaxSamples);
    return status;
}
#endif

#if gcdENABLE_3D
/*******************************************************************************
**
**  gcoHAL_QueryShaderCaps
**
**  Query the shader capabilities.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**  OUTPUT:
**
**      gctUINT * UnifiedUniforms
**          Pointer to a variable receiving total number of unified uniforms, 0 if not support unified uniforms.
**
**      gctUINT * VertexUniforms
**          Pointer to a variable receiving the number of uniforms in the vertex shader.
**
**      gctUINT * FragmentUniforms
**          Pointer to a variable receiving the number of uniforms in the fragment shader.
**
**      gctUINT * Varyings
**          Pointer to a variable receiving the maximum number of varyings.
**
**      gctUINT * ShaderCoreCount
**          Pointer to a variable receiving the number of shader core count.
**
**      gctUINT * ThreadCount
**          Pointer to a variable receiving the number of thread count.
**
**      gctUINT * VertInstructionCount
**          Pointer to a variable receiving the number of supported vs instruction count.
**
**      gctUINT * FragInstructionCount
**          Pointer to a variable receiving the number of supported fs instruction count.
*/
gceSTATUS
gcoHAL_QueryShaderCaps(
    IN  gcoHAL    Hal,
    OUT gctUINT * UnifiedUniforms,
    OUT gctUINT * VertUniforms,
    OUT gctUINT * FragUniforms,
    OUT gctUINT * Varyings,
    OUT gctUINT * ShaderCoreCount,
    OUT gctUINT * ThreadCount,
    OUT gctUINT * VertInstructionCount,
    OUT gctUINT * FragInstructionCount
    )
{
    gceSTATUS status;

    gcmHEADER();

    /* Query the hardware. */
    status = gcoHARDWARE_QueryShaderCaps(gcvNULL,
                                         UnifiedUniforms,
                                         VertUniforms,
                                         FragUniforms,
                                         Varyings,
                                         ShaderCoreCount,
                                         ThreadCount,
                                         VertInstructionCount,
                                         FragInstructionCount);


    gcmFOOTER_ARG("status=%d UnifiedUniforms=%u VertUniforms=%u FragUniforms=%u Varyings=%u "
                  "ShaderCoreCount=%u ThreadCount=%u VertInstructionCount=%u FragInstructionCount=%u",
                  status, gcmOPT_VALUE(UnifiedUniforms), gcmOPT_VALUE(VertUniforms), gcmOPT_VALUE(FragUniforms),
                  gcmOPT_VALUE(Varyings), gcmOPT_VALUE(ShaderCoreCount), gcmOPT_VALUE(ThreadCount),
                  gcmOPT_VALUE(VertInstructionCount), gcmOPT_VALUE(FragInstructionCount));
    return status;
}

gceSTATUS
gcoHAL_QuerySamplerBase(
    IN  gcoHAL Hal,
    OUT gctUINT32 * VertexCount,
    OUT gctINT_PTR VertexBase,
    OUT gctUINT32 * FragmentCount,
    OUT gctINT_PTR FragmentBase
    )
{
    gceSTATUS status;
    gctUINT32 samplerCount[gcvPROGRAM_STAGE_LAST] = {0};
    gctUINT32 samplerBase[gcvPROGRAM_STAGE_LAST] = {0};

    gcmHEADER();

    /* Query the hardware. */
    status = gcoHARDWARE_QuerySamplerBase(gcvNULL,
                                          samplerCount,
                                          samplerBase,
                                          gcvNULL);

    if (VertexCount != gcvNULL)
    {
        *VertexCount = samplerCount[gcvPROGRAM_STAGE_VERTEX];
    }

    if (FragmentCount != gcvNULL)
    {
        *FragmentCount = samplerCount[gcvPROGRAM_STAGE_FRAGMENT];
    }

    if (VertexBase != gcvNULL)
    {
        *VertexBase = samplerBase[gcvPROGRAM_STAGE_VERTEX];
    }

    if (FragmentBase != gcvNULL)
    {
        *FragmentBase = samplerBase[gcvPROGRAM_STAGE_FRAGMENT];
    }

    if (gcoHAL_IsFeatureAvailable(Hal, gcvFEATURE_SAMPLER_BASE_OFFSET))
    {
        if (VertexBase)
        {
            *VertexBase = 0;
        }
    }

    gcmFOOTER_ARG("status=%d *VertexCount=%u *VertexBase=%u "
        "*FragmentCount=%u" "*FragmentBase=%u", status, gcmOPT_VALUE(VertexCount),
        gcmOPT_VALUE(VertexBase), gcmOPT_VALUE(FragmentCount), gcmOPT_VALUE(FragmentBase));

    return status;
}

gceSTATUS
gcoHAL_QueryUniformBase(
    IN  gcoHAL Hal,
    OUT gctUINT32 * VertexBase,
    OUT gctUINT32 * FragmentBase
    )
{
    gceSTATUS status;

    gcmHEADER();

    status = gcoHARDWARE_QueryUniformBase(gcvNULL, VertexBase, FragmentBase);

    gcmFOOTER();

    return status;
}

/*******************************************************************************
**
**  gcoHAL_QueryTextureCaps
**
**  Query the texture capabilities.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**  OUTPUT:
**
**      gctUINT * MaxWidth
**          Pointer to a variable receiving the maximum width of a texture.
**
**      gctUINT * MaxHeight
**          Pointer to a variable receiving the maximum height of a texture.
**
**      gctUINT * MaxDepth
**          Pointer to a variable receiving the maximum depth of a texture.  If
**          volume textures are not supported, 0 will be returned.
**
**      gctBOOL * Cubic
**          Pointer to a variable receiving a flag whether the hardware supports
**          cubic textures or not.
**
**      gctBOOL * NonPowerOfTwo
**          Pointer to a variable receiving a flag whether the hardware supports
**          non-power-of-two textures or not.
**
**      gctUINT * VertexSamplers
**          Pointer to a variable receiving the number of sampler units in the
**          vertex shader.
**
**      gctUINT * PixelSamplers
**          Pointer to a variable receiving the number of sampler units in the
**          pixel shader.
*/
gceSTATUS
gcoHAL_QueryTextureCaps(
    IN gcoHAL Hal,
    OUT gctUINT * MaxWidth,
    OUT gctUINT * MaxHeight,
    OUT gctUINT * MaxDepth,
    OUT gctBOOL * Cubic,
    OUT gctBOOL * NonPowerOfTwo,
    OUT gctUINT * VertexSamplers,
    OUT gctUINT * PixelSamplers
    )
{
    gceSTATUS status;

    gcmHEADER();

    /* Query the hardware. */
    status = gcoHARDWARE_QueryTextureCaps(gcvNULL,
                                          MaxWidth,
                                          MaxHeight,
                                          MaxDepth,
                                          Cubic,
                                          NonPowerOfTwo,
                                          VertexSamplers,
                                          PixelSamplers,
                                          gcvNULL);

    gcmFOOTER_ARG("status=%d *MaxWidth=%u *MaxHeight=%u *MaxDepth=%u *Cubic=%d "
                  "*NonPowerOfTwo=%d *VertexSamplers=%u *PixelSamplers=%u",
                  status, gcmOPT_VALUE(MaxWidth), gcmOPT_VALUE(MaxHeight),
                  gcmOPT_VALUE(MaxDepth), gcmOPT_VALUE(Cubic),
                  gcmOPT_VALUE(NonPowerOfTwo), gcmOPT_VALUE(VertexSamplers),
                  gcmOPT_VALUE(PixelSamplers));
    return status;
}

/*******************************************************************************
**
**  gcoHAL_QueryTextureMaxAniso
**
**  Query the texture capabilities.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**  OUTPUT:
**
**      gctUINT * MaxAnisoValue
**          Pointer to a variable receiving the maximum parameter value of
**          anisotropic filter.
*/
gceSTATUS
gcoHAL_QueryTextureMaxAniso(
    IN gcoHAL Hal,
    OUT gctUINT * MaxAnisoValue
    )
{
    gceSTATUS status;

    gcmHEADER();

    /* Query the hardware. */
    status = gcoHARDWARE_QueryTextureCaps(gcvNULL,
                                          gcvNULL,
                                          gcvNULL,
                                          gcvNULL,
                                          gcvNULL,
                                          gcvNULL,
                                          gcvNULL,
                                          gcvNULL,
                                          MaxAnisoValue);

    gcmFOOTER_ARG("status=%d *MaxAnisoValue=%u",
                  status, gcmOPT_VALUE(MaxAnisoValue));
    return status;
}

/*******************************************************************************
**
**  gcoHAL_QueryStreamCaps
**
**  Query the stream capabilities of the hardware.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**  OUTPUT:
**
**      gctUINT * MaxAttributes
**          Pointer to a variable that will hold the maximum number of
**          atrtributes for a primitive on success.
**
**      gctUINT * MaxStreamSize
**          Pointer to a variable that will hold the maximum number of bytes of
**          a stream on success.
**
**      gctUINT * NumberOfStreams
**          Pointer to a variable that will hold the number of streams on
**          success.
**
**      gctUINT * Alignment
**          Pointer to a variable that will receive the stream alignment
**          requirement.
**
**      gctUINT * MaxAttribOffset
**          Pointer to a variable that will receive the attribute relative offset.
**
*/
gceSTATUS
gcoHAL_QueryStreamCaps(
    IN gcoHAL Hal,
    OUT gctUINT32 * MaxAttributes,
    OUT gctUINT32 * MaxStreamStride,
    OUT gctUINT32 * NumberOfStreams,
    OUT gctUINT32 * Alignment,
    OUT gctUINT32 * MaxAttribOffset
    )
{
    gceSTATUS status;

    gcmHEADER();

    /* Query the hardware. */
    status = gcoHARDWARE_QueryStreamCaps(gcvNULL,
                                         MaxAttributes,
                                         MaxStreamStride,
                                         NumberOfStreams,
                                         Alignment,
                                         MaxAttribOffset);

    gcmFOOTER_ARG("status=%d *MaxAttributes=%u *MaxStreamStride=%u "
                  "*NumberOfStreams=%u *Alignment=%u *MaxAttribOffset=%u",
                  status, gcmOPT_VALUE(MaxAttributes), gcmOPT_VALUE(MaxStreamStride),
                  gcmOPT_VALUE(NumberOfStreams), gcmOPT_VALUE(Alignment), gcmOPT_VALUE(MaxAttribOffset));
    return status;
}

#endif /* gcdENABLE_3D */

/*******************************************************************************
**
**  gcoHAL_SetHardwareType
**
**  Set the hal hardware type to the tls.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**
**      gceHARDWARE_TYPE HardwardType
**          hardware type.
**  OUTPUT:
**
*/
gceSTATUS
gcoHAL_SetHardwareType(
    IN gcoHAL Hal,
    IN gceHARDWARE_TYPE HardwardType
    )
{
    gceSTATUS status;
    gcsTLS_PTR __tls__;

    gcmHEADER_ARG("HardwardType=%d", HardwardType);

    gcmONERROR(gcoOS_GetTLS(&__tls__));

    if (__tls__->currentType != HardwardType)
    {
        /* When hardware type is changed, reset currentCore according to
        ** multiple GPUs affinity setting.
        **
        ** If mode is gcvMULTI_GPU_MODE_COMBINED, always uses 0 as main
        ** core index.
        **
        ** so hardware with single core works without considering core index,
        ** hardware with multiple cores must specify core index explicitly
        ** before every device control calls.
        **
        ** Core index is global, for example, if there are four cores whose
        ** type are gcoHARDWARE_3D, core indexes of them are 0, 1, 2, 3.
        ** If a gcoHARDWARE object uses two of them 2 and 3, it should set
        ** currentCore to 2/3 when call device control, instead of 0/1
        */

        gceMULTI_GPU_MODE mode;
        gctUINT mainCoreIndex = 0;

        gcmVERIFY_OK(gcoHAL_QueryMultiGPUAffinityConfig(
            HardwardType,
            &mode,
            &mainCoreIndex
            ));

        if (mode == gcvMULTI_GPU_MODE_COMBINED)
        {
            mainCoreIndex = 0;
        }

        __tls__->currentCoreIndex = mainCoreIndex;
    }

    __tls__->currentType = HardwardType;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHAL_GetHardwareType
**
**  get the hal hardware type to the tls.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**  OUTPUT:
**
**      gceHARDWARE_TYPE *HardwardType
**          Pointer to a variable that will hold hardware type.
**
*/
gceSTATUS
gcoHAL_GetHardwareType(
    IN gcoHAL Hal,
    OUT gceHARDWARE_TYPE *HardwardType
    )
{
    gceSTATUS status;
    gcsTLS_PTR __tls__;

    gcmHEADER();

    gcmONERROR(gcoOS_GetTLS(&__tls__));

    if (HardwardType != gcvNULL)
    {
        *HardwardType = __tls__->currentType;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHAL_SetCore
**
**  Set the current core to the tls.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**
**      gctUINT32 CoreIndex
**          hardware type.
**  OUTPUT:
**
*/
gceSTATUS
gcoHAL_SetCoreIndex(
    IN gcoHAL Hal,
    IN gctUINT32 CoreIndex
    )
{
    gceSTATUS status;
    gcsTLS_PTR __tls__;

    gcmHEADER_ARG("CoreIndex=%d", CoreIndex);

    gcmONERROR(gcoOS_GetTLS(&__tls__));

    __tls__->currentCoreIndex = CoreIndex;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**
**  gcoHAL_GetCurrentCore
**
**  Get the current core of the tls.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**  OUTPUT:
**
**      gctUINT32 *Core
**          Pointer to a variable that will hold core.
**
*/
gceSTATUS
gcoHAL_GetCurrentCoreIndex(
    IN gcoHAL Hal,
    OUT gctUINT32 *CoreIndex
    )
{
    gceSTATUS status;
    gcsTLS_PTR __tls__;

    gcmHEADER();

    gcmONERROR(gcoOS_GetTLS(&__tls__));

    if (CoreIndex != gcvNULL)
    {
        *CoreIndex = __tls__->currentCoreIndex;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS
gcoHAL_QueryChipCount(
    IN gcoHAL       Hal,
    OUT gctINT32   *Count
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Count != gcvNULL);

    if (gcPLS.hal == gcvNULL || gcPLS.hal->chipCount <= 0 || gcPLS.hal->chipCount > gcdCHIP_COUNT)
    {
        gcmONERROR(gcvSTATUS_CONTEXT_LOSSED);
    }

    *Count = gcPLS.hal->chipCount;

    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHAL_Query3DCoreCount(
    IN gcoHAL       Hal,
    OUT gctUINT32  *Count
    )
{
    gceSTATUS status;
    gcmHEADER();

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(Count != gcvNULL);

    status = gcoHARDWARE_Query3DCoreCount(gcvNULL, Count);

    gcmFOOTER_NO();
    return status;
}

/*******************************************************************************
**
**  gcoHAL_QueryCoreCount
**
**  Get count of cores of the specified hardware type.
**
**  INPUT:
**
**      gcoHAL Hal
**          Pointer to a gcoHAL object.
**
**      gceHARDWARE_Type Type
**          Hardware type
**
**  OUTPUT:
**
**      gctUINT *Count
**          Pointer to a variable that will hold count of cores.
*/
gceSTATUS
gcoHAL_QueryCoreCount(
    IN gcoHAL Hal,
    IN gceHARDWARE_TYPE Type,
    OUT gctUINT *Count,
    OUT gctUINT_PTR ChipIDs
    )
{
    gctUINT i;

    *Count = 0;

    for (i = 0; i < gcdCHIP_COUNT; i++)
    {
        if (gcPLS.hal->chipTypes[i] == Type)
        {
            /* Get chip ID of nth GPU of this Type. */
            ChipIDs[*Count] = gcPLS.hal->chipIDs[i];

            *Count += 1;
        }
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcoHAL_QuerySeparated2D(
    IN gcoHAL Hal
    )
{
    gceSTATUS status;
    gcmHEADER();

    if (gcPLS.hal->separated2D)
    {
        status = gcvSTATUS_TRUE;
        gcmFOOTER();
        return status;
    }
    else
    {
        status = gcvSTATUS_FALSE;
        gcmFOOTER();
        return status;
    }
}

gceSTATUS
gcoHAL_QueryHybrid2D(
    IN gcoHAL Hal
    )
{
    gceSTATUS status;
    gcmHEADER();

    status = gcPLS.hal->hybrid2D ? gcvSTATUS_TRUE
           : gcvSTATUS_FALSE;

    gcmFOOTER();
    return status;
}

gceSTATUS
gcoHAL_Is3DAvailable(
    IN gcoHAL Hal
    )
{
    gceSTATUS status;

    gcmHEADER();

    if (gcPLS.hal->is3DAvailable)
    {
        status = gcvSTATUS_TRUE;
    }
    else
    {
        status = gcvSTATUS_FALSE;
    }

    gcmFOOTER();
    return status;
}

static gceHARDWARE_TYPE
_GetHardwareType(
    gctINT32 Chip
    )
{
    if (Chip >= gcdCHIP_COUNT)
        return gcvHARDWARE_INVALID;

    return gcPLS.hal->chipTypes[Chip];
}

gceSTATUS
gcoHAL_QueryChipLimits(
    IN gcoHAL           Hal,
    IN gctINT32         Chip,
    OUT gcsHAL_LIMITS   *Limits
    )
{
    gceSTATUS       status;
    gceCHIPMODEL    chipModel    = 0;
    gctUINT32       maxWidth     = 0;
    gctUINT32       maxHeight    = 0;
    gctUINT32       maxSamples   = 0;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
    gceHARDWARE_TYPE type        = _GetHardwareType(Chip);

    /* Save the current hardware type */
    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

    /* Change to the new hardware type */
    gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, type));

    switch (type)
    {
#if gcdENABLE_3D
    case gcvHARDWARE_3D2D:
    case gcvHARDWARE_3D:
    case gcvHARDWARE_2D:
        gcmONERROR(gcoHARDWARE_QueryChipIdentity(
            gcvNULL, &chipModel,
            gcvNULL));

        gcmONERROR(gcoHARDWARE_QueryTargetCaps(
            gcvNULL,
            (gctUINT32_PTR) &maxWidth,
            (gctUINT32_PTR) &maxHeight,
            gcvNULL,
            (gctUINT32_PTR) &maxSamples
            ));
        break;
#endif

#if gcdENABLE_VG
    case gcvHARDWARE_VG:
        gcmONERROR(gcoVGHARDWARE_QueryChipIdentity(
            gcvNULL,
            &chipModel,
            gcvNULL));

        gcmONERROR(gcoVGHARDWARE_QueryTargetCaps(
            gcvNULL,
            (gctUINT32_PTR) &maxWidth,
            (gctUINT32_PTR) &maxHeight,
            gcvNULL,
            (gctUINT32_PTR) &maxSamples
            ));
        break;
#endif
    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Limits != gcvNULL)
    {
        Limits->chipModel   = chipModel;
        Limits->maxWidth    = maxWidth;
        Limits->maxHeight   = maxHeight;
        Limits->maxSamples  = maxSamples;
    }

    status = gcvSTATUS_OK;

OnError:
    /* Restore the current hardware type */
    gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    return status;

}

gceSTATUS
gcoHAL_QueryChipFeature(
    IN gcoHAL       Hal,
    IN gctINT32     Chip,
    IN gceFEATURE   Feature
    )
{
    gceSTATUS status;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
    gceHARDWARE_TYPE type = _GetHardwareType(Chip);

    /* Save the current hardware type */
    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

    /* Change to the new hardware type */
    gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, type));

    switch (type)
    {
#if (gcdENABLE_3D || gcdENABLE_2D)
    case gcvHARDWARE_3D2D:
    case gcvHARDWARE_3D:
    case gcvHARDWARE_2D:
        status = gcoHARDWARE_IsFeatureAvailable(gcvNULL, Feature);
        break;
#endif
#if gcdENABLE_VG
    case gcvHARDWARE_VG:
        status = gcoVGHARDWARE_IsFeatureAvailable(gcvNULL, Feature);
        break;
#endif
    default:
        status = gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Restore the current hardware type */
    gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));

    return status;
}

gceSTATUS
gcoHAL_QuerySuperTileMode(
    OUT gctUINT32_PTR SuperTileMode
    )
{
    return gcoHARDWARE_QuerySuperTileMode(gcvNULL, SuperTileMode);
}

/*******************************************************************************
**
**  gcoHAL_QueryMultiGPUAffinityConfig
**
**  Check and parse configure from "VIV_MGPU_AFFINITY".
**
**  1) HAL level Multi GPUs affinity configure through "VIV_MGPU_AFFINITY".
**
**    VIV_MGPU_AFFINITY is an environment variable to control the multi GPUs
**    affinity configure in HAL. It provides a way to configure multi GPUs affinity
**    when client driver don't handle it. No matter how it is set, client drivers
**    think they are using a standalone GPU through a gcoHARDWARE object.
**
**    Possible value:
**
**    Not defined or defined as "0"
**        gcoHARDWARE objects work in gcvMULTI_GPU_COMBINED mode.
**    "1:0"
**        gcoHARDWARE objects work in gcvMULTI_GPU_INDEPEDNENT mode and GPU 0 is used
**    "1:1"
**        gcoHARDWARE objects work in gcvMULTI_GPU_INDEPEDNENT mode and GPU 1 is used
**
**
**  2) Client driver can have their own multi GPU affinity policy.
**
**    Client driver doesn't need "VIV_MGPU_AFFINITY", and it can override it by using
**    its own policy.
**
**    gcoHAL_SetCurrentCoreIndex() is used to define current core index which is used
**    by gcoHARDWARE objects, it needs to be set before gcoHARDWARE objects created.
**
**    gcoHARDWARE_SetMultiGPUMode() is used to set the mode of specified gcoHARDWARE
**    object after which is created.
**
**    Client driver can call gcoHAL_SetCurrentCoreIndex() after gcoHAL_SetHardwareType()
**    to set core index it wants.
**
**    Client driver can call gcoHARDWARE_SetMultiGPUMode() after gcoHARDWARE_Construct()
**    to change the mode it wants.
**
**    Example 1, a client wants to use 3D GPU1 only, it should do like this:
**      a) gcoHAL_SetHardwareType(gcvHARDWARE_3D)
**      b) gcoHAL_SetCurrentCoreIndex(1)
**      c) gcoHARDWARE_Construct()
**      d) gcoHARDWARE_SetMultiGPUMode(gcvMULTI_GPU_MODE_INDEPENENT)
**
**    Example 2, a client wants to use 3D GPU0 and 3D GPU1 separately, it should do like this:
**      a) gcoHAL_SetHardwareType(gcvHARDWARE_3D)
**      b) gcoHAL_SetCurrentCoreIndex(0)
**      c) hardware0 = gcoHARDWARE_Construct()
**      d) gcoHARDWARE_SetMultiGPUMode(hardware0, gcvMULTI_GPU_MODE_INDEPENENT)
**      e) gcoHAL_SetCurrentCoreIndex(1)
**      f) hardware1 = gcoHARDWARE_Construct()
**      g) gcoHARDWARE_SetMultiGPUMode(hardware1, gcvMULTI_GPU_MODE_INDEPENENT)
**      h) gcoHAL_SetCurrentCoreIndex(0)
**      i) submit hardware0 related command to kernel
**      j) gcoHAL_SetCurrentCoreIndex(1)
**      k) submit hardware1 related command to kernel
**
**  INPUT:
**
**
**  OUTPUT:
**
**      gceMULTI_GPU_MODE *Mode
**          Pointer to a variable that indicates what mode should be used.
**
**      gctUINT32_PTR Mode
**          Pointer to a variable that indicates which core mode should be
**          used as main core. It is meaningless when multi GPUs work in
**          combined mode.
*/
gceSTATUS
gcoHAL_QueryMultiGPUAffinityConfig(
    IN gceHARDWARE_TYPE Type,
    OUT gceMULTI_GPU_MODE *Mode,
    OUT gctUINT32_PTR CoreIndex
    )
{
    gctSTRING affinity = gcvNULL;
    gctSIZE_T length;

    if (Type != gcvHARDWARE_3D && Type != gcvHARDWARE_3D2D)
    {
        if (Mode)
        {
            *Mode = gcvMULTI_GPU_MODE_COMBINED;
        }

        return gcvSTATUS_OK;
    }

    gcoOS_GetEnv(gcvNULL, "VIV_MGPU_AFFINITY", &affinity);

    if (affinity == gcvNULL)
    {
        /* No configure specified, use default. */
        if (Mode)
        {
            *Mode = gcvMULTI_GPU_MODE_COMBINED;
        }

        return gcvSTATUS_OK;
    }

    gcoOS_StrLen(affinity, &length);

    if (length < 1)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (affinity[0] == '0')
    {
        if (Mode)
        {
            *Mode = gcvMULTI_GPU_MODE_COMBINED;
        }

        return gcvSTATUS_OK;
    }
    else
    {
        if (length != 3
         || affinity[0] != '1'
         || affinity[1] != ':'
         || (affinity[2] != '0' && affinity[2] != '1')
         )
        {
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        if (Mode)
        {
            *Mode = gcvMULTI_GPU_MODE_INDEPENDENT;
        }

        if (CoreIndex)
        {
            *CoreIndex = affinity[2] - '0';
        }
    }

    return gcvSTATUS_OK;
}

