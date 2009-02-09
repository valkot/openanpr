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
