#ifndef drawing_h
#define drawing_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>

#ifndef ABS
    #define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#ifndef PI
    #define PI 3.14159265358979323846264338327950288419716939937510
#endif

class drawing
{
    public:
        static void drawBox(unsigned char* img, int img_width, int img_height, int x, int y, int radius, int r, int g, int b, int line_width);
        static void drawBox(unsigned char* img, int img_width, int img_height, int x, int y, int box_width, int box_height, float rotation, int r, int g, int b, int line_width);
        static void drawCross(unsigned char* img, int img_width, int img_height, int x, int y, int radius, int r, int g, int b, int line_width);
        static void drawCircle(unsigned char* img, int img_width, int img_height, int x, int y, int radius, int r, int g, int b, int line_width);
        static void drawCircle(unsigned char* img, int img_width, int img_height, float x, float y, float radius, int r, int g, int b, int line_width);
        static void drawSpot(unsigned char* img, int img_width, int img_height, int x, int y, int radius, int r, int g, int b);
        static void drawGrid(unsigned char* img, int img_width, int img_height, int centre_x, int centre_y, float rotation, float size_width, float size_height, int columns, int rows, int r, int g, int b, int linewidth);
        static void drawLine(unsigned char* img, int img_width, int img_height, int x1, int y1, int x2, int y2, int r, int g, int b, int linewidth, bool overwrite);
};

#endif
