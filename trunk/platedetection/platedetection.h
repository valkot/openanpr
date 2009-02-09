#ifndef PLATEDETECTION_H_
#define PLATEDETECTION_H_

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
	static bool Find(
		    unsigned char *img_mono,
		    int img_width, int img_height,
		    int erosion_dilation,
		    polygon2D* plate,
		    bool debug,
		    std::vector<unsigned char*> &debug_images);

	platedetection();
	virtual ~platedetection();
};

#endif /* PLATEDETECTION_H_ */
