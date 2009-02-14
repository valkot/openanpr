
#include "platereader.h"

void platereader::SeparateCharactersStandardPlate(
	float expected_character_width_percent,
	int plate_image_width,
	std::vector<int> plate_image_height,
    std::vector<unsigned char*> &binary_images,
    std::vector<std::vector<unsigned char*> > &characters,
    std::vector<std::vector<int> > &characters_dimensions)
{
	int* interval = new int[plate_image_width];
	int suppression_radius = (int)(plate_image_width * expected_character_width_percent / 200);

    for (int p = 0; p < (int)binary_images.size(); p++)
    {
    	unsigned char* number_plate = binary_images[p];

    	memset(interval, 0, plate_image_width * sizeof(int));

    	int height = plate_image_height[p];
    	for (int x = 0; x < plate_image_width; x++)
    	{
    		for (int y = 0; y < height; y++)
    		{
    		    int n = (y * plate_image_width) + x;
    		    if (number_plate[n] == 0) interval[x]++;
    		}
    	}

    	// remove below average data
    	float average_interval = 0;
    	for (int x = 0; x < plate_image_width; x++)
    	    average_interval += interval[x];
    	average_interval /= plate_image_width;
    	for (int x = 0; x < plate_image_width; x++)
    	    if (interval[x] < average_interval) interval[x] = 0;

        // non-maximal suppression
    	for (int x = 0; x < plate_image_width; x++)
    	{
        	for (int x2 = x + 1; x2 < x + suppression_radius; x2++)
        	{
        		if (interval[x2] < interval[x])
        		{
        			interval[x2] = 0;
        		}
        		else
        		{
        		    interval[x] = 0;
        		    break;
        		}
        	}
    	}

    	// lift and separate
    	std::vector<unsigned char*> chars;
    	std::vector<int> chars_dimensions;
    	int prev_x = 0;
    	for (int x = 0; x < plate_image_width; x++)
    	{
    		int xx = x;
            if ((interval[xx] > 0) || (xx == plate_image_width - 1))
            {
            	if (xx - prev_x > suppression_radius * 2)
            	{
            	    prev_x = xx - (suppression_radius * 2);
            	}
            	if (xx == plate_image_width - 1)
            	{
            		xx = prev_x + (suppression_radius * 2);
            		if (xx >= plate_image_width) xx = plate_image_width - 1;
            	}

                if (xx - prev_x > suppression_radius)
                {
                    int tx = prev_x;
                    int ty = 0;
                    int bx = xx;
                    int by = plate_image_height[p];

                    unsigned char *ch = new unsigned char[(bx-tx)*(by-ty)];
                    int n = 0;
                    for (int yy = ty; yy < by; yy++)
                    {
                    	for (int xx = tx; xx < bx; xx++, n++)
                    	{
                    		int n2 = (yy * plate_image_width) + xx;
                    		ch[n] = number_plate[n2];
                    	}
                    }
                    chars.push_back(ch);
                    chars_dimensions.push_back(bx - tx);
                    chars_dimensions.push_back(by - ty);
                }
            }
    	}

    	characters.push_back(chars);
    	characters_dimensions.push_back(chars_dimensions);
    }

    delete[] interval;
}

void platereader::Binarise(
	int plate_image_width,
	std::vector<int> plate_image_height,
    std::vector<unsigned char*> &plate_images,
    std::vector<unsigned char*> &binary_images)
{
	// allocate histogram buffers
	int* histogram = new int[256];
	int* temp_histogram_buffer = new int[256];

	float MeanDark = 0;
	float MeanLight = 0;
	float DarkRatio = 0;
    for (int p = 0; p < (int)plate_images.size(); p++)
    {
    	// number plate image
    	unsigned char* number_plate = plate_images[p];

    	// update the histogram
    	memset(histogram, 0, 256 * sizeof(int));
    	for (int i = (plate_image_width * plate_image_height[p])-1; i >= 0; i--)
    		histogram[number_plate[i]]++;

    	// find the black/white threshold from the histogram
    	unsigned char black_white_threshold = (unsigned char)thresholding::GetGlobalThreshold(histogram, 256, 0, temp_histogram_buffer, MeanDark, MeanLight, DarkRatio);

    	// binarise the image
    	unsigned char* edges = new unsigned char[plate_image_width * plate_image_height[p]];
    	memset(edges, 0, plate_image_width * plate_image_height[p] * sizeof(unsigned char));
    	for (int i = (plate_image_width * plate_image_height[p])-1; i >= 0; i--)
    	{
    	    if (number_plate[i] < black_white_threshold)
    	    {
    	        edges[i] = 255;
    	    }
    	}

    	binary_images.push_back(edges);
    }

    // free histogram buffers
    delete[] histogram;
    delete[] temp_histogram_buffer;
}
