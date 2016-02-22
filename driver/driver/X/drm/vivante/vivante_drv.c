/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2014 - 2016 Vivante Corporation
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (C) 2014 - 2016 Vivante Corporation
*
*    This program is free software; you can redistribute it and/or
*    modify it under the terms of the GNU General Public License
*    as published by the Free Software Foundation; either version 2
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/


#include <linux/version.h>
#include <linux/module.h>

#include "drmP.h"
#include "vivante_drv.h"

#include "drm_pciids.h"

static struct platform_device platformdev={0};

static char platformdevicename[] = "Vivante GCCore";
static struct platform_device *pplatformdev=NULL;

static struct drm_driver driver = {

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38)
    .driver_features = DRIVER_USE_MTRR,
#else
    .driver_features = DRIVER_USE_MTRR | DRIVER_USE_PLATFORM_DEVICE,
#endif
    .reclaim_buffers = drm_core_reclaim_buffers,
    .fops = {
         .owner = THIS_MODULE,
         .open = drm_open,
         .release = drm_release,
         .unlocked_ioctl = drm_ioctl,
         .mmap = drm_mmap,
         .poll = drm_poll,
         .fasync = drm_fasync,
         .llseek = noop_llseek,
    },

    .name = DRIVER_NAME,
    .desc = DRIVER_DESC,
    .date = DRIVER_DATE,
    .major = DRIVER_MAJOR,
    .minor = DRIVER_MINOR,
    .patchlevel = DRIVER_PATCHLEVEL,
};

static int __init vivante_init(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38)
    int retcode;

    pplatformdev=platform_device_register_simple(platformdevicename,-1,NULL,0);
    if (pplatformdev==NULL) printk(KERN_ERR"Platform device is null\n");

    retcode=drm_platform_init(&driver,pplatformdev);

    return retcode;
#else
    platformdev.name = platformdevicename;
    driver.platform_device = &platformdev;
    return drm_init(&driver);
#endif
}

static void __exit vivante_exit(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38)
    if (pplatformdev) {
        drm_platform_exit(&driver,pplatformdev);
        platform_device_unregister(pplatformdev);
        pplatformdev=NULL;
    }
#else
    drm_exit(&driver);
#endif
}

module_init(vivante_init);
module_exit(vivante_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL and additional rights");
