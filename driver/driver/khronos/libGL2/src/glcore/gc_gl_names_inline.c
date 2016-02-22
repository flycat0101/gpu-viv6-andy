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


/*
** The following are inline functions used for object management
*/
#ifndef __gl_names_inline_
#define __gl_names_inline_

#include "gc_gl_context.h"


__GL_INLINE GLvoid *
__glGetObject(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    if (shared->linearTable)
    {
        if (id < shared->linearTableSize)
            return shared->linearTable[id];
        else
            return NULL;
    }
    else
    {
        __GLobjItem **objItem = __glLookupObjectItem(gc, shared, id);

        if (objItem && (*objItem))
            return (*objItem)->obj;
        else
            return NULL;
    }
}

__GL_INLINE GLvoid
__glAddObject(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id, GLvoid *obj)
{
    if (shared->linearTable)
    {
        __glCheckLinearTableSize(gc, shared, (id + 1));
    }

    if (shared->linearTable)
    {
        /* Save object pointer to the linear table */
        shared->linearTable[id] = obj;
    }
    else
    {
        /* Insert object to the hash table */
        __GLobjItem *item = __glFindObjItemNode(gc, shared, id);
        item->obj = obj;
    }
}

__GL_INLINE GLvoid
__glDeleteObject(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    if (shared->linearTable)
    {
        GLvoid *obj = NULL;

        /* Delete object from the linear table.
        */
        if (id < shared->linearTableSize) {
            obj = shared->linearTable[id];
        }
        if (obj) {
            if ((*shared->deleteObject)(gc, obj)) {
                /*
                ** Remove the Object from the linear table only if the object is truly deleted.
                */
                shared->linearTable[id] = NULL;
            }
        }
    }
    else
    {   /* Delete object from the hash table.
        */
        __GLobjItem **hpp = __glLookupObjectItem(gc, shared, id);

        if (hpp) {
            __GLobjItem *hp = *hpp;
            __GLobjItem *next = hp->next;

            if ((*shared->deleteObject)(gc, hp->obj)) {
                /*
                ** Remove the ObjItem from the link list only if the object is truly deleted.
                */
                (*gc->imports.free)(gc, hp);
                *hpp = next;
            }
        }
    }
}

__GL_INLINE GLvoid __glMarkNameUsed(__GLcontext *gc,
        __GLsharedObjectMachine *shared, GLuint id)
{
    __GLnameAllocation *allocated = shared->nameArray;
    __GLnameAllocation *temp;
    GLuint blstart, blend;

    if (allocated == NULL || id < allocated->start-1) {
        /* id is now the first entry in idlist */
        allocated = (__GLnameAllocation*)
            (*gc->imports.malloc)(gc, sizeof(__GLnameAllocation) );
        allocated->next = shared->nameArray;
        allocated->start = id;
        allocated->number = 1;
        shared->nameArray = allocated;
        return;
    }

    while (allocated) {
        blstart = allocated->start;
        blend = blstart + allocated->number;
        if (allocated->next == NULL || id <= blend ||
            id < allocated->next->start-1) {

            /* Id already marked as used */
            if (id < blend && id >= blstart) return;

            /* Id goes in this block, or right after it */
            if (id == blstart-1) {
                allocated->start--;
                allocated->number++;
                return;
            }

            if (id == blend) {
                /* bump the number in this block */
                allocated->number++;
                blend++;

                /* Merge with next node if adjacent */
                if (allocated->next && blend == allocated->next->start) {
                    allocated->number += allocated->next->number;
                    temp = allocated->next;
                    allocated->next = allocated->next->next;
                    if (temp) {
                        (*gc->imports.free)(gc, temp);
                    }
                }

                return;
            }

            /* Only other possibility is that id belongs in its own id
            ** right after this one.
            */
            temp = (__GLnameAllocation*)
            (*gc->imports.malloc)(gc, sizeof(__GLnameAllocation) );
            temp->next = allocated->next;
            allocated->next = temp;
            temp->start = id;
            temp->number = 1;
            return;
        }

        allocated = allocated->next;
    }
}

__GL_INLINE __GLimageUser * __glAddImageUser(__GLcontext *gc, __GLimageUser **pUserListHead, GLvoid *obj, CleanupFunc func)
{
    __GLimageUser *user = *pUserListHead;

    /* Check the userList to see if "obj" is already a user in the list */
    while (user) {
        if (user->imageUser == obj) {
            /* "obj" is already a user in the list */
            return user;
        }
        user = user->next;
    }

    /* Create the user if it is necessary */
    if (user == NULL) {
        user = (__GLimageUser *)(*gc->imports.malloc)(gc, sizeof(__GLimageUser) );
        user->imageUser = obj;
        user->cleanUp = func;
        user->next = NULL;
    }

    /* Insert the user into the userlist */
    user->next = *pUserListHead;
    *pUserListHead = user;

    return user;
}

__GL_INLINE GLvoid __glRemoveImageUser(__GLcontext *gc, __GLimageUser **pUserListHead, GLvoid *obj)
{
    __GLimageUser *user,  *prevUser;

    if (*pUserListHead == NULL) {
        return;
    }

    /* Check the userList to see if "obj" is a user in the list */
    user = prevUser = *pUserListHead;
    while (user) {
        if (user->imageUser == obj) {
            /* "obj" is in the userList, unlink the user from the list */
            if (user == *pUserListHead) {
                *pUserListHead = user->next;
            }
            else {
                prevUser->next = user->next;
            }

            (*gc->imports.free)(gc, user);
            break;
        }
        prevUser = user;
        user = user->next;
    }

}

__GL_INLINE GLvoid  __glFreeImageUserList(__GLcontext *gc, __GLimageUser **pUserListHead)
{
    __GLimageUser *user = *pUserListHead;
    __GLimageUser *next;
    while (user) {
        next = user->next;

        if(user->cleanUp)
            (*user->cleanUp)(gc, user->imageUser);

        (*gc->imports.free)(gc, user);
        user = next;
    }
}

#endif
