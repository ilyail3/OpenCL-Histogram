//
// Created by ilya on 6/3/16.
//

#ifndef OPEN_CL_HISTOGRAM_BMP_H
#define OPEN_CL_HISTOGRAM_BMP_H



#pragma pack(push, 1)

#include <cstdint>

typedef struct tagBITMAPFILEHEADER
{
    uint16_t type;  // specifies the file type
    uint32_t size;  // specifies the size in bytes of the bitmap file
    uint32_t reserved;  // reserved; must be 0
    uint32_t offset;  // Offset to image data, bytes
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    uint32_t size;  //specifies the number of bytes required by the struct
    uint32_t width;  //specifies width in pixels
    uint32_t height;  //species height in pixels
    uint16_t planes; //specifies the number of color planes, must be 1
    uint16_t bit_count; //specifies the number of bit per pixel
    uint32_t compression;//spcifies the type of compression
    uint32_t image_size;  //size of image in bytes
    uint32_t resolution_x;  //number of pixels per meter in x axis
    uint32_t resolution_y;  //number of pixels per meter in y axis
    uint32_t number_colors;  //number of colors used by th ebitmap
    uint32_t important_colors;  //number of colors that are important
} BITMAPINFOHEADER;

#pragma pack(pop)

unsigned char* load_bitmap_file(const char *filename, BITMAPINFOHEADER *bitmap_info_header);

#endif //OPEN_CL_HISTOGRAM_BMP_H
