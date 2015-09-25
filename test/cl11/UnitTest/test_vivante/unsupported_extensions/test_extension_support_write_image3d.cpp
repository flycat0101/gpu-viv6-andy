/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include "testBase.h"
#ifndef _WIN32
#include <unistd.h>
#endif

const char *kernel_write_image3d = "#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable\n"
"__kernel void test_rgba8888_write(__global unsigned char *src, write_only image3d_t dstimg)\n"
"{\n"
"   int tid_x = get_global_id(0);\n"
"   int tid_y = get_global_id(1);\n"
"	int tid_z = get_global_id(2);\n"
"   int indx = tid_y * get_image_width(dstimg) + tid_z * get_image_depth(dstimg) + tid_x;\n"
"   %s4 color;\n"
"   indx *= 4;\n"
"   color = (%s4)((%s)src[indx+0], (%s)src[indx+1], (%s)src[indx+2], (%s)src[indx+3]);\n"
"   %s\n"
"   %s(dstimg, (int4)(tid_x, tid_y, tid_z, 0), color);\n"
"}\n";

unsigned char *generate_8888_image(int w, int h, int d)
{

	cl_uchar *ptr = (cl_uchar *)malloc(w * h * d * 4);
	int i;

	for (i=0; i<w*h*4; i++)
		ptr[i] = (cl_uchar)(rand()%256);

	return ptr;
}

int test_write_image3d(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail)
{
	cl_mem streams[2];
	cl_program program;
	cl_kernel kernel;
	size_t threads[3];
	char kernel_code_int[1024];
	const char *constkernelint;
	cl_uchar *output_h, *input_h;
	cl_image_format	img_format;
	int err;
	int img_width = 512;
	int img_height = 512;
	int img_depth = 512;

	size_t origin[3] = {0, 0, 0};
	size_t region[3] = {img_width, img_height, 1};
	size_t length = img_width * img_height * 4 * sizeof(unsigned char);

	int passCount = 0;
	int typeIndex = 0;

	char *floatNormalization = "color /= (float4)(255.0f, 255.0f, 255.0f, 255.0f);";
	const char    *types[] = {
		"float",
		"int",
		"unsigned int"
	};

	const char *func[] = {
		"write_imagef",
		"write_imagei",
		"write_imageui"
	};

	output_h = (cl_uchar*)malloc(length);
	srand ( (unsigned int)time(NULL) );
	input_h = generate_8888_image(img_width, img_height, img_depth);

	img_format.image_channel_order = CL_RGBA;



	for(int i = 0; i < 3; i++)
	{
		switch(i)
		{
			case 0:
				img_format.image_channel_data_type = CL_UNORM_INT8;
				break;
			case 1:
				img_format.image_channel_data_type = CL_SIGNED_INT8;
				floatNormalization = "\n";
				break;
			case 2:
				img_format.image_channel_data_type = CL_UNSIGNED_INT8;
				floatNormalization = "\n";
				break;
			default:
				break;
		}


		// if this condition is true, this means device does not support 3D images
		if(!clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, 0, NULL, NULL))
		{
			//only check if kernel build is successful
			sprintf(kernel_code_int, kernel_write_image3d, types[typeIndex], types[typeIndex], types[typeIndex], types[typeIndex], types[typeIndex],
				types[typeIndex], floatNormalization, func[i]);
			constkernelint = kernel_code_int;
			err = create_kernel(context, &program, &kernel, 1, &constkernelint, "test_rgba8888_write" );
			if (err)
			{
				// check if image extension is supported
				if(is_extension_available(device, "cl_khr_3d_image_writes"))
				{
					printf("Kernel compilation error using 3D image writes.\n");
					clReleaseMemObject(streams[0]);
					clReleaseMemObject(streams[1]);
					if(!kernel)
						clReleaseKernel(kernel);
					clReleaseProgram(program);
					free(output_h);
					return -1;
				}
				printf("\n!!Kernel build not successful.\n");
				printf("cl_khr_3d_image_writes extension is not supported.!!\n");
				printf("Test passed.\n");

				passCount++;
                                typeIndex++;

				continue;
			}
		}
		else
		{
			streams[0] = clCreateImage3D(context, CL_MEM_READ_WRITE, &img_format, img_width, img_height, img_depth, 0, 0, NULL, NULL);
			if (!streams[0])
			{
				printf("clCreateImage3D failed\n");
				free(output_h);
				free(input_h);
				return -1;
			}

			streams[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
			if (!streams[1])
			{
				printf("clCreateImage3D failed\n");
				free(output_h);
				free(input_h);
				return -1;
			}

			err = clEnqueueWriteBuffer(queue, streams[0], CL_TRUE, 0, length, input_h, 0, NULL, NULL);
			if (err != CL_SUCCESS)
			{
				printf("clEnqueueWriteBuffer failed\n");
				free(output_h);
				free(input_h);
				clReleaseMemObject(streams[0]);
				return -1;
			}

			sprintf(kernel_code_int, kernel_write_image3d, types[typeIndex], types[typeIndex], types[typeIndex], types[typeIndex]);
			constkernelint = kernel_code_int;
			err = create_kernel(context, &program, &kernel, 1, &constkernelint, "test_rgba8888_write" );
			if (err)
			{
				printf("kernel build failed\n");
				clReleaseMemObject(streams[0]);
				clReleaseMemObject(streams[1]);
				if(!kernel)
					clReleaseKernel(kernel);
				clReleaseProgram(program);
				free(output_h);
				return -1;
			}

			err  = clSetKernelArg(kernel, 0, sizeof(streams[0]), &streams[0]);
			if (err != CL_SUCCESS)
			{
				printf("clSetKernelArgs failed\n");
				clReleaseMemObject(streams[0]);
				clReleaseMemObject(streams[1]);
				if(!kernel)
					clReleaseKernel(kernel);
				clReleaseProgram(program);
				free(output_h);
				return -1;
			}

			err  = clSetKernelArg(kernel, 0, sizeof(streams[1]), &streams[1]);
			if (err != CL_SUCCESS)
			{
				printf("clSetKernelArgs failed\n");
				clReleaseMemObject(streams[0]);
				clReleaseMemObject(streams[1]);
				if(!kernel)
					clReleaseKernel(kernel);
				clReleaseProgram(program);
				free(output_h);
				return -1;
			}

			threads[0] = img_width;
			threads[1] = img_height;
			threads[2] = img_depth;

			err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, threads, NULL, 0, NULL, NULL);
			if (err != CL_SUCCESS)
			{
				printf("clEnqueueNDRangeKernel failed\n");
				clReleaseMemObject(streams[0]);
				clReleaseMemObject(streams[1]);
				if(!kernel)
					clReleaseKernel(kernel);
				clReleaseProgram(program);
				free(output_h);
				return -1;
			}

			clReleaseMemObject(streams[0]);
			clReleaseMemObject(streams[1]);
			if(!kernel)
				clReleaseKernel(kernel);
			clReleaseProgram(program);
			free(output_h);
		}

		typeIndex++;
	}

/*
	printf("--------------------------------------------------------\n");
	printf("cl_khr_fp16 tests:\n");
	printf("%d / %d internal tests passed.\n",passCount, typeIndex);
*/
	total += typeIndex;
	passed += passCount;
	fail += typeIndex - passCount;

	return 0;
}
