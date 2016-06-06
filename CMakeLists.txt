cmake_minimum_required(VERSION 3.5)
project(open_cl_histogram)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# CFLAGS  = -g -Wall -I /usr/local/cuda-7.5/targets/x86_64-linux/include -std=c99
# LINKFLAGS = -lOpenCL
include_directories(/usr/local/cuda-7.5/targets/x86_64-linux/include)
link_libraries (OpenCL)

set(SOURCE_FILES main.cpp bmp.h bmp.cpp histogram_writer.h histogram_writer.cpp opencl_debug.cpp opencl_debug.h)
add_executable(open_cl_histogram ${SOURCE_FILES})

file(COPY analyze.cl DESTINATION "${CMAKE_CURRENT_BINARY_DIR}" NO_SOURCE_PERMISSIONS)