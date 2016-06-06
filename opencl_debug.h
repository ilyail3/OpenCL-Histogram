//
// Created by ilya on 6/6/16.
//

#ifndef OPEN_CL_HISTOGRAM_OPENCL_DEBUG_H
#define OPEN_CL_HISTOGRAM_OPENCL_DEBUG_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

void debug_log(const char* error_msg, cl_program program, cl_device_id device_id, cl_build_status status, cl_int ret);
void check_error(const char* error_msg, cl_int ret);

#endif //OPEN_CL_HISTOGRAM_OPENCL_DEBUG_H
