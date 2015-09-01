/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __icdver_h_
#define __icdver_h_

#define MAJORVERSION 2
#define MINORVERSION 0

#define BUILDNUMBER 1
#define ORIGINALBUILDNUMBER 1

#define VERSION_MACRO(x,y,z)            #x "." #y "." #z
#define VERSION_ALL(x,y,z)              VERSION_MACRO(x,y,z)
#define VERSION_ICD                     VERSION_ALL(MAJORVERSION,MINORVERSION,ORIGINALBUILDNUMBER)
#define VER_PRODUCTVERSION_BUILD_VIV    VERSION_ICD
#define VER_PRODUCTVERSION_BUILD        MAJORVERSION,MINORVERSION,BUILDNUMBER
#endif /* __icdver_h_ */
