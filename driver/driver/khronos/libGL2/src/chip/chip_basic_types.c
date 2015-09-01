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


#include "gc_gl_context.h"
#include "chip_context.h"

#define _GC_OBJ_ZONE glvZONE_TRACE

static const GLfixed sinx_table[1024] =
{
    0x000000, 0x000097, 0x0000fb, 0x000160, 0x0001c4, 0x000229, 0x00028d, 0x0002f2,
    0x000356, 0x0003bb, 0x000420, 0x000484, 0x0004e9, 0x00054d, 0x0005b2, 0x000616,
    0x00067b, 0x0006df, 0x000744, 0x0007a8, 0x00080d, 0x000871, 0x0008d5, 0x00093a,
    0x00099e, 0x000a03, 0x000a67, 0x000acc, 0x000b30, 0x000b95, 0x000bf9, 0x000c5d,
    0x000cc2, 0x000d56, 0x000d66, 0x000def, 0x000e53, 0x000eb8, 0x000f1c, 0x000f81,
    0x000fe5, 0x001049, 0x0010ae, 0x001112, 0x001176, 0x0011da, 0x00123f, 0x0012a3,
    0x001307, 0x00136c, 0x0013d0, 0x001434, 0x001498, 0x0014fc, 0x001561, 0x0015c5,
    0x001629, 0x00168d, 0x0016f1, 0x001755, 0x0017b9, 0x00181d, 0x001882, 0x0018e6,
    0x00194a, 0x0019ae, 0x001a12, 0x001a76, 0x001ada, 0x001b3e, 0x001ba2, 0x001c06,
    0x001c69, 0x001ccd, 0x001d31, 0x001d95, 0x001df9, 0x001e5d, 0x001ec1, 0x001f24,
    0x001f88, 0x001fec, 0x002050, 0x0020b3, 0x002117, 0x00217b, 0x0021de, 0x002242,
    0x0022a6, 0x002309, 0x00236d, 0x0023d0, 0x002434, 0x002497, 0x0024fb, 0x00255e,
    0x0025c2, 0x002625, 0x002689, 0x0026ec, 0x00274f, 0x0027b3, 0x002816, 0x002879,
    0x0028dd, 0x002940, 0x0029a3, 0x002a06, 0x002a69, 0x002acc, 0x002b30, 0x002b93,
    0x002bf6, 0x002c59, 0x002cbc, 0x002d1f, 0x002d82, 0x002de5, 0x002e47, 0x002eaa,
    0x002f0d, 0x002f70, 0x002fd3, 0x003035, 0x003098, 0x0030fb, 0x00315e, 0x0031c0,
    0x003223, 0x003285, 0x0032e8, 0x00334a, 0x0033ad, 0x00340f, 0x003472, 0x0034d4,
    0x003536, 0x003599, 0x0035fb, 0x00365d, 0x0036c0, 0x003722, 0x003784, 0x0037e6,
    0x003848, 0x0038aa, 0x00390c, 0x00396e, 0x0039d0, 0x003a32, 0x003a94, 0x003af6,
    0x003b58, 0x003bb9, 0x003c1b, 0x003c7d, 0x003cde, 0x003d40, 0x003da2, 0x003e03,
    0x003e65, 0x003ec6, 0x003f28, 0x003f89, 0x003fea, 0x00404c, 0x0040ad, 0x00410e,
    0x00416f, 0x0041d1, 0x004232, 0x004293, 0x0042f4, 0x004355, 0x0043b6, 0x004417,
    0x004478, 0x0044d9, 0x004539, 0x00459a, 0x0045fb, 0x00465c, 0x0046bc, 0x00471d,
    0x00477d, 0x0047de, 0x00483e, 0x00489f, 0x0048ff, 0x00495f, 0x0049c0, 0x004a20,
    0x004a80, 0x004ae0, 0x004b40, 0x004ba1, 0x004c01, 0x004c61, 0x004cc0, 0x004d20,
    0x004d80, 0x004de0, 0x004e40, 0x004e9f, 0x004eff, 0x004f5f, 0x004fbe, 0x00501e,
    0x00507d, 0x0050dd, 0x00513c, 0x00519b, 0x0051fb, 0x00525a, 0x0052b9, 0x005318,
    0x005377, 0x0053d6, 0x005435, 0x005494, 0x0054f3, 0x005552, 0x0055b0, 0x00560f,
    0x00566e, 0x0056cc, 0x00572b, 0x005789, 0x0057e8, 0x005846, 0x0058a5, 0x005903,
    0x005961, 0x0059bf, 0x005a1d, 0x005a7b, 0x005ad9, 0x005b37, 0x005b95, 0x005bf3,
    0x005c51, 0x005caf, 0x005d0c, 0x005d6a, 0x005dc8, 0x005e25, 0x005e83, 0x005ee0,
    0x005f3d, 0x005f9b, 0x005ff8, 0x006055, 0x0060b2, 0x00610f, 0x00616c, 0x0061c9,
    0x006226, 0x006283, 0x0062e0, 0x00633c, 0x006399, 0x0063f5, 0x006452, 0x0064ae,
    0x00650b, 0x006567, 0x0065c3, 0x006620, 0x00667c, 0x0066d8, 0x006734, 0x006790,
    0x0067ec, 0x006848, 0x0068a3, 0x0068ff, 0x00695b, 0x0069b6, 0x006a12, 0x006a6d,
    0x006ac9, 0x006b24, 0x006b7f, 0x006bdb, 0x006c36, 0x006c91, 0x006cec, 0x006d47,
    0x006da2, 0x006dfc, 0x006e57, 0x006eb2, 0x006f0d, 0x006f67, 0x006fc2, 0x00701c,
    0x007076, 0x0070d1, 0x00712b, 0x007185, 0x0071df, 0x007239, 0x007293, 0x0072ed,
    0x007347, 0x0073a0, 0x0073fa, 0x007454, 0x0074ad, 0x007507, 0x007560, 0x0075b9,
    0x007612, 0x00766c, 0x0076c5, 0x00771e, 0x007777, 0x0077d0, 0x007828, 0x007881,
    0x0078da, 0x007932, 0x00798b, 0x0079e3, 0x007a3c, 0x007a94, 0x007aec, 0x007b44,
    0x007b9c, 0x007bf4, 0x007c4c, 0x007ca4, 0x007cfc, 0x007d54, 0x007dab, 0x007e03,
    0x007e5a, 0x007eb2, 0x007f09, 0x007f60, 0x007fb7, 0x00800f, 0x008066, 0x0080bc,
    0x008113, 0x00816a, 0x0081c1, 0x008217, 0x00826e, 0x0082c4, 0x00831b, 0x008371,
    0x0083c7, 0x00841d, 0x008474, 0x0084ca, 0x00851f, 0x008575, 0x0085cb, 0x008621,
    0x008676, 0x0086cc, 0x008721, 0x008777, 0x0087cc, 0x008821, 0x008876, 0x0088cb,
    0x008920, 0x008975, 0x0089ca, 0x008a1e, 0x008a73, 0x008ac7, 0x008b1c, 0x008b70,
    0x008bc5, 0x008c19, 0x008c6d, 0x008cc1, 0x008d15, 0x008d69, 0x008dbc, 0x008e10,
    0x008e64, 0x008eb7, 0x008f0b, 0x008f5e, 0x008fb1, 0x009004, 0x009057, 0x0090aa,
    0x0090fd, 0x009150, 0x0091a3, 0x0091f5, 0x009248, 0x00929a, 0x0092ed, 0x00933f,
    0x009391, 0x0093e3, 0x009435, 0x009487, 0x0094d9, 0x00952b, 0x00957d, 0x0095ce,
    0x009620, 0x009671, 0x0096c2, 0x009713, 0x009765, 0x0097b6, 0x009807, 0x009857,
    0x0098a8, 0x0098f9, 0x009949, 0x00999a, 0x0099ea, 0x009a3a, 0x009a8b, 0x009adb,
    0x009b2b, 0x009b7b, 0x009bca, 0x009c1a, 0x009c6a, 0x009cb9, 0x009d09, 0x009d58,
    0x009da7, 0x009df7, 0x009e46, 0x009e95, 0x009ee3, 0x009f32, 0x009f81, 0x009fd0,
    0x00a01e, 0x00a06c, 0x00a0bb, 0x00a109, 0x00a157, 0x00a1a5, 0x00a1f3, 0x00a241,
    0x00a28e, 0x00a2dc, 0x00a32a, 0x00a377, 0x00a3c4, 0x00a412, 0x00a45f, 0x00a4ac,
    0x00a4f9, 0x00a545, 0x00a592, 0x00a5df, 0x00a62b, 0x00a678, 0x00a6c4, 0x00a710,
    0x00a75c, 0x00a7a8, 0x00a7f4, 0x00a840, 0x00a88c, 0x00a8d7, 0x00a923, 0x00a96e,
    0x00a9ba, 0x00aa05, 0x00aa50, 0x00aa9b, 0x00aae6, 0x00ab31, 0x00ab7b, 0x00abc6,
    0x00ac11, 0x00ac5b, 0x00aca5, 0x00acef, 0x00ad39, 0x00ad83, 0x00adcd, 0x00ae17,
    0x00ae61, 0x00aeaa, 0x00aef4, 0x00af3d, 0x00af86, 0x00afcf, 0x00b018, 0x00b061,
    0x00b0aa, 0x00b0f3, 0x00b13b, 0x00b184, 0x00b1cc, 0x00b215, 0x00b25d, 0x00b2a5,
    0x00b2ed, 0x00b335, 0x00b37c, 0x00b3c4, 0x00b40b, 0x00b453, 0x00b49a, 0x00b4e1,
    0x00b528, 0x00b56f, 0x00b5b6, 0x00b5fd, 0x00b644, 0x00b68a, 0x00b6d1, 0x00b717,
    0x00b75d, 0x00b7a3, 0x00b7e9, 0x00b82f, 0x00b875, 0x00b8bb, 0x00b900, 0x00b946,
    0x00b98b, 0x00b9d0, 0x00ba15, 0x00ba5a, 0x00ba9f, 0x00bae4, 0x00bb28, 0x00bb6d,
    0x00bbb1, 0x00bbf6, 0x00bc3a, 0x00bc7e, 0x00bcc2, 0x00bd06, 0x00bd4a, 0x00bd8d,
    0x00bdd1, 0x00be14, 0x00be57, 0x00be9b, 0x00bede, 0x00bf21, 0x00bf63, 0x00bfa6,
    0x00bfe9, 0x00c02b, 0x00c06e, 0x00c0b0, 0x00c0f2, 0x00c134, 0x00c176, 0x00c1b8,
    0x00c1f9, 0x00c23b, 0x00c27c, 0x00c2be, 0x00c2ff, 0x00c340, 0x00c381, 0x00c3c2,
    0x00c402, 0x00c443, 0x00c483, 0x00c4c4, 0x00c504, 0x00c544, 0x00c584, 0x00c5c4,
    0x00c604, 0x00c644, 0x00c683, 0x00c6c2, 0x00c702, 0x00c741, 0x00c780, 0x00c7bf,
    0x00c7fe, 0x00c83c, 0x00c87b, 0x00c8ba, 0x00c8f8, 0x00c936, 0x00c974, 0x00c9b2,
    0x00c9f0, 0x00ca2e, 0x00ca6b, 0x00caa9, 0x00cae6, 0x00cb23, 0x00cb61, 0x00cb9e,
    0x00cbda, 0x00cc17, 0x00cc54, 0x00cc90, 0x00cccd, 0x00cd09, 0x00cd45, 0x00cd81,
    0x00cdbd, 0x00cdf9, 0x00ce34, 0x00ce70, 0x00ceab, 0x00cee7, 0x00cf22, 0x00cf5d,
    0x00cf98, 0x00cfd2, 0x00d00d, 0x00d047, 0x00d082, 0x00d0bc, 0x00d0f6, 0x00d130,
    0x00d16a, 0x00d1a4, 0x00d1de, 0x00d217, 0x00d250, 0x00d28a, 0x00d2c3, 0x00d2fc,
    0x00d335, 0x00d36d, 0x00d3a6, 0x00d3df, 0x00d417, 0x00d44f, 0x00d487, 0x00d4bf,
    0x00d4f7, 0x00d52f, 0x00d566, 0x00d59e, 0x00d5d5, 0x00d60c, 0x00d644, 0x00d67a,
    0x00d6b1, 0x00d6e8, 0x00d71f, 0x00d755, 0x00d78b, 0x00d7c1, 0x00d7f8, 0x00d82d,
    0x00d863, 0x00d899, 0x00d8ce, 0x00d904, 0x00d939, 0x00d96e, 0x00d9a3, 0x00d9d8,
    0x00da0d, 0x00da41, 0x00da76, 0x00daaa, 0x00dade, 0x00db12, 0x00db46, 0x00db7a,
    0x00dbae, 0x00dbe1, 0x00dc15, 0x00dc48, 0x00dc7b, 0x00dcae, 0x00dce1, 0x00dd14,
    0x00dd47, 0x00dd79, 0x00ddab, 0x00ddde, 0x00de10, 0x00de42, 0x00de74, 0x00dea5,
    0x00ded7, 0x00df08, 0x00df39, 0x00df6b, 0x00df9c, 0x00dfcd, 0x00dffd, 0x00e02e,
    0x00e05e, 0x00e08f, 0x00e0bf, 0x00e0ef, 0x00e11f, 0x00e14f, 0x00e17e, 0x00e1ae,
    0x00e1dd, 0x00e20d, 0x00e23c, 0x00e26b, 0x00e299, 0x00e2c8, 0x00e2f7, 0x00e325,
    0x00e353, 0x00e382, 0x00e3b0, 0x00e3de, 0x00e40b, 0x00e439, 0x00e466, 0x00e494,
    0x00e4c1, 0x00e4ee, 0x00e51b, 0x00e548, 0x00e574, 0x00e5a1, 0x00e5cd, 0x00e5f9,
    0x00e626, 0x00e652, 0x00e67d, 0x00e6a9, 0x00e6d5, 0x00e700, 0x00e72b, 0x00e756,
    0x00e781, 0x00e7ac, 0x00e7d7, 0x00e801, 0x00e82c, 0x00e856, 0x00e880, 0x00e8aa,
    0x00e8d4, 0x00e8fe, 0x00e927, 0x00e951, 0x00e97a, 0x00e9a3, 0x00e9cc, 0x00e9f5,
    0x00ea1e, 0x00ea47, 0x00ea6f, 0x00ea97, 0x00eac0, 0x00eae8, 0x00eb0f, 0x00eb37,
    0x00eb5f, 0x00eb86, 0x00ebae, 0x00ebd5, 0x00ebfc, 0x00ec23, 0x00ec4a, 0x00ec70,
    0x00ec97, 0x00ecbd, 0x00ece3, 0x00ed09, 0x00ed2f, 0x00ed55, 0x00ed7a, 0x00eda0,
    0x00edc5, 0x00edea, 0x00ee0f, 0x00ee34, 0x00ee59, 0x00ee7e, 0x00eea2, 0x00eec7,
    0x00eeeb, 0x00ef0f, 0x00ef33, 0x00ef56, 0x00ef7a, 0x00ef9d, 0x00efc1, 0x00efe4,
    0x00f007, 0x00f02a, 0x00f04d, 0x00f06f, 0x00f092, 0x00f0b4, 0x00f0d6, 0x00f0f8,
    0x00f11a, 0x00f13c, 0x00f15d, 0x00f17f, 0x00f1a0, 0x00f1c1, 0x00f1e2, 0x00f203,
    0x00f224, 0x00f244, 0x00f265, 0x00f285, 0x00f2a5, 0x00f2c5, 0x00f2e5, 0x00f304,
    0x00f324, 0x00f343, 0x00f363, 0x00f382, 0x00f3a1, 0x00f3c0, 0x00f3de, 0x00f3fd,
    0x00f41b, 0x00f439, 0x00f457, 0x00f475, 0x00f493, 0x00f4b1, 0x00f4ce, 0x00f4eb,
    0x00f509, 0x00f526, 0x00f543, 0x00f55f, 0x00f57c, 0x00f598, 0x00f5b5, 0x00f5d1,
    0x00f5ed, 0x00f609, 0x00f624, 0x00f640, 0x00f65b, 0x00f677, 0x00f692, 0x00f6ad,
    0x00f6c7, 0x00f6e2, 0x00f6fd, 0x00f717, 0x00f731, 0x00f74b, 0x00f765, 0x00f77f,
    0x00f799, 0x00f7b2, 0x00f7cb, 0x00f7e5, 0x00f7fe, 0x00f816, 0x00f82f, 0x00f848,
    0x00f860, 0x00f878, 0x00f891, 0x00f8a9, 0x00f8c0, 0x00f8d8, 0x00f8f0, 0x00f907,
    0x00f91e, 0x00f935, 0x00f94c, 0x00f963, 0x00f97a, 0x00f990, 0x00f9a6, 0x00f9bd,
    0x00f9d3, 0x00f9e8, 0x00f9fe, 0x00fa14, 0x00fa29, 0x00fa3e, 0x00fa54, 0x00fa69,
    0x00fa7d, 0x00fa92, 0x00faa7, 0x00fabb, 0x00facf, 0x00fae3, 0x00faf7, 0x00fb0b,
    0x00fb1f, 0x00fb32, 0x00fb45, 0x00fb58, 0x00fb6b, 0x00fb7e, 0x00fb91, 0x00fba4,
    0x00fbb6, 0x00fbc8, 0x00fbda, 0x00fbec, 0x00fbfe, 0x00fc10, 0x00fc21, 0x00fc33,
    0x00fc44, 0x00fc55, 0x00fc66, 0x00fc76, 0x00fc87, 0x00fc97, 0x00fca8, 0x00fcb8,
    0x00fcc8, 0x00fcd8, 0x00fce7, 0x00fcf7, 0x00fd06, 0x00fd15, 0x00fd24, 0x00fd33,
    0x00fd42, 0x00fd51, 0x00fd5f, 0x00fd6d, 0x00fd7c, 0x00fd89, 0x00fd97, 0x00fda5,
    0x00fdb3, 0x00fdc0, 0x00fdcd, 0x00fdda, 0x00fde7, 0x00fdf4, 0x00fe01, 0x00fe0d,
    0x00fe19, 0x00fe25, 0x00fe31, 0x00fe3d, 0x00fe49, 0x00fe55, 0x00fe60, 0x00fe6b,
    0x00fe76, 0x00fe81, 0x00fe8c, 0x00fe97, 0x00fea1, 0x00feab, 0x00feb5, 0x00febf,
    0x00fec9, 0x00fed3, 0x00fedd, 0x00fee6, 0x00feef, 0x00fef8, 0x00ff01, 0x00ff0a,
    0x00ff13, 0x00ff1b, 0x00ff23, 0x00ff2c, 0x00ff34, 0x00ff3b, 0x00ff43, 0x00ff4b,
    0x00ff52, 0x00ff59, 0x00ff60, 0x00ff67, 0x00ff6e, 0x00ff75, 0x00ff7b, 0x00ff82,
    0x00ff88, 0x00ff8e, 0x00ff94, 0x00ff99, 0x00ff9f, 0x00ffa4, 0x00ffa7, 0x00ffaf,
    0x00ffb4, 0x00ffb8, 0x00ffbd, 0x00ffc1, 0x00ffc6, 0x00ffca, 0x00ffce, 0x00ffd2,
    0x00ffd5, 0x00ffd9, 0x00ffdc, 0x00ffe0, 0x00ffe3, 0x00ffe6, 0x00ffe8, 0x00ffeb,
    0x00ffed, 0x00fff0, 0x00fff2, 0x00fff4, 0x00fff6, 0x00fff7, 0x00fff9, 0x00fffa,
    0x00fffc, 0x00fffd, 0x00fffe, 0x00fffe, 0x00ffff, 0x010000, 0x010000, 0x010000
};

GLfixed glfCosX(
     GLfixed Angle
     )
{
    GLfixed a;
    GLfixed result;
    gcmHEADER_ARG("Angle=0x%08x", Angle);

    /* Keep adding 2 * pi until angle is positive. */
    for (a = Angle; a < 0; a += glvFIXEDPITIMES2) ;

    /* Divide angle by 2 * pi. */
    a = gcmXDivide(a, glvFIXEDPITIMES2);

    /* Drop 4 bits of significance. */
    a >>= 4;

    /* Lookup cos value. */
    switch (a & 0xC00)
    {
    case 0x000:
        result = sinx_table[0x3FF - (a & 0x3FF)];
        break;

    case 0x400:
        result = -sinx_table[a & 0x3FF];
        break;

    case 0x800:
        result = -sinx_table[0x3FF - (a & 0x3FF)];
        break;

    default:
        result = sinx_table[a & 0x3FF];
        break;
    }
    gcmFOOTER_ARG("result=0x%08x", result);
    return result;
}


/*******************************************************************************
**
**  glfRSQX
**
**  Reciprocal square root in fixed point.
**
**  INPUT:
**
**      X
**          Input value.
**
**  OUTPUT:
**
**      1 / sqrt(X).
*/
GLfixed glfRSQX(
    GLfixed X
    )
{
    static const GLushort rsqrtx_table[8] =
    {
        0x6A0A, 0x5555, 0x43D1, 0x34BF, 0x279A, 0x1C02, 0x11AD, 0x0865
    };

    GLfixed r;
    int exp, i;

    gcmHEADER_ARG("X=0x%08x", X);

    gcmASSERT(X >= 0);

    if (X == gcvONE_X)
    {
        gcmFOOTER_ARG("0x%x", gcvONE_X);
        return gcvONE_X;
    }

    r   = X;
    exp = 31;

    if (r & 0xFFFF0000)
    {
        exp -= 16;
        r >>= 16;
    }

    if (r & 0xFF00)
    {
        exp -= 8;
        r >>= 8;
    }

    if (r & 0xF0)
    {
        exp -= 4;
        r >>= 4;
    }

    if (r & 0xC)
    {
        exp -= 2;
        r >>= 2;
    }

    if (r & 0x2)
    {
        exp -= 1;
    }

    if (exp > 28)
    {
        static const GLfixed low_value_result[] =
        {
            0x7FFFFFFF, 0x01000000, 0x00B504F3, 0x0093CD3A,
            0x00800000, 0x00727C97, 0x006882F5, 0x0060C247
        };

        gcmFOOTER_ARG("0x%x", low_value_result[X & 7]);
        return low_value_result[X & 7];
    }

    r    = gcvONE_X + rsqrtx_table[(X >> (28 - exp)) & 0x7];
    exp -= 16;

    if (exp <= 0)
    {
        r >>= -exp >> 1;
    }
    else
    {
        r <<= (exp >> 1) + (exp & 1);
    }

    if (exp & 1)
    {
        r = gcmXMultiply(r, rsqrtx_table[0]);
    }

    for (i = 0; i < 3; i++)
    {
        r = gcmXMultiply(
            (r >> 1),
            (3 << 16) - gcmXMultiply(gcmXMultiply(r, r), X)
            );
    }

    gcmFOOTER_ARG("0x%x", r);
    return r;
}

/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

/*******************************************************************************
**
**  _UpdateXXXFlags
**
**  _UpdateXXXFlags set of functions updates flags such as zero and one.
**
**  INPUT:
**
**      Variable
**          Variable to be updated.
**
**  OUTPUT:
**
**      Nothing.
*/

static void _UpdateMutantFlags(
    glsMUTANT_PTR Variable
    )
{
    gcmHEADER_ARG("Variable=0x%x", Variable);

    Variable->zero = (Variable->value.i == 0);

    switch (Variable->type)
    {
    case glvINT:
        Variable->one = (Variable->value.i == 1);
        break;

    case glvFIXED:
        Variable->one = (Variable->value.x == gcvONE_X);
        break;

    case glvFLOAT:
        Variable->one = (Variable->value.f == 1.0f);
        break;

    default:
        gcmFATAL("_UpdateMutantFlags: invalid type %d", Variable->type);
    }
    gcmFOOTER_NO();
}

static void _UpdateVectorFlags(
    glsVECTOR_PTR Variable
    )
{
    gcmHEADER_ARG("Variable=0x%x", Variable);

    Variable->zero3
        = ((Variable->value[0].i == 0)
        && (Variable->value[1].i == 0)
        && (Variable->value[2].i == 0));

    Variable->zero4
        = ((Variable->value[3].i == 0)
        && (Variable->zero3));

    switch (Variable->type)
    {
    case glvINT:
        Variable->one3
            = ((Variable->value[0].i == 1)
            && (Variable->value[1].i == 1)
            && (Variable->value[2].i == 1));

        Variable->one4
            = ((Variable->value[3].i == 1)
            && (Variable->one3));
        break;

    case glvFIXED:
        Variable->one3
            = ((Variable->value[0].x == gcvONE_X)
            && (Variable->value[1].x == gcvONE_X)
            && (Variable->value[2].x == gcvONE_X));

        Variable->one4
            = ((Variable->value[3].x == gcvONE_X)
            && (Variable->one3));
        break;

    case glvFLOAT:
        Variable->one3
            = ((Variable->value[0].f == 1.0f)
            && (Variable->value[1].f == 1.0f)
            && (Variable->value[2].f == 1.0f));

        Variable->one4
            = ((Variable->value[3].f == 1.0f)
            && (Variable->one3));
        break;

    default:
        gcmFATAL("_UpdateVectorFlags: invalid type %d", Variable->type);
    }

    gcmFOOTER_NO();
}


/******************************************************************************\
***************** Value Conversion From/To OpenGL Enumerations *****************
\******************************************************************************/

/*******************************************************************************
**
**  glfConvertGLEnum
**
**  Converts OpenGL GL_xxx constants into intrernal enumerations.
**
**  INPUT:
**
**      Names
**      NameCount
**          Pointer and a count of the GL enumaration array.
**
**      Value
**      Type
**          GL enumeration value to be converted.
**
**  OUTPUT:
**
**      Result
**          Internal value corresponding to the GL enumeration.
*/

GLboolean glfConvertGLEnum(
    const GLenum* Names,
    GLint NameCount,
    const GLvoid* Value,
    gleTYPE Type,
    GLuint* Result
    )
{
    /* Convert the enum. */
    GLenum value = (Type == glvFLOAT)
        ? glmFLOAT2INT(* (GLfloat *) Value)
        :             (* (GLint   *) Value);

    /* Search the map for it. */
    GLint i;

    gcmHEADER_ARG("Names=0x%x NameCount=%d Value=0x%x Type=0x%04x Result=0x%x",
                    Names, NameCount, Value, Type, Result);

    for (i = 0; i < NameCount; i++)
    {
        if (Names[i] == value)
        {
            *Result = i;
            gcmFOOTER_ARG("return=%d", GL_TRUE);
            return GL_TRUE;
        }
    }

    gcmFOOTER_ARG("return=%d", GL_FALSE);

    /* Bad enum. */
    return GL_FALSE;
}

/*******************************************************************************
**
**  glfConvertGLboolean
**
**  Validates OpenGL GLboolean constants.
**
**  INPUT:
**
**      Value
**      Type
**          GL enumeration value to be converted.
**
**  OUTPUT:
**
**      Result
**          Internal value corresponding to the GL enumeration.
*/

GLboolean glfConvertGLboolean(
    const GLvoid* Value,
    gleTYPE Type,
    GLuint* Result
    )
{
    GLboolean result;
    static GLenum _BooleanNames[] =
    {
        GL_FALSE,
        GL_TRUE
    };

    gcmHEADER_ARG("Value=0x%x Type=0x%04x Result=0x%x", Value, Type, Result);

    result = glfConvertGLEnum(
        _BooleanNames,
        gcmCOUNTOF(_BooleanNames),
        Value,
        Type,
        Result
        );

    gcmFOOTER_ARG("return=%d", result);

    return result;
}


/******************************************************************************\
***************************** Value Type Converters ****************************
\******************************************************************************/

void glfGetFromBool(
    GLboolean Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=%d Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromBoolArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromBoolArray(
    const GLboolean* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;
    gcmHEADER_ARG("Variable=%d Value=0x%x Type=0x%04x", Variables, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = Variables[i];
            break;

        case glvINT:
            ((GLint *) Value) [i] = glmBOOL2INT(Variables[i]);
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = glmBOOL2FIXED(Variables[i]);
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = glmBOOL2FLOAT(Variables[i]);
            break;

        default:
            gcmFATAL("glfGetFromIntArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromInt(
    GLint Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=%d Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromIntArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromIntArray(
    const GLint* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;

    gcmHEADER_ARG("Variables=0x%x Count=%d Value=0x%x Type=0x%04x", Variables, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = glmINT2BOOL(Variables[i]);
            break;

        case glvINT:
            ((GLint *) Value) [i] = Variables[i];
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = glmINT2FIXED(Variables[i]);
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = glmINT2FLOAT(Variables[i]);
            break;

        default:
            gcmFATAL("glfGetFromIntArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromFixed(
    GLfixed Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=%d Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromFixedArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromFixedArray(
    const GLfixed* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;

    gcmHEADER_ARG("Variables=0x%x Count=%d Value=0x%x Type=0x%04x", Variables, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = glmFIXED2BOOL(Variables[i]);
            break;

        case glvINT:
            ((GLint *) Value) [i] = glmFIXED2INT(Variables[i]);
            break;

        case glvNORM:
            ((GLint *) Value) [i] = glmFIXED2NORM(Variables[i]);
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = Variables[i];
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = glmFIXED2FLOAT(Variables[i]);
            break;

        default:
            gcmFATAL("glfGetFromFixedArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromFloat(
    GLfloat Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=%f Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromFloatArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromFloatArray(
    const GLfloat* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;
    gcmHEADER_ARG("Variables=%f Count=%d Value=0x%x Type=0x%04x", Variables, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = glmFLOAT2BOOL(Variables[i]);
            break;

        case glvINT:
            ((GLint *) Value) [i] = glmFLOAT2INT(Variables[i]);
            break;

        case glvNORM:
            ((GLint *) Value) [i] = glmFLOAT2NORM(Variables[i]);
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = glmFLOAT2FIXED(Variables[i]);
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = Variables[i];
            break;

        default:
            gcmFATAL("glfGetFromFloatArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromEnum(
    GLenum Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=0x%04x Value=0x%x Type=0x%04x", Variable, Value, Type);
    glfGetFromEnumArray(&Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromEnumArray(
    const GLenum* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;

    gcmHEADER_ARG("Variables=0x%x Count=%d Value=0x%x Type=0x%04x", Variables, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            ((GLboolean *) Value) [i] = glmINT2BOOL(Variables[i]);
            break;

        case glvINT:
            ((GLint *) Value) [i] = Variables[i];
            break;

        case glvFIXED:
            ((GLfixed *) Value) [i] = Variables[i];
            break;

        case glvFLOAT:
            ((GLfloat *) Value) [i] = glmINT2FLOAT(Variables[i]);
            break;

        default:
            gcmFATAL("glfGetFromEnumArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromMutable(
    gluMUTABLE Variable,
    gleTYPE VariableType,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=0x%x VariableType=0x%04x Value=0x%x Type=0x%04x",
                    Variable, VariableType, Value, Type);
    glfGetFromMutableArray(&Variable, VariableType, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromMutableArray(
    const gluMUTABLE_PTR Variables,
    gleTYPE VariableType,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;
    gcmHEADER_ARG("Variables=0x%x VariableType=0x%04x Count=%d Value=0x%x Type=0x%04x",
                    Variables, VariableType, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        GLvoid* value = gcvNULL;

        switch (Type)
        {
        case glvBOOL:
            value = & ((GLboolean *) Value) [i];
            break;

        case glvINT:
        case glvNORM:
            value = & ((GLint *) Value) [i];
            break;

        case glvFIXED:
            value = & ((GLfixed *) Value) [i];
            break;

        case glvFLOAT:
            value = & ((GLfloat *) Value) [i];
            break;

        default:
            gcmFATAL("glfGetFromMutableArray: invalid type %d", Type);
        }

        switch (VariableType)
        {
        case glvINT:
            glfGetFromInt(Variables[i].i, value, Type);
            break;

        case glvFIXED:
            glfGetFromFixed(Variables[i].x, value, Type);
            break;

        case glvFLOAT:
            glfGetFromFloat(Variables[i].f, value, Type);
            break;

        default:
            gcmFATAL("glfGetFromMutableArray: invalid source type %d", VariableType);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromMutant(
    const glsMUTANT_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x",
                    Variable, Value, Type);
    glfGetFromMutantArray(Variable, 1, Value, Type);
    gcmFOOTER_NO();
}

void glfGetFromMutantArray(
    const glsMUTANT_PTR Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;
    gcmHEADER_ARG("Variables=0x%x Count=%d Value=0x%x Type=0x%04x",
                    Variables, Count, Value, Type);

    for (i = 0; i < Count; i++)
    {
        switch (Type)
        {
        case glvBOOL:
            glfGetFromMutable(
                Variables[i].value,
                Variables[i].type,
                & ((GLboolean *) Value) [i],
                Type
                );
            break;

        case glvINT:
        case glvNORM:
            glfGetFromMutable(
                Variables[i].value,
                Variables[i].type,
                & ((GLint *) Value) [i],
                Type
                );
            break;

        case glvFIXED:
            glfGetFromMutable(
                Variables[i].value,
                Variables[i].type,
                & ((GLfixed *) Value) [i],
                Type
                );
            break;

        case glvFLOAT:
            glfGetFromMutable(
                Variables[i].value,
                Variables[i].type,
                & ((GLfloat *) Value) [i],
                Type
                );
            break;

        default:
            gcmFATAL("glfGetFromMutableArray: invalid type %d", Type);
        }
    }
    gcmFOOTER_NO();
}

void glfGetFromVector3(
    const glsVECTOR_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x",
                    Variable, Value, Type);

    switch (Type)
    {
    case glvBOOL:
        for (i = 0; i < 3; i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLboolean *) Value) [i],
                Type
                );
        }
        break;

    case glvINT:
        for (i = 0; i < 3; i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLint *) Value) [i],
                Type
                );
        }
        break;

    case glvFIXED:
        for (i = 0; i < 3; i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLfixed *) Value) [i],
                Type
                );
        }
        break;

    case glvFLOAT:
        for (i = 0; i < 3; i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLfloat *) Value) [i],
                Type
                );
        }
        break;

    default:
        gcmFATAL("glfGetFromVector3: invalid type %d", Type);
    }
    gcmFOOTER_NO();
}

void glfGetFromVector4(
    const glsVECTOR_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x",
                    Variable, Value, Type);

    switch (Type)
    {
    case glvBOOL:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLboolean *) Value) [i],
                Type
                );
        }
        break;

    case glvINT:
    case glvNORM:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLint *) Value) [i],
                Type
                );
        }
        break;

    case glvFIXED:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLfixed *) Value) [i],
                Type
                );
        }
        break;

    case glvFLOAT:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLfloat *) Value) [i],
                Type
                );
        }
        break;

    default:
        gcmFATAL("glfGetFromVector4: invalid type %d", Type);
    }
    gcmFOOTER_NO();
}

void glfGetFromMatrix(
    const glsMATRIX_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x",
                    Variable, Value, Type);

    switch (Type)
    {
    case glvBOOL:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLboolean *) Value) [i],
                Type
                );
        }
        break;

    case glvINT:
    case glvNORM:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLint *) Value) [i],
                Type
                );
        }
        break;

    case glvFIXED:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLfixed *) Value) [i],
                Type
                );
        }
        break;

    case glvFLOAT:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            glfGetFromMutable(
                Variable->value[i],
                Variable->type,
                & ((GLfloat *) Value) [i],
                Type
                );
        }
        break;

    default:
        gcmFATAL("glfGetFromMatrix: invalid type %d", Type);
    }
    gcmFOOTER_NO();
}


/******************************************************************************\
**************************** Get Values From Raw Input *************************
\******************************************************************************/

GLfixed glfFixedFromRaw(
    const GLvoid* Variable,
    gleTYPE Type
    )
{
    GLfixed result=0;
    gcmHEADER_ARG("Variable=0x%x Type=0x%04x", Variable, Type);
    switch (Type)
    {
    case glvINT:
        result = glmINT2FIXED(* (GLint *) Variable);
        break;

    case glvFIXED:
        result = * (GLfixed *) Variable;
        break;

    case glvFLOAT:
        result = glmFLOAT2FIXED(* (GLfloat *) Variable);
        break;

    default:
        gcmFATAL("glfFixedFromRaw: invalid type %d", Type);
        break;
    }
    gcmFOOTER_ARG("return=0x%d08x", result);
    return result;
}

GLfloat glfFloatFromRaw(
    const GLvoid* Variable,
    gleTYPE Type
    )
{
    GLfloat result = 0.0;
    gcmHEADER_ARG("Variable=0x%x Type=0x%04x", Variable, Type);
    switch (Type)
    {
    case glvINT:
        result = glmINT2FLOAT(* (GLint *) Variable);
        break;
    case glvFIXED:
        result = glmFIXED2FLOAT(* (GLfixed *) Variable);
        break;
    case glvFLOAT:
        result = * (GLfloat *) Variable;
        break;

    default:
        gcmFATAL("glfFloatFromRaw: invalid type %d", Type);
        break;
    }
    gcmFOOTER_ARG("return=%f", result);
    return result;
}


/******************************************************************************\
**************************** Get Values From Mutables **************************
\******************************************************************************/

GLboolean glfBoolFromMutable(
    gluMUTABLE Variable,
    gleTYPE Type
    )
{
    GLboolean result;
    gcmHEADER_ARG("Variable=0x%x Type=0x%04x", Variable, Type);
    glfGetFromMutable(Variable, Type, &result, glvBOOL);
    gcmFOOTER_ARG("return=%d", result);
    return result;
}

GLint glfIntFromMutable(
    gluMUTABLE Variable,
    gleTYPE Type
    )
{
    GLint result;
    gcmHEADER_ARG("Variable=0x%x Type=0x%04x", Variable, Type);
    glfGetFromMutable(Variable, Type, &result, glvINT);
    gcmFOOTER_ARG("return=%d", result);
    return result;
}

GLfixed glfFixedFromMutable(
    gluMUTABLE Variable,
    gleTYPE Type
    )
{
    GLfixed result;
    gcmHEADER_ARG("Variable=0x%x Type=0x%04x", Variable, Type);
    glfGetFromMutable(Variable, Type, &result, glvFIXED);
    gcmFOOTER_ARG("return=0x%08x", result);
    return result;
}

GLfloat glfFloatFromMutable(
    gluMUTABLE Variable,
    gleTYPE Type
    )
{
    GLfloat result;
    gcmHEADER_ARG("Variable=0x%x Type=0x%04x", Variable, Type);
    glfGetFromMutable(Variable, Type, &result, glvFLOAT);
    gcmFOOTER_ARG("return=%f", result);
    return result;
}


/******************************************************************************\
**************************** Get Values From Mutants ***************************
\******************************************************************************/

GLboolean glfBoolFromMutant(
    const glsMUTANT_PTR Variable
    )
{
    GLboolean result;
    gcmHEADER_ARG("Variable=0x%x", Variable);
    glfGetFromMutant(Variable, &result, glvBOOL);
    gcmFOOTER_ARG("return=%d", result);
    return result;
}

GLint glfIntFromMutant(
    const glsMUTANT_PTR Variable
    )
{
    GLint result;
    gcmHEADER_ARG("Variable=0x%x", Variable);
    glfGetFromMutant(Variable, &result, glvINT);
    gcmFOOTER_ARG("return=%d", result);
    return result;
}

GLfixed glfFixedFromMutant(
    const glsMUTANT_PTR Variable
    )
{
    GLfixed result;
    gcmHEADER_ARG("Variable=0x%x", Variable);
    glfGetFromMutant(Variable, &result, glvFIXED);
    gcmFOOTER_ARG("return=0x%08x", result);
    return result;
}

GLfloat glfFloatFromMutant(
    const glsMUTANT_PTR Variable
    )
{
    GLfloat result;
    gcmHEADER_ARG("Variable=0x%x", Variable);
    glfGetFromMutant(Variable, &result, glvFLOAT);
    gcmFOOTER_ARG("return=%f", result);
    return result;
}


/******************************************************************************\
***************************** Set Values To Mutants ****************************
\******************************************************************************/

void glfSetIntMutant(
    glsMUTANT_PTR Variable,
    GLint Value
    )
{
    gcmHEADER_ARG("Variable=0x%x Value=%d", Variable, Value);
    /* Set the type. */
    Variable->type = glvINT;

    /* Set the value. */
    Variable->value.i = Value;

    /* Update special value flags. */
    _UpdateMutantFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetFixedMutant(
    glsMUTANT_PTR Variable,
    GLfixed Value
    )
{
    gcmHEADER_ARG("Variable=0x%x Value=0x%08x", Variable, Value);
    /* Set the type. */
    Variable->type = glvFIXED;

    /* Set the value. */
    Variable->value.x = Value;

    /* Update special value flags. */
    _UpdateMutantFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetFloatMutant(
    glsMUTANT_PTR Variable,
    GLfloat Value
    )
{
    gcmHEADER_ARG("Variable=0x%x Value=%f", Variable, Value);
    /* Set the type. */
    Variable->type = glvFLOAT;

    /* Set the value. */
    Variable->value.f = Value;

    /* Update special value flags. */
    _UpdateMutantFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetMutant(
    glsMUTANT_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x", Variable, Value, Type);
    /* Set the type. */
    Variable->type = Type;

    /* Set the value. */
    switch (Type)
    {
    case glvINT:
        Variable->value.i = * (GLint *) Value;
        break;

    case glvFIXED:
        Variable->value.x = * (GLfixed *) Value;
        break;

    case glvFLOAT:
        Variable->value.f = * (GLfloat *) Value;
        break;

    default:
        gcmFATAL("glfSetMutant: invalid type %d", Type);
    }

    /* Update special value flags. */
    _UpdateMutantFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetClampedMutant(
    glsMUTANT_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x", Variable, Value, Type);
    /* Set the type. */
    Variable->type = Type;

    /* Set the value. */
    switch (Type)
    {
    case glvFIXED:
        Variable->value.x = glmFIXEDCLAMP_0_TO_1(* (GLfixed *) Value);
        break;

    case glvFLOAT:
        Variable->value.f = glmFLOATCLAMP_0_TO_1(* (GLfloat *) Value);
        break;

    default:
        gcmFATAL("glfSetClampedMutant: invalid type %d", Type);
    }

    /* Update special value flags. */
    _UpdateMutantFlags(Variable);

    gcmFOOTER_NO();
}


/******************************************************************************\
***************************** Set Values To Vectors ****************************
\******************************************************************************/

void glfSetIntVector4(
    glsVECTOR_PTR Variable,
    GLint X,
    GLint Y,
    GLint Z,
    GLint W
    )
{
    gcmHEADER_ARG("Variable=0x%x X=%d Y=%d Z=%d W=%d", Variable, X, Y, Z, W);
    /* Set the type. */
    Variable->type = glvINT;

    /* Set components. */
    Variable->value[0].i = X;
    Variable->value[1].i = Y;
    Variable->value[2].i = Z;
    Variable->value[3].i = W;

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);
    gcmFOOTER_NO();
}

void glfSetFixedVector4(
    glsVECTOR_PTR Variable,
    GLfixed X,
    GLfixed Y,
    GLfixed Z,
    GLfixed W
    )
{
    gcmHEADER_ARG("Variable=0x%x X=0x%08x Y=0x%08x Z=0x%08x W=0x%08x", Variable, X, Y, Z, W);
    /* Set the type. */
    Variable->type = glvFIXED;

    /* Set components. */
    Variable->value[0].x = X;
    Variable->value[1].x = Y;
    Variable->value[2].x = Z;
    Variable->value[3].x = W;

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);
    gcmFOOTER_NO();
}

void glfSetFloatVector4(
    glsVECTOR_PTR Variable,
    GLfloat X,
    GLfloat Y,
    GLfloat Z,
    GLfloat W
    )
{
    gcmHEADER_ARG("Variable=0x%x X=%f Y=%f Z=%f W=%f", Variable, X, Y, Z, W);
    /* Set the type. */
    Variable->type = glvFLOAT;

    /* Set components. */
    Variable->value[0].f = X;
    Variable->value[1].f = Y;
    Variable->value[2].f = Z;
    Variable->value[3].f = W;

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);
    gcmFOOTER_NO();
}

void glfSetVector3(
    glsVECTOR_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLint i;

    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x", Variable, Value, Type);

    /* Set the type. */
    Variable->type = Type;

    /* Set the value. */
    switch (Type)
    {
    case glvINT:
        for (i = 0; i < 3; i++)
        {
            Variable->value[i].i = ((GLint *) Value) [i];
        }
        break;

    case glvFIXED:
        for (i = 0; i < 3; i++)
        {
            Variable->value[i].x = ((GLfixed *) Value) [i];
        }
        break;

    case glvFLOAT:
        for (i = 0; i < 3; i++)
        {
            Variable->value[i].f = ((GLfloat *) Value) [i];
        }
        break;

    default:
        gcmFATAL("glfSetVector3: invalid type %d", Type);
    }

    /* Set the fourth component to zero. */
    Variable->value[3].i = 0;

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetVector4(
    glsVECTOR_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x", Variable, Value, Type);

    /* Set the type. */
    Variable->type = Type;

    /* Set the value. */
    switch (Type)
    {
    case glvINT:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            Variable->value[i].i = ((GLint *) Value) [i];
        }
        break;

    case glvFIXED:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            Variable->value[i].x = ((GLfixed *) Value) [i];
        }
        break;

    case glvFLOAT:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            Variable->value[i].f = ((GLfloat *) Value) [i];
        }
        break;

    default:
        gcmFATAL("glfSetVector4: invalid type %d", Type);
    }

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetClampedVector4(
    glsVECTOR_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLuint i;
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x", Variable, Value, Type);

    /* Set the type. */
    Variable->type = Type;

    /* Set the value. */
    switch (Type)
    {
    case glvFIXED:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            Variable->value[i].x
                = glmFIXEDCLAMP_0_TO_1(((GLfixed *) Value) [i]);
        }
        break;

    case glvFLOAT:
        for (i = 0; i < gcmCOUNTOF(Variable->value); i++)
        {
            Variable->value[i].f
                = glmFLOATCLAMP_0_TO_1(((GLfloat *) Value) [i]);
        }
        break;

    default:
        gcmFATAL("glfSetVector4: invalid type %d", Type);
    }

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

void glfSetHomogeneousVector4(
    glsVECTOR_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Variable=0x%x Value=0x%x Type=0x%04x", Variable, Value, Type);
    /* Set the type. */
    Variable->type = Type;

    /* Set the value. */
    switch (Type)
    {
    case glvINT:
        {
            GLint* value = ((GLint *) Value);

            if ((value[3] != 0) && (value[3] != 1))
            {
                Variable->value[0].i = gcoMATH_DivideInt(value[0], value[3]);
                Variable->value[1].i = gcoMATH_DivideInt(value[1], value[3]);
                Variable->value[2].i = gcoMATH_DivideInt(value[2], value[3]);
                Variable->value[3].i = 1;
            }
            else
            {
                Variable->value[0].i = value[0];
                Variable->value[1].i = value[1];
                Variable->value[2].i = value[2];
                Variable->value[3].i = value[3];
            }
        }
        break;

    case glvFIXED:
        {
            GLfixed* value = ((GLfixed *) Value);

            if ((value[3] != gcvZERO_X) && (value[3] != gcvONE_X))
            {
                Variable->value[0].x = gcmXDivide(value[0], value[3]);
                Variable->value[1].x = gcmXDivide(value[1], value[3]);
                Variable->value[2].x = gcmXDivide(value[2], value[3]);
                Variable->value[3].x = gcvONE_X;
            }
            else
            {
                Variable->value[0].x = value[0];
                Variable->value[1].x = value[1];
                Variable->value[2].x = value[2];
                Variable->value[3].x = value[3];
            }
        }
        break;

    case glvFLOAT:
        {
            GLfloat* value = ((GLfloat *) Value);

            if ((value[3] != 0.0f) && (value[3] != 1.0f))
            {
                Variable->value[0].f = gcoMATH_Divide(value[0], value[3]);
                Variable->value[1].f = gcoMATH_Divide(value[1], value[3]);
                Variable->value[2].f = gcoMATH_Divide(value[2], value[3]);
                Variable->value[3].f = 1.0f;
            }
            else
            {
                Variable->value[0].f = value[0];
                Variable->value[1].f = value[1];
                Variable->value[2].f = value[2];
                Variable->value[3].f = value[3];
            }
        }
        break;

    default:
        gcmFATAL("glfSetHomogeneousVector4: invalid type %d", Type);
    }

    /* Update special value flags. */
    _UpdateVectorFlags(Variable);

    gcmFOOTER_NO();
}

/* Normalize vector3 to vector4.                */
/* Using float to do intermedia calculation.    */
void glfNorm3Vector4f(
    const GLfloat * Vector,
    GLfloat * Result
    )
{
    GLfloat squareSum, norm;
    gcmHEADER_ARG("Vector=0x%x Result=0x%x", Vector, Result);

    /* Compute normal. */
    squareSum
        = Vector[0] * Vector[0]
          + Vector[1] * Vector[1]
          + Vector[2] * Vector[2];

    norm = (squareSum < 0)
            ? squareSum
            : gcoMATH_Divide(1.0f, __GL_SQRTF(squareSum));

    /* Multiply vector by normal. */
    Result[0] = Vector[0] * norm;
    Result[1] = Vector[1] * norm;
    Result[2] = Vector[2] * norm;
    Result[3] = 0.0f;

    gcmFOOTER_NO();
}
