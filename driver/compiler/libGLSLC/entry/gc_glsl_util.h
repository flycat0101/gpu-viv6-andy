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


#ifndef __gc_glsh_util_
#define __gc_glsh_util_

#define _SWIZZLE_X        gcSL_SWIZZLE_XXXX
#define _SWIZZLE_Y        gcSL_SWIZZLE_YYYY
#define _SWIZZLE_Z        gcSL_SWIZZLE_ZZZZ
#define _SWIZZLE_W        gcSL_SWIZZLE_WWWW
#define _SWIZZLE_XY        gcSL_SWIZZLE_XYYY
#define _SWIZZLE_XYZ    gcSL_SWIZZLE_XYZZ
#define _SWIZZLE_XYZW    gcSL_SWIZZLE_XYZW
#define _SWIZZLE_YZW    gcSL_SWIZZLE_YZWW
#define _SWIZZLE_ZW        gcSL_SWIZZLE_ZWWW

#define ADD_ATTRIBUTE(name,type,precision) \
    gcmONERROR(gcSHADER_AddAttribute(Shader, \
                                    #name, \
                                    gcSHADER_##type, \
                                    1, \
                                    gcvFALSE, \
                                    gcSHADER_SHADER_DEFAULT, \
                                    precision, \
                                    &name))

#define ADD_ATTRIBUTE_TX(name,type,precision) \
    gcmONERROR(gcSHADER_AddAttribute(Shader, \
                                    #name, \
                                    gcSHADER_##type, \
                                    1, \
                                    gcvTRUE, \
                                    gcSHADER_SHADER_DEFAULT, \
                                    precision, \
                                    &name))

#define ADD_ATTRIBUTE_ARRAY(name,type,length,precision) \
    gcmONERROR(gcSHADER_AddAttribute(Shader, \
                                    #name, \
                                    gcSHADER_##type, \
                                    length, \
                                    gcvFALSE, \
                                    gcSHADER_SHADER_DEFAULT, \
                                    precision, \
                                    &name))

#define ADD_ATTRIBUTE_ARRAY_TX(name,type,length,precision) \
    gcmONERROR(gcSHADER_AddAttribute(Shader, \
                                    #name, \
                                    gcSHADER_##type, \
                                    length, \
                                    gcvTRUE, \
                                    gcSHADER_SHADER_DEFAULT, \
                                    precision, \
                                    &name))

#define ADD_UNIFORM(name,type,precision) \
    gcmONERROR(gcSHADER_AddUniform(Shader, \
                                  #name, \
                                  gcSHADER_##type, \
                                  1, \
                                  precision, \
                                  &name))

#define ADD_UNIFORM_ARRAY(name,type,length,precision) \
    do \
    { \
        gctINT Length = length; \
        gcmONERROR(gcSHADER_AddUniformEx1(Shader, \
                                      #name, \
                                      gcSHADER_##type, \
                                      precision, \
                                      -1, \
                                      -1, \
                                      -1, \
                                      1, \
                                      &Length, \
                                      gcSHADER_VAR_CATEGORY_NORMAL, \
                                      0, \
                                      -1, \
                                      -1, \
                                      gcIMAGE_FORMAT_DEFAULT, \
                                      gcvNULL, \
                                      &name)); \
    }while(gcvFALSE);

#define POSITION(reg) \
    gcmONERROR(gcSHADER_AddOutputWithLocation(Shader, \
                                              "#Position", \
                                              gcSHADER_FLOAT_X4, \
                                              gcSHADER_PRECISION_HIGH, \
                                              gcvFALSE, \
                                              1, \
                                              reg, \
                                              gcSHADER_SHADER_DEFAULT, \
                                              gcSHADER_GetOutputDefaultLocation(Shader), \
                                              -1, \
                                              gcvFALSE, \
                                              gcvFALSE, \
                                              gcvNULL))

#define COLOR(reg,precision) \
    gcmONERROR(gcSHADER_AddOutputWithLocation(Shader, \
                                              "#Color", \
                                              gcSHADER_FLOAT_X4, \
                                              precision, \
                                              gcvFALSE, \
                                              1, \
                                              reg, \
                                              gcSHADER_SHADER_DEFAULT, \
                                              gcSHADER_GetOutputDefaultLocation(Shader), \
                                              -1, \
                                              gcvFALSE, \
                                              gcvFALSE, \
                                              gcvNULL))

#define ADD_OUTPUT(name,type,precision) \
    gcmONERROR(gcSHADER_AddOutputWithLocation(Shader, \
                                              #name, \
                                              gcSHADER_##type, \
                                              precision, \
                                              gcvFALSE, \
                                              1, \
                                              (gctUINT32) -1, \
                                              gcSHADER_SHADER_DEFAULT, \
                                              gcSHADER_GetOutputDefaultLocation(Shader), \
                                              -1, \
                                              gcvFALSE, \
                                              gcvFALSE, \
                                              gcvNULL))

#define ADD_OUTPUT_ARRAY(name,type,length,precision) \
    gcmONERROR(gcSHADER_AddOutputWithLocation(Shader, \
                                              #name, \
                                              gcSHADER_##type, \
                                              precision, \
                                              length > 1, \
                                              length, \
                                              (gctUINT32) -1, \
                                              gcSHADER_SHADER_DEFAULT, \
                                              gcSHADER_GetOutputDefaultLocation(Shader), \
                                              -1, \
                                              gcvFALSE, \
                                              gcvFALSE, \
                                              gcvNULL))

#define LABEL(lbl) \
    gcmONERROR(gcSHADER_AddLabel(Shader, lbl))

#define OPCODE(op,reg,enable,precision) \
    gcmONERROR(gcSHADER_AddOpcode(Shader, \
                                    gcSL_##op, \
                                    reg, \
                                    gcSL_ENABLE_##enable, \
                                    gcSL_FLOAT, \
                                    precision, \
                                    0))

#define OPCODE_COND_ENABLE(op,cond,reg,enable,precision) \
    gcmONERROR(gcSHADER_AddOpcode2(Shader, \
                                     gcSL_##op, \
                                     gcSL_##cond, \
                                     reg, \
                                     gcSL_ENABLE_##enable, \
                                     gcSL_FLOAT, \
                                     precision, \
                                     0))

#define OPCODE_COND(op,cond,target) \
    gcmONERROR(gcSHADER_AddOpcodeConditional(Shader, \
                                            gcSL_##op, \
                                            gcSL_##cond, \
                                            target, 0))

#define UNIFORM(name,swizzle) \
    gcmONERROR(gcSHADER_AddSourceUniform(Shader, \
                                        name, \
                                        _SWIZZLE_##swizzle, \
                                        0))

#define UNIFORM_ARRAY(name,swizzle,index) \
    gcmONERROR(gcSHADER_AddSourceUniform(Shader, \
                                        name, \
                                        _SWIZZLE_##swizzle, \
                                        index))

#define UNIFORM_ARRAY_INDEX(name,swizzle,index,reg,mode) \
    gcmONERROR(gcSHADER_AddSourceUniformIndexed(Shader, \
                                               name, \
                                               _SWIZZLE_##swizzle, \
                                               index, \
                                               gcSL_INDEXED_##mode, \
                                               reg))

#define SAMPLER(name) \
    do { \
        gctUINT32 index; \
        gcmONERROR(gcUNIFORM_GetSampler(name, &index)); \
        CONST((gctFLOAT) index); \
    } while (gcvFALSE)

#define SAMPLER_INDEX(swizzle,reg,mode) \
    gcmONERROR(gcSHADER_AddSourceSamplerIndexed(Shader, \
                                                  _SWIZZLE_##swizzle, \
                                                  gcSL_INDEXED_##mode, \
                                                  reg))

#define ATTRIBUTE(name,swizzle) \
    gcmONERROR(gcSHADER_AddSourceAttribute(Shader, \
                                          name, \
                                          _SWIZZLE_##swizzle, \
                                          0))

#define ATTRIBUTE_ARRAY(name,swizzle,index) \
    gcmONERROR(gcSHADER_AddSourceAttribute(Shader, \
                                          name, \
                                          _SWIZZLE_##swizzle, \
                                          index))

#define ATTRIBUTE_ARRAY_INDEX(name,swizzle,index,reg,mode) \
    gcmONERROR(gcSHADER_AddSourceAttributeIndexed(Shader, \
                                                 name, \
                                                 _SWIZZLE_##swizzle, \
                                                 index, \
                                                 gcSL_INDEXED_##mode, \
                                                 reg))
#ifdef CONST
#    undef CONST
#endif

#define CONST(c) \
    gcmONERROR(gcSHADER_AddSourceConstant(Shader, c))

#define TEMP(reg, swizzle, precision) \
    gcmONERROR(gcSHADER_AddSource(Shader, \
                                 gcSL_TEMP, \
                                 reg, \
                                 _SWIZZLE_##swizzle, \
                                 gcSL_FLOAT, \
                                 precision))

#define OUTPUT(name,temp) \
    gcmONERROR(gcSHADER_AddOutputIndexed(Shader, \
                                        #name, \
                                        0, \
                                        temp))

#define OUTPUT_ARRAY(name,temp,index) \
    gcmONERROR(gcSHADER_AddOutputIndexed(Shader, \
                                        #name, \
                                        index, \
                                        temp))

#define PACK() \
    gcmONERROR(gcSHADER_Pack(Shader))

#define ADD_FUNCTION(func) \
    gcmONERROR(gcSHADER_AddFunction(Shader, #func, &func))

#define ADD_FUNCTION_ARGUMENT(func,temp,enable,qualifier,precision) \
    gcmONERROR(gcFUNCTION_AddArgument(func, -1, temp, gcSL_ENABLE_##enable, qualifier, precision))

#define BEGIN_FUNCTION(func) \
    gcmONERROR(gcSHADER_BeginFunction(Shader, func))

#define END_FUNCTION(func) \
    gcmONERROR(gcSHADER_EndFunction(Shader, func))

#define FUNCTION_INPUT(func,index) \
    do { \
        gctUINT32 temp; \
        gctUINT8 enable; \
        gcmONERROR(gcFUNCTION_GetArgument(func, \
                                            index, \
                                            &temp, \
                                            &enable, \
                                            gcvNULL)); \
        gcmONERROR(gcSHADER_AddOpcode(Shader, \
                                        gcSL_MOV, \
                                        temp, \
                                        enable, \
                                        gcSL_FLOAT, \
                                        0)); \
    } while (gcvFALSE)

#define CALL(func,cond) \
    do { \
        gctUINT label; \
        gcmONERROR(gcFUNCTION_GetLabel(func, &label)); \
        gcmONERROR(gcSHADER_AddOpcodeConditional(Shader, \
                                                   gcSL_CALL, \
                                                   gcSL_##cond, \
                                                   label, 0)); \
    } while (gcvFALSE)

#define FUNCTION_OUTPUT(func,index) \
    gcmONERROR(gcSHADER_AddSource(Shader, \
                                 gcSL_TEMP, \
                                 gcFUNCTION_GetTemp(func, index), \
                                 gcFUNCTION_GetSwizzle(func, index), \
                                 gcSL_FLOAT))

#endif /* __gc_glsh_util_ */
