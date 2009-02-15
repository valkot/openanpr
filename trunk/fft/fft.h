/*
    fast fourier transform
    Copyright (C) 2009 Bob Mottram
    fuzzgun@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
