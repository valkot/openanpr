/*
    ANPR
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

#include "anpr.h"

void anpr::Read(
    unsigned char* img_colour,
    int img_width,
    int img_height,
    std::vector<polygon2D*> &plates,
    std::vector<std::string> &numbers)
{
    bool debug = false;
    std::vector<unsigned char*> debug_images;
    int debug_image_width = 0;
    int debug_image_height = 0;

    bool plates_found = platedetection::Find(
        img_colour,
        img_width, img_height,
        plates,
        debug,
        debug_images,
        debug_image_width,
        debug_image_height);

    std::vector<unsigned char*> plate_images;
    std::vector<unsigned char*> binary_images;
    if (plates.size() > 0)
    {
        int plate_image_width = 200;
        std::vector<int> plate_image_height;
	    platedetection::ExtractPlateImages(
	        img_colour,
	        img_width, img_height,
	        plates,
	        plate_image_width,
	        plate_image_height,
	        plate_images);

	    platereader::Binarise(
	    	plate_image_width,
	    	plate_image_height,
	        plate_images,
	        binary_images);

	    float minimum_character_width_percent = 2.5f;
	    std::vector<std::vector<unsigned char*> > characters;
	    std::vector<std::vector<int> > characters_dimensions;
	    std::vector<std::vector<int> > characters_positions;
	    platereader::SeparateCharacters(
	    	minimum_character_width_percent,
	    	plate_image_width,
	    	plate_image_height,
	        binary_images,
	        characters,
	        characters_dimensions,
	        characters_positions);

	    for (int p = 0; p < (int)plates.size(); p++)
	    {
	    	std::vector<unsigned char*> chars = characters[p];
	    	std::vector<unsigned char*> resampled_chars;
	    	std::vector<int> chars_dimensions = characters_dimensions[p];

	    	// resample to a fixed resolution
	    	int resampled_width = 32;
	    	int resampled_height = 32;
	    	platereader::Resample(chars_dimensions,
	                 chars,
	    	         resampled_width,
	    	         resampled_height,
	                 resampled_chars);

	    	//TODO: recognise chars

	    	std::string plate_number = "";
	    	numbers.push_back(plate_number);

	    	for (int c = 0; c < (int)resampled_chars.size(); c++)
	    	{
	    		delete[] resampled_chars[c];
	    		resampled_chars[c] = NULL;
	    	}

	    	for (int c = 0; c < (int)chars.size(); c++)
	    	{
	    		delete[] chars[c];
	    		chars[c] = NULL;
	    	}
	    }

    }

    for (int i = 0; i < (int)plates.size(); i++)
    {
    	delete plates[i];
    	plates[i] = NULL;
    }

    for (int i = 0; i < (int)plate_images.size(); i++)
    {
    	delete[] plate_images[i];
    	plate_images[i] = NULL;
    }

    for (int i = 0; i < (int)binary_images.size(); i++)
    {
    	delete[] binary_images[i];
    	binary_images[i] = NULL;
    }

}

