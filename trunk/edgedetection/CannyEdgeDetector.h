/*
    canny edge detection
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

#ifndef CANNYEDGEDETECTOR_H_
#define CANNYEDGEDETECTOR_H_

#define HISTOGRAM_LENGTH	256

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#define SQUARE_MAG(a,b) (a*a + b*b)

#include <omp.h>
#include <math.h>
#include <string>
#include <cstdlib>
#include <vector>
#include "../common.h"
#include "../utils/Image.h"

#ifndef PI
    #define PI 3.14159265358979323846264338327950288419716939937510
#endif

struct Kernel {
	unsigned int	Size;
	float			*Data;
};

class CannyEdgeDetector
{
private:
	float	GAUSSIAN_CUT_OFF;
	float	MAGNITUDE_SCALE;
	float	MAGNITUDE_LIMIT;
	float   MAGNITUDE_LIMIT_SQR;
	int		MAGNITUDE_MAX;

	float	gaussianKernelRadius;
	float	lowThreshold;
	float	highThreshold;
	unsigned int		gaussianKernelWidth;
	unsigned int		kwidth;
	Image	image;
	Kernel	kernel;
	Kernel	diffKernel;
	unsigned int		picSize;

	int		*data;
	int		*magnitude;

	float	*xConv;
	float	*yConv;
	float	*xGradient;
	float	*yGradient;

    int     *edge_pixel_index;
    float   *edge_magnitude;

	void			GetThresholds(unsigned int histogram[], float *meanDark, float *meanLight);
	void			AutoThreshold(int samplingStepSize);
	unsigned char	Luminance(unsigned char r, unsigned char g, unsigned char b);
	void			ReadLuminance();
	int 			ComputeGradients(float kernelRadius, unsigned int kernelWidth);
	void			CreateMasks(float kernelRadius, unsigned int kernelWidth);
	void			PerformHysteresis(int low, int high, int *followedEdges, int no_of_edges);
    void            Follow(unsigned int x1, unsigned int y1, unsigned int i1, int *threshold, int *followedEdges, int *width, int *height);
    void            Follow2(unsigned int x1, unsigned int y1, unsigned int i1, int *threshold, int *followedEdges, int *width, int *height);
	void			thresholdEdges();
	float			Gaussian(float x, float sigma);
	void			initArrays();
    void            GetCorners(int width, int height, int bytes_per_pixel, std::vector<int> &corners);

public:
	bool	         automaticThresholds;
	std::vector<int> edges;
	unsigned char*   edgesImage;

	float contrast_multiplier;
	float lowThresholdOffset;
	float lowThresholdMultiplier;
	float highhresholdOffset;
	float highhresholdMultiplier;

	CannyEdgeDetector();
	~CannyEdgeDetector();

	void	         Update(Image sourceImage);
	unsigned char*   Update(unsigned char* data, int image_width, int image_height,
                            int bytes_per_pixel);

    void             ConnectBrokenEdges(int maximum_separation, int width, int height, int bytes_per_pixel);
};

#endif /*CANNYEDGEDETECTOR_H_*/
