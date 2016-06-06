float get_hue(float red, float green, float blue) {

    float min = fmin(fmin(red, green), blue);
    float max = fmax(fmax(red, green), blue);

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

    return hue;
}

__kernel void analyze(__global unsigned char* data, __global int* results, int width, int height, int row_size, int color_bytes, int buckets){
    float bucket_size = 360.0 / buckets;
    int y = get_global_id(0);

    if(y <= height){
        int offset = y * buckets;

        for (int x = 0; x < width; x++) {
            int indx = convert_int(get_hue(
                                   *(data + y * row_size + x * color_bytes),
                                   *(data + y * row_size + x * color_bytes + 1),
                                   *(data + y * row_size + x * color_bytes + 2)
                               )/bucket_size);

            results[offset + indx]++;
        }
    }

}
