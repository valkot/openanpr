/*
    detect circles
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

#ifndef circleDetector_h
#define circleDetector_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "../utils/drawing.h"
#include "../utils/geometry.h"

#ifndef ABS
    #define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#ifndef MAX_RAND
    #define MAX_RAND 999999
#endif

class circleDetector
{
    public:
        float centre_x, centre_y;
        float radius;
        std::vector<float> intersections;
        std::vector<float> radii;
        std::vector<float> possible_centres;
        int maximum_possible_centre_separation;
        int minimum_centre_response;
        int image_width, image_height;
        int dilation;

        void ShowPossibleCentres(unsigned char* img, int img_width, int img_height, bool clear_image);
        void ShowCircle(unsigned char* img, int img_width, int img_height);
        bool Find(std::vector<float> &connected_points, int image_width, int image_height);

        circleDetector();
        ~circleDetector();
};

#endif
