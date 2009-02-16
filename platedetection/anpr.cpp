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

/*!
 * \brief returns a list of all filenames within a directory
 * \param dir directory
 * \param filenames list of filenames
 */
void anpr::GetFilesInDirectory(std::string dir, std::vector<std::string> &filenames)
{
	filenames.clear();

    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL)
    {
    	cout << "Error opening " << dir << endl;
    	return;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
    	std::string filename = dirp->d_name;
    	if (filename.size() > 3)
    	{
    		if ((filename.substr(filename.size()-3,3) == "bmp") ||
    		    (filename.substr(filename.size()-3,3) == "BMP"))
    		{
    		    //cout << "filename: " << filename << endl;
                filenames.push_back(filename);
    		}
    	}
    }
    std::sort(filenames.begin(), filenames.end());
    closedir(dp);
}

void anpr::ReadDirectory(
    std::string directory,
    std::vector<std::string> &numbers,
    bool save_characters,
    int model_image_width,
    int model_image_height,
    std::vector<float*> &models,
    float* average_model)
{
	int character_index = 0;

	std::vector<std::string> filenames;
	GetFilesInDirectory(directory, filenames);
	for (int i = 0; i < (int)filenames.size(); i++)
	{
		std::string filename = directory + "/" + filenames[i];

		cout << filename << "...";

        Bitmap *bmp = new Bitmap();
        if (bmp->FromFile(filename))
        {
            std::vector<polygon2D*> plates;
            std::vector<std::string> temp_numbers;
        	Read(bmp->Data,
        		 bmp->Width, bmp->Height,
        		 plates,
        		 temp_numbers,
        		 save_characters,
        		 character_index,
        	     model_image_width,
        	     model_image_height,
        	     models,
        	     average_model,
        	     "");

        	if ((int)temp_numbers.size() > 0)
        	{
        	    for (int p = 0; p < (int)temp_numbers.size(); p++)
        	    {
        	    	numbers.push_back(temp_numbers[p]);
        	    	cout << temp_numbers[p] << " ";
        	    }
        	    cout << endl;
        	}
        	else
        	{
        		cout << "No plates found" << endl;
        	}
        }

        delete bmp;
	}
}

void anpr::ReadFile(
    std::string filename,
	std::vector<polygon2D*> &plates,
    std::vector<std::string> &numbers,
    bool save_characters,
    int &character_index,
    int model_image_width,
    int model_image_height,
    std::vector<float*> &models,
    float* average_model,
    std::string plates_filename,
    std::string filtered_image_filename)
{
    Bitmap* bmp = new Bitmap();
    if (bmp->FromFile(filename))
    {
		Read(bmp->Data,
			 bmp->Width,
			 bmp->Height,
			 plates,
			 numbers,
			 save_characters,
			 character_index,
			 model_image_width,
			 model_image_height,
			 models,
			 average_model,
			 filtered_image_filename);

		if (((int)plates.size() > 0) && (plates_filename != ""))
		{
		    for (int i = 0; i < (int)plates.size(); i++)
		    {
		    	polygon2D* poly = plates[i];
		        poly->show(bmp->Data, bmp->Width, bmp->Height, 0, 255, 0, 0);
		    }
		    bmp->SavePPM(plates_filename.c_str());
		}
    }
    else
    {
    	cout << "Cannot load " << filename << endl;
    }

    delete bmp;
}

void anpr::Read(
    unsigned char* img_colour,
    int img_width,
    int img_height,
    std::vector<polygon2D*> &plates,
    std::vector<std::string> &numbers,
    bool save_characters,
    int &character_index,
    int model_image_width,
    int model_image_height,
    std::vector<float*> &models,
    float* average_model,
    std::string filtered_image_filename)
{
    bool debug = false;
    std::vector<unsigned char*> debug_images;
    int debug_image_width = 0;
    int debug_image_height = 0;

    platedetection::Find(
        img_colour,
        img_width, img_height,
        plates,
        debug,
        debug_images,
        debug_image_width,
        debug_image_height,
        filtered_image_filename);

    cout << "plates: " << (int)plates.size() << endl;

    std::vector<unsigned char*> plate_images;
    std::vector<unsigned char*> binary_images;
    if ((int)plates.size() > 0)
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
	    	std::vector<int> chars_positions = characters_positions[p];
	    	std::vector<int> chars_dimensions = characters_dimensions[p];

	    	platereader::RemoveStragglers(
	    	    chars_dimensions,
	    	    chars_positions,
	    	    chars);

	    	// resample to a fixed resolution
	    	int resampled_width = 20;
	    	int resampled_height = 20;
	    	platereader::Resample(
	    	    chars_dimensions,
	            chars,
	    	    resampled_width,
	    	    resampled_height,
	            resampled_chars);

	    	if (save_characters)
	    	{
		    	for (int c = 0; c < (int)resampled_chars.size(); c++)
		    	{
					std::string char_filename = "";
					std::stringstream s_char_filename;
					s_char_filename << "char" << character_index << ".ppm";
					s_char_filename >> char_filename;

					cout << "Saving " << char_filename << endl;

					Bitmap *bmp_plate = new Bitmap(resampled_chars[c], resampled_width, resampled_height, 1);
					//bmp_plate->Save(char_filename.c_str());
					bmp_plate->SavePPM(char_filename.c_str());
					delete bmp_plate;
					character_index++;
		    	}
	    	}

	    	// recognise chars
	    	std::string plate_number = "";
	    	if ((int)models.size() > 0)
	    	{
	    		std::string plate_number =
	    			ocr::RecognizeCharacters(
	                    resampled_width,
	    		        resampled_height,
	    		        resampled_chars,
	    		        models,
	    		        average_model);
	    	}

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
/*
    for (int i = 0; i < (int)plates.size(); i++)
    {
    	delete plates[i];
    	plates[i] = NULL;
    }
*/
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

