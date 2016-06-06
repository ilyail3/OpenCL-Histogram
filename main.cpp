#include "bmp.h"
#include "histogram_writer.h"

#include <cstdio>
#include <libgen.h>
#include <cmath>

#define HISTOGRAM_BUCKETS 90
#define MAX_SOURCE_SIZE (0x100000)

#define GPU
#ifdef CPU
unsigned short get_hue(unsigned char red, unsigned char green, unsigned char blue) {

    float min = (float)fmin(fmin(red, green), blue);
    float max = (float)fmax(fmax(red, green), blue);

    float hue = 0;

    if (max == red) {
        hue = (float)(green - blue) / (max - min);

    } else if (max == green) {
        hue = 2 + (float)(blue - red) / (max - min);

    } else {
        hue = 4 + (float)(red - green) / (max - min);
    }

    hue = hue * 60;
    if (hue < 0) hue = hue + 360;

    return (unsigned short)round(hue);
}

int main(int argc, char *argv[]) {
    BITMAPINFOHEADER bitmap_info_header;
    unsigned char *data = load_bitmap_file("/home/ilya/Desktop/huawei-980x864.bmp", &bitmap_info_header);

    const int color_bytes = bitmap_info_header.bit_count / 8;
    int row_size = color_bytes * bitmap_info_header.width;

    if (row_size % 4 != 0)
        row_size += 4 - (row_size % 4);

    const int bucket_size = 360 / HISTOGRAM_BUCKETS;
    int histogram[HISTOGRAM_BUCKETS] = {0};

    printf("row_size: %d(%d) %d\n", row_size, color_bytes, bitmap_info_header.image_size);

    for (int y = 0; y < bitmap_info_header.height; y++) {
        for (int x = 0; x < bitmap_info_header.width; x++) {
            //printf("offset:%d\n", y * row_size + x * color_bytes);

            unsigned char *address = data + y * row_size + x * color_bytes;

            unsigned char *R = address;
            unsigned char *G = R + 1;
            unsigned char *B = G + 1;

            //printf("(%d,%d) R:%d, G:%d, B:%d H:%d\n", x, y, *R, *G, *B, get_hue(*R, *G, *B));

            histogram[get_hue(*R, *G, *B) / bucket_size]++;
        }
    }

    /*for(int bucket = 0 ; bucket < HISTOGRAM_BUCKETS ; bucket++){
        printf("Hue %d: %d\n", bucket * bucket_size, histogram[bucket]);
    }*/
    write_histogram("/home/ilya/Desktop/histogram.bmp", histogram, HISTOGRAM_BUCKETS);
}
#endif
#ifdef GPU

#include "opencl_debug.h"

// This is just used as a shortcut because I don't want to repeat the same check_error
// code for each argument
struct kernel_args {
    size_t size;
    void *ptr;
};

int main(int argc, char *argv[]) {
    // argv[0] is the executable file
    char* exe_dir = dirname(argv[0]);

    BITMAPINFOHEADER bitmap_info_header;
    unsigned char *data = load_bitmap_file(argv[1], &bitmap_info_header);

    const int color_bytes = bitmap_info_header.bit_count / 8;
    int row_size = color_bytes * bitmap_info_header.width;

    // This is all the OpenCL objects I will need
    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_mem result_mem, data_bytes = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    cl_build_status status;

    char fileName[250];
    sprintf(fileName, "%s/%s", exe_dir, "analyze.cl");

    char *source_str;
    size_t source_size;

    /* Load the source code containing the kernel */
    FILE *fp = fopen(fileName, "r");
    if (!fp) {
        fprintf(stderr, "Failed to load code.\n");
        exit(1);
    }

    source_str = (char *) malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    /* Get platform and device info */
    check_error("Failed to get platform id", clGetPlatformIDs(1, &platform_id, &ret_num_platforms));
    check_error("Failed to get device id",
                clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices));

    /* Create OpenCL context */
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    check_error("Failed to create context", ret);

    /* Create command queue */
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    check_error("Failed to create command queue", ret);

    size_t result_size = HISTOGRAM_BUCKETS * bitmap_info_header.height * 4;

    int result[HISTOGRAM_BUCKETS * bitmap_info_header.height];

    for (int i = 0; i < HISTOGRAM_BUCKETS * bitmap_info_header.height; i++) {
        result[i] = 0;
    }

    /* Create memory buffer */
    // CL_MEM_USE_HOST_PTR means copy data from result, this is just an array of zeros, but I need
    // zeros as an initial state for each value in the counters
    result_mem = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, result_size, result, &ret);
    check_error("Failed to create result buffer", ret);

    // Copy bitmap data to gpu
    data_bytes = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, bitmap_info_header.image_size, data, &ret);
    check_error("Failed to create bitmap buffer", ret);

    /* Create Kernel Program from the source */
    program = clCreateProgramWithSource(context, 1, (const char **) &source_str, (const size_t *) &source_size, &ret);
    debug_log("Failed to build program", program, device_id, status, ret);

    /* Build Kernel Program */
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    debug_log("Failed to build program", program, device_id, status, ret);

    /* Create OpenCL Kernel */
    kernel = clCreateKernel(program, "analyze", &ret);
    check_error("Kernel creation failed", ret);

    const int buckets = HISTOGRAM_BUCKETS;

    struct kernel_args args[] = {
            {sizeof(cl_mem), (void *) &data_bytes},
            {sizeof(cl_mem), (void *) &result_mem},
            {sizeof(int),    (void *) &bitmap_info_header.width},
            {sizeof(int),    (void *) &bitmap_info_header.height},
            {sizeof(int),    (void *) &row_size},
            {sizeof(int),    (void *) &color_bytes},
            {sizeof(int),    (void *) &buckets},
            {0, NULL}
    };

    cl_uint arg = 0;
    char error_msg[30];
    for (arg = 0; args[arg].ptr != NULL; arg++) {
        sprintf(error_msg, "Set Kernel arg failed, ARG%d", arg);

        check_error(
                error_msg,
                clSetKernelArg(kernel, arg, args[arg].size, args[arg].ptr)
        );
    }

    const size_t local_wg_size = 32;
    const size_t m = bitmap_info_header.height;
    size_t global_wg_size = (m / local_wg_size) * local_wg_size;
    // If the number of rows isn't dividable by local_wg_size, add another one
    // the opencl code will do bonds check
    if(m % local_wg_size != 0)
        global_wg_size += local_wg_size;

    size_t global[1] = {global_wg_size};
    size_t local[1] = {local_wg_size};
    printf("wg size, global:%ld, local:%ld\n", global[0], local[0]);

    /* Execute OpenCL Kernel */
    ret = clEnqueueNDRangeKernel(
            command_queue,
            kernel,
            1,
            NULL,
            global,
            local,
            0,
            NULL,
            NULL
    );
    check_error("Enqueue task failed", ret);

    int total_result[HISTOGRAM_BUCKETS];

    for (int i = 0; i < HISTOGRAM_BUCKETS; i++) {
        total_result[i] = 0;
    }

    /* Copy results from the memory buffer */
    check_error(
            "Enqueue read buffer",
            clEnqueueReadBuffer(command_queue, result_mem, CL_TRUE, 0, result_size, result, 0, NULL, NULL)
    );

    for (int y = 0; y < bitmap_info_header.height; y++) {
        for (int bucket = 0; bucket < HISTOGRAM_BUCKETS; bucket++) {
            total_result[bucket] += result[y * HISTOGRAM_BUCKETS + bucket];
        }
    }

    write_histogram(argv[2], total_result, HISTOGRAM_BUCKETS);

    check_error("release kernel failed", clReleaseKernel(kernel));
    check_error("release program failed", clReleaseProgram(program));
    check_error("release memory failed", clReleaseMemObject(result_mem));
    check_error("release memory failed", clReleaseMemObject(data_bytes));
    check_error("release command queue", clReleaseCommandQueue(command_queue));
    check_error("release context", clReleaseContext(context));

}
#endif
