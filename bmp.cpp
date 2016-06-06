//
// Created by ilya on 6/3/16.
//
#include <cstdio>
#include <cstdlib>
#include "bmp.h"

unsigned char *load_bitmap_file(const char *filename, BITMAPINFOHEADER *bitmap_info_header) {
    FILE *filePtr; //our file pointer
    BITMAPFILEHEADER bitmap_file_header; //our bitmap file header
    unsigned char *bitmap_image;  //store image data
    int imageIdx = 0;  //image index counter
    unsigned char tempRGB;  //our swap variable

    //open filename in read binary mode
    filePtr = fopen(filename, "rb");
    if (filePtr == NULL)
        return NULL;

    //read the bitmap file header
    fread(&bitmap_file_header, sizeof(BITMAPFILEHEADER), 1, filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmap_file_header.type != 0x4D42) {
        fclose(filePtr);
        return NULL;
    }

    //read the bitmap info header
    fread(bitmap_info_header, sizeof(BITMAPINFOHEADER), 1,
          filePtr); // small edit. forgot to add the closing bracket at sizeof

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmap_file_header.offset, SEEK_SET);

    //allocate enough memory for the bitmap image data
    bitmap_image = (unsigned char *) malloc(bitmap_info_header->image_size);

    //verify memory allocation
    if (!bitmap_image) {
        free(bitmap_image);
        fclose(filePtr);
        return NULL;
    }

    //read in the bitmap image data
    int read_bytes = fread(bitmap_image, 1, bitmap_info_header->image_size, filePtr);

    //make sure bitmap image data was read
    if (bitmap_image == NULL) {
        fclose(filePtr);
        return NULL;
    }

    //swap the r and b values to get RGB (bitmap is BGR)
    for (imageIdx = 0; imageIdx < bitmap_info_header->size; imageIdx += 3) {
        tempRGB = bitmap_image[imageIdx];
        bitmap_image[imageIdx] = bitmap_image[imageIdx + 2];
        bitmap_image[imageIdx + 2] = tempRGB;
    }

    //close file and return bitmap iamge data
    fclose(filePtr);
    return bitmap_image;
}
