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

#include "CannyEdgeDetector.h"

/*! \brief Constructor */
CannyEdgeDetector::CannyEdgeDetector()
{
	GAUSSIAN_CUT_OFF	= 0.005;
	MAGNITUDE_SCALE		= 100.0;
	MAGNITUDE_LIMIT		= 1000.0;
	MAGNITUDE_LIMIT_SQR	= MAGNITUDE_LIMIT * MAGNITUDE_LIMIT;
	MAGNITUDE_MAX		= (int)(MAGNITUDE_SCALE * MAGNITUDE_LIMIT);

	lowThreshold = 2.5;
	highThreshold = 7.5;
	gaussianKernelRadius = 2.0;
	gaussianKernelWidth = 8;
	kwidth = 0;
	image.Width = 0;
	image.Height = 0;
	image.BytesPerPixel = 0;
	kernel.Size = 0;
	diffKernel.Size = 0;
	picSize = 0;
	automaticThresholds = true;

	contrast_multiplier = 0.35f;
	lowThresholdOffset = 1.6f;
	lowThresholdMultiplier = 6.4f;
	highhresholdOffset = 2.0f;
	highhresholdMultiplier = 8.0f;

	// Ensure pointers are NULLified, ensures free() works even if no allocation occured
	data			 = NULL;
	magnitude		 = NULL;
	xConv			 = NULL;
	yConv			 = NULL;
	xGradient		 = NULL;
	yGradient		 = NULL;
	kernel.Data		 = NULL;
	diffKernel.Data	 = NULL;
    edge_pixel_index = NULL;
    edge_magnitude   = NULL;
    edgesImage       = NULL;

	// Initalise kernel size's to be zero
	kernel.Size = 0;
	diffKernel.Size = 0;
}

/*! \brief calls the main update routine
 * \param sourceImage The image to be processed
 * \param image_width width of the image
 * \param image_height height of the image
 */
unsigned char* CannyEdgeDetector::Update(
    unsigned char* data,
    int image_width,
    int image_height,
    int bytes_per_pixel)
{
	Image img;
	img.Data = data;
	img.Width = image_width;
	img.Height = image_height;
	img.BytesPerPixel = bytes_per_pixel;

	Update(img);

	return(edgesImage);
}

/*! \brief The main update routine
 * \param sourceImage The image to be processed
 */
void CannyEdgeDetector::Update(Image sourceImage)
{
	bool reinitialise = false;
	if ((sourceImage.Width != image.Width) ||
		(sourceImage.Height != image.Height))
		reinitialise = true;

	image = sourceImage;
	int low = 0;
	int high = 0;

	// Adjust thresholds automatically
	if(automaticThresholds)
	{
		AutoThreshold(2);
	}

	picSize = image.Width * image.Height;

	initArrays();
	ReadLuminance();
	int no_of_edges = ComputeGradients(gaussianKernelRadius, gaussianKernelWidth);

	low = (int)roundf(lowThreshold * MAGNITUDE_SCALE);
	high = (int)roundf(highThreshold * MAGNITUDE_SCALE);

	PerformHysteresis(low, high, data, no_of_edges);
	thresholdEdges();

    // create an image showing the edges
    int pixels = image.Width * image.Height * image.BytesPerPixel;

    if ((edgesImage == NULL) || (reinitialise))
    {
    	if (edgesImage != NULL) delete[] edgesImage;
        edgesImage = new unsigned char[pixels];
    }

    for (int i = pixels-1; i >= 0; i--) edgesImage[i] = 255;

    for (int i = 0; i < (int)edges.size(); i += 2)
    {
        int x = edges[i];
        int y = edges[i + 1];

        int n = ((y * image.Width) + x) * image.BytesPerPixel;
        for (int col = 0; col < image.BytesPerPixel; col++)
            edgesImage[n + col] = 0;
    }
}

/*! \brief Automatically discover appropriate high and low thresholds
 * \param samplingStepSize Step size to be used when sampling the raw image
 */
void CannyEdgeDetector::AutoThreshold(int samplingStepSize)
{
	unsigned int	tx = image.Width / 3;
	unsigned int	ty = image.Height / 3;
	unsigned int	bx = image.Width - 1 - tx;
	unsigned int	by = image.Height - 1 - ty;
	unsigned int	histogram[HISTOGRAM_LENGTH] = {0.0};
	float			meanDark = 0;
	float			meanLight = 0;
	float			contrast = 0;
	float			fraction = 0;

	for(unsigned int y = ty; y <= by; y += samplingStepSize)
	{
		unsigned int n = (y * image.Width) + tx;

		if(image.BytesPerPixel == 1)
		{
			for (unsigned int x = tx; x <= bx; x += samplingStepSize, n++)
				histogram[image.Data[n]]++;
		}
		else
		{
			n *= image.BytesPerPixel;
			for (unsigned int x = tx; x <= bx; x += samplingStepSize, n += image.BytesPerPixel)
				histogram[image.Data[n + 2]]++;
		}
	}

	GetThresholds(histogram, &meanDark, &meanLight);
	meanDark /= 255.0;
	meanLight /= 255.0;
	contrast = meanLight - meanDark;

	float contrast_mult = (1.0f - (contrast * contrast_multiplier));
	contrast *= contrast_mult;

	fraction = (contrast - 0.048) / (0.42 - 0.048);

	lowThreshold = lowThresholdOffset + (fraction * lowThresholdMultiplier);
	highThreshold = highhresholdOffset + (fraction * highhresholdMultiplier);
}

/*! \brief Detects the high and low thresholds used by the hysteresis function
 * \param histogram luminence histogram
 * \param meanDark low threshold
 * \param meanLight high threshold
 */
void CannyEdgeDetector::GetThresholds(unsigned int histogram[], float *meanDark, float *meanLight)
{
	float	Tmin = 0.0;
	float	Tmax = 0.0;
	float	minVariance = 999999.0;	// FIXME: Some large value
	float	currMeanDark = 0.0;
	float	currMeanLight = 0.0;
	float	varianceDark = 0.0;
	float	varianceLight = 0.0;
	float	darkHits = 0.0;
	float	lightHits = 0.0;
	float	bestDarkHits = 0.0;
	float	bestLightHits = 0.0;
	float	histogramSquaredMagnitude[HISTOGRAM_LENGTH] = {0.0};
	float	mult1 = 0.0;
	//float	mult2 = 0.0;
	int		h = 0;
	int		bucket = 0;
	float	magnitudeSqr = 0.0;
	float	variance = 0.0;
	float	divisor	= 0.0;
	*meanDark = 0;
	*meanLight = 0;

	// Calculate squared magnitudes - avoids unneccessary multiplies later on
	for(unsigned int i = 0; i < HISTOGRAM_LENGTH; i++)
	{
		histogramSquaredMagnitude[i] = histogram[i] * histogram[i];
	}

	// Precompute some values to avoid excessive division
	mult1 = (float)(HISTOGRAM_LENGTH-1) / 255.0f;

	// Evaluate all possible thresholds
	for(int greyLevel = 255; greyLevel >= 0; greyLevel--)
	{
		darkHits = 0;
		lightHits = 0;
		currMeanDark = 0;
		currMeanLight = 0;
		varianceDark = 0;
		varianceLight = 0;

		bucket = (int)(greyLevel * mult1);

		for(h = HISTOGRAM_LENGTH - 1; h >= 0; h--)
		{
			magnitudeSqr = histogramSquaredMagnitude[h];
			if(h < bucket)
			{
				currMeanDark += h * magnitudeSqr;
				varianceDark += (bucket - h) * magnitudeSqr;
				darkHits += magnitudeSqr;
			}
			else
			{
				currMeanLight += h * magnitudeSqr;
				varianceLight += (bucket - h) * magnitudeSqr;
				lightHits += magnitudeSqr;
			}
		}

		if(darkHits > 0)
		{
			// Rescale into 0-255 range
			divisor = darkHits * HISTOGRAM_LENGTH;
			currMeanDark = (currMeanDark * 255) / divisor;
			varianceDark = (varianceDark * 255) / divisor;
		}

		if(lightHits > 0)
		{
			// Rescale into 0-255 range
			divisor = lightHits * HISTOGRAM_LENGTH;
			currMeanLight = (currMeanLight * 255) / divisor;
			varianceLight = (varianceLight * 255) / divisor;
		}

		variance = varianceDark + varianceLight;
		if(variance < 0)
		{
			variance = -variance;
		}

		if(variance < minVariance)
		{
			minVariance = variance;
			Tmin = greyLevel;
			*meanDark = currMeanDark;
			*meanLight = currMeanLight;
			bestDarkHits = darkHits;
			bestLightHits = lightHits;
		}

		if((int)(variance * 1000) == (int)(minVariance * 1000))
		{
			Tmax = greyLevel;
			*meanLight = currMeanLight;
			bestLightHits = lightHits;
		}
	}
}

/*! \brief Converts RGB to luminence
 * \param r red
 * \param g green
 * \param b blue
 * \return luminence
 */
unsigned char CannyEdgeDetector::Luminance(unsigned char r, unsigned char g, unsigned char b)
{
	return (unsigned char)(((299 * r) + (587 * g) + (114 * b)) / 1000);
}

/*! \brief Converts an RGB image to a luminence image */
void CannyEdgeDetector::ReadLuminance()
{
	unsigned char	*pixels	= image.Data;
	int				offset	= picSize - 1;

	if(picSize == image.Width * image.Height * 3)
	{

		int n = (image.Width * image.Height) - 1;
		for(int i = picSize - 2; i >= 0; i -= 3)
		{
			unsigned char r = pixels[offset--] & 0xFF;
			unsigned char g = pixels[offset--] & 0xFF;
			unsigned char b = pixels[offset--] & 0xFF;
			data[n--] = Luminance(r, g, b);
		}
	}
	else
	{
		for(int i = picSize - 1; i >= 0; i--)
		{
			data[i] = pixels[offset--] & 0xFF;
		}
	}
}

/*! \brief convolve with a gaussian filter and perform non-maximal supression
 * \param kernelRadius radius of the gaussian
 * \param kernelWidth width of the convolution kernel in pixels
 */
int CannyEdgeDetector::ComputeGradients(float kernelRadius, unsigned int kernelWidth)
{
	int initX = 0;
	int maxX = 0;
	int initY = 0;
	int maxY = 0;
	int no_of_edges = 0;

	// Create gaussian convolution masks
	CreateMasks(kernelRadius, kernelWidth);

	initX = kwidth - 1;
	maxX = image.Width - (kwidth - 1);
	initY = image.Width * (kwidth - 1);
	maxY = image.Width * (image.Height - (kwidth - 1));
    int w = image.Width;

	// Perform convolution in x and y directions
	for(int y = initY; y < maxY; y+= w)
	{
		for(int x = initX; x < maxX; x++)
		{
			int index = x + y;
			float sumX = data[index] * kernel.Data[0];
			float sumY = sumX;
			unsigned int xOffset = 1;
			int yOffset = w;
			int index1 = index - yOffset;
			int index2 = index + yOffset;
			int index3 = index - xOffset;
			int index4 = index + xOffset;
			float k = 0.0f;
			for (xOffset = 1; xOffset < kwidth; xOffset++, index3--,index4++)
			{
				k = kernel.Data[xOffset];

				sumY += k * (data[index1] + data[index2]);
				sumX += k * (data[index3] + data[index4]);
				index1 -= w;
				index2 += w;
			}

			yConv[index] = sumY;
			xConv[index] = sumX;
		}
	}

	float *kern = diffKernel.Data;
	for(int x = initX; x < maxX; x++)
	{
		for(int y = initY; y < maxY; y += w)
		{
			float sum = 0.0f;
			int index = x + y;
			for(unsigned int i = 1; i < kwidth; i++)
			{
				sum += kern[i] * (yConv[index - i] - yConv[index + i]);
			}

			xGradient[index] = sum;
		}
	}

	for(unsigned int x = kwidth; x < w - kwidth; x++)
	{
		for (int y = initY; y < maxY; y += w)
		{
			float sum = 0.0f;
			int index = x + y;
			int yOffset = w;
			for(unsigned int i = 1; i < kwidth; i++)
			{
				sum += kern[i] * (xConv[index - yOffset] - xConv[index + yOffset]);
				yOffset += w;
			}

			yGradient[index] = sum;
		}
	}

	initX = kwidth;
	maxX = image.Width - kwidth;
	initY = image.Width * kwidth;
	maxY = image.Width * (image.Height - kwidth);

	for (int y = initY; y < maxY; y += image.Width)
	{
		for (int x = initX; x < maxX; x++)
		{
			int index = x + y;

			float xGrad = xGradient[index];
			float yGrad = yGradient[index];

            float gradMag = SQUARE_MAG(xGrad, yGrad);

			//perform non-maximal supression
			float tmp = 0;

            float xGrad_abs = ABS(xGrad);
            float yGrad_abs = ABS(yGrad);

            bool is_edge = false;
            if (xGrad * yGrad <= 0.0f)
            {
                int indexNE = index - w + 1;
                float neMag = SQUARE_MAG(xGradient[indexNE], yGradient[indexNE]);
	            float sumGrad = xGrad + yGrad;
                if (xGrad_abs >= yGrad_abs)
                {
                    int indexE = index + 1;
                    float eMag = SQUARE_MAG(xGradient[indexE], yGradient[indexE]);
                    if ((tmp = xGrad_abs * gradMag) >= ABS((yGrad * neMag) - (sumGrad * eMag)))
                    {
                        int indexSW = index + w - 1;
                        int indexW = index - 1;
                        float swMag = SQUARE_MAG(xGradient[indexSW], yGradient[indexSW]);
                        float wMag = SQUARE_MAG(xGradient[indexW], yGradient[indexW]);
                        if (tmp > ABS((yGrad * swMag) - (sumGrad * wMag)))
                            is_edge = true;
                    }
                }
                else
                {
                    int indexN = index - w;
                    float nMag = SQUARE_MAG(xGradient[indexN], yGradient[indexN]);
                    if ((tmp = ABS(yGrad * gradMag)) >= ABS((xGrad * neMag) - (sumGrad * nMag)))
                    {
                        int indexS = index + w;
                        int indexSW = indexS - 1;
                        float swMag = SQUARE_MAG(xGradient[indexSW], yGradient[indexSW]);
                        float sMag = SQUARE_MAG(xGradient[indexS], yGradient[indexS]);
                        if (tmp > ABS((xGrad * swMag) - (sumGrad * sMag)))
                            is_edge = true;
                    }
                }
            }
            else
            {
                int indexSE = index + w + 1;
                float seMag = SQUARE_MAG(xGradient[indexSE], yGradient[indexSE]);
                if (xGrad_abs >= yGrad_abs)
                {
                    int indexE = index + 1;
                    float eMag = SQUARE_MAG(xGradient[indexE], yGradient[indexE]);
                    if ((tmp = xGrad_abs * gradMag) >= ABS((yGrad * seMag) + ((xGrad - yGrad) * eMag)))
                    {
                        int indexNW = index - w - 1;
                        int indexW = index - 1;
                        float nwMag = SQUARE_MAG(xGradient[indexNW], yGradient[indexNW]);
                        float wMag = SQUARE_MAG(xGradient[indexW], yGradient[indexW]);
                        if (tmp > ABS((yGrad * nwMag) + ((xGrad - yGrad) * wMag)))
                            is_edge = true;
                    }
                }
                else
                {
                    int indexS = index + w;
                    float sMag = SQUARE_MAG(xGradient[indexS], yGradient[indexS]);
                    if ((tmp = yGrad_abs * gradMag) >= ABS((xGrad * seMag) + ((yGrad - xGrad) * sMag)))
                    {
                        int indexN = index - w;
                        int indexNW = indexN - 1;
                        float nwMag = SQUARE_MAG(xGradient[indexNW], yGradient[indexNW]);
                        float nMag = SQUARE_MAG(xGradient[indexN], yGradient[indexN]);
                        if (tmp > ABS((xGrad * nwMag) + ((yGrad - xGrad) * nMag)))
                            is_edge = true;
                    }
                }
            }

            if (is_edge)
            {
	            // record the squared magnitude
                if (gradMag >= MAGNITUDE_LIMIT_SQR)
                    edge_magnitude[index] = -1;
                else
                    edge_magnitude[index] = gradMag;

                // record the edge position
                edge_pixel_index[no_of_edges++] = index;

                if (gradMag < 0)
                    magnitude[index] = MAGNITUDE_MAX;
                else
                    magnitude[index] = (int)(MAGNITUDE_SCALE * sqrt(gradMag));
            }
		}
	}

	return(no_of_edges);
}

/*! \brief creates convolution masks
 * \param kernelRadius radius of the gaussian
 * \param kernelWidth width of the convolution kernel in pixels
 */
void CannyEdgeDetector::CreateMasks(float kernelRadius, unsigned int kernelWidth)
{
	if(kernel.Size == 0 || kernel.Size != kernelWidth)
	{
		kernel.Data		= (float*) realloc(kernel.Data,kernelWidth*sizeof(float));
		kernel.Size		= kernelWidth;
		diffKernel.Data	= (float*) realloc(diffKernel.Data,kernelWidth*sizeof(float));
		diffKernel.Size = kernelWidth;

		for(unsigned int i = 0; i < kernelWidth; i++)
		{
			kernel.Data[i] = 0;
			diffKernel.Data[i] = 0;
		}

		for(kwidth = 0; kwidth < kernelWidth; kwidth++)
		{
			float g1 = Gaussian(kwidth, kernelRadius);
			if(g1 <= GAUSSIAN_CUT_OFF && kwidth >= 2)
				break;
			float g2 = Gaussian(kwidth - 0.5, kernelRadius);
			float g3 = Gaussian(kwidth + 0.5, kernelRadius);
			kernel.Data[kwidth] = ((g1 + g2 + g3) / 3.0) / (2.0 * (float)PI * kernelRadius * kernelRadius);
			diffKernel.Data[kwidth] = g3 - g2;
		}
	}
}

/*! \brief hysteresis
 * \param low low threshold
 * \param high high threshold
 * \param followedEdges list of pixel indexes for edges followed
 */
void CannyEdgeDetector::PerformHysteresis(int low, int high, int *followedEdges, int no_of_edges)
{
	int imageSize = image.Width * image.Height;
    int offset = imageSize - 1;

	// clear array containing followed edges
	for(int i = offset; i >= 0; i--)
	{
		followedEdges[i] = 0;
	}

    int w = image.Width;
    int h = image.Height;

    for (int i = no_of_edges - 1; i >= 0; i--)
    {
	    int index = edge_pixel_index[i];

	    if (followedEdges[index] == 0)
	    {
	        if (magnitude[index] >= high)
    		{
    			int y = index % w;
    			int x = index - (y * w);
    			Follow2(x,y,index,&low,followedEdges,&w,&h);
    		}
	    }
    }
}

/*! \brief recursively follow an edge until a minimum magnitude is reached
 * \param x1 current x coordinate
 * \param y1 current y coordinate
 * \param il current pixel index
 * \param threshold minimum gradient magnitude threshold
 * \param followedEdges list of pixel indexes for edges followed
 */
void CannyEdgeDetector::Follow(unsigned int x1, unsigned int y1, unsigned int i1, int *threshold, int *followedEdges,
                               int *width, int *height)
{
	unsigned int x0 = x1 == 0 ? x1 : x1 - 1;
	unsigned int x2 = x1 == (unsigned int)*width - 1 ? x1 : x1 + 1;
	unsigned int y0 = y1 == 0 ? y1 : y1 - 1;
	unsigned int y2 = y1 == (unsigned int)*height - 1 ? y1 : y1 + 1;

	followedEdges[i1] = magnitude[i1];

	for(unsigned int y = y0; y <= y2; y++)
	{
		unsigned int i2 = y * *width;
        unsigned int i3 = i2 + x0;

		for(unsigned int x = x0; x <= x2; x++, i3++)
		{
			if (followedEdges[i3] == 0)        // hasn't been followed
		        if(magnitude[i3] >= *threshold) // with sufficient magnitude
			    {
				    Follow(x, y, i3, threshold, followedEdges, width, height);
				    return;
			    }
		}
	}
}

/*! \brief follow an edge until a minimum magnitude is reached
 * \param x1 current x coordinate
 * \param y1 current y coordinate
 * \param il current pixel index
 * \param threshold minimum gradient magnitude threshold
 * \param followedEdges list of pixel indexes for edges followed
 */
void CannyEdgeDetector::Follow2(unsigned int x1, unsigned int y1, unsigned int i1, int *threshold, int *followedEdges,
                                int *width, int *height)
{
    bool following = true;
    while (following)
    {
        following = false;

        unsigned int x0 = x1 == 0 ? x1 : x1 - 1;
        unsigned int x2 = x1 == (unsigned int)*width - 1 ? x1 : x1 + 1;
        unsigned int y0 = y1 == 0 ? y1 : y1 - 1;
        unsigned int y2 = y1 == (unsigned int)*height - 1 ? y1 : y1 + 1;

        followedEdges[i1] = magnitude[i1];

        unsigned int y = y0;
        while ((y <= y2) && (!following))
        {
            unsigned int i2 = y * *width;
            unsigned int i3 = i2 + x0;

            unsigned int x = x0;
            while ((x <= x2) && (!following))
            {
                if (followedEdges[i3] == 0)        // hasn't been followed
                    if(magnitude[i3] >= *threshold) // with sufficient magnitude
                    {
                        i1 = i3;
                        x1 = x;
                        y1 = y;
                        following = true;
                    }
                x++;
                i3++;
            }
            y++;
        }
    }
}

/*! \brief binarise edge values to 0 and 255 */
void CannyEdgeDetector::thresholdEdges()
{
	edges.clear();
	int x = image.Width-1;
	int y = image.Height-1;
	for (int i = (image.Width * image.Height) - 1; i >= 0; i--)
	{
		if (data[i] != 0)
		{
		    data[i] = 0;
		    edges.push_back(x);
		    edges.push_back(y);
		}
		else
		{
		    data[i] = 255;
	    }

	    x--;
	    if (x < 0)
	    {
		    x += image.Width;
		    y--;
	    }
    }
}

/*! \brief gaussian function
 * \param x
 * \param sigma
 */
float CannyEdgeDetector::Gaussian(float x, float sigma)
{
	return(exp(-(x * x) / (2.0f * sigma * sigma)));
}

/*! \brief initialise arrays */
void CannyEdgeDetector::initArrays()
{
	data		= (int*)	realloc(data,picSize*sizeof(int));
	magnitude	= (int*)	realloc(magnitude,picSize*sizeof(int));
	xConv		= (float*)	realloc(xConv,picSize*sizeof(float));
	yConv		= (float*)	realloc(yConv,picSize*sizeof(float));
	xGradient	= (float*)	realloc(xGradient,picSize*sizeof(float));
	yGradient	= (float*)	realloc(yGradient,picSize*sizeof(float));
    edge_pixel_index = (int*) realloc(edge_pixel_index,picSize*sizeof(int));
    edge_magnitude = (float*) realloc(edge_magnitude, picSize*sizeof(float));

	for(unsigned int i = 0; i < picSize; i++)
	{
		data[i] = 0;
		magnitude[i] = 0;
		xConv[i] = 0;
		yConv[i] = 0;
		xGradient[i] = 0;
		yGradient[i] = 0;
		edge_pixel_index[i] = 0;
		edge_magnitude[i] = 0;
	}

}


void CannyEdgeDetector::GetCorners(int width, int height,
                                   int bytes_per_pixel,
                                   std::vector<int> &corners)
{
    //corners.erase(corners.begin(), corners.end());

    int stride = (width - bytes_per_pixel) * bytes_per_pixel;

    for (int i = 0; i < (int)edges.size(); i += 2)
    {
        int x = edges[i];
        int y = edges[i + 1];
        if ((x > 0) && (x < width - 1) &&
            (y > 0) && (y < height - 1))
        {
            int adjacent_edges = 0;
            int n = (((y - 1) * width) + x - 1) * bytes_per_pixel;
            for (int yy = y - 1; yy <= y + 1; yy++)
            {
                for (int xx = x - 1; xx <= x + 1; xx++)
                {
                    if (edgesImage[n] == 0)
                    {
                        adjacent_edges++;
                        if (adjacent_edges > 2)
                        {
                            xx = x + 2;
                            yy = y + 2;
                        }
                    }
                    n += 3;
                }
                n += stride;
            }

            if (adjacent_edges <= 2)
            {
                corners.push_back(x);
                corners.push_back(y);
            }
        }
    }
}

void CannyEdgeDetector::ConnectBrokenEdges(
    int maximum_separation,
    int width,
    int height,
    int bytes_per_pixel)
{
    std::vector<int> corners;
    GetCorners(width, height, bytes_per_pixel, corners);
    if ((int)corners.size() > 0)
    {
        bool* connected = new bool[(int)corners.size()];
        for (int i = (int)corners.size()-1; i >= 0; i--)
            connected[i] = false;

        for (int i = 0; i < (int)corners.size() - 2; i += 2)
        {
            int x0 = corners[i];
            int y0 = corners[i + 1];
            for (int j = i + 2; j < (int)corners.size(); j += 2)
            {
                if (!connected[j])
                {
                    int x1 = corners[j];
                    int dx = x1 - x0;
                    if (dx < 0) dx = -dx;
                    if (dx <= maximum_separation)
                    {
                        int y1 = corners[j + 1];
                        int dy = y1 - y0;
                        if (dy < 0) dy = -dy;
                        if (dy <= maximum_separation)
                        {
                            int dist = (int)sqrt((dx * dx) + (dy * dy));
                            for (int d = 1; d < dist; d++)
                            {
                                int ix = x0 + (d * dx / dist);
                                int iy = y0 + (d * dy / dist);
                                edges.push_back(ix);
                                edges.push_back(iy);
                            }

                            connected[j] = true;
                            j = (int)corners.size();
                        }
                    }
                }
            }
        }
        delete[] connected;
    }
}



/*! \brief destructor */
CannyEdgeDetector::~CannyEdgeDetector()
{
    if (kernel.Data != NULL)
    {
    	free(data);
    	free(magnitude);
    	free(xConv);
    	free(yConv);
    	free(xGradient);
    	free(yGradient);
    	free(kernel.Data);
    	free(diffKernel.Data);
    	free(edge_pixel_index);
    	free(edge_magnitude);
    }
    delete[] edgesImage;
}
