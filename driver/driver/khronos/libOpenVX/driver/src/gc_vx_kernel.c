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


#include <gc_vx_common.h>


#define _cldPrintfConvSpecifiers "diouxXfFeEgGaAcsp"
#define _cldDigits               "0123456789"
#define _cldFlags                "-+ #0"
#define _cldLowercase "0123456789abcdefghijklmnopqrstuvwxyz"
#define _cldUppercase "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define LEFT 0x01
#define PLUS 0x02
#define SPACE 0x04
#define SPECIAL 0x08
#define ZEROPAD 0x10
#define SMALL 0x20
#define LARGE 0x40
#define SIGN 0x80

typedef enum _cleARGTYPE
{
    cleARGTYPE_CHAR = 1,
    cleARGTYPE_UCHAR,
    cleARGTYPE_SHORT,
    cleARGTYPE_USHORT,
    cleARGTYPE_LONG,
    cleARGTYPE_ULONG,
    cleARGTYPE_HALF,
    cleARGTYPE_INT,
    cleARGTYPE_UINT,
    cleARGTYPE_FLOAT,
    cleARGTYPE_DOUBLE

}cleARGTYPE;

gctBOOL gcfVX_IsNan(gctFLOAT val)
{
    gcuFLOAT_UINT32 uValue;
    uValue.f = val;
    if ((uValue.u & (gctUINT)0x7FFFFFFF) > (gctUINT)0x7F800000)
    {
        return gcvTRUE;
    }else{
        return gcvFALSE;
    }
}

gctBOOL gcfVX_IsInf(gctFLOAT val)
{
    gcuFLOAT_UINT32 uValue;
    uValue.f = val;
    if ((uValue.u & (gctUINT)0x7FFFFFFF) == (gctUINT)0x7F800000)
    {
        return gcvTRUE;
    }else{
        return gcvFALSE;
    }
}

gctBOOL gcfVX_IsInString(gctCHAR s, gctCHAR* string)
{
    while (*string)
    {
        if (s != *string++)
            continue;
        return gcvTRUE;
    }
    return gcvFALSE;
}

void
gcfVX_GetSingleFormat(
    gctCHAR* StartPtr,
    gctCHAR* EndPtr,
    gctCHAR* Format,
    gctINT*  VectorSize,
    gctINT*  ArgType,
    gctINT*  Flags,
    gctINT*  FieldWidth,
    gctINT*  Precision
    )
{
    gctCHAR chr;
    gctCHAR followChr;
    gctBOOL isVectorSpecifier = gcvFALSE;

    gctINT vectorSize = 0;
    *ArgType = 0;
    *VectorSize = 0;
    *Flags = 0;
    *FieldWidth = 0;
    *Precision = 0;

    while (StartPtr <= EndPtr) {
         chr = *StartPtr++;

         switch(chr) {
         case 'v':
             if (StartPtr < EndPtr)
             {
                 chr = *StartPtr++;
                 switch(chr) {
                 case '1':
                     chr = *StartPtr++;
                     if(StartPtr < EndPtr) {
                         if(chr != '6') return;
                     }
                     else return;
                     vectorSize = 16;
                     break;
                 case '2':
                     vectorSize = 2;
                     break;

                 case '4':
                     vectorSize = 4;
                     break;

                 case '8':
                     vectorSize = 8;
                     break;

                 default:
                     return;
                 }
             }
             else return;
             isVectorSpecifier = gcvTRUE;
             break;

         case 'h':
             *Format++ = chr;
             followChr = *StartPtr;
             if ((followChr == 'd') || (followChr == 'i'))
             {
                 *ArgType = cleARGTYPE_SHORT;
             }else if ((followChr == 'o') || (followChr == 'u') || (followChr == 'x') || (followChr == 'X'))
             {
                 *ArgType = cleARGTYPE_USHORT;
             }else if ((followChr == 'a') || (followChr == 'A') || (followChr == 'e') || (followChr == 'E') ||
                       (followChr == 'f') || (followChr == 'F') || (followChr == 'g') || (followChr == 'G'))
             {
                 *ArgType = cleARGTYPE_HALF;
             }
             switch (*StartPtr) {
             case 'h':
                 *Format++ = 'h';
                 StartPtr++;
                 followChr = *StartPtr;
                 if ((followChr == 'd') || (followChr == 'o'))
                 {
                     *ArgType = cleARGTYPE_CHAR;
                 }else if ((followChr == 'o') || (followChr == 'u') || (followChr == 'x') || (followChr == 'X'))
                 {
                     *ArgType = cleARGTYPE_UCHAR;
                 }
                 break;

             case 'l':
                 if(isVectorSpecifier) {  /* hl applies to vector specifier appeared only */
                     Format--;
                     StartPtr++;
                     followChr = *StartPtr;
                     if ((followChr == 'd') || (followChr == 'o'))
                     {
                         *ArgType = cleARGTYPE_INT;
                     }else if ((followChr == 'o') || (followChr == 'u') || (followChr == 'x') || (followChr == 'X'))
                     {
                         *ArgType = cleARGTYPE_UINT;
                     }else if ((followChr == 'a') || (followChr == 'A') || (followChr == 'e') || (followChr == 'E') ||
                               (followChr == 'f') || (followChr == 'F') || (followChr == 'g') || (followChr == 'G'))
                     {
                         *ArgType = cleARGTYPE_FLOAT;
                     }
                 }
                 else return;
                 break;

             default:
                 break;
             }

             break;

         case 'l':
             *Format++ = chr;
             followChr = *StartPtr;
              if (followChr == 'd' || followChr == 'o')
              {
                  *ArgType = cleARGTYPE_LONG;
              }else if (followChr == 'o' || followChr == 'u' || followChr == 'x' || followChr == 'X')
              {
                  *ArgType = cleARGTYPE_ULONG;
              }else if (followChr == 'a' || followChr == 'A' || followChr == 'e' || followChr == 'E' ||
                        followChr == 'f' || followChr == 'F' || followChr == 'g' || followChr == 'G')
              {
                  *ArgType = cleARGTYPE_DOUBLE;
              }
             break;

         default:
             {
                 /* process flags */
                 if(gcfVX_IsInString(chr, (gctCHAR*)_cldFlags))
                 {
                     *Format++ = chr;
                     switch(chr)
                     {
                     case '-':
                         *Flags |= LEFT;
                         break;
                     case '+':
                         *Flags |= PLUS;
                         break;
                     case ' ':
                         *Flags |= SPACE;
                         break;
                     case '#':
                         *Flags |= SPECIAL;
                         break;
                     case '0':
                         *Flags |= ZEROPAD;
                         break;
                     }
                 }else if (gcfVX_IsInString(chr, (gctCHAR*)_cldDigits)) /* get field width */
                 {
                     gctINT i = 0;
                     do
                     {
                         *Format++ = chr;
                         i = i*10 + chr - '0';
                         chr = *StartPtr++;
                     }while (gcfVX_IsInString(chr, (gctCHAR*)_cldDigits));
                     *FieldWidth = i;
                     StartPtr--;
                 }else if (chr == '.') /* get the precision */
                 {
                     *Format++ = chr;
                     chr = *StartPtr++;
                     if (gcfVX_IsInString(chr, (gctCHAR*)_cldDigits))
                     {
                         gctINT i = 0;
                         do
                         {
                             *Format++ = chr;
                             i = i*10 + chr - '0';
                             chr = *StartPtr++;
                         }while (gcfVX_IsInString(chr, (gctCHAR*)_cldDigits));
                         *Precision = i;
                         StartPtr--;
                     }
                     if (*Precision < 0)
                     {
                         *Precision = 0;
                     }
                 }else{
                     *Format++ = chr;
                 }
                 break;
             }
        }
    }

    *VectorSize = vectorSize;
    return;
}

void floatToaHex(
    double Num,
    gctINT Precision,
    gctBOOL IsUpperCase,
    gctCHAR* Buff,
    gctINT* DecimalPos,
    gctINT* Exp)
{
    gctINT intData;
    gctINT i = 0;
    gctINT j = 0;
    double decimalData;
    gctBOOL binaryData[255] = {0};
    gctBOOL binaryInt[255] = {0};
    gctCHAR* dig = (char*)_cldLowercase;
    gctINT len;

    if (IsUpperCase)
    {
        dig = (char*)_cldUppercase;
    }
    /* Process value sign */
    if (Num < 0.0)
    {
        Num = -Num;
        *Buff++ = '-';
    }
    else
    {
        *Buff++ = '+';
    }

    if (Num > -0.000005 && Num < 0.000005)
    {
        for (i = 0; i < Precision + 1; i++)
        {
            *Buff++ = '0';
        }
        *Buff++ = '\0';
        *DecimalPos = 1;
        *Exp = 0;
        return;
    }

    intData = (gctINT)Num;
    decimalData = Num - intData;
    *Exp = 0;

    if (intData == 0)
    {
        j = 0;
        while(decimalData < 1.0)
        {
            decimalData *= 2;
            j++;
        }
        *Exp = -j;
        intData = (int)decimalData;
        decimalData -= intData;
    }
    /* handle integer part */
    while(intData)
    {
        binaryInt[i++] = (intData % 2 == 1) ? gcvTRUE : gcvFALSE;
        intData /= 2;
    }
    *Exp = ((*Exp) == 0) ? i - 1 : *Exp;
    j = 0;
    while(i)
    {
        binaryData[j++] = binaryInt[--i];
    }

    if (decimalData > -0.000005 && decimalData < 0.000005)
    {
        for(i = 0; i < 4 * Precision; i++)
        {
            binaryData[j++] = gcvFALSE;
        }
    }else
    {
        /* handle decimal part */
        while(decimalData != 0.0)
        {
            gctINT t = (gctINT)(decimalData * 2);
            binaryData[j++] = (t > 0) ? gcvTRUE : gcvFALSE;
            decimalData = decimalData * 2 - t;
        }
    }

    len = j - 1;

    if (len % 4)
    {
        memset(binaryData + len + 1, 0, 4 - len % 4);
        len += 4 - len % 4;
    }

    *Buff++ = binaryData[0] ? '1':'0';
    *DecimalPos = 1;

    i = 0;
    j = 0;
    while((i < len))
    {
        *Buff++ = dig[((binaryData[i+1] ? 8 : 0) + (binaryData[i+2] ? 4 : 0) + (binaryData[i+3] ? 2 : 0) + (binaryData[i+4] ? 1 : 0)) % 16];
        i += 4;
        j++;
    }
    *Buff++ = '\0';
}

void fltRound(
    gctCHAR* NumBuf,
    gctINT* DecimalPos,
    gctINT Precision,
    gctBOOL IsUppderCase)
{
    gctCHAR* last_digit = NumBuf + 1 +Precision;
    gctCHAR* after_last = last_digit + 1;

    if (*after_last > '4')
    {
        gctCHAR *p = last_digit;
        gctINT carry = 1;

        do
        {
            gctINT sum;
            if (*p == '.')
            {
                p--;
            }
            if (IsUppderCase)
            {
                if (*p == '9')
                {
                    *p-- = 'A';
                    carry = 0;
                }
                else
                {
                    sum = *p + carry;
                    carry = sum > 'F';
                    *p-- = (gctCHAR)(sum - carry * 23);
                }
            }else
            {
                if (*p == '9')
                {
                    *p-- = 'a';
                    carry = 0;
                }
                else
                {
                    sum = *p + carry;
                    carry = sum > 'f';
                    *p-- = (gctCHAR)(sum - carry * 55);
                }
            }
        } while (carry && p >= NumBuf);

        /* We have fffff... which needs to be rounded to 100000.. */
        if (carry && p == NumBuf)
        {
            *p = '1';
            *DecimalPos += 1;
        }
    }
}

void floatToText(
    double Value,
    gctINT Precision,
    gctCHAR *Buf,
    gctBOOL IsUpperCase)
{
    char chBuffer[255] = {'\0'};

    char *cvtBuf = chBuffer;
    int decimalPos;
    int pos;
    int exp;

    floatToaHex(Value, Precision, IsUpperCase, cvtBuf, &decimalPos, &exp);
    fltRound(cvtBuf, &decimalPos, Precision, IsUpperCase);

    if ('-' == *cvtBuf++)
    {
        *Buf++ = '-';
    }

    if (*cvtBuf)
    {
        *Buf++ = '0';
        *Buf++ = IsUpperCase ? 'X' : 'x';
        *Buf++ = *cvtBuf;
        if (Precision > 0)
        {
            *Buf++ = '.';
        }
        gcoOS_MemCopy(Buf, cvtBuf + 1, Precision);
        Buf += Precision;
        {
            *Buf++ = IsUpperCase ? 'P' : 'p';
        }

        if (exp < 0)
        {
            *Buf++ = '-';
            exp = -exp;
        }else{
            *Buf++ = '+';
        }

        do
        {
            if ((exp / 10) == 0)
            {
                *Buf++ = (gctCHAR)(exp % 10 + '0');
                break;
            }
            else
            {
                *Buf++ = (gctCHAR)(exp / 10 + '0');
                exp = exp % 10;
            }
        }while(exp);
    }
    else
    {
        *Buf++ = '0';
        if (Precision > 0)
        {
            *Buf++ = '.';
            for (pos = 0; pos < Precision; pos++)
            {
                *Buf++ = '0';
            }
        }
    }
}

void printf_aA(
    gctCHAR *Str,
    double Num,
    gctINT Size,
    gctINT Precision,
    gctCHAR Fmt,
    gctINT Flags)
{
    gctCHAR tmp[255] = {'\0'};
    gctCHAR c, sign;
    gctINT n, i;
    gctBOOL isUpperCase = gcvFALSE;

    if (Flags & LEFT) Flags &= ~ZEROPAD;

    c = (Flags & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (Flags & SIGN)
    {
        if (Num < 0.0)
        {
            sign = '-';
            Num = -Num;
            Size--;
        }
        else if (Flags & PLUS)
        {
            sign = '+';
            Size--;
        }
        else if (Flags & SPACE)
        {
            sign = ' ';
            Size--;
        }
    }

    if (Precision < 0)
    {
        Precision = 6; /* Default precision: 6 */
    }

    isUpperCase = (Fmt == 'A') ? gcvTRUE : gcvFALSE;

    floatToText(Num, Precision, tmp, isUpperCase);

    n = (gctINT)strlen(tmp);

    Size -= n;
    if (!(Flags & (ZEROPAD | LEFT)))
    {
        while (Size-- > 0)
        {
            *Str++ = ' ';
        }
    }
    if (sign)
    {
        *Str++ = sign;
    }
    if (!(Flags & LEFT))
    {
        while (Size-- > 0)
        {
            *Str++ = c;
        }
    }
    for (i = 0; i < n; i++)
    {
        *Str++ = tmp[i];
    }
    while (Size-- > 0)
    {
        *Str++ = ' ';
    }
}

void
gcfVX_PrintData(
    gctPOINTER** Data,
    gctCHAR* Format,
    gctINT ArgType,
    gctCHAR FollowType,
    gctINT Flags,
    gctINT FieldWidth,
    gctINT Precision,
    gctBOOL IsDoublePrecision)
{
    if (ArgType)
    {
        switch (ArgType)
        {
        case cleARGTYPE_CHAR:
            printf(Format, *(gctINT8*)(**Data));
            **Data = (gctCHAR*)(**Data) + 1;
            break;
        case cleARGTYPE_UCHAR:
            printf(Format, *(gctUINT8*)(**Data));
            **Data = (gctUINT8*)(**Data) + 1;
            break;
        case cleARGTYPE_SHORT:
            printf(Format, *(gctINT16*)(**Data));
            **Data = (gctINT16*)(**Data) + 1;
            break;
        case cleARGTYPE_USHORT:
            printf(Format, *(gctUINT16*)(**Data));
            **Data = (gctUINT16*)(**Data) + 1;
            break;
        case cleARGTYPE_LONG:
            printf(Format, *(gctINT64*)(**Data));
            **Data = (gctINT64*)(**Data) + 1;
            break;
        case cleARGTYPE_ULONG:
            printf(Format, *(gctINT64*)(**Data));
            **Data = (gctINT64*)(**Data) + 1;
            break;
        case cleARGTYPE_HALF:
            printf(Format, *(gctINT16*)(**Data));
            **Data = (gctINT16*)(**Data) + 1;
            break;
        case cleARGTYPE_INT:
            printf(Format, *(gctINT32*)(**Data));
            **Data = (gctINT32*)(**Data) + 1;
            break;
        case cleARGTYPE_UINT:
            printf(Format, *(gctUINT32*)(**Data));
            **Data = (gctUINT32*)(**Data) + 1;
            break;
        case cleARGTYPE_FLOAT:
            {
                gctFLOAT value = *(gctFLOAT*)(**Data);
                if (gcfVX_IsNan(value))
                {
                    printf("%s", "nan");
                }else if (gcfVX_IsInf(value))
                {
                    printf("%s", "inf");
                }else{
                    if (FollowType == 'a' || FollowType == 'A')
                    {
                        gctCHAR tmpBuf[512] = {'\0'};
                        printf_aA(tmpBuf, *(gctFLOAT*)(**Data), FieldWidth, Precision, FollowType, Flags);
                        printf("%s", tmpBuf);
                    }else{
                        printf(Format, value);
                    }
                }

                if (IsDoublePrecision == 0)
                {
                    **Data = (gctFLOAT*)(**Data) + 1;
                }else{
                    **Data = (double*)(**Data) + 1;
                }
            }
            break;
        case cleARGTYPE_DOUBLE:
            {
                gctFLOAT value = *(gctFLOAT*)(**Data);
                if (gcfVX_IsNan(value))
                {
                    printf("%s", "nan");
                }else if (gcfVX_IsInf(value))
                {
                    printf("%s", "inf");
                }else{
                    if (FollowType == 'a' || FollowType == 'A')
                    {
                        gctCHAR tmpBuf[512] = {'\0'};
                        printf_aA(tmpBuf, *(double*)(**Data), FieldWidth, Precision, FollowType, Flags);
                        printf("%s", tmpBuf);
                    }else{
                        printf(Format, *(double*)(**Data));
                    }
                }
                if (IsDoublePrecision == 0)
                {
                    **Data = (gctFLOAT*)(**Data) + 1;
                }else{
                    **Data = (double*)(**Data) + 1;
                }
            }
            break;
        default:
            break;
        }
    }
    else
    {
        switch(FollowType)
        {
         case 'd':
         case 'i':
         case 'o':
         case 'u':
         case 'x':
         case 'X':
             printf(Format, *(gctINT32*)(**Data));
             **Data = (gctINT32*)(**Data) + 1;
             break;
         case 'a':
         case 'A':
         case 'e':
         case 'E':
         case 'f':
         case 'F':
         case 'g':
         case 'G':
             {
                 gctFLOAT value = *(gctFLOAT*)(**Data);
                 if (gcfVX_IsNan(value))
                 {
                     printf("%s", "nan");
                 }else if (gcfVX_IsInf(value))
                 {
                     printf("%s", "inf");
                 }else if (FollowType == 'a' || FollowType == 'A')
                 {
                     gctCHAR tmpBuf[512] = {'\0'};
                     printf_aA(tmpBuf, value, FieldWidth, Precision, FollowType, Flags);
                     printf("%s", tmpBuf);
                 }else
                 {
                     printf(Format, value);
                 }
                 **Data = (gctFLOAT*)(**Data) + 1;
             }
             break;
         case 'c':
             printf(Format, *(gctINT8*)(**Data));
             **Data = (gctINT8*)(**Data) + 1;
             break;
         default:
            break;
        }
    }
}

void
gcfVX_PrintfFmt(
    gctCHAR* FmtString,
    gctCHAR SpecificChar,
    gctPOINTER* Data,
    gctINT VectorSize,
    gctINT ArgType,
    gctINT Flags,
    gctINT FieldWidth,
    gctINT Precision
    )
{
    gctINT i = 0;
    gctINT numVector = (VectorSize == 0) ? 1 : VectorSize;
    gctINT isDoublePrecision = 0;
    /* Get precision info */
    isDoublePrecision = *(gctINT*)(*Data);
    /* Get Data */
    *Data = (gctINT*)(*Data) + 1;
    switch(SpecificChar) {
        case 'd':
        case 'i':
        case 'o':
        case 'u':
        case 'x':
        case 'X':
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'a':
        case 'A':
        case 'c':
            while (i < numVector)
            {
                if (i > 0 && i < numVector)
                {
                    printf(",");
                }
                gcfVX_PrintData(&Data, FmtString, ArgType, SpecificChar, Flags, FieldWidth, Precision, isDoublePrecision);
                i++;
            }
            break;

        case 'p':
            printf("%016x", *(gctUINT*)(*Data));
            *Data = (gctUINT*)(*Data) + 1;
            break;

        default:
            break;
    }
}

void gcfVX_PrintParseData(
    gctCHAR *formatString,
    gctPOINTER data)
{
    gctCHAR* bufPtr = formatString;

    while(*bufPtr) {
        gctCHAR c = *bufPtr++;

        if(c == '%') {
            if(*bufPtr == '%') {
                bufPtr++;
                continue;
            }else {
                /*indentify the conversion specification */
                gctCHAR* startPtr = bufPtr - 1;
                while (*bufPtr)
                {
                    c = *bufPtr++;
                    if (gcfVX_IsInString(c, (gctCHAR*)_cldPrintfConvSpecifiers))
                    {
                        gctINT vectorSize = 0;
                        gctINT argType = 0;
                        gctINT flags = 0;
                        gctINT fieldWidth = 0;
                        gctINT precision = 0;
                        gctCHAR* endPtr = bufPtr - 1;
                        gctCHAR fmt[255] = {'\0'};
                        /* get vector size and length and new format */
                        gcfVX_GetSingleFormat(startPtr, endPtr, fmt, &vectorSize, &argType, &flags, &fieldWidth, &precision);
                        if (c == 's')
                        {
                            gctUINT value = *((gctINT*)data + 1);
                            if (value == 0xFFFFFFFF) /* handle printf("%s", 0);*/
                            {
                                printf(fmt, "(null)");
                            }else{
                                /* the string is stored in const buffer, offset is stored in printf buffer */
                                gctUINT offset = value;
                                printf(fmt, formatString + offset);
                            }
                            data = (gctINT*)data + 2;
                        }else{
                            gcfVX_PrintfFmt(fmt, c, &data, vectorSize, argType, flags, fieldWidth, precision);
                        }
                        break;
                    }
                }
            }
        }else {
            gctINT constStrLen = 0;
            gctCHAR* constString;
            gctCHAR* startConstStrPtr;
            bufPtr--;
            startConstStrPtr = bufPtr;

            while((*bufPtr) && (*bufPtr != '%'))
            {
                constStrLen++;
                bufPtr++;
            }
            gcoOS_Allocate(gcvNULL, (constStrLen+1)*sizeof(gctCHAR), (gctPOINTER *)&constString);
            gcoOS_StrCopySafe(constString, constStrLen + 1, startConstStrPtr);
            *(constString + constStrLen) = '\0';
            printf("%s", constString);
            gcoOS_Free(gcvNULL, constString);
        }
    }
}

gceSTATUS
gcfVX_AdjustLocalWorkSize(
    vx_shader    Kernel,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3]
    )
{

    if (LocalWorkSize[0] == 0 &&
        (WorkDim < 2 || LocalWorkSize[1] == 0) &&
        (WorkDim < 3 || LocalWorkSize[2] == 0))
    {
        gctSIZE_T           preferredWorkGroupSize;
        gctSIZE_T           workGroupSize = 1;
        gctUINT             i;

        /* Find the largest workgroup size which has a common multiple with
         * preferred workgroup size but still less than max work group size
         */
        for (i = 0; i < WorkDim; i++)
        {
            preferredWorkGroupSize = Kernel->preferredWorkGroupSizeMultiple;
            while (preferredWorkGroupSize % 2 == 0)
            {
                if ((GlobalWorkSize[i] % preferredWorkGroupSize == 0) &&
                    (workGroupSize * preferredWorkGroupSize <= Kernel->maxWorkGroupSize))
                {
                    LocalWorkSize[i] = preferredWorkGroupSize;
                    workGroupSize *= LocalWorkSize[i];
                    break;
                }
                preferredWorkGroupSize /= 2;
            }
        }

        if (workGroupSize == 1)
        {

            /* No common multiple found
             * Try adjusting wrt global work size
             */
            for (i=0; i<WorkDim; i++)
            {
                if (workGroupSize * GlobalWorkSize[i] <= Kernel->maxWorkGroupSize)
                {
                    LocalWorkSize[i] = GlobalWorkSize[i];
                    workGroupSize *= LocalWorkSize[i];
                }
            }
        }

        /*not lucky, so just keep localworksize invalid */
        for (i = 0; i< WorkDim; i++)
        {
            if (LocalWorkSize[i] == 0)
            {
                LocalWorkSize[i] = 1;
            }
        }
    }

    return gcvSTATUS_OK;
}


/* Return how many uniform entries will be reported to app */
static gctUINT
gcfVX_GetUniformArrayInfo(
    gcUNIFORM uniform,
    gctUINT *maxNameLen, /* max possible name len of this entry, exclude bottom-level "[0]" */
    gctBOOL *isArray,
    gctUINT *arraySize
)
{
    gctINT j;
    gctUINT32 length;
    gctUINT entries = 1;

    gcUNIFORM_GetName(uniform, &length, gcvNULL);

    /* Multiple entries will be reported for array of arrays.
    ** If a uniform is an array, or array of array, its name length should be added
    ** with several dim of array indices, like "name[x]...[x][0]".
    */
    for (j = 0; j < uniform->arrayLengthCount - 1; ++j)
    {
        gctUINT decimalLen = 1;
        gctINT arrayLen = uniform->arrayLengthList[j];

        gcmASSERT(arrayLen > 0);
        entries *= arrayLen;

        arrayLen--;    /* Get max arrayIndex */
        while (arrayLen >= 10)
        {
            ++decimalLen;
            arrayLen /= 10;
        }

        length += (decimalLen + 2);
    }

    if (maxNameLen)
    {
        *maxNameLen = length;
    }

    if (isArray)
    {
        *isArray = uniform->arrayLengthCount > 0 ? gcvTRUE : gcvFALSE;
    }

    if (arraySize)
    {
        *arraySize = uniform->arrayLengthCount > 0
                   ? (uniform->arrayLengthList[uniform->arrayLengthCount - 1] > 0 ?
                      uniform->arrayLengthList[uniform->arrayLengthCount - 1] : 0)
                   : 1;
    }

    return entries;
}

gceSTATUS
gcfVX_LoadKernelArgLocalMemValues(
    vx_shader    Kernel,
    gctUINT             NumArgs,
    vx_argument         Args,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3]
    )
{
    gcSHADER_TYPE       type;
    gctUINT             length;
    vx_argument         argument;
    gctINT              totalNumGroups;
    gctINT              totalSize = 0;
    gctINT              totalAlignSize = 0;
    gctINT              totalArgSize = 0;
    gctINT              totalArgAlignSize = 0;
    gctUINT             i;
    vx_mem_alloc_info   memAllocInfo;
    gctINT *            data;
    gctUINT             allocatedSize = 0;
    gctPHYS_ADDR        physical = gcvNULL;
    gctPOINTER          logical = gcvNULL;
    gcsSURF_NODE_PTR    node = gcvNULL;
    gceSTATUS           status;

    /* Add up sizes for all local kernel args. */
    for (i = 0; i < NumArgs; i++)
    {
        argument = &Args[i];

        if (argument->uniform == gcvNULL) continue;

        if (isUniformInactive(argument->uniform)) continue;

        if (isUniformKernelArgLocal(argument->uniform))
        {
            memAllocInfo = (vx_mem_alloc_info) argument->data;

            if (!argument->isMemAlloc)
            {
                status = gcvSTATUS_INVALID_DATA;
                goto OnError;
            }

            if (memAllocInfo->allocatedSize <= 0)
            {
                status = gcvSTATUS_INVALID_DATA;
                goto OnError;
            }

            totalSize += gcmALIGN(memAllocInfo->allocatedSize, 4);
            totalAlignSize = gcmALIGN(totalSize, memAllocInfo->allocatedSize);
            totalSize = totalAlignSize;
        }
    }

    if (totalSize > 0)
    {
        totalNumGroups = (gctINT)(GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1)
                       * (WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 1)
                       * (WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 1));

        allocatedSize = totalSize * totalNumGroups;

        /* Allocate the physical buffer */
        gcmONERROR(gcoVX_AllocateMemoryEx(&allocatedSize, &physical, &logical, &node));

        /* Set up relative address for all local kernel args. */
        for (i = 0; i < NumArgs; i++)
        {
            argument = &Args[i];

            if (argument->uniform == gcvNULL) continue;

            gcmONERROR(gcUNIFORM_GetType(argument->uniform, &type, &length));

            if (isUniformInactive(argument->uniform)) continue;

            if (isUniformKernelArgLocal(argument->uniform))
            {
                memAllocInfo = (vx_mem_alloc_info) argument->data;

                if (!argument->isMemAlloc)
                {
                    status = gcvSTATUS_INVALID_DATA;
                    goto OnError;
                }

                if (memAllocInfo->allocatedSize <= 0)
                {
                    status = gcvSTATUS_INVALID_DATA;
                    goto OnError;
                }

                totalArgAlignSize = gcmALIGN(totalArgSize, memAllocInfo->allocatedSize);
                memAllocInfo->physical = (gctPHYS_ADDR)((gctUINTPTR_T)physical + totalArgAlignSize);
                totalArgSize += gcmALIGN(memAllocInfo->allocatedSize, 4);

                data = (gctINT *) &memAllocInfo->physical;
                gcmONERROR(gcUNIFORM_SetValue(argument->uniform, length, data));
            }
            else if (isUniformKernelArgLocalMemSize(argument->uniform))
            {
                memAllocInfo = (vx_mem_alloc_info) argument->data;

                if (!argument->isMemAlloc)
                {
                    status = gcvSTATUS_INVALID_DATA;
                    goto OnError;
                }

                memAllocInfo->allocatedSize = allocatedSize;
                memAllocInfo->node          = node;

                data = (gctINT *) &totalSize;
                gcmONERROR(gcUNIFORM_SetValue(argument->uniform, length, data));
            }
        }
    }

    status = gcvSTATUS_OK;

OnError:
    if(node && gcmIS_ERROR(status))
    {
       gcoVX_FreeMemoryEx(physical, logical, allocatedSize, node);
    }
    return status;
}

static gceSTATUS
gcfVX_LoadKernelArgValues(
    vx_shader    Kernel,
    gcSHADER            Shader,
    vx_argument         Arg,
    vx_border_mode_t*   BorderMode,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3]
    )
{
    gcSHADER_TYPE       type;
    gctUINT             length;
    gcSL_FORMAT         format;
    gctBOOL             isPointer;
    gceUNIFORM_FLAGS    flags;
    gceSTATUS           status;
    gcoVX_Kernel_Context *context = gcvNULL;

    if (Arg->uniform == gcvNULL)
    {
        return gcvSTATUS_OK;
    }

    gcmONERROR(gcUNIFORM_GetType(Arg->uniform, &type, &length));

    gcmONERROR(gcUNIFORM_GetFormat(Arg->uniform, &format, &isPointer));

    gcmONERROR(gcUNIFORM_GetFlags(Arg->uniform, &flags));

    if (isUniformKernelArg(Arg->uniform) ||
        isUniformKernelArgConstant(Arg->uniform))
    {
        if (isPointer)
        {
            if ((type == gcSHADER_IMAGE_2D) && (Arg->data != gcvNULL))
            {
                vx_image image = *(vx_image*) Arg->data;

                if (image)
                {
                    gcsVX_IMAGE_INFO     info = {0};

                    context = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));

#if gcdVX_OPTIMIZER
                    context->borders = BorderMode->mode;
#else
                    context->params.borders = BorderMode->mode;
#endif

                    gcmONERROR(gcfVX_GetImageInfo(context, (vx_image)image, &info, 0));

                    info.isVXC = gcvTRUE;

                    vxFree(context);

                    context = gcvNULL;

                    gcmONERROR(gcoVX_BindImage(GetUniformPhysical(Arg->uniform), &info));
                }
                else
                {
                    gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                        length,
                                        (gctINT*)Arg->data));
                }
            }
            else if ((type == gcSHADER_IMAGE_2D_ARRAY) && (Arg->data != gcvNULL))
            {
                vx_reference ref = *(vx_reference*) Arg->data;

                if (ref && ref->type == VX_TYPE_OBJECT_ARRAY)
                {
                    vx_object_array imageArray = (vx_object_array) ref;

                    gcsVX_IMAGE_INFO     info = {0};
                    gcoVX_Kernel_Context context = {0};

                    vx_image_s image =  *(vx_image)imageArray->itemsTable[0];

                    /* warp to an imageArray/image3D */
                    image.arraySize = (gctUINT32)imageArray->itemCount;


#if gcdVX_OPTIMIZER
                    context.borders = BorderMode->mode;
#else
                    context.params.borders = BorderMode->mode;
#endif

                    gcmONERROR(gcfVX_GetImageInfo(&context, (vx_image)&image, &info, 0));

                    info.isVXC = gcvTRUE;

                    gcmONERROR(gcoVX_BindImage(GetUniformPhysical(Arg->uniform), &info));
                }
                else if (ref && ref->type == VX_TYPE_TENSOR)
                {
                    vx_tensor tensor = (vx_tensor) ref;
                    gcsVX_IMAGE_INFO     info = {0};

                    gcmONERROR(gcfVX_GetImageInfoFromTensor(BorderMode->mode, tensor, &info));

                    info.isVXC = gcvTRUE;

                    gcmONERROR(gcoVX_BindImage(GetUniformPhysical(Arg->uniform), &info));
                }
                else
                {
                    gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                        length,
                                        (gctINT*)Arg->data));
                }
            }
            else
            {
                gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                        length,
                                        (gctINT*)Arg->data));
            }
        }
        else
        {
            gctUINT arraySize;
            gctUINT32 physicalAddress, numCols, numRows;
            gctUINT8_PTR pData = (gctUINT8_PTR)Arg->data;
            switch (type)
            {
            case gcSHADER_FLOAT_X1:
            case gcSHADER_FLOAT_X2:
            case gcSHADER_FLOAT_X3:
            case gcSHADER_FLOAT_X4:
            case gcSHADER_FLOAT_2X2:
            case gcSHADER_FLOAT_3X3:
            case gcSHADER_FLOAT_4X4:
                gcmONERROR(gcUNIFORM_SetValueF(Arg->uniform,
                                               length,
                                               (gctFLOAT*)Arg->data));
                break;

            case gcSHADER_BOOLEAN_X1:
            case gcSHADER_BOOLEAN_X2:
            case gcSHADER_BOOLEAN_X3:
            case gcSHADER_BOOLEAN_X4:
            case gcSHADER_INTEGER_X1:
            case gcSHADER_INTEGER_X2:
            case gcSHADER_INTEGER_X3:
            case gcSHADER_INTEGER_X4:
            case gcSHADER_UINT_X1:
            case gcSHADER_UINT_X2:
            case gcSHADER_UINT_X3:
            case gcSHADER_UINT_X4:

                switch (format)
                {
                case gcSL_INT8:
                case gcSL_UINT8:
                case gcSL_INT16:
                case gcSL_UINT16:
                    {
                        /* Unpack argument data into 4B chunks */
                        gctUINT32 *data= gcvNULL;
                        gctUINT32 signMask, signExt;
                        gctPOINTER pointer= gcvNULL;
                        gctSIZE_T i, elemSize, numElem, bytes;

                        elemSize =  ((format == gcSL_INT8)   ? 1 :
                                     (format == gcSL_UINT8)  ? 1 :
                                     (format == gcSL_INT16)  ? 2 :
                                     (format == gcSL_UINT16) ? 2 : 0);

                        signMask =  ((format == gcSL_INT8)   ? 0x80 :
                                     (format == gcSL_INT16)  ? 0x8000 : 0);

                        signExt =   ((format == gcSL_INT8)   ? 0xFFFFFF00 :
                                     (format == gcSL_INT16)  ? 0xFFFF0000 : 0);

                        numElem = Arg->size / elemSize;
                        bytes = numElem * 4;

                        gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                        gcoOS_ZeroMemory(pointer, bytes);
                        data = (gctUINT32 *) pointer;
                        for (i=0; i<numElem; i++) {
                            pointer = (gctPOINTER)(((gctUINTPTR_T)Arg->data)+(i*elemSize));
                            gcoOS_MemCopy(&data[i], pointer, elemSize);
                            if (data[i]&signMask) {
                                data[i] |= signExt;
                            }
                        }
                        pointer = data;
                        status = gcUNIFORM_SetValue(Arg->uniform, length, (gctINT*)pointer);
                        gcoOS_Free(gcvNULL, data);
                        if (gcmIS_ERROR(status)) goto OnError;

                    }
                    break;

                default:
                    gcmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, (gctINT*)Arg->data));
                    break;
                }
                break;

            case gcSHADER_INT64_X1:
            case gcSHADER_INT64_X2:
            case gcSHADER_INT64_X3:
            case gcSHADER_INT64_X4:
            case gcSHADER_UINT64_X1:
            case gcSHADER_UINT64_X2:
            case gcSHADER_UINT64_X3:
            case gcSHADER_UINT64_X4:
                gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(Kernel->states.hints->hwConstRegBases,
                                                                   Arg->uniform,
                                                                   &physicalAddress));

                gcTYPE_GetTypeInfo(GetUniformType(Arg->uniform), &numCols, &numRows, gcvNULL);

                arraySize = Arg->uniform->arraySize;

                gcmONERROR(gcoSHADER_ProgramUniformEx(gcvNULL, physicalAddress,
                                                        numCols, numRows, arraySize, gcvTRUE,
                                                        sizeof(gctINT64),
                                                        4*sizeof(gctINT64),
                                                        pData, gcvUNIFORMCVT_NONE, GetUniformShaderKind(Arg->uniform)));
                break;

            case gcSHADER_IMAGE_2D:
            case gcSHADER_IMAGE_3D:
                {
                vx_image image = *(vx_image*) Arg->data;

                gcsVX_IMAGE_INFO     info = {0};

                context = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));
#if gcdVX_OPTIMIZER
                context->borders = VX_BORDER_UNDEFINED;
#else
                context->params.borders = BorderMode->mode;
#endif

                gcmONERROR(gcfVX_GetImageInfo(context, (vx_image)image, &info, 1));

                info.isVXC = gcvTRUE;

                vxFree(context);

                context = gcvNULL;

                gcmONERROR(gcoVX_BindImage(0, &info));

                }
                break;

            case gcSHADER_SAMPLER:
                {
                gcmASSERT(length == 1);
                gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                              length,
                                              (gctINT*)Arg->data));
                }
                break;

            default:
                break;
            }
        }
    }
    else if (isUniformWorkDim(Arg->uniform))
    {
        gcoOS_MemCopy(Arg->data, &WorkDim, gcmSIZEOF(WorkDim));

        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      (gctINT*)Arg->data));
    }
    else if (isUniformGlobalSize(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, GlobalWorkSize, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 globalWorkSize[3];

        globalWorkSize[0] = (gctUINT32)GlobalWorkSize[0];
        globalWorkSize[1] = (gctUINT32)GlobalWorkSize[1];
        globalWorkSize[2] = (gctUINT32)GlobalWorkSize[2];
        gcoOS_MemCopy(Arg->data, globalWorkSize, gcmSIZEOF(gctUINT32) * 3);

        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      (gctINT*)Arg->data));
    }
    else if (isUniformLocalSize(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, LocalWorkSize, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 localWorkSize[3];

        localWorkSize[0] = (gctUINT32)(LocalWorkSize[0]);
        localWorkSize[1] = (gctUINT32)(LocalWorkSize[1]);
        localWorkSize[2] = (gctUINT32)(LocalWorkSize[2]);
        gcoOS_MemCopy(Arg->data, localWorkSize, gcmSIZEOF(gctUINT32) * 3);

        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      (gctINT*)Arg->data));
    }
    else if (isUniformNumGroups(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*size_t numGroups[3];*/
        gctUINT32 numGroups[3];

        numGroups[0] = (gctUINT32)(GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1));
        numGroups[1] = (gctUINT32)(WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 0);
        numGroups[2] = (gctUINT32)(WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 0);

        gcoOS_MemCopy(Arg->data, numGroups, gcmSIZEOF(numGroups));

        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      (gctINT*)Arg->data));

    }
    else if (isUniformGlobalOffset(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, GlobalWorkOffset, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 glocalWorkOffset[3];

        glocalWorkOffset[0] = (gctUINT32)(GlobalWorkOffset[0]);
        glocalWorkOffset[1] = (gctUINT32)(GlobalWorkOffset[1]);
        glocalWorkOffset[2] = (gctUINT32)(GlobalWorkOffset[2]);
        gcoOS_MemCopy(Arg->data, glocalWorkOffset, gcmSIZEOF(gctUINT32) * 3);

        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      (gctINT*)Arg->data));
    }
    else if (isUniformLocalAddressSpace(Arg->uniform) ||
             isUniformPrivateAddressSpace(Arg->uniform))
    {
        vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Arg->data;

        /* Size may be zero if declared but never used */
        if (memAllocInfo->allocatedSize > 0)
        {
            gctINT * data;

            gctINT totalNumGroups = (gctINT)(GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1)
                                  * (WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 1)
                                  * (WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 1));

            gctINT totalNumItems  = (gctINT)(GlobalWorkSize[0]
                                  * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                                  * (WorkDim > 2 ? GlobalWorkSize[2] : 1));

            if (!Arg->isMemAlloc)
            {
                status = gcvSTATUS_INVALID_DATA;
                goto OnError;
            }

            memAllocInfo->allocatedSize *= isUniformPrivateAddressSpace(Arg->uniform) ? totalNumItems : totalNumGroups;

            /* Allocate the physical buffer */
            gcmONERROR(gcoVX_AllocateMemoryEx(&memAllocInfo->allocatedSize,
                                            &memAllocInfo->physical,
                                            &memAllocInfo->logical,
                                            &memAllocInfo->node));

            data = (gctINT *) &memAllocInfo->physical;
            gcmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data));
        }
    }
    else if (isUniformConstantAddressSpace(Arg->uniform) && (GetUniformPhysical(Arg->uniform) != -1))
    {
        vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Arg->data;
        gctINT * data;

        if (!Arg->isMemAlloc)
        {
            status = gcvSTATUS_INVALID_DATA;
            goto OnError;
        }

        if (memAllocInfo->allocatedSize <= 0)
        {
            status = gcvSTATUS_INVALID_DATA;
            goto OnError;
        }
        /* Allocate the physical buffer */
        gcmONERROR(gcoVX_AllocateMemoryEx(&memAllocInfo->allocatedSize,
                                        &memAllocInfo->physical,
                                        &memAllocInfo->logical,
                                        &memAllocInfo->node));

        data = (gctINT *) &memAllocInfo->physical;
        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data));

        /* Copy the constant data to the buffer */
        gcoOS_MemCopy(memAllocInfo->logical, Kernel->constantMemBuffer, Kernel->constantMemSize);

    }
    else if (isUniformPrintfAddress(Arg->uniform))
    {
        vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Arg->data;
        gctINT * data;

        if (!Arg->isMemAlloc)
        {
            status = gcvSTATUS_INVALID_DATA;
            goto OnError;
        }


        if (memAllocInfo->node)
        {
            gcmONERROR(gcoVX_FreeMemoryEx(memAllocInfo->physical,
                        memAllocInfo->logical,
                        memAllocInfo->allocatedSize,
                        memAllocInfo->node));

            gcoOS_ZeroMemory(memAllocInfo, sizeof(vx_mem_alloc_info_s));
        }

        memAllocInfo->allocatedSize = VX_MAX_PRINTF_BUFFER_SIZE;

        {
            /* Allocate the physical buffer */
            gcmONERROR(gcoVX_AllocateMemoryEx(&memAllocInfo->allocatedSize,
                                        &memAllocInfo->physical,
                                        &memAllocInfo->logical,
                                        &memAllocInfo->node));
        }

        data = (gctINT *) &memAllocInfo->physical;
        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data));

    }
    else if (isUniformWorkItemPrintfBufferSize(Arg->uniform))
    {
        gctINT totalNumItems  = (gctINT)(GlobalWorkSize[0]
                                  * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                                  * (WorkDim > 2 ? GlobalWorkSize[2] : 1));

        gctINT printBufferSize = VX_MAX_PRINTF_BUFFER_SIZE / totalNumItems;
        gcoOS_MemCopy(Arg->data, &printBufferSize, gcmSIZEOF(printBufferSize));

        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      (gctINT*)Arg->data));
    }
    else if (isUniformKernelArgSampler(Arg->uniform))
    {
        gcmASSERT(length == 1);
        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                              length,
                                              (gctINT*)Arg->data));

    }
    else if (isUniformKernelArgLocal(Arg->uniform) ||
                isUniformKernelArgLocalMemSize(Arg->uniform))
    {
        /* Special case, handled separately, do nothing now */
    }
    else if (isUniformKernelArgPrivate(Arg->uniform))
    {
        vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Arg->data;
        gctINT * data;
        gctINT i;
        gctPOINTER pointer;

        gctINT totalNumItems  = (gctINT)(GlobalWorkSize[0]
                              * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                              * (WorkDim > 2 ? GlobalWorkSize[2] : 1));

        if (!Arg->isMemAlloc)
        {
            status = gcvSTATUS_INVALID_DATA;
            goto OnError;
        }

        if (memAllocInfo->allocatedSize <= 0)
        {
            status = gcvSTATUS_INVALID_DATA;
            goto OnError;
        }

        memAllocInfo->allocatedSize *= totalNumItems;

        /* Allocate the physical buffer */
        gcmONERROR(gcoVX_AllocateMemoryEx(&memAllocInfo->allocatedSize,
                                        &memAllocInfo->physical,
                                        &memAllocInfo->logical,
                                        &memAllocInfo->node));

        data = (gctINT *) &memAllocInfo->physical;
        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data));

        /* Copy the private data to the buffer */
        for (i = 0; i < totalNumItems; i++)
        {
            pointer = (gctPOINTER)((gctUINTPTR_T)memAllocInfo->logical + i*Arg->size);
            gcoOS_MemCopy(pointer, memAllocInfo->data, Arg->size);
        }
    }
    else if (isUniformKernelArgPatch(Arg->uniform))
    {
        switch (type)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
        case gcSHADER_FLOAT_2X2:
        case gcSHADER_FLOAT_3X3:
        case gcSHADER_FLOAT_4X4:
            gcmONERROR(gcUNIFORM_SetValueF(Arg->uniform,
                                           length,
                                           (gctFLOAT*)Arg->data));
            break;

        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_BOOLEAN_X4:
        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:

            switch (format)
            {
            case gcSL_INT8:
            case gcSL_UINT8:
            case gcSL_INT16:
            case gcSL_UINT16:
                {
                    /* Unpack argument data into 4B chunks */
                    gctUINT32 *data= gcvNULL;
                    gctUINT32 signMask, signExt;
                    gctPOINTER pointer= gcvNULL;
                    gctSIZE_T i, elemSize, numElem, bytes;

                    elemSize =  ((format == gcSL_INT8)   ? 1 :
                                 (format == gcSL_UINT8)  ? 1 :
                                 (format == gcSL_INT16)  ? 2 :
                                 (format == gcSL_UINT16) ? 2 : 0);

                    signMask =  ((format == gcSL_INT8)   ? 0x80 :
                                 (format == gcSL_INT16)  ? 0x8000 : 0);

                    signExt =   ((format == gcSL_INT8)   ? 0xFFFFFF00 :
                                 (format == gcSL_INT16)  ? 0xFFFF0000 : 0);

                    numElem = Arg->size / elemSize;
                    bytes = numElem * 4;

                    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                    gcoOS_ZeroMemory(pointer, bytes);
                    data = (gctUINT32 *) pointer;
                    for (i=0; i<numElem; i++) {
                        pointer = (gctPOINTER)(((gctUINTPTR_T)Arg->data)+(i*elemSize));
                        gcoOS_MemCopy(&data[i], pointer, elemSize);
                        if (data[i]&signMask) {
                            data[i] |= signExt;
                        }
                    }
                    pointer = data;
                    status = gcUNIFORM_SetValue(Arg->uniform, length, (gctINT*)pointer);
                    gcoOS_Free(gcvNULL, data);
                    if (gcmIS_ERROR(status)) goto OnError;

                }
                break;

            default:
                gcmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, (gctINT*)Arg->data));
                break;
            }
            break;

        default:
            break;
        }
    }
    else if (gcoOS_StrCmp(Arg->uniform->name, "$ConstBorderValue") == gcvSTATUS_OK)
    {
        gctUINT32   data[4] = {0};
        gctUINT32   data32 = (gctUINT32)BorderMode->constant_value.U32;

        gctUINT8  data8 = data32 & 0xFF;
        gctUINT16 data16 = data32 & 0xFFFF;
        gctFLOAT  dataf  = (gctFLOAT)(data32);

        data[0] = data8 | (data8 << 8) | (data8 << 16) | (data8 << 24);

        data[1] = data16 | (data16 << 16);

        data[2] = data32;

        data[3] = *(gctUINT32*)(&dataf);

        gcmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, (gctINT*)data));
    }
    else
    {
        gcsUNIFORM_BLOCK uniformBlock = gcvNULL;
        gcUNIFORM bUniform = gcvNULL;
        gctINT16   blockIndex = GetUniformBlockID(Arg->uniform);
        gctUINT32 physicalAddress = 0;
        gctUINT      entries, arraySize;
        gctUINT32    numCols = 0, numRows = 0;
        if ((blockIndex >= 0) && (GetUniformPhysical(Arg->uniform) != -1))
        {
            gcmONERROR(gcSHADER_GetUniformBlock(Shader, blockIndex, &uniformBlock));
            gcmONERROR(gcSHADER_GetUniform(Shader, GetUBIndex(uniformBlock), &bUniform));
            if(isUniformConstantAddressSpace(bUniform))
            {
                gctUINT8_PTR pData = (gctUINT8_PTR)(Kernel->constantMemBuffer) + GetUniformOffset(Arg->uniform);

                gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(Kernel->states.hints->hwConstRegBases,
                                                                   Arg->uniform,
                                                                   &physicalAddress));

                gcTYPE_GetTypeInfo(GetUniformType(Arg->uniform), &numCols, &numRows, gcvNULL);

                entries = gcfVX_GetUniformArrayInfo(Arg->uniform, gcvNULL, gcvNULL, &arraySize);
                arraySize *= entries;   /* Expand array size for array of arrays to one dimension */

                gcmONERROR(gcoSHADER_ProgramUniformEx(gcvNULL, physicalAddress,
                                                        numCols, numRows, arraySize, gcvFALSE,
                                                        (gctSIZE_T)GetUniformMatrixStride(Arg->uniform),
                                                        (gctSIZE_T)GetUniformArrayStride(Arg->uniform),
                                                        pData, gcvUNIFORMCVT_NONE, GetUniformShaderKind(Arg->uniform)));
            }
        }
        else
        {
            gctUINT arraySize;
            gctUINT32 physicalAddress, numCols, numRows;
            gctUINT8_PTR pData = (gctUINT8_PTR)Arg->data;
            switch (type)
            {
            case gcSHADER_FLOAT_X1:
            case gcSHADER_FLOAT_X2:
            case gcSHADER_FLOAT_X3:
            case gcSHADER_FLOAT_X4:
            case gcSHADER_FLOAT_2X2:
            case gcSHADER_FLOAT_3X3:
            case gcSHADER_FLOAT_4X4:
                gcmONERROR(gcUNIFORM_SetValueF(Arg->uniform,
                                               length,
                                               (gctFLOAT*)Arg->data));
                break;

            case gcSHADER_BOOLEAN_X1:
            case gcSHADER_BOOLEAN_X2:
            case gcSHADER_BOOLEAN_X3:
            case gcSHADER_BOOLEAN_X4:
            case gcSHADER_INTEGER_X1:
            case gcSHADER_INTEGER_X2:
            case gcSHADER_INTEGER_X3:
            case gcSHADER_INTEGER_X4:
            case gcSHADER_UINT_X1:
            case gcSHADER_UINT_X2:
            case gcSHADER_UINT_X3:
            case gcSHADER_UINT_X4:
            case gcSHADER_UINT_X16:

                switch (format)
                {
                case gcSL_INT8:
                case gcSL_UINT8:
                case gcSL_INT16:
                case gcSL_UINT16:
                    {
                        /* Unpack argument data into 4B chunks */
                        gctUINT32 *data= gcvNULL;
                        gctUINT32 signMask, signExt;
                        gctPOINTER pointer= gcvNULL;
                        gctSIZE_T i, elemSize, numElem, bytes;

                        elemSize =  ((format == gcSL_INT8)   ? 1 :
                                     (format == gcSL_UINT8)  ? 1 :
                                     (format == gcSL_INT16)  ? 2 :
                                     (format == gcSL_UINT16) ? 2 : 0);

                        signMask =  ((format == gcSL_INT8)   ? 0x80 :
                                     (format == gcSL_INT16)  ? 0x8000 : 0);

                        signExt =   ((format == gcSL_INT8)   ? 0xFFFFFF00 :
                                     (format == gcSL_INT16)  ? 0xFFFF0000 : 0);

                        numElem = Arg->size / elemSize;
                        bytes = numElem * 4;

                        gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                        gcoOS_ZeroMemory(pointer, bytes);
                        data = (gctUINT32 *) pointer;
                        for (i=0; i<numElem; i++) {
                            pointer = (gctPOINTER)(((gctUINTPTR_T)Arg->data)+(i*elemSize));
                            gcoOS_MemCopy(&data[i], pointer, elemSize);
                            if (data[i]&signMask) {
                                data[i] |= signExt;
                            }
                        }
                        pointer = data;
                        status = gcUNIFORM_SetValue(Arg->uniform, length, (gctINT*)pointer);
                        gcoOS_Free(gcvNULL, data);
                        if (gcmIS_ERROR(status)) goto OnError;

                    }
                    break;

                default:
                    gcmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, (gctINT*)Arg->data));
                    break;
                }
                break;

            case gcSHADER_INT64_X1:
            case gcSHADER_INT64_X2:
            case gcSHADER_INT64_X3:
            case gcSHADER_INT64_X4:
            case gcSHADER_UINT64_X1:
            case gcSHADER_UINT64_X2:
            case gcSHADER_UINT64_X3:
            case gcSHADER_UINT64_X4:
                gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(Kernel->states.hints->hwConstRegBases,
                                                                   Arg->uniform,
                                                                   &physicalAddress));

                gcTYPE_GetTypeInfo(GetUniformType(Arg->uniform), &numCols, &numRows, gcvNULL);

                arraySize = Arg->uniform->arraySize;

                gcmONERROR(gcoSHADER_ProgramUniformEx(gcvNULL, physicalAddress,
                                                        numCols, numRows, arraySize, gcvTRUE,
                                                        sizeof(gctINT64),
                                                        4*sizeof(gctINT64),
                                                        pData, gcvUNIFORMCVT_NONE, GetUniformShaderKind(Arg->uniform)));
                break;


            default:
                status = gcvSTATUS_INVALID_DATA;
                goto OnError;
            }
        }
    }

    status = gcvSTATUS_OK;

OnError:
    if (context != gcvNULL) {
        vxFree(context);
    }
    return status;
}

static vx_argument
gcfVX_GetKernelArg(
    vx_shader    Kernel,
    gctUINT             Index,
    gctBOOL *           isLocal,
    gctBOOL *           isPrivate,
    gctBOOL *           isSampler
    )
{
    vx_argument         arg;
    gctUINT             i, argIndex = 0;

    for (i = 0; i < Kernel->numArgs; i++)
    {
        arg = &Kernel->args[i];
        if (arg->uniform == gcvNULL) continue;
        if (! hasUniformKernelArgKind(arg->uniform)) continue;
        if (argIndex == Index)
        {
            if (isLocal) *isLocal = isUniformKernelArgLocal(arg->uniform);
            if (isPrivate) *isPrivate = isUniformKernelArgPrivate(arg->uniform);
            if (isSampler) *isSampler = isUniformKernelArgSampler(arg->uniform);
            return arg;
        }
        argIndex++;
    }
    return gcvNULL;
}

gceSTATUS
gcfVX_SetKernelArg(
    vx_shader           Kernel,
    vx_uint32           ArgIndex,
    vx_uint32           ArgSize,
    const void *        ArgValue
    )
{
    vx_argument     argument;
    gceSTATUS       status;
    gctBOOL         isLocal, isPrivate, isSampler;
    gctBOOL         acquired = gcvFALSE;

    if (Kernel == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    if (ArgIndex > Kernel->numArgs)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

 //   gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, Kernel->argMutex, gcvINFINITE));
 //   acquired = gcvTRUE;

    argument = gcfVX_GetKernelArg(Kernel, ArgIndex, &isLocal, &isPrivate, &isSampler);

    if (argument == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    if (isLocal)
    {
        if (ArgSize == 0)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }

        if (argument->isMemAlloc != gcvTRUE)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }

        ((vx_mem_alloc_info)argument->data)->allocatedSize = (gctINT)ArgSize;
        argument->size = ArgSize;
        Kernel->localMemSize += ArgSize;
    }
    else if (isPrivate)
    {
        vx_mem_alloc_info memAllocInfo;
        gctPOINTER pointer;

        if (ArgSize == 0)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }

        if (argument->isMemAlloc != gcvTRUE)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }

        memAllocInfo = (vx_mem_alloc_info) argument->data;
        memAllocInfo->allocatedSize = (gctINT)ArgSize;
        argument->size = ArgSize;

        status = gcoOS_Allocate(gcvNULL, ArgSize, &pointer);
        if (gcmIS_ERROR(status))
        {
            status = gcvSTATUS_OUT_OF_MEMORY;
            goto OnError;
        }
        memAllocInfo->data = pointer;

        gcoOS_MemCopy(memAllocInfo->data, ArgValue, ArgSize);
    }
    else if (isSampler)
    {
    }
    else
    {
        if(ArgSize != argument->size)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            goto OnError;
        }

        gcoOS_MemCopy(argument->data, ArgValue, ArgSize);
    }

    argument->set = gcvTRUE;


    status = gcvSTATUS_OK;

OnError:
    if (acquired)
    {
       // gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    }

    return status;
}

gceSTATUS
gcfVX_AllocateKernelArgs(
    vx_shader   Kernel
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gctPOINTER      pointer;
    gctUINT32       bytes;
    vx_argument     argument;
    gctUINT         i, uniformCount;
    gcSHADER        shader = (gcSHADER)Kernel->states.binary;
    gcUNIFORM       uniform;

        /* Get the number of uniforms. */
    gcmONERROR(gcSHADER_GetUniformCount(
                        shader,
                        &uniformCount));


    if (uniformCount == 0)
    {
        Kernel->args = gcvNULL;
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    Kernel->numArgs = 0;

    for (i = 0; i < uniformCount; i++)
    {
        gcmONERROR(gcSHADER_GetUniform(
                        shader,
                        i, &uniform));

        if (!uniform || (uniform && isUniformCompiletimeInitialized(uniform)))
            continue;

        Kernel->numArgs++;
    }

    /* Allocate the array of arguments. */
    bytes = Kernel->numArgs * gcmSIZEOF(vx_argument_s);
    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
    gcoOS_ZeroMemory(pointer, bytes);

    Kernel->args = argument = pointer;
    for (i = 0; i < uniformCount; i++)
    {
        gcUNIFORM uniform;
        gcSHADER_TYPE type;
        gcSL_FORMAT format;
        gctBOOL isPointer;
        gctUINT length;
        gctUINT32 bytes;

        gcmONERROR(gcSHADER_GetUniform(
                        shader,
                        i, &uniform));

        if (!uniform || (uniform && isUniformCompiletimeInitialized(uniform))) continue;

        gcmONERROR(gcUNIFORM_GetType(uniform, &type, &length));
        gcmONERROR(gcUNIFORM_GetFormat(uniform, &format, &isPointer));

        if (isUniformLocalAddressSpace(uniform) ||
            isUniformPrivateAddressSpace(uniform) ||
            isUniformConstantAddressSpace(uniform) ||
            isUniformKernelArgPrivate(uniform) ||
            isUniformKernelArgLocal(uniform) ||
            isUniformKernelArgLocalMemSize(uniform) ||
            isUniformPrintfAddress(uniform))
        {
            vx_mem_alloc_info memAllocInfo;
            gctPOINTER pointer;

            bytes = sizeof(vx_mem_alloc_info_s);

            /* Allocate the memory allocation info. */
            gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
            gcoOS_ZeroMemory(pointer, bytes);
            memAllocInfo = (vx_mem_alloc_info)pointer;

            /* Get the required memory size.
             * For local/private kernel arguments it will be set by the application.
             */
            if (isUniformLocalAddressSpace(uniform))
            {
                gcmONERROR(gcSHADER_GetLocalMemorySize(shader, &memAllocInfo->allocatedSize));
                Kernel->localMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformPrivateAddressSpace(uniform))
            {
                gcmONERROR(gcSHADER_GetPrivateMemorySize(shader, &memAllocInfo->allocatedSize));
                Kernel->privateMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformConstantAddressSpace(uniform))
            {
                gcmONERROR(gcSHADER_GetConstantMemorySize(shader, &memAllocInfo->allocatedSize, &Kernel->constantMemBuffer ));
                Kernel->constantMemSize += memAllocInfo->allocatedSize;
            }

            argument->data       = memAllocInfo;
            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvTRUE;
            argument->isPointer  = gcvFALSE;
        }
        else
        {
            if (isPointer)
            {
                if ((type == gcSHADER_IMAGE_2D) || (type == gcSHADER_IMAGE_2D_ARRAY))
                {
                    bytes = gcmSIZEOF(gctPOINTER);
                }
                else
                {
                    bytes = gcmSIZEOF(gctUINT32);
                }
            }
            else
            {
                switch (type)
                {
                case gcSHADER_FLOAT_X1:
                case gcSHADER_BOOLEAN_X1:
                case gcSHADER_INTEGER_X1:
                case gcSHADER_UINT_X1:
                    bytes = 1 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_FLOAT_X2:
                case gcSHADER_BOOLEAN_X2:
                case gcSHADER_INTEGER_X2:
                case gcSHADER_UINT_X2:
                    bytes = 2 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_FLOAT_X3:
                case gcSHADER_BOOLEAN_X3:
                case gcSHADER_INTEGER_X3:
                case gcSHADER_UINT_X3:
                    bytes = 3 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_FLOAT_X4:
                case gcSHADER_BOOLEAN_X4:
                case gcSHADER_INTEGER_X4:
                case gcSHADER_UINT_X4:
                    bytes = 4 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_UINT_X16:
                    bytes = 16 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_INT64_X1:
                case gcSHADER_UINT64_X1:
                    bytes = 1 * gcmSIZEOF(vx_int64) * length;
                    break;
                case gcSHADER_INT64_X2:
                case gcSHADER_UINT64_X2:
                    bytes = 2 * gcmSIZEOF(vx_int64) * length;
                    break;
                case gcSHADER_INT64_X3:
                case gcSHADER_UINT64_X3:
                    bytes = 3 * gcmSIZEOF(vx_int64) * length;
                    break;
                case gcSHADER_INT64_X4:
                case gcSHADER_UINT64_X4:
                    bytes = 4 * gcmSIZEOF(vx_int64) * length;
                    break;

                case gcSHADER_FLOAT_2X2:
                    bytes = 2 * 2 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_FLOAT_3X3:
                    bytes = 3 * 3 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_FLOAT_4X4:
                    bytes = 4 * 4 * gcmSIZEOF(vx_float32) * length;
                    break;

                case gcSHADER_IMAGE_2D:
                case gcSHADER_IMAGE_3D:
                case gcSHADER_SAMPLER:
                case gcSHADER_IMAGE_1D:
                case gcSHADER_IMAGE_1D_ARRAY:
                case gcSHADER_IMAGE_1D_BUFFER:
                case gcSHADER_IMAGE_2D_ARRAY:
                    bytes = 1 * gcmSIZEOF(vx_uint32) * length;
                    break;

                case gcSHADER_SAMPLER_2D:
                case gcSHADER_SAMPLER_3D:
                    bytes = 1 * gcmSIZEOF(vx_float32) * length;
                    break;

                default:
                    gcmASSERT("Unknown shader type");
                    bytes = 0;
                    status = gcvSTATUS_INVALID_ARGUMENT;
                    goto OnError;
                }

                switch (format)
                {
                case gcSL_INT8:
                case gcSL_UINT8:
                    bytes /= 4;
                    break;

                case gcSL_INT16:
                case gcSL_UINT16:
                    bytes /= 2;
                    break;

                default:
                    break;
                }
            }

            /* Allocate the data array. */
            gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data));

            gcoOS_ZeroMemory(argument->data, bytes);

            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvFALSE;
            argument->isPointer  = isPointer;
        }

        argument++;
    }

OnError:
    return status;
}

gceSTATUS
gcfVX_ExecuteKernel(
    vx_shader    Kernel,
    gctUINT             NumArgs,
    vx_argument         Args,
    vx_border_mode_t*   BorderMode,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkScale[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3]
)
{
    gctUINT i;
    gceSTATUS status;
    /* Load kernel states. */
    gcmONERROR(gcoVX_LoadKernelShader(Kernel->states.stateBufferSize,
                                Kernel->states.stateBuffer,
                                Kernel->states.hints));


    /* Load argument values. */
    for (i = 0; i < NumArgs; i++)
    {
        if (Args[i].uniform && !isUniformInactive(Args[i].uniform))
        {
            gcmONERROR(gcfVX_LoadKernelArgValues(Kernel,
                                                (gcSHADER) Kernel->states.binary,
                                                &Args[i],
                                                BorderMode,
                                                WorkDim,
                                                GlobalWorkOffset,
                                                GlobalWorkSize,
                                                LocalWorkSize));
        }
    }

    /* Set up local memory for kernel arguments. */
    gcmONERROR(gcfVX_LoadKernelArgLocalMemValues(Kernel,
                                              NumArgs,
                                              Args,
                                              WorkDim,
                                              GlobalWorkOffset,
                                              GlobalWorkSize,
                                              LocalWorkSize));


    gcmONERROR(gcoVX_InvokeKernelShader((gcSHADER) Kernel->states.binary,
                                  WorkDim,
                                  GlobalWorkOffset,
                                  GlobalWorkScale,
                                  GlobalWorkSize,
                                  LocalWorkSize,
                                  Kernel->states.hints->valueOrder));


    gcmONERROR(gcoVX_Flush(gcvFALSE));

    status = gcvSTATUS_OK;

OnError:
    return status;
}




gceSTATUS
gcfVX_CreateShader(vx_program program, vx_char name[VX_MAX_KERNEL_NAME], gctBOOL constBorder, vx_shader *kernelShader)
{
    gcSHADER        pgmBinary, kernelBinary;
    gctUINT         binarySize;
    gctUINT         i;
    gctUINT         count, propertySize = 0;
    gctINT          propertyType = 0;
    gctSIZE_T       propertyValues[3] = {0};
    gceSHADER_FLAGS flags;
    gctPOINTER      pointer     = gcvNULL;
    gceSTATUS       status;
    gctPOINTER      buffer      = gcvNULL;
    gctUINT         bufferSize  = 0;
    gcsHINT_PTR     hints       = gcvNULL;
    gctSIZE_T       strLen = 0;

    gcKERNEL_FUNCTION   kernelFunction;
    vx_shader           kernel = gcvNULL;
    gctUINT             maxComputeUnits, threadCount, maxDeviceWorkGroupSize;

    /* Allocate kernel. */
    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_shader_s), (gctPOINTER*)&kernel));
    gcoOS_ZeroMemory(kernel, sizeof(vx_shader_s));

    /* Save program binary into buffer */
    pgmBinary = (gcSHADER) program->binary;
    gcmONERROR(gcSHADER_SaveEx(pgmBinary, gcvNULL, &binarySize));
    gcmONERROR(gcoOS_Allocate(gcvNULL, binarySize, &pointer));
    gcmONERROR(gcSHADER_SaveEx(pgmBinary, pointer, &binarySize));

    /* Construct kernel binary. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &kernelBinary));

    kernel->states.binary          = (gctUINT8_PTR) kernelBinary;

    /* Load kernel binary from program binary */
    gcmONERROR(gcSHADER_LoadEx(kernelBinary, pointer, binarySize));

    gcoOS_Free(gcvNULL, pointer);
    pointer = gcvNULL;

    /* Load kernel binary uniforms with the given kernel name */
    gcmONERROR(gcSHADER_LoadKernel(kernelBinary, name));

    /* Set the required work group size. */
    gcmONERROR(gcSHADER_GetKernelFunctionByName(kernelBinary, name, &kernelFunction));
    gcKERNEL_FUNCTION_GetPropertyCount(kernelFunction, &count);

    for (i = 0; i < count; i++)
    {
        gcKERNEL_FUNCTION_GetProperty(kernelFunction, i, &propertySize, &propertyType, (gctINT *)propertyValues);

        if (propertyType == gcvPROPERTY_REQD_WORK_GRP_SIZE)
        {
            gcoOS_MemCopy(kernel->compileWorkGroupSize,
                propertyValues,
                gcmSIZEOF(gctINT) * propertySize);
        }
    }

    /* Assume all dead code is removed by optimizer. */
    flags = gcvSHADER_RESOURCE_USAGE /*| gcvSHADER_DEAD_CODE*/ | gcvSHADER_OPTIMIZER;

    gcSetCLCompiler(program->base.context->compileKernel);

    if (constBorder)
    {
        gcOPT_SetFeature(FB_ENABLE_CONST_BORDER);
    }

    gcmONERROR(gcLinkKernel(kernelBinary,
                          flags | gcvSHADER_REMOVE_UNUSED_UNIFORMS,
                          &bufferSize,
                          &buffer,
                          &hints));

    kernel->states.stateBuffer     = buffer;
    kernel->states.stateBufferSize = bufferSize;
    kernel->states.hints           = hints;

    if (constBorder)
    {
        gcOPT_ResetFeature(FB_ENABLE_CONST_BORDER);
    }

    /* Copy kernel name */
    strLen = gcoOS_StrLen(name, gcvNULL) + 1;
    gcmONERROR(gcoOS_Allocate(gcvNULL, strLen, &pointer));
    gcoOS_StrCopySafe((gctSTRING)pointer, strLen, name);
    kernel->name                   = (gctSTRING) pointer;

    pointer = gcvNULL;




    gcmONERROR(
        gcoHAL_QueryShaderCaps(gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    &maxComputeUnits,
                                    &threadCount,
                                    gcvNULL,
                                    gcvNULL));

    kernel->maxWorkItemSizes[0]   = gcmMIN(threadCount, 1024);
    kernel->maxWorkItemSizes[1]   = gcmMIN(threadCount, 1024);
    kernel->maxWorkItemSizes[2]   = gcmMIN(threadCount, 1024);

    kernel->maxGlobalWorkSize     = (gctUINT64) 4*1024*1024*1024;

    maxDeviceWorkGroupSize        = gcmMIN(threadCount, 1024);


    kernel->preferredWorkGroupSizeMultiple = 4 * maxComputeUnits;

    if (hints->threadWalkerInPS)
    {
        kernel->maxWorkGroupSize = (gctUINT32)(113 / gcmMAX(2, kernel->states.hints->fsMaxTemp)) *
                                   4 * maxComputeUnits;
    }
    else
    {
        kernel->maxWorkGroupSize = (gctUINT32)(113 / gcmMAX(2, kernel->states.hints->vsMaxTemp)) *
                                   4 * maxComputeUnits;
    }

    /* maxWorkGroupSize should not over the device's maxWorkGroupSize. */
    if (kernel->maxWorkGroupSize > maxDeviceWorkGroupSize)
    {
        kernel->maxWorkGroupSize = maxDeviceWorkGroupSize;
    }

    gcmVERIFY_OK(gcSHADER_GetAttributeCount(kernelBinary, &kernel->attributeCount));

    /* Allocate kernel arguments. */
    gcmONERROR(gcfVX_AllocateKernelArgs(kernel));

    *kernelShader = kernel;

    status =  gcvSTATUS_OK;

    return status;
OnError:
    if (kernel)
    {
        vxoShader_Free(kernel);
    }

    if(pointer != gcvNULL) gcmOS_SAFE_FREE(gcvNULL, pointer);

    return status;
}

VX_PRIVATE_API vx_string _getShaderName(vx_string orignal, vx_string name)
{
    vx_char* pointer = strrchr(orignal, '.');

    if(pointer)
    {
        gctSTRING suffix = strchr(pointer, ':');
        pointer = pointer + 1;
        if(suffix)
            gcoOS_StrCopySafe(name, suffix - pointer + 1, pointer);
        else
            gcoOS_StrCopySafe(name, strlen(pointer) + 1, pointer);
    }
    else
        gcoOS_StrCopySafe(name, strlen(orignal)+1, orignal);

    return name;
}



gceSTATUS gcfVX_SplitWork(
    vx_uint32 deviceCount,
    vx_kernel_execution_parameters_t * srcParameter,
    vx_kernel_execution_parameters_t * eachDevicePararmeter,
    vx_uint32 *usedDeviceCount
    )
{
    gctINT32 gpuCount        = deviceCount;
    gctINT32 usedGPUCount    = gpuCount;
    gcsTHREAD_WALKER_INFO eachGPUInfo[VX_MAX_DEVICES] = {{0}}, info;
    gctINT32 groupCountX, groupCountY, groupCountZ, i;
    gctINT32 restGroupCount = 0;

    gcoOS_ZeroMemory(&info, sizeof(gcsTHREAD_WALKER_INFO));

    info.dimensions      = srcParameter->workDim;
    info.globalSizeX     = (gctUINT32)srcParameter->globalWorkSize[0];
    info.globalOffsetX   = (gctUINT32)srcParameter->globalWorkOffset[0];
    info.globalScaleX    = (gctUINT32)srcParameter->globalWorkScale[0];
    info.workGroupSizeX  = srcParameter->localWorkSize[0] ? (gctUINT32)srcParameter->localWorkSize[0] : 1;

    info.workGroupCountX = info.globalSizeX / info.workGroupSizeX;

    if ((info.globalSizeX % info.workGroupSizeX) != 0) goto OnError;

    if (srcParameter->workDim > 1)
    {
        info.globalSizeY     = (gctUINT32)srcParameter->globalWorkSize[1];
        info.globalOffsetY   = (gctUINT32)srcParameter->globalWorkOffset[1];
        info.globalScaleY    = (gctUINT32)srcParameter->globalWorkScale[1];
        info.workGroupSizeY  = srcParameter->localWorkSize[1] ? (gctUINT32)srcParameter->localWorkSize[1] : 1;

        info.workGroupCountY = info.globalSizeY / info.workGroupSizeY;

        if ((info.globalSizeY % info.workGroupSizeY) != 0) goto OnError;

    }
    if (srcParameter->workDim > 2)
    {
        info.globalSizeZ     = (gctUINT32)srcParameter->globalWorkSize[2];
        info.globalOffsetZ   = (gctUINT32)srcParameter->globalWorkOffset[2];
        info.globalScaleZ    = (gctUINT32)srcParameter->globalWorkScale[2];
        info.workGroupSizeZ  = srcParameter->localWorkSize[2] ? (gctUINT32)srcParameter->localWorkSize[2] : 1;
        info.workGroupCountZ = info.globalSizeZ / info.workGroupSizeZ;

        if ((info.globalSizeZ % info.workGroupSizeZ) != 0) goto OnError;
    }


    for (i = 0; i < usedGPUCount; i++)
    {
        eachGPUInfo[i] = info;
        eachDevicePararmeter[i] = *srcParameter;
    }

    if ((info.dimensions == 1) || ((info.dimensions == 2) && (info.workGroupCountY == 1)))
    {
        groupCountX     = info.workGroupCountX / gpuCount;
        restGroupCount  = info.workGroupCountX % gpuCount;

        if (groupCountX  == 0) usedGPUCount = restGroupCount;

        for (i = 0; i < usedGPUCount; i++)
        {
            eachGPUInfo[i].workGroupCountX = groupCountX;
            eachDevicePararmeter[i].globalWorkSize[0] = eachGPUInfo[i].workGroupCountX * info.workGroupSizeX;
        }

        for(i = 0; i < restGroupCount; i++)
        {
            eachGPUInfo[i].workGroupCountX++;
            eachDevicePararmeter[i].globalWorkSize[0] = eachGPUInfo[i].workGroupCountX * info.workGroupSizeX;
        }

        for(i = 1; i < usedGPUCount; i++)
        {

            eachGPUInfo[i].globalOffsetX = eachGPUInfo[i-1].workGroupCountX * info.workGroupSizeX + eachGPUInfo[i-1].globalOffsetX;

            eachDevicePararmeter[i].globalWorkOffset[0] = eachGPUInfo[i].globalOffsetX;
        }
    }
    else if ((info.dimensions == 2) || ((info.dimensions == 3) && (info.workGroupCountZ == 1)))
    {
        groupCountY     = info.workGroupCountY / gpuCount;
        restGroupCount  = info.workGroupCountY % gpuCount;

        if (groupCountY  == 0) usedGPUCount = restGroupCount;

        for (i = 0; i < usedGPUCount; i++)
        {
            eachGPUInfo[i].workGroupCountY = groupCountY;
            eachDevicePararmeter[i].globalWorkSize[1] = eachGPUInfo[i].workGroupCountY * info.workGroupSizeY;
        }

        for(i = 0; i < restGroupCount; i++)
        {
            eachGPUInfo[i].workGroupCountY++;
            eachDevicePararmeter[i].globalWorkSize[1] = eachGPUInfo[i].workGroupCountY * info.workGroupSizeY;
        }

        for(i = 1; i < usedGPUCount; i++)
        {
            eachGPUInfo[i].globalOffsetY = eachGPUInfo[i-1].workGroupCountY * info.workGroupSizeY + eachGPUInfo[i-1].globalOffsetY;
            eachDevicePararmeter[i].globalWorkOffset[1] = eachGPUInfo[i].globalOffsetY;
        }

    }
    else if (info.dimensions == 3)
    {
        groupCountZ     = info.workGroupCountZ / gpuCount;
        restGroupCount  = info.workGroupCountZ % gpuCount;

        if (groupCountZ  == 0) usedGPUCount = restGroupCount;

        for (i = 0; i < usedGPUCount; i++)
        {
            eachGPUInfo[i].workGroupCountZ = groupCountZ;
            eachDevicePararmeter[i].globalWorkSize[2] = eachGPUInfo[i].workGroupCountZ * info.workGroupSizeZ;
        }

        for(i = 0; i < restGroupCount; i++)
        {
            eachGPUInfo[i].workGroupCountZ++;
            eachDevicePararmeter[i].globalWorkSize[2] = eachGPUInfo[i].workGroupCountZ * info.workGroupSizeZ;
        }

        for(i = 1; i < usedGPUCount; i++)
        {
            eachGPUInfo[i].globalOffsetZ = eachGPUInfo[i-1].workGroupCountZ * info.workGroupSizeZ + eachGPUInfo[i-1].globalOffsetZ;
            eachDevicePararmeter[i].globalWorkOffset[2] = eachGPUInfo[i].globalOffsetZ;
        }
    }

    *usedDeviceCount = usedGPUCount;

    return gcvSTATUS_OK;

OnError:
    return gcvSTATUS_INVALID_ARGUMENT;
}

gceSTATUS gcfVX_LoadShaderFromLinkedBinary(gctPOINTER linkedBinary, gctUINT32 linkedBinarySize, gctCHAR *name, vx_shader *kernelShader)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER  kernelBinary = gcvNULL;
    vx_shader kernel = gcvNULL;
    gctPOINTER pointer;
    gctSIZE_T strLen;

    gcKERNEL_FUNCTION   kernelFunction;
    gctUINT             maxComputeUnits, threadCount, maxDeviceWorkGroupSize;

    gctUINT         count, propertySize = 0, i;
    gctINT          propertyType = 0;
    gctSIZE_T       propertyValues[3] = {0};


    /* Allocate kernel. */
    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(vx_shader_s), (gctPOINTER*)&kernel));
    gcoOS_ZeroMemory(kernel, sizeof(vx_shader_s));

    /* Construct kernel binary. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &kernelBinary));

    kernel->states.binary = (gctUINT8_PTR)kernelBinary;

    gcmONERROR(gcLoadCLSingleKernel(
        (gctPOINTER)linkedBinary,
        linkedBinarySize,
        kernelBinary,
        (gctUINT32*)&kernel->states.stateBufferSize,
        &kernel->states.stateBuffer,
        &kernel->states.hints));


    /* Set the required work group size. */
    gcmONERROR(gcSHADER_GetKernelFunctionByName(kernelBinary, name, &kernelFunction));
    gcKERNEL_FUNCTION_GetPropertyCount(kernelFunction, &count);

    for (i = 0; i < count; i++)
    {
        gcKERNEL_FUNCTION_GetProperty(kernelFunction, i, &propertySize, &propertyType, (gctINT *)propertyValues);

        if (propertyType == gcvPROPERTY_REQD_WORK_GRP_SIZE)
        {
            gcoOS_MemCopy(kernel->compileWorkGroupSize,
                propertyValues,
                gcmSIZEOF(gctINT) * propertySize);
        }
    }

    /* Copy kernel name */
    strLen = gcoOS_StrLen(name, gcvNULL) + 1;
    gcmONERROR(gcoOS_Allocate(gcvNULL, strLen, &pointer));
    gcoOS_StrCopySafe((gctSTRING)pointer, strLen, name);
    kernel->name                   = (gctSTRING) pointer;


    gcmONERROR(
        gcoHAL_QueryShaderCaps(gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    gcvNULL,
                                    &maxComputeUnits,
                                    &threadCount,
                                    gcvNULL,
                                    gcvNULL));

    kernel->maxWorkItemSizes[0]   = gcmMIN(threadCount, 1024);
    kernel->maxWorkItemSizes[1]   = gcmMIN(threadCount, 1024);
    kernel->maxWorkItemSizes[2]   = gcmMIN(threadCount, 1024);

    kernel->maxGlobalWorkSize     = (gctUINT64) 4*1024*1024*1024;

    maxDeviceWorkGroupSize        = gcmMIN(threadCount, 1024);


    kernel->preferredWorkGroupSizeMultiple = 4 * maxComputeUnits;

    if (kernel->states.hints->threadWalkerInPS)
    {
        kernel->maxWorkGroupSize = (gctUINT32)(113 / gcmMAX(2, kernel->states.hints->fsMaxTemp)) *
                                   4 * maxComputeUnits;
    }
    else
    {
        kernel->maxWorkGroupSize = (gctUINT32)(113 / gcmMAX(2, kernel->states.hints->vsMaxTemp)) *
                                   4 * maxComputeUnits;
    }

    /* maxWorkGroupSize should not over the device's maxWorkGroupSize. */
    if (kernel->maxWorkGroupSize > maxDeviceWorkGroupSize)
    {
        kernel->maxWorkGroupSize = maxDeviceWorkGroupSize;
    }

    gcmVERIFY_OK(gcSHADER_GetAttributeCount(kernelBinary, &kernel->attributeCount));

    /* Allocate kernel arguments. */
    gcmONERROR(gcfVX_AllocateKernelArgs(kernel));

    *kernelShader = kernel;

    return gcvSTATUS_OK;

OnError:

    vxoShader_Free(kernel);

    return status;
}

VX_INTERNAL_API vx_status vxoKernel_CreateShaders(vx_program program, vx_char *name, vx_uint32* kernelShaderCount, vx_shader ** kernelShaders)
{
    gceSTATUS status;
    gctUINT    i;
    gcKERNEL_FUNCTION kernelFunction;
    gctSTRING kernelName;
    gcSHADER  kernelBinary = (gcSHADER) program->binary;
    gctUINT32 kernelCount = 0;
    gctSIZE_T length;

    vx_char             searchName[128]= {0};
    vx_uint32           findkernelShaderCount = 0;
    vx_shader           *findKernelShaders = gcvNULL;

    if (kernelBinary == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    gcoOS_StrLen(name, &length);

    if (!program->linked)
    {
        gcmONERROR(gcSHADER_GetKernelFunctionCount(kernelBinary, &kernelCount));
        gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gctPOINTER) * kernelCount * 2, (gctPOINTER*)&findKernelShaders));
        gcoOS_ZeroMemory(findKernelShaders, gcmSIZEOF(gctPOINTER) * kernelCount * 2);

        for (i = 0; i < kernelCount; i++)
        {
            gcmONERROR(gcSHADER_GetKernelFunction(kernelBinary, i, &kernelFunction));
            gcmONERROR(gcKERNEL_FUNCTION_GetName(kernelFunction, gcvNULL, (gctCONST_STRING *)&kernelName));
            gcoOS_StrCopySafe(searchName, length+1, kernelName);

            if (gcoOS_StrCmp(name, searchName) == 0)
            {
                gcmONERROR(gcfVX_CreateShader(program, kernelName, gcvFALSE, &findKernelShaders[findkernelShaderCount*2]));
                gcmONERROR(gcfVX_CreateShader(program, kernelName, gcvTRUE, &findKernelShaders[findkernelShaderCount*2 + 1]));
                findkernelShaderCount++;
            }
        }
    }
    else
    {
        gctUINT32 subKernelCount = 0, linkedBinarySize, kernelNameLength = 0;
        gctUINT32 *linkedBinary = (gctUINT32*)program->binary;
        gctBOOL   find = gcvFALSE;

        linkedBinary += 2;
        kernelCount = *linkedBinary;
        linkedBinary++;
        gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gctPOINTER) * kernelCount * 2, (gctPOINTER*)&findKernelShaders));
        gcoOS_ZeroMemory(findKernelShaders, gcmSIZEOF(gctPOINTER) * kernelCount * 2);
        /* skip the full size */
        linkedBinary++;

        for (i = 0; i < kernelCount; i++)
        {
            /* skip the kernel name length */
            kernelNameLength = *linkedBinary;
            linkedBinary++;

            kernelName = (gctCHAR*)linkedBinary;
            linkedBinary = (gctUINT32*)((gctUINT8*)linkedBinary + kernelNameLength);
            gcoOS_StrCopySafe(searchName, length+1, kernelName);

            find = (gcoOS_StrCmp(name, searchName) == 0);

            subKernelCount = *linkedBinary;
            if (subKernelCount != 2) goto OnError;
            linkedBinary++;

            if (*linkedBinary != gcvKERNEL_BINARY_NONE) goto OnError;
            linkedBinary++;

            linkedBinarySize = *linkedBinary;
            linkedBinary++;

            if (find)
            {
                gcmONERROR(gcfVX_LoadShaderFromLinkedBinary(
                    (gctPOINTER)linkedBinary,
                    linkedBinarySize, kernelName,
                    &findKernelShaders[findkernelShaderCount*2]));
            }

            /* locate to the next kernel binary */
            linkedBinary = (gctUINT32*) ((gctUINT8*)(linkedBinary) + linkedBinarySize);

            if (*linkedBinary != gcvKERNEL_BINARY_CONST_BORDER) goto OnError;
            linkedBinary++;

            linkedBinarySize = *linkedBinary;
            linkedBinary++;

            if (find)
            {
                gcmONERROR(gcfVX_LoadShaderFromLinkedBinary(
                    (gctPOINTER)linkedBinary,
                    linkedBinarySize,
                    kernelName,
                    &findKernelShaders[findkernelShaderCount*2 + 1]));

                findkernelShaderCount++;
            }

            /* locate to the next kernel binary */
            linkedBinary = (gctUINT32*) ((gctUINT8*)(linkedBinary) + linkedBinarySize);
        }

    }

    *kernelShaderCount  = findkernelShaderCount;
    *kernelShaders      = findKernelShaders;

    return VX_SUCCESS;

OnError:

    if (findKernelShaders)
        for(i = 0; i < kernelCount * 2; i++)
        {
            if (findKernelShaders[i]) vxoShader_Free(findKernelShaders[i]);
        }

        gcoOS_Free(gcvNULL, findKernelShaders);

    return VX_FAILURE;

}


VX_INTERNAL_API vx_status vxoKernel_Initialize(
    vx_context context,
    vx_kernel kernel,
    const vx_char name[VX_MAX_KERNEL_NAME],
    vx_enum kernelEnum,
    vx_program program,
    vx_kernel_f function,
    vx_param_description_s *parameters,
    vx_uint32 paramCount,
    vx_kernel_validate_f validateFunction,
    vx_kernel_input_validate_f inputValidateFunction,
    vx_kernel_output_validate_f outputValidateFunction,
    vx_kernel_initialize_f initializeFunction,
    vx_kernel_deinitialize_f deinitializeFunction
#if gcdVX_OPTIMIZER
, vx_kernel_optimization_attribute_s optAttributes
#endif
    )
{
    vx_uint32 i;

    vxmASSERT(context);
    vxmASSERT(paramCount <= VX_MAX_PARAMETERS);

    if (kernel == VX_NULL) return VX_FAILURE;

    vxoReference_Initialize(&kernel->base, context, VX_TYPE_KERNEL, &context->base);

    vxoContext_AddObject(context, &kernel->base);

    vxoReference_Increment(&kernel->base, VX_REF_INTERNAL);

    vxStrCopySafe(kernel->name, VX_MAX_KERNEL_NAME, name);

    kernel->enumeration             = kernelEnum;

    kernel->program                 = program;

    kernel->function                = function;

    kernel->signature.paramCount    = paramCount;

    kernel->validateFunction        = validateFunction;

    kernel->inputValidateFunction   = inputValidateFunction;
    kernel->outputValidateFunction  = outputValidateFunction;
    kernel->initializeFunction      = initializeFunction;
    kernel->deinitializeFunction    = deinitializeFunction;

    kernel->attributes.borderMode.mode              = VX_BORDER_UNDEFINED;
    kernel->attributes.borderMode.constant_value.U32= 0;

#if gcdVX_OPTIMIZER
    kernel->attributes.optAttributes                = optAttributes;
#endif

    kernel->attributes.isGPUKernel                  = vx_true_e;

    if (kernel->program != VX_NULL)
    {
        vx_char shader_name[128] = {0};

        vx_status status = vxoKernel_CreateShaders(kernel->program,
                                                    _getShaderName(kernel->name, shader_name),
                                                    &kernel->kernelShaderCount,
                                                    &kernel->kernelShader);
        if (status != VX_SUCCESS)
        {
             return status;
        }

    }

    if (parameters != VX_NULL)
    {
        for (i = 0; i < paramCount; i++)
        {
            kernel->signature.directionTable[i] = parameters[i].direction;
            kernel->signature.dataTypeTable[i]  = parameters[i].dataType;
            kernel->signature.stateTable[i]     = parameters[i].state;
        }

        //kernel->enabled = vx_true_e;
    }


    return VX_SUCCESS;
}

VX_INTERNAL_API void vxoKernel_Dump(vx_kernel kernel)
{
    if (kernel == VX_NULL)
    {
        vxTrace(VX_TRACE_KERNEL, "<kernel>null</kernel>\n");
    }
    else
    {
        vxoReference_Dump((vx_reference)kernel);

        vxTrace(VX_TRACE_KERNEL,
                "<kernel>\n"
                "   <address>"VX_FORMAT_HEX"</address>\n"
                "   <name>%s</name>\n"
                "   <enumeration>"VX_FORMAT_HEX"</enumeration>\n"
                "   <enabled>%s</enabled>\n"
                "</kernel>",
                kernel, kernel->name, kernel->enumeration, vxmBOOL_TO_STRING(kernel->enabled));
    }
}

VX_INTERNAL_API vx_status vxoKernel_InternalRelease(vx_kernel_ptr kernelPtr)
{
    vx_kernel kernel;

    vxmASSERT(kernelPtr);

    kernel = *kernelPtr;

    if (kernel == VX_NULL) return VX_ERROR_INVALID_REFERENCE;

    *kernelPtr = VX_NULL;

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    return vxoReference_Release((vx_reference_ptr)&kernel, VX_TYPE_KERNEL, VX_REF_INTERNAL);
}

VX_INTERNAL_API vx_status vxoKernel_ExternalRelease(vx_kernel_ptr kernelPtr)
{
    vx_kernel kernel;

    vxmASSERT(kernelPtr);

    kernel = *kernelPtr;

    if (kernel == VX_NULL) return VX_ERROR_INVALID_REFERENCE;

    *kernelPtr = VX_NULL;

    return vxoReference_Release((vx_reference_ptr)&kernel, VX_TYPE_KERNEL, VX_REF_EXTERNAL);
}


VX_INTERNAL_API vx_bool vxoKernel_IsUnique(vx_kernel kernel)
{
    vx_context context;
    vx_uint32 i, k;

    vxmASSERT(kernel);

    context = kernel->base.context;

    vxmASSERT(context);

    for (i = 0u; i < context->targetCount; i++)
    {
        for (k = 0u; k < context->targetTable[i].kernelCount; k++)
        {
            if (context->targetTable[i].kernelTable[k].enabled
                && context->targetTable[i].kernelTable[k].enumeration == kernel->enumeration)
            {
                return vx_false_e;
            }
        }
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_size gcOOS_StrNIndex(const vx_char *str, vx_char c, vx_size limit)
{
    vx_size index = 0;
    while (index < limit && *str != c)
    {
        if(!*str)
        {
            index = limit;
            break;
        }
        str++;
        index++;
    }
    return index;
}


VX_PRIVATE_API vx_status vxoKernel_Remove(vx_kernel kernel)
{
    vx_status status = VX_SUCCESS;

    if (kernel == NULL
        || !vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)
        || !kernel->isUserkernel)
        return VX_ERROR_INVALID_PARAMETERS;
    if (kernel->base.context->targetCount > 0)
    {
        vx_uint32 t = 0, k = 0;
        vx_target target = VX_NULL;

        vx_char targetName[VX_MAX_TARGET_NAME];
        vx_uint32 kernelIdx = 0u;
        vx_context context = kernel->base.context;

        /* find back references to kernel's target and kernel in target->kernels array */
        vx_uint32 index = (vx_uint32)(gcOOS_StrNIndex(kernel->name, ':', VX_MAX_TARGET_NAME));
        if (index == VX_MAX_TARGET_NAME)
        {
            strcpy(targetName,"vivante.any");
        }
        else
        {
            strncpy(targetName, kernel->name, index);
        }

        for (t = 0u; t < kernel->base.context->targetCount; t++)
        {
            target = &context->targetTable[t];
            if (strncmp(targetName,target->name, VX_MAX_TARGET_NAME) == 0)
            {
                break;
            }
            target = NULL;
        }

        if (target)
        {
            for (k = 0u; k < VX_MAX_KERNEL_COUNT; k++)
            {
                if (kernel == &(target->kernelTable[k]))
                {
                    kernelIdx = k;
                    break;
                }
            }
        }

        if (target && kernelIdx < VX_MAX_KERNEL_COUNT)
        {

            if (kernel->enabled)
            {
                kernel->enabled = vx_false_e;
                context->kernelCount --;

                if (vxoKernel_IsUnique(kernel))
                    context->uniqueKernelCount--;
            }

            target->kernelCount--;

            kernel->isUserkernel = vx_false_e;

            status = vxoReference_Decrement(&kernel->base, VX_REF_EXTERNAL);

            status = vxoReference_Release((vx_reference*)&kernel, VX_TYPE_KERNEL, VX_REF_INTERNAL);

            if (status == VX_SUCCESS)
            {
                target->kernelTable[kernelIdx].enumeration = VX_KERNEL_INVALID;
                target->kernelTable[kernelIdx].isUserkernel = vx_false_e;
            }
            else
            {
                vxError("Can't deinitialize kernel properly\n");
            }
        }
        else
        {
            vxError("Can't locate kernel in its context\n");
        }
    }

    return status;
}

VX_INTERNAL_API vx_kernel vxoKernel_GetByEnumFromTarget(vx_context context, vx_target target, vx_uint32 targetIndex, vx_enum kernelEnum)
{
    vx_uint32 kernelIndex;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (kernelEnum < VX_KERNEL_INVALID)
    {
        return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    if (target == VX_NULL || !target->enabled) return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);

    for (kernelIndex = 0; kernelIndex < target->kernelCount; kernelIndex++)
    {
        if (target->kernelTable[kernelIndex].enumeration == kernelEnum)
        {
            vx_kernel kernel = &target->kernelTable[kernelIndex];

            if (!kernel->enabled) continue;

            kernel->targetIndex = targetIndex;

            vxoReference_Increment(&kernel->base, VX_REF_EXTERNAL);

            vxoKernel_Dump(kernel);

            return kernel;
        }
    }
    vxError("Kernel enum %d does not exist", kernelEnum);

    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_INTERNAL_API vx_kernel vxoKernel_GetByEnum(vx_context context, vx_enum kernelEnum)
{
    vx_uint32 index, targetIndex;
    vx_kernel kernel = VX_NULL;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (kernelEnum < VX_KERNEL_INVALID)
    {
        return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    for (index = 0; index < context->targetCount; index++)
    {
        vx_target target;
        targetIndex = context->targetPriorityTable[index];
        target = &context->targetTable[targetIndex];

        if (target == VX_NULL || !target->enabled) continue;

        kernel = vxoKernel_GetByEnumFromTarget(context, target, targetIndex, kernelEnum);

        if (kernel == VX_NULL) continue;

                return kernel;
    }

    vxError("Kernel enum %d does not exist", kernelEnum);

    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_PRIVATE_API vx_size vxString_GetCharIndex(vx_const_string string, vx_char ch, vx_size limit)
{
    vx_size index;

    vxmASSERT(string);

    for (index = 0; index < limit; index++)
    {
        if (string[index] == '\0') return limit;

        if (string[index] == ch) break;
    }

    return index;
}

VX_PRIVATE_API vx_size vxString_GetCharCount(vx_const_string string, vx_size size, vx_char ch)
{
    vx_size index;
    vx_size count = 0;

    for (index = 0; index < size; index++)
    {
        if (string[index] == '\0') break;

        if (string[index] == ch) count++;
    }

    return count;
}

VX_API_ENTRY vx_status VX_API_CALL vxLoadKernels(vx_context context, const vx_char *module)
{
    return vxContext_LoadKernels(context, (vx_string)module);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxGetKernelByName(vx_context context, const vx_char string[VX_MAX_KERNEL_NAME])
{
    vx_char     tempString[VX_MAX_KERNEL_NAME];
    vx_uint32   targetIndex;

    vx_char     targetName[VX_MAX_TARGET_NAME] = "default";
    vx_char     kernelName[VX_MAX_KERNEL_NAME];

#if defined(OPENVX_USE_VARIANTS)
    vx_char     variantName[VX_MAX_VARIANT_NAME] = "default";

#if defined(OPENVX_USE_TARGET)
    vx_char     defaultTargets[][VX_MAX_TARGET_NAME] = {
        "default",
        "power",
        "performance",
        "memory",
        "bandwidth",
    };
#endif

#endif

    if (!vxoContext_IsValid(context)) return VX_NULL;

    vxStrCopySafe(tempString, VX_MAX_KERNEL_NAME, string);

    switch (vxString_GetCharCount(string, VX_MAX_KERNEL_NAME, ':'))
    {
        case 0:
            vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, string);
            break;

        case 1:
            {
#if defined(OPENVX_USE_TARGET) || defined(OPENVX_USE_VARIANTS)

                vx_string   firstName   = strtok(tempString, ":");
                vx_string   lastName    = strtok(VX_NULL, ":");

#if defined(OPENVX_USE_TARGET) && defined(OPENVX_USE_VARIANTS)

                vx_bool     isTarget = vx_false_e;

                for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
                {
                    if (vxIsSameString(firstName, context->targetTable[targetIndex].name, VX_MAX_TARGET_NAME))
                    {
                        isTarget = vx_true_e;
                        break;
                    }
                }

                if (!isTarget)
                {
                    for (targetIndex = 0u; targetIndex < vxmLENGTH_OF(defaultTargets); targetIndex++)
                    {
                        if (vxIsSameString(firstName, defaultTargets[targetIndex], VX_MAX_TARGET_NAME))
                        {
                            isTarget = vx_true_e;
                            break;
                        }
                    }
                }

                if (isTarget)
                {
                    vxStrCopySafe(targetName, VX_MAX_TARGET_NAME, firstName);
                    vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, lastName);
                }
                else
                {
                    vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, firstName);
                    vxStrCopySafe(variantName, VX_MAX_VARIANT_NAME, lastName);
                }

#elif defined(OPENVX_USE_TARGET)

                vxStrCopySafe(targetName, VX_MAX_TARGET_NAME, firstName);
                vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, lastName);

#elif defined(OPENVX_USE_VARIANTS)

                vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, firstName);
                vxStrCopySafe(variantName, VX_MAX_VARIANT_NAME, lastName);

#endif /* defined(OPENVX_USE_TARGET) && defined(OPENVX_USE_VARIANTS) */

#else
                vxError("Invalid kernel name: \"%s\"", string);
                return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
#endif /* defined(OPENVX_USE_TARGET) || defined(OPENVX_USE_VARIANTS) */
            }
            break;

        case 2:
            {
#if defined(OPENVX_USE_TARGET) && defined(OPENVX_USE_VARIANTS)
                vx_string target    = strtok(tempString, ":");
                vx_string kernel    = strtok(VX_NULL, ":");
                vx_string variant   = strtok(VX_NULL, ":");

                vxStrCopySafe(targetName, VX_MAX_TARGET_NAME, target);
                vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, kernel);
                vxStrCopySafe(variantName, VX_MAX_VARIANT_NAME, variant);
#else
                vxError("Invalid kernel name: \"%s\"", string);
                return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
#endif
            }
            break;

        default:
            vxError("Invalid kernel name: \"%s\"", string);
            return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_status status;
        vx_uint32 kernelIndex;
        vx_target target = &context->targetTable[context->targetPriorityTable[targetIndex]];
        vx_kernel kernel;

        if (target == VX_NULL || !target->enabled) continue;

        status = target->funcs.iskernelsupported(target, targetName, kernelName,
#if defined(OPENVX_USE_VARIANTS)
                                                    variantName,
#endif
                                                    &kernelIndex);

        if (status != VX_SUCCESS) continue;

        kernel = &target->kernelTable[kernelIndex];

        if (!kernel->enabled) continue;

        kernel->targetIndex = context->targetPriorityTable[targetIndex];

        vxoReference_Increment(&kernel->base, VX_REF_EXTERNAL);

        vxoKernel_Dump(kernel);

        return kernel;
    }

    vxError("Kernel \"%s\" does not exist", string);

    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxGetKernelByEnum(vx_context context, vx_enum kernel_enum)
{
   return vxoKernel_GetByEnum(context, kernel_enum);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseKernel(vx_kernel *kernel)
{
    return vxoKernel_ExternalRelease(kernel);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxoKernel_Add(
        vx_context context, const vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_kernel_f func_ptr, vx_uint32 num_params, vx_kernel_validate_f validate, vx_kernel_input_validate_f input,
        vx_kernel_output_validate_f output, vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{
    vx_size     colonCharIndex;
    vx_uint32   targetIndex;
    vx_char     targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;
    vx_kernel   kernel = VX_NULL;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (name == VX_NULL || strlen(name) == 0) goto ErrorExit;

    if (func_ptr == VX_NULL) goto ErrorExit;

    if (num_params == 0 || num_params > VX_MAX_PARAMETERS) goto ErrorExit;

    /* The initialize and de-initialize function can be null */
    if ((validate == NULL) && (input == NULL || output == NULL)) goto ErrorExit;

    colonCharIndex = vxString_GetCharIndex(name, ':', VX_MAX_TARGET_NAME);

    colonCharIndex == VX_MAX_TARGET_NAME ? strcpy(targetName,"vivante.any") : strncpy(targetName, name, colonCharIndex);

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
        {
            vxmASSERT(target->funcs.addkernel);

            kernel = target->funcs.addkernel(target, name, enumeration,
                                             VX_NULL, func_ptr, num_params, validate,
                                             input, output, initialize, deinitialize);

            kernel->attributes.isGPUKernel = vx_false_e;
            kernel->isUserkernel = vx_true_e;

            vxoReference_Increment(&kernel->base, VX_REF_EXTERNAL);

            return kernel;
        }
    }

    vxError("Faild to find target \"%s\" for vxAddKernel", targetName);

ErrorExit:
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddKernel(
        vx_context context, const vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_kernel_f func_ptr, vx_uint32 num_params, vx_kernel_input_validate_f input,
        vx_kernel_output_validate_f output, vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{
    return vxoKernel_Add(context, name, enumeration, func_ptr, num_params, VX_NULL, input, output, initialize, deinitialize);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddUserKernel(vx_context context,
                             const vx_char name[VX_MAX_KERNEL_NAME],
                             vx_enum enumeration,
                             vx_kernel_f func_ptr,
                             vx_uint32 numParams,
                             vx_kernel_validate_f validate,
                             vx_kernel_initialize_f initialize,
                             vx_kernel_deinitialize_f deinitialize)
{
    return vxoKernel_Add(context, name, enumeration, func_ptr, numParams, validate,
                         VX_NULL, VX_NULL, initialize, deinitialize);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddTilingKernel(
        vx_context context, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_tiling_kernel_f flexible_func_ptr, vx_tiling_kernel_f fast_func_ptr,
        vx_uint32 num_params, vx_kernel_input_validate_f input, vx_kernel_output_validate_f output)
{
    vx_size     colonCharIndex;
    vx_uint32   targetIndex;
    vx_char     targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (name == VX_NULL || strlen(name) == 0) goto ErrorExit;

    if (flexible_func_ptr == VX_NULL && fast_func_ptr == VX_NULL) goto ErrorExit;

    if (num_params == 0 || num_params > VX_MAX_PARAMETERS) goto ErrorExit;

    /* The initialize and de-initialize function can be null */
    if (input == VX_NULL || output == VX_NULL) goto ErrorExit;

    colonCharIndex = vxString_GetCharIndex(name, ':', VX_MAX_TARGET_NAME);

    if (colonCharIndex != VX_MAX_TARGET_NAME) vxStrCopySafe(targetName, colonCharIndex, name);

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
        {
            if (target && target->funcs.addtilingkernel != VX_NULL)
            {
                return target->funcs.addtilingkernel(target, name, enumeration,
                                                    flexible_func_ptr, fast_func_ptr,
                                                    num_params, input, output);
            }
        }
    }

    vxError("Faild to find target \"%s\" for vxAddTilingKernel", targetName);

ErrorExit:
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_status VX_API_CALL vxRemoveKernel(vx_kernel kernel)
{
    return vxoKernel_Remove(kernel);
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToKernel(
        vx_kernel kernel, vx_uint32 index, vx_enum dir, vx_enum dataType, vx_enum state)
{
    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    if (index >= kernel->signature.paramCount) return VX_ERROR_INVALID_PARAMETERS;

    if (kernel->tilingFunction == VX_NULL)
    {
        if (!vxDataType_IsValid(dataType)) return VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        if (dataType != VX_TYPE_IMAGE && dataType != VX_TYPE_SCALAR) return VX_ERROR_INVALID_PARAMETERS;
    }

    if (!vxmIS_VALID_DIRECTION_FOR_USER_KERNEL(dir) || !vxmIS_VALID_STATE(state) || !vxDataType_IsValid(dataType) || (dataType == VX_TYPE_DELAY && dir != VX_INPUT))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    kernel->signature.directionTable[index] = dir;
    kernel->signature.dataTypeTable[index]  = dataType;
    kernel->signature.stateTable[index]     = state;

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxFinalizeKernel(vx_kernel kernel)
{
    vx_uint32 i;

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    for (i = 0; i < kernel->signature.paramCount; i++)
    {
        if (i >= kernel->signature.paramCount)
            break;
        if (!vxmIS_VALID_DIRECTION(kernel->signature.directionTable[i])
            || !vxDataType_IsValid(kernel->signature.dataTypeTable[i]))
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if (i == kernel->signature.paramCount)
    {

    if (vxoKernel_IsUnique(kernel))
    {
        kernel->base.context->uniqueKernelCount++;
    }

    kernel->enabled = vx_true_e;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryKernel(vx_kernel kernel, vx_enum attribute, void *ptr, vx_size size)
{
    vx_char name[VX_MAX_KERNEL_NAME];
    vx_char *namePtr;

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_KERNEL_PARAMETERS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = kernel->signature.paramCount;
            break;

        case VX_KERNEL_NAME:
            if (ptr == NULL || size > VX_MAX_KERNEL_NAME) return VX_ERROR_INVALID_PARAMETERS;

            vxStrCopySafe(name, VX_MAX_KERNEL_NAME, kernel->name);

            namePtr = strtok(name, ":");

            vxStrCopySafe((vx_string)ptr, VX_MAX_KERNEL_NAME, namePtr);
            break;

        case VX_KERNEL_ENUM:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = kernel->enumeration;
            break;

        case VX_KERNEL_LOCAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = kernel->attributes.localDataSize;
            break;

#ifdef OPENVX_KHR_TILING
        case VX_KERNEL_INPUT_NEIGHBORHOOD:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_neighborhood_size_t, 0x3);

            *(vx_neighborhood_size_t *)ptr = kernel->attributes.inputNeighborhoodSize;
            break;

        case VX_KERNEL_OUTPUT_TILE_BLOCK_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_tile_block_size_t, 0x3);

            *(vx_tile_block_size_t *)ptr = kernel->attributes.tileBlockSize;
            break;
#endif

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetKernelAttribute(vx_kernel kernel, vx_enum attribute, const void *ptr, vx_size size)
{
#ifdef OPENVX_KHR_TILING
    vx_border_t *borderMode;
#endif

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    if (kernel->enabled) return VX_ERROR_NOT_SUPPORTED;

    switch (attribute)
    {
        case VX_KERNEL_LOCAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            kernel->attributes.localDataSize = *(vx_size *)ptr;
            break;


#ifdef OPENVX_KHR_TILING
        case VX_KERNEL_INPUT_NEIGHBORHOOD:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_neighborhood_size_t, 0x3);

            kernel->attributes.inputNeighborhoodSize = *(vx_neighborhood_size_t *)ptr;
            break;

        case VX_KERNEL_OUTPUT_TILE_BLOCK_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_tile_block_size_t, 0x3);

            kernel->attributes.tileBlockSize = *(vx_tile_block_size_t *)ptr;
            break;

        case VX_KERNEL_BORDER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_t, 0x3);

            borderMode = (vx_border_t *)ptr;

            switch (borderMode->mode)
            {
                case VX_BORDER_SELF:
                case VX_BORDER_UNDEFINED:
                    break;

                default:
                    vxError("Unsupported border mode: %d", borderMode->mode);
                    return VX_ERROR_INVALID_VALUE;
            }

            kernel->attributes.borderMode = *borderMode;
            break;
#endif

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

#if gcdDUMP
VX_INTERNAL_API vx_status vxoDumpOutput(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_uint32 i = 0;

    for(i = 0; i < paramCount; i++)
    {
        vx_reference ref = (vx_reference)parameters[i];
        if (node->kernel->signature.directionTable[i] != VX_INPUT
#if (gcdDUMP == 1)
            && !ref->isVirtual
#endif
            )
        {
            vx_uint8_ptr physical = 0, logical = 0;
            vx_uint32 size = 0;

            switch(ref->type)
            {
            case VX_TYPE_IMAGE:
                physical    = (vx_uint8_ptr)(((vx_image)ref)->memory.physicals[0]);
                logical     = ((vx_image)ref)->memory.logicals[0];
                size        = (vx_uint32)(((vx_image)ref)->memory.strides[0][2] * ((vx_image)ref)->height);
                break;
            case VX_TYPE_DISTRIBUTION:
                physical    = (vx_uint8_ptr)(((vx_image)ref)->memory.physicals[0]);
                logical     = (vx_uint8_ptr)((vx_distribution)ref)->memAllocInfo.logical;
                size        = ((vx_distribution)ref)->memAllocInfo.allocatedSize;
                break;
            case VX_TYPE_LUT:
            case VX_TYPE_ARRAY:
                physical    = (vx_uint8_ptr)(((vx_image)ref)->memory.physicals[0]);
                logical     = ((vx_array)ref)->memory.logicals[0];
                size        = ((vx_array)ref)->memAllocInfo.allocatedSize;
                break;
            case VX_TYPE_SCALAR:
                physical    = (vx_uint8_ptr)(((vx_scalar)ref)->physical);
                size        = ((vx_scalar)ref)->value->s;

                logical     = (vx_uint8_ptr)((vx_scalar)ref)->value;
                break;
            default:
                gcmPRINT("Unkown type(%d)!", ref->type);
                break;
            }

            /* Dump memory */
            gcmDUMP_BUFFER(gcvNULL,
                        "verify",
                        (gctUINT32)physical,
                        (gctPOINTER)logical,
                        0,
                        size);
        }
    }

    return VX_SUCCESS;
}
#endif


VX_INTERNAL_API vx_status vxoShader_SetParameters(vx_shader kernelShader, vx_reference parameters[], vx_uint32 paramCount, vx_enum dataTypeTable[])
{
    gceSTATUS status;
    gctUINT   argIndex = 0;
    vx_uint32 i;

    for (i = 0; i < paramCount; i++)
    {
        vx_type_e type = (vx_type_e)((parameters[i] != gcvNULL) ? parameters[i]->type : dataTypeTable[i]);

        switch(type)
        {
        case VX_TYPE_SCALAR:
        {
            gctBOOL isPointer;
            vx_scalar scalar = (vx_scalar)parameters[i];
            vx_argument argument = gcfVX_GetKernelArg(kernelShader, argIndex, gcvNULL, gcvNULL, gcvNULL);

            if (scalar)
            {
                gcmONERROR(gcUNIFORM_GetFormat(argument->uniform, gcvNULL, &isPointer));

                if (!isPointer)
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),/*vxDataType_GetSize((vx_type_e)scalar->dataType),*/
                                scalar->value));
                }
                else
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &scalar->physical));
                }

                argIndex++;
            }
            else
            {
                argIndex++;
            }
            break;
        }
        case VX_TYPE_LUT:
        case VX_TYPE_ARRAY:
        {
            vx_array array = (vx_array)parameters[i];

            if (array)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &array->capacity));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &array->memory.physicals[0]));
                argIndex ++;
            }
            else
            {
                argIndex += 2;
            }
            break;
        }
        case VX_TYPE_CONVOLUTION:
        {
            vx_convolution conv = (vx_convolution)parameters[i];
            if (conv)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &conv->matrix.columns));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &conv->matrix.rows));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &conv->matrix.memory.physicals[0]));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &conv->scale));
                argIndex ++;
            }
            else
            {
                argIndex += 4;
            }
            break;
        }
        case VX_TYPE_MATRIX:
        {
            vx_matrix matrix = (vx_matrix)parameters[i];
            vxReadMatrix(matrix, NULL);

            if (matrix)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &matrix->columns));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &matrix->rows));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &matrix->memory.physicals[0]));
                argIndex ++;
            }
            else
            {
                argIndex += 3;
            }
            break;
        }
        case VX_TYPE_THRESHOLD:
        {
            vx_threshold threshold = (vx_threshold)parameters[i];
            if (threshold)
            {
                gctUINT32  value = FV(threshold->value + 1);
                gctUINT32  lower = (gctINT32)FV(threshold->lower);
                gctUINT32  upper = (gctINT32)FV(threshold->upper);


                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &threshold->dataType));
                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &value));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &lower));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &upper));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &threshold->trueValue));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &threshold->falseValue));

                argIndex ++;
            }
            else
            {
                argIndex += 6;
            }

            break;
        }
        case VX_TYPE_DISTRIBUTION:
        {
            vx_distribution distribution = (vx_distribution)parameters[i];

            if (distribution)
            {
                gctUINT32  rang = distribution->memory.dims[0][VX_DIM_X] * distribution->windowX;
                gctFLOAT   window = 1.0f/distribution->windowX;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &distribution->memory.dims[0][VX_DIM_X]));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &rang));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &distribution->offsetX));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &window));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &distribution->memory.physicals[0]));

                argIndex ++;
            }
            else
            {
                argIndex += 5;
            }

            break;
        }
        case VX_TYPE_REMAP:
        {
            vx_remap remap = (vx_remap)parameters[i];

            if (remap)
            {

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &remap->destWidth));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &remap->destHeight));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                kernelShader,
                                argIndex,
                                sizeof(gctUINT32),
                                &remap->memory.physicals[0]));

                argIndex ++;
            }
            else
            {
                argIndex += 3;
            }

            break;

        }
        case VX_TYPE_IMAGE:
        {

            vx_image image = (vx_image)parameters[i];

            if (image && image->memory.logicals[0])
            {
                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &parameters[i]));
            }
            else
            {
                vx_reference nullRef = VX_NULL;

                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &nullRef));
            }

            argIndex ++;

            break;
        }
        case VX_TYPE_OBJECT_ARRAY:
        {

            vx_object_array objArray = (vx_object_array)parameters[i];

            if (objArray && (objArray->itemType == VX_TYPE_IMAGE))
            {
                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &parameters[i]));
            }

            argIndex ++;

            break;
        }
        case VX_TYPE_TENSOR:
        {

            vx_tensor tensor = (vx_tensor)parameters[i];

            if (tensor)
            {
                gcmONERROR(gcfVX_SetKernelArg(
                        kernelShader,
                        argIndex,
                        sizeof(vx_reference),
                        &parameters[i]));
            }

            argIndex ++;

            break;
        }
        case VX_TYPE_PYRAMID:
        {
            gctUINT32 j = 0;
            vx_uint32  maxLevel = 10;

            vx_pyramid pyramid = (vx_pyramid)parameters[i];

            if (pyramid)
            {

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &pyramid->scale));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &pyramid->width));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &pyramid->height));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &pyramid->format));

                argIndex ++;

                gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex,
                                    sizeof(gctUINT32),
                                    &pyramid->levelCount));

                argIndex ++;

                maxLevel = gcmMIN(pyramid->levelCount, maxLevel);

                for (j = 0; j < maxLevel; j++)
                {
                    gcmONERROR(gcfVX_SetKernelArg(
                                    kernelShader,
                                    argIndex + j,
                                    sizeof(vx_image),
                                    &pyramid->levels[j]));

                }

                /* default is 10, need to match with the compiler option -cl-viv-vx-image-array-maxlevel=n */
                argIndex += 10;

            }
            else
            {
                argIndex += 15;
            }

            break;
        }
        default:
            goto OnError;
        }
    }

    return VX_SUCCESS;

OnError:
    return VX_FAILURE;

}

VX_INTERNAL_API vx_status vxoShader_Execute(
    vx_shader kernelShader,
    vx_border_mode_t *borderMode,
    vx_kernel_execution_parameters_t *shaderParameter,
    gctPOINTER* devices,
    vx_uint32 deviceCount)
{
    gceSTATUS status;
    vx_size   workGroupSize = 1;
    vx_uint32           i;

    vx_kernel_execution_parameters_t newShaderParameter = *shaderParameter;

    /* adjust the localworksize while it is unset */
    gcmONERROR(gcfVX_AdjustLocalWorkSize(kernelShader,
                              newShaderParameter.workDim,
                              newShaderParameter.globalWorkOffset,
                              newShaderParameter.globalWorkSize,
                              newShaderParameter.localWorkSize));

    /* validate the arguments */
    for (i = 0; i < newShaderParameter.workDim; i++)
    {
        if(newShaderParameter.globalWorkSize[i] == 0)
        {
            goto OnError;
        }

        if (newShaderParameter.globalWorkSize[i] > kernelShader->maxGlobalWorkSize)
        {
            goto OnError;
        }

        if ((newShaderParameter.globalWorkSize[i] + newShaderParameter.globalWorkOffset[i]) > kernelShader->maxGlobalWorkSize)
        {
            goto OnError;
        }


        if((newShaderParameter.globalWorkSize[i] % newShaderParameter.localWorkSize[i]) != 0)
        {
            goto OnError;
        }

        if (newShaderParameter.localWorkSize[i] > kernelShader->maxWorkItemSizes[i])
        {
            goto OnError;
        }

        workGroupSize *= newShaderParameter.localWorkSize[i];

    }

    if (workGroupSize > kernelShader->maxWorkGroupSize) goto OnError;

    if (deviceCount > 1)
    {
        gctINT  i;
        gctUINT usedDeviceCount;
        vx_kernel_execution_parameters_t shaderParam[VX_MAX_DEVICES] = {{0}};

        gcmONERROR(gcfVX_SplitWork(deviceCount,
                        &newShaderParameter,
                        shaderParam,
                        &usedDeviceCount));


        if (usedDeviceCount > 1)
        {
            for (i = usedDeviceCount - 1; i >= 0; i--)
            {
                gcmONERROR(gcoVX_SetCurrentDevice(devices[i], i));

                gcmONERROR(gcfVX_ExecuteKernel(kernelShader,
                            kernelShader->numArgs,
                            kernelShader->args,
                            borderMode,
                            shaderParam[i].workDim,
                            shaderParam[i].globalWorkOffset,
                            shaderParam[i].globalWorkScale,
                            shaderParam[i].globalWorkSize,
                            shaderParam[i].localWorkSize));

               gcmONERROR(gcoVX_MultiDeviceSync(gcvNULL));

            }
        }
        else
        {
            gcmONERROR(gcfVX_ExecuteKernel(kernelShader,
                        kernelShader->numArgs,
                        kernelShader->args,
                        borderMode,
                        newShaderParameter.workDim,
                        newShaderParameter.globalWorkOffset,
                        newShaderParameter.globalWorkScale,
                        newShaderParameter.globalWorkSize,
                        newShaderParameter.localWorkSize));
        }
    }
    else
    {
        gcmONERROR(gcfVX_ExecuteKernel(kernelShader,
                        kernelShader->numArgs,
                        kernelShader->args,
                        borderMode,
                        newShaderParameter.workDim,
                        newShaderParameter.globalWorkOffset,
                        newShaderParameter.globalWorkScale,
                        newShaderParameter.globalWorkSize,
                        newShaderParameter.localWorkSize));
    }


    return VX_SUCCESS;

OnError:
    return VX_FAILURE;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoProgramKernel_Function(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    vx_uint32           i;
    gctUINT             shaderID;
    vx_shader           kernelShader;
    gctCHAR             kernelName[256] = {0};
    vx_char             kernelMainName[128] = {0};
    vx_status           status = VX_FAILURE;

    gcoOS_StrCopySafe(kernelName, 256, _getShaderName(node->kernel->name, kernelMainName));
    gcoOS_StrCatSafe(kernelName, 256, node->kernel->subname);

    for(i = 0; i < node->kernel->kernelShaderCount; i++)
    {
        if (gcoOS_StrCmp(node->kernel->kernelShader[i*2]->name, kernelName) == 0)
            break;
    }

    if (i == node->kernel->kernelShaderCount) goto error;

    shaderID = ((node->kernelAttributes.borderMode.mode == VX_BORDER_MODE_CONSTANT) ? 1 : 0);

    kernelShader = node->kernel->kernelShader[i*2 + shaderID];

    node->kernel->currShaderID = i*2 + shaderID;



    status = vxoShader_SetParameters(kernelShader, (vx_reference*)parameters, paramCount, node->kernel->signature.dataTypeTable);
    if (status != VX_SUCCESS) goto error;


    for(i = 0; i < node->uniformCount; i++)
    {
        status = vxoShader_SetUniform(kernelShader, node->uniforms[i].name, node->uniforms[i].count, node->uniforms[i].data);
        if (status != VX_SUCCESS) goto error;

    }

    status = vxoShader_Execute(kernelShader,
                                &node->kernelAttributes.borderMode,
                                &node->kernelAttributes.shaderParameter, node->base.context->devices,
                                node->base.context->deviceCount);
    if (status != VX_SUCCESS) goto error;

#if gcdDUMP
    gcfVX_Flush(gcvTRUE);

    vxoDumpOutput(node, parameters, paramCount);
#endif

error:
    return status;
}

VX_API_ENTRY vx_status VX_CALLBACK vxProgramKernel_Function(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    return vxoProgramKernel_Function(node, parameters, paramCount);
}

/*unused code*/
VX_PRIVATE_API vx_status VX_CALLBACK vxoProgramKernel_Initialize(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    /* TODO */

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoProgramKernel_Deinitialize(vx_node node, const vx_reference parameters[], vx_uint32 paramCount)
{
    /* TODO */

    return VX_SUCCESS;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddKernelInProgramEx(
        vx_program program, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration, vx_kernel_f func_ptr,
        vx_uint32 num_params, vx_kernel_validate_f validate, vx_kernel_input_validate_f input,
        vx_kernel_output_validate_f output, vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{
    vx_context  context;
    vx_uint32   targetIndex;
    vx_char     targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;

    if (!vxoReference_IsValidAndSpecific(&program->base, (vx_type_e)VX_TYPE_PROGRAM)) return VX_NULL;

    context = program->base.context;

    if (name == VX_NULL || strlen(name) == 0) goto ErrorExit;

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
        {
            vxmASSERT(target->funcs.addkernel);

            return target->funcs.addkernel(target, name, enumeration,
                                            program,
                                            (func_ptr != VX_NULL) ? func_ptr:vxoProgramKernel_Function,
                                            num_params, validate,
                                            input, output,
                                            (initialize != VX_NULL)?initialize:vxoProgramKernel_Initialize,
                                            (deinitialize != VX_NULL)?deinitialize:vxoProgramKernel_Deinitialize);
        }
    }

    vxError("Faild to find target \"%s\" for vxAddKernelInProgram", targetName);

ErrorExit:
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddKernelInProgram(
        vx_program program, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration, vx_uint32 num_params, vx_kernel_validate_f validate,
        vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{
    return vxAddKernelInProgramEx(program, name, enumeration, vxoProgramKernel_Function, num_params, validate, VX_NULL, VX_NULL, initialize, deinitialize);
}

VX_API_ENTRY vx_status VX_API_CALL vxSelectKernelSubname(vx_node node, const vx_char * subname)
{
    gcoOS_StrCopySafe(node->kernel->subname, VX_MAX_KERNEL_NAME, subname);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetNodeUniform(vx_node node, const vx_char * name, vx_size count, void * value)
{
    vx_uint32 size;
    vx_status vStatus = VX_FAILURE;
    gceSTATUS status;

    if (node->kernel->kernelShader[0] && (node->uniformCount >= node->kernel->kernelShader[0]->numArgs)) goto error;

    if (!node->uniforms)
    {
        /*allocat the maximum number uniforms */
        status = gcoOS_Allocate(gcvNULL, node->kernel->kernelShader[0]->numArgs * gcmSIZEOF(vx_node_s), (gctPOINTER*)&node->uniforms);
        if (gcmIS_ERROR(status))
        {
            vStatus = VX_FAILURE;
            goto error;
        }
    }


    vStatus = vxoShader_GetUniformSize(node->kernel->kernelShader[0], (vx_char*)name, &size);
    if (vStatus != VX_SUCCESS) goto error;

    status = gcoOS_Allocate(gcvNULL, size, (gctPOINTER*)&node->uniforms[node->uniformCount].data);
    if (gcmIS_ERROR(status))
    {
        vStatus = VX_FAILURE;
        goto error;
    }

    gcoOS_MemCopy(node->uniforms[node->uniformCount].data, value, size);

    node->uniforms[node->uniformCount].count = (vx_uint32)count;
    node->uniforms[node->uniformCount].name  = (vx_char*)name;
    node->uniforms[node->uniformCount].size  = size;

    node->uniformCount++;

error:
    return vStatus;
}

gceSTATUS
gcfVX_FreeKernelArgs(
    gctUINT         NumArgs,
    vx_argument     Args,
    gctBOOL         FreeAllocData
    )
{
    gctUINT         i;

    if (Args == gcvNULL || NumArgs == 0)
    {
        return gcvSTATUS_OK;
    }

    for (i = 0; i < NumArgs; i++)
    {
        if (Args[i].isMemAlloc)
        {
            vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info) Args[i].data;
            gcoVX_FreeMemoryEx(memAllocInfo->physical,
                             memAllocInfo->logical,
                             memAllocInfo->allocatedSize,
                             memAllocInfo->node);
            if (FreeAllocData && memAllocInfo->data) gcmOS_SAFE_FREE(gcvNULL, memAllocInfo->data);
        }

        if (Args[i].data)
        {
            gcmOS_SAFE_FREE(gcvNULL, Args[i].data);
        }
    }
    gcmOS_SAFE_FREE(gcvNULL, Args);

    return gcvSTATUS_OK;
}


VX_INTERNAL_API vx_status vxoShader_Free(vx_shader kernel)
{
    if (kernel)
    {
        gcfVX_FreeKernelArgs(kernel->numArgs, kernel->args, gcvTRUE);

        if (kernel->states.stateBuffer) gcoOS_Free(gcvNULL, kernel->states.stateBuffer);
        if (kernel->states.hints) gcHINTS_Destroy(kernel->states.hints);
        if (kernel->states.hints) gcoOS_Free(gcvNULL, kernel->states.hints);
        if (kernel->states.binary) gcSHADER_Destroy((gcSHADER)kernel->states.binary);
        if (kernel->name) gcoOS_Free(gcvNULL, kernel->name);

        gcoOS_Free(gcvNULL, kernel);
    }

    return gcvSTATUS_OK;
}

VX_INTERNAL_CALLBACK_API void vxoKernel_Destructor(vx_reference ref)
{
    vx_kernel vKernel = (vx_kernel)ref;

    gctUINT i;

    for (i = 0; i < vKernel->kernelShaderCount*2; i++)
    {
        vxoShader_Free(vKernel->kernelShader[i]);
    }

    if (vKernel->kernelShader) gcoOS_Free(gcvNULL, vKernel->kernelShader);
}

VX_INTERNAL_API vx_status vxoKernel_ProcessKernelShaderPrint(vx_kernel kernel, vx_kernel_execution_parameters_t* shaderParameter)
{
    if (kernel->kernelShader)
    {
        gctUINT i, j;
        char *fmt;
        vx_shader shader;

        gctUINT totalNumItems  =
            (gctUINT)(shaderParameter->globalWorkSize[0]
            * (shaderParameter->workDim > 1 ? shaderParameter->globalWorkSize[1] : 1)
            * (shaderParameter->workDim > 2 ? shaderParameter->globalWorkSize[2] : 1));


        gcmASSERT((kernel->currShaderID >= 0) && (kernel->currShaderID < (kernel->kernelShaderCount * 2)));
        shader = kernel->kernelShader[kernel->currShaderID];
        gcmASSERT (shader);

        fmt = shader->constantMemBuffer;
        for (i = 0; i < shader->numArgs; i++)
        {
            if (shader->args[i].isMemAlloc)
            {
                vx_mem_alloc_info memAllocInfo = (vx_mem_alloc_info)shader->args[i].data;
                if (strcmp(shader->args[i].uniform->name, "#printf_address") == 0)
                {
                    gctCHAR * printfAddr = (gctCHAR*)memAllocInfo->logical;
                    for(j = 0; j < totalNumItems; j++)
                    {
                        void* dataAddress = (void*)((gctUINT*)(printfAddr) + 1);
                        gcfVX_PrintParseData(fmt, dataAddress);
                        printfAddr += (VX_MAX_PRINTF_BUFFER_SIZE/totalNumItems);
                    }
                }
            }
        }
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoShader_SetUniform(vx_shader shader, vx_char * name, vx_size count, void * value)
{
    vx_argument         arg;
    gctUINT             i;
    gceSTATUS           status;
    gctUINT             length;
    vx_status           vStatus = VX_FAILURE;

    for (i = 0; i < shader->numArgs; i++)
    {
        arg = &shader->args[i];

        if (arg->uniform == gcvNULL) continue;

        if ((gcoOS_StrCmp(arg->uniform->name, name) == gcvSTATUS_OK))
        {
            gcmONERROR(gcUNIFORM_GetType(arg->uniform, gcvNULL, &length));

            if (length != count) break;

            gcoOS_MemCopy(arg->data, value, arg->size);

            vStatus = VX_SUCCESS;
            break;
        }
    }

OnError:
    return vStatus;
}


VX_API_ENTRY vx_status VX_API_CALL vxUnloadKernels(vx_context context, const vx_char *name)
{
    vx_status status = VX_SUCCESS;

    vx_char module[VX_INT_MAX_PATH];
    vx_uint32 m = 0;
    vx_unpublish_kernels_f unpublish = NULL;

    sprintf(module, VX_MODULE_NAME("%s"), (name?name:"openvx-ext"));

    if (vxoContext_IsValid(context) == vx_false_e)
    {
        vxError("Context is invalid!\n");
        return VX_ERROR_INVALID_REFERENCE;
    }

    for (m = 0; m < VX_MAX_MODULE_COUNT; m++)
    {
        if (context->moduleTable[m].handle != NULL && strncmp(name, context->moduleTable[m].name, VX_INT_MAX_PATH) == 0)
        {
            vx_symbol_handle sym = vxGetSymbol(context->moduleTable[m].handle, "vxUnpublishKernels");
            unpublish = (vx_unpublish_kernels_f)sym;
            if (unpublish == NULL)
            {
                vxError("Failed to load symbol vxUnpublishKernels\n");
                status = VX_ERROR_INVALID_MODULE;
            }
            else
            {
                vxError("Calling %s unpublish function\n", module);
                status = unpublish((vx_context)context);
                if (status != VX_SUCCESS)
                {
                    vxError("Failed to unpublish kernels in module\n");
                }
                else
                {
                    vxUnloadModule(context->moduleTable[m].handle);
                    context->moduleTable[m].handle = NULL;
                    context->moduleCount--;
                    return VX_SUCCESS;
                }
            }
        }
    }

    vxError("Failed to find module %s in libraries path\n", module);

    return status;
}

VX_INTERNAL_API vx_status vxoShader_GetUniformSize(vx_shader shader, vx_char * name, vx_uint32 *size)
{
    vx_argument         arg;
    gctUINT             i;

    for (i = 0; i < shader->numArgs; i++)
    {
        arg = &shader->args[i];

        if (arg->uniform == gcvNULL) continue;

        if ((gcoOS_StrCmp(arg->uniform->name, name) == gcvSTATUS_OK))
        {
            *size = arg->size;
            return VX_SUCCESS;
        }
    }

    *size = 0;

    return VX_FAILURE;
}

