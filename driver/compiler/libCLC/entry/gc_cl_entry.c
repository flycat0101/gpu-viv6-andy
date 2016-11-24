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


#include "gc_cl_compiler.h"

static gctINT _clDebugCompiler = 0;  /* debug !!! */
#define cldDumpOptions (_clDebugCompiler == 1 ? clvDUMP_IR :\
                        _clDebugCompiler == 2 ? clvDUMP_COMPILER : \
                        _clDebugCompiler == 3 ? clvDUMP_SCANNER : clvDUMP_NONE)

#define cldVivanteBuiltinLibraryInclude "#include \"VIV_BUILTIN_LIB.cl\"\n"
#define cldVivanteBuiltinLibraryIncludeLength 36
#define cldLinkVivanteBuiltinLibrary 0

#define cldPatchKernel  1

#if cldPatchKernel
#include "gc_cl_patch.h"

static void __clChipUtilsDecrypt( IN OUT gctSTRING Source)
{
    gctUINT8        key = 0xFF;
    gctSTRING       source = (gctSTRING)Source;

    do
    {
        /* Check if it's encrypt. */
        if ((strstr(source, ";") != gcvNULL)
            || (strstr(source, "\n") != gcvNULL)
            || (strstr(source, "f") != gcvNULL)
            || (strstr(source, "/") != gcvNULL)
            || (strstr(source, "#") != gcvNULL)
            || (strstr(source, " ") != gcvNULL)
        )
            break;

        while(*source != '\0')
        {
            gctCHAR ch = *source ^ key;
            *source++ = ch;

            if (ch == 0)key ^= 0xFF;
            key ^= ch;
        }

    }while(gcvFALSE);

}
#endif

/*******************************************************************************
**                              gcLoadKernelCompiler
********************************************************************************
**
**    OpenCL kernel shader compiler load.
**
*/
gceSTATUS
gcLoadKernelCompiler(
    IN gcsHWCaps *HWCaps
    )
{
    gceSTATUS status;

#if cldPatchKernel
    lookup * lookup;

    for (lookup = compiledShaders; lookup->sourceSize != 0; ++lookup)
    {
        __clChipUtilsDecrypt(lookup->source1);
        __clChipUtilsDecrypt(lookup->source2);
    }
#endif

    if (HWCaps)
    {
        *gcGetHWCaps() = *HWCaps;
    }

    gcmONERROR(cloCOMPILER_Load());

    gcmONERROR(gcInitializeRecompilation());

OnError:
    return status;
}

/*******************************************************************************
**                              gcUnloadKernelCompiler
********************************************************************************
**
**    OpenCL kernel shader compiler unload.
**
*/
gceSTATUS
gcUnloadKernelCompiler(
void
)
{
    gceSTATUS status;
    status = cloCOMPILER_Unload();
    if (gcmIS_ERROR(status)) return status;
    return gcFinalizeRecompilation();
}

static gceSTATUS
_CompileKernel(
    IN cloCOMPILER Compiler,
    IN gctUINT SourceSize,
    IN gctCONST_STRING Source,
    IN gctCONST_STRING Options,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT sourceStringCount = 1;
    gctCONST_STRING sourceStrings[] = {Source};

#if cldLinkVivanteBuiltinLibrary
    gctSIZE_T sourceLength;
    gctPOINTER pointer;
#endif

#if cldPatchKernel
    {
        const char *p, *q;
        lookup * lookup;

        if (SourceSize == 0)
        {
            SourceSize = (gctUINT32)gcoOS_StrLen(Source, gcvNULL);
        }

        /* Look up any hand compiled shader. */
        for (lookup = compiledShaders; lookup->sourceSize != 0; ++lookup)
        {
            if (SourceSize < lookup->sourceSize)
            {
                /*continue;*/
            }

            p = Source;
            q = lookup->source1;

            while (*p)
            {
                gctUINT i;

                /* ignore key words first. */
                for (i = 0; i < gcmCOUNTOF(SkippedWords); ++i)
                {
                    if (gcoOS_StrNCmp(p, SkippedWords[i].keyword, SkippedWords[i].length) == gcvSTATUS_OK)
                    {
                        if (IS_BLANK(*(p + SkippedWords[i].length)))
                        {
                            p += SkippedWords[i].length;
                            break;
                        }
                    }
                }

                for (i = 0; i < gcmCOUNTOF(SkippedWords); ++i)
                {
                    if (gcoOS_StrNCmp(q, SkippedWords[i].keyword, SkippedWords[i].length) == gcvSTATUS_OK)
                    {
                        if (IS_BLANK(*(q + SkippedWords[i].length)))
                        {
                            q += SkippedWords[i].length;
                            break;
                        }
                    }
                }

                if (*p == *q)
                {
                    p++;
                    q++;
                }
                else if (IS_BLANK(*p))
                {
                    p++;
                }
                else if (IS_BLANK(*q))
                {
                    q++;
                }
                else
                {
                    break;
                }
            }

            if (*p == '\0' && *q == '\0')
            {
                sourceStrings[0] = lookup->source2;
            }
        }
    }
#endif

#if cldLinkVivanteBuiltinLibrary
    sourceLength = gcoOS_StrLen(Source, gcvNULL) +
                   cldVivanteBuiltinLibraryIncludeLength + 1;
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              sourceLength,
                              &pointer));
    sourceStrings[0] = pointer;

    gcmVERIFY_OK(gcoOS_StrCopySafe((gctSTRING) sourceStrings[0], sourceLength, cldVivanteBuiltinLibraryInclude));
    gcmVERIFY_OK(gcoOS_StrCatSafe((gctSTRING) sourceStrings[0], sourceLength, Source));
#endif

    gcmONERROR(cloCOMPILER_Compile(Compiler,
                                   cldOPTIMIZATION_OPTIONS_DEFAULT,
                                   cldDumpOptions,
                                   sourceStringCount,
                                   sourceStrings,
                                   Options,
                                   Binary,
                                   Log));

OnError:
    return status;
}

/*******************************************************************************
**                              gcCompileKernel
********************************************************************************
**
**    Compile a OpenCL kernel shader.
**
**    INPUT:
**
**        gcoOS Hal
**            Pointer to an gcoHAL object.
**
**        gctUINT SourceSize
**            Size of the source buffer in bytes.
**
**        gctCONST_STRING Source
**            Pointer to the buffer containing the shader source code.
**
**        gctCONST_STRING Options
**            Pointer to the buffer containing the compiler options.
**
**    OUTPUT:
**
**        gcSHADER * Binary
**            Pointer to a variable receiving the pointer to a gcSHADER object
**            containg the compiled shader code.
**
**        gctSTRING * Log
**            Pointer to a variable receiving a string pointer containging the
**            compile log.
*/
gceSTATUS
gcCompileKernel(
    IN gcoHAL Hal,
    IN gctUINT SourceSize,
    IN gctCONST_STRING Source,
    IN gctCONST_STRING Options,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    )
{
    gceSTATUS status;
    cloCOMPILER compiler = gcvNULL;

    gcmONERROR(cloCOMPILER_Construct(&compiler));

    gcmONERROR(_CompileKernel(compiler,
                              SourceSize,
                              Source,
                              Options,
                              Binary,
                              Log));

OnError:
    if (compiler != gcvNULL) gcmVERIFY_OK(cloCOMPILER_Destroy(compiler));

    return status;
}

/*******************************************************************************
**                              gcCLCompileProgram
********************************************************************************
**
**    Compile a OpenCL kernel program as in clCompileProgram().
**
**    INPUT:
**
**        gcoOS Hal
**            Pointer to an gcoHAL object.
**
**        gctUINT SourceSize
**            Size of the source buffer in bytes.
**
**        gctCONST_STRING Source
**            Pointer to the buffer containing the shader source code.
**
**        gctCONST_STRING Options
**            Pointer to the buffer containing the compiler options.
**
**        gctUINT NumInputHeaders
**            Number of embedded input headers.
**
**        gctCONST_STRING *InputHeaders
**            Array of pointers to the embedded header sources.
**
**        gctCONST_STRING *HeaderIncludeNames
**            Array of pointers to the header include names.
**
**    OUTPUT:
**
**        gcSHADER * Binary
**            Pointer to a variable receiving the pointer to a gcSHADER object
**            containg the compiled shader code.
**
**        gctSTRING * Log
**            Pointer to a variable receiving a string pointer containging the
**            compile log.
*/
gceSTATUS
gcCLCompileProgram(
    IN gcoHAL Hal,
    IN gctUINT SourceSize,
    IN gctCONST_STRING Source,
    IN gctCONST_STRING Options,
    IN gctUINT NumInputHeaders,
    IN gctCONST_STRING *InputHeaders,
    IN gctCONST_STRING *HeaderIncludeNames,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    )
{
    gceSTATUS status;
    cloCOMPILER compiler = gcvNULL;
    gctCONST_STRING newSourceString = gcvNULL;
    gctUINT sourceSize;
    gctCONST_STRING sourceString;

    gcmONERROR(cloCOMPILER_ConstructByLangVersion(_cldCL1Dot2, &compiler));

    sourceString = Source;
    sourceSize = SourceSize;
    if(NumInputHeaders && SourceSize) {
        gctUINT i, j, k;
        gctUINT length;
        gctUINT *headerSizes = gcvNULL;
        gctUINT *headerNameLengths = gcvNULL;
        gctUINT newSourceLength = 0;
        gctBOOL lineBegin, directiveBegin;
        gctUINT insertLocation;
        gctUINT lineNo;
        gctUINT copyStart;
        gctUINT maxLineDirectiveLength;

        struct _clsEMBEDDED_HEADER
        {
            slsSLINK_NODE node;
            gctUINT insertAt;    /* insert header at character number */
            gctUINT resumeFrom;  /* resume copying of orginal source from character number */
            gctUINT index;       /* index to header source */
            gctUINT lineNo;      /* line number of original source after header substitution */
        };

        slsSLINK_LIST *embeddedHeaderList;
        struct _clsEMBEDDED_HEADER *embeddedHeader;

        #define _cldWhiteSpaces " \t"
        #define _cldSourceStringLabel "source_string_"

        status = cloCOMPILER_Allocate(compiler,
                                      (gctSIZE_T)sizeof(gctUINT) * NumInputHeaders,
                                      (gctPOINTER *) &headerSizes);
        if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;

        status = cloCOMPILER_Allocate(compiler,
                                      (gctSIZE_T)sizeof(gctUINT) * NumInputHeaders,
                                      (gctPOINTER *) &headerNameLengths);
        if (gcmIS_ERROR(status)) return gcvSTATUS_OUT_OF_MEMORY;

        maxLineDirectiveLength = gcoOS_StrLen(_cldSourceStringLabel, gcvNULL) + 2;
        for(i = 0; i < NumInputHeaders; i++) {
            headerSizes[i] =  gcoOS_StrLen(InputHeaders[i], gcvNULL);
            length = gcoOS_StrLen(HeaderIncludeNames[i], gcvNULL);
            if(maxLineDirectiveLength < length) {
                maxLineDirectiveLength = length;
            }
            headerNameLengths[i] = length;
        }
        /* two #line directives for each header include replacement.
           Example:
           #line 150 "<source_string>"
           10 decimal characters will suffice for the line #
        */
        maxLineDirectiveLength = (maxLineDirectiveLength + 18) << 1;

        {
            sourceString = Source;
            length = SourceSize + 1;
            lineBegin = gcvTRUE;
            insertLocation = 0;
            lineNo = 1;
            copyStart = 0;
            directiveBegin = gcvFALSE;
            slmSLINK_LIST_Initialize(embeddedHeaderList);

            j = 0;
            while(j < length) {
                switch(sourceString[j]) {
                case '\n':
                    lineBegin = gcvTRUE;
                    directiveBegin = gcvFALSE;
                    insertLocation = j;
                    lineNo++;
                    break;

                case ' ':
                case '\t':
                    break;

                case '#':
                    if(lineBegin) {
                        directiveBegin = gcvTRUE;
                    }
                    else directiveBegin = gcvFALSE;
                    lineBegin = gcvFALSE;
                    break;

                case 'i':
                    lineBegin = gcvFALSE;
                    if(directiveBegin &&
                       (gcmIS_SUCCESS(gcoOS_StrNCmp(sourceString + j, "include ", 8)) ||
                        gcmIS_SUCCESS(gcoOS_StrNCmp(sourceString + j, "include\t", 8)))) {
                        gctUINT includeNameStart;
                        gctUINT insertAt = insertLocation;

                        j += 8;
                        while (j < length)  {
                            switch(sourceString[j]) {
                            case ' ':
                            case '\t':
                                break;

                            case '"':
                                includeNameStart = j + 1;
                                while (++j < length)  {
                                    if(sourceString[j] == '"') {
                                        gctBOOL valid;
                                        gctUINT includeNameLength = j - includeNameStart;

                                        if(includeNameLength == 0) break;
                                        valid = gcvTRUE;
                                        while(++j < length) {
                                            char c;

                                            c = sourceString[j];
                                            if(c == ' ' || c == '\t') continue;
                                            else if(c == '\n') {
                                               lineBegin = gcvTRUE;
                                               insertLocation = j + 1;
                                               lineNo++;
                                               break;
                                            }
                                            else {
                                               valid = gcvFALSE;
                                               break;
                                            }
                                        }
                                        if(valid) {
                                            for(k = 0; k < NumInputHeaders; k++) {
                                                if(includeNameLength != headerNameLengths[k]) continue;
                                                if(gcmIS_SUCCESS(gcoOS_StrNCmp(HeaderIncludeNames[k],
                                                                               sourceString + includeNameStart,
                                                                               includeNameLength))) {
                                                    status = cloCOMPILER_Allocate(compiler,
                                                                                  (gctSIZE_T)sizeof(struct _clsEMBEDDED_HEADER),
                                                                                  (gctPOINTER *) &embeddedHeader);
                                                    if (gcmIS_ERROR(status)) return status;
                                                    embeddedHeader->insertAt = insertAt;
                                                    embeddedHeader->resumeFrom = j;
                                                    embeddedHeader->index = k;
                                                    embeddedHeader->lineNo = lineNo;
                                                    slmSLINK_LIST_InsertLast(embeddedHeaderList, &embeddedHeader->node);
                                                    newSourceLength += maxLineDirectiveLength +
                                                                       headerSizes[k] +
                                                                       (insertAt - copyStart);
                                                    copyStart = j;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                    else if(sourceString[j] == '\n') {
                                        lineBegin = gcvTRUE;
                                        insertLocation = j + 1;
                                        lineNo++;
                                        break;
                                    }
                                }
                                directiveBegin = gcvFALSE;
                                break;

                            default:
                                directiveBegin = gcvFALSE;
                                break;
                            }
                            if(directiveBegin == gcvFALSE) break;
                            j++;
                        }
                    }
                    directiveBegin = gcvFALSE;
                    break;

                default:
                    lineBegin = directiveBegin = gcvFALSE;
                    break;
                }

                j++;
            }

            if(!slmSLINK_LIST_IsEmpty(embeddedHeaderList)) {
                gctSTRING newSource;
                gctUINT start = 0;
                gctUINT copyLength = 0;
                gctUINT offset;

                newSourceLength += j - copyStart + 1;

                gcmONERROR(cloCOMPILER_Allocate(compiler,
                                                sizeof(gctCHAR) * newSourceLength,
                                                (gctPOINTER *) &newSource));
                newSourceString = (gctCONST_STRING)newSource;

                while (!slmSLINK_LIST_IsEmpty(embeddedHeaderList)) {
                    slmSLINK_LIST_DetachFirst(embeddedHeaderList, struct _clsEMBEDDED_HEADER, &embeddedHeader);
                    copyLength = embeddedHeader->insertAt - start;
                    if(copyLength) {
                        gcoOS_MemCopy(newSource,
                                      sourceString + start,
                                      copyLength);
                        newSource += copyLength;
                        newSourceLength -= copyLength;
                        start = embeddedHeader->resumeFrom;
                    }
                    if(headerSizes[embeddedHeader->index]) {
                        offset = 0;
                        gcmVERIFY_OK(gcoOS_PrintStrSafe(newSource,
                                                        newSourceLength,
                                                        &offset,
                                                        "#line 1 \"%s\"\n",
                                                        HeaderIncludeNames[embeddedHeader->index]));
                        newSource += offset;
                        newSourceLength -= offset;

                        copyLength = headerSizes[embeddedHeader->index];
                        gcoOS_MemCopy(newSource,
                                      InputHeaders[embeddedHeader->index],
                                      copyLength);
                        newSource += copyLength;
                        newSourceLength -= copyLength;

                        offset = 0;
                        gcmVERIFY_OK(gcoOS_PrintStrSafe(newSource,
                                                        newSourceLength,
                                                        &offset,
                                                        "#line %d \"<%s%d>\"",
                                                        embeddedHeader->lineNo,
                                                        _cldSourceStringLabel,
                                                        i));
                        newSource += offset;
                        gcmASSERT(newSourceLength > offset);
                        newSourceLength -= offset;
                    }
                    start = embeddedHeader->resumeFrom;

                    cloCOMPILER_Free(compiler, embeddedHeader);
                }

                gcmASSERT(newSourceLength > gcoOS_StrLen(sourceString + start, gcvNULL));
                gcmVERIFY_OK(gcoOS_StrCopySafe(newSource, newSourceLength, sourceString + start));
                sourceString = newSourceString;
                sourceSize = (gctUINT32)gcoOS_StrLen(sourceString, gcvNULL);
            }
            cloCOMPILER_Free(compiler, headerSizes);
            cloCOMPILER_Free(compiler, headerNameLengths);
        }
    }

    gcmONERROR(_CompileKernel(compiler,
                              sourceSize,
                              sourceString,
                              Options,
                              Binary,
                              Log));

OnError:

    if(newSourceString) {
        cloCOMPILER_Free(compiler, (gctSTRING)newSourceString);
    }

    if (compiler != gcvNULL) gcmVERIFY_OK(cloCOMPILER_Destroy(compiler));

    return status;
}
