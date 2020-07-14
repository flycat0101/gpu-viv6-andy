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


#ifndef __GC_HAL_KERNEL_RESOURCE_MANAGER_INFO_H
#define __GC_HAL_KERNEL_RESOURCE_MANAGER_INFO_H

int
gc_info_show(void *data);

int
gc_clients_show(void *data);

int
gc_meminfo_show(void *data);

int
gc_db_show(void *data);

int
gc_version_show(void *data);

int
gc_vidmem_show(void *data);

int
gc_dump_trigger_show(void *data);

int
gc_vidmem_write(void *data);

int
gc_dump_trigger_write(void *data);

#endif
