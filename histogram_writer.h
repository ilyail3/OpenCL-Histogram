//
// Created by ilya on 6/3/16.
//

#ifndef OPEN_CL_HISTOGRAM_HISTOGRAM_WRITER_H
#define OPEN_CL_HISTOGRAM_HISTOGRAM_WRITER_H

void write_histogram(const char* filename, int* histogram, int buckets);
void write_histogram2(const char *filename, int *histogram, int buckets);

#endif //OPEN_CL_HISTOGRAM_HISTOGRAM_WRITER_H
