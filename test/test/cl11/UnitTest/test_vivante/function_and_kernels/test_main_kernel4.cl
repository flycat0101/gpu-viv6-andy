__kernel void kernel3_4_2(__global unsigned char *src, __global unsigned char *dst )
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (8)));

}

 void NULLkernel2_4_2(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_4_2(src, dst );

}

__kernel void kernel4_2(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 0;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel3_4_2(src, dst );
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n)));

}

 void NULLkernel2_4_3(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_4_3(src, dst ,0);

}

__kernel void kernel4_3(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 8;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel3_4_3(src, dst ,n);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void NULLkernel2_4_4(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_4_4(src, dst ,0,0);

}

__kernel void kernel4_4(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel3_4_4(src, dst ,n, (unsigned char)2);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_2(__global unsigned char *src, __global unsigned char *dst )
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (8)));

}

 void NULLfunc2_4_2(__global unsigned char *src, __global unsigned char *dst )
{

	func3_4_2(src, dst );

}

__kernel void func4_2(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 0;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func3_4_2(src, dst );
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n)));

}

 void NULLfunc2_4_3(__global unsigned char *src, __global unsigned char *dst )
{

	func3_4_3(src, dst ,0);

}

__kernel void func4_3(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 8;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func3_4_3(src, dst ,n);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void NULLfunc2_4_4(__global unsigned char *src, __global unsigned char *dst )
{

	func3_4_4(src, dst ,0,0);

}

__kernel void func4_4(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func3_4_4(src, dst ,n, (unsigned char)2);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_2_2(__global unsigned char *src, __global unsigned char *dst )
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (8)));

}

 void func2_4_2_2(__global unsigned char *src, __global unsigned char *dst )
{

	func3_4_2_2(src, dst );

}

__kernel void func4_2_2(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 5;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func2_4_2_2(src, dst );
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_2_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n)));

}

 void func2_4_2_3(__global unsigned char *src, __global unsigned char *dst )
{

	func3_4_2_3(src, dst ,(unsigned char)8);

}

__kernel void func4_2_3(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func2_4_2_3(src, dst );
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_2_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void func2_4_2_4(__global unsigned char *src, __global unsigned char *dst )
{

	func3_4_2_4(src, dst ,(unsigned char)4, (unsigned char)2);

}

__kernel void func4_2_4(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func2_4_2_4(src, dst );
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_2_2(__global unsigned char *src, __global unsigned char *dst )
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (8)));

}

__kernel void kernel2_4_2_2(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_4_2_2(src, dst );

}

__kernel void kernel4_2_2(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 5;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel2_4_2_2(src, dst );
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_2_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n)));

}

__kernel void kernel2_4_2_3(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_4_2_3(src, dst ,(unsigned char)8);

}

__kernel void kernel4_2_3(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel2_4_2_3(src, dst );
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_2_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n*m)));

}

__kernel void kernel2_4_2_4(__global unsigned char *src, __global unsigned char *dst )
{

	kernel3_4_2_4(src, dst ,(unsigned char)4, (unsigned char)2);

}

__kernel void kernel4_2_4(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel2_4_2_4(src, dst );
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_3_2(__global unsigned char *src, __global unsigned char *dst )
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (8)));

}

 void func2_4_3_2(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	if(n==5)
		func3_4_3_2(src, dst );

}

__kernel void func4_3_2(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 5;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func2_4_3_2(src, dst ,n);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_3_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n)));

}

 void func2_4_3_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	func3_4_3_3(src, dst ,n*2);

}

__kernel void func4_3_3(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func2_4_3_3(src, dst ,n);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_3_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void func2_4_3_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	func3_4_3_4(src, dst ,n, (unsigned char)2);

}

__kernel void func4_3_4(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func2_4_3_4(src, dst ,n);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_3_2(__global unsigned char *src, __global unsigned char *dst )
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (8)));

}

__kernel void kernel2_4_3_2(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	if(n==5)
		kernel3_4_3_2(src, dst );

}

__kernel void kernel4_3_2(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 5;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel2_4_3_2(src, dst ,n);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_3_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n)));

}

__kernel void kernel2_4_3_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	kernel3_4_3_3(src, dst ,n*2);

}

__kernel void kernel4_3_3(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel2_4_3_3(src, dst ,n);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_3_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n*m)));

}

__kernel void kernel2_4_3_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{

	kernel3_4_3_4(src, dst ,n, (unsigned char)2);

}

__kernel void kernel4_3_4(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel2_4_3_4(src, dst ,n);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_4_2(__global unsigned char *src, __global unsigned char *dst )
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (8)));

}

 void func2_4_4_2(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	if(m*n==5)
		func3_4_4_2(src, dst );

}

__kernel void func4_4_2(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 1;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func2_4_4_2(src, dst ,n,5);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n)));

}

 void func2_4_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	if(m==5)
		func3_4_4_3(src, dst ,n*2);

}

__kernel void func4_4_3(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func2_4_4_3(src, dst ,n,5);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
 void func3_4_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n*m)));

}

 void func2_4_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	func3_4_4_4(src, dst ,n, m);

}

__kernel void func4_4_4(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		func2_4_4_4(src, dst ,n,(unsigned char)2);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_4_2(__global unsigned char *src, __global unsigned char *dst )
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (8)));

}

__kernel void kernel2_4_4_2(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	if(m*n==5)
		kernel3_4_4_2(src, dst );

}

__kernel void kernel4_4_2(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 1;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel2_4_4_2(src, dst ,n,5);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n)));

}

__kernel void kernel2_4_4_3(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	if(m==5)
		kernel3_4_4_3(src, dst ,n*2);

}

__kernel void kernel4_4_3(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel2_4_4_3(src, dst ,n,5);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
__kernel void kernel3_4_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{
   int  tid = get_global_id(0);

   dst[tid] = tid (1 << (src[tid] (n*m)));

}

__kernel void kernel2_4_4_4(__global unsigned char *src, __global unsigned char *dst , unsigned char n , unsigned char m)
{

	kernel3_4_4_4(src, dst ,n, m);

}

__kernel void kernel4_4_4(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)
{
	__private unsigned char n;
	n= 4;
	int  tid = get_global_id(0);

	if(test2[tid] == test1[tid]+tid)
		kernel2_4_4_4(src, dst ,n,(unsigned char)2);
//test1[tid] = test1[tid]+tid;// + test2[tid];

}
