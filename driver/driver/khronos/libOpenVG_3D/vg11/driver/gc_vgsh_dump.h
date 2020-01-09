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


#ifndef __gc_vgsh_debug_h_
#define __gc_vgsh_debug_h_

#ifdef __cplusplus
extern "C" {
#endif

#define __VG_API_ENTRIES(vgApiMacro) \
    vgApiMacro(AppendPath), \
    vgApiMacro(AppendPathData), \
    vgApiMacro(ChildImage), \
    vgApiMacro(Clear), \
    vgApiMacro(ClearGlyph), \
    vgApiMacro(ClearImage), \
    vgApiMacro(ClearPath), \
    vgApiMacro(ColorMatrix), \
    vgApiMacro(Convolve), \
    vgApiMacro(CopyImage), \
    vgApiMacro(CopyMask), \
    vgApiMacro(CopyPixels), \
    vgApiMacro(CreateEGLImageTargetKHR), \
    vgApiMacro(CreateFont), \
    vgApiMacro(CreateImage), \
    vgApiMacro(CreateMaskLayer), \
    vgApiMacro(CreatePaint), \
    vgApiMacro(CreatePath), \
    vgApiMacro(DestroyFont), \
    vgApiMacro(DestroyImage), \
    vgApiMacro(DestroyMaskLayer), \
    vgApiMacro(DestroyPaint), \
    vgApiMacro(DestroyPath), \
    vgApiMacro(DrawGlyph), \
    vgApiMacro(DrawGlyphs), \
    vgApiMacro(DrawImage), \
    vgApiMacro(DrawPath), \
    vgApiMacro(FillMaskLayer), \
    vgApiMacro(Finish), \
    vgApiMacro(Flush), \
    vgApiMacro(GaussianBlur), \
    vgApiMacro(GetColor), \
    vgApiMacro(GetError), \
    vgApiMacro(GetImageSubData), \
    vgApiMacro(GetMatrix), \
    vgApiMacro(GetPaint), \
    vgApiMacro(GetParameterVectorSize), \
    vgApiMacro(GetParameterf), \
    vgApiMacro(GetParameterfv), \
    vgApiMacro(GetParameteri), \
    vgApiMacro(GetParameteriv), \
    vgApiMacro(GetParent), \
    vgApiMacro(GetPathCapabilities), \
    vgApiMacro(GetPixels), \
    vgApiMacro(GetString), \
    vgApiMacro(GetVectorSize), \
    vgApiMacro(Getf), \
    vgApiMacro(Getfv), \
    vgApiMacro(Geti), \
    vgApiMacro(Getiv), \
    vgApiMacro(HardwareQuery), \
    vgApiMacro(ImageSubData), \
    vgApiMacro(InterpolatePath), \
    vgApiMacro(LoadIdentity), \
    vgApiMacro(LoadMatrix), \
    vgApiMacro(Lookup), \
    vgApiMacro(LookupSingle), \
    vgApiMacro(Mask), \
    vgApiMacro(ModifyPathCoords), \
    vgApiMacro(MultMatrix), \
    vgApiMacro(PaintPattern), \
    vgApiMacro(PathBounds), \
    vgApiMacro(PathLength), \
    vgApiMacro(PathTransformedBounds), \
    vgApiMacro(PointAlongPath), \
    vgApiMacro(ReadPixels), \
    vgApiMacro(RemovePathCapabilities), \
    vgApiMacro(RenderToMask), \
    vgApiMacro(Rotate), \
    vgApiMacro(Scale), \
    vgApiMacro(SeparableConvolve), \
    vgApiMacro(SetColor), \
    vgApiMacro(SetGlyphToImage), \
    vgApiMacro(SetGlyphToPath), \
    vgApiMacro(SetPaint), \
    vgApiMacro(SetParameterf), \
    vgApiMacro(SetParameterfv), \
    vgApiMacro(SetParameteri), \
    vgApiMacro(SetParameteriv), \
    vgApiMacro(SetPixels), \
    vgApiMacro(Setf), \
    vgApiMacro(Setfv), \
    vgApiMacro(Seti), \
    vgApiMacro(Setiv), \
    vgApiMacro(Shear), \
    vgApiMacro(TransformPath), \
    vgApiMacro(Translate), \
    vgApiMacro(WritePixels), \
    vgApiMacro(uArc), \
    vgApiMacro(uComputeWarpQuadToQuad), \
    vgApiMacro(uComputeWarpQuadToSquare), \
    vgApiMacro(uComputeWarpSquareToQuad), \
    vgApiMacro(uEllipse), \
    vgApiMacro(uLine), \
    vgApiMacro(uPolygon), \
    vgApiMacro(uRect), \
    vgApiMacro(uRoundRect),


#ifndef VIV_EGL_BUILD

VG_API_CALL void OVGFrameDump(gctFILE *pFile, int flag);
VG_API_CALL void OVGAvgFrameDump(gctFILE *pFile);
VG_API_CALL void OVGSetNonCache(void);

#endif  /* VIV_EGL_BUILD */

#ifdef __cplusplus
}
#endif

#endif /* __gc_vgsh_debug_h_ */
