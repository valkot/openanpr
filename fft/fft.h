#ifndef FFT_h
#define FFT_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#ifndef PI
    #define PI 3.14159265358979323846264338327950288419716939937510
#endif

class FFT
{
    public:

        static int fft_1D(double* data, int data_length, bool forward, int min_frequency, int max_frequency);
        static int Test(int frequency);


    private:

        static void fft_1D(double* data, int no_of_elements, bool forward);


};

#endif
