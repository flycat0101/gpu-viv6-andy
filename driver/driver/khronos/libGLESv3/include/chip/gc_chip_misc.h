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


#ifndef __chip_misc_h__
#define __chip_misc_h__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __GLchipDrawableRec
{
    __GLchipStencilOpt *stencilOpt;
} __GLchipDrawable;

typedef struct __GLchipXfbHeaderRec
{
    gcsSURF_NODE headerNode;
    gctPOINTER   headerLocked;
}__GLchipXfbHeader;

typedef struct __GLchipQueryHeaderRec
{
    gcsSURF_NODE headerNode;
    gctUINT32    headerSize;
    gctINT32     headerIndex;
    gctPOINTER   headerLocked;
    gceSURF_TYPE headerSurfType;
}__GLchipQueryHeader;

typedef struct __GLchipQueryObjectRec
{
    /* Query information */
    gctSIGNAL querySignal;
    __GLchipQueryHeader *queryHeader;
    gceQueryType  type;
}__GLchipQueryObject;

extern GLboolean
__glChipBeginQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    );

extern GLboolean
__glChipEndQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    );

extern GLboolean
__glChipGetQueryObject(
    __GLcontext *gc,
    GLenum pname,
    __GLqueryObject *queryObj
    );

extern GLvoid
__glChipDeleteQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    );

extern GLboolean
__glChipCreateSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject
    );

extern GLboolean
__glChipDeleteSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject
    );

extern GLenum
__glChipWaitSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject,
    GLuint64 timeout
    );

extern GLboolean
__glChipSyncImage(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipSetImageSrc(
    void* eglImage,
    gcoSURF surface
    );

GLvoid
__glChipBindXFB(
    __GLcontext *gc,
    __GLxfbObject   *xfbObj
    );

GLvoid
__glChipDeleteXFB(
    __GLcontext *gc,
    __GLxfbObject   *xfbObj
    );

extern GLvoid
__glChipBeginXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    );

extern GLvoid
__glChipEndXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    );

extern GLvoid
__glChipPauseXFB(
    __GLcontext *gc
    );

extern GLvoid
__glChipResumeXFB(
    __GLcontext *gc
    );

extern GLvoid
__glChipGetXFBVarying(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    GLsizei bufSize,
    GLsizei* length,
    GLsizei* size,
    GLenum* type,
    GLchar* name
    );

extern GLboolean
__glChipCheckXFBBufSizes(
    __GLcontext *gc,
    __GLxfbObject *xfbObj,
    GLuint64 count
    );

GLvoid
__glChipGetSampleLocation(
    __GLcontext *gc,
     GLuint index,
     GLfloat *val
     );

GLvoid
__glChipMemoryBarrier(
    __GLcontext *gc,
    GLbitfield barriers
    );

GLvoid
__glChipBlendBarrier(
    __GLcontext *gc
    );

extern GLboolean
gcChipCheckRecompileEnable(
    __GLcontext *gc,
    gceSURF_FORMAT format
    );

#ifdef __cplusplus
}
#endif

#endif /* __chip_misc_h__ */
