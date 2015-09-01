__kernel void kernel3_2_2(__global unsigned char *src, __global unsigned char *dst )
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (8)));

}

 void NULLkernel2_2_2(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_2_2(src, dst );

}

__kernel void kernel2_2(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 0;

    kernel3_2_2(src, dst );

}
__kernel void kernel3_2_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n)));

}

 void NULLkernel2_2_3(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_2_3(src, dst ,0);

}

__kernel void kernel2_3(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 8;

    kernel3_2_3(src, dst ,n);

}
__kernel void kernel3_2_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void NULLkernel2_2_4(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_2_4(src, dst ,0,0);

}

__kernel void kernel2_4(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    kernel3_2_4(src, dst ,n, (unsigned char)2);

}
 void func3_2_2(__global unsigned char *src, __global unsigned char *dst )
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (8)));

}

 void NULLfunc2_2_2(__global unsigned char *src, __global unsigned char *dst )
{

	func3_2_2(src, dst );

}

__kernel void func2_2(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 0;

    func3_2_2(src, dst );

}
 void func3_2_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n)));

}

 void NULLfunc2_2_3(__global unsigned char *src, __global unsigned char *dst )
{

	func3_2_3(src, dst ,0);

}

__kernel void func2_3(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 8;

    func3_2_3(src, dst ,n);

}
 void func3_2_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void NULLfunc2_2_4(__global unsigned char *src, __global unsigned char *dst )
{

	func3_2_4(src, dst ,0,0);

}

__kernel void func2_4(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    func3_2_4(src, dst ,n, (unsigned char)2);

}
 void func3_2_2_2(__global unsigned char *src, __global unsigned char *dst )
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (8)));

}

 void func2_2_2_2(__global unsigned char *src, __global unsigned char *dst )
{

	func3_2_2_2(src, dst );

}

__kernel void func2_2_2(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 5;

    func2_2_2_2(src, dst );

}
 void func3_2_2_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n)));

}

 void func2_2_2_3(__global unsigned char *src, __global unsigned char *dst )
{

	func3_2_2_3(src, dst ,(unsigned char)8);

}

__kernel void func2_2_3(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    func2_2_2_3(src, dst );

}
 void func3_2_2_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void func2_2_2_4(__global unsigned char *src, __global unsigned char *dst )
{

	func3_2_2_4(src, dst ,(unsigned char)4, (unsigned char)2);

}

__kernel void func2_2_4(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    func2_2_2_4(src, dst );

}
__kernel void kernel3_2_2_2(__global unsigned char *src, __global unsigned char *dst )
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (8)));

}

__kernel void kernel2_2_2_2(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_2_2_2(src, dst );

}

__kernel void kernel2_2_2(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 5;

    kernel2_2_2_2(src, dst );

}
__kernel void kernel3_2_2_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n)));

}

__kernel void kernel2_2_2_3(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_2_2_3(src, dst ,(unsigned char)8);

}

__kernel void kernel2_2_3(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    kernel2_2_2_3(src, dst );

}
__kernel void kernel3_2_2_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n*m)));

}

__kernel void kernel2_2_2_4(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_2_2_4(src, dst ,(unsigned char)4, (unsigned char)2);

}

__kernel void kernel2_2_4(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    kernel2_2_2_4(src, dst );

}
 void func3_2_3_2(__global unsigned char *src, __global unsigned char *dst )
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (8)));

}

 void func2_2_3_2(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	if(n==5)
		func3_2_3_2(src, dst );

}

__kernel void func2_3_2(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 5;

    func2_2_3_2(src, dst ,n);

}
 void func3_2_3_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n)));

}

 void func2_2_3_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	func3_2_3_3(src, dst ,n*2);

}

__kernel void func2_3_3(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    func2_2_3_3(src, dst ,n);

}
 void func3_2_3_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void func2_2_3_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	func3_2_3_4(src, dst ,n, (unsigned char)2);

}

__kernel void func2_3_4(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    func2_2_3_4(src, dst ,n);

}
__kernel void kernel3_2_3_2(__global unsigned char *src, __global unsigned char *dst )
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (8)));

}

__kernel void kernel2_2_3_2(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	if(n==5)
		kernel3_2_3_2(src, dst );

}

__kernel void kernel2_3_2(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 5;

    kernel2_2_3_2(src, dst ,n);

}
__kernel void kernel3_2_3_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n)));

}

__kernel void kernel2_2_3_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	kernel3_2_3_3(src, dst ,n*2);

}

__kernel void kernel2_3_3(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    kernel2_2_3_3(src, dst ,n);

}
__kernel void kernel3_2_3_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n*m)));

}

__kernel void kernel2_2_3_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	kernel3_2_3_4(src, dst ,n, (unsigned char)2);

}

__kernel void kernel2_3_4(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    kernel2_2_3_4(src, dst ,n);

}
 void func3_2_4_2(__global unsigned char *src, __global unsigned char *dst )
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (8)));

}

 void func2_2_4_2(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	if(m*n==5)
		func3_2_4_2(src, dst );

}

__kernel void func2_4_2(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 1;

    func2_2_4_2(src, dst ,n,5);

}
 void func3_2_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n)));

}

 void func2_2_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	if(m==5)
		func3_2_4_3(src, dst ,n*2);

}

__kernel void func2_4_3(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    func2_2_4_3(src, dst ,n,5);

}
 void func3_2_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void func2_2_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	func3_2_4_4(src, dst ,n, m);

}

__kernel void func2_4_4(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    func2_2_4_4(src, dst ,n,(unsigned char)2);

}
__kernel void kernel3_2_4_2(__global unsigned char *src, __global unsigned char *dst )
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (8)));

}

__kernel void kernel2_2_4_2(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	if(m*n==5)
		kernel3_2_4_2(src, dst );

}

__kernel void kernel2_4_2(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 1;

    kernel2_2_4_2(src, dst ,n,5);

}
__kernel void kernel3_2_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n)));

}

__kernel void kernel2_2_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	if(m==5)
		kernel3_2_4_3(src, dst ,n*2);

}

__kernel void kernel2_4_3(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    kernel2_2_4_3(src, dst ,n,5);

}
__kernel void kernel3_2_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
    int  tid = get_global_id(0);

    dst[tid] = tid (1 << (src[tid] (n*m)));

}

__kernel void kernel2_2_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	kernel3_2_4_4(src, dst ,n, m);

}

__kernel void kernel2_4_4(__global unsigned char *src, __global unsigned char *dst)
{
	__private unsigned char n;
	n= 4;

    kernel2_2_4_4(src, dst ,n,(unsigned char)2);

}
