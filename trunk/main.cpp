/*
    Automatic Number Plate Recognition (ANPR) utility
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

#include <algorithm>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <errno.h>

#include "anyoption.h"
#include "utils/Image.h"
#include "RawImage1.h"
#include "RawImage2.h"
#include "RawImage3.h"
#include "RawImage4.h"
#include "edgedetection/CannyEdgeDetector.h"
#include "cppunitlite/TestHarness.h"
#include "cppunitlite/TestResultStdErr.h"
#include "UnitTests.h"
#include "utils/bitmap.h"
#include "platedetection/ocr.h"

using namespace std;

/*!
 * \brief returns true if the given file exists
 * \param filename name of the file
 */
bool FileExists(char* filename)
{
    bool flag = false;
    fstream fin;
    cout << "file exists: " << filename << endl;
    fin.open(filename, ios::in);
    if (fin.is_open())
        flag = true;
    fin.close();
    return(flag);
}

/*!
 * \brief returns true if the given file exists
 * \param filename name of the file
 */
bool FileExists(std::string filename)
{
    ifstream inf;

    bool flag = false;
    inf.open(filename.c_str());
    if (inf.good()) flag = true;
    inf.close();
    return(flag);
}


/*!
 * \brief returns a list of all filenames within a directory
 * \param dir directory
 * \param filenames list of filenames
 */
void GetFilesInDirectory(std::string dir, std::vector<std::string> &filenames)
{
	filenames.clear();

    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL)
    {
    	cout << "Error " << errno << " opening " << dir << endl;
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

/*!
 * \brief attempt to detect a number plate
 * \param filename filename
 * \param minimum_volume_percent minimum volume of the license plate as a percent of the image volume
 * \param maximum_volume_percent maximum volume of the license plate as a percent of the image volume
 * \param debug save extra debugging data
 * \return license plate text
 */
std::string Detect(
    std::string filename,
    int minimum_volume_percent,
    int maximum_volume_percent,
    bool debug)
{
    std::string license_plate_text = "";

    if (FileExists(filename))
    {
        // create a bitmap object
        Bitmap *bmp = new Bitmap();
        if (bmp->FromFile(filename))
        {
	    if ((bmp->Width < 10000) && (bmp->Width > 0) &&
		(bmp->Height < 10000) && (bmp->Height > 0))
	    {
  	        std::vector<unsigned char*> debug_images;
			int debug_image_width = 0;
			int debug_image_height = 0;

		unsigned char* img_mono = bmp->Data;
		if (bmp->bytes_per_pixel > 1)
		{
		    // convert from colour to mono
		    img_mono = new unsigned char[bmp->Width * bmp->Height];
		    int n = (bmp->Width * bmp->Height) - 1;
		    for (int i = (bmp->Width * bmp->Height*3) - 3; i >= 0; i -= 3, n--)
			 img_mono[n] = (unsigned char)(((int)bmp->Data[i] + (int)bmp->Data[i + 1] + (int)bmp->Data[i + 2]) / 3);
		}

		// TODO: detect plate
		license_plate_text = "";


		// save the debug images
		std::string debug_filename;
		for (unsigned int i = 0; i < debug_images.size(); i++)
		{
		    // make a file name
		    debug_filename = "";
		    std::stringstream s_debug_filename;
		    s_debug_filename << "debug_" << i << ".ppm";
		    s_debug_filename >> debug_filename;

		    // create a bitmap object
		    Bitmap *bmp_debug = new Bitmap(debug_images[i], debug_image_width, debug_image_height, 3);

		    // save the bitmap object
		    printf("Saving debug image %s\n", debug_filename.c_str());
		    bmp_debug->SavePPM(debug_filename.c_str());

		    delete bmp_debug;
		}

		if (license_plate_text != "")
		{
		    cout << license_plate_text << endl;
		}

		if (img_mono != bmp->Data) delete[] img_mono;
		delete bmp;
	    }
	    else
	    {
	        cout << "Error reading image file" << endl;
	    }
        }
    }
    else
    {
   		cout << "File not found" << endl;
    }
    return(license_plate_text);
}


/*!
 * \brief run all unit tests
 */
void RunUnitTests()
{
    TestResultStdErr result;
    TestRegistry::runAllTests(result);
}


void Run( int argc, char* argv[], float VERSION )
{
    AnyOption *opt = new AnyOption();

    // help
    opt->addUsage( "" );
    opt->addUsage( "Usage: " );
    opt->addUsage( "" );
    opt->addUsage( " -h  --help                 Prints this help " );
    opt->addUsage( " -f  --filename img1.bmp    Image file to be analysed " );
    opt->addUsage( " -d  --dir                  Directory containing images to be analysed " );
    opt->addUsage( "     --minvol <value>       Minimum volume of the license plate as a % of the image " );
    opt->addUsage( "     --maxvol <value>       Maximum volume of the license plate as a % of the image " );
    opt->addUsage( "     --test                 Run unit tests " );
    opt->addUsage( "     --debug                Save debugging info " );
    opt->addUsage( " -c  --chars                Save characters " );
    opt->addUsage( " -m  --model <filename>     Use the given character model " );
    opt->addUsage( " -l  --learn <directory>    Learn character model " );
    opt->addUsage( " -v  --version              Shows the version number " );
    opt->addUsage( "" );

    opt->setFlag(  "help", 'h' );       // a flag (takes no argument), supporting long and short form
    opt->setOption(  "filename", 'f' ); // an option (takes an argument), filename to search
    opt->setOption(  "dir", 'd' );      // an option (takes an argument), directory to search
    opt->setOption(  "minvol" );        // minimum volume of the license plate as a percent of the image volume
    opt->setOption(  "maxvol" );        // maximum volume of the license plate as a percent of the image volume
    opt->setFlag(  "test", 't' );       // a flag (takes no argument) used to run unit tests
    opt->setFlag(  "debug" );           // a flag (takes no argument) used to save debugging images
    opt->setFlag(  "chars", 'c' );
    opt->setOption(  "model", 'm' );
    opt->setOption(  "learn", 'l' );
    opt->setFlag(  "version", 'v' );

    opt->processCommandArgs(argc, argv);

    if( ! opt->hasOptions())
    {
        // print usage if no options
        opt->printUsage();
        delete opt;
        return;
    }

    if( opt->getFlag( "version" ) || opt->getFlag( 'v' ) )
    {
        printf("Version %f\n", VERSION);
        return;
    }

    bool save_characters = false;
    if( opt->getFlag( "chars" ) || opt->getFlag( 'c' ) )
    {
    	save_characters = true;
    }

    if( opt->getFlag( "help" ) || opt->getFlag( 'h' ) )
        opt->printUsage();

    if( opt->getFlag( "test" ) || opt->getFlag( 't' ) )
        RunUnitTests();

    bool debug = false;
    if( opt->getFlag( "debug" ) )
        debug = true;

    int minimum_volume_percent = 10;
    if( opt->getValue( "minvol" ) != NULL  )
    {
    	minimum_volume_percent = atoi(opt->getValue("minvol"));
        if (minimum_volume_percent < 5) minimum_volume_percent = 5;
    }
    int maximum_volume_percent = 100;
    if( opt->getValue( "maxvol" ) != NULL  )
    {
    	maximum_volume_percent = atoi(opt->getValue("maxvol"));
        if (maximum_volume_percent > 100) maximum_volume_percent = 100;
    }

	int model_image_width = 20;
	int model_image_height = 20;
    float* average_model = new float[model_image_width * model_image_height];
	std::vector<float*> models;

    if( opt->getValue( "model" ) != NULL || opt->getValue( 'm' ) != NULL  )
    {
    	std::string filename = opt->getValue("model");
    	ocr::LoadCharacterModels(filename, model_image_width, model_image_height, models, average_model);
    }

    if( opt->getValue( "learn" ) != NULL || opt->getValue( 'l' ) != NULL  )
    {
    	std::string directory = opt->getValue("learn");
    	std::vector<std::string> numbers;

    	ocr::CreateCharacterModels(directory, model_image_width, model_image_height, models);
    	if ((int)models.size() == 36)
    	{
    	    ocr::CreateCharacterEigenModels(model_image_width,model_image_height,models, average_model);
    	    ocr::SaveCharacterModels("model.dat", model_image_width, model_image_height, models, average_model);

    	    cout << "Model saved: " << endl;
    	}
    	else
    	{
    		cout << "Incorrect number of models: " << (int)models.size() << endl;
    	}
    }

    if( opt->getValue( 'f' ) != NULL  || opt->getValue( "filename" ) != NULL  )
    {

    	std::string filename = opt->getValue('f');

    	//cout << "filename: " << filename << endl;
    	std::vector<std::string> numbers;
    	std::vector<polygon2D*> plates;

    	int character_index = 0;
    	anpr::ReadFile(
    	    filename,
    		plates,
    	    numbers,
    	    save_characters,
    	    character_index,
    	    model_image_width,
    	    model_image_height,
    	    models,
    	    average_model,
    	    "number_plates.ppm",
    	    "filtered_image.ppm");

    	for (int i = 0; i < (int)plates.size(); i++)
    	{
    		delete plates[i];
    		plates[i] = NULL;
    	}
    }


    if( opt->getValue( "dir" ) != NULL || opt->getValue( 'd' ) != NULL  )
    {
    	std::string directory = opt->getValue("dir");
    	std::vector<std::string> numbers;
    	anpr::ReadDirectory(directory, numbers, save_characters, model_image_width, model_image_height, models, average_model);
    }

    for (int i = 0; i < (int)models.size(); i++)
    {
    	delete[] models[i];
    	models[i] = NULL;
    }
    delete[] average_model;
    delete opt;
}

int main( int argc, char* argv[] )
{
    const float VERSION = 0.01f;

    Run(argc, argv, VERSION);

    // Uncomment this if you want to run valgrind on the unit tests
    //RunUnitTests();

    return EXIT_SUCCESS;
}
