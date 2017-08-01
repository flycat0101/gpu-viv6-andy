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


#ifndef _ICD_STRUCTS_H_
#define _ICD_STRUCTS_H_

typedef struct CLIicdDispatchTable_st  CLIicdDispatchTable;
typedef struct CLIplatform_st CLIplatform;

struct CLIicdDispatchTable_st
{
    void *entries[256];
    int entryCount;
};

struct CLIplatform_st
{
    CLIicdDispatchTable* dispatch;
};

extern cl_int cliIcdDispatchTableCreate(CLIicdDispatchTable **outDispatchTable);
extern void cliIcdDispatchTableDestroy(CLIicdDispatchTable *dispatchTable);

extern CLIicdDispatchTable * clgDispatchTable;
#endif /* _ICD_STRUCTS_H_ */
