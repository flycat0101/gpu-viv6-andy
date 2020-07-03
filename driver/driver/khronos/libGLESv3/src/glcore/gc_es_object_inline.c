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


/*
** The following are inline functions used for object management
*/
#ifndef __gl_object_inline_
#define __gl_object_inline_

#include "gc_es_context.h"


__GL_INLINE GLvoid *
__glGetObject(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    GLvoid *result;

    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    if (shared->linearTable)
    {
        result = (id < shared->linearTableSize) ? shared->linearTable[id] : gcvNULL;
    }
    else
    {
        __GLobjItem **objItem = __glLookupObjectItem(gc, shared, id);
        result = (objItem && (*objItem)) ? (*objItem)->obj : gcvNULL;
    }

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }

    return result;
}

__GL_INLINE GLvoid
__glAddObject(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id, GLvoid *obj)
{
    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    if (shared->linearTable)
    {
        __glCheckLinearTableSize(gc, shared, (id == ~0u) ? id : (id + 1));
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
        if(item != gcvNULL)
            item->obj = obj;
    }

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }
}

__GL_INLINE GLvoid
__glDeleteObject(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    if (shared->linearTable)
    {
        GLvoid *obj = gcvNULL;

        /* Delete object from the linear table.
        */
        if (id < shared->linearTableSize)
        {
            obj = shared->linearTable[id];
        }

        if (obj)
        {
            GLboolean ret = (*shared->deleteObject)(gc, obj);

            /* Remove object from list if its name need to marked invalid immediately,
            ** or the object is truly deleted.
            */
            if (shared->immediateInvalid || ret)
            {
                __glDeleteNamesFrList(gc, shared, id, 1);
                shared->linearTable[id] = gcvNULL;
            }
        }
        else
        {
            __glDeleteNamesFrList(gc, shared, id, 1);
        }
    }
    else
    {
        /* Delete object from the hash table.
        */
        __GLobjItem **hpp = __glLookupObjectItem(gc, shared, id);

        if (hpp)
        {
            __GLobjItem *hp = *hpp;
            __GLobjItem *next = hp->next;

            GLboolean ret = (*shared->deleteObject)(gc, hp->obj);

            if (shared->immediateInvalid || ret)
            {
                /* Remove object from hash if its name need to marked invalid immediately,
                ** or the object is truly deleted.
                */
                __glDeleteNamesFrList(gc, shared, id, 1);
                gcmOS_SAFE_FREE(gcvNULL, hp);
                *hpp = next;
            }
        }
        else
        {
            __glDeleteNamesFrList(gc, shared, id, 1);
        }
    }

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }
}

__GL_INLINE GLint __glMarkNameUsed(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint id)
{
    __GLnameAllocation *allocated;
    __GLnameAllocation *temp;
    GLuint blstart, blend;
    GLuint uniqueId;

    if (shared->lock)
    {
        (*gc->imports.lockMutex)(shared->lock);
    }

    allocated = shared->nameArray;

    if (allocated == gcvNULL || id < allocated->start-1)
    {
        /* Id is now the first entry in idlist */
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLnameAllocation), (gctPOINTER *)&allocated)))
        {
            if (shared->lock)
            {
                (*gc->imports.unlockMutex)(shared->lock);
            }
            return gcvSTATUS_OUT_OF_MEMORY;
        }
        allocated->next = shared->nameArray;
        allocated->start = id;
        allocated->number = 1;
        shared->nameArray = allocated;
    }
    else
    {
        while (allocated)
        {
            blstart = allocated->start;
            blend = blstart + allocated->number;
            if (allocated->next == gcvNULL || id <= blend || id < allocated->next->start-1)
            {
                /* Id already marked as used */
                if (id < blend && id >= blstart)
                {
                    break;
                }
                /* Id goes in this block, or right after it */
                else if (id == blstart-1)
                {
                    allocated->start--;
                    allocated->number++;
                    break;
                }
                else if (id == blend)
                {
                    /* Bump the number in this block */
                    allocated->number++;
                    blend++;

                    /* Merge with next node if adjacent */
                    if (allocated->next && blend == allocated->next->start)
                    {
                        allocated->number += allocated->next->number;
                        temp = allocated->next;
                        allocated->next = allocated->next->next;
                        if (temp)
                        {
                            gcmOS_SAFE_FREE(gcvNULL, temp);
                        }
                    }
                    break;
                }
                else
                {
                    /* Only other possibility is that id belongs in its own id
                    ** right after this one.
                    */
                    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLnameAllocation), (gctPOINTER *)&temp)))
                    {
                        if (shared->lock)
                        {
                            (*gc->imports.unlockMutex)(shared->lock);
                        }
                        return gcvSTATUS_OUT_OF_MEMORY;
                    }

                    temp->next = allocated->next;
                    allocated->next = temp;
                    temp->start = id;
                    temp->number = 1;
                    break;
                }
            }

            allocated = allocated->next;
        }
    }

    uniqueId = ++shared->uniqueId;

    if (shared->lock)
    {
        (*gc->imports.unlockMutex)(shared->lock);
    }

    return uniqueId;
}

__GL_INLINE __GLimageUser * __glAddImageUser(__GLcontext *gc, __GLimageUser **pUserListHead, GLvoid *obj)
{
    __GLimageUser *user = *pUserListHead;

    /* Check the userList to see if "obj" is already a user in the list */
    while (user)
    {
        if (user->imageUser == obj)
        {
            /* "obj" is already a user in the list */
            ++user->refCount;
            return user;
        }
        user = user->next;
    }

    /* Create the user if it is necessary */

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLimageUser), (gctPOINTER *)&user)))
    {
        return gcvNULL;
    }
    else
    {
        user->imageUser = obj;
        user->refCount = 1;
        user->next = gcvNULL;

        /* Insert the user into the userlist */
        user->next = *pUserListHead;
        *pUserListHead = user;

        return user;
    }
}

__GL_INLINE GLvoid __glRemoveImageUser(__GLcontext *gc, __GLimageUser **pUserListHead, GLvoid *obj)
{
    __GLimageUser *user,  *prevUser;

    if (*pUserListHead == gcvNULL)
    {
        return;
    }

    /* Check the userList to see if "obj" is a user in the list */
    user = prevUser = *pUserListHead;
    while (user)
    {
        if (user->imageUser == obj)
        {
            if ((--user->refCount) == 0)
            {
                /* "obj" is in the userList, unlink the user from the list */
                if (user == *pUserListHead)
                {
                    *pUserListHead = user->next;
                }
                else
                {
                    prevUser->next = user->next;
                }

                gcmOS_SAFE_FREE(gcvNULL, user);
            }
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
    while (user)
    {
        next = user->next;
        gcmOS_SAFE_FREE(gcvNULL, user);
        user = next;
    }
}

#endif
