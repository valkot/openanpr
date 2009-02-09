#ifndef geometry_h
#define geometry_h

#define GEOM_INVALID 9999

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef ABS
    #define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

class geometry
{
    public:
        static float threePointAngle(float x0, float y0, float x1, float y1, float x2, float y2);
        static float circleDistanceFromLine(float x0, float y0, float x1, float y1, float circle_x, float circle_y, float circle_radius);
        static float pointDistanceFromLine(float x0, float y0, float x1, float y1, float point_x, float point_y, float& line_intersection_x, float& line_intersection_y);
        static float pointDistanceFromLine(float x0, float y0, float x1, float y1, float point_x, float point_y);        
        static bool intersection(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float& xi, float& yi);
};

#endif
