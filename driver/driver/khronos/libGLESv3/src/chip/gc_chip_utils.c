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


#include "gc_es_context.h"
#include "gc_chip_context.h"
#include "gc_es_object_inline.c"

#define _GC_OBJ_ZONE    __GLES3_ZONE_TRACE

GLuint
gcChipUtilsEvaluateCRC32(
    GLvoid *pData,
    GLuint dataSizeInByte
    )
{
    GLuint crc = 0xFFFFFFFF;
    GLbyte *start = (GLbyte*)pData;
    GLbyte *end = start + dataSizeInByte;

    static const GLuint crc32Table[256] =
    {
        0x00000000,0x77073096,0xee0e612c,0x990951ba,
        0x076dc419,0x706af48f,0xe963a535,0x9e6495a3,
        0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,
        0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,
        0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,
        0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
        0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,
        0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,
        0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,
        0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
        0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,
        0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
        0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,
        0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
        0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,
        0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,
        0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a,
        0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433,
        0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,
        0x7f6a0dbb,0x086d3d2d,0x91646c97,0xe6635c01,
        0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,
        0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,
        0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c,
        0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
        0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,
        0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,
        0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,
        0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
        0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,
        0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
        0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,
        0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,
        0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,
        0xead54739,0x9dd277af,0x04db2615,0x73dc1683,
        0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8,
        0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1,
        0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,
        0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,
        0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,
        0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
        0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,
        0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
        0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,
        0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,
        0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,
        0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,
        0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,
        0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
        0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,
        0x9c0906a9,0xeb0e363f,0x72076785,0x05005713,
        0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38,
        0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,
        0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e,
        0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
        0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,
        0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
        0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,
        0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,
        0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,
        0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
        0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,
        0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,
        0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,
        0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d
    };
    gcmHEADER_ARG("pData=0x%x dataSizeInByte=%d",pData, dataSizeInByte);

    while (start < end)
    {
        GLuint data = *start ++;
        data &= 0xFF;
        crc = crc32Table[(crc & 255) ^ data] ^ (crc >> 8);
    }
    gcmFOOTER_ARG("return=%u", ~crc);
    return ~crc;
}

GLvoid
gcChipUtilsObjectAddRef(
    __GLchipUtilsObject *pObj
    )
{
    pObj->refCount++;
}

GLvoid
gcChipUtilsObjectReleaseRef(
    __GLchipUtilsObject *pObj
    )
{
    pObj->refCount--;

    gcmASSERT(pObj->refCount >= 0);
}

__GLchipUtilsHash*
gcChipUtilsHashCreate(
    __GLcontext *gc,
    GLuint tbEntryNum,
    GLuint maxEntryObjs,
    __GLchipDeleteUserDataFunc pfnDeleteUserData
    )
{
    __GLchipUtilsHash *pHash = gcvNULL;
    gcmHEADER_ARG("gc=0x%x tbEntryNum=%d maxEntryObjs=%d pfnDeleteUserData=0x%x",
                   gc, tbEntryNum, maxEntryObjs, pfnDeleteUserData);

    pHash = (__GLchipUtilsHash*)(*gc->imports.calloc)(gc, 1, sizeof(__GLchipUtilsHash));
    gcmASSERT(pHash != gcvNULL);
    if(pHash == gcvNULL)
    {
        gcmFOOTER_ARG("return=0x%x", pHash);
        return gcvNULL;
    }

    pHash->tbEntryNum = tbEntryNum;
    pHash->maxEntryObjs = maxEntryObjs;
    pHash->year = 0;
    pHash->pfnDeleteUserData = pfnDeleteUserData;

    pHash->ppHashTable = (__GLchipUtilsObject**)(*gc->imports.calloc)(gc, tbEntryNum, sizeof(__GLchipUtilsObject*));
    pHash->pEntryCounts = (GLuint*)(*gc->imports.calloc)(gc, tbEntryNum, sizeof(GLuint));
    gcmASSERT(pHash->ppHashTable && pHash->pEntryCounts);

    gcmFOOTER_ARG("return=0x%x", pHash);
    return pHash;
}

GLvoid
gcChipUtilsHashDestory(
    __GLcontext *gc,
    __GLchipUtilsHash *pHash
    )
{
    gcmHEADER_ARG("gc=0x%x pHash=0x%x ",gc, pHash);

    gcChipUtilsHashDeleteAllObjects(gc, pHash);
    (*gc->imports.free)(gc, pHash->pEntryCounts);
    (*gc->imports.free)(gc, pHash->ppHashTable);
    (*gc->imports.free)(gc, pHash);

    gcmFOOTER_NO();
}

GLvoid
gcChipUtilsHashDeleteObject(
    __GLcontext *gc,
    __GLchipUtilsHash *pHash,
    __GLchipUtilsObject *pObj
    )
{
    GLuint entryId = pObj->key & (pHash->tbEntryNum - 1);
    __GLchipUtilsObject *pCurObj = pHash->ppHashTable[entryId];
    __GLchipUtilsObject *pPreObj = gcvNULL;

    gcmHEADER_ARG("gc=0x%x pHash=0x%x pObj=0x%x",gc, pHash, pObj);

    while (pCurObj)
    {
        if (pCurObj == pObj)
        {
            break;
        }

        pPreObj = pCurObj;
        pCurObj = pCurObj->next;
    }

    /* Is this object being used? */
    gcmASSERT(pObj->refCount == 0);

    if (pPreObj == gcvNULL)
    {
        pHash->ppHashTable[entryId] = pCurObj->next;
    }
    else
    {
        pPreObj->next = pCurObj->next;
    }

    --pHash->pEntryCounts[entryId];
    pHash->pfnDeleteUserData(gc, pCurObj->pUserData);
    (*gc->imports.free)(gc, pCurObj);

    gcmFOOTER_NO();
}

GLvoid
gcChipUtilsHashDeleteAllObjects(
    __GLcontext *gc,
    __GLchipUtilsHash *pHash
    )
{
    GLuint entryId;
    gcmHEADER_ARG("gc=0x%x pHash=0x%x",gc, pHash);

    for (entryId = 0; entryId < pHash->tbEntryNum; entryId ++)
    {
        while (pHash->ppHashTable[entryId])
        {
            gcChipUtilsHashDeleteObject(gc, pHash, pHash->ppHashTable[entryId]);
        }
    }
    gcmFOOTER_NO();
}

__GLchipUtilsObject*
gcChipUtilsHashAddObject(
    __GLcontext *gc,
    __GLchipUtilsHash *pHash,
    GLvoid *pUserData,
    GLuint key,
    GLboolean bPerpetual
    )
{
    __GLchipUtilsObject *pNewObj = gcvNULL;
    GLuint entryId = key & (pHash->tbEntryNum - 1);

    gcmHEADER_ARG("gc=0x%x pHash=0x%x pUserData=0x%x key=%u bPerpetual=%d",gc, pHash, pUserData, key, bPerpetual);

    pNewObj = (__GLchipUtilsObject*)(*gc->imports.calloc)(gc, 1, sizeof(__GLchipUtilsObject));
    gcmASSERT(pNewObj);
    if (pNewObj == gcvNULL)
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    pNewObj->pUserData = pUserData;
    pNewObj->key = key;
    pNewObj->refCount = 0;
    pNewObj->year = pHash->year++;
    pNewObj->perpetual = bPerpetual;

    /* Check if objects of this hash slot exceeds */
    if (++pHash->pEntryCounts[entryId] > pHash->maxEntryObjs)
    {
        GLuint earliestYear = 0xFFFFFFFF;
        __GLchipUtilsObject *pOldestObj = gcvNULL;
        __GLchipUtilsObject *pObj = pHash->ppHashTable[entryId];
        while (pObj)
        {
            /* Found oldest object except perpetual ones */
            if (!pObj->perpetual && earliestYear > pObj->year)
            {
                earliestYear = pObj->year;
                pOldestObj = pObj;
            }

            pObj = pObj->next;
        }
        gcChipUtilsHashDeleteObject(gc, pHash, pOldestObj);
    }

    pNewObj->next = pHash->ppHashTable[entryId];
    pHash->ppHashTable[entryId] = pNewObj;

    gcmFOOTER_ARG("return=0x%x", pNewObj);
    return pNewObj;
}

__GLchipUtilsObject*
gcChipUtilsHashFindObjectByKey(
    __GLcontext *gc,
    __GLchipUtilsHash *pHash,
    GLuint key
    )
{
    GLuint entryId = key & (pHash->tbEntryNum - 1);
    __GLchipUtilsObject *pObj = pHash->ppHashTable[entryId];
    __GLchipUtilsObject *retObj = gcvNULL;

    gcmHEADER_ARG("gc=0x%x pHash=0x%x key=%u",gc, pHash, key);

    while (pObj)
    {
        if (pObj->key == key)
        {
            retObj = pObj;
            break;
        }

        pObj = pObj->next;
    }

    /* Update year to be recent one */
    if (retObj)
    {
        retObj->year = pHash->year++;
    }

    gcmFOOTER_ARG("return=0x%x", gcvNULL);
    return retObj;
}

gceSTATUS
gcChipUtilsDumpSurfaceTGA(
    __GLcontext *gc,
    gcsSURF_VIEW *surfView,
    gctCONST_STRING fileName,
    GLboolean yInverted
    )
{
    gctPOINTER frameMemory = gcvNULL;
    gctFILE file = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gcsSURF_VIEW tgtView = {gcvNULL, 0, 1};
    gcsSURF_VIEW resolveRTView = {gcvNULL, 0, 1};
    gctPOINTER logical[3]  = {0};
    gctUINT width = 0, height = 0, depth = 0;
    char level[6] = "+.tga";
    GLchar fName[__GLES_MAX_FILENAME_LEN]= {0};
    gctUINT8 tgaHeader[18];
    const GLchar *formatName = "";

    gcmHEADER_ARG("gc=0x%x, surfView=0x%x, fileName=%s, yInverted=%d,", gc, surfView, fileName, yInverted);

    do
    {
        gctINT32 resolveStride;
        gctUINT8_PTR frameBGR;
        gctUINT8_PTR frameARGB;
        gctINT32 x, y;
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};
        gctBOOL visualizeDepth = gcvFALSE;
        gcsSURF_FORMAT_INFO_PTR srcFmtInfo;

        gcmERR_BREAK(gcoSURF_GetSize(surfView->surf, &width, &height, &depth));

        /* Construct temp linear surface. */
        gcmERR_BREAK(gcoSURF_Construct(gcvNULL, width, height, 1, gcvSURF_BITMAP,
                                       gcvSURF_A8R8G8B8,
                                       gcvPOOL_DEFAULT,
                                       &tgtView.surf));

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted  = !yInverted;
        rlvArgs.uArgs.v2.rectSize.x = width;
        rlvArgs.uArgs.v2.rectSize.y = height;
        rlvArgs.uArgs.v2.numSlices  = 1;

        gcmERR_BREAK(gcoSURF_GetFormatInfo(surfView->surf, &srcFmtInfo));
        formatName = srcFmtInfo->formatName;

        if (srcFmtInfo->fmtClass == gcvFORMAT_CLASS_DEPTH)
        {
            rlvArgs.uArgs.v2.visualizeDepth = gcvTRUE;
            visualizeDepth = gcvTRUE;
        }

        /* Resolve render target to linear surface. */
        if (gcmIS_ERROR(gcoSURF_ResolveRect(surfView, &tgtView, &rlvArgs)))
        {
            gcsSURF_BLIT_ARGS cpuBltArgs;

            gcmASSERT(surfView->surf);
            gcoOS_ZeroMemory(&cpuBltArgs, sizeof(cpuBltArgs));
            cpuBltArgs.srcSurface  = surfView->surf;
            cpuBltArgs.srcX        = 0;
            cpuBltArgs.srcY        = 0;
            cpuBltArgs.srcZ        = surfView->firstSlice;
            cpuBltArgs.srcWidth    = width;
            cpuBltArgs.srcHeight   = height;
            cpuBltArgs.srcDepth    = 1;
            cpuBltArgs.srcNumSlice = surfView->numSlices;

            cpuBltArgs.dstSurface  = tgtView.surf;
            cpuBltArgs.dstX        = 0;
            cpuBltArgs.dstY        = 0;
            cpuBltArgs.dstZ        = tgtView.firstSlice;
            cpuBltArgs.dstWidth    = width;
            cpuBltArgs.dstHeight   = height;
            cpuBltArgs.dstDepth    = 1;
            cpuBltArgs.yReverse    = !yInverted;
            cpuBltArgs.dstNumSlice = tgtView.numSlices;

            if (gcmIS_ERROR(gcoSURF_BlitCPU(&cpuBltArgs)))
            {
                gscSURF_BLITDRAW_BLIT blitArgs;
                /* Construct temp linear surface. */
                gcmERR_BREAK(gcoSURF_Construct(gcvNULL, width, height, 1, gcvSURF_RENDER_TARGET,
                                               gcvSURF_A8R8G8B8,
                                               gcvPOOL_DEFAULT,
                                               &resolveRTView.surf));


                gcoOS_ZeroMemory(&blitArgs, sizeof(blitArgs));
                blitArgs.srcRect.right  = width;
                blitArgs.srcRect.bottom = height;
                blitArgs.dstRect.right  = width;
                blitArgs.dstRect.bottom = height;
                blitArgs.filterMode     = gcvTEXTURE_POINT;
                gcmERR_BREAK(gcoSURF_DrawBlit(surfView, &resolveRTView, &blitArgs));
                gcmERR_BREAK(gcoSURF_ResolveRect(&resolveRTView, &tgtView, &rlvArgs));
            }

            gcmERR_BREAK(status);
        }

        /* Commit resolve command and stall hardware. */
        gcmERR_BREAK(gcoHAL_Commit(gcvNULL, gcvTRUE));

        /* Lock for linear surface memory. */
        gcmERR_BREAK(gcoSURF_Lock(tgtView.surf, gcvNULL, logical));

        /* Query linear surface stride. */
        gcmERR_BREAK(gcoSURF_GetAlignedSize(tgtView.surf, gcvNULL, gcvNULL, &resolveStride));

        /* Allocate frame memory. */
        gcmERR_BREAK(gcoOS_Allocate(gcvNULL, width * height * 3, &frameMemory));

        /* Color format conversion: ARGB to RGB. */
        frameBGR = (gctUINT8_PTR)frameMemory;

        level[0] = '-';

        for (y = 0; y < (gctINT32)height; ++y)
        {
            frameARGB = (gctUINT8_PTR) logical[0] + y * resolveStride;

            for (x = 0; x < (gctINT32)width; ++x)
            {
                if (visualizeDepth)
                {
                    frameBGR[0] = frameARGB[1];
                    frameBGR[1] = frameARGB[2];
                    frameBGR[2] = frameARGB[3];
                }
                else
                {
                    frameBGR[0] = frameARGB[0];
                    frameBGR[1] = frameARGB[1];
                    frameBGR[2] = frameARGB[2];
                }
                frameARGB += 4;
                frameBGR  += 3;
            }
        }
    } while(gcvFALSE);

    if (gcvNULL != logical[0])
    {
        /* Unlock and destroy linear surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(tgtView.surf, logical[0]));
        logical[0] = gcvNULL;
    }

    if (tgtView.surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(tgtView.surf));
        tgtView.surf = gcvNULL;
    }

    if (resolveRTView.surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(resolveRTView.surf));
        resolveRTView.surf = gcvNULL;
    }

    /* Prepare tga file header. */
    tgaHeader[ 0] = 0;
    tgaHeader[ 1] = 0;
    tgaHeader[ 2] = 2;
    tgaHeader[ 3] = 0;
    tgaHeader[ 4] = 0;
    tgaHeader[ 5] = 0;
    tgaHeader[ 6] = 0;
    tgaHeader[ 7] = 0;
    tgaHeader[ 8] = 0;
    tgaHeader[ 9] = 0;
    tgaHeader[10] = 0;
    tgaHeader[11] = 0;
    tgaHeader[12] = (width & 0x00ff);
    tgaHeader[13] = (width & 0xff00) >> 8;
    tgaHeader[14] = (height & 0x00ff);
    tgaHeader[15] = (height & 0xff00) >> 8;
    tgaHeader[16] = 24;
    tgaHeader[17] = (0x01 << 5);

    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, gcdDUMP_PATH));
    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, fileName));
    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, formatName));
    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, level));

    /* Open tga file for write. */
    gcmVERIFY_OK(gcoOS_Open(gcvNULL, fName, gcvFILE_CREATE, &file));

    /* Write tga file header. */
    gcmVERIFY_OK(gcoOS_Write(gcvNULL, file, 18, tgaHeader));

    if (frameMemory)
    {
        /* Write pixel data. */
        gcmVERIFY_OK(gcoOS_Write(gcvNULL, file, width * height * 3, frameMemory));
        gcmOS_SAFE_FREE(gcvNULL, frameMemory);
    }

    if (gcvNULL != file)
    {
        /* Close tga file. */
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, file));
    }

    gcmFOOTER();
    return gcvSTATUS_OK;
}

gceSTATUS
gcChipUtilsDumpSurfaceRAW(
    __GLcontext *gc,
    gcsSURF_VIEW *surfView,
    gctCONST_STRING fileName,
    GLboolean yInverted
    )
{
    gctFILE file = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gcsSURF_VIEW tgtView = {gcvNULL, 0, 1};
    gcsSURF_VIEW resolveRTView = {gcvNULL, 0, 1};
    gctPOINTER logical[3]  = {0};
    gctUINT width = 0, height = 0, depth = 0;
    gctINT stride = 0, layerSize = 0;
    char level[6] = "+.raw";
    GLchar fName[__GLES_MAX_FILENAME_LEN]= {0};
    __GLsurfRawHead rawHead;
    const GLchar *formatName = "";

    gcmHEADER_ARG("gc=0x%x, surfView=0x%x, fileName=%s, yInverted=%d", gc, surfView, fileName, yInverted);

    do
    {
        gcsSURF_FORMAT_INFO_PTR srcFmtInfo;
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        gcmERR_BREAK(gcoSURF_GetSize(surfView->surf, &width, &height, &depth));
        gcmERR_BREAK(gcoSURF_GetFormatInfo(surfView->surf, &srcFmtInfo));
        formatName = srcFmtInfo->formatName;

        /* Construct temp linear surface. */
        gcmERR_BREAK(gcoSURF_Construct(gcvNULL, width, height, 1, gcvSURF_BITMAP,
                                       srcFmtInfo->format,
                                       gcvPOOL_DEFAULT,
                                       &tgtView.surf));

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted  = !yInverted;
        rlvArgs.uArgs.v2.rectSize.x = width;
        rlvArgs.uArgs.v2.rectSize.y = height;
        rlvArgs.uArgs.v2.numSlices  = 1;

        /* Resolve render target to linear surface. */
        if (gcmIS_ERROR(gcoSURF_ResolveRect(surfView, &tgtView, &rlvArgs)))
        {
            gcsSURF_BLIT_ARGS cpuBltArgs;

            gcmASSERT(surfView->surf);
            gcoOS_ZeroMemory(&cpuBltArgs, sizeof(cpuBltArgs));
            cpuBltArgs.srcSurface  = surfView->surf;
            cpuBltArgs.srcX        = 0;
            cpuBltArgs.srcY        = 0;
            cpuBltArgs.srcZ        = surfView->firstSlice;
            cpuBltArgs.srcWidth    = width;
            cpuBltArgs.srcHeight   = height;
            cpuBltArgs.srcDepth    = 1;
            cpuBltArgs.srcNumSlice = surfView->numSlices;

            cpuBltArgs.dstSurface  = tgtView.surf;
            cpuBltArgs.dstX        = 0;
            cpuBltArgs.dstY        = 0;
            cpuBltArgs.dstZ        = tgtView.firstSlice;
            cpuBltArgs.dstWidth    = width;
            cpuBltArgs.dstHeight   = height;
            cpuBltArgs.dstDepth    = 1;
            cpuBltArgs.yReverse    = !yInverted;
            cpuBltArgs.dstNumSlice = tgtView.numSlices;

            if (gcmIS_ERROR(gcoSURF_BlitCPU(&cpuBltArgs)))
            {
                gscSURF_BLITDRAW_BLIT blitArgs;
                /* Construct temp linear surface. */
                gcmERR_BREAK(gcoSURF_Construct(gcvNULL, width, height, 1, gcvSURF_RENDER_TARGET,
                                               srcFmtInfo->format,
                                               gcvPOOL_DEFAULT,
                                               &resolveRTView.surf));


                gcoOS_ZeroMemory(&blitArgs, sizeof(blitArgs));
                blitArgs.srcRect.right  = width;
                blitArgs.srcRect.bottom = height;
                blitArgs.dstRect.right  = width;
                blitArgs.dstRect.bottom = height;
                blitArgs.filterMode     = gcvTEXTURE_POINT;
                gcmERR_BREAK(gcoSURF_DrawBlit(surfView, &resolveRTView, &blitArgs));

                gcmERR_BREAK(gcoSURF_ResolveRect(&resolveRTView, &tgtView, &rlvArgs));
            }

            gcmERR_BREAK(status);
        }

        /* Commit resolve command and stall hardware. */
        gcmERR_BREAK(gcoHAL_Commit(gcvNULL, gcvTRUE));
        gcmERR_BREAK(gcoSURF_Lock(tgtView.surf, gcvNULL, logical));
        gcmERR_BREAK(gcoSURF_GetAlignedSize(tgtView.surf, gcvNULL, gcvNULL, &stride));
        gcmERR_BREAK(gcoSURF_GetInfo(tgtView.surf, gcvSURF_INFO_LAYERSIZE, &layerSize));

        level[0] = '-';
    } while (GL_FALSE);

    if (resolveRTView.surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(resolveRTView.surf));
        resolveRTView.surf = gcvNULL;
    }

    /* Prepare file header */
    __GL_MEMZERO(&rawHead, sizeof(rawHead));
    rawHead.width = width;
    rawHead.height = height;
    rawHead.stride = (GLuint)stride;
    rawHead.layerSize = (GLuint)layerSize;
    gcmVERIFY_OK(gcoOS_StrCopySafe(rawHead.formatName, 64, formatName));

    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, gcdDUMP_PATH));
    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, fileName));
    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, formatName));
    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, level));

    gcmVERIFY_OK(gcoOS_Open(gcvNULL, fName, gcvFILE_CREATE, &file));

    /* Write raw file header. */
    gcmVERIFY_OK(gcoOS_Write(gcvNULL, file, sizeof(rawHead), &rawHead));

    if (logical[0])
    {
        /* Write pixel data. */
        gcmVERIFY_OK(gcoOS_Write(gcvNULL, file, stride * height, logical[0]));

        /* Unlock linear surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(tgtView.surf, logical[0]));
        logical[0] = gcvNULL;
    }

    if (gcvNULL != file)
    {
        /* Close tga file. */
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, file));
    }

    if (tgtView.surf)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(tgtView.surf));
        tgtView.surf = gcvNULL;
    }

    gcmFOOTER();
    return gcvSTATUS_OK;
}

gceSTATUS
gcChipUtilsDumpSurfaceCOMPRAW(
    __GLcontext *gc,
    gcsSURF_VIEW *surfView,
    gctCONST_STRING fileName,
    GLboolean yInverted
    )
{
    gctFILE file = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER logical[3]  = {0};
    gctUINT height = 0;
    gctINT stride = 0;
    char level[10] = "+.compraw";
    GLchar fName[__GLES_MAX_FILENAME_LEN]= {0};
    const GLchar *formatName = "";

    gcmHEADER_ARG("gc=0x%x, surfView=0x%x, fileName=%s, yInverted=%d", gc, surfView, fileName, yInverted);

    do
    {
        gcsSURF_FORMAT_INFO_PTR srcFmtInfo;

        gcmERR_BREAK(gcoSURF_GetSize(surfView->surf, gcvNULL, &height, gcvNULL));
        gcmERR_BREAK(gcoSURF_GetAlignedSize(surfView->surf, gcvNULL, gcvNULL, &stride));
        gcmERR_BREAK(gcoSURF_GetFormatInfo(surfView->surf, &srcFmtInfo));
        formatName = srcFmtInfo->formatName;

        /* Commit resolve command and stall hardware. */
        gcmERR_BREAK(gcoHAL_Commit(gcvNULL, gcvTRUE));
        gcmERR_BREAK(gcoSURF_Lock(surfView->surf, gcvNULL, logical));

        level[0] = '-';
    } while (GL_FALSE);

    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, gcdDUMP_PATH));
    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, fileName));
    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, formatName));
    gcmVERIFY_OK(gcoOS_StrCatSafe(fName, __GLES_MAX_FILENAME_LEN, level));

    gcmVERIFY_OK(gcoOS_Open(gcvNULL, fName, gcvFILE_CREATE, &file));

    if (logical[0])
    {
        /* Write pixel data. */
        gcmVERIFY_OK(gcoOS_Write(gcvNULL, file, stride * height, logical[0]));

        /* Unlock linear surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(surfView->surf, logical[0]));
        logical[0] = gcvNULL;
    }

    if (gcvNULL != file)
    {
        /* Close tga file. */
        gcmVERIFY_OK(gcoOS_Close(gcvNULL, file));
    }

    gcmFOOTER();
    return gcvSTATUS_OK;
}

gceSTATUS
gcChipUtilsDumpSurface(
    __GLcontext *gc,
    gcsSURF_VIEW *surfView,
    gctCONST_STRING fileName,
    GLboolean yInverted,
    GLbitfield saveMask
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x, surfView=0x%x, fileName=%s, yInverted=%d, saveMask=0x%x",
                  gc, surfView, fileName, yInverted, saveMask);

#if defined(ANDROID)
    {
        enum DUMP_FLAG
        {
            DUMP_UNINITIALIZED,
            DUMP_TRUE,
            DUMP_FALSE,
        };
        static gctINT dump = DUMP_UNINITIALIZED;

        if (DUMP_UNINITIALIZED == dump)
        {
            if (gcvSTATUS_TRUE == gcoOS_DetectProcessByName(gcdDUMP_KEY))
            {
                dump = DUMP_TRUE;
            }
            else
            {
                dump = DUMP_FALSE;
            }
        }

        if (DUMP_TRUE != dump)
        {
            gcmFOOTER_NO();
            return gcvSTATUS_OK;
        }
    }
#endif

    if (saveMask & __GL_SAVE_SURF_AS_TGA)
    {
        gcChipUtilsDumpSurfaceTGA(gc, surfView, fileName, yInverted);
    }

    if (saveMask & __GL_SAVE_SURF_AS_RAW)
    {
        gcChipUtilsDumpSurfaceRAW(gc, surfView, fileName, yInverted);
    }

    if (saveMask & __GL_SAVE_SURF_AS_COMPRESSED)
    {
        gcChipUtilsDumpSurfaceCOMPRAW(gc, surfView, fileName, yInverted);
    }
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipUtilsVerifyRT(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gctUINT32 physical[3] = {0};
    gctPOINTER logical[3] = {gcvNULL};
    gctINT32 sliceSize = 0;
    GLuint index;
    gceSTATUS status = gcvSTATUS_OK;

    /* Flush the cache. */
    gcmONERROR(gcoSURF_Flush(gcvNULL));

    /* Commit command buffer. */
    gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));

    for (index = 0; index < gc->constants.shaderCaps.maxDrawBuffers; ++index)
    {
        gcsSURF_VIEW *rtView = &chipCtx->drawRtViews[index];
        if (rtView->surf)
        {
            gcmONERROR(gcoSURF_DisableTileStatus(rtView, gcvTRUE));
            gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));
            gcmONERROR(gcoSURF_Lock(rtView->surf, physical, logical));
            gcmONERROR(gcoSURF_Unlock(rtView->surf, logical[0]));
            gcmONERROR(gcoSURF_GetInfo(rtView->surf, gcvSURF_INFO_SLICESIZE, &sliceSize));

            gcmDUMP(gcvNULL, "#[info: verify rt%d", index);
            gcmDUMP_BUFFER(gcvNULL,
                           "verify",
                           physical[0] + gcChipGetSurfOffset(rtView),
                           (gctUINT8_PTR)logical[0] + gcChipGetSurfOffset(rtView),
                           0,
                           sliceSize);
        }

    }

    if (chipCtx->drawDepthView.surf)
    {
        gcmONERROR(gcoSURF_DisableTileStatus(&chipCtx->drawDepthView, gcvTRUE));
        gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));
        gcmONERROR(gcoSURF_Lock(chipCtx->drawDepthView.surf, physical, logical));
        gcmONERROR(gcoSURF_Unlock(chipCtx->drawDepthView.surf, logical[0]));
        gcmONERROR(gcoSURF_GetInfo(chipCtx->drawDepthView.surf, gcvSURF_INFO_SLICESIZE, &sliceSize));

        gcmDUMP(gcvNULL, "#[info: verify depth");
        gcmDUMP_BUFFER(gcvNULL,
                       "verify",
                       physical[0] + gcChipGetSurfOffset(&chipCtx->drawDepthView),
                       (gctUINT8_PTR)logical[0] + gcChipGetSurfOffset(&chipCtx->drawDepthView),
                       0,
                       sliceSize);
    }

OnError:
    return status;
}



gceSTATUS
gcChipUtilsVerifyImagesCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgramInstance *pInstance = program->curPgInstance;

    if (pInstance->extraImageUniformCount + program->imageUniformCount)
    {
        GLuint unit = 0;

        /* Flush the cache. */
        gcmONERROR(gcoSURF_Flush(gcvNULL));

        /* Commit command buffer. */
        gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));

        while (unit < gc->constants.shaderCaps.maxImageUnit)
        {
            __GLchipImageUnit2Uniform *pImageUnit2Uniform, *pExtraImageUnit2Uniform;
            __GLimageUnitState *imageUnit;

            pImageUnit2Uniform = &program->imageUnit2Uniform[unit];
            pExtraImageUnit2Uniform = &pInstance->extraImageUnit2Uniform[unit];
            imageUnit = &gc->state.image.imageUnit[unit];

            if (pImageUnit2Uniform->numUniform + pExtraImageUnit2Uniform->numUniform > 0)
            {
                gcsSURF_VIEW texView;
                gctINT32 offset = 0;
                gctINT32 sliceSize = 0;
                gctINT32 layerSize = 0;
                gctUINT32 physical[3] = {0};
                gctPOINTER logical[3] = {gcvNULL};
                gcsSURF_FORMAT_INFO_PTR formatInfo;
                __GLtextureObject *texObj = imageUnit->texObj;
                gctUINT8 i;
                GLuint sliceNumbers = 1;
                GLboolean layered = ((imageUnit->type != __GL_IMAGE_2D) && !imageUnit->singleLayered);

                texView = gcChipGetTextureSurface(chipCtx, texObj, layered, imageUnit->level, imageUnit->actualLayer);

                if ((imageUnit->type != __GL_IMAGE_2D) && !imageUnit->singleLayered)
                {
                    texView.firstSlice = 0;
                    sliceNumbers = texView.surf->requestD;
                }

                if (imageUnit->singleLayered)
                {
                    texView.numSlices = 1;
                }

                gcmONERROR(gcoSURF_Lock(texView.surf, physical, logical));
                gcmONERROR(gcoSURF_Unlock(texView.surf, gcvNULL));
                gcmONERROR(gcoSURF_GetInfo(texView.surf, gcvSURF_INFO_LAYERSIZE, &layerSize));
                gcmONERROR(gcoSURF_GetInfo(texView.surf, gcvSURF_INFO_SLICESIZE, &sliceSize));
                gcmONERROR(gcoSURF_GetFormatInfo(texView.surf, &formatInfo));

                offset = gcChipGetSurfOffset(&texView);
                for (i = 0; i < formatInfo->layers; i++)
                {
                    GLuint j;
                    gcmDUMP(gcvNULL, "#[info: verify image multi-layer%d", i);

                    for (j = 0; j < sliceNumbers; j++)
                    {
                        gcmDUMP(gcvNULL, "#[info: verify image slice[%d]", j);
                        gcmDUMP_BUFFER(gcvNULL,
                                       "verify",
                                       physical[0] + offset +  layerSize * i,
                                       (gctUINT8_PTR)logical[0] + offset + layerSize * i,
                                       0,
                                       sliceSize);

                        offset += sliceSize;
                    }
                }
            }
            unit++;
        }
    }

OnError:
    return status;

}


gceSTATUS
gcChipUtilsVerifyImages(
    __GLcontext *gc
    )
{
   __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
   gceSTATUS status;

   gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipUtilsVerifyImagesCB));
OnError:
    return status;
}

#if gcdFRAMEINFO_STATISTIC
extern GLbitfield g_dbgDumpImagePerDraw;

gceSTATUS
gcChipUtilsDumpTexture(
    __GLcontext *gc,
    __GLtextureObject *tex
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gctSTRING fileName = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    static char *txTypeStr[] = { "2D",
                                 "3D",
                                 "CUBE",
                                 "2D_A",
                                 "EXT",
                                 "2DMS",
                                 "2DMS_A",
                                 "CUBE_A"};
    GLuint level = 0;

    gctUINT32 frameCount, drawCount;

    gcoHAL_FrameInfoOps(chipCtx->hal,
                            gcvFRAMEINFO_FRAME_NUM,
                            gcvFRAMEINFO_OP_GET,
                            &frameCount);

    gcoHAL_FrameInfoOps(chipCtx->hal,
                            gcvFRAMEINFO_DRAW_NUM,
                            gcvFRAMEINFO_OP_GET,
                            &drawCount);

    /* increased in __glChipDrawBegin */
    drawCount--;

     /* Allocate memory for output file name string. */
    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, __GLES_MAX_FILENAME_LEN, (gctPOINTER *) &fileName));
    do
    {
        gctUINT slice = 0;
        gcsSURF_VIEW surfView;

        do
        {
            gctUINT fileNameOffset = 0;

            surfView = gcChipGetTextureSurface(chipCtx, tex, gcvFALSE, level, slice);
            if (!surfView.surf)
            {
                break;
            }

            gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName,
                                            __GLES_MAX_FILENAME_LEN,
                                            &fileNameOffset,
                                            "fID%04d_dID%04d(%s)_texID%04d[%s]_level%02d_slice%02d",
                                            frameCount,
                                            drawCount,
                                            (gc->shaderProgram.mode == __GLSL_MODE_COMPUTE ? "compute" : "draw"),
                                            tex->name,
                                            txTypeStr[tex->targetIndex],
                                            level,
                                            slice
                                            ));

            gcmERR_BREAK(gcChipUtilsDumpSurface(gc, &surfView, fileName, gcvFALSE, (g_dbgDumpImagePerDraw >> 16)));

            ++slice;
        } while (gcvTRUE);

        slice = 0;
        surfView = gcChipGetTextureSurface(chipCtx, tex, gcvFALSE, ++level, slice);
        if (!surfView.surf)
        {
            break;
        }
    } while(gcvTRUE);

    if (fileName)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, fileName));
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gcChipUtilsDumpRT(
    __GLcontext *gc,
    GLuint flag
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gctSTRING fileName;
    /* Build file name.*/
    gctUINT fileNameOffset = 0;
    GLuint index;
    gctUINT32 frameCount, drawCount;
    GLuint pID, ppID;
    static char *txTypeStr[] = {
        "2D",
        "3D",
        "CUBE",
        "2D_A",
        "EXT",
        "2DMS",
        "2DMS_A",
        "CUBE_A"
        };
    gcsSURF_VIEW *dsView = chipCtx->drawDepthView.surf ? &chipCtx->drawDepthView : &chipCtx->drawStencilView;

    /* Allocate memory for output file name string. */
    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, __GLES_MAX_FILENAME_LEN, (gctPOINTER *) &fileName));

    gcoHAL_FrameInfoOps(chipCtx->hal,
                            gcvFRAMEINFO_FRAME_NUM,
                            gcvFRAMEINFO_OP_GET,
                            &frameCount);

    gcoHAL_FrameInfoOps(chipCtx->hal,
                            gcvFRAMEINFO_DRAW_NUM,
                            gcvFRAMEINFO_OP_GET,
                            &drawCount);
    /* increased in begin */
    drawCount--;
    pID = gc->shaderProgram.currentProgram ? gc->shaderProgram.currentProgram->objectInfo.id : 0,
    ppID = gc->shaderProgram.currentProgram ? 0 : (gc->shaderProgram.boundPPO ? gc->shaderProgram.boundPPO->name : 0);

    switch (flag)
    {
    case (__GL_PERDRAW_DUMP_CLEAR_RT | __GL_PERDRAW_DUMP_CLEAR_DS):
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName,
                                            __GLES_MAX_FILENAME_LEN,
                                            &fileNameOffset,
                                            "fID%04d_dID%04d(clear)_pID%04d_ppID%04d_",
                                            frameCount,
                                            drawCount,
                                            pID,
                                            ppID));
        }
        break;
    case (__GL_PERDRAW_DUMP_DRAW_RT | __GL_PERDRAW_DUMP_DRAW_DS):
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName,
                                            __GLES_MAX_FILENAME_LEN,
                                            &fileNameOffset,
                                            "fID%04d_dID%04d(draw)_pID%04d_ppID%04d_",
                                            frameCount,
                                            drawCount,
                                            pID,
                                            ppID));
        }
    break;

    case (__GL_PERDRAW_DUMP_BLITFBO_RT | __GL_PERDRAW_DUMP_BLITFBO_DS):
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName,
                                            __GLES_MAX_FILENAME_LEN,
                                            &fileNameOffset,
                                            "fID%04d_dID%04d(blit)_pID%04d_ppID%04d_",
                                            frameCount,
                                            drawCount,
                                            pID,
                                            ppID));
        }
        break;

    default:
        GL_ASSERT(0);
    }

    for (index = 0; index < gc->constants.shaderCaps.maxDrawBuffers; ++index)
    {
        gcsSURF_VIEW *rtView = &chipCtx->drawRtViews[index];
        if (rtView->surf)
        {
            gctUINT fileNameOffset2 = fileNameOffset;

            if (gc->frameBuffer.drawFramebufObj->name)
            {
                __GLframebufferObject *fbo = gc->frameBuffer.drawFramebufObj;
                GLint attachIndex = __glMapAttachmentToIndex(fbo->drawBuffers[index]);
                __GLfboAttachPoint *attachPoint = &fbo->attachPoint[attachIndex];
                 __GLtextureObject *texObj = gcvNULL;

                if (attachPoint->objType == GL_TEXTURE)
                {
                    texObj = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
                }

                /* clear */
                gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName,
                                                __GLES_MAX_FILENAME_LEN,
                                                &fileNameOffset2,
                                                "fbo%04d(%s[%s]ID%04d_level%02d_face%d_layer%02d)_RT%d",
                                                gc->frameBuffer.drawFramebufObj->name,
                                                (attachPoint->objType == GL_RENDERBUFFER) ? "rbo" : "tex",
                                                (attachPoint->objType == GL_RENDERBUFFER) ? "" : txTypeStr[texObj->targetIndex],
                                                attachPoint->objName,
                                                attachPoint->level,
                                                attachPoint->face,
                                                attachPoint->layer,
                                                index));
            }
            else
            {
                GL_ASSERT(index == 0);
                gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName,
                                                __GLES_MAX_FILENAME_LEN,
                                                &fileNameOffset2,
                                                "window_RT"));
            }

            gcmVERIFY_OK(gcChipUtilsDumpSurface(gc, rtView, fileName, chipCtx->drawYInverted, (g_dbgDumpImagePerDraw >> 16)));
        }
    }

    if ((dsView->surf)
     && (((g_dbgDumpImagePerDraw & (__GL_PERDRAW_DUMP_CLEAR_DS
                                  | __GL_PERDRAW_DUMP_DRAW_DS
                                  | __GL_PERDRAW_DUMP_BLITFBO_DS))
       || (chipCtx->drawRTnum == 0))))
    {
        gctUINT fileNameOffset2 = fileNameOffset;

        if (gc->frameBuffer.drawFramebufObj->name)
        {
            __GLframebufferObject *fbo = gc->frameBuffer.drawFramebufObj;
            GLint attachIndex = __glMapAttachmentToIndex(GL_DEPTH_ATTACHMENT);
            __GLfboAttachPoint *attachPoint = &fbo->attachPoint[attachIndex];
            __GLtextureObject *texObj = gcvNULL;

            if (attachPoint->objType == GL_TEXTURE)
            {
                texObj = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
            }

            gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName,
                                            __GLES_MAX_FILENAME_LEN,
                                            &fileNameOffset2,
                                            "fbo%04d(%s[%s]ID%04d_level%02d_face%d_layer%02d)_depth",
                                            gc->frameBuffer.drawFramebufObj->name,
                                            (attachPoint->objType == GL_RENDERBUFFER) ? "rbo" : "tex",
                                            (attachPoint->objType == GL_RENDERBUFFER) ? "" : txTypeStr[texObj->targetIndex],
                                            attachPoint->objName,
                                            attachPoint->level,
                                            attachPoint->face,
                                            attachPoint->layer));
        }
        else
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName,
                                            __GLES_MAX_FILENAME_LEN,
                                            &fileNameOffset2,
                                            "window_depth"
                                            ));
        }
        gcmVERIFY_OK(gcChipUtilsDumpSurface(gc, dsView, fileName, chipCtx->drawYInverted, (g_dbgDumpImagePerDraw >> 16)));
    }
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, fileName));

    return gcvSTATUS_OK;
}

#endif

