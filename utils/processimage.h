#ifndef processimage_h
#define processimage_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

class processimage
{
    public:
    	static void yellowFilter(unsigned char* img, int img_width, int img_height, unsigned char* filtered);
        static void colourImage(unsigned char* img_mono, int img_width, int img_height, unsigned char* output);
        static void monoImage(unsigned char* img_colour, int img_width, int img_height, int conversion_type, unsigned char* mono_image);
        static void downSample(unsigned char* img, int img_width, int img_height, int bytes_per_pixel, int new_width, int new_height, unsigned char* result);
        static void downSample(unsigned char* img, int img_width, int img_height, int bytes_per_pixel, int new_width, int new_height, int new_bytes_per_pixel, unsigned char* result);
        static void downSample(unsigned char* img, int img_width, int img_height, int bytes_per_pixel, unsigned char *result);
        static void downSample(unsigned char* img, int img_width, int img_height, int bytes_per_pixel, int factor, int* buffer0, int* buffer1, unsigned char *result);
        static void Mirror(unsigned char* bmp, int wdth, int hght, int bytes_per_pixel, unsigned char* result);
        static void Flip(unsigned char* bmp, int wdth, int hght, int bytes_per_pixel, unsigned char* result);
        static void ErodeDilate(unsigned char* bmp_mono, int width, int height, int radius, unsigned char* result_erode, unsigned char* result_dilate);
        static void Opening(unsigned char* bmp, int width, int height, unsigned char* buffer, int radius, unsigned char* result);
        static void Closing(unsigned char* bmp, int width, int height, unsigned char* buffer, int radius, unsigned char* result);
        static void Dilate(unsigned char* bmp, int width, int height, unsigned char* buffer, int radius, unsigned char* result);
        static void Erode(unsigned char* bmp, int width, int height, unsigned char* buffer, int radius, unsigned char* result);
        static void createSubImage(unsigned char* img, int img_width, int img_height, int bytes_per_pixel, int tx, int ty, int bx, int by, unsigned char* result);
        static void cropImage(unsigned char* img, int img_width, int img_height, int bytes_per_pixel, int tx, int ty, int bx, int by, unsigned char* result);
        static bool IsBlank(unsigned char* img, int img_width, int img_height, int bytes_per_pixel, int step_size);

    private:

        static void downSampleSum(int* img, int img_width, int img_height, int bytes_per_pixel, int* result);
};

#endif
