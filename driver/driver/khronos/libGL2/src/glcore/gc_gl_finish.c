/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_gl_context.h"
#include "dri/viv_lock.h"
#include "gc_gl_debug.h"


#if __GL_ENABLE_HW_NULL
extern GLuint drawCount;
#endif

GLvoid APIENTRY __glim_Finish(GLvoid)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    {
        GLuint closeLogFile = GL_FALSE;
        if(closeLogFile)
        {
            dbgLogFileClose();
        }
    }

    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Finish", DT_GLnull);
#endif

#if ((defined(DEBUG) || defined(_DEBUG)))

    /* because player won't call SwapBuffer, we have to reset those counts here */
    if(gdbg_logFrameDrawCount)
    {
        if( (gdbg_frameCount >= gdbg_logFrameStartIndex) &&
             (gdbg_frameCount < gdbg_logFrameEndIndex) )
        {
            if(NULL == gdbg_logFrameFile)
            {
                gdbg_logFrameFile =fopen("frameDrawCount.log","w");
                GL_ASSERT(gdbg_logFrameFile);
                fprintf(gdbg_logFrameFile, "frameIndex  drawImmeCount drawArrayCount drawDlistCount  drawCount \n");
            }
            fprintf(gdbg_logFrameFile, "%d \t\t %d \t\t %d \t\t %d \t\t %d \n", gdbg_frameCount,
                gdbg_drawImmeCount, gdbg_drawArrayCount, gdbg_drawDlistCount, gdbg_drawCount);
            fflush(gdbg_logFrameFile);

            if( gdbg_frameCount == gdbg_logFrameEndIndex -1 )
            {
                    GL_ASSERT(gdbg_logFrameFile);
                    fclose(gdbg_logFrameFile);
                    gdbg_logFrameFile = NULL;
            }
        }
    }

    gdbg_drawCount = 0;
    gdbg_drawImmeCount = 0;
    gdbg_drawArrayCount = 0;
    gdbg_drawDlistCount = 0;
    // frameCount++;  /* If use player, uncomment this line */
    if(gdbg_frameCount == gdbg_frameCountTarget)
        gdbg_frameCount = gdbg_frameCount;
#endif

#if __GL_ENABLE_HW_NULL
    drawCount = 0;
#endif
    __GL_VERTEX_BUFFER_FLUSH(gc);

   LINUX_LOCK_FRAMEBUFFER(gc);

   (*gc->dp.finish)(gc);

   LINUX_UNLOCK_FRAMEBUFFER(gc);
}


GLvoid APIENTRY __glim_Flush(GLvoid)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    {
        GLuint closeLogFile = GL_FALSE;
        if(closeLogFile)
        {
            dbgLogFileClose();
        }

    }

    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_Flush", DT_GLnull);
#endif

    __GL_VERTEX_BUFFER_FLUSH(gc);

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.flush)(gc);

    LINUX_UNLOCK_FRAMEBUFFER(gc);

}

