/*
    polygons
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

#ifndef polygon2D_h
#define polygon2D_h

#ifndef ABS
    #define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#ifndef PI
    #define PI 3.14159265358979323846264338327950288419716939937510
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <vector>
#include <float.h>
#include "geometry.h"
#include "drawing.h"

class polygon2D
{
    public:
        std::string name;
        int type;
        bool occupied;
        std::vector<float> x_points;
        std::vector<float> y_points;

        static polygon2D *CreateCircle(float centre_x, float centre_y, float radius, int circumference_steps);
        polygon2D *Copy();
        void Scale(float factor, polygon2D *scaled);
        polygon2D *Scale(int original_image_width, int original_image_height, int new_image_width, int new_image_height);
        void rotate(float rotation, float centre_x, float centre_y);
        void show(unsigned char* img, int image_width, int image_height, int r, int g, int b, int lineWidth, int x_offset, int y_offset);
        void show(unsigned char* img, int image_width, int image_height, int r, int g, int b, int lineWidth);
        void getCentreOfGravity(float& centre_x, float& centre_y);
        float getPerimeterLength();
        float getShortestSide();
        float* getOrientations();
        float* getGradients();
        float getLongestSide();
        float getSideLength(int index);
        void getSidePositions(int index, float& tx, float& ty, float& bx, float& by);
        float GetInteriorAngle(int vertex);
        float GetMaxDifferenceFromSquare();
        float getLongestSideOrientation();
        void GetSquareCentre(float& centre_x, float& centre_y);
        float getSquareness();
        float GetSquareOrientation();
        float GetSquareHorizontal();
        float GetSquareVertical();
        void BoundingBox(float& tx, float& ty, float& bx, float& by);
        float left();
        float right();
        float top();
        float bottom();
        void Clear();
        void Add(float x, float y);
        void Add(int index, float x, float y);
        void Remove();
        void Mirror(int image_width, int image_height);
        void Flip(int image_width, int image_height);
        bool isInside(float x, float y);
        bool overlaps(polygon2D *other);
        bool overlaps(polygon2D *other, int image_width, int image_height);
        float Compare(polygon2D *other);
        polygon2D *SnapToGrid(float grid_spacing);
        float ApproximateArea(float grid_spacing);

        polygon2D() {};
        //~polygon2D() { x_points.clear(); y_points.clear(); };

};

#endif
