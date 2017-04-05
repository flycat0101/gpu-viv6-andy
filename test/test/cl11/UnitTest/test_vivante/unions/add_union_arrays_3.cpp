/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
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


#include <iostream>
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <math.h>

#include "types_3.h"
#include "tests.h"

const char *kernel_add_union_arrays_3 =
"#ifndef _TYPES_H_                                                                                                                 \n"
"#define _TYPES_H_                                                                                                                   \n"
"                                                                                                                                   \n"
"#ifdef __OPENCL_VERSION__                                                                                                           \n"
"#define ALIGNED_STRUCT(structureType, alignBytes) structureType __attribute__ ((aligned(alignBytes)))                               \n"
"#else // __OPENCL_VERSION__                                                                                                       \n"
"#define ALIGNED_STRUCT(structureType, alignBytes) __declspec(align(alignBytes)) structureType                                       \n"
"#endif // __OPENCL_VERSION__                                                                                                       \n"
"                                                                                                                                   \n"
"ALIGNED_STRUCT(union, 32) InputA {                                                                                                   \n"
"    short a[4];                                                                                                                       \n"
"    float h[4];                                                                                                                   \n"
"};                                                                                                                                \n"
"                                                                                                                                  \n"
"ALIGNED_STRUCT(union, 32) InputB {                                                                                                \n"
"    short k[4];                                                                                                                     \n"
"    float b[4];                                                                                                                   \n"
"};                                                                                                                                \n"
"                                                                                                                                  \n"
"ALIGNED_STRUCT(union, 32) Result {                                                                                                \n"
"    short z[4];                                                                                                                     \n"
"    float r[4];                                                                                                                   \n"
"};                                                                                                                                \n"
"                                                                                                                                  \n"
"#endif // _TYPES_H_                                                                                                               \n"
"__kernel void sum(__global const union InputA* a, __global const union InputB* b, __global union Result* c, int iNumElements)\n"
"{                                                                                                                                 \n"
"    // get index into global data array                                                                                           \n"
"    int tid = get_global_id(0);                                                                                                   \n"
"                                                                                                                                  \n"
"    // bound check (equivalent to the limit on a 'for' loop for standard/serial C code                                            \n"
"                                                                                                                                  \n"
"    for (int i = 0; i < sizeof(c[tid].r) / sizeof(c[tid].r[0]); ++i) {                                                            \n"
"        c[tid].r[i] = a[tid].a[i] + b[tid].b[i];                                                                                   \n"
"    }                                                                                                                             \n"
"}                                                                                                                                 \n";

class TestCase {
public:
    TestCase(cl::Program program_, cl::Context context, const std::vector<cl::Device>& devices)
        : _program(program_)
        , _context(context)
        , _devices(devices)
    {
    }

    virtual ~TestCase() {}

    virtual void SetUp() {}
    virtual bool Execute() = 0;
    virtual void TearDown() {}

protected:
    cl::Program _program;
    cl::Context _context;
    const std::vector<cl::Device>& _devices;
};

class TestCase_add_union_arrays_3 : public TestCase  {
public:
    TestCase_add_union_arrays_3(int numElements, cl::Program program_, cl::Context context, const std::vector<cl::Device>& devices)
        : TestCase(program_, context, devices)
        , _numElements(numElements)
    {
    }

    virtual void SetUp()
    {
        // Find the relevant kernel
        cl_int err;
        _kernel = cl::Kernel(_program, "sum", &err);

        _localWorkSize = 256;
        _globalWorkSize = (size_t)(_localWorkSize * ceil((float)_numElements / _localWorkSize));  // rounded up to the nearest multiple of the LocalWorkSize

        // Allocate the arrays
        _inputA = new InputA[_globalWorkSize];
        _inputB = new InputB[_globalWorkSize];
        _result = new Result[_globalWorkSize];
        _goldStandard = new Result[_numElements];

        _deviceInputA = cl::Buffer(_context, CL_MEM_READ_ONLY, sizeof(InputA) * _globalWorkSize);
        _deviceInputB = cl::Buffer(_context, CL_MEM_READ_ONLY, sizeof(InputB) * _globalWorkSize);
        _deviceResult = cl::Buffer(_context, CL_MEM_WRITE_ONLY, sizeof(Result) * _globalWorkSize);

        _kernel.setArg(0,sizeof (_deviceInputA), &_deviceInputA);
        _kernel.setArg(1,sizeof (_deviceInputB), &_deviceInputB);
        _kernel.setArg(2,sizeof (_deviceResult), &_deviceResult);
        _kernel.setArg(3,sizeof (_numElements), &_numElements);
    }

    virtual bool Execute()
    {
        _fillData();
        _computeGoldStandard();
        _computeDeviceResult();
        return _compare();
    }

    virtual void TearDown()
    {
        delete [] _inputA;
        delete [] _inputB;
        delete [] _result;
        delete [] _goldStandard;
    }
private:
    void _fillData()
    {
        for (int i = 0; i < _numElements; ++i) {
            for (int j = 0; j < 4; ++j) {
                _inputA[i].a[j] = rand();
                _inputA[i].h[j] = (float)rand();
                _inputB[i].k[j] = rand();
                _inputB[i].b[j] = (float)rand();
            }
        }
    }

    void _computeGoldStandard()
    {
        for (int i = 0; i < _numElements; i++) {
            for (int j = 0; j < 4; ++j) {
                _goldStandard[i].r[j] = _inputA[i].a[j] + (ISSUBNORM(_inputB[i].b[j]) ? 0 : _inputB[i].b[j]);
            }
        }
    }

    void _computeDeviceResult()
    {
        int err;
        cl::CommandQueue queue(_context, _devices[0], 0, &err);

        queue.enqueueWriteBuffer(_deviceInputA, false, 0, sizeof(InputA) * _globalWorkSize, _inputA);
        queue.enqueueWriteBuffer(_deviceInputB, false, 0, sizeof(InputB) * _globalWorkSize, _inputB);
        queue.enqueueNDRangeKernel(_kernel, cl::NullRange, cl::NDRange(_globalWorkSize), cl::NullRange);
        queue.enqueueReadBuffer(_deviceResult, true, 0, sizeof(Result) * _globalWorkSize, _result);
    }

    bool _compare()
    {
        unsigned int checkResult1;
        unsigned int checkResult2;
        float checkResult3;
        float checkResult4;

        for (int i = 0; i < _numElements; ++i) {
            for (int j = 0; j < 4; ++j) {
                checkResult1 = (*(unsigned int *)&(_goldStandard[i].r[j]) + 2);
                checkResult2 = (*(unsigned int *)&(_goldStandard[i].r[j]) - 2);
                checkResult3 = fabsf(*( float *) &checkResult1);
                checkResult4 = fabsf(*( float *) &checkResult2);
                if ((fabsf(_result[i].r[j]) > checkResult3 || fabsf(_result[i].r[j]) < checkResult4) && !isnan(_goldStandard[i].r[j]) && !isinf(_goldStandard[i].r[j])){
                    std::cout << "  c:" << _goldStandard[i].r[j] << " ocl:" << _result[i].r[j] << std::endl;
                    return false;
                }
            }
        }
        return true;
    }

private:
    int _numElements;

    InputA* _inputA;
    InputB* _inputB;
    Result* _result;
    Result* _goldStandard;

    cl::Kernel _kernel;
    cl::Buffer _deviceInputA;
    cl::Buffer _deviceInputB;
    cl::Buffer _deviceResult;

    size_t _globalWorkSize;
    size_t _localWorkSize;
};



int add_union_arrays_3(void)
{
    cl_int err = CL_SUCCESS;
    int cnt = 1;

    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.size() == 0) {
            std::cerr << "Failed to find any platforms." << std::endl;
            return 0;
        }

        cl_context_properties properties[] =
        { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
        cl::Context context(CL_DEVICE_TYPE_GPU, properties);

        std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        
        const char* clProgramSource = kernel_add_union_arrays_3;//oclLoadProgSource("multiply.cl", "", &szKernelLength);
        if (clProgramSource == 0) {
            std::cerr << "OpenCL program not found." << std::endl;
            return 0;
        }

        cl::Program::Sources source(1, std::make_pair(clProgramSource,strlen(clProgramSource)));
        cl::Program program_ = cl::Program(context, source);
        try {
            program_.build(devices);
        }
        catch (...) {
            for (size_t i = 0; i < devices.size(); ++i) {
                cl::STRING_CLASS  log;
                program_.getBuildInfo(devices[i], CL_PROGRAM_BUILD_LOG, &log);
                std::cerr << log.c_str() << std::endl << std::endl;
            }
            throw;
        }

        std::cout << "Running test add_union_arrays_3..." << std::endl;
        TestCase_add_union_arrays_3 add_union_arrays_3(10, program_, context, devices);

        bool control = true;
        add_union_arrays_3.SetUp();
        for (int i = 0; i < 10; ++i) {
            if(!add_union_arrays_3.Execute()){
                control = false;
                cnt = 0;
            }
            std::cout << "RUN " << i + 1<< ": " << (control ? "PASSED" : "FAILED!") << std::endl;
        }
        add_union_arrays_3.TearDown();
    }
    catch (cl::Error err) {
        std::cerr
            << "ERROR: "
            << err.what()
            << "("
            << err.err()
            << ")"
            << std::endl;
    }
    return cnt;
}