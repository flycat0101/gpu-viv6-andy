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


/*
**  Shader disassembler.
*/

#include "gc_vsc.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

static void
_DebugSwizzle(
              IN char * Buffer,
              IN gctSIZE_T BufferSize,
              IN OUT gctUINT * Offset,
              IN gctUINT32 Swizzle
              )
{
    static const char _swizzle[] = "xyzw";

    gctUINT32 swizzle[4];
    swizzle[0] = (Swizzle >> 0) & 3;
    swizzle[1] = (Swizzle >> 2) & 3;
    swizzle[2] = (Swizzle >> 4) & 3;
    swizzle[3] = (Swizzle >> 6) & 3;

    /* Only decode swizzle if not .xyzw. */
    if ((swizzle[0] != 0)
        ||    (swizzle[1] != 1)
        ||    (swizzle[2] != 2)
        ||    (swizzle[3] != 3)
        )
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
            ".%c", _swizzle[swizzle[0]]));

        /* Continue if swizzle is 2 or more components. */
        if ((swizzle[1] != swizzle[0])
            ||  (swizzle[2] != swizzle[0])
            ||  (swizzle[3] != swizzle[0])
            )
        {
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
                "%c", _swizzle[swizzle[1]]));

            /* Continue if swizzle is 3 or more components. */
            if ((swizzle[2] != swizzle[1])
                ||  (swizzle[3] != swizzle[1])
                )
            {
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
                    "%c", _swizzle[swizzle[2]]));

                /* Continue if swizzle is 4 components. */
                if (swizzle[3] != swizzle[2])
                {
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
                        "%c", _swizzle[swizzle[3]]));
                }
            }
        }
    }
}

static void
_DebugAddressing(
                 IN char * Buffer,
                 IN gctSIZE_T BufferSize,
                 IN OUT gctUINT * Offset,
                 IN gctUINT32 Relative
                 )
{
    static const char * _relative[] =
    {
        "",
        "[a0.x]",
        "[a0.y]",
        "[a0.z]",
        "[a0.w]",
        "[aL]",
    };

    /* Decode addressing. */
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, _relative[Relative]));
}

static void
_DebugRegister(
               IN char * Buffer,
               IN gctSIZE_T BufferSize,
               IN OUT gctUINT * Offset,
               IN gctUINT32 Type,
               IN gctUINT32 Address,
               IN gctUINT32 Relative,
               IN gctUINT32 Swizzle,
               IN gctUINT32 Negate,
               IN gctUINT32 Absolute,
               IN gctBOOL Precision
               )
{
    static const char* _type[] =
    {
        "r", /* 0x0     */
        "vFace", /* 0x1             */
        "c", /* 0x2  */
        "c", /* 0x3    */
        "VertexID", /* 0x4        */
        "r", /* 0x4       */
        "Ext", /* 0x5         */
        "InstanceID", /* 0x5      */
        "L", /* 0x6            */
        "I"           /* 0x7        */
    };

    static const char* _inst_index[] =
    {
        "instanceId", /* 0x00         */
        "in_primitiveId", /* 0x01  */
        "out_primitiveId", /* 0x02 */
        "iu", /* 0x03         */
        "is", /* 0x04         */
        "cash_id", /* 0x05            */
        "remap", /* 0x06               */
        "%#@!$", /* Reserved                                       */
        "remap0", /* 0x08              */
        "remap1", /* 0x09              */
        "remap2", /* 0x0A              */
        "remap3", /* 0x0B              */
        "remap4", /* 0x0C              */
        "remap5", /* 0x0D              */
        "next_pc", /* 0x0E         */
        "vertexid", /* 0x0F           */
        "sampleid", /* 0x10           */
        "samplepos", /* 0x11     */
        "in_samplemask", /* 0x12      */
    };

    if (0x7 == Type)
    {
        gctUINT32 data = ((Address << 0)
            |
            (Swizzle << 9)
            |
            (Negate << 17)
            |
            (Absolute << 18)
            |
            (Relative << 19)
            );

        /* Grab the fields.*/
        gctUINT32 type = (data >> 20) & 0x03;
        gctUINT32 raw  = data & 0xFFFFF;
        union {
            gctUINT32  hex;
            gctINT32   i;
            gctUINT16  u16;
            gctFLOAT   f32;
        } v;
        v.hex = raw;

        switch(type) {
        case 0x0:
            v.hex = GetFP32BianryFromFP20Binary(raw);
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
                "%f.f20", v.f32));
            break;
        case 0x1:
            if (raw & 0x80000)
                v.hex |= 0xFFF00000; /* sign extension */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
                "%d.s20", v.i));
            break;
        case 0x2:
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
                "%u.u20", v.hex));
            break;
        case 0x3:
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
                "0x%X.packed", v.u16));
            break;
        default:
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
                "???"));
            break;
        }

        return;
    }

    gcmASSERT(Type <= 0x6);
    if (Negate)
    {
        /* Negate prefix. */
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, "-"));
    }

    if (Absolute)
    {
        /* Absolute prefix. */
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, "|"));
    }

    /* Decode register type and address. */
    /* A temp solution because it is not clear vertexid/instanceid has been moved into extended type */
    if ((Address == 0x01 ||
         Address == 0x02) &&
        Type == 0x5)
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
            "%s", _inst_index[Address]));
    }
    else if ((Address >= 0x10 &&
              Address <= 0x12) &&
             Type == 0x5)
    {
         gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
            "%s", _inst_index[Address]));

         /* Decode swizzle. */
        _DebugSwizzle(Buffer, BufferSize, Offset, Swizzle);
    }
    else
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufferSize, Offset,
            "%s%u", (0x4 == Type && Precision) ?
            _type[0] : _type[Type], Address));

        /* Decode addressing. */
        _DebugAddressing(Buffer, BufferSize, Offset, Relative);

        if(Precision)
        {
            /* do not print .mp, it is default */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, "%s",
                (Type == 0x0) ? "" : ".hp"));
        }

        /* Decode swizzle. */
        _DebugSwizzle(Buffer, BufferSize, Offset, Swizzle);
    }

    if (Absolute)
    {
        /* Absolute postfix. */
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufferSize, Offset, "|"));
    }
}

/* dump machine code states */
void
gcDumpMCStates(
               IN gctUINT32 Address,
               IN gctUINT32 * States,
               IN gctBOOL OutputFormat,
               IN gctBOOL OutputHexStates,
               IN gctBOOL OutputDual16Modifiers,
               IN gctSIZE_T BufSize,
               OUT gctSTRING Buffer
               )
{
    gctUINT offset = 0;
    gctUINT column;

    /* Get instruction fields. */
    gctUINT32 opcode          = (((((gctUINT32) (States[0])) >> (0 ? 5:0)) & ((gctUINT32) ((((1 ? 5:0) - (0 ? 5:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:0) - (0 ? 5:0) + 1)))))) );
    gctUINT32 condition       = (((((gctUINT32) (States[0])) >> (0 ? 10:6)) & ((gctUINT32) ((((1 ? 10:6) - (0 ? 10:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:6) - (0 ? 10:6) + 1)))))) );
    gctUINT32 saturate        = (((((gctUINT32) (States[0])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) );
    gctUINT32 dest_valid      = (((((gctUINT32) (States[0])) >> (0 ? 12:12)) & ((gctUINT32) ((((1 ? 12:12) - (0 ? 12:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:12) - (0 ? 12:12) + 1)))))) );
    gctUINT32 dest_rel        = (((((gctUINT32) (States[0])) >> (0 ? 15:13)) & ((gctUINT32) ((((1 ? 15:13) - (0 ? 15:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:13) - (0 ? 15:13) + 1)))))) );
    gctUINT32 dest_addr       = (((((gctUINT32) (States[0])) >> (0 ? 22:16)) & ((gctUINT32) ((((1 ? 22:16) - (0 ? 22:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:16) - (0 ? 22:16) + 1)))))) );
    gctUINT32 dest_enable      = (((((gctUINT32) (States[0])) >> (0 ? 26:23)) & ((gctUINT32) ((((1 ? 26:23) - (0 ? 26:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:23) - (0 ? 26:23) + 1)))))) );
    gctUINT32 sampler_num     = (((((gctUINT32) (States[0])) >> (0 ? 31:27)) & ((gctUINT32) ((((1 ? 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1)))))) );
    gctUINT32 sampler_rel     = (((((gctUINT32) (States[1])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
    gctUINT32 sampler_swizzle = (((((gctUINT32) (States[1])) >> (0 ? 10:3)) & ((gctUINT32) ((((1 ? 10:3) - (0 ? 10:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:3) - (0 ? 10:3) + 1)))))) );
    gctUINT32 src0_valid      = (((((gctUINT32) (States[1])) >> (0 ? 11:11)) & ((gctUINT32) ((((1 ? 11:11) - (0 ? 11:11) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 11:11) - (0 ? 11:11) + 1)))))) );
    gctUINT32 src0_addr       = (((((gctUINT32) (States[1])) >> (0 ? 20:12)) & ((gctUINT32) ((((1 ? 20:12) - (0 ? 20:12) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 20:12) - (0 ? 20:12) + 1)))))) );
    gctUINT32 src0_swizzle    = (((((gctUINT32) (States[1])) >> (0 ? 29:22)) & ((gctUINT32) ((((1 ? 29:22) - (0 ? 29:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:22) - (0 ? 29:22) + 1)))))) );
    gctUINT32 src0_neg        = (((((gctUINT32) (States[1])) >> (0 ? 30:30)) & ((gctUINT32) ((((1 ? 30:30) - (0 ? 30:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:30) - (0 ? 30:30) + 1)))))) );
    gctUINT32 src0_abs        = (((((gctUINT32) (States[1])) >> (0 ? 31:31)) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) );
    gctUINT32 src0_rel        = (((((gctUINT32) (States[2])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
    gctUINT32 src0_type       = (((((gctUINT32) (States[2])) >> (0 ? 5:3)) & ((gctUINT32) ((((1 ? 5:3) - (0 ? 5:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:3) - (0 ? 5:3) + 1)))))) );
    gctUINT32 src1_valid      = (((((gctUINT32) (States[2])) >> (0 ? 6:6)) & ((gctUINT32) ((((1 ? 6:6) - (0 ? 6:6) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 6:6) - (0 ? 6:6) + 1)))))) );
    gctUINT32 src1_addr       = (((((gctUINT32) (States[2])) >> (0 ? 15:7)) & ((gctUINT32) ((((1 ? 15:7) - (0 ? 15:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:7) - (0 ? 15:7) + 1)))))) );
    gctUINT32 src1_swizzle    = (((((gctUINT32) (States[2])) >> (0 ? 24:17)) & ((gctUINT32) ((((1 ? 24:17) - (0 ? 24:17) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:17) - (0 ? 24:17) + 1)))))) );
    gctUINT32 src1_neg        = (((((gctUINT32) (States[2])) >> (0 ? 25:25)) & ((gctUINT32) ((((1 ? 25:25) - (0 ? 25:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:25) - (0 ? 25:25) + 1)))))) );
    gctUINT32 src1_abs        = (((((gctUINT32) (States[2])) >> (0 ? 26:26)) & ((gctUINT32) ((((1 ? 26:26) - (0 ? 26:26) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:26) - (0 ? 26:26) + 1)))))) );
    gctUINT32 src1_rel        = (((((gctUINT32) (States[2])) >> (0 ? 29:27)) & ((gctUINT32) ((((1 ? 29:27) - (0 ? 29:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 29:27) - (0 ? 29:27) + 1)))))) );
    gctUINT32 opcode_msb6     = (((((gctUINT32) (States[2])) >> (0 ? 16:16)) & ((gctUINT32) ((((1 ? 16:16) - (0 ? 16:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 16:16) - (0 ? 16:16) + 1)))))) );
    gctUINT32 src1_type       = (((((gctUINT32) (States[3])) >> (0 ? 2:0)) & ((gctUINT32) ((((1 ? 2:0) - (0 ? 2:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 2:0) - (0 ? 2:0) + 1)))))) );
    gctUINT32 src2_valid      = (((((gctUINT32) (States[3])) >> (0 ? 3:3)) & ((gctUINT32) ((((1 ? 3:3) - (0 ? 3:3) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 3:3) - (0 ? 3:3) + 1)))))) );
    gctUINT32 src2_addr       = (((((gctUINT32) (States[3])) >> (0 ? 12:4)) & ((gctUINT32) ((((1 ? 12:4) - (0 ? 12:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 12:4) - (0 ? 12:4) + 1)))))) );
    gctUINT32 src2_swizzle    = (((((gctUINT32) (States[3])) >> (0 ? 21:14)) & ((gctUINT32) ((((1 ? 21:14) - (0 ? 21:14) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:14) - (0 ? 21:14) + 1)))))) );
    gctUINT32 src2_neg        = (((((gctUINT32) (States[3])) >> (0 ? 22:22)) & ((gctUINT32) ((((1 ? 22:22) - (0 ? 22:22) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 22:22) - (0 ? 22:22) + 1)))))) );
    gctUINT32 src2_abs        = (((((gctUINT32) (States[3])) >> (0 ? 23:23)) & ((gctUINT32) ((((1 ? 23:23) - (0 ? 23:23) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 23:23) - (0 ? 23:23) + 1)))))) );
    gctUINT32 src2_rel        = (((((gctUINT32) (States[3])) >> (0 ? 27:25)) & ((gctUINT32) ((((1 ? 27:25) - (0 ? 27:25) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 27:25) - (0 ? 27:25) + 1)))))) );
    gctUINT32 src2_type       = (((((gctUINT32) (States[3])) >> (0 ? 30:28)) & ((gctUINT32) ((((1 ? 30:28) - (0 ? 30:28) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 30:28) - (0 ? 30:28) + 1)))))) );
    gctUINT32 target          = (((((gctUINT32) (States[3])) >> (0 ? 26:7)) & ((gctUINT32) ((((1 ? 26:7) - (0 ? 26:7) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 26:7) - (0 ? 26:7) + 1)))))) );

    gctUINT32 rounding_mode   = 0;
    gctUINT32 threadMode      = 0;
    gctUINT32 vision_opcode   = 0;
    gctUINT32 start_dst_comp_idx = dest_enable, end_dst_comp_idx = (sampler_num & 0xF);
    gctUINT32 unpackMode         = (sampler_rel >> 2) & 0x1;

    static const char * _opcode[] =
    {
        "nop", /* 0x00 */
        "add", /* 0x01 */
        "mad", /* 0x02 */
        "mul", /* 0x03 */
        "dst", /* 0x04 */
        "dp3", /* 0x05 */
        "dp4", /* 0x06 */
        "dsx", /* 0x07 */
        "dsy", /* 0x08 */
        "mov", /* 0x09 */
        "movar", /* 0x0a */
        "movaf", /* 0x0b */
        "rcp", /* 0x0c */
        "rsq", /* 0x0d */
        "litp", /* 0x0e */
        "select", /* 0x0f */
        "set", /* 0x10 */
        "exp", /* 0x11 */
        "log", /* 0x12 */
        "frc", /* 0x13 */
        "call", /* 0x14 */
        "ret", /* 0x15 */
        "branch", /* 0x16 */
        "texkill", /* 0x17 */
        "texld", /* 0x18 */
        "texldb", /* 0x19 */
        "texldd", /* 0x1a */
        "texldl", /* 0x1b */
        "texldpcf", /* 0x1c */
        "rep", /* 0x1d */
        "endrep", /* 0x1e */
        "loop", /* 0x1f */
        "endloop", /* 0x20 */
        "sqrt", /* 0x21 */
        "sin", /* 0x22 */
        "cos", /* 0x23 */
        "branch_any", /* 0x24 */
        "floor", /* 0x25 */
        "ceil", /* 0x26 */
        "sign", /* 0x27 */
        "addlo", /* 0x28 */
        "mullo", /* 0x29 */
        "barrier", /* 0x2a */
        "swizzle", /* 0x2b */
        "i2i", /* 0x2c */
        "i2f", /* 0x2d */
        "f2i", /* 0x2e */
        "f2irnd", /* 0x2f */
        "fma", /* 0x30 */
        "cmp", /* 0x31 */
        "load", /* 0x32 */
        "store", /* 0x33 */
        "img_load_3d", /* 0x34 */
        "img_store_3d", /* 0x35 */
        "clamp0_max", /* 0x36 */
        "img_addr", /* 0x37 */
        "img_addr_3d", /* 0x38 */
        "roundeven", /* 0x39 */
        "roundaway", /* 0x3a */
        "iaddsat", /* 0x3b */
        "imullo0", /* 0x3c */
        "imullo1", /* 0x3d */
        "imullosat0", /* 0x3e */
        "imullosat1", /* 0x3f */
        "imulhi0", /* 0x40 */
        "imulhi1", /* 0x41 */
        "store_attr", /* 0x42 */
        "select_map", /* 0x43 */
        "idiv0", /* 0x44 */
        "evis", /* 0x45 */
        "img_atom", /* 0x46 */
        "_reserved", /* 0x47 */
        "imod0", /* 0x48 */
        "texld_u_s_l", /* 0x49 */
        "texld_u_u_l", /* 0x4a */
        "texld_u_f_l", /* 0x4b */
        "imadlo0", /* 0x4c */
        "imadlo1", /* 0x4d */
        "imadlosat0", /* 0x4e */
        "imadlosat1", /* 0x4f */
        "imadhi0", /* 0x50 */
        "imadhi1", /* 0x51 */
        "imadhisat0", /* 0x52 */
        "imadhisat1", /* 0x53 */
        "bit_insert1", /* 0x54 */
        "bit_insert2", /* 0x55 */
        "movai", /* 0x56 */
        "iabs", /* 0x57 */
        "leadzero", /* 0x58 */
        "lshift", /* 0x59 */
        "rshift", /* 0x5a */
        "rotate", /* 0x5b */
        "or", /* 0x5c */
        "and", /* 0x5d */
        "xor", /* 0x5e */
        "not", /* 0x5f */
        "bit_extract", /* 0x60 */
        "popcount", /* 0x61 */
        "_reserved", /* 0x62 */
        "arc_trig", /* 0x63 */
        "div", /* 0x64 */
        "atom_add", /* 0x65 */
        "atom_xchg", /* 0x66 */
        "atom_cmp_xchg", /* 0x67 */
        "atom_min", /* 0x68 */
        "atom_max", /* 0x69 */
        "atom_or", /* 0x6a */
        "atom_and", /* 0x6b */
        "atom_xor", /* 0x6c */
        "bit_rev", /* 0x6d */
        "byte_rev", /* 0x6e */
        "texldl", /* 0x6f */
        "texldgpcf", /* 0x70 */
        "pack", /* 0x71 */
        "conv", /* 0x72 */
        "dp2", /* 0x73 */
        "norm_dp2", /* 0x74 */
        "norm_dp3", /* 0x75 */
        "norm_dp4", /* 0x76 */
        "norm_mul", /* 0x77 */
        "load_attr", /* 0x78 */
        "img_load", /* 0x79 */
        "img_store", /* 0x7a */
        "texld_u", /* 0x7b */
        "lodqg", /* 0x7c */
        "texld_gather", /* 0x7d */
        "_reserved", /* 0x7e */
        "_extended", /* 0x7f */
        "emit", /* 0x01 */
        "restart", /* 0x02 */
        "flush", /* 0x03 */
        "lodq", /* 0x04 */
        "_reserved", /* 0x05 */
        "_reserved", /* 0x06 */
        "_reserved", /* 0x07 */
        "_reserved", /* 0x08 */
        "_reserved", /* 0x09 */
        "_reserved", /* 0x0a */
        "bit_findlsb", /* 0x0b */
        "bit_findmsb", /* 0x0c */
        "texld_fetchMS", /* 0x0d */
        "texld_fetchMS_u", /* 0x0e */
        "halfmix", /* 0x0f */
        "ldexp", /* 0x10 */
    };

    static const char* _vision_opcode[] =
    {
        "&%^$#@(*",
        "vx_abs_diff",
        "vx_iadd",
        "vx_iacc_sq",
        "vx_lerp",
        "vx_filter",
        "vx_mag_phase",
        "vx_mul_shift",
        "vx_dp16x1",
        "vx_dp8x2",
        "vx_dp4x4",
        "vx_dp2x8",
        "vx_clamp",
        "vx_bi_linear",
        "vx_select_add",
        "vx_atomic_add",
        "vx_bit_extract",
        "vx_bit_replace",
        "vx_dp32x1",
        "vx_dp16x2",
        "vx_dp8x4",
        "vx_dp4x8",
        "vx_dp2x16",
    };

    /* same as definitions in AQShader.h */
    static const char * _condition[] =
    {
        "true",
        "gt",
        "lt",
        "ge",
        "le",
        "eq",
        "ne",
        "and",
        "or",
        "xor",
        "not",
        "nz",
        "gez",
        "gz",
        "lez",
        "lz",
        "finite",
        "infinite",
        "nan",
        "normal",
        "anymsb",
        "allmsb",
        "selmsb",
        "ucarry",
        "helper",
        "nothelper",
    };

    static const char * _typeNames[] =
    {
        "f32",
        "f16",
        "s32",
        "s16",
        "s8",
        "u32",
        "u16",
        "u8"
    };

    static const char * _roundingModeNames[] =
    {
        "", /* default */
        "rtz", /* round to zero */
        "rtne"    /* round to nearest even */
    };

    static const char * _threadTypeNames[] =
    {
        "", /* mode TOT1 */
        "t0", /* mode T0 */
        "t1"      /* mode T1 */
    };

    /* Assemble opcode. */
    opcode += opcode_msb6 << 6;

    /* Decode extended opcode. */
    if (opcode == 0x7F)
    {
        /* Get extended opcode from src2. */
        gctUINT32 data = ((src2_addr    <<  0)
                        | (src2_swizzle <<  9)
                        | (src2_neg     << 17)
                        | (src2_abs     << 18)
                        | (src2_rel     << 19)
                        );
        gctUINT32 raw  = data & 0xFFFFF;

        gcmASSERT(src2_valid);
        gcmASSERT(0x7 == src2_type);
        gcmASSERT(0x2 == ((data >> 20) & 0x03));
        opcode += raw;
    }

    if (opcode == 0x45)
    {
        vision_opcode = (dest_rel | (((sampler_num & 0x10) >> 4) << 3) | (sampler_rel & 0x3) << 4);
    }

    /* set rounding mode for the instruction */
    switch (opcode)
    {
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x7D:
    case 0x1C:
    case 0x6F:
    case 0x70:
    case 0x7B:
    case 0x7C:
    case 0x7F + 0x04:
    case 0x7F + 0x0D:
    case 0x16:
    case 0x14:
    case 0x45:
        break;

    default:
        /* borrow sampler_rel bits for rounding mode */
        rounding_mode = (sampler_rel & 0x3);
        break;
    }

    /* set thread mode for the instruction */
    switch (opcode)
    {
    case 0x14:
        break;

    default:
        threadMode = OutputDual16Modifiers ? (((((((gctUINT32) (States[3])) >> (0 ? 24:24)) & ((gctUINT32) ((((1 ? 24:24) - (0 ? 24:24) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 24:24) - (0 ? 24:24) + 1)))))) ) << 1) |
            (((((gctUINT32) (States[3])) >> (0 ? 13:13)) & ((gctUINT32) ((((1 ? 13:13) - (0 ? 13:13) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 13:13) - (0 ? 13:13) + 1)))))) ))
            : 0;

        break;
    }
    /* Decode address. */
    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
        "%03u: ", Address));

    column = 24;

    /* Decode opcode. */
    if (opcode == 0x45)
    {
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
            "%s", _vision_opcode[vision_opcode]));
    }
    else
    {
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
            "%s", _opcode[opcode]));
    }

    /* Print pcf/bias if needed. */
    if (opcode == 0x18)
    {
        if (src1_valid)
        {
            if (src2_valid)
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "_bias_pcf"));
            }
            else
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "_bias"));
            }
        }
        else
        {
            if (src2_valid)
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "_pcf"));
            }
        }
    }
    else if (opcode == 0x6F ||
             opcode == 0x7D)
    {
        if (src2_valid)
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "_pcf"));
        }
    }

    if(OutputDual16Modifiers && threadMode > 0)
    {
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
            ".%s", _threadTypeNames[threadMode]));
    }

    /* Decode condition if not gcvTRUE. */
    if (condition != 0x00)
    {
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
            ".%s", _condition[condition]));
    }

    /* Decode rounding mode if not default. */
    if (rounding_mode)
    {
        gcmASSERT(rounding_mode <= 2);
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
            ".%s", _roundingModeNames[rounding_mode]));
    }
    if (unpackMode)
    {
        gctSTRING  str = (opcode == 0x7A ||
                          opcode == 0x79) ? ".evis" :
                         (opcode == 0x45)     ? ""
                                                              : ".pack";
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "%s", str));
    }
    if (OutputFormat)
    {
        gctUINT32 inst_type0  = (((((gctUINT32) (States[1])) >> (0 ? 21:21)) & ((gctUINT32) ((((1 ? 21:21) - (0 ? 21:21) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 21:21) - (0 ? 21:21) + 1)))))) );
        gctUINT32 inst_type1  = (((((gctUINT32) (States[2])) >> (0 ? 31:30)) & ((gctUINT32) ((((1 ? 31:30) - (0 ? 31:30) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:30) - (0 ? 31:30) + 1)))))) );
        gctUINT32 value_type0 = (inst_type1 << 1) + inst_type0;

        /* Decode format for instructions supporting multiple formats. */
        switch (opcode)
        {
        case 0x09:
        case 0x01:
        case 0x10:
        case 0x31:
        case 0x16:
        case 0x0F:
        case 0x27:
        case 0x32:
        case 0x2E:
        case 0x2D:
        case 0x57:
        case 0x58:
        case 0x5F:
        case 0x44:
        case 0x48:
        case 0x3B:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x3C:
        case 0x3D:
        case 0x40:
        case 0x41    :
        case 0x3E:
        case 0x3F:
        case 0x4C:
        case 0x4D:
        case 0x50:
        case 0x51:
        case 0x4E:
        case 0x4F:
        case 0x52:
        case 0x53:
        case 0x61:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x2B:
            if (value_type0 > 0)
            {
                /* by convention, default is .f32, so ignore it. */
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
                    ".%s", _typeNames[value_type0]));
            }
            break;

        case 0x2C:
        case 0x72:
        case 0x79:
        case 0x34:
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
                ".%s", _typeNames[value_type0]));
            break;

        case 0x0A:
        case 0x0B:
        case 0x56:
        case 0x33:
        case 0x7A:
        case 0x35:
        case 0x42:
            if (opcode != 0x7A &&
                opcode != 0x35 &&
                opcode != 0x42)
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
                    ".%s", _typeNames[value_type0]));
            }

            if ((opcode == 0x7A ||
                 opcode == 0x35) &&
                 (sampler_rel & 0x4))
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, ".{%d, %d}",
                             start_dst_comp_idx, end_dst_comp_idx));
            }
            else
            {
                if (dest_enable != 0xF)
                {
                    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "."));
                    if (dest_enable & 0x1)
                    {
                        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "x"));
                    }
                    if (dest_enable & 0x2)
                    {
                        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "y"));
                    }
                    if (dest_enable & 0x4)
                    {
                        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "z"));
                    }
                    if (dest_enable & 0x8)
                    {
                        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "w"));
                    }
                }
            }
            break;

        default:
            break;
        }
    }

    /* Decode saturate post modifier. */
    if (saturate)
    {
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, ".sat"));
    }

    /* Move to operand column. */
    while (offset < column)
    {
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, " "));
    }

    /* Decode destination. */
    if (dest_valid)
    {
        if (((opcode == 0x79 ||
              opcode == 0x34) &&
              (sampler_rel & 0x4)) ||
            (opcode == 0x45))
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "%c%u", 'r', dest_addr));

            if(OutputDual16Modifiers)
            {
                gctUINT precision;
                precision = (((((gctUINT32) (States[3])) >> (0 ? 31:31)) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) );
                /* do not print .mp, it is default */
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "%s",
                    (precision == 0x0) ? "": ".hp"));

                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, ".{%d, %d}",
                             start_dst_comp_idx, end_dst_comp_idx));
            }
        }
        else if (dest_enable != 0)
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "%c%u",
                ((opcode == 0x0A)
                || (opcode == 0x0B)
                || (opcode == 0x56)) ? 'a' : 'r',
                dest_addr));

            _DebugAddressing(Buffer, BufSize, &offset, dest_rel);

            if(OutputDual16Modifiers)
            {
                gctUINT precision;
                precision = (((((gctUINT32) (States[3])) >> (0 ? 31:31)) & ((gctUINT32) ((((1 ? 31:31) - (0 ? 31:31) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:31) - (0 ? 31:31) + 1)))))) );
                /* do not print .mp, it is default */
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "%s",
                    (precision == 0x0) ? "": ".hp"));
            }

            if (dest_enable != 0xF)
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "."));
                if (dest_enable & 0x1)
                {
                    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "x"));
                }
                if (dest_enable & 0x2)
                {
                    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "y"));
                }
                if (dest_enable & 0x4)
                {
                    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "z"));
                }
                if (dest_enable & 0x8)
                {
                    gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, "w"));
                }
            }
        }
    }
    /* Decode source 0. */
    if (src0_valid)
    {
        /* Append comma if not first operand. */
        if (offset > column)
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, ", "));
        }

        _DebugRegister(Buffer, BufSize, &offset,
            src0_type,
            src0_addr,
            src0_rel,
            (opcode == 0x45) ? 0xE4 : src0_swizzle,
            src0_neg,
            src0_abs,
            OutputDual16Modifiers);
    }

    /* Decode source 1. */
    if (src1_valid)
    {
        /* Append comma if not first operand. */
        if (offset > column)
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, ", "));
        }

        _DebugRegister(Buffer, BufSize, &offset,
            src1_type,
            src1_addr,
            src1_rel,
            src1_swizzle,
            src1_neg,
            src1_abs,
            OutputDual16Modifiers);
    }

    /* Decode source 2. */
    if (src2_valid && (opcode < 0x7F))
    {
        /* Append comma if not first operand. */
        if (offset > column)
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, ", "));
        }

        _DebugRegister(Buffer, BufSize, &offset,
            src2_type,
            src2_addr,
            src2_rel,
            src2_swizzle,
            src2_neg,
            src2_abs,
            OutputDual16Modifiers);
    }

    /* Handle special opcodes. */
    switch (opcode)
    {
    case 0x18:
    case 0x4B:
    case 0x49:
    case 0x4A:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x7D:
    case 0x1C:
    case 0x6F:
    case 0x70:
    case 0x7B:
    case 0x7C:
    case 0x7F + 0x04:
    case 0x7F + 0x0D:
        /* Append comma if not first operand. */
        if (offset > column)
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, ", "));
        }

        /* Append sampler. */
        gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
            "s%u", sampler_num));
        _DebugAddressing(Buffer, BufSize, &offset, sampler_rel);
        _DebugSwizzle(Buffer, BufSize, &offset, sampler_swizzle);
        break;

    case 0x16:
    case 0x14:
        /* branch/call instruction may use src2 20bit immediate number as target */
        if (!src2_valid)
        {
            gcmASSERT(!OutputDual16Modifiers || opcode == 0x16 || threadMode == 0);
            /* Append comma if not first operand. */
            if (offset > column)
            {
                gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, ", "));
            }

            /* Append target. */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
                "%u", target));
            break;
        }
        else
        {
            gcmASSERT(0x7 == src2_type);
        }
    }

    /* Decode hex states */
    if (OutputHexStates)
    {
        column += 36;
        while (offset < column)
        {
            gcmVERIFY_OK(gcoOS_PrintStrSafe(Buffer, BufSize, &offset, " "));
        }

        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(Buffer, BufSize, &offset,
            "  # 0x%08x 0x%08x 0x%08x 0x%08x",
            States[0], States[1], States[2], States[3]));

    }

} /* gcDumpMCStates */

void dbg_dumpMC(
                IN gctUINT32_PTR States
                )
{
    gctCHAR buffer[192];
    gcDumpMCStates(0, States, gcvTRUE,
        gcvFALSE, gcvFALSE,
        sizeof(buffer), buffer);
    gcoOS_Print("%s", buffer);
}

void dbg_dumpMC16(
                  IN gctUINT32_PTR States
                  )
{
    gctCHAR buffer[192];
    gcDumpMCStates(0, States, gcvTRUE,
        gcvFALSE, gcvTRUE,
        sizeof(buffer), buffer);
    gcoOS_Print("%s", buffer);
}

void
_DumpShader(
            IN gctUINT32_PTR States,
            IN gctUINT32 StateBufferOffset,
            IN gctBOOL OutputFormat,
            IN gctUINT InstBase,
            IN gctUINT InstMax,
            IN gctBOOL IsDual16Shader
            )
{
    gctUINTPTR_T lastState;
    gctUINT32 address, count, nextAddress;

    lastState = (gctUINTPTR_T)((gctUINT8_PTR) States + StateBufferOffset);
    nextAddress = 0;

    while ((gctUINTPTR_T) States < lastState)
    {
        gctUINT32 state = *States++;

        gcmASSERT(((((gctUINT32) (state)) >> (0 ? 31:27) & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:27) + 1)))))) == (0x01 & ((gctUINT32) ((((1 ?
 31:27) - (0 ? 31:27) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 31:27) - (0 ? 31:
27) + 1))))))));

        address = (((((gctUINT32) (state)) >> (0 ? 15:0)) & ((gctUINT32) ((((1 ? 15:0) - (0 ? 15:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 15:0) - (0 ? 15:0) + 1)))))) );
        count   = (((((gctUINT32) (state)) >> (0 ? 25:16)) & ((gctUINT32) ((((1 ? 25:16) - (0 ? 25:16) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 25:16) - (0 ? 25:16) + 1)))))) );

        if ((address >= InstBase) &&
            (address < InstBase + InstMax) )
        {
            if (nextAddress == 0) {
                /* Header. */
                gcoOS_Print("***** [ Generated Shader Code ] *****");
            }

            /* Dump all states. */
            for (address = 0; count >= 4; count -= 4)
            {
                gctCHAR buffer[192];

                /* Dump shader code. */
                gcDumpMCStates(address++ + nextAddress, States, OutputFormat,
                    gcvTRUE, IsDual16Shader, sizeof(buffer), buffer);
                gcoOS_Print("%s", buffer);

                /* Next instruction. */
                States += 4;
            }
            nextAddress += address;
        }
        else
        {
            States += count;
        }

        if ((count & 1) == 0) ++States;
    }
}


