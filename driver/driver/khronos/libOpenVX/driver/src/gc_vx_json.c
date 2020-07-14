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



/* vxcJSON */

/* disable warnings about old C89 functions in MSVC */
#if !defined(_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

#include "gc_vx_json.h"

#define _GC_OBJ_ZONE            gcdZONE_VX_OTHERS

/* define our own boolean type */

typedef struct {
    const unsigned char *json;
    size_t position;
} error;
VX_PRIVATE_API error global_error = { NULL, 0 };

VX_INTERNAL_API const char * vxoJson_GetErrorPtr(void)
{
    return (const char*) (global_error.json + global_error.position);
}

VX_INTERNAL_API char * vxoJson_GetStringValue(vxcJSON *item) {
    if (!vxoJson_IsString(item)) {
        return NULL;
    }

    return item->valuestring;
}

/* Case insensitive string comparison, doesn't consider two NULL pointers equal though */
VX_PRIVATE_API int case_insensitive_strcmp(const unsigned char *string1, const unsigned char *string2)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    for(; tolower(*string1) == tolower(*string2); (void)string1++, string2++)
    {
        if (*string1 == '\0')
        {
            return 0;
        }
    }

    return tolower(*string1) - tolower(*string2);
}

typedef struct internal_hooks
{
    void *(*allocate)(size_t size);
    void (*deallocate)(void *pointer);
    void *(*reallocate)(void *pointer, size_t size);
} internal_hooks;

#if defined(_MSC_VER)
VX_PRIVATE_API void *internal_malloc(size_t size)
{
    return malloc(size);
}
VX_PRIVATE_API void internal_free(void *pointer)
{
    free(pointer);
}
VX_PRIVATE_API void *internal_realloc(void *pointer, size_t size)
{
    return realloc(pointer, size);
}
#else
#define internal_malloc malloc
#define internal_free free
#define internal_realloc realloc
#endif

VX_PRIVATE_API internal_hooks global_hooks = { internal_malloc, internal_free, internal_realloc };

VX_PRIVATE_API unsigned char* vxoJson_strdup(const unsigned char* string, const internal_hooks * const hooks)
{
    size_t length = 0;
    unsigned char *copy = NULL;

    gcmHEADER_ARG("string=%s, hooks=%p", string, hooks);

    if (string == NULL)
    {
        gcmFOOTER_NO();
        return NULL;
    }

    length = strlen((const char*)string) + sizeof("");
    copy = (unsigned char*)hooks->allocate(length);
    if (copy == NULL)
    {
        gcmFOOTER_NO();
        return NULL;
    }
    memcpy(copy, string, length);

    gcmFOOTER_ARG("%s", copy);
    return copy;
}

VX_INTERNAL_API void vxoJson_InitHooks(vxoJson_Hooks* hooks)
{
    gcmHEADER_ARG("hooks=%p", hooks);
    if (hooks == NULL)
    {
        /* Reset hooks */
        global_hooks.allocate = malloc;
        global_hooks.deallocate = free;
        global_hooks.reallocate = realloc;
        gcmFOOTER_NO();
        return;
    }

    global_hooks.allocate = malloc;
    if (hooks->malloc_fn != NULL)
    {
        global_hooks.allocate = hooks->malloc_fn;
    }

    global_hooks.deallocate = free;
    if (hooks->free_fn != NULL)
    {
        global_hooks.deallocate = hooks->free_fn;
    }

    /* use realloc only if both free and malloc are used */
    global_hooks.reallocate = NULL;
    if ((global_hooks.allocate == malloc) && (global_hooks.deallocate == free))
    {
        global_hooks.reallocate = realloc;
    }
    gcmFOOTER_NO();
}

/* Internal constructor. */
VX_PRIVATE_API vxcJSON *vxoJson_New_Item(const internal_hooks * const hooks)
{
    vxcJSON* node = (vxcJSON*)hooks->allocate(sizeof(vxcJSON));
    gcmHEADER_ARG("hooks=%p", hooks);
    if (node)
    {
        memset(node, '\0', sizeof(vxcJSON));
    }

    gcmFOOTER_ARG("node=%p", node);
    return node;
}

/* Delete a vxcJson structure. */
VX_INTERNAL_API void vxoJson_Delete(vxcJSON *item)
{
    vxcJSON *next = NULL;
    gcmHEADER_ARG("item=%p", item);

    while (item != NULL)
    {
        next = item->next;
        if (!(item->type & vxoJson_IsReference) && (item->child != NULL))
        {
            vxoJson_Delete(item->child);
        }
        if (!(item->type & vxoJson_IsReference) && (item->valuestring != NULL))
        {
            global_hooks.deallocate(item->valuestring);
        }
        if (!(item->type & vxoJson_StringIsConst) && (item->string != NULL))
        {
            global_hooks.deallocate(item->string);
        }
        global_hooks.deallocate(item);
        item = next;
    }
    gcmFOOTER_NO();
}

/* get the decimal point character of the current locale */
VX_PRIVATE_API unsigned char get_decimal_point(void)
{
#ifdef ENABLE_LOCALES
    struct lconv *lconv = localeconv();
    return (unsigned char) lconv->decimal_point[0];
#else
    return '.';
#endif
}

typedef struct
{
    const unsigned char *content;
    size_t length;
    size_t offset;
    size_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
    internal_hooks hooks;
} parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting with 1) */
#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
/* check if the buffer can be accessed at the given index (starting with 0) */
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
/* get a pointer to the buffer at the position */
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

/* Parse the input text to generate a number, and populate the result into item. */
VX_PRIVATE_API vx_bool parse_number(vxcJSON * const item, parse_buffer * const input_buffer)
{
    double number = 0;
    unsigned char *after_end = NULL;
    unsigned char number_c_string[64];
    unsigned char decimal_point = get_decimal_point();
    size_t i = 0;

    gcmHEADER_ARG("item=%p, input_buffer=%p", item, input_buffer);

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = 0; (i < (sizeof(number_c_string) - 1)) && can_access_at_index(input_buffer, i); i++)
    {
        switch (buffer_at_offset(input_buffer)[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case 'e':
            case 'E':
                number_c_string[i] = buffer_at_offset(input_buffer)[i];
                break;

            case '.':
                number_c_string[i] = decimal_point;
                break;

            default:
                goto loop_end;
        }
    }
loop_end:
    number_c_string[i] = '\0';

    number = strtod((const char*)number_c_string, (char**)&after_end);
    if (number_c_string == after_end)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e; /* parse_error */
    }

    item->valuedouble = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX)
    {
        item->valueint = INT_MAX;
    }
    else if (number <= INT_MIN)
    {
        item->valueint = INT_MIN;
    }
    else
    {
        item->valueint = (int)number;
    }

    item->type = vxoJson_Number;

    input_buffer->offset += (size_t)(after_end - number_c_string);

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

/* don't ask me, but the original vxoJson_SetNumberValue returns an integer or double */
VX_INTERNAL_API double vxoJson_SetNumberHelper(vxcJSON *object, double number)
{
    if (number >= INT_MAX)
    {
        object->valueint = INT_MAX;
    }
    else if (number <= INT_MIN)
    {
        object->valueint = INT_MIN;
    }
    else
    {
        object->valueint = (int)number;
    }

    return object->valuedouble = number;
}

typedef struct
{
    unsigned char *buffer;
    size_t length;
    size_t offset;
    size_t depth; /* current nesting depth (for formatted printing) */
    vx_bool noalloc;
    vx_bool format; /* is this print a formatted print */
    internal_hooks hooks;
} printbuffer;

/* realloc printbuffer if necessary to have at least "needed" bytes more */
VX_PRIVATE_API unsigned char* ensure(printbuffer * const p, size_t needed)
{
    unsigned char *newbuffer = NULL;
    size_t newsize = 0;

    gcmHEADER_ARG("p=%p, needed=0x%lx", p, needed);

    if ((p == NULL) || (p->buffer == NULL))
    {
        gcmFOOTER_NO();
        return NULL;
    }

    if ((p->length > 0) && (p->offset >= p->length))
    {
        /* make sure that offset is valid */
        gcmFOOTER_NO();
        return NULL;
    }

    if (needed > INT_MAX)
    {
        /* sizes bigger than INT_MAX are currently not supported */
        gcmFOOTER_NO();
        return NULL;
    }

    needed += p->offset + 1;
    if (needed <= p->length)
    {
        gcmFOOTER_NO();
        return p->buffer + p->offset;
    }

    if (p->noalloc)
    {
        gcmFOOTER_NO();
        return NULL;
    }

    /* calculate new buffer size */
    if (needed > (INT_MAX / 2))
    {
        /* overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX)
        {
            newsize = INT_MAX;
        }
        else
        {
            gcmFOOTER_NO();
            return NULL;
        }
    }
    else
    {
        newsize = needed * 2;
    }

    if (p->hooks.reallocate != NULL)
    {
        /* reallocate with realloc if available */
        newbuffer = (unsigned char*)p->hooks.reallocate(p->buffer, newsize);
        if (newbuffer == NULL)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;
            gcmFOOTER_NO();
            return NULL;
        }
    }
    else
    {
        /* otherwise reallocate manually */
        newbuffer = (unsigned char*)p->hooks.allocate(newsize);
        if (!newbuffer)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;
            gcmFOOTER_NO();
            return NULL;
        }
        if (newbuffer)
        {
            memcpy(newbuffer, p->buffer, p->offset + 1);
        }
        p->hooks.deallocate(p->buffer);
    }
    p->length = newsize;
    p->buffer = newbuffer;

    gcmFOOTER_NO();
    return newbuffer + p->offset;
}

/* calculate the new length of the string in a printbuffer and update the offset */
VX_PRIVATE_API void update_offset(printbuffer * const buffer)
{
    const unsigned char *buffer_pointer = NULL;
    gcmHEADER_ARG("buffer=%p", buffer);
    if ((buffer == NULL) || (buffer->buffer == NULL))
    {
        gcmFOOTER_NO();
        return;
    }
    buffer_pointer = buffer->buffer + buffer->offset;

    buffer->offset += strlen((const char*)buffer_pointer);
    gcmFOOTER_NO();
}

/* Render the number nicely from the given item into a string. */
VX_PRIVATE_API vx_bool print_number(const vxcJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output_pointer = NULL;
    double d = item->valuedouble;
    int length = 0;
    size_t i = 0;
    unsigned char number_buffer[26]; /* temporary buffer to print the number into */
    unsigned char decimal_point = get_decimal_point();
    double test;

    gcmHEADER_ARG("item=%p, output_buffer=%p", item, output_buffer);

    if (output_buffer == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    /* This checks for NaN and Infinity */
    if ((d * 0) != 0)
    {
        length = sprintf((char*)number_buffer, "null");
    }
    else
    {
        /* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
        length = sprintf((char*)number_buffer, "%1.15g", d);

        /* Check whether the original double can be recovered */
        if ((sscanf((char*)number_buffer, "%lg", &test) != 1) || ((double)test != d))
        {
            /* If not, print with 17 decimal places of precision */
            length = sprintf((char*)number_buffer, "%1.17g", d);
        }
    }

    /* sprintf failed or buffer overrun occured */
    if ((length < 0) || (length > (int)(sizeof(number_buffer) - 1)))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    /* reserve appropriate space in the output */
    output_pointer = ensure(output_buffer, (size_t)length + sizeof(""));
    if (output_pointer == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    /* copy the printed number to the output and replace locale
     * dependent decimal point with '.' */
    for (i = 0; i < ((size_t)length); i++)
    {
        if (number_buffer[i] == decimal_point)
        {
            output_pointer[i] = '.';
            continue;
        }

        output_pointer[i] = number_buffer[i];
    }
    output_pointer[i] = '\0';

    output_buffer->offset += (size_t)length;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

/* parse 4 digit hexadecimal number */
VX_PRIVATE_API unsigned parse_hex4(const unsigned char * const input)
{
    unsigned int h = 0;
    size_t i = 0;

    for (i = 0; i < 4; i++)
    {
        /* parse digit */
        if ((input[i] >= '0') && (input[i] <= '9'))
        {
            h += (unsigned int) input[i] - '0';
        }
        else if ((input[i] >= 'A') && (input[i] <= 'F'))
        {
            h += (unsigned int) 10 + input[i] - 'A';
        }
        else if ((input[i] >= 'a') && (input[i] <= 'f'))
        {
            h += (unsigned int) 10 + input[i] - 'a';
        }
        else /* invalid */
        {
            return 0;
        }

        if (i < 3)
        {
            /* shift left to make place for the next nibble */
            h = h << 4;
        }
    }

    return h;
}

/* converts a UTF-16 literal to UTF-8
 * A literal can be one or two sequences of the form \uXXXX */
VX_PRIVATE_API unsigned char utf16_literal_to_utf8(const unsigned char * const input_pointer, const unsigned char * const input_end, unsigned char **output_pointer)
{
    long unsigned int codepoint = 0;
    unsigned int first_code = 0;
    const unsigned char *first_sequence = input_pointer;
    unsigned char utf8_length = 0;
    unsigned char utf8_position = 0;
    unsigned char sequence_length = 0;
    unsigned char first_byte_mark = 0;

    gcmHEADER_ARG("input_pointer=%s, input_end=%s, output_pointer=%p", input_pointer, input_end, output_pointer);

    if ((input_end - first_sequence) < 6)
    {
        /* input ends unexpectedly */
        goto fail;
    }

    /* get the first utf16 sequence */
    first_code = parse_hex4(first_sequence + 2);

    /* check that the code is valid */
    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
    {
        goto fail;
    }

    /* UTF16 surrogate pair */
    if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
    {
        const unsigned char *second_sequence = first_sequence + 6;
        unsigned int second_code = 0;
        sequence_length = 12; /* \uXXXX\uXXXX */

        if ((input_end - second_sequence) < 6)
        {
            /* input ends unexpectedly */
            goto fail;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
        {
            /* missing second half of the surrogate pair */
            goto fail;
        }

        /* get the second utf16 sequence */
        second_code = parse_hex4(second_sequence + 2);
        /* check that the code is valid */
        if ((second_code < 0xDC00) || (second_code > 0xDFFF))
        {
            /* invalid second half of the surrogate pair */
            goto fail;
        }


        /* calculate the unicode codepoint from the surrogate pair */
        codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    }
    else
    {
        sequence_length = 6; /* \uXXXX */
        codepoint = first_code;
    }

    /* encode as UTF-8
     * takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint < 0x80)
    {
        /* normal ascii, encoding 0xxxxxxx */
        utf8_length = 1;
    }
    else if (codepoint < 0x800)
    {
        /* two bytes, encoding 110xxxxx 10xxxxxx */
        utf8_length = 2;
        first_byte_mark = 0xC0; /* 11000000 */
    }
    else if (codepoint < 0x10000)
    {
        /* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
        utf8_length = 3;
        first_byte_mark = 0xE0; /* 11100000 */
    }
    else if (codepoint <= 0x10FFFF)
    {
        /* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8_length = 4;
        first_byte_mark = 0xF0; /* 11110000 */
    }
    else
    {
        /* invalid unicode codepoint */
        goto fail;
    }

    /* encode as utf8 */
    for (utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0; utf8_position--)
    {
        /* 10xxxxxx */
        (*output_pointer)[utf8_position] = (unsigned char)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    /* encode first byte */
    if (utf8_length > 1)
    {
        (*output_pointer)[0] = (unsigned char)((codepoint | first_byte_mark) & 0xFF);
    }
    else
    {
        (*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
    }

    *output_pointer += utf8_length;

    gcmFOOTER_ARG("sequence_length=0x%x", sequence_length);
    return sequence_length;

fail:
    gcmFOOTER_ARG("%d", 0);
    return 0;
}

/* Parse the input text into an unescaped cinput, and populate item. */
VX_PRIVATE_API vx_bool parse_string(vxcJSON * const item, parse_buffer * const input_buffer)
{
    const unsigned char *input_pointer = buffer_at_offset(input_buffer) + 1;
    const unsigned char *input_end = buffer_at_offset(input_buffer) + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    gcmHEADER_ARG("item=%p, input_buffer=%p", item, input_buffer);

    /* not a string */
    if (buffer_at_offset(input_buffer)[0] != '\"')
    {
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        while (((size_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
        {
            /* is escape sequence */
            if (input_end[0] == '\\')
            {
                if ((size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
                {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (((size_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
        {
            goto fail; /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocation_length = (size_t) (input_end - buffer_at_offset(input_buffer)) - skipped_bytes;
        output = (unsigned char*)input_buffer->hooks.allocate(allocation_length + sizeof(""));
        if (output == NULL)
        {
            goto fail; /* allocation failure */
        }
    }

    output_pointer = output;
    /* loop through the string literal */
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        /* escape sequence */
        else
        {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1)
            {
                goto fail;
            }

            switch (input_pointer[1])
            {
                case 'b':
                    *output_pointer++ = '\b';
                    break;
                case 'f':
                    *output_pointer++ = '\f';
                    break;
                case 'n':
                    *output_pointer++ = '\n';
                    break;
                case 'r':
                    *output_pointer++ = '\r';
                    break;
                case 't':
                    *output_pointer++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *output_pointer++ = input_pointer[1];
                    break;

                /* UTF-16 literal */
                case 'u':
                    sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
                    if (sequence_length == 0)
                    {
                        /* failed to convert UTF16-literal to UTF-8 */
                        goto fail;
                    }
                    break;

                default:
                    goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    /* zero terminate the output */
    *output_pointer = '\0';

    item->type = vxoJson_String;
    item->valuestring = (char*)output;

    input_buffer->offset = (size_t) (input_end - input_buffer->content);
    input_buffer->offset++;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;

fail:
    if (output != NULL)
    {
        input_buffer->hooks.deallocate(output);
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    gcmFOOTER_ARG("%d", vx_false_e);
    return vx_false_e;
}

/* Render the cstring provided to an escaped version that can be printed. */
VX_PRIVATE_API vx_bool print_string_ptr(const unsigned char * const input, printbuffer * const output_buffer)
{
    const unsigned char *input_pointer = NULL;
    unsigned char *output = NULL;
    unsigned char *output_pointer = NULL;
    size_t output_length = 0;
    /* numbers of additional characters needed for escaping */
    size_t escape_characters = 0;

    gcmHEADER_ARG("input=%s, output_buffer=%p", input, output_buffer);

    if (output_buffer == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    /* empty string */
    if (input == NULL)
    {
        output = ensure(output_buffer, sizeof("\"\""));
        if (output == NULL)
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
        strncpy((char*)output, "\"\"", sizeof("\"\""));

        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (input_pointer = input; *input_pointer; input_pointer++)
    {
        switch (*input_pointer)
        {
            case '\"':
            case '\\':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                /* one character escape sequence */
                escape_characters++;
                break;
            default:
                if (*input_pointer < 32)
                {
                    /* UTF-16 escape sequence uXXXX */
                    escape_characters += 5;
                }
                break;
        }
    }
    output_length = (size_t)(input_pointer - input) + escape_characters;

    output = ensure(output_buffer, output_length + sizeof("\"\""));
    if (output == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    /* no characters have to be escaped */
    if (escape_characters == 0)
    {
        output[0] = '\"';
        memcpy(output + 1, input, output_length);
        output[output_length + 1] = '\"';
        output[output_length + 2] = '\0';

        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }

    output[0] = '\"';
    output_pointer = output + 1;
    /* copy the string */
    for (input_pointer = input; *input_pointer != '\0'; (void)input_pointer++, output_pointer++)
    {
        if ((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\'))
        {
            /* normal character, copy */
            *output_pointer = *input_pointer;
        }
        else
        {
            /* character needs to be escaped */
            *output_pointer++ = '\\';
            switch (*input_pointer)
            {
                case '\\':
                    *output_pointer = '\\';
                    break;
                case '\"':
                    *output_pointer = '\"';
                    break;
                case '\b':
                    *output_pointer = 'b';
                    break;
                case '\f':
                    *output_pointer = 'f';
                    break;
                case '\n':
                    *output_pointer = 'n';
                    break;
                case '\r':
                    *output_pointer = 'r';
                    break;
                case '\t':
                    *output_pointer = 't';
                    break;
                default:
                    /* escape and print as unicode codepoint */
                    sprintf((char*)output_pointer, "u%04x", *input_pointer);
                    output_pointer += 4;
                    break;
            }
        }
    }
    output[output_length + 1] = '\"';
    output[output_length + 2] = '\0';
    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

/* Invoke print_string_ptr (which is useful) on an item. */
VX_PRIVATE_API vx_bool print_string(const vxcJSON * const item, printbuffer * const p)
{
    return print_string_ptr((unsigned char*)item->valuestring, p);
}

/* Predeclare these prototypes. */
VX_PRIVATE_API vx_bool parse_value(vxcJSON * const item, parse_buffer * const input_buffer);
VX_PRIVATE_API vx_bool print_value(const vxcJSON * const item, printbuffer * const output_buffer);
VX_PRIVATE_API vx_bool parse_array(vxcJSON * const item, parse_buffer * const input_buffer);
VX_PRIVATE_API vx_bool print_array(const vxcJSON * const item, printbuffer * const output_buffer);
VX_PRIVATE_API vx_bool parse_object(vxcJSON * const item, parse_buffer * const input_buffer);
VX_PRIVATE_API vx_bool print_object(const vxcJSON * const item, printbuffer * const output_buffer);

/* Utility to jump whitespace and cr/lf */
VX_PRIVATE_API parse_buffer *buffer_skip_whitespace(parse_buffer * const buffer)
{
    gcmHEADER_ARG("buffer=%p", buffer);

    if ((buffer == NULL) || (buffer->content == NULL))
    {
        gcmFOOTER_NO();
        return NULL;
    }

    while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
    {
       buffer->offset++;
    }

    if (buffer->offset == buffer->length)
    {
        buffer->offset--;
    }

    gcmFOOTER_ARG("buffer=%p", buffer);
    return buffer;
}

/* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
VX_PRIVATE_API parse_buffer *skip_utf8_bom(parse_buffer * const buffer)
{
    gcmHEADER_ARG("buffer=%p", buffer);
    if ((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0))
    {
        gcmFOOTER_NO();
        return NULL;
    }

    if (can_access_at_index(buffer, 4) && (strncmp((const char*)buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) == 0))
    {
        buffer->offset += 3;
    }

    gcmFOOTER_ARG("buffer=%p", buffer);
    return buffer;
}

/* Parse an object - create a new root, and populate. */
VX_INTERNAL_API vxcJSON * vxoJson_ParseWithOpts(const char *value, const char **return_parse_end, vx_bool require_null_terminated)
{
    parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    vxcJSON *item = NULL;

    gcmHEADER_ARG("value=%s, return_parse_end=%p, require_null_terminated=0x%x", value, return_parse_end, require_null_terminated);

    /* reset error position */
    global_error.json = NULL;
    global_error.position = 0;

    if (value == NULL)
    {
        goto fail;
    }

    buffer.content = (const unsigned char*)value;
    buffer.length = strlen((const char*)value) + sizeof("");
    buffer.offset = 0;
    buffer.hooks = global_hooks;

    item = vxoJson_New_Item(&global_hooks);
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }

    if (!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        /* parse failure. ep is set. */
        goto fail;
    }

    /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (require_null_terminated)
    {
        buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
        {
            goto fail;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char*)buffer_at_offset(&buffer);
    }

    gcmFOOTER_ARG("item=%p", item);
    return item;

fail:
    if (item != NULL)
    {
        vxoJson_Delete(item);
    }

    if (value != NULL)
    {
        error local_error;
        local_error.json = (const unsigned char*)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length)
        {
            local_error.position = buffer.offset;
        }
        else if (buffer.length > 0)
        {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL)
        {
            *return_parse_end = (const char*)local_error.json + local_error.position;
        }

        global_error = local_error;
    }

    gcmFOOTER_NO();
    return NULL;
}

/* Default options for vxoJson_Parse */
VX_INTERNAL_API vxcJSON * vxoJson_Parse(const char *value)
{
    return vxoJson_ParseWithOpts(value, 0, 0);
}

#define cjson_min(a, b) ((a < b) ? a : b)

VX_PRIVATE_API unsigned char *print(const vxcJSON * const item, vx_bool format, const internal_hooks * const hooks)
{
    VX_PRIVATE_API const size_t default_buffer_size = 256;
    printbuffer buffer[1];
    unsigned char *printed = NULL;

    gcmHEADER_ARG("item=%p, format=0x%x, hooks=%p", item, format, hooks);

    memset(buffer, 0, sizeof(buffer));

    /* create buffer */
    buffer->buffer = (unsigned char*) hooks->allocate(default_buffer_size);
    buffer->length = default_buffer_size;
    buffer->format = format;
    buffer->hooks = *hooks;
    if (buffer->buffer == NULL)
    {
        goto fail;
    }

    /* print the value */
    if (!print_value(item, buffer))
    {
        goto fail;
    }
    update_offset(buffer);

    /* check if reallocate is available */
    if (hooks->reallocate != NULL)
    {
        printed = (unsigned char*) hooks->reallocate(buffer->buffer, buffer->offset + 1);
        if (printed == NULL) {
            goto fail;
        }
        buffer->buffer = NULL;
    }
    else /* otherwise copy the JSON over to a new buffer */
    {
        printed = (unsigned char*) hooks->allocate(buffer->offset + 1);
        if (printed == NULL)
        {
            goto fail;
        }
        memcpy(printed, buffer->buffer, cjson_min(buffer->length, buffer->offset + 1));
        printed[buffer->offset] = '\0'; /* just to be sure */

        /* free the buffer */
        hooks->deallocate(buffer->buffer);
    }

    gcmFOOTER_ARG("printed=%s", printed);
    return printed;

fail:
    if (buffer->buffer != NULL)
    {
        hooks->deallocate(buffer->buffer);
    }

    gcmFOOTER_NO();
    return NULL;
}

/* Render a vxcJson item/entity/structure to text. */
VX_INTERNAL_API char * vxoJson_Print(const vxcJSON *item)
{
    return (char*)print(item, vx_true_e, &global_hooks);
}

VX_INTERNAL_API char * vxoJson_PrintUnformatted(const vxcJSON *item)
{
    return (char*)print(item, vx_false_e, &global_hooks);
}

VX_INTERNAL_API char * vxoJson_PrintBuffered(const vxcJSON *item, int prebuffer, vx_bool fmt)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };

    if (prebuffer < 0)
    {
        return NULL;
    }

    p.buffer = (unsigned char*)global_hooks.allocate((size_t)prebuffer);
    if (!p.buffer)
    {
        return NULL;
    }

    p.length = (size_t)prebuffer;
    p.offset = 0;
    p.noalloc = vx_false_e;
    p.format = fmt;
    p.hooks = global_hooks;

    if (!print_value(item, &p))
    {
        global_hooks.deallocate(p.buffer);
        return NULL;
    }

    return (char*)p.buffer;
}

VX_INTERNAL_API vx_bool vxoJson_PrintPreallocated(vxcJSON *item, char *buf, const int len, const vx_bool fmt)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };

    if ((len < 0) || (buf == NULL))
    {
        return vx_false_e;
    }

    p.buffer = (unsigned char*)buf;
    p.length = (size_t)len;
    p.offset = 0;
    p.noalloc = vx_true_e;
    p.format = fmt;
    p.hooks = global_hooks;

    return print_value(item, &p);
}

/* Parser core - when encountering text, process appropriately. */
VX_PRIVATE_API vx_bool parse_value(vxcJSON * const item, parse_buffer * const input_buffer)
{
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return vx_false_e; /* no input */
    }

    /* parse the different types of values */
    /* null */
    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = vxoJson_NULL;
        input_buffer->offset += 4;
        return vx_true_e;
    }
    /* vx_false_e */
    if (can_read(input_buffer, 5) && (strncmp((const char*)buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = vxoJson_False;
        input_buffer->offset += 5;
        return vx_true_e;
    }
    /* vx_true_e */
    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = vxoJson_True;
        item->valueint = 1;
        input_buffer->offset += 4;
        return vx_true_e;
    }
    /* string */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"'))
    {
        return parse_string(item, input_buffer);
    }
    /* number */
    if (can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        return parse_number(item, input_buffer);
    }
    /* array */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
    {
        return parse_array(item, input_buffer);
    }
    /* object */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {
        return parse_object(item, input_buffer);
    }

    return vx_false_e;
}

/* Render a value to text. */
VX_PRIVATE_API vx_bool print_value(const vxcJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output = NULL;

    if ((item == NULL) || (output_buffer == NULL))
    {
        return vx_false_e;
    }

    switch ((item->type) & 0xFF)
    {
        case vxoJson_NULL:
            output = ensure(output_buffer, 5);
            if (output == NULL)
            {
                return vx_false_e;
            }
            strncpy((char*)output, "null", 5);
            return vx_true_e;

        case vxoJson_False:
            output = ensure(output_buffer, 6);
            if (output == NULL)
            {
                return vx_false_e;
            }
            strncpy((char*)output, "false",6);
            return vx_true_e;

        case vxoJson_True:
            output = ensure(output_buffer, 5);
            if (output == NULL)
            {
                return vx_false_e;
            }
            strncpy((char*)output, "true",5);
            return vx_true_e;

        case vxoJson_Number:
            return print_number(item, output_buffer);

        case vxoJson_Raw:
        {
            size_t raw_length = 0;
            if (item->valuestring == NULL)
            {
                return vx_false_e;
            }

            raw_length = strlen(item->valuestring) + sizeof("");
            output = ensure(output_buffer, raw_length);
            if (output == NULL)
            {
                return vx_false_e;
            }
            memcpy(output, item->valuestring, raw_length);
            return vx_true_e;
        }

        case vxoJson_String:
            return print_string(item, output_buffer);

        case vxoJson_Array:
            return print_array(item, output_buffer);

        case vxoJson_Object:
            return print_object(item, output_buffer);

        default:
            return vx_false_e;
    }
}

/* Build an array from input text. */
VX_PRIVATE_API vx_bool parse_array(vxcJSON * const item, parse_buffer * const input_buffer)
{
    vxcJSON *head = NULL; /* head of the linked list */
    vxcJSON *current_item = NULL;
    gcmHEADER_ARG("item=%p, input_buffer=%p", item, input_buffer);

    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e; /* to deeply nested */
    }
    input_buffer->depth++;

    if (buffer_at_offset(input_buffer)[0] != '[')
    {
        /* not an array */
        goto fail;
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
    {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        vxcJSON *new_item = vxoJson_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse next value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    }
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || buffer_at_offset(input_buffer)[0] != ']')
    {
        goto fail; /* expected end of array */
    }

success:
    input_buffer->depth--;

    item->type = vxoJson_Array;
    item->child = head;

    input_buffer->offset++;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;

fail:
    if (head != NULL)
    {
        vxoJson_Delete(head);
    }

    gcmFOOTER_ARG("%d", vx_false_e);
    return vx_false_e;
}

/* Render an array to text */
VX_PRIVATE_API vx_bool print_array(const vxcJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    vxcJSON *current_element = item->child;

    gcmHEADER_ARG("item=%p, output_buffer=%p", item, output_buffer);

    if (output_buffer == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    /* Compose the output array. */
    /* opening square bracket */
    output_pointer = ensure(output_buffer, 1);
    if (output_pointer == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    *output_pointer = '[';
    output_buffer->offset++;
    output_buffer->depth++;

    while (current_element != NULL)
    {
        if (!print_value(current_element, output_buffer))
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
        update_offset(output_buffer);
        if (current_element->next)
        {
            length = (size_t) (output_buffer->format ? 2 : 1);
            output_pointer = ensure(output_buffer, length + 1);
            if (output_pointer == NULL)
            {
                gcmFOOTER_ARG("%d", vx_false_e);
                return vx_false_e;
            }
            *output_pointer++ = ',';
            if(output_buffer->format)
            {
                *output_pointer++ = ' ';
            }
            *output_pointer = '\0';
            output_buffer->offset += length;
        }
        current_element = current_element->next;
    }

    output_pointer = ensure(output_buffer, 2);
    if (output_pointer == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    *output_pointer++ = ']';
    *output_pointer = '\0';
    output_buffer->depth--;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

/* Build an object from the text. */
VX_PRIVATE_API vx_bool parse_object(vxcJSON * const item, parse_buffer * const input_buffer)
{
    vxcJSON *head = NULL; /* linked list head */
    vxcJSON *current_item = NULL;

    gcmHEADER_ARG("item=%p, input_buffer=%p", item, input_buffer);

    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e; /* to deeply nested */
    }
    input_buffer->depth++;

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '{'))
    {
        goto fail; /* not an object */
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        vxcJSON *new_item = vxoJson_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse the name of the child */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_string(current_item, input_buffer))
        {
            goto fail; /* faile to parse name */
        }
        buffer_skip_whitespace(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        current_item->string = current_item->valuestring;
        current_item->valuestring = NULL;

        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    }
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail; /* expected end of object */
    }

success:
    input_buffer->depth--;

    item->type = vxoJson_Object;
    item->child = head;

    input_buffer->offset++;
    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;

fail:
    if (head != NULL)
    {
        vxoJson_Delete(head);
    }
    gcmFOOTER_ARG("%d", vx_false_e);
    return vx_false_e;
}

/* Render an object to text. */
VX_PRIVATE_API vx_bool print_object(const vxcJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output_pointer = NULL;
    size_t length = 0;
    vxcJSON *current_item = item->child;
    gcmHEADER_ARG("item=%p, output_buffer=%p", item, output_buffer);

    if (output_buffer == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    /* Compose the output: */
    length = (size_t) (output_buffer->format ? 2 : 1); /* fmt: {\n */
    output_pointer = ensure(output_buffer, length + 1);
    if (output_pointer == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    *output_pointer++ = '{';
    output_buffer->depth++;
    if (output_buffer->format)
    {
        *output_pointer++ = '\n';
    }
    output_buffer->offset += length;

    while (current_item)
    {
        if (output_buffer->format)
        {
            size_t i;
            output_pointer = ensure(output_buffer, output_buffer->depth);
            if (output_pointer == NULL)
            {
                gcmFOOTER_ARG("%d", vx_false_e);
                return vx_false_e;
            }
            for (i = 0; i < output_buffer->depth; i++)
            {
                *output_pointer++ = '\t';
            }
            output_buffer->offset += output_buffer->depth;
        }

        /* print key */
        if (!print_string_ptr((unsigned char*)current_item->string, output_buffer))
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
        update_offset(output_buffer);

        length = (size_t) (output_buffer->format ? 2 : 1);
        output_pointer = ensure(output_buffer, length);
        if (output_pointer == NULL)
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
        *output_pointer++ = ':';
        if (output_buffer->format)
        {
            *output_pointer++ = '\t';
        }
        output_buffer->offset += length;

        /* print value */
        if (!print_value(current_item, output_buffer))
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
        update_offset(output_buffer);

        /* print comma if not last */
        length = (size_t) ((output_buffer->format ? 1 : 0) + (current_item->next ? 1 : 0));
        output_pointer = ensure(output_buffer, length + 1);
        if (output_pointer == NULL)
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }
        if (current_item->next)
        {
            *output_pointer++ = ',';
        }

        if (output_buffer->format)
        {
            *output_pointer++ = '\n';
        }
        *output_pointer = '\0';
        output_buffer->offset += length;

        current_item = current_item->next;
    }

    output_pointer = ensure(output_buffer, output_buffer->format ? (output_buffer->depth + 1) : 2);
    if (output_pointer == NULL)
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }
    if (output_buffer->format)
    {
        size_t i;
        for (i = 0; i < (output_buffer->depth - 1); i++)
        {
            *output_pointer++ = '\t';
        }
    }
    *output_pointer++ = '}';
    *output_pointer = '\0';
    output_buffer->depth--;

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

/* Get Array size/item / object item. */
VX_INTERNAL_API int vxoJson_GetArraySize(const vxcJSON *array)
{
    vxcJSON *child = NULL;
    size_t size = 0;

    gcmHEADER_ARG("array=%p", array);

    if (array == NULL)
    {
        gcmFOOTER_ARG("%d", 0);
        return 0;
    }

    child = array->child;

    while(child != NULL)
    {
        size++;
        child = child->next;
    }

    /* FIXME: Can overflow here. Cannot be fixed without breaking the API */

    gcmFOOTER_ARG("size=0x%lx", size);
    return (int)size;
}

VX_PRIVATE_API vxcJSON* get_array_item(const vxcJSON *array, size_t index)
{
    vxcJSON *current_child = NULL;

    if (array == NULL)
    {
        return NULL;
    }

    current_child = array->child;
    while ((current_child != NULL) && (index > 0))
    {
        index--;
        current_child = current_child->next;
    }

    return current_child;
}

VX_INTERNAL_API vxcJSON * vxoJson_GetArrayItem(const vxcJSON *array, int index)
{
    if (index < 0)
    {
        return NULL;
    }

    return get_array_item(array, (size_t)index);
}

VX_PRIVATE_API vxcJSON *get_object_item(const vxcJSON * const object, const char * const name, const vx_bool case_sensitive)
{
    vxcJSON *current_element = NULL;

    gcmHEADER_ARG("object=%p, name=%s, case_sensitive=0x%x", object, name, case_sensitive);

    if ((object == NULL) || (name == NULL))
    {
        gcmFOOTER_NO();
        return NULL;
    }

    current_element = object->child;
    if (case_sensitive)
    {
        while ((current_element != NULL) && (strcmp(name, current_element->string) != 0))
        {
            current_element = current_element->next;
        }
    }
    else
    {
        while ((current_element != NULL) && (case_insensitive_strcmp((const unsigned char*)name, (const unsigned char*)(current_element->string)) != 0))
        {
            current_element = current_element->next;
        }
    }

    gcmFOOTER_ARG("current_element=%p", current_element);
    return current_element;
}

VX_INTERNAL_API vxcJSON * vxoJson_GetObjectItem(const vxcJSON * const object, const char * const string)
{
    return get_object_item(object, string, vx_false_e);
}

VX_INTERNAL_API vxcJSON * vxoJson_GetObjectItemCaseSensitive(const vxcJSON * const object, const char * const string)
{
    return get_object_item(object, string, vx_true_e);
}

VX_INTERNAL_API vx_bool vxoJson_HasObjectItem(const vxcJSON *object, const char *string)
{
    return vxoJson_GetObjectItem(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
VX_PRIVATE_API void suffix_object(vxcJSON *prev, vxcJSON *item)
{
    prev->next = item;
    item->prev = prev;
}

/* Utility for handling references. */
VX_PRIVATE_API vxcJSON *create_reference(const vxcJSON *item, const internal_hooks * const hooks)
{
    vxcJSON *reference = NULL;
    gcmHEADER_ARG("item=%p, hooks=%p", item, hooks);
    if (item == NULL)
    {
        gcmFOOTER_NO();
        return NULL;
    }

    reference = vxoJson_New_Item(hooks);
    if (reference == NULL)
    {
        gcmFOOTER_NO();
        return NULL;
    }

    memcpy(reference, item, sizeof(vxcJSON));
    reference->string = NULL;
    reference->type |= vxoJson_IsReference;
    reference->next = reference->prev = NULL;

    gcmFOOTER_ARG("reference=%p", reference);
    return reference;
}

VX_PRIVATE_API vx_bool add_item_to_array(vxcJSON *array, vxcJSON *item)
{
    vxcJSON *child = NULL;

    gcmHEADER_ARG("array=%p, item=%p", array, item);

    if ((item == NULL) || (array == NULL))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    child = array->child;

    if (child == NULL)
    {
        /* list is empty, start new one */
        array->child = item;
    }
    else
    {
        /* append to the end */
        while (child->next)
        {
            child = child->next;
        }
        suffix_object(child, item);
    }

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

/* Add item to array/object. */
VX_INTERNAL_API void vxoJson_AddItemToArray(vxcJSON *array, vxcJSON *item)
{
    add_item_to_array(array, item);
}

#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
    #pragma GCC diagnostic push
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
/* helper function to cast away const */
VX_PRIVATE_API void* cast_away_const(const void* string)
{
    return (void*)string;
}
#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
    #pragma GCC diagnostic pop
#endif


VX_PRIVATE_API vx_bool add_item_to_object(vxcJSON * const object, const char * const string, vxcJSON * const item, const internal_hooks * const hooks, const vx_bool constant_key)
{
    char *new_key = NULL;
    int new_type = vxoJson_Invalid;

    gcmHEADER_ARG("object=%p, string=%s, item=%p, hooks=%p, constant_key=0x%x", object, string, item, hooks, constant_key);

    if ((object == NULL) || (string == NULL) || (item == NULL))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    if (constant_key)
    {
        new_key = (char*)cast_away_const(string);
        new_type = item->type | vxoJson_StringIsConst;
    }
    else
    {
        new_key = (char*)vxoJson_strdup((const unsigned char*)string, hooks);
        if (new_key == NULL)
        {
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }

        new_type = item->type & ~vxoJson_StringIsConst;
    }

    if (!(item->type & vxoJson_StringIsConst) && (item->string != NULL))
    {
        hooks->deallocate(item->string);
    }

    item->string = new_key;
    item->type = new_type;

    gcmFOOTER_NO();
    return add_item_to_array(object, item);
}

VX_INTERNAL_API vx_bool vxoJson_AddItemToObject(vxcJSON *object, const char *string, vxcJSON *item)
{
    return add_item_to_object(object, string, item, &global_hooks, vx_false_e);
}

/* Add an item to an object with constant string as key */
VX_INTERNAL_API vx_bool vxoJson_AddItemToObjectCS(vxcJSON *object, const char *string, vxcJSON *item)
{
    return add_item_to_object(object, string, item, &global_hooks, vx_true_e);
}

VX_INTERNAL_API vx_bool vxoJson_AddItemReferenceToArray(vxcJSON *array, vxcJSON *item)
{
    if (array == NULL)
    {
        return vx_false_e;
    }

    return add_item_to_array(array, create_reference(item, &global_hooks));
}

VX_INTERNAL_API vx_bool vxoJson_AddItemReferenceToObject(vxcJSON *object, const char *string, vxcJSON *item)
{
    if ((object == NULL) || (string == NULL))
    {
        return vx_false_e;
    }

    return add_item_to_object(object, string, create_reference(item, &global_hooks), &global_hooks, vx_false_e);
}

VX_INTERNAL_API vxcJSON* vxoJson_AddNullToObject(vxcJSON * const object, const char * const name)
{
    vxcJSON *null = vxoJson_CreateNull();
    if (add_item_to_object(object, name, null, &global_hooks, vx_false_e))
    {
        return null;
    }

    vxoJson_Delete(null);
    return NULL;
}

VX_INTERNAL_API vxcJSON* vxoJson_AddTrueToObject(vxcJSON * const object, const char * const name)
{
    vxcJSON *true_item = vxoJson_CreateTrue();
    if (add_item_to_object(object, name, true_item, &global_hooks, vx_false_e))
    {
        return true_item;
    }

    vxoJson_Delete(true_item);
    return NULL;
}

VX_INTERNAL_API vxcJSON* vxoJson_AddFalseToObject(vxcJSON * const object, const char * const name)
{
    vxcJSON *false_item = vxoJson_CreateFalse();
    if (add_item_to_object(object, name, false_item, &global_hooks, vx_false_e))
    {
        return false_item;
    }

    vxoJson_Delete(false_item);
    return NULL;
}

VX_INTERNAL_API vxcJSON* vxoJson_AddBoolToObject(vxcJSON * const object, const char * const name, const vx_bool boolean)
{
    vxcJSON *bool_item = vxoJson_CreateBool(boolean);
    if (add_item_to_object(object, name, bool_item, &global_hooks, vx_false_e))
    {
        return bool_item;
    }

    vxoJson_Delete(bool_item);
    return NULL;
}

VX_INTERNAL_API vxcJSON* vxoJson_AddNumberToObject(vxcJSON * const object, const char * const name, const double number)
{
    vxcJSON *number_item = vxoJson_CreateNumber(number);
    if (add_item_to_object(object, name, number_item, &global_hooks, vx_false_e))
    {
        return number_item;
    }

    vxoJson_Delete(number_item);
    return NULL;
}

VX_INTERNAL_API vxcJSON* vxoJson_AddStringToObject(vxcJSON * const object, const char * const name, const char * const string)
{
    vxcJSON *string_item = vxoJson_CreateString(string);
    if (add_item_to_object(object, name, string_item, &global_hooks, vx_false_e))
    {
        return string_item;
    }

    vxoJson_Delete(string_item);
    return NULL;
}

VX_INTERNAL_API vxcJSON* vxoJson_AddRawToObject(vxcJSON * const object, const char * const name, const char * const raw)
{
    vxcJSON *raw_item = vxoJson_CreateRaw(raw);
    if (add_item_to_object(object, name, raw_item, &global_hooks, vx_false_e))
    {
        return raw_item;
    }

    vxoJson_Delete(raw_item);
    return NULL;
}

VX_INTERNAL_API vxcJSON* vxoJson_AddObjectToObject(vxcJSON * const object, const char * const name)
{
    vxcJSON *object_item = vxoJson_CreateObject();
    if (add_item_to_object(object, name, object_item, &global_hooks, vx_false_e))
    {
        return object_item;
    }

    vxoJson_Delete(object_item);
    return NULL;
}

VX_INTERNAL_API vxcJSON* vxoJson_AddArrayToObject(vxcJSON * const object, const char * const name)
{
    vxcJSON *array = vxoJson_CreateArray();
    if (add_item_to_object(object, name, array, &global_hooks, vx_false_e))
    {
        return array;
    }

    vxoJson_Delete(array);
    return NULL;
}

VX_INTERNAL_API vxcJSON * vxoJson_DetachItemViaPointer(vxcJSON *parent, vxcJSON * const item)
{
    gcmHEADER_ARG("parent=%p, item=%p", parent, item);
    if ((parent == NULL) || (item == NULL))
    {
        gcmFOOTER_NO();
        return NULL;
    }

    if (item->prev != NULL)
    {
        /* not the first element */
        item->prev->next = item->next;
    }
    if (item->next != NULL)
    {
        /* not the last element */
        item->next->prev = item->prev;
    }

    if (item == parent->child)
    {
        /* first element */
        parent->child = item->next;
    }
    /* make sure the detached item doesn't point anywhere anymore */
    item->prev = NULL;
    item->next = NULL;

    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_DetachItemFromArray(vxcJSON *array, int which)
{
    if (which < 0)
    {
        return NULL;
    }

    return vxoJson_DetachItemViaPointer(array, get_array_item(array, (size_t)which));
}

VX_INTERNAL_API void vxoJson_DeleteItemFromArray(vxcJSON *array, int which)
{
    vxoJson_Delete(vxoJson_DetachItemFromArray(array, which));
}

VX_INTERNAL_API vxcJSON * vxoJson_DetachItemFromObject(vxcJSON *object, const char *string)
{
    vxcJSON *to_detach = vxoJson_GetObjectItem(object, string);

    return vxoJson_DetachItemViaPointer(object, to_detach);
}

VX_INTERNAL_API vxcJSON * vxoJson_DetachItemFromObjectCaseSensitive(vxcJSON *object, const char *string)
{
    vxcJSON *to_detach = vxoJson_GetObjectItemCaseSensitive(object, string);

    return vxoJson_DetachItemViaPointer(object, to_detach);
}

VX_INTERNAL_API void vxoJson_DeleteItemFromObject(vxcJSON *object, const char *string)
{
    vxoJson_Delete(vxoJson_DetachItemFromObject(object, string));
}

VX_INTERNAL_API void vxoJson_DeleteItemFromObjectCaseSensitive(vxcJSON *object, const char *string)
{
    vxoJson_Delete(vxoJson_DetachItemFromObjectCaseSensitive(object, string));
}

/* Replace array/object items with new ones. */
VX_INTERNAL_API void vxoJson_InsertItemInArray(vxcJSON *array, int which, vxcJSON *newitem)
{
    vxcJSON *after_inserted = NULL;

    gcmHEADER_ARG("array=%p, which=0x%x, newitem=%p", array, which, newitem);

    if (which < 0)
    {
        gcmFOOTER_NO();
        return;
    }

    after_inserted = get_array_item(array, (size_t)which);
    if (after_inserted == NULL)
    {
        add_item_to_array(array, newitem);
        gcmFOOTER_NO();
        return;
    }

    newitem->next = after_inserted;
    newitem->prev = after_inserted->prev;
    after_inserted->prev = newitem;
    if (after_inserted == array->child)
    {
        array->child = newitem;
    }
    else
    {
        newitem->prev->next = newitem;
    }
    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_bool vxoJson_ReplaceItemViaPointer(vxcJSON * const parent, vxcJSON * const item, vxcJSON * replacement)
{
    gcmHEADER_ARG("parent=%p, item=%p, replacement=%p", parent, item, replacement);

    if ((parent == NULL) || (replacement == NULL) || (item == NULL))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    if (replacement == item)
    {
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }

    replacement->next = item->next;
    replacement->prev = item->prev;

    if (replacement->next != NULL)
    {
        replacement->next->prev = replacement;
    }
    if (replacement->prev != NULL)
    {
        replacement->prev->next = replacement;
    }
    if (parent->child == item)
    {
        parent->child = replacement;
    }

    item->next = NULL;
    item->prev = NULL;
    vxoJson_Delete(item);

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API void vxoJson_ReplaceItemInArray(vxcJSON *array, int which, vxcJSON *newitem)
{
    if (which < 0)
    {
        return;
    }

    vxoJson_ReplaceItemViaPointer(array, get_array_item(array, (size_t)which), newitem);
}

VX_PRIVATE_API vx_bool replace_item_in_object(vxcJSON *object, const char *string, vxcJSON *replacement, vx_bool case_sensitive)
{
    gcmHEADER_ARG("object=%p, string=%s, replacement=%p, case_sensitive=0x%x", object, string, replacement, case_sensitive);
    if ((replacement == NULL) || (string == NULL))
    {
        gcmFOOTER_ARG("%d", vx_false_e);
        return vx_false_e;
    }

    /* replace the name in the replacement */
    if (!(replacement->type & vxoJson_StringIsConst) && (replacement->string != NULL))
    {
        vxoJson_free(replacement->string);
    }
    replacement->string = (char*)vxoJson_strdup((const unsigned char*)string, &global_hooks);
    replacement->type &= ~vxoJson_StringIsConst;

    vxoJson_ReplaceItemViaPointer(object, get_object_item(object, string, case_sensitive), replacement);

    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;
}

VX_INTERNAL_API void vxoJson_ReplaceItemInObject(vxcJSON *object, const char *string, vxcJSON *newitem)
{
    replace_item_in_object(object, string, newitem, vx_false_e);
}

VX_INTERNAL_API void vxoJson_ReplaceItemInObjectCaseSensitive(vxcJSON *object, const char *string, vxcJSON *newitem)
{
    replace_item_in_object(object, string, newitem, vx_true_e);
}

/* Create basic types: */
VX_INTERNAL_API vxcJSON * vxoJson_CreateNull(void)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);
    gcmHEADER();
    if(item)
    {
        item->type = vxoJson_NULL;
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateTrue(void)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);
    gcmHEADER();
    if(item)
    {
        item->type = vxoJson_True;
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateFalse(void)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);
    gcmHEADER();
    if(item)
    {
        item->type = vxoJson_False;
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateBool(vx_bool b)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);
    gcmHEADER_ARG("b=0x%x", b);
    if(item)
    {
        item->type = b ? vxoJson_True : vxoJson_False;
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateNumber(double num)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);

    gcmHEADER_ARG("num=%f", num);
    if(item)
    {
        item->type = vxoJson_Number;
        item->valuedouble = num;

        /* use saturation in case of overflow */
        if (num >= INT_MAX)
        {
            item->valueint = INT_MAX;
        }
        else if (num <= INT_MIN)
        {
            item->valueint = INT_MIN;
        }
        else
        {
            item->valueint = (int)num;
        }
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateString(const char *string)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);
    gcmHEADER_ARG("string=%s", string);
    if(item)
    {
        item->type = vxoJson_String;
        item->valuestring = (char*)vxoJson_strdup((const unsigned char*)string, &global_hooks);
        if(!item->valuestring)
        {
            vxoJson_Delete(item);
            return NULL;
        }
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateStringReference(const char *string)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);

    gcmHEADER_ARG("string=%s", string);
    if (item != NULL)
    {
        item->type = vxoJson_String | vxoJson_IsReference;
        item->valuestring = (char*)cast_away_const(string);
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateObjectReference(const vxcJSON *child)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);
    gcmHEADER_ARG("child=%p", child);

    if (item != NULL) {
        item->type = vxoJson_Object | vxoJson_IsReference;
        item->child = (vxcJSON*)cast_away_const(child);
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateArrayReference(const vxcJSON *child) {
    vxcJSON *item = vxoJson_New_Item(&global_hooks);

    gcmHEADER_ARG("child=%p", child);

    if (item != NULL) {
        item->type = vxoJson_Array | vxoJson_IsReference;
        item->child = (vxcJSON*)cast_away_const(child);
    }

    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateRaw(const char *raw)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);

    gcmHEADER_ARG("raw=%s", raw);
    if(item)
    {
        item->type = vxoJson_Raw;
        item->valuestring = (char*)vxoJson_strdup((const unsigned char*)raw, &global_hooks);
        if(!item->valuestring)
        {
            vxoJson_Delete(item);
            return NULL;
        }
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateArray(void)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);
    gcmHEADER();
    if(item)
    {
        item->type=vxoJson_Array;
    }
    gcmFOOTER_ARG("item=%p", item);
    return item;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateObject(void)
{
    vxcJSON *item = vxoJson_New_Item(&global_hooks);
    gcmHEADER();
    if (item)
    {
        item->type = vxoJson_Object;
    }

    gcmFOOTER_ARG("item=%p", item);
    return item;
}

/* Create Arrays: */
VX_INTERNAL_API vxcJSON * vxoJson_CreateIntArray(const int *numbers, int count)
{
    size_t i = 0;
    vxcJSON *n = NULL;
    vxcJSON *p = NULL;
    vxcJSON *a = NULL;
    gcmHEADER_ARG("numbers=%p, count=0x%x", numbers, count);

    if ((count < 0) || (numbers == NULL))
    {
        gcmFOOTER_NO();
        return NULL;
    }

    a = vxoJson_CreateArray();
    for(i = 0; a && (i < (size_t)count); i++)
    {
        n = vxoJson_CreateNumber(numbers[i]);
        if (!n)
        {
            vxoJson_Delete(a);
            gcmFOOTER_NO();
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    gcmFOOTER_ARG("a=%p", a);
    return a;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateFloatArray(const float *numbers, int count)
{
    size_t i = 0;
    vxcJSON *n = NULL;
    vxcJSON *p = NULL;
    vxcJSON *a = NULL;

    gcmHEADER_ARG("numbers=%p, count=0x%x", numbers, count);

    if ((count < 0) || (numbers == NULL))
    {
        gcmFOOTER_NO();
        return NULL;
    }

    a = vxoJson_CreateArray();

    for(i = 0; a && (i < (size_t)count); i++)
    {
        n = vxoJson_CreateNumber((double)numbers[i]);
        if(!n)
        {
            vxoJson_Delete(a);
            gcmFOOTER_NO();
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    gcmFOOTER_ARG("a=%p", a);
    return a;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateDoubleArray(const double *numbers, int count)
{
    size_t i = 0;
    vxcJSON *n = NULL;
    vxcJSON *p = NULL;
    vxcJSON *a = NULL;

    gcmHEADER_ARG("numbers=%p, count=0x%x", numbers, count);

    if ((count < 0) || (numbers == NULL))
    {
        gcmFOOTER_NO();
        return NULL;
    }

    a = vxoJson_CreateArray();

    for(i = 0;a && (i < (size_t)count); i++)
    {
        n = vxoJson_CreateNumber(numbers[i]);
        if(!n)
        {
            vxoJson_Delete(a);
            gcmFOOTER_NO();
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    gcmFOOTER_ARG("a=%p", a);
    return a;
}

VX_INTERNAL_API vxcJSON * vxoJson_CreateStringArray(const char **strings, int count)
{
    size_t i = 0;
    vxcJSON *n = NULL;
    vxcJSON *p = NULL;
    vxcJSON *a = NULL;

    gcmHEADER_ARG("strings=%p, count=0x%x", strings, count);

    if ((count < 0) || (strings == NULL))
    {
        gcmFOOTER_NO();
        return NULL;
    }

    a = vxoJson_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = vxoJson_CreateString(strings[i]);
        if(!n)
        {
            vxoJson_Delete(a);
            gcmFOOTER_NO();
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p,n);
        }
        p = n;
    }

    gcmFOOTER_ARG("a=%p", a);
    return a;
}

/* Duplication */
VX_INTERNAL_API vxcJSON * vxoJson_Duplicate(const vxcJSON *item, vx_bool recurse)
{
    vxcJSON *newitem = NULL;
    vxcJSON *child = NULL;
    vxcJSON *next = NULL;
    vxcJSON *newchild = NULL;

    gcmHEADER_ARG("items=%p, recurse=0x%x", item, recurse);

    /* Bail on bad ptr */
    if (!item)
    {
        goto fail;
    }
    /* Create new item */
    newitem = vxoJson_New_Item(&global_hooks);
    if (!newitem)
    {
        goto fail;
    }
    /* Copy over all vars */
    newitem->type = item->type & (~vxoJson_IsReference);
    newitem->valueint = item->valueint;
    newitem->valuedouble = item->valuedouble;
    if (item->valuestring)
    {
        newitem->valuestring = (char*)vxoJson_strdup((unsigned char*)item->valuestring, &global_hooks);
        if (!newitem->valuestring)
        {
            goto fail;
        }
    }
    if (item->string)
    {
        newitem->string = (item->type&vxoJson_StringIsConst) ? item->string : (char*)vxoJson_strdup((unsigned char*)item->string, &global_hooks);
        if (!newitem->string)
        {
            goto fail;
        }
    }
    /* If non-recursive, then we're done! */
    if (!recurse)
    {
        gcmFOOTER_ARG("newitem=%p", newitem);
        return newitem;
    }
    /* Walk the ->next chain for the child. */
    child = item->child;
    while (child != NULL)
    {
        newchild = vxoJson_Duplicate(child, vx_true_e); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild)
        {
            goto fail;
        }
        if (next != NULL)
        {
            /* If newitem->child already set, then crosswire ->prev and ->next and move on */
            next->next = newchild;
            newchild->prev = next;
            next = newchild;
        }
        else
        {
            /* Set newitem->child and move to it */
            newitem->child = newchild;
            next = newchild;
        }
        child = child->next;
    }

    gcmFOOTER_ARG("newitem=%p", newitem);
    return newitem;

fail:
    if (newitem != NULL)
    {
        vxoJson_Delete(newitem);
    }
    gcmFOOTER_NO();
    return NULL;
}

VX_INTERNAL_API void vxoJson_Minify(char *json)
{
    unsigned char *into = (unsigned char*)json;

    gcmHEADER_ARG("json=%p", json);

    if (json == NULL)
    {
        gcmFOOTER_NO();
        return;
    }

    while (*json)
    {
        if (*json == ' ')
        {
            json++;
        }
        else if (*json == '\t')
        {
            /* Whitespace characters. */
            json++;
        }
        else if (*json == '\r')
        {
            json++;
        }
        else if (*json=='\n')
        {
            json++;
        }
        else if ((*json == '/') && (json[1] == '/'))
        {
            /* double-slash comments, to end of line. */
            while (*json && (*json != '\n'))
            {
                json++;
            }
        }
        else if ((*json == '/') && (json[1] == '*'))
        {
            /* multiline comments. */
            while (*json && !((*json == '*') && (json[1] == '/')))
            {
                json++;
            }
            json += 2;
        }
        else if (*json == '\"')
        {
            /* string literals, which are \" sensitive. */
            *into++ = (unsigned char)*json++;
            while (*json && (*json != '\"'))
            {
                if (*json == '\\')
                {
                    *into++ = (unsigned char)*json++;
                }
                *into++ = (unsigned char)*json++;
            }
            *into++ = (unsigned char)*json++;
        }
        else
        {
            /* All other characters. */
            *into++ = (unsigned char)*json++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_bool vxoJson_IsInvalid(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & 0xFF) == vxoJson_Invalid;
}

VX_INTERNAL_API vx_bool vxoJson_IsFalse(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & 0xFF) == vxoJson_False;
}

VX_INTERNAL_API vx_bool vxoJson_IsTrue(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & 0xff) == vxoJson_True;
}


VX_INTERNAL_API vx_bool vxoJson_IsBool(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & (vxoJson_True | vxoJson_False)) != 0;
}
VX_INTERNAL_API vx_bool vxoJson_IsNull(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & 0xFF) == vxoJson_NULL;
}

VX_INTERNAL_API vx_bool vxoJson_IsNumber(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & 0xFF) == vxoJson_Number;
}

VX_INTERNAL_API vx_bool vxoJson_IsString(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & 0xFF) == vxoJson_String;
}

VX_INTERNAL_API vx_bool vxoJson_IsArray(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & 0xFF) == vxoJson_Array;
}

VX_INTERNAL_API vx_bool vxoJson_IsObject(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & 0xFF) == vxoJson_Object;
}

VX_INTERNAL_API vx_bool vxoJson_IsRaw(const vxcJSON * const item)
{
    if (item == NULL)
    {
        return vx_false_e;
    }

    return (item->type & 0xFF) == vxoJson_Raw;
}

VX_INTERNAL_API vx_bool vxoJson_Compare(const vxcJSON * const a, const vxcJSON * const b, const vx_bool case_sensitive)
{
    if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF)) || vxoJson_IsInvalid(a))
    {
        return vx_false_e;
    }

    /* check if type is valid */
    switch (a->type & 0xFF)
    {
        case vxoJson_False:
        case vxoJson_True:
        case vxoJson_NULL:
        case vxoJson_Number:
        case vxoJson_String:
        case vxoJson_Raw:
        case vxoJson_Array:
        case vxoJson_Object:
            break;

        default:
            return vx_false_e;
    }

    /* identical objects are equal */
    if (a == b)
    {
        return vx_true_e;
    }

    switch (a->type & 0xFF)
    {
        /* in these cases and equal type is enough */
        case vxoJson_False:
        case vxoJson_True:
        case vxoJson_NULL:
            return vx_true_e;

        case vxoJson_Number:
            if (a->valuedouble == b->valuedouble)
            {
                return vx_true_e;
            }
            return vx_false_e;

        case vxoJson_String:
        case vxoJson_Raw:
            if ((a->valuestring == NULL) || (b->valuestring == NULL))
            {
                return vx_false_e;
            }
            if (strcmp(a->valuestring, b->valuestring) == 0)
            {
                return vx_true_e;
            }

            return vx_false_e;

        case vxoJson_Array:
        {
            vxcJSON *a_element = a->child;
            vxcJSON *b_element = b->child;

            for (; (a_element != NULL) && (b_element != NULL);)
            {
                if (!vxoJson_Compare(a_element, b_element, case_sensitive))
                {
                    return vx_false_e;
                }

                a_element = a_element->next;
                b_element = b_element->next;
            }

            /* one of the arrays is longer than the other */
            if (a_element != b_element) {
                return vx_false_e;
            }

            return vx_true_e;
        }

        case vxoJson_Object:
        {
            vxcJSON *a_element = NULL;
            vxcJSON *b_element = NULL;
            vxoJson_ArrayForEach(a_element, a)
            {
                /* TODO This has O(n^2) runtime, which is horrible! */
                b_element = get_object_item(b, a_element->string, case_sensitive);
                if (b_element == NULL)
                {
                    return vx_false_e;
                }

                if (!vxoJson_Compare(a_element, b_element, case_sensitive))
                {
                    return vx_false_e;
                }
            }

            /* doing this twice, once on a and b to prevent vx_true_e comparison if a subset of b
             * TODO: Do this the proper way, this is just a fix for now */
            vxoJson_ArrayForEach(b_element, b)
            {
                a_element = get_object_item(a, b_element->string, case_sensitive);
                if (a_element == NULL)
                {
                    return vx_false_e;
                }

                if (!vxoJson_Compare(b_element, a_element, case_sensitive))
                {
                    return vx_false_e;
                }
            }

            return vx_true_e;
        }

        default:
            return vx_false_e;
    }
}

VX_INTERNAL_API void * vxoJson_malloc(size_t size)
{
    return global_hooks.allocate(size);
}

VX_INTERNAL_API void vxoJson_free(void *object)
{
    global_hooks.deallocate(object);
}


