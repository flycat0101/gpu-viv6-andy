/* cl_channel_type */
#define CL_SNORM_INT8                               0x10D0
#define CL_SNORM_INT16                              0x10D1
#define CL_UNORM_INT8                               0x10D2
#define CL_UNORM_INT16                              0x10D3
#define CL_UNORM_SHORT_565                          0x10D4
#define CL_UNORM_SHORT_555                          0x10D5
#define CL_UNORM_INT_101010                         0x10D6
#define CL_SIGNED_INT8                              0x10D7
#define CL_SIGNED_INT16                             0x10D8
#define CL_SIGNED_INT32                             0x10D9
#define CL_UNSIGNED_INT8                            0x10DA
#define CL_UNSIGNED_INT16                           0x10DB
#define CL_UNSIGNED_INT32                           0x10DC
#define CL_HALF_FLOAT                               0x10DD
#define CL_FLOAT                                    0x10DE

/* cl_channel_order */
#define CL_R                                        0x10B0
#define CL_A                                        0x10B1
#define CL_RG                                       0x10B2
#define CL_RA                                       0x10B3
#define CL_RGB                                      0x10B4
#define CL_RGBA                                     0x10B5
#define CL_BGRA                                     0x10B6
#define CL_ARGB                                     0x10B7
#define CL_INTENSITY                                0x10B8
#define CL_LUMINANCE                                0x10B9
#define CL_Rx                                       0x10BA
#define CL_RGx                                      0x10BB
#define CL_RGBx                                     0x10BC
/* cl_addressing_mode */
#define CL_ADDRESS_NONE                             0x1130
#define CL_ADDRESS_CLAMP_TO_EDGE                    0x1131
#define CL_ADDRESS_CLAMP                            0x1132
#define CL_ADDRESS_REPEAT                           0x1133
#define CL_ADDRESS_MIRRORED_REPEAT                  0x1134

int addressing_mode(int inputValue, int dimension, int addressingMode){

	if(addressingMode ==2){//CL_ADDRESS_CLAMP
		return viv_clamp(inputValue, -1, dimension);
	}
	else	{ // CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE or undefined
		return viv_clamp(inputValue, 0, dimension -1); //just return the input
	}
}

char all__c(
	char src0
	)
{
	if((char)(-128)&src0){
		return (char)1;
	}
	return 0;
}


char all__Dv16_c(
	char16 src0

	)
{
	char temp = (char)(-128);	//create mask for MSB
	src0.s01234567=(src0.s01234567&src0.s789abcef);	//D&C to apply 'or'
	src0.s0123 = (src0.s0123&src0.s4567);
	src0.s01 = (src0.s01&src0.s23);
	src0.s0=(src0.s0&src0.s1)&temp;  //apply mask and check
	if(src0.s0){
		return (char)1;
	}
	return 0;
}


char all__Dv2_c(
	char2 src0
	)
{
	char temp = (char)(-128);
	if((src0.s0&src0.s1)&temp){
		return (char)1;
	}
	return 0;
}


char all__Dv3_c(
	char3 src0
	)
{
	char temp = (char)(-128);
	src0.s0=(src0.s0&src0.s1&src0.s2)&temp;
	if(src0.s0){
		return (char)1;
	}
	return 0;
}


char all__Dv4_c(
	char4 src0
	)
{
	char temp = (char)(-128);
	src0.s01=(src0.s01&src0.s23);
	src0.s0=(src0.s0&src0.s1)&temp;
	if(src0.s0){
		return (char)1;
	}
	return 0;
}


char all__Dv8_c(
	char8 src0
	)
{
	char temp = (char)(-128);
	src0.s0123=(src0.s0123&src0.s4567);
	src0.s01 = (src0.s01&src0.s23);
	src0.s0=(src0.s0&src0.s1)&temp;
	if(src0.s0){
		return (char)1;
	}
	return 0;
}


int all__i(
	int src0
	)
{
	if((int)(-2147483648)&src0){
		return (int)1;
	}
	return 0;
}


int all__Dv16_i(
	int16 src0

	)
{
	int temp = (int)(-2147483648);	//create mask for MSB
	src0.s01234567=(src0.s01234567&src0.s789abcef);	//D&C to apply 'or'
	src0.s0123 = (src0.s0123&src0.s4567);
	src0.s01 = (src0.s01&src0.s23);
	src0.s0=(src0.s0&src0.s1)&temp;  //apply mask and check
	if(src0.s0){
		return (int)1;
	}
	return 0;
}


int all__Dv2_i(
	int2 src0
	)
{
	int temp = (int)(-2147483648);
	if((src0.s0&src0.s1)&temp){
		return (int)1;
	}
	return 0;
}


int all__Dv3_i(
	int3 src0
	)
{
	int temp = (int)(-2147483648);
	src0.s0=(src0.s0&src0.s1&src0.s2)&temp;
	if(src0.s0){
		return (int)1;
	}
	return 0;
}


int all__Dv4_i(
	int4 src0
	)
{
	int temp = (int)(-2147483648);
	src0.s01=(src0.s01&src0.s23);
	src0.s0=(src0.s0&src0.s1)&temp;
	if(src0.s0){
		return (int)1;
	}
	return 0;
}


int all__Dv8_i(
	int8 src0
	)
{
	int temp = (int)(-2147483648);
	src0.s0123=(src0.s0123&src0.s4567);
	src0.s01 = (src0.s01&src0.s23);
	src0.s0=(src0.s0&src0.s1)&temp;
	if(src0.s0){
		return (int)1;
	}
	return 0;
}


short all__s(
	short src0
	)
{
	if((short)(-32768)&src0){
		return (short)1;
	}
	return 0;
}


short all__Dv16_s(
	short16 src0

	)
{
	short temp = (short)(-32768);	//create mask for MSB
	src0.s01234567=(src0.s01234567&src0.s789abcef);	//D&C to apply 'or'
	src0.s0123 = (src0.s0123&src0.s4567);
	src0.s01 = (src0.s01&src0.s23);
	src0.s0=(src0.s0&src0.s1)&temp;  //apply mask and check
	if(src0.s0){
		return (short)1;
	}
	return 0;
}


short all__Dv2_s(
	short2 src0
	)
{
	short temp = (short)(-32768);
	if((src0.s0&src0.s1)&temp){
		return (short)1;
	}
	return 0;
}


short all__Dv3_s(
	short3 src0
	)
{
	short temp = (short)(-32768);
	src0.s0=(src0.s0&src0.s1&src0.s2)&temp;
	if(src0.s0){
		return (short)1;
	}
	return 0;
}


short all__Dv4_s(
	short4 src0
	)
{
	short temp = (short)(-32768);
	src0.s01=(src0.s01&src0.s23);
	src0.s0=(src0.s0&src0.s1)&temp;
	if(src0.s0){
		return (short)1;
	}
	return 0;
}


short all__Dv8_s(
	short8 src0

	)
{
	short temp = (short)(-32768);
	src0.s0123=(src0.s0123&src0.s4567);
	src0.s01 = (src0.s01&src0.s23);
	src0.s0=(src0.s0&src0.s1)&temp;
	if(src0.s0){
		return (short)1;
	}
	return 0;
}


char any__c(
	char src0
	)
{
	if((char)(-128)&src0){
		return (char)1;
	}
	return 0;
}


char any__Dv16_c(
	char16 src0

	)
{
	char temp = (char)(-128);	//create mask for MSB
	src0.s01234567=(src0.s01234567|src0.s789abcef);	//D&C to apply 'or'
	src0.s0123 = (src0.s0123|src0.s4567);
	src0.s01 = (src0.s01|src0.s23);
	src0.s0=(src0.s0|src0.s1)&temp;  //apply mask and check
	if(src0.s0){
		return (char)1;
	}
	return 0;
}


char any__Dv2_c(
	char2 src0
	)
{
	char temp = (char)(-128);
	if((src0.s0|src0.s1)&temp){
		return (char)1;
	}
	return 0;
}


char any__Dv3_c(
	char3 src0
	)
{
	char temp = (char)(-128);
	src0.s0=(src0.s0|src0.s1|src0.s2)&temp;
	if(src0.s0){
		return (char)1;
	}
	return 0;
}


char any__Dv4_c(
	char4 src0
	)
{
	char temp = (char)(-128);
	src0.s01=(src0.s01|src0.s23);
	src0.s0=(src0.s0|src0.s1)&temp;
	if(src0.s0){
		return (char)1;
	}
	return 0;
}


char any__Dv8_c(
	char8 src0
	)
{
	char temp = (char)(-128);
	src0.s0123=(src0.s0123|src0.s4567);
	src0.s01 = (src0.s01|src0.s23);
	src0.s0=(src0.s0|src0.s1)&temp;
	if(src0.s0){
		return (char)1;
	}
	return 0;
}


int any__i(
	int src0
	)
{
	if((int)(-2147483648)&src0){
		return (int)1;
	}
	return 0;
}


int any__Dv16_i(
	int16 src0

	)
{
	int temp = (int)(-2147483648);	//create mask for MSB
	src0.s01234567=(src0.s01234567|src0.s789abcef);	//D&C to apply 'or'
	src0.s0123 = (src0.s0123|src0.s4567);
	src0.s01 = (src0.s01|src0.s23);
	src0.s0=(src0.s0|src0.s1)&temp;  //apply mask and check
	if(src0.s0){
		return (int)1;
	}
	return 0;
}


int any__Dv2_i(
	int2 src0
	)
{
	int temp = (int)(-2147483648);
	if((src0.s0|src0.s1)&temp){
		return (int)1;
	}
	return 0;
}


int any__Dv3_i(
	int3 src0
	)
{
	int temp = (int)(-2147483648);
	src0.s0=(src0.s0|src0.s1|src0.s2)&temp;
	if(src0.s0){
		return (int)1;
	}
	return 0;
}


int any__Dv4_i(
	int4 src0
	)
{
	int temp = (int)(-2147483648);
	src0.s01=(src0.s01|src0.s23);
	src0.s0=(src0.s0|src0.s1)&temp;
	if(src0.s0){
		return (int)1;
	}
	return 0;
}


int any__Dv8_i(
	int8 src0
	)
{
	int temp = (int)(-2147483648);
	src0.s0123=(src0.s0123|src0.s4567);
	src0.s01 = (src0.s01|src0.s23);
	src0.s0=(src0.s0|src0.s1)&temp;
	if(src0.s0){
		return (int)1;
	}
	return 0;
}


short any__s(
	short src0
	)
{
	if((short)(-32768)&src0){
		return (short)1;
	}
	return 0;
}


short any__Dv16_s(
	short16 src0

	)
{
	short temp = (short)(-32768);	//create mask for MSB
	src0.s01234567=(src0.s01234567|src0.s789abcef);	//D&C to apply 'or'
	src0.s0123 = (src0.s0123|src0.s4567);
	src0.s01 = (src0.s01|src0.s23);
	src0.s0=(src0.s0|src0.s1)&temp;  //apply mask and check
	if(src0.s0){
		return (short)1;
	}
	return 0;
}


short any__Dv2_s(
	short2 src0
	)
{
	short temp = (short)(-32768);
	if((src0.s0|src0.s1)&temp){
		return (short)1;
	}
	return 0;
}


short any__Dv3_s(
	short3 src0
	)
{
	short temp = (short)(-32768);
	src0.s0=(src0.s0|src0.s1|src0.s2)&temp;
	if(src0.s0){
		return (short)1;
	}
	return 0;
}


short any__Dv4_s(
	short4 src0
	)
{
	short temp = (short)(-32768);
	src0.s01=(src0.s01|src0.s23);
	src0.s0=(src0.s0|src0.s1)&temp;
	if(src0.s0){
		return (short)1;
	}
	return 0;
}


short any__Dv8_s(
	short8 src0

	)
{
	short temp = (short)(-32768);
	src0.s0123=(src0.s0123|src0.s4567);
	src0.s01 = (src0.s01|src0.s23);
	src0.s0=(src0.s0|src0.s1)&temp;
	if(src0.s0){
		return (short)1;
	}
	return 0;
}


uchar bitselect__hhh(
	uchar src0,
	uchar src1,
	uchar src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}

char bitselect__ccc(
	char src0,
	char src1,
	char src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}

char16 bitselect__Dv16_cDv16_cDv16_c(
	char16 src0,
	char16 src1,
	char16 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


char2 bitselect__Dv2_cDv2_cDv2_c(
	char2 src0,
	char2 src1,
	char2 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


char3 bitselect__Dv3_cDv3_cDv3_c(
	char3 src0,
	char3 src1,
	char3 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


char4 bitselect__Dv4_cDv4_cDv4_c(
	char4 src0,
	char4 src1,
	char4 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


char8 bitselect__Dv8_cDv8_cDv8_c(
	char8 src0,
	char8 src1,
	char8 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


int bitselect__iii(
	int src0,
	int src1,
	int src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


int16 bitselect__Dv16_iDv16_iDv16_i(
	int16 src0,
	int16 src1,
	int16 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


int2 bitselect__Dv2_iDv2_iDv2_i(
	int2 src0,
	int2 src1,
	int2 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


int3 bitselect__Dv3_iDv3_iDv3_i(
	int3 src0,
	int3 src1,
	int3 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


int4 bitselect__Dv4_iDv4_iDv4_i(
	int4 src0,
	int4 src1,
	int4 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


int8 bitselect__Dv8_iDv8_iDv8_i(
	int8 src0,
	int8 src1,
	int8 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


short bitselect__sss(
	short src0,
	short src1,
	short src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


short16 bitselect__Dv16_sDv16_sDv16_s(
	short16 src0,
	short16 src1,
	short16 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


short2 bitselect__Dv2_sDv2_sDv2_s(
	short2 src0,
	short2 src1,
	short2 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


short3 bitselect__Dv3_sDv3_sDv3_s(
	short3 src0,
	short3 src1,
	short3 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


short4 bitselect__Dv4_sDv4_sDv4_s(
	short4 src0,
	short4 src1,
	short4 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


short8 bitselect__Dv8_sDv8_sDv8_s(
	short8 src0,
	short8 src1,
	short8 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uchar16 bitselect__Dv16_hDv16_hDv16_h(
	uchar16 src0,
	uchar16 src1,
	uchar16 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uchar2 bitselect__Dv2_hDv2_hDv2_h(
	uchar2 src0,
	uchar2 src1,
	uchar2 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uchar3 bitselect__Dv3_hDv3_hDv3_h(
	uchar3 src0,
	uchar3 src1,
	uchar3 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uchar4 bitselect__Dv4_hDv4_hDv4_h(
	uchar4 src0,
	uchar4 src1,
	uchar4 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uchar8 bitselect__Dv8_hDv8_hDv8_h(
	uchar8 src0,
	uchar8 src1,
	uchar8 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uint bitselect__jjj(
	uint src0,
	uint src1,
	uint src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uint16 bitselect__Dv16_jDv16_jDv16_j(
	uint16 src0,
	uint16 src1,
	uint16 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uint2 bitselect__Dv2_jDv2_jDv2_j(
	uint2 src0,
	uint2 src1,
	uint2 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uint3 bitselect__Dv3_jDv3_jDv3_j(
	uint3 src0,
	uint3 src1,
	uint3 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uint4 bitselect__Dv4_jDv4_jDv4_j(
	uint4 src0,
	uint4 src1,
	uint4 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


uint8 bitselect__Dv8_jDv8_jDv8_j(
	uint8 src0,
	uint8 src1,
	uint8 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


ushort bitselect__ttt(
	ushort src0,
	ushort src1,
	ushort src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


ushort16 bitselect__Dv16_tDv16_tDv16_t(
	ushort16 src0,
	ushort16 src1,
	ushort16 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


ushort2 bitselect__Dv2_tDv2_tDv2_t(
	ushort2 src0,
	ushort2 src1,
	ushort2 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


ushort3 bitselect__Dv3_tDv3_tDv3_t(
	ushort3 src0,
	ushort3 src1,
	ushort3 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


ushort4 bitselect__Dv4_tDv4_tDv4_t(
	ushort4 src0,
	ushort4 src1,
	ushort4 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


ushort8 bitselect__Dv8_tDv8_tDv8_t(
	ushort8 src0,
	ushort8 src1,
	ushort8 src2
	)
{
	return ((src2&src1)|((~src2)&src0));
}


float bitselect__fff(
	float src0,
	float src1,
	float src2
	)
{
	int isrc0 = as_int(src0);
	int isrc1 = as_int(src1);
	int isrc2 = as_int(src2);

	return as_float(((isrc2&isrc1)|((~isrc2)&isrc0)));
}


float8 bitselect__Dv8_fDv8_fDv8_f(
	float8 src0,
	float8 src1,
	float8 src2
	)
{
	int8 isrc0,isrc1,isrc2;
	isrc0 = as_int8(src0);
	isrc1 = as_int8(src1);
	isrc2 = as_int8(src2);

	return as_float8(((isrc2&isrc1)|((~isrc2)&isrc0)));
}

float4 bitselect__Dv4_fDv4_fDv4_f(
	float4 src0,
	float4 src1,
	float4 src2
	)
{
	int4 isrc0,isrc1,isrc2;
	isrc0 = as_int4(src0);
	isrc1 = as_int4(src1);
	isrc2 = as_int4(src2);

	return as_float4(((isrc2&isrc1)|((~isrc2)&isrc0)));
}

float16 bitselect__Dv16_fDv16_fDv16_f(
	float16 src0,
	float16 src1,
	float16 src2
	)
{
	int16 isrc0,isrc1,isrc2;
	isrc0 = as_int16(src0);
	isrc1 = as_int16(src1);
	isrc2 = as_int16(src2);

	return as_float16(((isrc2&isrc1)|((~isrc2)&isrc0)));
}

float3 bitselect__Dv3_fDv3_fDv3_f(
	float3 src0,
	float3 src1,
	float3 src2
	)
{
	int3 isrc0,isrc1,isrc2;
	isrc0 = as_int3(src0);
	isrc1 = as_int3(src1);
	isrc2 = as_int3(src2);

	return as_float3(((isrc2&isrc1)|((~isrc2)&isrc0)));
}

float2 bitselect__Dv2_fDv2_fDv2_f(
	float2 src0,
	float2 src1,
	float2 src2
	)
{
	int2 isrc0,isrc1,isrc2;
	isrc0 = as_int2(src0);
	isrc1 = as_int2(src1);
	isrc2 = as_int2(src2);

	return as_float2(((isrc2&isrc1)|((~isrc2)&isrc0)));
}

float rint__f(
    float in
    )
{
    float int_part = viv_floor(in);
    float fract_part = in - int_part;
    if (fract_part < 0.5f)
    {
        return (int_part);
    }
    else if (fract_part > 0.5f)
    {
        return (int_part + 1.0f);
    }
    else
    {
        int int_value = (int) int_part;
        if ((int_value & 1) == 0)
        {
            return (int_part);
        }
        else
        {
            return (int_part + 1.0f);
        }
    }
}


int hadd__ii(
    int src0,
    int src1
    )
{
    return ((src0 >> 1) + (src1 >> 1) + (1 & src0 & src1));
}


float distance__ff(
    float src0,
    float src1
    )
{
	float diff = viv_fabs(src0-src1);
	return diff;
}


float distance__Dv2_fDv2_f(
    float2 src0,
    float2 src1
    )
{
	float2 diff = viv_fabs(src0-src1);
	return viv_sqrt(viv_dot(diff,diff));
}


float distance__Dv3_fDv3_f(
    float3 src0,
    float3 src1
    )
{
	float3 diff = viv_fabs(src0-src1);
	return viv_sqrt(viv_dot(diff,diff));
}


float distance__Dv4_fDv4_f(
    float4 src0,
    float4 src1
    )
{
	float4 diff = viv_fabs(src0-src1);
	return viv_sqrt(viv_dot(diff,diff));
}


float fast_distance__ff(
	float src0,
    float src1
	)
{
	float diff = viv_fabs(src0-src1);
	return diff;
}


float fast_distance__Dv2_fDv2_f(
    float2 src0,
    float2 src1
    )
{
	float2 diff = viv_fabs(src0-src1);
	return viv_half_sqrt(viv_dot(diff,diff));
}


float fast_distance__Dv3_fDv3_f(
    float3 src0,
    float3 src1
    )
{
	float3 diff = viv_fabs(src0-src1);
	return viv_half_sqrt(viv_dot(diff,diff));
}


float fast_distance__Dv4_fDv4_f(
    float4 src0,
    float4 src1
    )
{
	float4 diff = viv_fabs(src0-src1);
	return viv_half_sqrt(viv_dot(diff,diff));
}


float fast_length__f(
	float src0
	)
{
	return viv_fabs(src0);
}


float fast_length__Dv2_f(
	float2 src0
	)
{
	return viv_half_sqrt(viv_dot(src0,src0));
}


float fast_length__Dv3_f(
	float3 src0
	)
{
	return viv_half_sqrt(viv_dot(src0,src0));
}


float fast_length__Dv4_f(
	float4 src0
	)
{
	return viv_half_sqrt(viv_dot(src0,src0));
}


float fast_normalize__f(
	float src0
	)
{
	return 1.0f;
}


float2 fast_normalize__Dv2_f(
	float2 src0
	)
{
	return src0*viv_half_rsqrt(viv_dot(src0,src0));
}


float3 fast_normalize__Dv3_f(
	float3 src0
	)
{
	return src0*viv_half_rsqrt(viv_dot(src0,src0));
}


float4 fast_normalize__Dv4_f(
	float4 src0
	)
{
	return src0*viv_half_rsqrt(viv_dot(src0,src0));
}









int get_image_channel_data_type__11ocl_image2d(
	image2d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	return (*img0).channelDataType;
}



int get_image_channel_data_type__11ocl_image3d(
	image3d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	return (*img0).channelDataType;
}



int get_image_channel_order__11ocl_image2d(
	image2d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	return (*img0).channelOrder;
}



int get_image_channel_order__11ocl_image3d(
	image3d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	return (*img0).channelOrder;
}



int get_image_depth__11ocl_image3d(
	image3d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	return (*img0).depth;
}



int2 get_image_dim__11ocl_image2d(
	image2d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	int width = ((*img0).width);
	int height = ((*img0).height);
	return (int2)(width,height) ;
	//return (int2) (    ((*img0).width)   ,    ((*img0).height)    );
}





int4 get_image_dim__11ocl_image3d(
	image3d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	int width = ((*img0).width);
	int height = ((*img0).height);
	int depth = ((*img0).depth);
	return (int4)(width,height,depth,0) ;
}



int get_image_height__11ocl_image2d(
	image2d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	return (*img0).height;
}






int get_image_height__11ocl_image3d(
	image3d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	return (*img0).height;
}



int get_image_width__11ocl_image2d(
	image2d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	return (*img0).width;
}



int get_image_width__11ocl_image3d(
	image3d_t image
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	return (*img0).width;
}



int isequal__ff(
	float src0,
	float src1
	)
{
	return(src0==src1);
}


int16 isequal__Dv16_fDv16_f(
	float16 src0,
	float16 src1
	)
{
	return(src0==src1);
}


int2 isequal__Dv2_fDv2_f(
	float2 src0,
	float2 src1
	)
{
	return(src0==src1);
}


int3 isequal__Dv3_fDv3_f(
	float3 src0,
	float3 src1
	)
{
	return(src0==src1);
}


int4 isequal__Dv4_fDv4_f(
	float4 src0,
	float4 src1
	)
{
	return(src0==src1);
}


int8 isequal__Dv8_fDv8_f(
	float8 src0,
	float8 src1
	)
{
	return(src0==src1);
}


int isfinite__f(
	float src0
	)
{
	union float_int{
		float f;
		int i;
	} float_int_t;

	float_int_t.f = src0;

	//uint src0_uint = as_uint(src0); // as of 6/24/13 bug in as_type;
	if((0x7f800000&float_int_t.i)==0x7f800000){ //if exponent bits are all set, value is NOT finite
		return 0;
	}
	return 1;
}


int16 isfinite__Dv16_f(
	float16 src0
	)
{
	union float_int{
		float16 f;
		uint16 i;
	} float_int_t;

	float_int_t.f = src0;
	float_int_t.i = float_int_t.i & 0x7f800000;

	int16 ret = (int16)(-1);
	//int16 ret = (int16)(-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1);

	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=0;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=0;
	}
	if((float_int_t.i.s2)==0x7f800000){
		ret.s2=0;
	}
	if((float_int_t.i.s3)==0x7f800000){
		ret.s3=0;
	}
	if((float_int_t.i.s4)==0x7f800000){
		ret.s4=0;
	}
    if((float_int_t.i.s5)==0x7f800000){
		ret.s5=0;
	}
	if((float_int_t.i.s6)==0x7f800000){
		ret.s6=0;
	}
	if((float_int_t.i.s7)==0x7f800000){
		ret.s7=0;
	}
	if((float_int_t.i.s8)==0x7f800000){
		ret.s8=0;
	}
    if((float_int_t.i.s9)==0x7f800000){
		ret.s9=0;
	}
	if((float_int_t.i.sA)==0x7f800000){
		ret.sA=0;
	}
	if((float_int_t.i.sB)==0x7f800000){
		ret.sB=0;
	}
	if((float_int_t.i.sC)==0x7f800000){
		ret.sC=0;
	}
    if((float_int_t.i.sD)==0x7f800000){
		ret.sD=0;
	}
	if((float_int_t.i.sE)==0x7f800000){
		ret.sE=0;
	}
	if((float_int_t.i.sF)==0x7f800000){
		ret.sF=0;
	}
    return ret;
}


int2 isfinite__Dv2_f(
	float2 src0
	)
{
	union float_int{
		float2 f;
		int2 i;
	} float_int_t;

	float_int_t.f = src0;
	float_int_t.i = float_int_t.i & 0x7f800000;

	//uint2 src0_uint2 = as_uint2(src0); // reinterpret float2 as int2 for bitwise operations
    int2 ret = -1;

	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=0;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=0;
	}
    return ret;
}


int3 isfinite__Dv3_f(
	float3 src0
	)
{
	union float_int{
		float3 f;
		int3 i;
	}float_int_t;

	float_int_t.f = src0;
	float_int_t.i = float_int_t.i & 0x7f800000;

	//uint3 src0_uint3 = as_uint3(src0); // as of 6/24/13 bug in as_type function
    int3 ret = -1;

	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=0;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=0;
	}
	if((float_int_t.i.s2)==0x7f800000){
		ret.s2=0;
	}
    return ret;
}


int4 isfinite__Dv4_f(
	float4 src0
	)
{
	union float_int{
		float4 f;
		int4 i;
	} float_int_t;

	float_int_t.f = src0;
	float_int_t.i = float_int_t.i & 0x7f800000;

	//uint4 src0_uint4 = as_uint4(src0); // as of 6/24/13 bug in as_type function
    int4 ret = -1;

	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=0;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=0;
	}
	if((float_int_t.i.s2)==0x7f800000){
		ret.s2=0;
	}
	if((float_int_t.i.s3)==0x7f800000){
		ret.s3=0;
	}
    return ret;
}


int8 isfinite__Dv8_f(
	float8 src0
	)
{
	union float_int{
		float8 f;
		uint8 i;
	} float_int_t;

	float_int_t.f = src0;
	float_int_t.i = float_int_t.i & 0x7f800000;

	//uint8 src0_uint8 = as_uint8(src0); // as of 6/24/13 buy in as_type functions

	int8 ret = (int8)(-1,-1,-1,-1,-1,-1,-1,-1);

	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=0;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=0;
	}
	if((float_int_t.i.s2)==0x7f800000){
		ret.s2=0;
	}
	if((float_int_t.i.s3)==0x7f800000){
		ret.s3=0;
	}
	if((float_int_t.i.s4)==0x7f800000){
		ret.s4=0;
	}
    if((float_int_t.i.s5)==0x7f800000){
		ret.s5=0;
	}
	if((float_int_t.i.s6)==0x7f800000){
		ret.s6=0;
	}
	if((float_int_t.i.s7)==0x7f800000){
		ret.s7=0;
	}
    return ret;
}


int isgreater__ff(
	float src0,
	float src1
	)
{
	return(src0>src1);
}


int16 isgreater__Dv16_fDv16_f(
	float16 src0,
	float16 src1
	)
{
	return(src0>src1);
}


int2 isgreater__Dv2_fDv2_f(
	float2 src0,
	float2 src1
	)
{
	return(src0>src1);
}


int3 isgreater__Dv3_fDv3_f(
	float3 src0,
	float3 src1
	)
{
	return(src0>src1);
}


int4 isgreater__Dv4_fDv4_f(
	float4 src0,
	float4 src1
	)
{
	return(src0>src1);
}


int8 isgreater__Dv8_fDv8_f(
	float8 src0,
	float8 src1
	)
{
	return(src0>src1);
}


int isgreaterequal__ff(
	float src0,
	float src1
	)
{
	return(src0>=src1);
}


int16 isgreaterequal__Dv16_fDv16_f(
	float16 src0,
	float16 src1
	)
{
	return(src0>=src1);
}


int2 isgreaterequal__Dv2_fDv2_f(
	float2 src0,
	float2 src1
	)
{
	return(src0>=src1);
}


int3 isgreaterequal__Dv3_fDv3_f(
	float3 src0,
	float3 src1
	)
{
	return(src0>=src1);
}


int4 isgreaterequal__Dv4_fDv4_f(
	float4 src0,
	float4 src1
	)
{
	return(src0>=src1);
}


int8 isgreaterequal__Dv8_fDv8_f(
	float8 src0,
	float8 src1
	)
{
	return(src0>=src1);
}


int isinf__f(
	float src0
	)
{
	union float_int{
		float f;
		uint i;
	}float_int_t;

	float_int_t.f = src0;

	//uint src0_uint = as_uint(src0); // as of 6/24/13, bug in as_type function
	if((0x7fffffff&float_int_t.i)==0x7f800000){ //if exponent bits are all set and mantissa bits are all 0, value is an inf
		return 1;
	}
	return 0;
}


int16 isinf__Dv16_f(
	float16 src0
	)
{
	union float_int{
		float16 f;
		uint16 i;
	}float_int_t;

	float_int_t.f = src0;
	float_int_t.i = float_int_t.i&0x7FFFFFFF;

	//uint16 src0_uint16 = as_uint16(src0); // as of 6/24/13 bug in as_type function
    int16 ret = (int16)(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=-1;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=-1;
	}
	if((float_int_t.i.s2)==0x7f800000){
		ret.s2=-1;
	}
	if((float_int_t.i.s3)==0x7f800000){
		ret.s3=-1;
	}
	if((float_int_t.i.s4)==0x7f800000){
		ret.s4=-1;
	}
    if((float_int_t.i.s5)==0x7f800000){
		ret.s5=-1;
	}
	if((float_int_t.i.s6)==0x7f800000){
		ret.s6=-1;
	}
	if((float_int_t.i.s7)==0x7f800000){
		ret.s7=-1;
	}
	if((float_int_t.i.s8)==0x7f800000){
		ret.s8=-1;
	}
    if((float_int_t.i.s9)==0x7f800000){
		ret.s9=-1;
	}
	if((float_int_t.i.sA)==0x7f800000){
		ret.sA=-1;
	}
	if((float_int_t.i.sB)==0x7f800000){
		ret.sB=-1;
	}
	if((float_int_t.i.sC)==0x7f800000){
		ret.sC=-1;
	}
    if((float_int_t.i.sD)==0x7f800000){
		ret.sD=-1;
	}
	if((float_int_t.i.sE)==0x7f800000){
		ret.sE=-1;
	}
	if((float_int_t.i.sF)==0x7f800000){
		ret.sF=-1;
	}
    return ret;
}


int2 isinf__Dv2_f(
	float2 src0
	)
{
	union float_int{
		float2 f;
		uint2 i;
	} float_int_t;

	float_int_t.f = src0;
	float_int_t.i = float_int_t.i &(uint2)0x7fffffff;

	//uint2 src0_uint2 = as_uint2(src0); //  as of 6/24/13, bug in as_type function


	int2 ret = (int2) (0,0);

	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=-1;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=-1;
	}
    return ret;
}


int3 isinf__Dv3_f(
	float3 src0
	)
{

	union float_int{
		float3 f;
		uint3 i;
	} float_int_t;
	float_int_t.f = src0;
	float_int_t.i = float_int_t.i&0x7fffffff;

	//uint3 src0_uint3 = as_uint3(src0); //  as of 6/24/13, bug in as_type function
    int3 ret = (int3)(0,0,0);
	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=-1;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=-1;
	}
	if((float_int_t.i.s2)==0x7f800000){
		ret.s2=-1;
	}
    return ret;
}


int4 isinf__Dv4_f(
	float4 src0
	)
{
	union float_int {
		float4 f;
		uint4 i;
	} float_int_t;

	float_int_t.f = src0;
	float_int_t.i = float_int_t.i&(uint4)0x7fffffff;
	//uint4 src0_uint4 = as_uint4(src0); // as of 6/24/13, bug in as_type function
    int4 ret = (int4)(0,0,0,0);

	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=-1;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=-1;
	}
	if((float_int_t.i.s2)==0x7f800000){
		ret.s2=-1;
	}
	if((float_int_t.i.s3)==0x7f800000){
		ret.s3=-1;
	}
    return ret;
}


int8 isinf__Dv8_f(
	float8 src0
	)
{
	union float_int{
		float8 f;
		int8 i;
	} float_int_t;
	float_int_t.f = src0;
	float_int_t.i = float_int_t.i & 0x7fffffff;

	//uint8 src0_uint8 = as_uint8(src0); // as of 6/24/13, bug in as_type function
	int8 ret = (int8)(0,0,0,0,0,0,0,0);

	if((float_int_t.i.s0)==0x7f800000){
		ret.s0=-1;
	}
    if((float_int_t.i.s1)==0x7f800000){
		ret.s1=-1;
	}
	if((float_int_t.i.s2)==0x7f800000){
		ret.s2=-1;
	}
	if((float_int_t.i.s3)==0x7f800000){
		ret.s3=-1;
	}
	if((float_int_t.i.s4)==0x7f800000){
		ret.s4=-1;
	}
    if((float_int_t.i.s5)==0x7f800000){
		ret.s5=-1;
	}
	if((float_int_t.i.s6)==0x7f800000){
		ret.s6=-1;
	}
	if((float_int_t.i.s7)==0x7f800000){
		ret.s7=-1;
	}
    return ret;
}


int isless__ff(
	float src0,
	float src1
	)
{
	return(src0<src1);
}


int16 isless__Dv16_fDv16_f(
	float16 src0,
	float16 src1
	)
{
	return(src0<src1);
}


int2 isless__Dv2_fDv2_f(
	float2 src0,
	float2 src1
	)
{
	return(src0<src1);
}


int3 isless__Dv3_fDv3_f(
	float3 src0,
	float3 src1
	)
{
	return(src0<src1);
}


int4 isless__Dv4_fDv4_f(
	float4 src0,
	float4 src1
	)
{
	return(src0<src1);
}


int8 isless__Dv8_fDv8_f(
	float8 src0,
	float8 src1
	)
{
	return(src0<src1);
}


int islessequal__ff(
	float src0,
	float src1
	)
{
	return(src0<=src1);
}


int16 islessequal__Dv16_fDv16_f(
	float16 src0,
	float16 src1
	)
{
	return(src0<=src1);
}


int2 islessequal__Dv2_fDv2_f(
	float2 src0,
	float2 src1
	)
{
	return(src0<=src1);
}


int3 islessequal__Dv3_fDv3_f(
	float3 src0,
	float3 src1
	)
{
	return(src0<=src1);
}


int4 islessequal__Dv4_fDv4_f(
	float4 src0,
	float4 src1
	)
{
	return(src0<=src1);
}


int8 islessequal__Dv8_fDv8_f(
	float8 src0,
	float8 src1
	)
{
	return(src0<=src1);
}


int islessgreater__ff(
	float src0,
	float src1
	)
{
	return((src0<src1)||(src0>src1));
}


int16 islessgreater__Dv16_fDv16_f(
	float16 src0,
	float16 src1
	)
{
	return((src0<src1)||(src0>src1));
}


int2 islessgreater__Dv2_fDv2_f(
	float2 src0,
	float2 src1
	)
{
	return((src0<src1)||(src0>src1));
}


int3 islessgreater__Dv3_fDv3_f(
	float3 src0,
	float3 src1
	)
{
	return((src0<src1)||(src0>src1));
}


int4 islessgreater__Dv4_fDv4_f(
	float4 src0,
	float4 src1
	)
{
	return((src0<src1)||(src0>src1));
}


int8 islessgreater__Dv8_fDv8_f(
	float8 src0,
	float8 src1
	)
{
	return((src0<src1)||(src0>src1));
}


int isnan__f(
	float src0
	)
{

	if(src0!=src0){ //NaN != NaN
		return 1;
	}
	return 0;
}


int16 isnan__Dv16_f(
	float16 src0
	)
{
	int16 ret = (int16) (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

	if(src0.s0!=src0.s0){ //NaN != NaN
		ret.s0 = -1;
	}
	if(src0.s1!=src0.s1){ //NaN != NaN
		ret.s1 = -1;
	}
	if(src0.s2!=src0.s2){ //NaN != NaN
		ret.s2 = -1;
	}
	if(src0.s3!=src0.s3){ //NaN != NaN
		ret.s3 = -1;
	}
	if(src0.s4!=src0.s4){ //NaN != NaN
		ret.s4 = -1;
	}
	if(src0.s5!=src0.s5){ //NaN != NaN
		ret.s5 = -1;
	}
	if(src0.s6!=src0.s6){ //NaN != NaN
		ret.s6 = -1;
	}
	if(src0.s7!=src0.s7){ //NaN != NaN
		ret.s7 = -1;
	}
	if(src0.s8!=src0.s8){ //NaN != NaN
		ret.s8 = -1;
	}
	if(src0.s9!=src0.s9){ //NaN != NaN
		ret.s9 = -1;
	}
	if(src0.sA!=src0.sA){ //NaN != NaN
		ret.sA = -1;
	}
	if(src0.sB!=src0.sB){ //NaN != NaN
		ret.sB = -1;
	}
	if(src0.sC!=src0.sC){ //NaN != NaN
		ret.sC = -1;
	}
	if(src0.sD!=src0.sD){ //NaN != NaN
		ret.sD = -1;
	}
	if(src0.sE!=src0.sE){ //NaN != NaN
		ret.sE = -1;
	}
	if(src0.sF!=src0.sF){ //NaN != NaN
		ret.sF = -1;
	}
	return ret;
}


int2 isnan__Dv2_f(
	float2 src0
	)
{
	int2 ret = (int2) 0;

	if(src0.s0!=src0.s0){ //NaN != NaN
		ret.s0 = -1;
	}
	if(src0.s1!=src0.s1){ //NaN != NaN
		ret.s1 = -1;
	}

	return ret;
}


int3 isnan__Dv3_f(
	float3 src0
	)
{
	int3 ret = (int3) (0,0,0);

	if(src0.s0!=src0.s0){ //NaN != NaN
		ret.s0 = -1;
	}
	if(src0.s1!=src0.s1){ //NaN != NaN
		ret.s1 = -1;
	}
	if(src0.s2!=src0.s2){ //NaN != NaN
		ret.s2 = -1;
	}

	return ret;
}


int4 isnan__Dv4_f(
	float4 src0
	)
{
	int4 ret = (int4) (0,0,0,0);

	if(src0.s0!=src0.s0){ //NaN != NaN
		ret.s0 = -1;
	}
	if(src0.s1!=src0.s1){ //NaN != NaN
		ret.s1 = -1;
	}
	if(src0.s2!=src0.s2){ //NaN != NaN
		ret.s2 = -1;
	}
	if(src0.s3!=src0.s3){ //NaN != NaN
		ret.s3 = -1;
	}
	return ret;
}


int8 isnan__Dv8_f(
	float8 src0
	)
{
	int8 ret = (int8) (0,0,0,0,0,0,0,0);

	if(src0.s0!=src0.s0){ //NaN != NaN
		ret.s0 = -1;
	}
	if(src0.s1!=src0.s1){ //NaN != NaN
		ret.s1 = -1;
	}
	if(src0.s2!=src0.s2){ //NaN != NaN
		ret.s2 = -1;
	}
	if(src0.s3!=src0.s3){ //NaN != NaN
		ret.s3 = -1;
	}
	if(src0.s4!=src0.s4){ //NaN != NaN
		ret.s4 = -1;
	}
	if(src0.s5!=src0.s5){ //NaN != NaN
		ret.s5 = -1;
	}
	if(src0.s6!=src0.s6){ //NaN != NaN
		ret.s6 = -1;
	}
	if(src0.s7!=src0.s7){ //NaN != NaN
		ret.s7 = -1;
	}
	return ret;
}


int isnormal__f(
	float src0
	)
{
	union float_int{
		float f;
		int i;
	} float_int_t;

	float_int_t.f = src0;
	int temp = 0x7f800000&float_int_t.i;
	//uint src0_uint = as_uint(src0); // as of 6/24/13 bug in as_type;
	if((temp==0)||(temp==0x7f800000)){ //if exponent bits are all 0: DEnormalized; all 1: infinity/NaN
		return 0;
	}
	return 1;
}


int2 isnormal__Dv2_f(
	float2 src0
	)
{
	int2 ret = (int2) -1;

	//Intermediate Variables//
	/*int2 zero =-1;
	int2 inf =-1;*/

    int2 srcI = as_int2(src0);

	//float2_int2_t.f = src0; //set floating point value

	srcI = 0x7f800000&srcI;// apply mask for only exponent bits

	//zero = (srcI==0); //Check if component's exponent bits are equal to zero (Denormalized)
	//inf = (srcI == 0x7f800000); //Check whether the components are (+/-) infinity or NaN


	if((srcI.x==0)||(srcI.x==0x7f800000)){ //if Denorm/NaN/Infinity then the component is not normal
		ret.s0 = 0;
	}
	if((srcI.y==0)||(srcI.y==0x7f800000)){
		ret.s1 = 0;
	}

	//return srcI;
	//return zero;
	return ret;
}


int isnotequal__ff(
	float src0,
	float src1
	)
{
	return(src0!=src1);
}


int16 isnotequal__Dv16_fDv16_f(
	float16 src0,
	float16 src1
	)
{
	return(src0!=src1);
}


int2 isnotequal__Dv2_fDv2_f(
	float2 src0,
	float2 src1
	)
{
	return(src0!=src1);
}


int3 isnotequal__Dv3_fDv3_f(
	float3 src0,
	float3 src1
	)
{
	return(src0!=src1);
}


int4 isnotequal__Dv4_fDv4_f(
	float4 src0,
	float4 src1
	)
{
	return(src0!=src1);
}


int8 isnotequal__Dv8_fDv8_f(
	float8 src0,
	float8 src1
	)
{
	return(src0!=src1);
}


int isordered__ff(
	float src0,
	float src1
	)
{
	return (isequal(src0,src0)&&isequal(src1,src1));
	}


int16 isordered__Dv16_fDv16_f(
	float16 src0,
	float16 src1
	)
{
	int s0 = (isequal(src0.s0,src0.s0)&&isequal(src1.s0,src1.s0));
	int s1 = (isequal(src0.s1,src0.s1)&&isequal(src1.s1,src1.s1));
	int s2 = (isequal(src0.s2,src0.s2)&&isequal(src1.s2,src1.s2));
	int s3 = (isequal(src0.s3,src0.s3)&&isequal(src1.s3,src1.s3));
	int s4 = (isequal(src0.s4,src0.s4)&&isequal(src1.s4,src1.s4));
	int s5 = (isequal(src0.s5,src0.s5)&&isequal(src1.s5,src1.s5));
	int s6 = (isequal(src0.s6,src0.s6)&&isequal(src1.s6,src1.s6));
	int s7 = (isequal(src0.s7,src0.s7)&&isequal(src1.s7,src1.s7));
	int s8 = (isequal(src0.s8,src0.s8)&&isequal(src1.s8,src1.s8));
	int s9 = (isequal(src0.s9,src0.s9)&&isequal(src1.s9,src1.s9));
	int sA = (isequal(src0.sA,src0.sA)&&isequal(src1.sA,src1.sA));
	int sB = (isequal(src0.sB,src0.sB)&&isequal(src1.sB,src1.sB));
	int sC = (isequal(src0.sC,src0.sC)&&isequal(src1.sC,src1.sC));
	int sD = (isequal(src0.sD,src0.sD)&&isequal(src1.sD,src1.sD));
	int sE = (isequal(src0.sE,src0.sE)&&isequal(src1.sE,src1.sE));
	int sF = (isequal(src0.sF,src0.sF)&&isequal(src1.sF,src1.sF));
	return (int16) (-s0,-s1,-s2,-s3,-s4,-s5,-s6,-s7,-s8,-s9,-sA,-sB,-sC,-sD,-sE,-sF);
	}


int2 isordered__Dv2_fDv2_f(
	float2 src0,
	float2 src1
	)
{
	int s0 = (isequal(src0.s0,src0.s0)&&isequal(src1.s0,src1.s0));
	int s1 = (isequal(src0.s1,src0.s1)&&isequal(src1.s1,src1.s1));
	return (int2) (-s0,-s1);
	}


int3 isordered__Dv3_fDv3_f(
	float3 src0,
	float3 src1
	)
{
	int s0 = (isequal(src0.s0,src0.s0)&&isequal(src1.s0,src1.s0));
	int s1 = (isequal(src0.s1,src0.s1)&&isequal(src1.s1,src1.s1));
	int s2 = (isequal(src0.s2,src0.s2)&&isequal(src1.s2,src1.s2));
	return (int3) (-s0,-s1,-s2);
	}


int4 isordered__Dv4_fDv4_f(
	float4 src0,
	float4 src1
	)
{
	int s0 = (isequal(src0.s0,src0.s0)&&isequal(src1.s0,src1.s0));
	int s1 = (isequal(src0.s1,src0.s1)&&isequal(src1.s1,src1.s1));
	int s2 = (isequal(src0.s2,src0.s2)&&isequal(src1.s2,src1.s2));
	int s3 = (isequal(src0.s3,src0.s3)&&isequal(src1.s3,src1.s3));
	return (int4) (-s0,-s1,-s2,-s3);
	}


int8 isordered__Dv8_fDv8_f(
	float8 src0,
	float8 src1
	)
{
	int s0 = (isequal(src0.s0,src0.s0)&&isequal(src1.s0,src1.s0));
	int s1 = (isequal(src0.s1,src0.s1)&&isequal(src1.s1,src1.s1));
	int s2 = (isequal(src0.s2,src0.s2)&&isequal(src1.s2,src1.s2));
	int s3 = (isequal(src0.s3,src0.s3)&&isequal(src1.s3,src1.s3));
	int s4 = (isequal(src0.s4,src0.s4)&&isequal(src1.s4,src1.s4));
	int s5 = (isequal(src0.s5,src0.s5)&&isequal(src1.s5,src1.s5));
	int s6 = (isequal(src0.s6,src0.s6)&&isequal(src1.s6,src1.s6));
	int s7 = (isequal(src0.s7,src0.s7)&&isequal(src1.s7,src1.s7));
	return (int8) (-s0,-s1,-s2,-s3,-s4,-s5,-s6,-s7);
	}


int isunordered__ff(
	float src0,
	float src1
	)
{
	return ((src0!=src0)||(src1!=src1));
	}


int16 isunordered__Dv16_fDv16_f(
	float16 src0,
	float16 src1
	)
{
	int s0 = ((src0.s0!=src0.s0)||(src1.s0!=src1.s0));
	int s1 = ((src0.s1!=src0.s1)||(src1.s1!=src1.s1));
	int s2 = ((src0.s2!=src0.s2)||(src1.s2!=src1.s2));
	int s3 = ((src0.s3!=src0.s3)||(src1.s3!=src1.s3));
	int s4 = ((src0.s4!=src0.s4)||(src1.s4!=src1.s4));
	int s5 = ((src0.s5!=src0.s5)||(src1.s5!=src1.s5));
	int s6 = ((src0.s6!=src0.s6)||(src1.s6!=src1.s6));
	int s7 = ((src0.s7!=src0.s7)||(src1.s7!=src1.s7));
	int s8 = ((src0.s8!=src0.s8)||(src1.s8!=src1.s8));
	int s9 = ((src0.s9!=src0.s9)||(src1.s9!=src1.s9));
	int sA = ((src0.sA!=src0.sA)||(src1.sA!=src1.sA));
	int sB = ((src0.sB!=src0.sB)||(src1.sB!=src1.sB));
	int sC = ((src0.sC!=src0.sC)||(src1.sC!=src1.sC));
	int sD = ((src0.sD!=src0.sD)||(src1.sD!=src1.sD));
	int sE = ((src0.sE!=src0.sE)||(src1.sE!=src1.sE));
	int sF = ((src0.sF!=src0.sF)||(src1.sF!=src1.sF));
	return (int16) (-s0,-s1,-s2,-s3,-s4,-s5,-s6,-s7,-s8,-s9,-sA,-sB,-sC,-sD,-sE,-sF);
	}


int2 isunordered__Dv2_fDv2_f(
	float2 src0,
	float2 src1
	)
{
	int s0 = ((src0.s0!=src0.s0)||(src1.s0!=src1.s0));
	int s1 = ((src0.s1!=src0.s1)||(src1.s1!=src1.s1));
	return (int2) (-s0,-s1);
	}


int3 isunordered__Dv3_fDv3_f(
	float3 src0,
	float3 src1
	)
{
	int s0 = ((src0.s0!=src0.s0)||(src1.s0!=src1.s0));
	int s1 = ((src0.s1!=src0.s1)||(src1.s1!=src1.s1));
	int s2 = ((src0.s2!=src0.s2)||(src1.s2!=src1.s2));
	//int s3 = ((src0.s3!=src0.s3)||(src1.s3!=src1.s3));
	return (int3) (-s0,-s1,-s2);
	}


int4 isunordered__Dv4_fDv4_f(
	float4 src0,
	float4 src1
	)
{
	int s0 = ((src0.s0!=src0.s0)||(src1.s0!=src1.s0));
	int s1 = ((src0.s1!=src0.s1)||(src1.s1!=src1.s1));
	int s2 = ((src0.s2!=src0.s2)||(src1.s2!=src1.s2));
	int s3 = ((src0.s3!=src0.s3)||(src1.s3!=src1.s3));
	return (int4) (-s0,-s1,-s2,-s3);
	}


int8 isunordered__Dv8_fDv8_f(
	float8 src0,
	float8 src1
	)
{
	int s0 = ((src0.s0!=src0.s0)||(src1.s0!=src1.s0));
	int s1 = ((src0.s1!=src0.s1)||(src1.s1!=src1.s1));
	int s2 = ((src0.s2!=src0.s2)||(src1.s2!=src1.s2));
	int s3 = ((src0.s3!=src0.s3)||(src1.s3!=src1.s3));
	int s4 = ((src0.s4!=src0.s4)||(src1.s4!=src1.s4));
	int s5 = ((src0.s5!=src0.s5)||(src1.s5!=src1.s5));
	int s6 = ((src0.s6!=src0.s6)||(src1.s6!=src1.s6));
	int s7 = ((src0.s7!=src0.s7)||(src1.s7!=src1.s7));
	return (int8) (-s0,-s1,-s2,-s3,-s4,-s5,-s6,-s7);
	}


float length__f(
	float src0
	)
{
	return viv_fabs(src0);
}


float length__Dv2_f(
	float2 src0
	)
{
	return viv_sqrt(viv_dot(src0,src0));
}


float length__Dv3_f(
	float3 src0
	)
{
	return viv_sqrt(viv_dot(src0,src0));
}


float length__Dv4_f(
	float4 src0
	)
{
	return viv_sqrt(viv_dot(src0,src0));
}


float normalize__f(
	float src0
	)
{
	return 1.0f;
}


float2 normalize__Dv2_f(
	float2 src0
	)
{
	return src0*viv_rsqrt(viv_dot(src0,src0));
}


float3 normalize__Dv3_f(
	float3 src0
	)
{
	return src0*viv_rsqrt(viv_dot(src0,src0));
}


float4 normalize__Dv4_f(
	float4 src0
	)
{
	return src0*viv_rsqrt(viv_dot(src0,src0));
}


float4 read_imagef__11ocl_image2d11ocl_samplerDv2_f(
	image2d_t image,
	sampler_t sampler,
	float2 fcoord
	)
{
	//From libOpenCL/Header Files/gc_cl_sampler.h
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*                 physical;			 /* Pointer to Coordinates */
	};
	int i0,j0,i1,j1;
	int2 coord = (int2)(0,0);

	float4 *c0;
	float a,b,u,v,temp;
	float2 coordTemp;
	float4 t00,t10,t01,t11;

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode =			(uint)(sampler) & 0xF;
	uint filterMode =				(uint)(sampler) & 0x00000F00; //set bit ==> linear
	uint normalizedCoordinates =	(uint)(sampler) & 0x000F0000; //set bit ==> true


	//-----Check addressing mode and filter mode-----//
	// OpenCL 1.2 Specs: Chapter 8.2

	//Possibilities:
		//CL_ADDRESS_NONE
		//CL_ADDRESS_CLAMP_TO_EDGE
		//CL_ADDRESS_CLAMP
		//CL_ADDRESS_REPEAT
		//CL_ADDRESS_MIRRORED_REPEAT

	//Addressing Mode: CL_ADDRESS_REPEAT
	if( addressingMode == (CL_ADDRESS_REPEAT & 0xF) ){
		//Filter Mode: CL_FILTER_NEAREST
		if( !filterMode ){
			coord = (int2)((int) viv_floor(   ( fcoord.x - viv_floor(fcoord.x) ) * (*img0).width   ),(int) viv_floor(   ( fcoord.y - viv_floor(fcoord.y) ) * (*img0).height   ) );
			if(   coord.x  >  ((*img0).width-1)   ){
				coord.x = coord.x - (*img0).width;
			}
			if(  coord.y > ((*img0).height -1) ){
				coord.y = coord.y - (*img0).height;
			}
		}

		//Filter Mode: CL_FILTER_LINEAR
		else{
			u = (  fcoord.x - viv_floor(fcoord.x)  ) * ((*img0).width);
			i0 = (int)viv_floor(u - 0.5f);
			i1 = i0 + 1;
			if (i0 < 0)
				i0 = (*img0).width + i0;
			if (i1 > ((*img0).width - 1 ))
				i1 = i1 - (*img0).width;

			v = (  fcoord.y - viv_floor(fcoord.y)  ) * ((*img0).height);
			j0 = (int)viv_floor(v - 0.5f);
			j1 = j0 + 1;
			if (j0 < 0)
				j0 = (*img0).height + j0;
			if (j1 > ((*img0).height - 1))
				j1 = j1 - (*img0).height;

			fcoord = (float2)(u,v); // pass u and v into fcoord to be used later in FILTER_LINEAR
		}

	}

	//Addressing Mode: CL_ADDRESS_MIRRORED_REPEAT
	else if( addressingMode == (CL_ADDRESS_MIRRORED_REPEAT & 0xF) ){
		//Filter Mode: CL_FILTER_NEAREST
		if( !filterMode ){
			coordTemp = (float2)(    (  2.0f * rint(0.5f * fcoord.x)  )  ,  (  2.0f * rint(0.5f * fcoord.y)  )    );
			coordTemp = (float2)(    (  viv_fabs(fcoord.x - coordTemp.x)  )  ,  (  viv_fabs(fcoord.y - coordTemp.y)  )    );
			coordTemp = (float2)(    (  coordTemp.x * (*img0).width  )  ,  (  coordTemp.y * (*img0).height  )    );
			coord = (int2)(    (  (int) viv_floor(coordTemp.x)  )  ,  (  (int) viv_floor(coordTemp.y)  )    );
			coord = (int2)(    (  viv_min(coord.x,( (*img0).width-1 ))  )  ,  (  viv_min(coord.y,( (*img0).height-1 ))  )    );
		}
		//Filter Mode: CL_FILTER_LINEAR
		else{

			u = viv_fabs(   fcoord.x - (  2.0f * rint(0.5f * fcoord.x) )   ) * (*img0).width;
			i0 = (int)viv_floor( u - 0.5f);
			i1 = i0 + 1;
			i0 = viv_max(i0,0);
			i1 = viv_min(i1, (*img0).width - 1);

			v = viv_fabs(   fcoord.y - (  2.0f * rint(0.5f * fcoord.y) )   ) * (*img0).height;
			j0 = (int)viv_floor( v - 0.5f);
			j1 = j0 + 1;
			j0 = viv_max(j0,0);
			j1 = viv_min(j1, (*img0).height - 1);

			fcoord = (float2)(u,v);
		}
	}

	//Addressing Mode: CL_ADDRESS_CLAMP
	else if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		//-----Check if normalized-----//
		if( normalizedCoordinates){
			fcoord = (float2)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)    );
		}
		//Filter Mode: CL_FILTER_NEAREST
		if( !filterMode ){
			coord = (int2)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 2),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 2));
		}
		//Filter Mode: CL_FILTER_LINEAR
		else{
			i0 = addressing_mode((int)viv_floor(fcoord.x - 0.5) , (*img0).width , 2);
			j0 = addressing_mode((int)viv_floor(fcoord.y - 0.5) , (*img0).height , 2);
			i1 = addressing_mode((int)viv_floor(fcoord.x - 0.5) + 1 , (*img0).width , 2);
			j1 = addressing_mode((int)viv_floor(fcoord.y - 0.5) + 1 , (*img0).height , 2);
		}
	}

	//Addressing Mode: CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE
	else{
		//-----Check if normalized-----//
		if( normalizedCoordinates){
			fcoord = (float2)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)    );
		}
		//Filter Mode: CL_FILTER_NEAREST
		if( !filterMode ){
			coord = (int2)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 0),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 0));
		}
		//Filter Mode: CL_FILTER_LINEAR
		else{
			i0 = addressing_mode((int)viv_floor(fcoord.x - 0.5) , (*img0).width , 0);
			j0 = addressing_mode((int)viv_floor(fcoord.y - 0.5) , (*img0).height , 0);
			i1 = addressing_mode((int)viv_floor(fcoord.x - 0.5) + 1 , (*img0).width , 0);
			j1 = addressing_mode((int)viv_floor(fcoord.y - 0.5) + 1 , (*img0).height , 0);
		}
	}

	if( filterMode) {
		//----Continue CL_FILTER_LINEAR for CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE and CL_ADDRESS_CLAMP-----//
		a = viv_fract(fcoord.x - 0.5, &temp);
		b = viv_fract(fcoord.y - 0.5, &temp);


		if ((*img0).channelDataType==CL_FLOAT && (*img0).channelOrder==CL_RGBA){
			//Calculate t00
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i0 >((*img0).width -1) ) || ( j0 >((*img0).height -1) )
				||  ( i0 < 0 )  || ( j0 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t00 = (float4)(0,0,0,1);
				}*/
				t00 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i0 + ((*img0).rowPitch)/sizeof(float)*j0 );
				t00 = (float4)( *c0);
			}

			//Calculate t10
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i1 >((*img0).width -1) ) || ( j0 >((*img0).height -1) )
				||  ( i1 < 0 )  || ( j0 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t10 = (float4)(0,0,0,1);
				}*/

				t10 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i1 + ((*img0).rowPitch)/sizeof(float)*j0 );
				t10 = (float4)( *c0);
			}

			//Calculate t01
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i0 >((*img0).width -1) ) || ( j1 >((*img0).height -1) )
				||  ( i0 < 0 )  || ( j1 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t01 = (float4)(0,0,0,1);
				}*/

				t01 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i0 + ((*img0).rowPitch)/sizeof(float)*j1 );
				t01 = (float4)( *c0);
			}

			//Calculate t11
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i1 >((*img0).width -1) ) || ( j1 >((*img0).height -1) )
				||  ( i1 < 0 )  || ( j1 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t11 = (float4)(0,0,0,1);
				}*/
				t11 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i1 + ((*img0).rowPitch)/sizeof(float)*j1 );
				t11 = (float4)( *c0);
			}



		}
		else if ((*img0).channelDataType==CL_UNORM_INT8 && ( (*img0).channelOrder==CL_RGBA || (*img0).channelOrder==CL_BGRA ) ){
			//Calculate t00
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i0 >((*img0).width -1) ) || ( j0 >((*img0).height -1) )
				||  ( i0 < 0 )  || ( j0 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t00 = (float4)(0,0,0,1);
				}*/
				t00 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i0 + ((*img0).rowPitch)/sizeof(uchar)*j0 );
				t00 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t10
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i1 >((*img0).width -1) ) || ( j0 >((*img0).height -1) )
				||  ( i1 < 0 )  || ( j0 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t10 = (float4)(0,0,0,1);
				}*/

				t10 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i1 + ((*img0).rowPitch)/sizeof(uchar)*j0 );
				t10 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t01
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i0 >((*img0).width -1) ) || ( j1 >((*img0).height -1) )
				||  ( i0 < 0 )  || ( j1 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t01 = (float4)(0,0,0,1);
				}*/

				t01 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i0 + ((*img0).rowPitch)/sizeof(uchar)*j1 );
				t01 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t11
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i1 >((*img0).width -1) ) || ( j1 >((*img0).height -1) )
				||  ( i1 < 0 )  || ( j1 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t11 = (float4)(0,0,0,1);
				}*/
				t11 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i1 + ((*img0).rowPitch)/sizeof(uchar)*j1 );
				t11 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

		}
		else if ((*img0).channelDataType==CL_UNORM_INT16  && (*img0).channelOrder==CL_RGBA ){
			//Calculate t00
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i0 >((*img0).width -1) ) || ( j0 >((*img0).height -1) )
				||  ( i0 < 0 )  || ( j0 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t00 = (float4)(0,0,0,1);
				}*/
				t00 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(ushort)*j0 );
				t00 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t10
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i1 >((*img0).width -1) ) || ( j0 >((*img0).height -1) )
				||  ( i1 < 0 )  || ( j0 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t10 = (float4)(0,0,0,1);
				}*/

				t10 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(ushort)*j0 );
				t10 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t01
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i0 >((*img0).width -1) ) || ( j1 >((*img0).height -1) )
				||  ( i0 < 0 )  || ( j1 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t01 = (float4)(0,0,0,1);
				}*/

				t01 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(ushort)*j1 );
				t01 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t11
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i1 >((*img0).width -1) ) || ( j1 >((*img0).height -1) )
				||  ( i1 < 0 )  || ( j1 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t11 = (float4)(0,0,0,1);
				}*/
				t11 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(ushort)*j1 );
				t11 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}
		}


		else if ((*img0).channelDataType==CL_HALF_FLOAT  && (*img0).channelOrder==CL_RGBA){
			//Calculate t00
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i0 >((*img0).width -1) ) || ( j0 >((*img0).height -1) )
				||  ( i0 < 0 )  || ( j0 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t00 = (float4)(0,0,0,1);
				}*/
				t00 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(half)*j0 );
				t00 = vload_half4(0,c0);
			}

			//Calculate t10
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i1 >((*img0).width -1) ) || ( j0 >((*img0).height -1) )
				||  ( i1 < 0 )  || ( j0 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t10 = (float4)(0,0,0,1);
				}*/

				t10 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(half)*j0 );
				t10 = vload_half4(0,c0);
			}

			//Calculate t01
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i0 >((*img0).width -1) ) || ( j1 >((*img0).height -1) )
				||  ( i0 < 0 )  || ( j1 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t01 = (float4)(0,0,0,1);
				}*/

				t01 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(half)*j1 );
				t01 = vload_half4(0,c0);
			}

			//Calculate t11
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( i1 >((*img0).width -1) ) || ( j1 >((*img0).height -1) )
				||  ( i1 < 0 )  || ( j1 < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t11 = (float4)(0,0,0,1);
				}*/
				t11 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(half)*j1 );
				t11 = vload_half4(0,c0);
			}

		}


		return (float4) (		(1-a)	*	(1-b)	*	t00
							+	  a		*	(1-b)	*	t10
							+	(1-a)	*	  b		*	t01
							+	  a		*	  b		*	t11		);
		//-----END CL_FILTER_LINEAR for CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE and CL_ADDRESS_CLAMP-----//
	}

	//-----End Addressing and Filter Mode processing-----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (float4)(0,0,0,1);
		}*/

		return (float4)(0,0,0,0);
	}

	if ((*img0).channelDataType==CL_FLOAT && (*img0).channelOrder==CL_RGBA){

		//-----Fetch the 4 image elements----//
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1); // adjust address offset based on coordinate
		return (float4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8 && ( (*img0).channelOrder==CL_RGBA  ||  (*img0).channelOrder==CL_BGRA ) ){

		//-----Fetch the 4 image elements----//
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1); // adjust address offset based on coordinate
		return (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16 && (*img0).channelOrder==CL_RGBA){

		//-----Fetch the 4 image elements----//
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1); // adjust address offset based on coordinate
		return (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT && (*img0).channelOrder==CL_RGBA){

		//-----Fetch the 4 image elements----//
		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1); // adjust address offset based on coordinate
		return vload_half4(0, c0);
	}
}









float4 read_imagef__11ocl_image2d11ocl_samplerDv4_f(
	image2d_t image,
	sampler_t sampler,
	float4 fcoord
	)
{
	//From libOpenCL/Header Files/gc_cl_sampler.h
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*                 physical;			 /* Pointer to Coordinates */
	};
	int i0,j0,i1,j1,k0,k1;
	int4 coord = (int4)(0,0,0,0);

	float4 *c0;
	float a,b,c,u,v,w,temp;
	float4 coordTemp;
	float4 t000,t100,t010,t110,t001,t101,t011,t111;

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode =			(uint)(sampler) & 0xF;
	uint filterMode =				(uint)(sampler) & 0x00000F00; //set bit ==> linear
	uint normalizedCoordinates =	(uint)(sampler) & 0x000F0000; //set bit ==> true


	//-----Check addressing mode and filter mode-----//
	// OpenCL 1.2 Specs: Chapter 8.2

	//Possibilities:
		//CL_ADDRESS_NONE
		//CL_ADDRESS_CLAMP_TO_EDGE
		//CL_ADDRESS_CLAMP
		//CL_ADDRESS_REPEAT
		//CL_ADDRESS_MIRRORED_REPEAT

	//Addressing Mode: CL_ADDRESS_REPEAT
	if( addressingMode == (CL_ADDRESS_REPEAT & 0xF) ){
		//Filter Mode: CL_FILTER_NEAREST
		if( !filterMode ){
			coord.xyz = (int3)((int) viv_floor(   ( fcoord.x - viv_floor(fcoord.x) ) * (*img0).width   ),(int) viv_floor(   ( fcoord.y - viv_floor(fcoord.y) ) * (*img0).height   ),(int) viv_floor(   ( fcoord.z - viv_floor(fcoord.z) ) * (*img0).depth   ) );
			if(   coord.x  >  ((*img0).width-1 ) ){
				coord.x = coord.x - (*img0).width;
			}
			if(  coord.y > ((*img0).height -1) ){
				coord.y = coord.y - (*img0).height;
			}
			if(  coord.z > ((*img0).depth -1) ){
				coord.z = coord.z - (*img0).depth;
			}
		}

		//Filter Mode: CL_FILTER_LINEAR
		else{
			u = (  fcoord.x - viv_floor(fcoord.x)  ) * ((*img0).width);
			i0 = (int)viv_floor(u - 0.5f);
			i1 = i0 + 1;
			if (i0 < 0)
				i0 = (*img0).width + i0;
			if (i1 > ((*img0).width - 1 ))
				i1 = i1 - (*img0).width;

			v = (  fcoord.y - viv_floor(fcoord.y)  ) * ((*img0).height);
			j0 = (int)viv_floor(v - 0.5f);
			j1 = j0 + 1;
			if (j0 < 0)
				j0 = (*img0).height + j0;
			if (j1 > ((*img0).height - 1))
				j1 = j1 - (*img0).height;

			w = (  fcoord.z - viv_floor(fcoord.z)  ) * ((*img0).depth);
			k0 = (int)viv_floor(w - 0.5f);
			k1 = k0 + 1;
			if (k0 < 0)
				k0 = (*img0).depth + k0;
			if (k1 > ((*img0).depth - 1))
				k1 = k1 - (*img0).depth;

			fcoord.xyz = (float3)(u,v,w); // pass u , v , w into fcoord to be used later in FILTER_LINEAR
		}

	}

	//Addressing Mode: CL_ADDRESS_MIRRORED_REPEAT
	else if( addressingMode == (CL_ADDRESS_MIRRORED_REPEAT & 0xF) ){
		//Filter Mode: CL_FILTER_NEAREST
		if( !filterMode ){
			coordTemp.xyz = (float3)(    (  2.0f * rint(0.5f * fcoord.x)  )  ,  (  2.0f * rint(0.5f * fcoord.y)  )  ,  (  2.0f * rint(0.5f * fcoord.z)  )    );
			coordTemp.xyz = (float3)(    (  viv_fabs(fcoord.x - coordTemp.x)  )  ,  (  viv_fabs(fcoord.y - coordTemp.y)  )  ,  (  viv_fabs(fcoord.z - coordTemp.z)  )    );
			coordTemp.xyz = (float3)(    (  coordTemp.x * (*img0).width  )  ,  (  coordTemp.y * (*img0).height  )  ,  (  coordTemp.z * (*img0).depth  )    );
			coord.xyz = (int3)(    (  (int) viv_floor(coordTemp.x)  )  ,  (  (int) viv_floor(coordTemp.y)  )  ,  (  (int) viv_floor(coordTemp.z)  )    );
			coord.xyz = (int3)(    (  viv_min(coord.x,( (*img0).width-1 ))  )  ,  (  viv_min(coord.y,( (*img0).height-1 ))  )  ,  (  viv_min(coord.z,( (*img0).depth-1 ))  )    );
		}
		//Filter Mode: CL_FILTER_LINEAR
		else{

			u = viv_fabs(   fcoord.x - (  2.0f * rint(0.5f * fcoord.x) )   ) * (*img0).width;
			i0 = (int)viv_floor( u - 0.5f);
			i1 = i0 + 1;
			i0 = viv_max(i0,0);
			i1 = viv_min(i1, (*img0).width - 1);

			v = viv_fabs(   fcoord.y - (  2.0f * rint(0.5f * fcoord.y) )   ) * (*img0).height;
			j0 = (int)viv_floor( v - 0.5f);
			j1 = j0 + 1;
			j0 = viv_max(j0,0);
			j1 = viv_min(j1, (*img0).height - 1);

			w = viv_fabs(   fcoord.z - (  2.0f * rint(0.5f * fcoord.z) )   ) * (*img0).depth;
			k0 = (int)viv_floor( w - 0.5f);
			k1 = k0 + 1;
			k0 = viv_max(k0,0);
			k1 = viv_min(k1, (*img0).depth - 1);

			fcoord.xyz = (float3)(u,v,w);
		}
	}

	//Addressing Mode: CL_ADDRESS_CLAMP
	else if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		//-----Check if normalized-----//
		if( normalizedCoordinates){
			fcoord.xyz = (float3)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)  ,  (fcoord.z * (*img0).height)    );
		}
		//Filter Mode: CL_FILTER_NEAREST
		if( !filterMode ){
			coord.xyz = (int3)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 2),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 2),addressing_mode((int)viv_floor(fcoord.z),(*img0).depth, 2));
		}
		//Filter Mode: CL_FILTER_LINEAR
		else{
			i0 = addressing_mode((int)viv_floor(fcoord.x - 0.5) , (*img0).width , 2);
			j0 = addressing_mode((int)viv_floor(fcoord.y - 0.5) , (*img0).height , 2);
			k0 = addressing_mode((int)viv_floor(fcoord.z - 0.5) , (*img0).depth , 2);
			i1 = addressing_mode((int)viv_floor(fcoord.x - 0.5) + 1 , (*img0).width , 2);
			j1 = addressing_mode((int)viv_floor(fcoord.y - 0.5) + 1 , (*img0).height , 2);
			k1 = addressing_mode((int)viv_floor(fcoord.z - 0.5) + 1 , (*img0).depth , 2);
		}
	}

	//Addressing Mode: CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE
	else{
		//-----Check if normalized-----//
		if( normalizedCoordinates){
			fcoord.xyz = (float3)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)  ,  (fcoord.z * (*img0).height)    );
		}
		//Filter Mode: CL_FILTER_NEAREST
		if( !filterMode ){
			coord.xyz = (int3)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 0)  ,  addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 0)  ,  addressing_mode((int)viv_floor(fcoord.z),(*img0).depth, 0));
		}
		//Filter Mode: CL_FILTER_LINEAR
		else{
			i0 = addressing_mode((int)viv_floor(fcoord.x - 0.5) , (*img0).width , 0);
			j0 = addressing_mode((int)viv_floor(fcoord.y - 0.5) , (*img0).height , 0);
			k0 = addressing_mode((int)viv_floor(fcoord.z - 0.5) , (*img0).depth , 0);
			i1 = addressing_mode((int)viv_floor(fcoord.x - 0.5) + 1 , (*img0).width , 0);
			j1 = addressing_mode((int)viv_floor(fcoord.y - 0.5) + 1 , (*img0).height , 0);
			j1 = addressing_mode((int)viv_floor(fcoord.z - 0.5) + 1 , (*img0).depth , 0);
		}
	}

	if( filterMode) {
		//----Continue CL_FILTER_LINEAR for CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE and CL_ADDRESS_CLAMP-----//
		a = viv_fract(fcoord.x - 0.5, &temp);
		b = viv_fract(fcoord.y - 0.5, &temp);
		c = viv_fract(fcoord.z - 0.5, &temp);


		if ((*img0).channelDataType==CL_FLOAT && (*img0).channelOrder==CL_RGBA){
			//Calculate t000
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t000 = (float4)(0,0,0,1);
				}*/
				t000 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i0 + ((*img0).rowPitch)/sizeof(float)*j0 + ((*img0).slicePitch)/sizeof(float)*k0 );
				t000 = (float4)( *c0);
			}

			//Calculate t100
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t100 = (float4)(0,0,0,1);
				}*/

				t100 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i1 + ((*img0).rowPitch)/sizeof(float)*j0 + ((*img0).slicePitch)/sizeof(float)*k0);
				t100 = (float4)( *c0);
			}

			//Calculate t010
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t010 = (float4)(0,0,0,1);
				}*/

				t010 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i0 + ((*img0).rowPitch)/sizeof(float)*j1 + ((*img0).slicePitch)/sizeof(float)*k0 );
				t010 = (float4)( *c0);
			}

			//Calculate t110
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t110 = (float4)(0,0,0,1);
				}*/
				t110 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i1 + ((*img0).rowPitch)/sizeof(float)*j1 + ((*img0).slicePitch)/sizeof(float)*k0 );
				t110 = (float4)( *c0);
			}

			//Calculate t001
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t001 = (float4)(0,0,0,1);
				}*/
				t001 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i0 + ((*img0).rowPitch)/sizeof(float)*j0 + ((*img0).slicePitch)/sizeof(float)*k1 );
				t001 = (float4)( *c0);
			}

			//Calculate t101
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t101 = (float4)(0,0,0,1);
				}*/

				t101 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i1 + ((*img0).rowPitch)/sizeof(float)*j0 + ((*img0).slicePitch)/sizeof(float)*k1);
				t101 = (float4)( *c0);
			}

			//Calculate t011
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t011 = (float4)(0,0,0,1);
				}*/

				t011 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i0 + ((*img0).rowPitch)/sizeof(float)*j1 + ((*img0).slicePitch)/sizeof(float)*k1 );
				t011 = (float4)( *c0);
			}

			//Calculate t111
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t111 = (float4)(0,0,0,1);
				}*/
				t111 = (float4)(0,0,0,0);
			}
			else{
				float4* c0 = (*img0).physical + 4*( i1 + ((*img0).rowPitch)/sizeof(float)*j1 + ((*img0).slicePitch)/sizeof(float)*k1 );
				t111 = (float4)( *c0);
			}

		}
		else if ((*img0).channelDataType==CL_UNORM_INT8 && ( (*img0).channelOrder==CL_RGBA || (*img0).channelOrder==CL_BGRA ) ){
			//Calculate t000
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t000 = (float4)(0,0,0,1);
				}*/
				t000 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i0 + ((*img0).rowPitch)/sizeof(uchar)*j0 + ((*img0).slicePitch)/sizeof(uchar)*k0 );
				t000 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t100
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t100 = (float4)(0,0,0,1);
				}*/

				t100 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i1 + ((*img0).rowPitch)/sizeof(uchar)*j0 + ((*img0).slicePitch)/sizeof(uchar)*k0 );
				t100 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t010
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t010 = (float4)(0,0,0,1);
				}*/

				t010 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i0 + ((*img0).rowPitch)/sizeof(uchar)*j1 + ((*img0).slicePitch)/sizeof(uchar)*k0 );
				t010 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t110
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t110 = (float4)(0,0,0,1);
				}*/
				t110 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i1 + ((*img0).rowPitch)/sizeof(uchar)*j1 + ((*img0).slicePitch)/sizeof(uchar)*k0 );
				t110 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t001
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t001 = (float4)(0,0,0,1);
				}*/
				t001 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i0 + ((*img0).rowPitch)/sizeof(uchar)*j0 + ((*img0).slicePitch)/sizeof(uchar)*k1 );
				t001 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t101
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t101 = (float4)(0,0,0,1);
				}*/

				t101 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i1 + ((*img0).rowPitch)/sizeof(uchar)*j0 + ((*img0).slicePitch)/sizeof(uchar)*k1 );
				t101 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t011
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t011 = (float4)(0,0,0,1);
				}*/

				t011 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i0 + ((*img0).rowPitch)/sizeof(uchar)*j1 + ((*img0).slicePitch)/sizeof(uchar)*k1 );
				t011 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

			//Calculate t111
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t111 = (float4)(0,0,0,1);
				}*/
				t111 = (float4)(0,0,0,0);
			}
			else{
				uchar4* c0 = (*img0).physical + ( i1 + ((*img0).rowPitch)/sizeof(uchar)*j1 + ((*img0).slicePitch)/sizeof(uchar)*k1 );
				t111 = (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
			}

		}
		else if ((*img0).channelDataType==CL_UNORM_INT16  && (*img0).channelOrder==CL_RGBA ){
			//Calculate t000
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t000 = (float4)(0,0,0,1);
				}*/
				t000 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(ushort)*j0 + ((*img0).slicePitch)/sizeof(ushort)*k0 );
				t000 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t100
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t100 = (float4)(0,0,0,1);
				}*/

				t100 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(ushort)*j0 + ((*img0).slicePitch)/sizeof(ushort)*k0 );
				t100 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t010
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t010 = (float4)(0,0,0,1);
				}*/

				t010 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(ushort)*j1 + ((*img0).slicePitch)/sizeof(ushort)*k0 );
				t010 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t110
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t110 = (float4)(0,0,0,1);
				}*/
				t110 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(ushort)*j1 + ((*img0).slicePitch)/sizeof(ushort)*k0 );
				t110 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t001
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t001 = (float4)(0,0,0,1);
				}*/
				t001 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(ushort)*j0 + ((*img0).slicePitch)/sizeof(ushort)*k1 );
				t001 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t101
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t101 = (float4)(0,0,0,1);
				}*/

				t101 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(ushort)*j0 + ((*img0).slicePitch)/sizeof(ushort)*k1 );
				t101 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t011
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t011 = (float4)(0,0,0,1);
				}*/

				t011 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(ushort)*j1 + ((*img0).slicePitch)/sizeof(ushort)*k1 );
				t011 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}

			//Calculate t111
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t111 = (float4)(0,0,0,1);
				}*/
				t111 = (float4)(0,0,0,0);
			}
			else{
				ushort4* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(ushort)*j1 + ((*img0).slicePitch)/sizeof(ushort)*k1 );
				t111 = (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
			}
		}


		else if ((*img0).channelDataType==CL_HALF_FLOAT  && (*img0).channelOrder==CL_RGBA){
			//Calculate t000
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t000 = (float4)(0,0,0,1);
				}*/
				t000 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(half)*j0 + ((*img0).slicePitch)/sizeof(half)*k0 );
				t000 = vload_half4(0,c0);
			}

			//Calculate t100
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t100 = (float4)(0,0,0,1);
				}*/

				t100 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(half)*j0 + ((*img0).slicePitch)/sizeof(half)*k0 );
				t100 = vload_half4(0,c0);
			}

			//Calculate t010
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t010 = (float4)(0,0,0,1);
				}*/

				t010 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(half)*j1 + ((*img0).slicePitch)/sizeof(half)*k0 );
				t010 = vload_half4(0,c0);
			}

			//Calculate t110
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t110 = (float4)(0,0,0,1);
				}*/
				t110 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(half)*j1 + ((*img0).slicePitch)/sizeof(half)*k0 );
				t110 = vload_half4(0,c0);
			}

			//Calculate t001
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t001 = (float4)(0,0,0,1);
				}*/
				t001 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(half)*j0 + ((*img0).slicePitch)/sizeof(half)*k1 );
				t001 = vload_half4(0,c0);
			}

			//Calculate t101
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t101 = (float4)(0,0,0,1);
				}*/

				t101 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(half)*j0 + ((*img0).slicePitch)/sizeof(half)*k1 );
				t101 = vload_half4(0,c0);
			}

			//Calculate t011
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t011 = (float4)(0,0,0,1);
				}*/

				t011 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i0 + ((*img0).rowPitch)/sizeof(half)*j1 + ((*img0).slicePitch)/sizeof(half)*k1 );
				t011 = vload_half4(0,c0);
			}

			//Calculate t111
			//Check if texel location is outside of the image, if so, then return the border color
			if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

				//*****NOTE: not required until a format below is supported*****//
				// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
				/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
					||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
						t111 = (float4)(0,0,0,1);
				}*/
				t111 = (float4)(0,0,0,0);
			}
			else{
				half* c0 = (*img0).physical + 2*( i1 + ((*img0).rowPitch)/sizeof(half)*j1 + ((*img0).slicePitch)/sizeof(half)*k1 );
				t111 = vload_half4(0,c0);
			}

		}


		return (float4) (		(1-a)	*	(1-b)	*	(1-c)	*	t000
							+	  a		*	(1-b)	*	(1-c)	*	t100
							+	(1-a)	*	  b		*	(1-c)	*	t010
							+	  a		*	  b		*	(1-c)	*	t110
							+	(1-a)	*	(1-b)	*	  c		*	t001
							+	  a		*	(1-b)	*	  c		*	t101
							+	(1-a)	*	  b		*	  c		*	t011
							+	  a		*	  b		*	  c		*	t111	);
		//-----END CL_FILTER_LINEAR for CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE and CL_ADDRESS_CLAMP-----//
	}

	//-----End Addressing and Filter Mode processing-----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
				||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (float4)(0,0,0,1);
		}*/

		return (float4)(0,0,0,0);
	}

	if ((*img0).channelDataType==CL_FLOAT && (*img0).channelOrder==CL_RGBA){

		//-----Fetch the 4 image elements----//
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1+((*img0).slicePitch)/sizeof(float)*coord.s2); // adjust address offset based on coordinate
		return (float4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8 && ( (*img0).channelOrder==CL_RGBA  ||  (*img0).channelOrder==CL_BGRA ) ){

		//-----Fetch the 4 image elements----//
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2); // adjust address offset based on coordinate
		return (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16 && (*img0).channelOrder==CL_RGBA){

		//-----Fetch the 4 image elements----//
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2); // adjust address offset based on coordinate
		return (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT && (*img0).channelOrder==CL_RGBA){

		//-----Fetch the 4 image elements----//
		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1+((*img0).slicePitch)/sizeof(half)*coord.s2); // adjust address offset based on coordinate
		return vload_half4(0, c0);
	}
}



float4 read_imagef__11ocl_image2d11ocl_samplerDv2_i(
	image2d_t image,
	sampler_t sampler,
	int2 coord
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*               physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if ( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){

		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE or undefined behavior//
	else {
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
	}

	//-----End Addressing-Mode Processing-----//


	if ((*img0).channelDataType==CL_FLOAT){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1.0f);
			}*/

			return (float4)(0,0,0,0);

		}
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1); // adjust address offset based on coordinate
		return (float4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1); // adjust address offset based on coordinate
		//return convert_float4((*c0)/65535.0f);
		return (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1); // adjust address offset based on coordinate
		return (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT){ //check to make channelDataType is supported
		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1); // adjust address offset based on coordinate
		return (float4)(vload_half(0,c0),vload_half(1,c0),vload_half(2,c0),vload_half(3,c0));
		//return vload_half4(0,c0);
	}

}









float4 read_imagef__11ocl_image2d11ocl_samplerDv4_i(
	image2d_t image,
	sampler_t sampler,
	int4 coord
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*               physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode = (uint)(sampler) & 0xF;
	coord.z = viv_clamp(coord.z,0,(*img0).depth-1);

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if ( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){

		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE or undefined behavior//
	else {
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
	}

	//-----End Addressing-Mode Processing-----//


	if ((*img0).channelDataType==CL_FLOAT){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1.0f);
			}*/

			return (float4)(0,0,0,0);

		}
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1+((*img0).slicePitch)/sizeof(float)*coord.s2); // adjust address offset based on coordinate
		return (float4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2); // adjust address offset based on coordinate
		//return convert_float4((*c0)/65535.0f);
		return (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2); // adjust address offset based on coordinate
		return (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT){ //check to make channelDataType is supported
		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1+((*img0).slicePitch)/sizeof(half)*coord.s2);; // adjust address offset based on coordinate
		return vload_half4(0,c0);
	}

}



float4 read_imagef__11ocl_image3d11ocl_samplerDv4_i(
	image3d_t image,
	sampler_t sampler,
	int4 coord
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*               physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if ( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){

		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
		coord.z = viv_clamp(coord.z, -1, (*img0).depth);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE or undefined behavior//
	else {
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
		coord.z = viv_clamp(coord.z, 0, (*img0).depth -1);
	}

	//-----End Addressing-Mode Processing-----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
	||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (float4)(0,0,0,1.0f);
		}*/

		return (float4)(0,0,0,0);

	}

	if ((*img0).channelDataType==CL_FLOAT){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1+((*img0).slicePitch)/sizeof(float)*coord.s2 ); // adjust address offset based on coordinate
		return (float4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2 );  // adjust address offset based on coordinate
		//return convert_float4((*c0)/65535.0f);
		return (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2 );  // adjust address offset based on coordinate
		return (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT){ //check to make channelDataType is supported
		//-----Fetch the 4 image elements----//
		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1+((*img0).slicePitch)/sizeof(half)*coord.s2 );  // adjust address offset based on coordinate
		return (float4)(vload_half(0,c0),vload_half(1,c0),vload_half(2,c0),vload_half(3,c0));
		//return vload_half4(0,c0);
	}

}



float4 read_imagef__11ocl_image3dDv4_i(
	image3d_t image,
	int4 coord
	)
{
	sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE| CLK_FILTER_NEAREST;
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*               physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if ( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){

		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
		coord.z = viv_clamp(coord.z, -1, (*img0).depth);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE or undefined behavior//
	else {
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
		coord.z = viv_clamp(coord.z, 0, (*img0).depth -1);
	}

	//-----End Addressing-Mode Processing-----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
	||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (float4)(0,0,0,1.0f);
		}*/

		return (float4)(0,0,0,0);

	}

	if ((*img0).channelDataType==CL_FLOAT){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1+((*img0).slicePitch)/sizeof(float)*coord.s2 ); // adjust address offset based on coordinate
		return (float4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2 );  // adjust address offset based on coordinate
		//return convert_float4((*c0)/65535.0f);
		return (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2 );  // adjust address offset based on coordinate
		return (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT){ //check to make channelDataType is supported
		//-----Fetch the 4 image elements----//
		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1+((*img0).slicePitch)/sizeof(half)*coord.s2 );  // adjust address offset based on coordinate
		return (float4)(vload_half(0,c0),vload_half(1,c0),vload_half(2,c0),vload_half(3,c0));
		//return vload_half4(0,c0);
	}

}



float4 read_imagef__11ocl_image2dDv2_i(
	image2d_t image,
	int2 coord
	)
{
	sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*               physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if ( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){

		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE or undefined behavior//
	else {
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
	}

	//-----End Addressing-Mode Processing-----//


	if ((*img0).channelDataType==CL_FLOAT){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1.0f);
			}*/

			return (float4)(0,0,0,0);

		}
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1); // adjust address offset based on coordinate
		return (float4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1); // adjust address offset based on coordinate
		//return convert_float4((*c0)/65535.0f);
		return (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1); // adjust address offset based on coordinate
		return (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT){ //check to make channelDataType is supported
		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1); // adjust address offset based on coordinate
		return (float4)(vload_half(0,c0),vload_half(1,c0),vload_half(2,c0),vload_half(3,c0));
		//return vload_half4(0,c0);
	}

}









float4 read_imagef__11ocl_image2dDv4_i(
	image2d_t image,
	int4 coord
	)
{
	sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*               physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode = (uint)(sampler) & 0xF;
	coord.z = viv_clamp(coord.z,0,(*img0).depth-1);

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if ( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){

		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE or undefined behavior//
	else {
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
	}

	//-----End Addressing-Mode Processing-----//


	if ((*img0).channelDataType==CL_FLOAT){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1.0f);
			}*/

			return (float4)(0,0,0,0);

		}
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1+((*img0).slicePitch)/sizeof(float)*coord.s2); // adjust address offset based on coordinate
		return (float4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2); // adjust address offset based on coordinate
		//return convert_float4((*c0)/65535.0f);
		return (float4)((*c0).x/65535.0f,(*c0).y/65535.0f,(*c0).z/65535.0f,(*c0).w/65535.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8){ //check to make channelDataType is supported

		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2); // adjust address offset based on coordinate
		return (float4)((*c0).x/255.0f,(*c0).y/255.0f,(*c0).z/255.0f,(*c0).w/255.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT){ //check to make channelDataType is supported
		//-----Fetch the 4 image elements----//

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (float4)(0,0,0,1);
			}*/

			return (float4)(0,0,0,0.0f);

		}
		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1+((*img0).slicePitch)/sizeof(half)*coord.s2);; // adjust address offset based on coordinate
		return vload_half4(0,c0);
	}

}



int4 read_imagei__11ocl_image2d11ocl_samplerDv2_f(
	image2d_t image,
	sampler_t sampler,
	float2 fcoord
	)
{
	//From libOpenCL/Header Files/gc_cl_sampler.h
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};
	int2 coord = (int2)(0,0);
	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image
	float2 coordTemp;

	uint addressingMode =			(uint)(sampler) & 0xF;
	uint normalizedCoordinates =	(uint)(sampler) & 0x000F0000; //set bit ==> true


	//-----Check addressing mode and filter mode-----//
	// OpenCL 1.2 Specs: Chapter 8.2


	//Possibilities:
		//CL_ADDRESS_NONE
		//CL_ADDRESS_CLAMP_TO_EDGE
		//CL_ADDRESS_CLAMP
		//CL_ADDRESS_REPEAT
		//CL_ADDRESS_MIRRORED_REPEAT

		//Addressing Mode: CL_ADDRESS_REPEAT
		if( addressingMode == (CL_ADDRESS_REPEAT & 0xF) ){
			//Filter Mode: CL_FILTER_NEAREST
			coord = (int2)((int) viv_floor(   ( fcoord.x - viv_floor(fcoord.x) ) * (*img0).width   ),(int) viv_floor(   ( fcoord.y - viv_floor(fcoord.y) ) * (*img0).height   ) );
			if(   coord.x  >  ((*img0).width-1)   ){
				coord.x = coord.x - (*img0).width;
			}
			if(  coord.y > ((*img0).height -1) ){
				coord.y = coord.y - (*img0).height;
			}
		}


		//Addressing Mode: CL_ADDRESS_MIRRORED_REPEAT
		else if( addressingMode == (CL_ADDRESS_MIRRORED_REPEAT & 0xF) ){
			//Filter ModeL CL_FILTER_NEAREST
			coordTemp = (float2)(    (  2.0f * rint(0.5f * fcoord.x)  )  ,  (  2.0f * rint(0.5f * fcoord.y)  )    );
			coordTemp = (float2)(    (  viv_fabs(fcoord.x - coordTemp.x)  )  ,  (  viv_fabs(fcoord.y - coordTemp.y)  )    );
			coordTemp = (float2)(    (  coordTemp.x * (*img0).width  )  ,  (  coordTemp.y * (*img0).height  )    );
			coord = (int2)(    (  (int) viv_floor(coordTemp.x)  )  ,  (  (int) viv_floor(coordTemp.y)  )    );
			coord = (int2)(    (  viv_min(coord.x,( (*img0).width-1 ))  )  ,  (  viv_min(coord.y,( (*img0).height-1 ))  )    );
		}



		//Addressing Mode: CL_ADDRESS_CLAMP
		else if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord = (float2)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord = (int2)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 2),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 2));
		}


		//Addressing Mode: CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE
		else{
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord = (float2)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord = (int2)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 0),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 0));
		}

	//-----End Addressing and Filter Mode processing-----//


	if ((*img0).channelDataType==CL_SIGNED_INT32){

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){
			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (int4)(0,0,0,1);
			}*/
			return (int4)(0,0,0,0);
		}

		//-----Fetch the 4 image elements----//
		int4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(int)*coord.s1); // adjust address offset based on coordinate
		return ( *c0 );
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT16){

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){
			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (int4)(0,0,0,1);
			}*/
			return (int4)(0,0,0,0);
		}

		//-----Fetch the 4 image elements----//
		short4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(short)*coord.s1); // adjust address offset based on coordinate
		return convert_int4( *c0 );
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT8){

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){
			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (int4)(0,0,0,1);
			}*/
			return (int4)(0,0,0,0);
		}

		//-----Fetch the 4 image elements----//
		char4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(char)*coord.s1); // adjust address offset based on coordinate
		return convert_int4( *c0 );
	}
}




int4 read_imagei__11ocl_image3d11ocl_samplerDv4_f(
	image3d_t image,
	sampler_t sampler,
	float4 fcoord
	)
{
	//From libOpenCL/Header Files/gc_cl_sampler.h
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};
	int4 coord = (int4)(0,0,0,0);
	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image
	float4 coordTemp;

	uint addressingMode =			(uint)(sampler) & 0xF;
	uint normalizedCoordinates =	(uint)(sampler) & 0x000F0000; //set bit ==> true


	//-----Check addressing mode and filter mode-----//
	// OpenCL 1.2 Specs: Chapter 8.2


	//Possibilities:
		//CL_ADDRESS_NONE
		//CL_ADDRESS_CLAMP_TO_EDGE
		//CL_ADDRESS_CLAMP
		//CL_ADDRESS_REPEAT
		//CL_ADDRESS_MIRRORED_REPEAT

		//Addressing Mode: CL_ADDRESS_REPEAT
		if( addressingMode == (CL_ADDRESS_REPEAT & 0xF) ){
			//Filter Mode: CL_FILTER_NEAREST
			coord.xy = (int2)((int) viv_floor(   ( fcoord.x - viv_floor(fcoord.x) ) * (*img0).width   ),(int) viv_floor(   ( fcoord.y - viv_floor(fcoord.y) ) * (*img0).height   ) );
			coord.z = (int) viv_floor(   ( fcoord.z - viv_floor(fcoord.z) ) * (*img0).depth  );
			if(   coord.x  >  ((*img0).width-1)   ){
				coord.x = coord.x - (*img0).width;
			}
			if(  coord.y > ((*img0).height -1) ){
				coord.y = coord.y - (*img0).height;
			}
			if(  coord.z > ((*img0).depth -1) ){
				coord.z = coord.z - (*img0).depth;
			}
		}


		//Addressing Mode: CL_ADDRESS_MIRRORED_REPEAT
		else if( addressingMode == (CL_ADDRESS_MIRRORED_REPEAT & 0xF) ){
			//Filter ModeL CL_FILTER_NEAREST
			coordTemp.xyz = (float3)(    (  2.0f * rint(0.5f * fcoord.x)  )  ,  (  2.0f * rint(0.5f * fcoord.y)  )  ,  (  2.0f * rint(0.5f * fcoord.z)  )    );
			coordTemp.xyz = (float3)(    (  viv_fabs(fcoord.x - coordTemp.x)  )  ,  (  viv_fabs(fcoord.y - coordTemp.y)  )  ,  (  viv_fabs(fcoord.z - coordTemp.z)  )    );
			coordTemp.xyz = (float3)(    (  coordTemp.x * (*img0).width  )  ,  (  coordTemp.y * (*img0).height  )  ,  (  coordTemp.z * (*img0).depth  )    );
			coord.xyz = (int3)(    (  (int) viv_floor(coordTemp.x)  )  ,  (  (int) viv_floor(coordTemp.y)  )  ,  (  (int) viv_floor(coordTemp.z)  )    );
			coord.xyz = (int3)(    (  viv_min(coord.x,( (*img0).width-1 ))  )  ,  (  viv_min(coord.y,( (*img0).height-1 ))  )  ,  (  viv_min(coord.z,( (*img0).depth-1 ))  )    );
		}



		//Addressing Mode: CL_ADDRESS_CLAMP
		else if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord.xyz = (float3)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)  ,  (fcoord.z * (*img0).depth)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord.xyz = (int3)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 2),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 2),addressing_mode((int)viv_floor(fcoord.z),(*img0).depth, 2));
		}


		//Addressing Mode: CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE
		else{
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord.xyz = (float3)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)  ,  (fcoord.z * (*img0).depth)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord.xyz = (int3)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 0),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 0),addressing_mode((int)viv_floor(fcoord.z),(*img0).depth, 0));
		}

	//-----End Addressing and Filter Mode processing-----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){
		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (int4)(0,0,0,1);
		}*/
		return (int4)(0,0,0,0);
	}

	if ((*img0).channelDataType==CL_SIGNED_INT32){

		//-----Fetch the 4 image elements----//
		int4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(int)*coord.s1+((*img0).slicePitch)/sizeof(int)*coord.s2 ); // adjust address offset based on coordinate
		return ( *c0 );
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT16){

		//-----Fetch the 4 image elements----//
		short4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(short)*coord.s1+((*img0).slicePitch)/sizeof(short)*coord.s2 ); // adjust address offset based on coordinate
		return convert_int4( *c0 );
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT8){

		//-----Fetch the 4 image elements----//
		char4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(char)*coord.s1+((*img0).slicePitch)/sizeof(char)*coord.s2 ); // adjust address offset based on coordinate
		return convert_int4( *c0 );
	}
}





int4 read_imagei__11ocl_image2d11ocl_samplerDv2_i(
	image2d_t image,
	sampler_t sampler,
	int2 coord
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode = (uint)(sampler) & 0xF;


	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//
	// TODO: use sampler to check addressing mode

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE


	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);

	}
	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE//
	else{
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);

	}
	//-----End Addressing-Mode Processing-----//


	//-----Fetch the 4 image elements----//


	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (int4)(0,0,0,1);
		}*/

		return (int4)(0,0,0,0);
	}


	if ((*img0).channelDataType==CL_SIGNED_INT32){
		int4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(int)*coord.s1); // adjust address offset based on coordinate
		return (int4)( *c0);
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT16){
		short4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(short)*coord.s1); // adjust address offset based on coordinate
		return convert_int4( *c0);
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT8){
		char4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(char)*coord.s1); // adjust address offset based on coordinate
		return convert_int4(*c0);
	}

}





int4 read_imagei__11ocl_image3d11ocl_samplerDv4_i(
	image3d_t image,
	sampler_t sampler,
	int4 coord
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	//if ((*img0).channelDataType!=CL_UNSIGNED_INT32) //check to make sure return type is the same as channelDataType
	//	return (int4)(1,1,1,0);

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//
	// TODO: use sampler to check addressing mode

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
		coord.z = viv_clamp(coord.z, -1, (*img0).depth);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE//
	else{
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
		coord.z = viv_clamp(coord.z, 0, (*img0).depth -1);
	}

	//-----End Addressing-Mode Processing-----//



	//-----Fetch the 4 image elements----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  )  || ( coord.z < 0  ) ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return uint4)(0,0,0,1);
		}*/

		return (int4)(0,0,0,0);
	}

	if ((*img0).channelDataType==CL_SIGNED_INT32){
		int4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(int)*coord.s1+((*img0).slicePitch)/sizeof(int)*coord.s2); // adjust address offset based on coordinate
		return (int4)(*c0);
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT16){
		short4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(short)*coord.s1+((*img0).slicePitch)/sizeof(short)*coord.s2); // adjust address offset based on coordinate
		return convert_int4(*c0);
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT8){
		char4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(char)*coord.s1+((*img0).slicePitch)/sizeof(char)*coord.s2); // adjust address offset based on coordinate
		return convert_int4(*c0);
	}
}



int4 read_imagei__11ocl_image3dDv4_i(
	image3d_t image,
	int4 coord
	)
{
	sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	//if ((*img0).channelDataType!=CL_UNSIGNED_INT32) //check to make sure return type is the same as channelDataType
	//	return (int4)(1,1,1,0);

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//
	// TODO: use sampler to check addressing mode

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
		coord.z = viv_clamp(coord.z, -1, (*img0).depth);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE//
	else{
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
		coord.z = viv_clamp(coord.z, 0, (*img0).depth -1);
	}

	//-----End Addressing-Mode Processing-----//



	//-----Fetch the 4 image elements----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  )  || ( coord.z < 0  ) ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return uint4)(0,0,0,1);
		}*/

		return (int4)(0,0,0,0);
	}

	if ((*img0).channelDataType==CL_SIGNED_INT32){
		int4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(int)*coord.s1+((*img0).slicePitch)/sizeof(int)*coord.s2); // adjust address offset based on coordinate
		return (int4)(*c0);
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT16){
		short4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(short)*coord.s1+((*img0).slicePitch)/sizeof(short)*coord.s2); // adjust address offset based on coordinate
		return convert_int4(*c0);
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT8){
		char4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(char)*coord.s1+((*img0).slicePitch)/sizeof(char)*coord.s2); // adjust address offset based on coordinate
		return convert_int4(*c0);
	}
}



int4 read_imagei__11ocl_image2dDv2_i(
	image2d_t image,
	int2 coord
	)
{
	sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	uint addressingMode = (uint)(sampler) & 0xF;


	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//
	// TODO: use sampler to check addressing mode

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE


	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);

	}
	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE//
	else{
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);

	}
	//-----End Addressing-Mode Processing-----//


	//-----Fetch the 4 image elements----//


	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (int4)(0,0,0,1);
		}*/

		return (int4)(0,0,0,0);
	}


	if ((*img0).channelDataType==CL_SIGNED_INT32){
		int4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(int)*coord.s1); // adjust address offset based on coordinate
		return (int4)( *c0);
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT16){
		short4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(short)*coord.s1); // adjust address offset based on coordinate
		return convert_int4( *c0);
	}
	else if ((*img0).channelDataType==CL_SIGNED_INT8){
		char4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(char)*coord.s1); // adjust address offset based on coordinate
		return convert_int4(*c0);
	}

}



uint4 read_imageui__11ocl_image2d11ocl_samplerDv2_f(
	image2d_t image,
	sampler_t sampler,
	float2 fcoord
	)
{
	//From libOpenCL/Header Files/gc_cl_sampler.h
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};
	int2 coord = (int2)(0,0);
	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image
	float2 coordTemp;

	uint addressingMode =			(uint)(sampler) & 0xF;
	uint normalizedCoordinates =	(uint)(sampler) & 0x000F0000; //set bit ==> true

	//if ((*img0).channelDataType!=CL_UNSIGNED_INT32) //check to make sure return type is the same as channelDataType
	//	return (uint4)(1,1,1,0);



	//-----Check addressing mode and filter mode-----//
	// OpenCL 1.2 Specs: Chapter 8.2


	//Possibilities:
		//CL_ADDRESS_NONE
		//CL_ADDRESS_CLAMP_TO_EDGE
		//CL_ADDRESS_CLAMP
		//CL_ADDRESS_REPEAT
		//CL_ADDRESS_MIRRORED_REPEAT

		//Addressing Mode: CL_ADDRESS_REPEAT
		if( addressingMode == (CL_ADDRESS_REPEAT & 0xF) ){
			//Filter Mode: CL_FILTER_NEAREST
			coord = (int2)((int) viv_floor(   ( fcoord.x - viv_floor(fcoord.x) ) * (*img0).width   ),(int) viv_floor(   ( fcoord.y - viv_floor(fcoord.y) ) * (*img0).height   ) );
			if(   coord.x  >  ((*img0).width-1)   ){
				coord.x = coord.x - (*img0).width;
			}
			if(  coord.y > ((*img0).height -1) ){
				coord.y = coord.y - (*img0).height;
			}
		}


		//Addressing Mode: CL_ADDRESS_MIRRORED_REPEAT
		else if( addressingMode == (CL_ADDRESS_MIRRORED_REPEAT & 0xF) ){
			//Filter ModeL CL_FILTER_NEAREST
			coordTemp = (float2)(    (  2.0f * rint(0.5f * fcoord.x)  )  ,  (  2.0f * rint(0.5f * fcoord.y)  )    );
			coordTemp = (float2)(    (  viv_fabs(fcoord.x - coordTemp.x)  )  ,  (  viv_fabs(fcoord.y - coordTemp.y)  )    );
			coordTemp = (float2)(    (  coordTemp.x * (*img0).width  )  ,  (  coordTemp.y * (*img0).height  )    );
			coord = (int2)(    (  (int) viv_floor(coordTemp.x)  )  ,  (  (int) viv_floor(coordTemp.y)  )    );
			coord = (int2)(    (  viv_min(coord.x,( (*img0).width-1 ))  )  ,  (  viv_min(coord.y,( (*img0).height-1 ))  )    );
		}



		//Addressing Mode: CL_ADDRESS_CLAMP
		else if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord = (float2)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord = (int2)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 2),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 2));
		}


		//Addressing Mode: CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE
		else{
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord = (float2)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord = (int2)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 0),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 0));
		}

	//-----End Addressing and Filter Mode processing-----//


	if ((*img0).channelDataType==CL_UNSIGNED_INT32){

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){
			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (uint4)(0,0,0,1);
			}*/
			return (uint4)(0,0,0,0);
		}

		//-----Fetch the 4 image elements----//
		uint4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(uint)*coord.s1); // adjust address offset based on coordinate
		return ( *c0 );
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT16){

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){
			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (uint4)(0,0,0,1);
			}*/
			return (uint4)(0,0,0,0);
		}

		//-----Fetch the 4 image elements----//
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1); // adjust address offset based on coordinate
		return convert_uint4( *c0 );
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT8){

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){
			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (uint4)(0,0,0,1);
			}*/
			return (uint4)(0,0,0,0);
		}

		//-----Fetch the 4 image elements----//
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1); // adjust address offset based on coordinate
		return convert_uint4( *c0 );
	}
}






uint4 read_imageui__11ocl_image2d11ocl_samplerDv4_f(
	image2d_t image,
	sampler_t sampler,
	float4 fcoord
	)
{
	//From libOpenCL/Header Files/gc_cl_sampler.h
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};
	int2 coord = (int2)(0,0);
	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image
	float2 coordTemp;

	uint addressingMode =			(uint)(sampler) & 0xF;
	uint normalizedCoordinates =	(uint)(sampler) & 0x000F0000; //set bit ==> true

	int layer = viv_clamp((int)rint(fcoord.z),0,(*img0).depth-1);


	//-----Check addressing mode and filter mode-----//
	// OpenCL 1.2 Specs: Chapter 8.2


	//Possibilities:
		//CL_ADDRESS_NONE
		//CL_ADDRESS_CLAMP_TO_EDGE
		//CL_ADDRESS_CLAMP
		//CL_ADDRESS_REPEAT
		//CL_ADDRESS_MIRRORED_REPEAT

		//Addressing Mode: CL_ADDRESS_REPEAT
		if( addressingMode == (CL_ADDRESS_REPEAT & 0xF) ){
			//Filter Mode: CL_FILTER_NEAREST
			coord = (int2)((int) viv_floor(   ( fcoord.x - viv_floor(fcoord.x) ) * (*img0).width   ),(int) viv_floor(   ( fcoord.y - viv_floor(fcoord.y) ) * (*img0).height   ) );
			if(   coord.x  >  ((*img0).width-1)   ){
				coord.x = coord.x - (*img0).width;
			}
			if(  coord.y > ((*img0).height -1) ){
				coord.y = coord.y - (*img0).height;
			}
		}


		//Addressing Mode: CL_ADDRESS_MIRRORED_REPEAT
		else if( addressingMode == (CL_ADDRESS_MIRRORED_REPEAT & 0xF) ){
			//Filter ModeL CL_FILTER_NEAREST
			coordTemp = (float2)(    (  2.0f * rint(0.5f * fcoord.x)  )  ,  (  2.0f * rint(0.5f * fcoord.y)  )    );
			coordTemp = (float2)(    (  viv_fabs(fcoord.x - coordTemp.x)  )  ,  (  viv_fabs(fcoord.y - coordTemp.y)  )    );
			coordTemp = (float2)(    (  coordTemp.x * (*img0).width  )  ,  (  coordTemp.y * (*img0).height  )    );
			coord = (int2)(    (  (int) viv_floor(coordTemp.x)  )  ,  (  (int) viv_floor(coordTemp.y)  )    );
			coord = (int2)(    (  viv_min(coord.x,( (*img0).width-1 ))  )  ,  (  viv_min(coord.y,( (*img0).height-1 ))  )    );
		}



		//Addressing Mode: CL_ADDRESS_CLAMP
		else if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord.xy = (float2)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord = (int2)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 2),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 2));
		}


		//Addressing Mode: CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE
		else{
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord.xy = (float2)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord = (int2)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 0),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 0));
		}

	//-----End Addressing and Filter Mode processing-----//


	if ((*img0).channelDataType==CL_UNSIGNED_INT32){

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){
			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (uint4)(0,0,0,1);
			}*/
			return (uint4)(0,0,0,0);
		}

		//-----Fetch the 4 image elements----//
		uint4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(uint)*coord.s1+((*img0).slicePitch)/sizeof(uint)*layer); // adjust address offset based on coordinate
		return ( *c0 );
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT16){

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){
			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (uint4)(0,0,0,1);
			}*/
			return (uint4)(0,0,0,0);
		}

		//-----Fetch the 4 image elements----//
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*layer); // adjust address offset based on coordinate
		return convert_uint4( *c0 );
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT8){

		//Check if texel location is outside of the image, if so, then return the border color
		if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
			||  ( coord.x < 0 )  || ( coord.y < 0  )  ){
			//*****NOTE: not required until a format below is supported*****//
			// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
			/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
				||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
					return (uint4)(0,0,0,1);
			}*/
			return (uint4)(0,0,0,0);
		}

		//-----Fetch the 4 image elements----//
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*layer); // adjust address offset based on coordinate
		return convert_uint4( *c0 );
	}
}





uint4 read_imageui__11ocl_image3d11ocl_samplerDv4_f(
	image3d_t image,
	sampler_t sampler,
	float4 fcoord
	)
{
	//From libOpenCL/Header Files/gc_cl_sampler.h
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};
	int4 coord = (int4)(0,0,0,0);
	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image
	float4 coordTemp;

	uint addressingMode =			(uint)(sampler) & 0xF;
	uint normalizedCoordinates =	(uint)(sampler) & 0x000F0000; //set bit ==> true

	//if ((*img0).channelDataType!=CL_UNSIGNED_INT32) //check to make sure return type is the same as channelDataType
	//	return (uint4)(1,1,1,0);



	//-----Check addressing mode and filter mode-----//
	// OpenCL 1.2 Specs: Chapter 8.2


	//Possibilities:
		//CL_ADDRESS_NONE
		//CL_ADDRESS_CLAMP_TO_EDGE
		//CL_ADDRESS_CLAMP
		//CL_ADDRESS_REPEAT
		//CL_ADDRESS_MIRRORED_REPEAT

		//Addressing Mode: CL_ADDRESS_REPEAT
		if( addressingMode == (CL_ADDRESS_REPEAT & 0xF) ){
			//Filter Mode: CL_FILTER_NEAREST
			coord.xy = (int2)((int) viv_floor(   ( fcoord.x - viv_floor(fcoord.x) ) * (*img0).width   ),(int) viv_floor(   ( fcoord.y - viv_floor(fcoord.y) ) * (*img0).height   ) );
			coord.z = (int) viv_floor(   ( fcoord.z - viv_floor(fcoord.z) ) * (*img0).depth  );
			if(   coord.x  >  ((*img0).width-1)   ){
				coord.x = coord.x - (*img0).width;
			}
			if(  coord.y > ((*img0).height -1) ){
				coord.y = coord.y - (*img0).height;
			}
			if(  coord.z > ((*img0).depth -1) ){
				coord.z = coord.z - (*img0).depth;
			}
		}


		//Addressing Mode: CL_ADDRESS_MIRRORED_REPEAT
		else if( addressingMode == (CL_ADDRESS_MIRRORED_REPEAT & 0xF) ){
			//Filter ModeL CL_FILTER_NEAREST
			coordTemp.xyz = (float3)(    (  2.0f * rint(0.5f * fcoord.x)  )  ,  (  2.0f * rint(0.5f * fcoord.y)  )  ,  (  2.0f * rint(0.5f * fcoord.z)  )    );
			coordTemp.xyz = (float3)(    (  viv_fabs(fcoord.x - coordTemp.x)  )  ,  (  viv_fabs(fcoord.y - coordTemp.y)  )  ,  (  viv_fabs(fcoord.z - coordTemp.z)  )    );
			coordTemp.xyz = (float3)(    (  coordTemp.x * (*img0).width  )  ,  (  coordTemp.y * (*img0).height  )  ,  (  coordTemp.z * (*img0).depth  )    );
			coord.xyz = (int3)(    (  (int) viv_floor(coordTemp.x)  )  ,  (  (int) viv_floor(coordTemp.y)  )  ,  (  (int) viv_floor(coordTemp.z)  )    );
			coord.xyz = (int3)(    (  viv_min(coord.x,( (*img0).width-1 ))  )  ,  (  viv_min(coord.y,( (*img0).height-1 ))  )  ,  (  viv_min(coord.z,( (*img0).depth-1 ))  )    );
		}



		//Addressing Mode: CL_ADDRESS_CLAMP
		else if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord.xyz = (float3)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)  ,  (fcoord.z * (*img0).depth)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord.xyz = (int3)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 2),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 2),addressing_mode((int)viv_floor(fcoord.z),(*img0).depth, 2));
		}


		//Addressing Mode: CL_ADDRESS_CLAMP_TO_EDGE or CL_ADDRESS_NONE
		else{
			//-----Check if normalized-----//
			if( normalizedCoordinates){
				fcoord.xyz = (float3)(    (fcoord.x * (*img0).width)  ,  (fcoord.y * (*img0).height)  ,  (fcoord.z * (*img0).depth)    );
			}
			//Filter Mode: CL_FILTER_NEAREST
			coord.xyz = (int3)(addressing_mode((int)viv_floor(fcoord.x),(*img0).width, 0),addressing_mode((int)viv_floor(fcoord.y),(*img0).height, 0),addressing_mode((int)viv_floor(fcoord.z),(*img0).depth, 0));
		}

	//-----End Addressing and Filter Mode processing-----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  ) || ( coord.z < 0  )  ){
		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (uint4)(0,0,0,1);
		}*/
		return (uint4)(0,0,0,0);
	}

	if ((*img0).channelDataType==CL_UNSIGNED_INT32){

		//-----Fetch the 4 image elements----//
		uint4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(uint)*coord.s1+((*img0).slicePitch)/sizeof(uint)*coord.s2 ); // adjust address offset based on coordinate
		return ( *c0 );
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT16){

		//-----Fetch the 4 image elements----//
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2 ); // adjust address offset based on coordinate
		return convert_uint4( *c0 );
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT8){

		//-----Fetch the 4 image elements----//
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2 ); // adjust address offset based on coordinate
		return convert_uint4( *c0 );
	}
}





uint4 read_imageui__11ocl_image2d11ocl_samplerDv2_i(
	image2d_t image,
	sampler_t sampler,
	int2 coord
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	//if ((*img0).channelDataType!=CL_UNSIGNED_INT32) //check to make sure return type is the same as channelDataType
	//	return (int4)(1,1,1,0);

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//
	// TODO: use sampler to check addressing mode

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE//
	else{
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
	}

	//-----End Addressing-Mode Processing-----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (uint4)(0,0,0,1);
		}*/

		return (uint4)(0,0,0,0);
	}

	//-----Fetch the 4 image elements----//

	if ((*img0).channelDataType==CL_UNSIGNED_INT32){

		uint4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(uint)*coord.s1); // adjust address offset based on coordinate
		return (uint4)( *c0);
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT16){

		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1); // adjust address offset based on coordinate
		return convert_uint4( *c0);
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT8){

		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1); // adjust address offset based on coordinate
		return convert_uint4(*c0);
	}
}



uint4 read_imageui__11ocl_image3d11ocl_samplerDv4_i(
	image3d_t image,
	sampler_t sampler,
	int4 coord
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	//if ((*img0).channelDataType!=CL_UNSIGNED_INT32) //check to make sure return type is the same as channelDataType
	//	return (int4)(1,1,1,0);

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//
	// TODO: use sampler to check addressing mode

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
		coord.z = viv_clamp(coord.z, -1, (*img0).depth);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE//
	else{
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
		coord.z = viv_clamp(coord.z, 0, (*img0).depth -1);
	}

	//-----End Addressing-Mode Processing-----//



	//-----Fetch the 4 image elements----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  )  || ( coord.z < 0  ) ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (uint4)(0,0,0,1);
		}*/

		return (uint4)(0,0,0,0);
	}

	if ((*img0).channelDataType==CL_UNSIGNED_INT32){
		uint4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(uint)*coord.s1+((*img0).slicePitch)/sizeof(uint)*coord.s2); // adjust address offset based on coordinate
		return (uint4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT16){
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2); // adjust address offset based on coordinate
		return convert_uint4(*c0);
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT8){
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2); // adjust address offset based on coordinate
		return convert_uint4(*c0);
	}
}



uint4 read_imageui__11ocl_image3dDv4_i(
	image3d_t image,
	int4 coord
	)
{
	sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	//if ((*img0).channelDataType!=CL_UNSIGNED_INT32) //check to make sure return type is the same as channelDataType
	//	return (int4)(1,1,1,0);

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//
	// TODO: use sampler to check addressing mode

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
		coord.z = viv_clamp(coord.z, -1, (*img0).depth);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE//
	else{
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
		coord.z = viv_clamp(coord.z, 0, (*img0).depth -1);
	}

	//-----End Addressing-Mode Processing-----//



	//-----Fetch the 4 image elements----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) ) || ( coord.z >((*img0).depth -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  )  || ( coord.z < 0  ) ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (uint4)(0,0,0,1);
		}*/

		return (uint4)(0,0,0,0);
	}

	if ((*img0).channelDataType==CL_UNSIGNED_INT32){
		uint4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(uint)*coord.s1+((*img0).slicePitch)/sizeof(uint)*coord.s2); // adjust address offset based on coordinate
		return (uint4)(*c0);
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT16){
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2); // adjust address offset based on coordinate
		return convert_uint4(*c0);
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT8){
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2); // adjust address offset based on coordinate
		return convert_uint4(*c0);
	}
}




uint4 read_imageui__11ocl_image2dDv2_i(
	image2d_t image,
	int2 coord
	)
{
	sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	//if ((*img0).channelDataType!=CL_UNSIGNED_INT32) //check to make sure return type is the same as channelDataType
	//	return (int4)(1,1,1,0);

	uint addressingMode = (uint)(sampler) & 0xF;

	//NOTE: Support only CL_FILTER_NEAREST and NORMALIZED_COORDS_FALSE
	//We will operate under those assumptions

	//-----Check addressing mode-----//
	// TODO: use sampler to check addressing mode

	//Possibilities:
		//CLK_ADDRESS_CLAMP_TO_EDGE
		//CLK_ADDRESS_CLAMP
		//CLK_ADDRESS_NONE

	//Addressing Mode: CLK_ADDRESS_CLAMP//
	if( addressingMode == (CL_ADDRESS_CLAMP & 0xF) ){
		coord.x = viv_clamp(coord.x, -1, (*img0).width);
		coord.y = viv_clamp(coord.y, -1, (*img0).height);
	}

	//Addressing Mode: CLK_ADDRESS_CLAMP_TO_EDGE or CLK_ADDRESS_NONE//
	else{
		coord.x = viv_clamp(coord.x, 0, (*img0).width -1);
		coord.y = viv_clamp(coord.y, 0, (*img0).height -1);
	}

	//-----End Addressing-Mode Processing-----//

	//Check if texel location is outside of the image, if so, then return the border color
	if( ( coord.x >((*img0).width -1) ) || ( coord.y >((*img0).height -1) )
		||  ( coord.x < 0 )  || ( coord.y < 0  )  ){

		//*****NOTE: not required until a format below is supported*****//
		// check if type is CL_R, CL_RG, CL_RGB, or CL_LUMINANCE
		/*if (    ( (*img0).channelOrder == CL_R  ) ||  ( (*img0).channelOrder == CL_RG  )
			||  ( (*img0).channelOrder == CL_RGB  )  ||  ( (*img0).channelOrder == CL_LUMINANCE  )    ){
				return (uint4)(0,0,0,1);
		}*/

		return (uint4)(0,0,0,0);
	}

	//-----Fetch the 4 image elements----//

	if ((*img0).channelDataType==CL_UNSIGNED_INT32){

		uint4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(uint)*coord.s1); // adjust address offset based on coordinate
		return (uint4)( *c0);
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT16){

		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1); // adjust address offset based on coordinate
		return convert_uint4( *c0);
	}
	else if ((*img0).channelDataType==CL_UNSIGNED_INT8){

		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1); // adjust address offset based on coordinate
		return convert_uint4(*c0);
	}
}





char select__ccc(
	char src0,
	char src1,
	char src2
	)
{
	return src2 ? src1:src0;
	}


char16 select__Dv16_cDv16_cDv16_c(
	char16 src0,
	char16 src1,
	char16 src2
	)
{
	src2 = src2 & (char16)(-128);
	return src2 ? src1:src0;
	}


char2 select__Dv2_cDv2_cDv2_c(
	char2 src0,
	char2 src1,
	char2 src2
	)
{
	src2 = src2 & (char2)(-128);
	return src2 ? src1:src0;
	}


char3 select__Dv3_cDv3_cDv3_c(
	char3 src0,
	char3 src1,
	char3 src2
	)
{
	src2 = src2 & (char3)(-128);
	return src2 ? src1:src0;
	}


char4 select__Dv4_cDv4_cDv4_c(
	char4 src0,
	char4 src1,
	char4 src2
	)
{
	src2 = src2 & (char4)(-128);
	return src2 ? src1:src0;
	}


char8 select__Dv8_cDv8_cDv8_c(
	char8 src0,
	char8 src1,
	char8 src2
	)
{
	src2 = src2 & (char8)(-128);
	return src2 ? src1:src0;
	}


char select__cch(
	char src0,
	char src1,
	uchar src2
	)
{
	return src2 ? src1:src0;
	}


char16 select__Dv16_cDv16_cDv16_h(
	char16 src0,
	char16 src1,
	uchar16 src2
	)
{
	src2 = src2 & (uchar16)(128);
	return src2 ? src1:src0;
	}


char2 select__Dv2_cDv2_cDv2_h(
	char2 src0,
	char2 src1,
	uchar2 src2
	)
{
	src2 = src2 & (uchar2)(128);
	return src2 ? src1:src0;
	}


char3 select__Dv3_cDv3_cDv3_h(
	char3 src0,
	char3 src1,
	uchar3 src2
	)
{
	src2 = src2 & (uchar3)(128);
	return src2 ? src1:src0;
	}


char4 select__Dv4_cDv4_cDv4_h(
	char4 src0,
	char4 src1,
	uchar4 src2
	)
{
	src2 = src2 & (uchar4)(128);
	return src2 ? src1:src0;
	}


char8 select__Dv8_cDv8_cDv8_h(
	char8 src0,
	char8 src1,
	uchar8 src2
	)
{
	src2 = src2 & (uchar8)(128);
	return src2 ? src1:src0;
	}


int select__iii(
	int src0,
	int src1,
	int src2
	)
{
	return src2 ? src1:src0;
	}


int16 select__Dv16_iDv16_iDv16_i(
	int16 src0,
	int16 src1,
	int16 src2
	)
{
	src2 = src2 & (int16)(-2147483648);
	return src2 ? src1:src0;
	}


int2 select__Dv2_iDv2_iDv2_i(
	int2 src0,
	int2 src1,
	int2 src2
	)
{
	src2 = src2 & (int2)(-2147483648);
	return src2 ? src1:src0;
	}


int3 select__Dv3_iDv3_iDv3_i(
	int3 src0,
	int3 src1,
	int3 src2
	)
{
	src2 = src2 & (int3)(-2147483648);
	return src2 ? src1:src0;
	}


int4 select__Dv4_iDv4_iDv4_i(
	int4 src0,
	int4 src1,
	int4 src2
	)
{
	src2 = src2 & (int4)(-2147483648);
	return src2 ? src1:src0;
	}


int8 select__Dv8_iDv8_iDv8_i(
	int8 src0,
	int8 src1,
	int8 src2
	)
{
	src2 = src2 & (int8)(-2147483648);
	return src2 ? src1:src0;
	}


int select__iij(
	int src0,
	int src1,
	uint src2
	)
{
	return src2 ? src1:src0;
	}


int16 select__Dv16_iDv16_iDv16_j(
	int16 src0,
	int16 src1,
	uint16 src2
	)
{
	src2 = src2 & (uint16)2147483648;
	return src2 ? src1:src0;
	}


int2 select__Dv2_iDv2_iDv2_j(
	int2 src0,
	int2 src1,
	uint2 src2
	)
{
	src2 = src2 & (uint2)2147483648;
	return src2 ? src1:src0;
	}


int3 select__Dv3_iDv3_iDv3_j(
	int3 src0,
	int3 src1,
	uint3 src2
	)
{
	src2 = src2 & (uint3)2147483648;
	return src2 ? src1:src0;
	}


int4 select__Dv4_iDv4_iDv4_j(
	int4 src0,
	int4 src1,
	uint4 src2
	)
{
	src2 = src2 & (uint4)2147483648;
	return src2 ? src1:src0;
	}


int8 select__Dv8_iDv8_iDv8_j(
	int8 src0,
	int8 src1,
	uint8 src2
	)
{
	src2 = src2 & (uint8)2147483648;
	return src2 ? src1:src0;
	}


short select__sss(
	short src0,
	short src1,
	short src2
	)
{
	return src2 ? src1:src0;
	}


short16 select__Dv16_sDv16_sDv16_s(
	short16 src0,
	short16 src1,
	short16 src2
	)
{
	src2 = src2 & (short)(-32768);
	return src2 ? src1:src0;
	}


short2 select__Dv2_sDv2_sDv2_s(
	short2 src0,
	short2 src1,
	short2 src2
	)
{
	src2 = src2 & (short)(-32768);
	return src2 ? src1:src0;
	}


short3 select__Dv3_sDv3_sDv3_s(
	short3 src0,
	short3 src1,
	short3 src2
	)
{
	src2 = src2 & (short)(-32768);
	return src2 ? src1:src0;
	}


short4 select__Dv4_sDv4_sDv4_s(
	short4 src0,
	short4 src1,
	short4 src2
	)
{
	src2 = src2 & (short)(-32768);
	return src2 ? src1:src0;
	}


short8 select__Dv8_sDv8_sDv8_s(
	short8 src0,
	short8 src1,
	short8 src2
	)
{
	src2 = src2 & (short)(-32768);
	return src2 ? src1:src0;
	}


short select__sst(
	short src0,
	short src1,
	ushort src2
	)
{
	return src2 ? src1:src0;
	}


short16 select__Dv16_sDv16_sDv16_t(
	short16 src0,
	short16 src1,
	ushort16 src2
	)
{
    src2 = src2 & (ushort)32768;
	return src2 ? src1:src0;
	}


short2 select__Dv2_sDv2_sDv2_t(
	short2 src0,
	short2 src1,
	ushort2 src2
	)
{
    src2 = src2 & (ushort)32768;
	return src2 ? src1:src0;
	}


short3 select__Dv3_sDv3_sDv3_t(
	short3 src0,
	short3 src1,
	ushort3 src2
	)
{
    src2 = src2 & (ushort) 32768;
	return src2 ? src1:src0;
	}


short4 select__Dv4_sDv4_sDv4_t(
	short4 src0,
	short4 src1,
	ushort4 src2
	)
{
    src2 = src2 & (ushort)32768;
	return src2 ? src1:src0;
	}


short8 select__Dv8_sDv8_sDv8_t(
	short8 src0,
	short8 src1,
	ushort8 src2
	)
{
    src2 = src2 & (ushort)32768;
	return src2 ? src1:src0;
	}


uchar select__hhc(
	uchar src0,
	uchar src1,
	char src2
	)
{
	return src2 ? src1:src0;
	}


uchar16 select__Dv16_hDv16_hDv16_c(
	uchar16 src0,
	uchar16 src1,
	char16 src2
	)
{
	src2 = src2 & (char16)(-128);
	return src2 ? src1:src0;
	}


uchar2 select__Dv2_hDv2_hDv2_c(
	uchar2 src0,
	uchar2 src1,
	char2 src2
	)
{
	src2 = src2 & (char2)(-128);
	return src2 ? src1:src0;
	}


uchar3 select__Dv3_hDv3_hDv3_c(
	uchar3 src0,
	uchar3 src1,
	char3 src2
	)
{
	src2 = src2 & (char3)(-128);
	return src2 ? src1:src0;
	}


uchar4 select__Dv4_hDv4_hDv4_c(
	uchar4 src0,
	uchar4 src1,
	char4 src2
	)
{
	src2 = src2 & (char4)(-128);
	return src2 ? src1:src0;
	}


uchar8 select__Dv8_hDv8_hDv8_c(
	uchar8 src0,
	uchar8 src1,
	char8 src2
	)
{
	src2 = src2 & (char8)(-128);
	return src2 ? src1:src0;
	}


uchar select__hhh(
	uchar src0,
	uchar src1,
	uchar src2
	)
{
	return src2 ? src1:src0;
	}


uchar16 select__Dv16_hDv16_hDv16_h(
	uchar16 src0,
	uchar16 src1,
	uchar16 src2
	)
{
	src2 = src2 & (uchar16)128;
	return src2 ? src1:src0;
	}


uchar2 select__Dv2_hDv2_hDv2_h(
	uchar2 src0,
	uchar2 src1,
	uchar2 src2
	)
{
	src2 = src2 & (uchar2)128;
	return src2 ? src1:src0;
	}


uchar3 select__Dv3_hDv3_hDv3_h(
	uchar3 src0,
	uchar3 src1,
	uchar3 src2
	)
{
	src2 = src2 & (uchar3)128;
	return src2 ? src1:src0;
	}


uchar4 select__Dv4_hDv4_hDv4_h(
	uchar4 src0,
	uchar4 src1,
	uchar4 src2
	)
{
	src2 = src2 & (uchar4)128;
	return src2 ? src1:src0;
	}


uchar8 select__Dv8_hDv8_hDv8_h(
	uchar8 src0,
	uchar8 src1,
	uchar8 src2
	)
{
	src2 = src2 & (uchar8)128;
	return src2 ? src1:src0;
	}


uint select__jji(
	uint src0,
	uint src1,
	int src2
	)
{
	return src2 ? src1:src0;
	}


uint16 select__Dv16_jDv16_jDv16_i(
	uint16 src0,
	uint16 src1,
	int16 src2
	)
{
	src2 = src2 & (int16)(-2147483648);
	return src2 ? src1:src0;
	}


uint2 select__Dv2_jDv2_jDv2_i(
	uint2 src0,
	uint2 src1,
	int2 src2
	)
{
	src2 = src2 & (int2)(-2147483648);
	return src2 ? src1:src0;
	}


uint3 select__Dv3_jDv3_jDv3_i(
	uint3 src0,
	uint3 src1,
	int3 src2
	)
{
	src2 = src2 & (int3)(-2147483648);
	return src2 ? src1:src0;
	}


uint4 select__Dv4_jDv4_jDv4_i(
	uint4 src0,
	uint4 src1,
	int4 src2
	)
{
	src2 = src2 & (int4)(-2147483648);
	return src2 ? src1:src0;
	}


uint8 select__Dv8_jDv8_jDv8_i(
	uint8 src0,
	uint8 src1,
	int8 src2
	)
{
	src2 = src2 & (int8)(-2147483648);
	return src2 ? src1:src0;
	}


uint select__jjj(
	uint src0,
	uint src1,
	uint src2
	)
{
	return src2 ? src1:src0;
	}


uint16 select__Dv16_jDv16_jDv16_j(
	uint16 src0,
	uint16 src1,
	uint16 src2
	)
{
	src2 = src2 & (uint16)2147483648;
	return src2 ? src1:src0;
	}


uint2 select__Dv2_jDv2_jDv2_j(
	uint2 src0,
	uint2 src1,
	uint2 src2
	)
{
	src2 = src2 & (uint2)2147483648;
	return src2 ? src1:src0;
	}


uint3 select__Dv3_jDv3_jDv3_j(
	uint3 src0,
	uint3 src1,
	uint3 src2
	)
{
	src2 = src2 & (uint3)2147483648;
	return src2 ? src1:src0;
	}


uint4 select__Dv4_jDv4_jDv4_j(
	uint4 src0,
	uint4 src1,
	uint4 src2
	)
{
	src2 = src2 & (uint4)2147483648;
	return src2 ? src1:src0;
	}


uint8 select__Dv8_jDv8_jDv8_j(
	uint8 src0,
	uint8 src1,
	uint8 src2
	)
{
	src2 = src2 & (uint8)2147483648;
	return src2 ? src1:src0;
	}


ushort select__tts(
	ushort src0,
	ushort src1,
	short src2
	)
{
	src2 = src2 & (short)(-32768);
	return src2 ? src1:src0;
	}


ushort16 select__Dv16_tDv16_tDv16_s(
	ushort16 src0,
	ushort16 src1,
	short16 src2
	)
{
	src2 = src2 & (short16)(-32768); //making the mask a scalar or vector results in the same number of instructions in the final function
	return src2 ? src1:src0;
	}


ushort2 select__Dv2_tDv2_tDv2_s(
	ushort2 src0,
	ushort2 src1,
	short2 src2
	)
{
	src2 = src2 & (short2)(-32768); //making the mask a scalar or vector results in the same number of instructions in the final function
	return src2 ? src1:src0;
	}


ushort3 select__Dv3_tDv3_tDv3_s(
	ushort3 src0,
	ushort3 src1,
	short3 src2
	)
{
	src2 = src2 & (short3)(-32768); //making the mask a scalar or vector results in the same number of instructions in the final function
	return src2 ? src1:src0;
	}


ushort4 select__Dv4_tDv4_tDv4_s(
	ushort4 src0,
	ushort4 src1,
	short4 src2
	)
{
	src2 = src2 & (short4)(-32768); //making the mask a scalar or vector results in the same number of instructions in the final function
	return src2 ? src1:src0;
	}


ushort8 select__Dv8_tDv8_tDv8_s(
	ushort8 src0,
	ushort8 src1,
	short8 src2
	)
{
	src2 = src2 & (short8)(-32768); //making the mask a scalar or vector results in the same number of instructions in the final function
	return src2 ? src1:src0;
	}


ushort select__ttt(
	ushort src0,
	ushort src1,
	ushort src2
	)
{
	return src2 ? src1:src0;
	}


ushort16 select__Dv16_tDv16_tDv16_t(
	ushort16 src0,
	ushort16 src1,
	ushort16 src2
	)
{
	src2 = src2 & (ushort)32768;
	return src2 ? src1:src0;
	}


ushort2 select__Dv2_tDv2_tDv2_t(
	ushort2 src0,
	ushort2 src1,
	ushort2 src2
	)
{
	src2 = src2 & (ushort)32768;
	return src2 ? src1:src0;
	}


ushort3 select__Dv3_tDv3_tDv3_t(
	ushort3 src0,
	ushort3 src1,
	ushort3 src2
	)
{
	src2 = src2 & (ushort)32768;
	return src2 ? src1:src0;
	}


ushort4 select__Dv4_tDv4_tDv4_t(
	ushort4 src0,
	ushort4 src1,
	ushort4 src2
	)
{
	src2 = src2 & (ushort)32768;
	return src2 ? src1:src0;
	}


ushort8 select__Dv8_tDv8_tDv8_t(
	ushort8 src0,
	ushort8 src1,
	ushort8 src2
	)
{
	src2 = src2 & (ushort)32768;
	return src2 ? src1:src0;
	}


int signbit__f(
	float src0
	)
{
	union float_int{
		float f;
		int i;
	} float_int_t;

	float_int_t.f = src0;
	return float_int_t.i>>31;
}


int16 signbit__Dv16_f(
	float16 src0
	)
{
	union float_int{
		float16 f;
		int16 i;
	} float_int_t;

	float_int_t.f = src0;
	return float_int_t.i >>31;

}


int2 signbit__Dv2_f(
	float2 src0
	)
{
	union float_int{
		float2 f;
		int2 i;
	} float_int_t;

	float_int_t.f = src0;
	return float_int_t.i >>31;

	//uint2 src0_uint2 = as_uint2(src0); // reinterpret float2 as int2 for bitwise operations
    //int2 ret = 0;

	/*if(float_int_t.i.s0){
		ret.s0=-1;
	}
    if(float_int_t.i.s1){
		ret.s1=-1;
	}*/

	//ret = float_int_t.i;

    //return ret;
}


int3 signbit__Dv3_f(
	float3 src0
	)
{
	union float_int{
		float3 f;
		int3 i;
	} float_int_t;

	float_int_t.f = src0;
	return float_int_t.i >>31;

	//uint3 src0_uint3 = as_uint3(src0); // reinterpret float3 as int3 for bitwise operations
    //int3 ret = 0;

	/*if(float_int_t.i.s0){
		ret.s0=-1;
	}
    if(float_int_t.i.s1){
		ret.s1=-1;
	}*/

	//ret = float_int_t.i;

    //return ret;
}


int4 signbit__Dv4_f(
	float4 src0
	)
{
	union float_int{
		float4 f;
		int4 i;
	} float_int_t;

	float_int_t.f = src0;
	return float_int_t.i >>31;

}


int8 signbit__Dv8_f(
	float8 src0
	)
{
	union float_int{
		float8 f;
		int8 i;
	} float_int_t;

	float_int_t.f = src0;
	return float_int_t.i >>31;

}


void write_imagef__11ocl_image2dDv2_iDv4_f(
	image2d_t image,
	int2 coord,
	float4 color
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*                 physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	// Coordinates outside of image ==> Do nothing
	if( coord.x < 0 || coord.x > ((*img0).width -1))
		return;
	if( coord.y < 0 || coord.y > ((*img0).height -1))
		return;

	//check to make sure ChannelDataType is correct
	if ((*img0).channelDataType==CL_FLOAT){
		// adjust address offset based on coordinate
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1);
		*c0 = color;
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8){
		// adjust address offset based on coordinate
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1);
		*c0 = convert_uchar4_sat_rte(color*255.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16){
		// adjust address offset based on coordinate
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1);
		*c0 = convert_ushort4_sat_rte(color*65535.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT){

		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1);
		vstore_half(color.x,0,c0);
		vstore_half(color.y,1,c0);
		vstore_half(color.z,2,c0);
		vstore_half(color.w,3,c0);

	}

}




void write_imagef__11ocl_image3dDv4_iDv4_f(
	image3d_t image,
	int4 coord,
	float4 color
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		float*               physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	// Coordinates outside of image ==> Do nothing
	if( coord.x < 0 || coord.x > ((*img0).width -1))
		return;
	if( coord.y < 0 || coord.y > ((*img0).height -1))
		return;
	if( coord.z < 0 || coord.z > ((*img0).depth -1))
		return;

	//check to make sure ChannelDataType is correct
	if ((*img0).channelDataType==CL_FLOAT){
		// adjust address offset based on coordinate
		float4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(float)*coord.s1+((*img0).slicePitch)/sizeof(float)*coord.s2);
		*c0 = color;
	}
	else if ((*img0).channelDataType==CL_UNORM_INT8){
		// adjust address offset based on coordinate
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2);
		*c0 = convert_uchar4_sat_rte(color*255.0f);
	}
	else if ((*img0).channelDataType==CL_UNORM_INT16){
		// adjust address offset based on coordinate
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2);
		*c0 = convert_ushort4_sat_rte(color*65535.0f);
	}
	else if ((*img0).channelDataType==CL_HALF_FLOAT){

		half* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(half)*coord.s1+((*img0).slicePitch)/sizeof(half)*coord.s2);
		vstore_half(color.x,0,c0);
		vstore_half(color.y,1,c0);
		vstore_half(color.z,2,c0);
		vstore_half(color.w,3,c0);
	}
}



void write_imagei__11ocl_image2dDv2_iDv4_i(
	image2d_t image,
	int2 coord,
	int4 color
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	// Coordinates outside of image ==> Do nothing
	if( coord.x < 0 || coord.x > ((*img0).width -1))
		return;
	if( coord.y < 0 || coord.y > ((*img0).height -1))
		return;

	//check to make sure ChannelDataType is valid
	if ((*img0).channelDataType==CL_SIGNED_INT32){
		// adjust address offset based on coordinate
		int4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(int)*coord.s1);
		*c0 = color;
	}
	else if((*img0).channelDataType==CL_SIGNED_INT16){
		// adjust address offset based on coordinate
		short4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(short)*coord.s1);
		*c0 = convert_short4_sat(color);
	}
	else if((*img0).channelDataType==CL_SIGNED_INT8){
		// adjust address offset based on coordinate
		char4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(char)*coord.s1);
		*c0 = convert_char4_sat(color);
	}
    else{
        return;
    }
}



void write_imagei__11ocl_image3dDv4_iDv4_i(
	image3d_t image,
	int4 coord,
	int4 color
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		int*                 physical;			 /* Pointer to Coordinates */
	};

	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image

	// Coordinates outside of image ==> Do nothing
	if( coord.x < 0 || coord.x > ((*img0).width -1))
		return;
	if( coord.y < 0 || coord.y > ((*img0).height -1))
		return;
	if( coord.z < 0 || coord.z > ((*img0).depth -1))
		return;

	//check to make sure ChannelDataType is valid
	if ((*img0).channelDataType==CL_SIGNED_INT32){
		// adjust address offset based on coordinate
		int4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(int)*coord.s1+((*img0).slicePitch)/sizeof(int)*coord.s2);
		*c0 = color;
	}
	else if((*img0).channelDataType==CL_SIGNED_INT16){
		// adjust address offset based on coordinate
		short4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(short)*coord.s1+((*img0).slicePitch)/sizeof(short)*coord.s2);
		*c0 = convert_short4_sat(color);
	}
	else if((*img0).channelDataType==CL_SIGNED_INT8){
		// adjust address offset based on coordinate
		char4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(char)*coord.s1+((*img0).slicePitch)/sizeof(char)*coord.s2);
		*c0 = convert_char4_sat(color);
	}
    else{
        return;
    }
}



void write_imageui__11ocl_image2dDv2_iDv4_j(
	image2d_t image,
	int2 coord,
	uint4 color
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		uint*                physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image


	// Coordinates outside of image ==> Do nothing
	if( coord.x < 0 || coord.x > ((*img0).width -1))
		return;
	if( coord.y < 0 || coord.y > ((*img0).height -1))
		return;

	//check to make sure ChannelDataType is valid
	if ((*img0).channelDataType==CL_UNSIGNED_INT32){
		// adjust address offset based on coordinate
		uint4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(uint)*coord.s1);
		*c0 = color;
	}
	else if((*img0).channelDataType==CL_UNSIGNED_INT16){
		// adjust address offset based on coordinate
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1);
		*c0 = convert_ushort4_sat(color);
	}
	else if((*img0).channelDataType==CL_UNSIGNED_INT8){
		// adjust address offset based on coordinate
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1);
		*c0 = convert_uchar4_sat(color);
	}
    else{
        return;
    }
}



void write_imageui__11ocl_image3dDv4_iDv4_j(
	image3d_t image,
	int4 coord,
	uint4 color
	)
{
	struct _cl_image_header
	{
		int                  width;
		int                  height;
		int                  depth;              /* 0: 2D image. */
		int                  channelDataType;
		int                  channelOrder;
		int                  samplerValue;
		int                  rowPitch;
		int                  slicePitch;
		int                  textureNum;         /* -1: no HW support. */
		uint*                physical;			 /* Pointer to Coordinates */
	};


	struct _cl_image_header *img0 = &image; // use struct in gc_cl_mem.h to interpret input image


	// Coordinates outside of image ==> Do nothing
	if( coord.x < 0 || coord.x > ((*img0).width -1))
		return;
	if( coord.y < 0 || coord.y > ((*img0).height -1))
		return;
	if( coord.z < 0 || coord.z > ((*img0).depth -1))
		return;

	//check to make sure ChannelDataType is valid
	if ((*img0).channelDataType==CL_UNSIGNED_INT32){
		// adjust address offset based on coordinate
		uint4* c0 = (*img0).physical + 4*( coord.s0+((*img0).rowPitch)/sizeof(uint)*coord.s1+((*img0).slicePitch)/sizeof(uint)*coord.s2);
		*c0 = color;
	}
	else if((*img0).channelDataType==CL_UNSIGNED_INT16){
		// adjust address offset based on coordinate
		ushort4* c0 = (*img0).physical + 2*( coord.s0+((*img0).rowPitch)/sizeof(ushort)*coord.s1+((*img0).slicePitch)/sizeof(ushort)*coord.s2);
		*c0 = convert_ushort4_sat(color);
	}
	else if((*img0).channelDataType==CL_UNSIGNED_INT8){
		// adjust address offset based on coordinate
		uchar4* c0 = (*img0).physical + ( coord.s0+((*img0).rowPitch)/sizeof(uchar)*coord.s1+((*img0).slicePitch)/sizeof(uchar)*coord.s2);
		*c0 = convert_uchar4_sat(color);
	}
    else{
        return;
    }
}




