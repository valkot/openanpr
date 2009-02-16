/*
    number plate reading
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

#include "platereader.h"



void platereader::RemoveStragglers(
    std::vector<int> &character_image_dimensions,
    std::vector<int> &character_positions,
    std::vector<unsigned char*> &character_images)
{
	if ((int)character_images.size() > 0)
	{
		int average_height = 0;
		for (int i = (int)character_images.size()-1; i >= 0; i--)
			average_height += character_image_dimensions[(i*2)+1];

		average_height /= (int)character_images.size();
        int minimum_height = average_height * 70/100;

        for (int i = (int)character_images.size()-1; i >= 0; i--)
		{
			int height = character_image_dimensions[(i*2)+1];
			if (height < minimum_height)
			{
				delete[] character_images[i];
				character_images[i] = NULL;
				character_images.erase(character_images.begin()+i);
				character_image_dimensions.erase(character_image_dimensions.begin()+(i*2)+1);
				character_image_dimensions.erase(character_image_dimensions.begin()+(i*2));
				character_positions.erase(character_positions.begin()+(i*2)+1);
				character_positions.erase(character_positions.begin()+(i*2));
			}
		}
	}
}

/*!
 * \brief resample the character images into a fixed resolution ready for recognition
 * \param character_image_dimensions width and height of each character image
 * \param character_images image data for each character
 * \param resampled_width resampled width
 * \param resampled_height resampled height
 * \param resampled_character_images resamples image for each character
 */
void platereader::Resample(
	std::vector<int> &character_image_dimensions,
    std::vector<unsigned char*> &character_images,
	int resampled_width,
	int resampled_height,
    std::vector<unsigned char*> &resampled_character_images)
{
    for (int i = 0; i < (int)character_images.size(); i++)
    {
        unsigned char* resampled = new unsigned char[resampled_width * resampled_height];
        Resample(character_image_dimensions[i*2],
        		 character_image_dimensions[(i*2)+1],
        		 character_images[i],
        		 resampled_width,
        		 resampled_height,
        		 resampled);
        resampled_character_images.push_back(resampled);
    }
}

/*!
 * \brief resample the character image into a fixed resolution ready for recognition
 * \param character_image_width width of the character image
 * \param character_image_height height of the character image
 * \param character_image image data
 * \param resampled_width resampled width
 * \param resampled_height resampled height
 * \param resampled_character_image resampled character image data
 */
void platereader::Resample(
    int character_image_width,
	int character_image_height,
	unsigned char* character_image,
	int resampled_width,
	int resampled_height,
	unsigned char* resampled_character_image)
{
	int min_width_percent = 40;
	int min_width = character_image_height* min_width_percent/100;
	int offset = (resampled_width/2) - (resampled_width*min_width_percent/200);
	memset(resampled_character_image, 0, resampled_width * resampled_height * sizeof(unsigned char));
	int n = 0;
	for (int y = 0; y < character_image_height; y++)
	{
		int yy = y * resampled_height / character_image_height;
		for (int x = 0; x < character_image_width; x++, n++)
		{
			if (character_image[n] > 0)
			{
			    int xx = x * resampled_width / character_image_width;

			    if (character_image_width < min_width)
			    {
			    	xx = offset + (x * resampled_width / character_image_height);
			    }

			    if ((xx > -1) && (xx < resampled_width))
			    {
			        int n2 = (yy * resampled_width) + xx;
			        resampled_character_image[n2] = (unsigned char)255;
			    }
			}
		}
	}
}


/*!
 * \brief erodes the given character image, which helps to ensure that the system is invariant to character line thickness
 * \param character_image_width width of the character image
 * \param character_image_height height of the character image
 * \param character_image image data
 * \param minimum_occupancy_percent minimum percentage of occupied pixels
 */
void platereader::Erode(
    int character_image_width,
    int character_image_height,
    unsigned char* character_image,
    int minimum_occupancy_percent)
{
	unsigned char* eroded = new unsigned char[character_image_width * character_image_height];
	unsigned char* buffer = NULL;
	unsigned char* result = NULL;
	memcpy(eroded, character_image, character_image_width * character_image_height * sizeof(unsigned char));

	bool finished = false;
	while (!finished)
	{
		int occupancy = 0;
		for (int i = (character_image_width * character_image_height)-1; i >= 0; i--)
			if (eroded[i] > 0) occupancy++;

		if (occupancy * 100 / (character_image_width * character_image_height) > minimum_occupancy_percent)
		{
		    if (buffer == NULL)
			{
			    buffer = new unsigned char[character_image_width * character_image_height];
			    result = new unsigned char[character_image_width * character_image_height];
		    }
		    processimage::Erode(eroded, character_image_width, character_image_height, buffer, 1, result);
		    memcpy(eroded, result, character_image_width * character_image_height * sizeof(unsigned char));
		}
		else finished = true;
	}

	memcpy(character_image, eroded, character_image_width * character_image_height * sizeof(unsigned char));
	if (result != NULL) delete[] result;
	if (buffer != NULL) delete[] buffer;
	delete[] eroded;
}

/*!
 * \brief trims excess from the given character image
 * \param plate_image_width width of the number plate
 * \param plate_image_height height of the number plate
 * \param number_plate binary image
 * \param threshold_percent threshold used for border
 * \param tx
 * \param ty
 * \param bx
 * \param by
 */
void platereader::Trim(
    int plate_image_width,
    int plate_image_height,
    unsigned char* number_plate,
    float threshold_percent,
    int &tx,
    int &ty,
    int &bx,
    int &by)
{
    int max_crop_x = (bx - tx) * 40 / 100;
    int max_crop_y = (by - ty) * 40 / 100;

    int temp_tx = tx;
    int temp_ty = ty;
    int temp_bx = bx;
    int temp_by = by;

    // top
    for (int y = ty; y < ty + max_crop_y; y++)
    {
    	int occupancy = 0;
    	for (int x = tx; x < bx; x++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] == 0) occupancy++;
    	}
		if (occupancy*100/(bx - tx) > 98) temp_ty = y;
    }

    for (int y = temp_ty; y < ty + max_crop_y; y++)
    {
    	int occupancy = 0;
    	for (int x = tx; x < bx; x++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] > 0) occupancy++;
    	}
		if (occupancy*100/(bx-tx) > threshold_percent)
			break;
		else
			temp_ty = y;
    }

    // bottom
    int start_y = by;
    int end_y = by - 1 - max_crop_y;
    for (int y = start_y; y > end_y; y--)
    {
    	int occupancy = 0;
    	for (int x = tx; x < bx; x++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] == 0) occupancy++;
    	}
		if (occupancy*100/(bx-tx) > 98) temp_by = y;
    }
    start_y = temp_by;
    for (int y = start_y; y > end_y; y--)
    {
    	int occupancy = 0;
    	for (int x = tx; x < bx; x++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] > 0) occupancy++;
    	}
		if (occupancy*100/(bx-tx) > threshold_percent)
			break;
		else
			temp_by = y;
    }

    // left
    for (int x = tx; x < tx+max_crop_x; x++)
    {
    	int occupancy = 0;
    	for (int y = ty; y < by; y++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] == 0) occupancy++;
    	}
		if (occupancy*100/(by-ty) > 98) temp_tx = x;
    }
    for (int x = temp_tx; x < max_crop_x; x++)
    {
    	int occupancy = 0;
    	for (int y = ty; y < by; y++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] > 0) occupancy++;
    	}
		if (occupancy*100/(by-ty) > threshold_percent)
			break;
		else
			temp_tx = x;
    }

    // right
    int start_x = bx;
    int end_x = bx - 1 - max_crop_x;
    for (int x = start_x; x > end_x; x--)
    {
    	int occupancy = 0;
    	for (int y = ty; y < by; y++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] == 0) occupancy++;
    	}
		if (occupancy*100/(by-ty) > 98) temp_bx = x;
    }
    start_x = temp_bx;
    for (int x = start_x; x > end_x; x--)
    {
    	int occupancy = 0;
    	for (int y = ty; y < by; y++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] > 0) occupancy++;
    	}
		if (occupancy*100/(by-ty) > threshold_percent)
			break;
		else
			temp_bx = x;
    }

    tx = temp_tx;
    ty = temp_ty;
    bx = temp_bx;
    by = temp_by;
}

/*!
 * \brief use to remove any vertical excess from the binarised number plate image
 * \param plate_image_width number plate image width
 * \param plate_image_height number plate image height
 * \param number_plate image data
 * \param top_y returned top y coordinate
 * \param bottom_y returned bottom y coordinate
 */
void platereader::VerticalCrop(
    int plate_image_width,
    int plate_image_height,
    unsigned char* number_plate,
    int &top_y,
    int &bottom_y)
{
    int max_crop = plate_image_height * 10 / 100;
    int min_occupancy = plate_image_width * 20 / 100;

    int start_y = top_y;
    top_y = 0;
    for (int y = start_y; y < start_y+max_crop; y++)
    {
    	int occupancy = 0;
    	for (int x = 0; x < plate_image_width; x++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] > 0) occupancy++;
    	}

    	if (occupancy < min_occupancy)
    	{
    		top_y = y;
    		break;
    	}
    }
    int end_y = bottom_y;
    for (int y = end_y; y > end_y - max_crop; y--)
    {
    	int occupancy = 0;
    	for (int x = 0; x < plate_image_width; x++)
    	{
    		int n = (y * plate_image_width) + x;
    		if (number_plate[n] > 0) occupancy++;
    	}

    	if (occupancy < min_occupancy)
    	{
    		bottom_y = y;
    		break;
    	}
    }
}

/*!
 * \brief Returns a set of images, one for each character
 * \param minimum_character_width_percent the minimum width of a character as a percentage of the plate image width
 * \param plate_image_width width of the number plate image
 * \param plate_image_height height of each number plate image
 * \param binary_images binarised number plate images
 * \param characters returned character images
 * \param characters_dimensions dimensions of each character image
 * \param characters_positions position of each character within the number plate image
 */
void platereader::SeparateCharacters(
	float minimum_character_width_percent,
	int plate_image_width,
	std::vector<int> plate_image_height,
    std::vector<unsigned char*> &binary_images,
    std::vector<std::vector<unsigned char*> > &characters,
    std::vector<std::vector<int> > &characters_dimensions,
    std::vector<std::vector<int> > &characters_positions)
{
	int* interval = new int[plate_image_width];
	int* separate = new int[plate_image_width];
	int minimum_character_width_pixels = (int)(plate_image_width * minimum_character_width_percent / 100);

    for (int p = 0; p < (int)binary_images.size(); p++)
    {
    	unsigned char* number_plate = binary_images[p];

    	int height = plate_image_height[p];

    	float aspect = plate_image_width / (float)height;

    	int no_of_segments = 1;
    	int vertical_separator_y = 0;
    	if (aspect < 2)
    	{
    		no_of_segments = 2;

    		// find the vertical separator
    		int ty = height * 30 / 100;
    		int by = height * 70 / 100;
    		int max_occupancy = 0;
    		for (int y = ty; y <= by; y++)
    		{
    			int occupancy = 0;
    			for (int x = 0; x < plate_image_width; x++)
    			{
    				int n = (y * plate_image_width) + x;
    				if (number_plate[n] == 0) occupancy++;
    			}
    			if (occupancy > max_occupancy)
    			{
    				max_occupancy = occupancy;
    				vertical_separator_y = y;
    			}
    		}
    	}

		std::vector<unsigned char*> chars;
		std::vector<int> chars_dimensions;
		std::vector<int> chars_positions;
		int initial_chars = 0;

    	for (int seg = 0; seg < no_of_segments; seg++)
    	{
			int top_y = 0;
			int bottom_y = height-1;

			if (no_of_segments > 1)
			{
			    if (seg == 0)
			    {
				    top_y = 0;
				    bottom_y = vertical_separator_y+1;
			    }
			    else
			    {
			    	top_y = vertical_separator_y;
			    	bottom_y = height-1;
			    }
			}

			float average_character_height = 0;
			float average_character_height_hits = 0;
			VerticalCrop(plate_image_width, height, number_plate, top_y, bottom_y);

			memset(interval, 0, plate_image_width * sizeof(int));
			memset(separate, 0, plate_image_width * sizeof(int));

			for (int x = 0; x < plate_image_width; x++)
			{
				for (int y = top_y; y <= bottom_y; y++)
				{
					int n = (y * plate_image_width) + x;
					if (number_plate[n] == 0) interval[x]++;
				}
			}

			int* histogram = new int[bottom_y-top_y+2];
			memset(histogram, 0, (bottom_y-top_y+2) * sizeof(int));
			int* histogram_buffer = new int[bottom_y-top_y+2];
			float MeanDark = 0;
			float MeanLight = 0;
			float DarkRatio = 0;
			for (int x = 0; x < plate_image_width; x++) histogram[interval[x]]++;
			thresholding::GetGlobalThreshold(histogram, bottom_y-top_y+2, 0, histogram_buffer, MeanDark, MeanLight, DarkRatio);
			delete[] histogram;
			delete[] histogram_buffer;

			//int max = (int)(MeanLight * 110/100);
			int max = (int)(MeanLight * 100/100);
			for (int x = 0; x < plate_image_width; x++)
			{
				if (interval[x] > max) separate[x] = 1;
			}

			// get the average character width
			int prev_x = 0;
			float average_character_width = 0;
			int hits = 0;
			for (int x = 1; x < plate_image_width; x++)
			{
				int xx = x;
				if (separate[xx])
				{
					if (xx - prev_x > minimum_character_width_pixels)
					{
						average_character_width += xx - prev_x;
						hits++;
					}
					prev_x = x;
				}
			}
			if (hits > 0) average_character_width /= hits;

			// lift and separate
			int tx=0,ty=0,bx=0,by=0;
			prev_x = 0;
			for (int x = 1; x < plate_image_width; x++)
			{
				int xx = x;
				if ((separate[xx]) || (x == plate_image_width-1))
				{
					if (xx - prev_x > minimum_character_width_pixels)
					{
						int character_separator_x = 0;
						int number_of_characters = 1;
						if ((xx - prev_x > average_character_width * 1.5f) &&
							(x != plate_image_width-1))
						{
							number_of_characters = 2;

							// find the separation position
							tx = prev_x + ((xx - prev_x) * 20 / 100);
							bx = prev_x + ((xx - prev_x) * 80 / 100);
							int max_v = 0;
							// Australians wouldn't give an xxxx for anything else
							for (int xxxx = tx; xxxx < bx; xxxx++)
							{
								if (interval[xxxx] > max_v)
								{
									max_v = interval[xxxx];
									character_separator_x = xxxx;
								}
							}
						}

						for (int c = 0; c < number_of_characters; c++)
						{
							ty = top_y;
							by = bottom_y+1;
							if (number_of_characters == 1)
							{
								tx = prev_x;
								bx = xx;
							}
							else
							{
								if (c == 0)
								{
									tx = prev_x;
									bx = character_separator_x;
								}
								else
								{
									tx = character_separator_x;
									bx = xx;
								}
							}

							for (int t = 0; t < 2; t++)
								Trim(plate_image_width, height, number_plate, 5, tx, ty, bx, by);

							unsigned char *ch = new unsigned char[(bx-tx)*(by-ty)];
							int n = 0;
							for (int yyy = ty; yyy < by; yyy++)
							{
								for (int xxx = tx; xxx < bx; xxx++, n++)
								{
									int n2 = (yyy * plate_image_width) + xxx;
									ch[n] = number_plate[n2];
								}
							}

							Erode(bx-tx, by-ty, ch, 50);

							chars.push_back(ch);
							chars_dimensions.push_back(bx - tx);
							chars_dimensions.push_back(by - ty);
							chars_positions.push_back(tx + ((bx - tx)/2));
							chars_positions.push_back(ty + ((by - ty)/2));
							average_character_height += by - ty;
							average_character_height_hits++;
						}
					}
					prev_x = x;
				}
			}

			if (average_character_height_hits > 0) average_character_height /= average_character_height_hits;

			// check character heights
			for (int i = initial_chars; i < (int)chars.size(); i++)
			{
				int wdth = chars_dimensions[i * 2];
				int hght = chars_dimensions[(i * 2) + 1];
				if (hght > average_character_height*120/100)
				{
					int px = chars_positions[i*2];
					int py = chars_positions[(i*2)+1];

					unsigned char *ch1 = new unsigned char[wdth*hght];
					int n = 0;
					for (int y = 0; y < (int)average_character_height; y++)
					{
						for (int x = 0; x < wdth; x++, n++)
						{
							ch1[n] = chars[i][n];
						}
					}
					unsigned char *ch2 = new unsigned char[wdth*hght];
					n = 0;
					int n2 = (hght - (int)average_character_height) * wdth;
					for (int y = 0; y < (int)average_character_height; y++)
					{
						for (int x = 0; x < wdth; x++, n++, n2++)
						{
							ch2[n] = chars[i][n2];
						}
					}
					delete[] chars[i];
					chars[i] = ch1;
					chars_dimensions[(i*2)+1] = (int)average_character_height;
					chars_positions[(i*2)+1] = py - ((hght - (int)average_character_height)/2);
					chars.push_back(ch2);
					chars_dimensions.push_back(wdth);
					chars_dimensions.push_back((int)average_character_height);
					chars_positions.push_back(chars_positions[i*2]);
					chars_positions.push_back(py + ((hght - (int)average_character_height)/2));
				}
			}

			initial_chars = (int)chars.size();

    	}

    	characters.push_back(chars);
    	characters_dimensions.push_back(chars_dimensions);
    	characters_positions.push_back(chars_positions);
    }

    delete[] separate;
    delete[] interval;
}

/*!
 * \brief converts grey images for each number plate into binarised versions
 * \param plate_image_width width of the number plate image
 * \param plate_image_height height of each number plate image
 * \param plate_images number plate images
 * \param binary_images returned binary number plate images
 */
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
