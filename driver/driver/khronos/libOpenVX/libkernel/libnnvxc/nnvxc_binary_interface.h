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


#ifndef __NN_VXC_BINARY_INTERFACE_H__
#define __NN_VXC_BINARY_INTERFACE_H__

typedef enum _nnvxc_kernel_enum
{
    NNVXC_KERNEL_NUM /*NNVXC_KERNEL_NUM should be the last item in the enum*/
}
nnvxc_kernel_enum;

typedef void * (*GetBinaryPtr_FUNC)(nnvxc_kernel_enum, unsigned int *);
#endif
