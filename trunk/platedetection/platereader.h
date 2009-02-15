#ifndef PLATEREADER_H_
#define PLATEREADER_H_

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

class platereader
{
public:
	static void Erode(
	    int character_image_width,
	    int character_image_height,
	    unsigned char* character_image,
	    int minimum_occupancy_percent);

	static void Trim(
	    int plate_image_width,
	    int plate_image_height,
	    unsigned char* number_plate,
	    float threshold_percent,
	    int &tx,
	    int &ty,
	    int &bx,
	    int &by);

	static void VerticalCrop(
	    int plate_image_width,
	    int plate_image_height,
	    unsigned char* number_plate,
	    int &top_y,
	    int &bottom_y);

	static void SeparateCharactersStandardPlate(
		float minimum_character_width_percent,
		int plate_image_width,
		std::vector<int> plate_image_height,
	    std::vector<unsigned char*> &binary_images,
	    std::vector<std::vector<unsigned char*> > &characters,
	    std::vector<std::vector<int> > &characters_dimensions);

	static void Binarise(
		int plate_image_width,
		std::vector<int> plate_image_height,
	    std::vector<unsigned char*> &plate_images,
	    std::vector<unsigned char*> &binary_images);

};

#endif /* PLATEREADER_H_ */
