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


#ifndef __gc_hal_kernel_debugfs_h_
#define __gc_hal_kernel_debugfs_h_

typedef struct _gcsINFO
{
    const char *    name;
    int             (*read)(void);
    int             (*write)(void *);
    char *          help_string;
}
gcsINFO;

gctINT
gckDEBUGFS_Initialize(
    /* [in] */ void *data
    );

gctINT
gckDEBUGFS_Terminate(void);

#endif /* __gc_hal_kernel_debugfs_h_ */
