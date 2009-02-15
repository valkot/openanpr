/*
    computational geometry
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
