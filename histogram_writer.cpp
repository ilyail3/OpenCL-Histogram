//
// Created by ilya on 6/3/16.
//

// Basic picture should be 256 x (256+20)

#include <cstdio>
#include <cstring>
#include <cmath>
#include "bmp.h"

#define COLOR_BYTES 3
#define WIDTH 360
#define BASE_HEIGHT 256
#define INDEX_HEIGHT 20
#define HEIGHT 276
#define ROW_BYTES COLOR_BYTES*WIDTH
#define IMAGE_BYTES WIDTH*HEIGHT*COLOR_BYTES

#define WIDTH2 400
#define HEIGHT2 400
#define ROW_BYTES2 COLOR_BYTES*WIDTH2
#define IMAGE_BYTES2 WIDTH2*HEIGHT2*COLOR_BYTES

typedef struct COLOR {
    unsigned char R, G, B;
} COLOR;

unsigned char *get_address(unsigned char *data, int x, int y) {
    return data + y * ROW_BYTES + x * COLOR_BYTES;
}

COLOR hsv2rgb(int hue, double sat, unsigned char value) {
    double hh, p, q, t, ff;
    long i;
    COLOR out;

    if (sat <= 0.0) {       // < is bogus, just shuts up warnings
        out.R = value;
        out.G = value;
        out.B = value;
        return out;
    }

    hh = hue;

    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;

    i = (long) hh;

    ff = hh - i;

    p = value * (1.0 - sat);
    q = value * (1.0 - (sat * ff));
    t = value * (1.0 - (sat * (1.0 - ff)));

    switch (i) {
        case 0:
            out.R = value;
            out.G = t;
            out.B = p;
            break;
        case 1:
            out.R = q;
            out.G = value;
            out.B = p;
            break;
        case 2:
            out.R = p;
            out.G = value;
            out.B = t;
            break;
        case 3:
            out.R = p;
            out.G = q;
            out.B = value;
            break;
        case 4:
            out.R = t;
            out.G = p;
            out.B = value;
            break;
        case 5:
        default:
            out.R = value;
            out.G = p;
            out.B = q;
            break;
    }
    return out;
}

void write_histogram(const char *filename, int *histogram, int buckets) {
    const int bucket_pixel_size = WIDTH / buckets;
    const int bucket_size = 360 / buckets;

    BITMAPFILEHEADER file_headers = {0x4D42, IMAGE_BYTES + 14 + 40, 0, 54};
    BITMAPINFOHEADER info_headers = {40, WIDTH, HEIGHT, 1, COLOR_BYTES * 8, 0, IMAGE_BYTES, 2835, 2835, 0, 0};

    FILE *output = fopen(filename, "wb");
    fwrite(&file_headers, sizeof(BITMAPFILEHEADER), 1, output);
    fwrite(&info_headers, sizeof(BITMAPINFOHEADER), 1, output);

    unsigned char data[IMAGE_BYTES];
    memset(data, 255, IMAGE_BYTES);

    int max_value = 0;

    for (int bucket = 0; bucket < buckets; bucket++) {
        if (histogram[bucket] > max_value)
            max_value = histogram[bucket];
    }

    COLOR black{0, 0, 0};

    for (int bucket = 0; bucket < buckets; bucket++) {
        int pixel_height = (int) round(((double) histogram[bucket] / (double) max_value) * (double) BASE_HEIGHT);

        for (int x = bucket * bucket_pixel_size; x < (bucket + 1) * bucket_pixel_size; x++) {
            for (int y = INDEX_HEIGHT; y < pixel_height + INDEX_HEIGHT; y++) {
                memcpy(get_address(data, x, y), &black, sizeof(COLOR));
            }
        }

        COLOR hue_color = hsv2rgb(bucket * bucket_size, 1.0, 255);

        for (int x = bucket * bucket_pixel_size; x < (bucket + 1) * bucket_pixel_size; x++) {
            for (int y = 0; y < INDEX_HEIGHT; y++) {
                memcpy(get_address(data, x, y), &hue_color, sizeof(COLOR));
            }
        }

    }

    fwrite(&data, 1, IMAGE_BYTES, output);

    fclose(output);
}

unsigned char *get_address2(unsigned char *data, int x, int y) {
    return data + y * ROW_BYTES2 + x * COLOR_BYTES;
}

void write_histogram2(const char *filename, int *histogram, int buckets) {

    BITMAPFILEHEADER file_headers = {0x4D42, IMAGE_BYTES2 + 14 + 40, 0, 54};
    BITMAPINFOHEADER info_headers = {40, WIDTH2, HEIGHT2, 1, COLOR_BYTES * 8, 0, IMAGE_BYTES2, 2835, 2835, 0, 0};

    const int bucket_size = 360 / buckets;

    double max_dist = fmin((double) WIDTH2 - 10, (double) HEIGHT2 - 10);

    double max_length_sqr = pow(fmin((double) WIDTH2, (double) HEIGHT2) / 2.0, 2.0);
    double min_length_sqr = pow(max_dist / 2.0, 2.0);

    FILE *output = fopen(filename, "wb");
    fwrite(&file_headers, sizeof(BITMAPFILEHEADER), 1, output);
    fwrite(&info_headers, sizeof(BITMAPINFOHEADER), 1, output);

    unsigned char data[IMAGE_BYTES2];
    memset(data, 255, IMAGE_BYTES2);

    int max_value = 0;

    for (int bucket = 0; bucket < buckets; bucket++) {
        if (histogram[bucket] > max_value)
            max_value = histogram[bucket];
    }

    COLOR *color;

    for (int y = 0; y < HEIGHT2; y++) {
        for (int x = 0; x < WIDTH2; x++) {
            double distance_sqr =
                    pow(fabs((double) y - (double) HEIGHT2 / 2.0), 2.0) +
                    pow(fabs((double) x - (double) WIDTH2 / 2.0), 2.0);

            //printf("distance x:%d y:%d distance:%0.2f max distance:%0.2f\n", x, y, distance_sqr, max_length_sqr);

            double deg = atan2(y - HEIGHT2/2, x - WIDTH2/2) * (180.0 / M_PI);
            if (deg < 0) deg = deg + 360;

            if (max_length_sqr > distance_sqr && min_length_sqr < distance_sqr) {

                COLOR c = hsv2rgb(deg, 1.0, 255);

                //printf("x:%d, y:%d, deg:%0.2f\n", x, y, deg);

                color = (COLOR *) get_address2(data, x, y);
                color->R = c.R;
                color->G = c.G;
                color->B = c.B;
                //printf("fill black\n");
            } else if(min_length_sqr >= distance_sqr){
                int bucket_a = (deg / bucket_size) * bucket_size;
                double dist_a = (deg - (double)bucket_a) / (double)bucket_size;

                int bucket_b = ((deg / bucket_size) + 1) * bucket_size;
                double dist_b = ((double)bucket_b - deg) / (double)bucket_size;

                if(bucket_b == buckets) bucket_b = 0;

                //printf("bucket a:%d, b:%d, dist a:%0.2f, b:%0.2f\n", bucket_a, bucket_b, dist_a, dist_b);
                double dist = 1 - (sqrt(pow(y - HEIGHT2/2, 2.0) + pow(x - WIDTH2/2, 2.0)) / (max_dist / 2));

                double rel_value_a = (double)histogram[bucket_a] / (double)max_value;
                double rel_value_b = (double)histogram[bucket_b] / (double)max_value;

                if((rel_value_a > dist && dist_a > 0.5) || (rel_value_b > dist && dist_b > 0.5)) {
                    COLOR c = hsv2rgb(deg, 1.0, 255);

                    //printf("x:%d, y:%d, deg:%0.2f\n", x, y, deg);

                    color = (COLOR *) get_address2(data, x, y);
                    color->R = c.R;
                    color->G = c.G;
                    color->B = c.B;
                }
            }


        }
    }


    fwrite(&data, 1, IMAGE_BYTES2, output);

    fclose(output);
}