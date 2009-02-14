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
	static void SeparateCharactersStandardPlate(
		float expected_character_width_percent,
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
