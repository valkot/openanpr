#ifndef PLATEDETECTION_H_
#define PLATEDETECTION_H_

#define PLATE_YELLOW  0
#define PLATE_WHITE   1

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#include <math.h>
#include <string>
#include <cstdlib>
#include <vector>
#include "../common.h"
#include "../utils/Image.h"
#include "../utils/polygon.h"
#include "../shapes/shapes.h"

class platedetection
{
public:
	static void MergeRectangles(std::vector<polygon2D*> &rectangles);

	static void ColourFilter(
	    unsigned char* img_colour,
	    int img_width, int img_height,
	    unsigned char* filtered);

	static bool Find(
		    unsigned char *img_colour,
		    int img_width, int img_height,
		    std::vector<polygon2D*> &plates,
		    bool debug,
		    std::vector<unsigned char*> &debug_images,
			int &debug_image_width,
			int &debug_image_height);

	static void ExtractPlateImages(
	    unsigned char *img_colour,
	    int img_width, int img_height,
	    std::vector<polygon2D*> &plates,
	    int plate_image_width,
	    std::vector<int> &plate_image_height,
	    std::vector<unsigned char*> &plate_images);

};

#endif /* PLATEDETECTION_H_ */
