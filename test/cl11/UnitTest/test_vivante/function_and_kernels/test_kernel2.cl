__kernel void kernel2(__global uchar *src, __global uchar *dst, uchar n)
{
    int  tid = get_global_id(0);

    dst[tid] = tid % (1 << (src[tid] % (n)));

}

__kernel void test_kernel2(__global uchar *src, __global uchar *dst)
{
	__private uchar n;
	n= 8;

    kernel2(src, dst, n);

}