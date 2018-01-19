/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_cl_log_h_
#define __gc_cl_log_h_

#ifdef __cplusplus
extern "C" {
#endif
/*change clCreateContext_Post to clCreateContext for driver get function address in vlogger. Other API likes this also.*/
#define __CL_API_ENTRY(CLApiMacro) \
    CLApiMacro(GetPlatformIDs),\
    CLApiMacro(GetPlatformInfo),\
    CLApiMacro(GetDeviceIDs),\
    CLApiMacro(GetDeviceInfo),\
    CLApiMacro(CreateSubDevices),\
    CLApiMacro(RetainDevice),\
    CLApiMacro(ReleaseDevice),\
    CLApiMacro(CreateContext_Pre),\
    CLApiMacro(CreateContext),\
    CLApiMacro(CreateContextFromType_Pre),\
    CLApiMacro(CreateContextFromType),\
    CLApiMacro(RetainContext),\
    CLApiMacro(ReleaseContext),\
    CLApiMacro(GetContextInfo),\
    CLApiMacro(CreateCommandQueue_Pre),\
    CLApiMacro(CreateCommandQueue),\
    CLApiMacro(RetainCommandQueue),\
    CLApiMacro(ReleaseCommandQueue),\
    CLApiMacro(GetCommandQueueInfo),\
    CLApiMacro(CreateBuffer_Pre),\
    CLApiMacro(CreateBuffer),\
    CLApiMacro(CreateSubBuffer_Pre),\
    CLApiMacro(CreateSubBuffer),\
    CLApiMacro(CreateImage_Pre),\
    CLApiMacro(CreateImage),\
    CLApiMacro(RetainMemObject),\
    CLApiMacro(ReleaseMemObject),\
    CLApiMacro(GetSupportedImageFormats),\
    CLApiMacro(GetMemObjectInfo),\
    CLApiMacro(GetImageInfo),\
    CLApiMacro(SetMemObjectDestructorCallback),\
    CLApiMacro(CreateSampler_Pre),\
    CLApiMacro(CreateSampler),\
    CLApiMacro(RetainSampler),\
    CLApiMacro(ReleaseSampler),\
    CLApiMacro(GetSamplerInfo),\
    CLApiMacro(CreateProgramWithSource_Pre),\
    CLApiMacro(CreateProgramWithSource),\
    CLApiMacro(CreateProgramWithBinary_Pre),\
    CLApiMacro(CreateProgramWithBinary),\
    CLApiMacro(CreateProgramWithBuiltInKernels_Pre),\
    CLApiMacro(CreateProgramWithBuiltInKernels),\
    CLApiMacro(RetainProgram),\
    CLApiMacro(ReleaseProgram),\
    CLApiMacro(BuildProgram),\
    CLApiMacro(CompileProgram),\
    CLApiMacro(LinkProgram_Pre),\
    CLApiMacro(LinkProgram),\
    CLApiMacro(UnloadPlatformCompiler),\
    CLApiMacro(GetProgramInfo),\
    CLApiMacro(GetProgramBuildInfo),\
    CLApiMacro(CreateKernel_Pre),\
    CLApiMacro(CreateKernel),\
    CLApiMacro(CreateKernelsInProgram),\
    CLApiMacro(RetainKernel),\
    CLApiMacro(ReleaseKernel),\
    CLApiMacro(SetKernelArg),\
    CLApiMacro(GetKernelInfo),\
    CLApiMacro(GetKernelArgInfo),\
    CLApiMacro(GetKernelWorkGroupInfo),\
    CLApiMacro(WaitForEvents),\
    CLApiMacro(GetEventInfo),\
    CLApiMacro(CreateUserEvent_Pre),\
    CLApiMacro(CreateUserEvent),\
    CLApiMacro(RetainEvent),\
    CLApiMacro(ReleaseEvent),\
    CLApiMacro(SetUserEventStatus),\
    CLApiMacro(SetEventCallback),\
    CLApiMacro(GetEventProfilingInfo),\
    CLApiMacro(Flush),\
    CLApiMacro(Finish),\
    CLApiMacro(EnqueueReadBuffer),\
    CLApiMacro(EnqueueReadBufferRect),\
    CLApiMacro(EnqueueWriteBuffer),\
    CLApiMacro(EnqueueWriteBufferRect),\
    CLApiMacro(EnqueueFillBuffer),\
    CLApiMacro(EnqueueCopyBuffer),\
    CLApiMacro(EnqueueCopyBufferRect),\
    CLApiMacro(EnqueueReadImage),\
    CLApiMacro(EnqueueWriteImage),\
    CLApiMacro(EnqueueFillImage),\
    CLApiMacro(EnqueueCopyImage),\
    CLApiMacro(EnqueueCopyImageToBuffer),\
    CLApiMacro(EnqueueCopyBufferToImage),\
    CLApiMacro(EnqueueMapBuffer_Pre),\
    CLApiMacro(EnqueueMapBuffer),\
    CLApiMacro(EnqueueMapImage_Pre),\
    CLApiMacro(EnqueueMapImage),\
    CLApiMacro(EnqueueUnmapMemObject),\
    CLApiMacro(EnqueueMigrateMemObjects),\
    CLApiMacro(EnqueueNDRangeKernel),\
    CLApiMacro(EnqueueTask),\
    CLApiMacro(EnqueueNativeKernel),\
    CLApiMacro(EnqueueMarkerWithWaitList),\
    CLApiMacro(EnqueueBarrierWithWaitList),\
    CLApiMacro(GetExtensionFunctionAddressForPlatform_Pre),\
    CLApiMacro(GetExtensionFunctionAddressForPlatform),\
    CLApiMacro(CreateImage2D_Pre),\
    CLApiMacro(CreateImage2D),\
    CLApiMacro(CreateImage3D_Pre),\
    CLApiMacro(CreateImage3D),\
    CLApiMacro(EnqueueMarker),\
    CLApiMacro(EnqueueWaitForEvents),\
    CLApiMacro(EnqueueBarrier),\
    CLApiMacro(UnloadCompiler),\
    CLApiMacro(getExtensionFunctionAddress_Pre),\
    CLApiMacro(getExtensionFunctionAddress),\
    CLApiMacro(IcdGetPlatformIDsKHR),\
    CLApiMacro(CreateFromGLBuffer_Pre),\
    CLApiMacro(CreateFromGLBuffer),\
    CLApiMacro(CreateFromGLTexture_Pre),\
    CLApiMacro(CreateFromGLTexture),\
    CLApiMacro(CreateFromGLRenderbuffer_Pre),\
    CLApiMacro(CreateFromGLRenderbuffer),\
    CLApiMacro(GetGLObjectInfo),\
    CLApiMacro(GetGLTextureInfo),\
    CLApiMacro(EnqueueAcquireGLObjects),\
    CLApiMacro(EnqueueReleaseGLObjects),\
    CLApiMacro(CreateFromGLTexture2D_Pre),\
    CLApiMacro(CreateFromGLTexture2D),\
    CLApiMacro(CreateFromGLTexture3D_Pre),\
    CLApiMacro(CreateFromGLTexture3D),\
    CLApiMacro(GetGLContextInfoKHR),\
    CLApiMacro(CreateEventFromGLsyncKHR_Pre),\
    CLApiMacro(CreateEventFromGLsyncKHR),


typedef struct {
    cl_int (* GetPlatformIDs)(cl_uint /* num_entries */,
                              cl_platform_id * /* platforms */,
                              cl_uint * /* num_platforms */);

    cl_int (* GetPlatformInfo)(cl_platform_id /* platform */,
                               cl_platform_info /* param_name */,
                               size_t /* param_value_size */,
                               void * /* param_value */,
                               size_t * /* param_value_size_ret */);

    cl_int (* GetDeviceIDs)(cl_platform_id   /* platform */,
                            cl_device_type   /* device_type */,
                            cl_uint          /* num_entries */,
                            cl_device_id *   /* devices */,
                            cl_uint *        /* num_devices */);

    cl_int (* GetDeviceInfo)(cl_device_id    /* device */,
                             cl_device_info  /* param_name */,
                             size_t          /* param_value_size */,
                             void *          /* param_value */,
                             size_t *        /* param_value_size_ret */);

    cl_int (* CreateSubDevices)(cl_device_id                         /* in_device */,
                                const cl_device_partition_property * /* properties */,
                                cl_uint                              /* num_devices */,
                                cl_device_id *                       /* out_devices */,
                                cl_uint *                            /* num_devices_ret */);

    cl_int (* RetainDevice)(cl_device_id /* device */);

    cl_int (* ReleaseDevice)(cl_device_id /* device */);

    cl_context (* CreateContext_Pre)(const cl_context_properties * /* properties */,
                                     cl_uint                 /* num_devices */,
                                     const cl_device_id *    /* devices */,
                                     void (CL_CALLBACK * /* pfn_notify */)(const char *, const void *, size_t, void *),
                                     void *                  /* user_data */,
                                     cl_int *                /* errcode_ret */);

    cl_context (* CreateContext_Post)(const cl_context_properties * /* properties */,
                                      cl_uint                 /* num_devices */,
                                      const cl_device_id *    /* devices */,
                                      void (CL_CALLBACK * /* pfn_notify */)(const char *, const void *, size_t, void *),
                                      void *                  /* user_data */,
                                      cl_int *                /* errcode_ret */,
                                      cl_context /* context created */);

    cl_context (* CreateContextFromType_Pre)(const cl_context_properties * /* properties */,
                                             cl_device_type          /* device_type */,
                                             void (CL_CALLBACK *     /* pfn_notify*/ )(const char *, const void *, size_t, void *),
                                             void *                  /* user_data */,
                                             cl_int *                /* errcode_ret */);

    cl_context (* CreateContextFromType_Post)(const cl_context_properties * /* properties */,
                                              cl_device_type          /* device_type */,
                                              void (CL_CALLBACK *     /* pfn_notify*/ )(const char *, const void *, size_t, void *),
                                              void *                  /* user_data */,
                                              cl_int *                /* errcode_ret */,
                                              cl_context /* context created */);

    cl_int (* RetainContext)(cl_context /* context */);

    cl_int (* ReleaseContext)(cl_context /* context */);

    cl_int (* GetContextInfo)(cl_context         /* context */,
                              cl_context_info    /* param_name */,
                              size_t             /* param_value_size */,
                              void *             /* param_value */,
                              size_t *           /* param_value_size_ret */);

    cl_command_queue (*CreateCommandQueue_Pre)(cl_context                     /* context */,
                                               cl_device_id                   /* device */,
                                               cl_command_queue_properties    /* properties */,
                                               cl_int *                       /* errcode_ret */);

    cl_command_queue (*CreateCommandQueue_Post)(cl_context                     /* context */,
                                                cl_device_id                   /* device */,
                                                cl_command_queue_properties    /* properties */,
                                                cl_int *                       /* errcode_ret */,
                                                cl_command_queue               /* commandqueue created */);

    cl_int (* RetainCommandQueue)(cl_command_queue /* command_queue */);

    cl_int (* ReleaseCommandQueue)(cl_command_queue /* command_queue */);

    cl_int (* GetCommandQueueInfo)(cl_command_queue      /* command_queue */,
                                   cl_command_queue_info /* param_name */,
                                   size_t                /* param_value_size */,
                                   void *                /* param_value */,
                                   size_t *              /* param_value_size_ret */);

    cl_mem (* CreateBuffer_Pre)(cl_context   /* context */,
                                cl_mem_flags /* flags */,
                                size_t       /* size */,
                                void *       /* host_ptr */,
                                cl_int *     /* errcode_ret */);

    cl_mem (* CreateBuffer_Post)(cl_context   /* context */,
                                 cl_mem_flags /* flags */,
                                 size_t       /* size */,
                                 void *       /* host_ptr */,
                                 cl_int *     /* errcode_ret */,
                                 cl_mem       /* bufobject created */);

    cl_mem (* CreateSubBuffer_Pre)(cl_mem                   /* buffer */,
                                   cl_mem_flags             /* flags */,
                                   cl_buffer_create_type    /* buffer_create_type */,
                                   const void *             /* buffer_create_info */,
                                   cl_int *                 /* errcode_ret */);

    cl_mem (* CreateSubBuffer_Post)(cl_mem                   /* buffer */,
                                    cl_mem_flags             /* flags */,
                                    cl_buffer_create_type    /* buffer_create_type */,
                                    const void *             /* buffer_create_info */,
                                    cl_int *                 /* errcode_ret */,
                                    cl_mem                   /* bufobject created */);

    cl_mem (* CreateImage_Pre)(cl_context              /* context */,
                               cl_mem_flags            /* flags */,
                               const cl_image_format * /* image_format */,
                               const cl_image_desc *   /* image_desc */,
                               void *                  /* host_ptr */,
                               cl_int *                /* errcode_ret */);

    cl_mem (* CreateImage_Post)(cl_context              /* context */,
                                cl_mem_flags            /* flags */,
                                const cl_image_format * /* image_format */,
                                const cl_image_desc *   /* image_desc */,
                                void *                  /* host_ptr */,
                                cl_int *                /* errcode_ret */,
                                cl_mem                  /* imgonject created */);

    cl_int (* RetainMemObject)(cl_mem /* memobj */);

    cl_int (* ReleaseMemObject)(cl_mem /* memobj */);

    cl_int (* GetSupportedImageFormats)(cl_context           /* context */,
                                        cl_mem_flags         /* flags */,
                                        cl_mem_object_type   /* image_type */,
                                        cl_uint              /* num_entries */,
                                        cl_image_format *    /* image_formats */,
                                        cl_uint *            /* num_image_formats */);

    cl_int (* GetMemObjectInfo)(cl_mem           /* memobj */,
                                cl_mem_info      /* param_name */,
                                size_t           /* param_value_size */,
                                void *           /* param_value */,
                                size_t *         /* param_value_size_ret */);

    cl_int (* GetImageInfo)(cl_mem           /* memobj */,
                            cl_mem_info      /* param_name */,
                            size_t           /* param_value_size */,
                            void *           /* param_value */,
                            size_t *         /* param_value_size_ret */);

    cl_int (* SetMemObjectDestructorCallback)(cl_mem /* memobj */,
                                              void (CL_CALLBACK * /*pfn_notify*/)( cl_mem /* memobj */, void* /*user_data*/),
                                              void * /*user_data */ );

    cl_sampler (* CreateSampler_Pre)(cl_context          /* context */,
                                     cl_bool             /* normalized_coords */,
                                     cl_addressing_mode  /* addressing_mode */,
                                     cl_filter_mode      /* filter_mode */,
                                     cl_int *            /* errcode_ret */);

    cl_sampler (* CreateSampler_Post)(cl_context          /* context */,
                                      cl_bool             /* normalized_coords */,
                                      cl_addressing_mode  /* addressing_mode */,
                                      cl_filter_mode      /* filter_mode */,
                                      cl_int *            /* errcode_ret */,
                                      cl_sampler          /* samplerobj created */);

    cl_int (* RetainSampler)(cl_sampler /* sampler */);

    cl_int (* ReleaseSampler)(cl_sampler /* sampler */);

    cl_int (* GetSamplerInfo)(cl_sampler         /* sampler */,
                              cl_sampler_info    /* param_name */,
                              size_t             /* param_value_size */,
                              void *             /* param_value */,
                              size_t *           /* param_value_size_ret */);

    cl_program (* CreateProgramWithSource_Pre)(cl_context        /* context */,
                                               cl_uint           /* count */,
                                               const char **     /* strings */,
                                               const size_t *    /* lengths */,
                                               cl_int *          /* errcode_ret */);

    cl_program (* CreateProgramWithSource_Post)(cl_context        /* context */,
                                                cl_uint           /* count */,
                                                const char **     /* strings */,
                                                const size_t *    /* lengths */,
                                                cl_int *          /* errcode_ret */,
                                                cl_program        /* programobg created */);

    cl_program (* CreateProgramWithBinary_Pre)(cl_context                     /* context */,
                                               cl_uint                        /* num_devices */,
                                               const cl_device_id *           /* device_list */,
                                               const size_t *                 /* lengths */,
                                               const unsigned char **         /* binaries */,
                                               cl_int *                       /* binary_status */,
                                               cl_int *                       /* errcode_ret */);

    cl_program (* CreateProgramWithBinary_Post)(cl_context                     /* context */,
                                                cl_uint                        /* num_devices */,
                                                const cl_device_id *           /* device_list */,
                                                const size_t *                 /* lengths */,
                                                const unsigned char **         /* binaries */,
                                                cl_int *                       /* binary_status */,
                                                cl_int *                       /* errcode_ret */,
                                                cl_program                     /* programobj created */);

    cl_program (* CreateProgramWithBuiltInKernels_Pre)(cl_context            /* context */,
                                                       cl_uint               /* num_devices */,
                                                       const cl_device_id *  /* device_list */,
                                                       const char *          /* kernel_names */,
                                                       cl_int *              /* errcode_ret */);

    cl_program (* CreateProgramWithBuiltInKernels_Post)(cl_context            /* context */,
                                                        cl_uint               /* num_devices */,
                                                        const cl_device_id *  /* device_list */,
                                                        const char *          /* kernel_names */,
                                                        cl_int *              /* errcode_ret */,
                                                        cl_program            /* programobj created */);

    cl_int (* RetainProgram)(cl_program /* program */);

    cl_int (* ReleaseProgram)(cl_program /* program */);

    cl_int (* BuildProgram)(cl_program           /* program */,
                            cl_uint              /* num_devices */,
                            const cl_device_id * /* device_list */,
                            const char *         /* options */,
                            void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                            void *               /* user_data */);

    cl_int (* CompileProgram)(cl_program           /* program */,
                              cl_uint              /* num_devices */,
                              const cl_device_id * /* device_list */,
                              const char *         /* options */,
                              cl_uint              /* num_input_headers */,
                              const cl_program *   /* input_headers */,
                              const char **        /* header_include_names */,
                              void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                              void *               /* user_data */);

    cl_program (* LinkProgram_Pre)(cl_context           /* context */,
                                   cl_uint              /* num_devices */,
                                   const cl_device_id * /* device_list */,
                                   const char *         /* options */,
                                   cl_uint              /* num_input_programs */,
                                   const cl_program *   /* input_programs */,
                                   void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                                   void *               /* user_data */,
                                   cl_int *             /* errcode_ret */ );

    cl_program (* LinkProgram_Post)(cl_context           /* context */,
                                    cl_uint              /* num_devices */,
                                    const cl_device_id * /* device_list */,
                                    const char *         /* options */,
                                    cl_uint              /* num_input_programs */,
                                    const cl_program *   /* input_programs */,
                                    void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                                    void *               /* user_data */,
                                    cl_int *             /* errcode_ret */,
                                    cl_program           /* programobj created */);

    cl_int (* UnloadPlatformCompiler)(cl_platform_id /* platform */);

    cl_int (* GetProgramInfo)(cl_program         /* program */,
                             cl_program_info    /* param_name */,
                             size_t             /* param_value_size */,
                             void *             /* param_value */,
                             size_t *           /* param_value_size_ret */);

    cl_int (* GetProgramBuildInfo)(cl_program            /* program */,
                                   cl_device_id          /* device */,
                                   cl_program_build_info /* param_name */,
                                   size_t                /* param_value_size */,
                                   void *                /* param_value */,
                                   size_t *              /* param_value_size_ret */);

    cl_kernel (* CreateKernel_Pre)(cl_program      /* program */,
                                   const char *    /* kernel_name */,
                                   cl_int *        /* errcode_ret */);

    cl_kernel (* CreateKernel_Post)(cl_program      /* program */,
                                    const char *    /* kernel_name */,
                                    cl_int *        /* errcode_ret */,
                                    cl_kernel       /* kernelobj created */);

    cl_int (* CreateKernelsInProgram)(cl_program     /* program */,
                                      cl_uint        /* num_kernels */,
                                      cl_kernel *    /* kernels */,
                                      cl_uint *      /* num_kernels_ret */);

    cl_int (* RetainKernel)(cl_kernel    /* kernel */);

    cl_int (* ReleaseKernel)(cl_kernel    /* kernel */);

    cl_int (* SetKernelArg)(cl_kernel    /* kernel */,
                            cl_uint      /* arg_index */,
                            size_t       /* arg_size */,
                            const void * /* arg_value */);

    cl_int (* GetKernelInfo)(cl_kernel       /* kernel */,
                             cl_kernel_info  /* param_name */,
                             size_t          /* param_value_size */,
                             void *          /* param_value */,
                             size_t *        /* param_value_size_ret */);

    cl_int (* GetKernelArgInfo)(cl_kernel       /* kernel */,
                                cl_uint         /* arg_indx */,
                                cl_kernel_arg_info  /* param_name */,
                                size_t          /* param_value_size */,
                                void *          /* param_value */,
                                size_t *        /* param_value_size_ret */);

    cl_int (* GetKernelWorkGroupInfo)(cl_kernel                  /* kernel */,
                                      cl_device_id               /* device */,
                                      cl_kernel_work_group_info  /* param_name */,
                                      size_t                     /* param_value_size */,
                                      void *                     /* param_value */,
                                      size_t *                   /* param_value_size_ret */);

    cl_int (* WaitForEvents)(cl_uint             /* num_events */,
                             const cl_event *    /* event_list */);

    cl_int (* GetEventInfo)(cl_event         /* event */,
                            cl_event_info    /* param_name */,
                            size_t           /* param_value_size */,
                            void *           /* param_value */,
                            size_t *         /* param_value_size_ret */);

    cl_event (* CreateUserEvent_Pre)(cl_context    /* context */,
                                     cl_int *      /* errcode_ret */);

    cl_event (* CreateUserEvent_Post)(cl_context    /* context */,
                                      cl_int *      /* errcode_ret */,
                                      cl_event      /* event created */);

    cl_int (* RetainEvent)(cl_event /* event */);

    cl_int (* ReleaseEvent)(cl_event /* event */);

    cl_int (* SetUserEventStatus)(cl_event   /* event */,
                                  cl_int     /* execution_status */);

    cl_int (* SetEventCallback)(cl_event    /* event */,
                                cl_int      /* command_exec_callback_type */,
                                void (CL_CALLBACK * /* pfn_notify */)(cl_event, cl_int, void *),
                                void *      /* user_data */);

    cl_int (* GetEventProfilingInfo)(cl_event            /* event */,
                                     cl_profiling_info   /* param_name */,
                                     size_t              /* param_value_size */,
                                     void *              /* param_value */,
                                     size_t *            /* param_value_size_ret */);

    cl_int (* Flush)(cl_command_queue /* command_queue */);

    cl_int (* Finish)(cl_command_queue /* command_queue */);

    cl_int (* EnqueueReadBuffer)(cl_command_queue    /* command_queue */,
                                 cl_mem              /* buffer */,
                                 cl_bool             /* blocking_read */,
                                 size_t              /* offset */,
                                 size_t              /* size */,
                                 void *              /* ptr */,
                                 cl_uint             /* num_events_in_wait_list */,
                                 const cl_event *    /* event_wait_list */,
                                 cl_event *          /* event */);

    cl_int (* EnqueueReadBufferRect)(cl_command_queue    /* command_queue */,
                                     cl_mem              /* buffer */,
                                     cl_bool             /* blocking_read */,
                                     const size_t *      /* buffer_offset */,
                                     const size_t *      /* host_offset */,
                                     const size_t *      /* region */,
                                     size_t              /* buffer_row_pitch */,
                                     size_t              /* buffer_slice_pitch */,
                                     size_t              /* host_row_pitch */,
                                     size_t              /* host_slice_pitch */,
                                     void *              /* ptr */,
                                     cl_uint             /* num_events_in_wait_list */,
                                     const cl_event *    /* event_wait_list */,
                                     cl_event *          /* event */);

    cl_int (* EnqueueWriteBuffer)(cl_command_queue   /* command_queue */,
                                  cl_mem             /* buffer */,
                                  cl_bool            /* blocking_write */,
                                  size_t             /* offset */,
                                  size_t             /* size */,
                                  const void *       /* ptr */,
                                  cl_uint            /* num_events_in_wait_list */,
                                  const cl_event *   /* event_wait_list */,
                                  cl_event *         /* event */);

    cl_int (* EnqueueWriteBufferRect)(cl_command_queue    /* command_queue */,
                                      cl_mem              /* buffer */,
                                      cl_bool             /* blocking_write */,
                                      const size_t *      /* buffer_offset */,
                                      const size_t *      /* host_offset */,
                                      const size_t *      /* region */,
                                      size_t              /* buffer_row_pitch */,
                                      size_t              /* buffer_slice_pitch */,
                                      size_t              /* host_row_pitch */,
                                      size_t              /* host_slice_pitch */,
                                      const void *        /* ptr */,
                                      cl_uint             /* num_events_in_wait_list */,
                                      const cl_event *    /* event_wait_list */,
                                      cl_event *          /* event */);

    cl_int (* EnqueueFillBuffer)(cl_command_queue   /* command_queue */,
                                 cl_mem             /* buffer */,
                                 const void *       /* pattern */,
                                 size_t             /* pattern_size */,
                                 size_t             /* offset */,
                                 size_t             /* size */,
                                 cl_uint            /* num_events_in_wait_list */,
                                 const cl_event *   /* event_wait_list */,
                                 cl_event *         /* event */);

    cl_int (* EnqueueCopyBuffer)(cl_command_queue    /* command_queue */,
                                 cl_mem              /* src_buffer */,
                                 cl_mem              /* dst_buffer */,
                                 size_t              /* src_offset */,
                                 size_t              /* dst_offset */,
                                 size_t              /* size */,
                                 cl_uint             /* num_events_in_wait_list */,
                                 const cl_event *    /* event_wait_list */,
                                 cl_event *          /* event */);

    cl_int (* EnqueueCopyBufferRect)(cl_command_queue    /* command_queue */,
                                     cl_mem              /* src_buffer */,
                                     cl_mem              /* dst_buffer */,
                                     const size_t *      /* src_origin */,
                                     const size_t *      /* dst_origin */,
                                     const size_t *      /* region */,
                                     size_t              /* src_row_pitch */,
                                     size_t              /* src_slice_pitch */,
                                     size_t              /* dst_row_pitch */,
                                     size_t              /* dst_slice_pitch */,
                                     cl_uint             /* num_events_in_wait_list */,
                                     const cl_event *    /* event_wait_list */,
                                     cl_event *          /* event */);

    cl_int (* EnqueueReadImage)(cl_command_queue     /* command_queue */,
                                cl_mem               /* image */,
                                cl_bool              /* blocking_read */,
                                const size_t *       /* origin[3] */,
                                const size_t *       /* region[3] */,
                                size_t               /* row_pitch */,
                                size_t               /* slice_pitch */,
                                void *               /* ptr */,
                                cl_uint              /* num_events_in_wait_list */,
                                const cl_event *     /* event_wait_list */,
                                cl_event *           /* event */);

    cl_int (* EnqueueWriteImage)(cl_command_queue    /* command_queue */,
                                 cl_mem              /* image */,
                                 cl_bool             /* blocking_write */,
                                 const size_t *      /* origin[3] */,
                                 const size_t *      /* region[3] */,
                                 size_t              /* input_row_pitch */,
                                 size_t              /* input_slice_pitch */,
                                 const void *        /* ptr */,
                                 cl_uint             /* num_events_in_wait_list */,
                                 const cl_event *    /* event_wait_list */,
                                 cl_event *          /* event */);

    cl_int (* EnqueueFillImage)(cl_command_queue   /* command_queue */,
                                cl_mem             /* image */,
                                const void *       /* fill_color */,
                                const size_t *     /* origin[3] */,
                                const size_t *     /* region[3] */,
                                cl_uint            /* num_events_in_wait_list */,
                                const cl_event *   /* event_wait_list */,
                                cl_event *         /* event */);

    cl_int (* EnqueueCopyImage)(cl_command_queue     /* command_queue */,
                                cl_mem               /* src_image */,
                                cl_mem               /* dst_image */,
                                const size_t *       /* src_origin[3] */,
                                const size_t *       /* dst_origin[3] */,
                                const size_t *       /* region[3] */,
                                cl_uint              /* num_events_in_wait_list */,
                                const cl_event *     /* event_wait_list */,
                                cl_event *           /* event */);

    cl_int (* EnqueueCopyImageToBuffer)(cl_command_queue /* command_queue */,
                                        cl_mem           /* src_image */,
                                        cl_mem           /* dst_buffer */,
                                        const size_t *   /* src_origin[3] */,
                                        const size_t *   /* region[3] */,
                                        size_t           /* dst_offset */,
                                        cl_uint          /* num_events_in_wait_list */,
                                        const cl_event * /* event_wait_list */,
                                        cl_event *       /* event */);

    cl_int (* EnqueueCopyBufferToImage)(cl_command_queue /* command_queue */,
                                        cl_mem           /* src_buffer */,
                                        cl_mem           /* dst_image */,
                                        size_t           /* src_offset */,
                                        const size_t *   /* dst_origin[3] */,
                                        const size_t *   /* region[3] */,
                                        cl_uint          /* num_events_in_wait_list */,
                                        const cl_event * /* event_wait_list */,
                                        cl_event *       /* event */);

    void * (* EnqueueMapBuffer_Pre)(cl_command_queue /* command_queue */,
                                    cl_mem           /* buffer */,
                                    cl_bool          /* blocking_map */,
                                    cl_map_flags     /* map_flags */,
                                    size_t           /* offset */,
                                    size_t           /* size */,
                                    cl_uint          /* num_events_in_wait_list */,
                                    const cl_event * /* event_wait_list */,
                                    cl_event *       /* event */,
                                    cl_int *         /* errcode_ret */);

    void * (* EnqueueMapBuffer_Post)(cl_command_queue /* command_queue */,
                                     cl_mem           /* buffer */,
                                     cl_bool          /* blocking_map */,
                                     cl_map_flags     /* map_flags */,
                                     size_t           /* offset */,
                                     size_t           /* size */,
                                     cl_uint          /* num_events_in_wait_list */,
                                     const cl_event * /* event_wait_list */,
                                     cl_event *       /* event */,
                                     cl_int *         /* errcode_ret */,
                                     void *           /* mapped buffer pointer */);

    void * (* EnqueueMapImage_Pre)(cl_command_queue  /* command_queue */,
                                   cl_mem            /* image */,
                                   cl_bool           /* blocking_map */,
                                   cl_map_flags      /* map_flags */,
                                   const size_t *    /* origin[3] */,
                                   const size_t *    /* region[3] */,
                                   size_t *          /* image_row_pitch */,
                                   size_t *          /* image_slice_pitch */,
                                   cl_uint           /* num_events_in_wait_list */,
                                   const cl_event *  /* event_wait_list */,
                                   cl_event *        /* event */,
                                   cl_int *          /* errcode_ret */);

    void * (* EnqueueMapImage_Post)(cl_command_queue  /* command_queue */,
                                    cl_mem            /* image */,
                                    cl_bool           /* blocking_map */,
                                    cl_map_flags      /* map_flags */,
                                    const size_t *    /* origin[3] */,
                                    const size_t *    /* region[3] */,
                                    size_t *          /* image_row_pitch */,
                                    size_t *          /* image_slice_pitch */,
                                    cl_uint           /* num_events_in_wait_list */,
                                    const cl_event *  /* event_wait_list */,
                                    cl_event *        /* event */,
                                    cl_int *          /* errcode_ret */,
                                    void *            /* mapped image pointer*/);

    cl_int (* EnqueueUnmapMemObject)(cl_command_queue /* command_queue */,
                                     cl_mem           /* memobj */,
                                     void *           /* mapped_ptr */,
                                     cl_uint          /* num_events_in_wait_list */,
                                     const cl_event *  /* event_wait_list */,
                                     cl_event *        /* event */);

    cl_int (* EnqueueMigrateMemObjects)(cl_command_queue       /* command_queue */,
                                        cl_uint                /* num_mem_objects */,
                                        const cl_mem *         /* mem_objects */,
                                        cl_mem_migration_flags /* flags */,
                                        cl_uint                /* num_events_in_wait_list */,
                                        const cl_event *       /* event_wait_list */,
                                        cl_event *             /* event */);

    cl_int (* EnqueueNDRangeKernel)(cl_command_queue /* command_queue */,
                                    cl_kernel        /* kernel */,
                                    cl_uint          /* work_dim */,
                                    const size_t *   /* global_work_offset */,
                                    const size_t *   /* global_work_size */,
                                    const size_t *   /* local_work_size */,
                                    cl_uint          /* num_events_in_wait_list */,
                                    const cl_event * /* event_wait_list */,
                                    cl_event *       /* event */);

    cl_int (* EnqueueTask)(cl_command_queue  /* command_queue */,
                           cl_kernel         /* kernel */,
                           cl_uint           /* num_events_in_wait_list */,
                           const cl_event *  /* event_wait_list */,
                           cl_event *        /* event */);

    cl_int (* EnqueueNativeKernel)(cl_command_queue  /* command_queue */,
                                   void (CL_CALLBACK * /*user_func*/)(void *),
                                   void *            /* args */,
                                   size_t            /* cb_args */,
                                   cl_uint           /* num_mem_objects */,
                                   const cl_mem *    /* mem_list */,
                                   const void **     /* args_mem_loc */,
                                   cl_uint           /* num_events_in_wait_list */,
                                   const cl_event *  /* event_wait_list */,
                                   cl_event *        /* event */);

    cl_int (* EnqueueMarkerWithWaitList)(cl_command_queue /* command_queue */,
                                         cl_uint           /* num_events_in_wait_list */,
                                         const cl_event *  /* event_wait_list */,
                                         cl_event *        /* event */);

    cl_int (* EnqueueBarrierWithWaitList)(cl_command_queue /* command_queue */,
                                          cl_uint           /* num_events_in_wait_list */,
                                          const cl_event *  /* event_wait_list */,
                                          cl_event *        /* event */);

    /* Extension functions */
    void * (* GetExtensionFunctionAddressForPlatform_Pre) (cl_platform_id /* platform */,
                                                           const char *   /* func_name */);

    void * (* GetExtensionFunctionAddressForPlatform_Post) (cl_platform_id /* platform */,
                                                            const char *   /* func_name */,
                                                            void *         /* address pointer */);

    cl_mem (* CreateImage2D_Pre)(cl_context              /* context */,
                                 cl_mem_flags            /* flags */,
                                 const cl_image_format * /* image_format */,
                                 size_t                  /* image_width */,
                                 size_t                  /* image_height */,
                                 size_t                  /* image_row_pitch */,
                                 void *                  /* host_ptr */,
                                 cl_int *                /* errcode_ret */);

    cl_mem (* CreateImage2D_Post)(cl_context              /* context */,
                                  cl_mem_flags            /* flags */,
                                  const cl_image_format * /* image_format */,
                                  size_t                  /* image_width */,
                                  size_t                  /* image_height */,
                                  size_t                  /* image_row_pitch */,
                                  void *                  /* host_ptr */,
                                  cl_int *                /* errcode_ret */,
                                  cl_mem                  /* memobj created */);

    cl_mem (* CreateImage3D_Pre)(cl_context              /* context */,
                                 cl_mem_flags            /* flags */,
                                 const cl_image_format * /* image_format */,
                                 size_t                  /* image_width */,
                                 size_t                  /* image_height */,
                                 size_t                  /* image_depth */,
                                 size_t                  /* image_row_pitch */,
                                 size_t                  /* image_slice_pitch */,
                                 void *                  /* host_ptr */,
                                 cl_int *                /* errcode_ret */);

    cl_mem (* CreateImage3D_Post)(cl_context              /* context */,
                                  cl_mem_flags            /* flags */,
                                  const cl_image_format * /* image_format */,
                                  size_t                  /* image_width */,
                                  size_t                  /* image_height */,
                                  size_t                  /* image_depth */,
                                  size_t                  /* image_row_pitch */,
                                  size_t                  /* image_slice_pitch */,
                                  void *                  /* host_ptr */,
                                  cl_int *                /* errcode_ret */,
                                  cl_mem                  /* memobj created */);

    cl_int (* EnqueueMarker)(cl_command_queue    /* command_queue */,
                             cl_event *          /* event */);

    cl_int (* EnqueueWaitForEvents)(cl_command_queue /* command_queue */,
                                    cl_uint          /* num_events */,
                                    const cl_event * /* event_list */);

    cl_int (* EnqueueBarrier)(cl_command_queue /* command_queue */);

    cl_int (* UnloadCompiler)(void);

    void * (* getExtensionFunctionAddress_Pre)(const char * /* func_name */);

    void * (* getExtensionFunctionAddress_Post)(const char * /* func_name */,
                                                void *       /* func address pointer */);

    cl_int (* IcdGetPlatformIDsKHR)(cl_uint          /* num_entries */,
                                    cl_platform_id * /* platforms */,
                                    cl_uint *        /* num_platforms */);

    cl_mem (* CreateFromGLBuffer_Pre)(cl_context     /* context */,
                                      cl_mem_flags   /* flags */,
                                      cl_GLuint      /* bufobj */,
                                      int *          /* errcode_ret */);

    cl_mem (* CreateFromGLBuffer_Post)(cl_context     /* context */,
                                       cl_mem_flags   /* flags */,
                                       cl_GLuint      /* bufobj */,
                                       int *          /* errcode_ret */,
                                       cl_mem         /* memobj created */);

    cl_mem (* CreateFromGLTexture_Pre)(cl_context      /* context */,
                                       cl_mem_flags    /* flags */,
                                       cl_GLenum       /* target */,
                                       cl_GLint        /* miplevel */,
                                       cl_GLuint       /* texture */,
                                       cl_int *        /* errcode_ret */);

    cl_mem (* CreateFromGLTexture_Post)(cl_context      /* context */,
                                        cl_mem_flags    /* flags */,
                                        cl_GLenum       /* target */,
                                        cl_GLint        /* miplevel */,
                                        cl_GLuint       /* texture */,
                                        cl_int *        /* errcode_ret */,
                                        cl_mem          /* memobj created */);

    cl_mem (* CreateFromGLRenderbuffer_Pre)(cl_context   /* context */,
                                            cl_mem_flags /* flags */,
                                            cl_GLuint    /* renderbuffer */,
                                            cl_int *     /* errcode_ret */);

    cl_mem (* CreateFromGLRenderbuffer_Post)(cl_context   /* context */,
                                             cl_mem_flags /* flags */,
                                             cl_GLuint    /* renderbuffer */,
                                             cl_int *     /* errcode_ret */,
                                             cl_mem       /* memobj created */);

    cl_int (* GetGLObjectInfo)(cl_mem                /* memobj */,
                               cl_gl_object_type *   /* gl_object_type */,
                               cl_GLuint *           /* gl_object_name */);

    cl_int (* GetGLTextureInfo)(cl_mem               /* memobj */,
                                cl_gl_texture_info   /* param_name */,
                                size_t               /* param_value_size */,
                                void *               /* param_value */,
                                size_t *             /* param_value_size_ret */);

    cl_int (* EnqueueAcquireGLObjects)(cl_command_queue      /* command_queue */,
                                       cl_uint               /* num_objects */,
                                       const cl_mem *        /* mem_objects */,
                                       cl_uint               /* num_events_in_wait_list */,
                                       const cl_event *      /* event_wait_list */,
                                       cl_event *            /* event */);

    cl_int (* EnqueueReleaseGLObjects)(cl_command_queue      /* command_queue */,
                                       cl_uint               /* num_objects */,
                                       const cl_mem *        /* mem_objects */,
                                       cl_uint               /* num_events_in_wait_list */,
                                       const cl_event *      /* event_wait_list */,
                                       cl_event *            /* event */);

    cl_mem (* CreateFromGLTexture2D_Pre)(cl_context      /* context */,
                                         cl_mem_flags    /* flags */,
                                         cl_GLenum       /* target */,
                                         cl_GLint        /* miplevel */,
                                         cl_GLuint       /* texture */,
                                         cl_int *        /* errcode_ret */);

    cl_mem (* CreateFromGLTexture2D_Post)(cl_context      /* context */,
                                          cl_mem_flags    /* flags */,
                                          cl_GLenum       /* target */,
                                          cl_GLint        /* miplevel */,
                                          cl_GLuint       /* texture */,
                                          cl_int *        /* errcode_ret */,
                                          cl_mem          /* memobj created */);

    cl_mem (* CreateFromGLTexture3D_Pre)(cl_context      /* context */,
                                         cl_mem_flags    /* flags */,
                                         cl_GLenum       /* target */,
                                         cl_GLint        /* miplevel */,
                                         cl_GLuint       /* texture */,
                                         cl_int *        /* errcode_ret */);

    cl_mem (* CreateFromGLTexture3D_Post)(cl_context      /* context */,
                                          cl_mem_flags    /* flags */,
                                          cl_GLenum       /* target */,
                                          cl_GLint        /* miplevel */,
                                          cl_GLuint       /* texture */,
                                          cl_int *        /* errcode_ret */,
                                          cl_mem          /* memobj created */);

    cl_int (* GetGLContextInfoKHR)(const cl_context_properties * /* properties */,
                                   cl_gl_context_info            /* param_name */,
                                   size_t                        /* param_value_size */,
                                   void *                        /* param_value */,
                                   size_t *                      /* param_value_size_ret */);

    cl_event (* CreateEventFromGLsyncKHR_Pre)(cl_context           /* context */,
                                              cl_GLsync            /* cl_GLsync */,
                                              cl_int *             /* errcode_ret */);

    cl_event (* CreateEventFromGLsyncKHR_Post)(cl_context           /* context */,
                                               cl_GLsync            /* cl_GLsync */,
                                               cl_int *             /* errcode_ret */,
                                               cl_event             /* eventobj created */);
}clsTracerDispatchTableStruct;

extern clsTracerDispatchTableStruct vclTracerDispatchTable;
extern clsTracerDispatchTableStruct vclLogFunctionTable;

#define VCL_TRACE_API(func) \
    if (vclTracerDispatchTable.func)(*vclTracerDispatchTable.func)

#define VCL_TRACE_API_PRE(func) \
    if (vclTracerDispatchTable.func##_pre)(*vclTracerDispatchTable.func##_pre)

#define VCL_TRACE_API_POST(func) \
    if (vclTracerDispatchTable.func##_post)(*vclTracerDispatchTable.func##_post)

#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_log_h_ */