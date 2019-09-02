/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/



#ifndef __GC_VX_JSON_H__
#define __GC_VX_JSON_H__

#include "gc_vx_common.h"

EXTERN_C_BEGIN


#include <stddef.h>

/* vxoJson Types: */
#define vxoJson_Invalid (0)
#define vxoJson_False  (1 << 0)
#define vxoJson_True   (1 << 1)
#define vxoJson_NULL   (1 << 2)
#define vxoJson_Number (1 << 3)
#define vxoJson_String (1 << 4)
#define vxoJson_Array  (1 << 5)
#define vxoJson_Object (1 << 6)
#define vxoJson_Raw    (1 << 7) /* raw json */

#define vxoJson_IsReference 256
#define vxoJson_StringIsConst 512

/* The vxcJson structure: */
typedef struct _vx_c_JSON_
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct _vx_c_JSON_ *next;
    struct _vx_c_JSON_ *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct _vx_c_JSON_ *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==vxoJson_String  and type == vxoJson_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use vxoJson_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==vxoJson_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} vxcJSON;

typedef struct vxoJson_Hooks
{
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} vxoJson_Hooks;


/* Limits how deeply nested arrays/objects can be before vxcJson rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1000
#endif

/* Supply malloc, realloc and free functions to vxcJSON */
VX_INTERNAL_API void vxoJson_InitHooks(vxoJson_Hooks* hooks);

/* Memory Management:
 the caller is always responsible to free the results from all variants of vxoJson_Parse (with vxoJson_Delete) and vxoJson_Print (with stdlib free, vxoJson_Hooks.free_fn, or vxoJson_free as appropriate). The exception is vxoJson_PrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a vxcJson object you can interrogate. */
VX_INTERNAL_API vxcJSON * vxoJson_Parse(const char *value);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match vxoJson_GetErrorPtr(). */
VX_INTERNAL_API vxcJSON * vxoJson_ParseWithOpts(const char *value, const char **return_parse_end, vx_bool require_null_terminated);

/* Render a vxcJson entity to text for transfer/storage. */
VX_INTERNAL_API char * vxoJson_Print(const vxcJSON *item);
/* Render a vxcJson entity to text for transfer/storage without any formatting. */
VX_INTERNAL_API char * vxoJson_PrintUnformatted(const vxcJSON *item);
/* Render a vxcJson entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
VX_INTERNAL_API char * vxoJson_PrintBuffered(const vxcJSON *item, int prebuffer, vx_bool fmt);
/* Render a vxcJson entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE: vxcJson is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
VX_INTERNAL_API vx_bool vxoJson_PrintPreallocated(vxcJSON *item, char *buffer, const int length, const vx_bool format);
/* Delete a vxcJson entity and all subentities. */
VX_INTERNAL_API void vxoJson_Delete(vxcJSON *c);

/* Returns the number of items in an array (or object). */
VX_INTERNAL_API int vxoJson_GetArraySize(const vxcJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
VX_INTERNAL_API vxcJSON * vxoJson_GetArrayItem(const vxcJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
VX_INTERNAL_API vxcJSON * vxoJson_GetObjectItem(const vxcJSON * const object, const char * const string);
VX_INTERNAL_API vxcJSON * vxoJson_GetObjectItemCaseSensitive(const vxcJSON * const object, const char * const string);
VX_INTERNAL_API vx_bool vxoJson_HasObjectItem(const vxcJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when vxoJson_Parse() returns 0. 0 when vxoJson_Parse() succeeds. */
VX_INTERNAL_API const char * vxoJson_GetErrorPtr(void);

/* Check if the item is a string and return its valuestring */
VX_INTERNAL_API char * vxoJson_GetStringValue(vxcJSON *item);

/* These functions check the type of an item */
VX_INTERNAL_API vx_bool vxoJson_IsInvalid(const vxcJSON * const item);
VX_INTERNAL_API vx_bool vxoJson_IsFalse(const vxcJSON * const item);
VX_INTERNAL_API vx_bool vxoJson_IsTrue(const vxcJSON * const item);
VX_INTERNAL_API vx_bool vxoJson_IsBool(const vxcJSON * const item);
VX_INTERNAL_API vx_bool vxoJson_IsNull(const vxcJSON * const item);
VX_INTERNAL_API vx_bool vxoJson_IsNumber(const vxcJSON * const item);
VX_INTERNAL_API vx_bool vxoJson_IsString(const vxcJSON * const item);
VX_INTERNAL_API vx_bool vxoJson_IsArray(const vxcJSON * const item);
VX_INTERNAL_API vx_bool vxoJson_IsObject(const vxcJSON * const item);
VX_INTERNAL_API vx_bool vxoJson_IsRaw(const vxcJSON * const item);

/* These calls create a vxcJson item of the appropriate type. */
VX_INTERNAL_API vxcJSON * vxoJson_CreateNull(void);
VX_INTERNAL_API vxcJSON * vxoJson_CreateTrue(void);
VX_INTERNAL_API vxcJSON * vxoJson_CreateFalse(void);
VX_INTERNAL_API vxcJSON * vxoJson_CreateBool(vx_bool boolean);
VX_INTERNAL_API vxcJSON * vxoJson_CreateNumber(double num);
VX_INTERNAL_API vxcJSON * vxoJson_CreateString(const char *string);
/* raw json */
VX_INTERNAL_API vxcJSON * vxoJson_CreateRaw(const char *raw);
VX_INTERNAL_API vxcJSON * vxoJson_CreateArray(void);
VX_INTERNAL_API vxcJSON * vxoJson_CreateObject(void);

/* Create a string where valuestring references a string so
 * it will not be freed by vxoJson_Delete */
VX_INTERNAL_API vxcJSON * vxoJson_CreateStringReference(const char *string);
/* Create an object/arrray that only references it's elements so
 * they will not be freed by vxoJson_Delete */
VX_INTERNAL_API vxcJSON * vxoJson_CreateObjectReference(const vxcJSON *child);
VX_INTERNAL_API vxcJSON * vxoJson_CreateArrayReference(const vxcJSON *child);

/* These utilities create an Array of count items. */
VX_INTERNAL_API vxcJSON * vxoJson_CreateIntArray(const int *numbers, int count);
VX_INTERNAL_API vxcJSON * vxoJson_CreateFloatArray(const float *numbers, int count);
VX_INTERNAL_API vxcJSON * vxoJson_CreateDoubleArray(const double *numbers, int count);
VX_INTERNAL_API vxcJSON * vxoJson_CreateStringArray(const char **strings, int count);

/* Append item to the specified array/object. */
VX_INTERNAL_API void vxoJson_AddItemToArray(vxcJSON *array, vxcJSON *item);
VX_INTERNAL_API vx_bool vxoJson_AddItemToObject(vxcJSON *object, const char *string, vxcJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the vxcJson object.
 * WARNING: When this function was used, make sure to always check that (item->type & vxoJson_StringIsConst) is zero before
 * writing to `item->string` */
VX_INTERNAL_API vx_bool vxoJson_AddItemToObjectCS(vxcJSON *object, const char *string, vxcJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing vxcJson to a new vxcJson, but don't want to corrupt your existing vxcJson. */
VX_INTERNAL_API vx_bool vxoJson_AddItemReferenceToArray(vxcJSON *array, vxcJSON *item);
VX_INTERNAL_API vx_bool vxoJson_AddItemReferenceToObject(vxcJSON *object, const char *string, vxcJSON *item);

/* Remove/Detatch items from Arrays/Objects. */
VX_INTERNAL_API vxcJSON * vxoJson_DetachItemViaPointer(vxcJSON *parent, vxcJSON * const item);
VX_INTERNAL_API vxcJSON * vxoJson_DetachItemFromArray(vxcJSON *array, int which);
VX_INTERNAL_API void vxoJson_DeleteItemFromArray(vxcJSON *array, int which);
VX_INTERNAL_API vxcJSON * vxoJson_DetachItemFromObject(vxcJSON *object, const char *string);
VX_INTERNAL_API vxcJSON * vxoJson_DetachItemFromObjectCaseSensitive(vxcJSON *object, const char *string);
VX_INTERNAL_API void vxoJson_DeleteItemFromObject(vxcJSON *object, const char *string);
VX_INTERNAL_API void vxoJson_DeleteItemFromObjectCaseSensitive(vxcJSON *object, const char *string);

/* Update array items. */
VX_INTERNAL_API void vxoJson_InsertItemInArray(vxcJSON *array, int which, vxcJSON *newitem); /* Shifts pre-existing items to the right. */
VX_INTERNAL_API vx_bool vxoJson_ReplaceItemViaPointer(vxcJSON * const parent, vxcJSON * const item, vxcJSON * replacement);
VX_INTERNAL_API void vxoJson_ReplaceItemInArray(vxcJSON *array, int which, vxcJSON *newitem);
VX_INTERNAL_API void vxoJson_ReplaceItemInObject(vxcJSON *object,const char *string,vxcJSON *newitem);
VX_INTERNAL_API void vxoJson_ReplaceItemInObjectCaseSensitive(vxcJSON *object,const char *string,vxcJSON *newitem);

/* Duplicate a vxcJson item */
VX_INTERNAL_API vxcJSON * vxoJson_Duplicate(const vxcJSON *item, vx_bool recurse);
/* Duplicate will create a new, identical vxcJson item to the one you pass, in new memory that will
need to be released. With recurse!=0, it will duplicate any children connected to the item.
The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two vxcJson items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
VX_INTERNAL_API vx_bool vxoJson_Compare(const vxcJSON * const a, const vxcJSON * const b, const vx_bool case_sensitive);


VX_INTERNAL_API void vxoJson_Minify(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
VX_INTERNAL_API vxcJSON* vxoJson_AddNullToObject(vxcJSON * const object, const char * const name);
VX_INTERNAL_API vxcJSON* vxoJson_AddTrueToObject(vxcJSON * const object, const char * const name);
VX_INTERNAL_API vxcJSON* vxoJson_AddFalseToObject(vxcJSON * const object, const char * const name);
VX_INTERNAL_API vxcJSON* vxoJson_AddBoolToObject(vxcJSON * const object, const char * const name, const vx_bool boolean);
VX_INTERNAL_API vxcJSON* vxoJson_AddNumberToObject(vxcJSON * const object, const char * const name, const double number);
VX_INTERNAL_API vxcJSON* vxoJson_AddStringToObject(vxcJSON * const object, const char * const name, const char * const string);
VX_INTERNAL_API vxcJSON* vxoJson_AddRawToObject(vxcJSON * const object, const char * const name, const char * const raw);
VX_INTERNAL_API vxcJSON* vxoJson_AddObjectToObject(vxcJSON * const object, const char * const name);
VX_INTERNAL_API vxcJSON* vxoJson_AddArrayToObject(vxcJSON * const object, const char * const name);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define vxoJson_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
/* helper for the vxoJson_SetNumberValue macro */
VX_INTERNAL_API double vxoJson_SetNumberHelper(vxcJSON *object, double number);
#define vxoJson_SetNumberValue(object, number) ((object != NULL) ? vxoJson_SetNumberHelper(object, (double)number) : (number))

/* Macro for iterating over an array or object */
#define vxoJson_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with vxoJson_InitHooks */
VX_INTERNAL_API void * vxoJson_malloc(size_t size);
VX_INTERNAL_API void vxoJson_free(void *object);

EXTERN_C_END

#endif


