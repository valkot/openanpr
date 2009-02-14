#ifndef unit_tests_h
#define unit_tests_h

// enable testing at different levels
// comment out as necessary
#define TEST_LOW_LEVEL
#define TEST_MEDIUM_LEVEL
#define TEST_HIGH_LEVEL
//#define MEMORYTEST

#include <omp.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
//#include <time.h>

#include "edgedetection/CannyEdgeDetector.h"
#include "fft/fft.h"
#include "hypergraph/hypergraph.h"
#include "shapes/shapes.h"
#include "utils/bitmap.h"
#include "utils/polygon.h"
#include "utils/processimage.h"
#include "platedetection/platedetection.h"

#include "cppunitlite/TestHarness.h"

#define DEBUG 1

// -----------------------------------------------------------------------------------------
// low level unit tests
// -----------------------------------------------------------------------------------------

#ifdef TEST_LOW_LEVEL

TEST (FindPlatesTest, MyTest)
{
	unsigned char* test_image = raw_image4;

    // image data
    Image image;
    image.Width = 640;
    image.Height = 480;
    image.Data = test_image;
    image.BytesPerPixel = 3;

    bool debug = false;
    std::vector<polygon2D*> plates;
    std::vector<unsigned char*> debug_images;
    int debug_image_width = 0;
    int debug_image_height = 0;

    bool plates_found = platedetection::Find(
        image.Data,
        image.Width, image.Height,
        plates,
        debug,
        debug_images,
        debug_image_width,
        debug_image_height);

    CHECK(plates_found == true);

    // save the debug images
    std::string debug_filename;
    for (unsigned int i = 0; i < debug_images.size(); i++)
    {
        // make a file name
        debug_filename = "";
        std::stringstream s_debug_filename;
        s_debug_filename << "debug_plates_" << i << ".ppm";
        s_debug_filename >> debug_filename;

        // create a bitmap object
        Bitmap *bmp_debug1 = new Bitmap(debug_images[i], debug_image_width, debug_image_height, 3);

        // save the bitmap object
        printf("Saving debug image %s\n", debug_filename.c_str());
        bmp_debug1->SavePPM(debug_filename.c_str());

        delete bmp_debug1;
    }

    if ((int)plates.size() == 0)
        printf("No number plates were detected\n");

    if (plates.size() > 0)
    {
        int plate_image_width = 200;
        std::vector<int> plate_image_height;
        std::vector<unsigned char*> plate_images;
	    platedetection::ExtractPlateImages(
	        image.Data,
	        image.Width, image.Height,
	        plates,
	        plate_image_width,
	        plate_image_height,
	        plate_images);

	    for (int p = 0; p < (int)plates.size(); p++)
	    {
	        std::string plate_filename = "";
	        std::stringstream s_plate_filename;
	        s_plate_filename << "plate_" << p << ".ppm";
	        s_plate_filename >> plate_filename;

	        Bitmap *bmp_plate = new Bitmap(plate_images[p], plate_image_width, plate_image_height[p], 1);
	        bmp_plate->SavePPM(plate_filename.c_str());
            delete bmp_plate;
	    }
    }

    unsigned char* rectangles_img = new unsigned char[image.Width * image.Height * 3];
    memcpy(rectangles_img, test_image, image.Width * image.Height * 3);
    for (int i = 0; i < (int)plates.size(); i++)
    {
    	polygon2D* poly = plates[i];
        poly->show(rectangles_img, image.Width, image.Height, 0, 255, 0, 0);
    }

    Bitmap *bmp = new Bitmap(rectangles_img, image.Width, image.Height, 3);
    bmp->SavePPM("number_plates.ppm");

    for (int i = 0; i < (int)plates.size(); i++)
    {
    	delete plates[i];
    	plates[i] = NULL;
    }

    delete bmp;
    delete[] rectangles_img;
}



TEST (rectanglesTest, MyTest)
{
	unsigned char* test_image = raw_image1;

    // image data
    Image image;
    image.Width = 640;
    image.Height = 480;
    image.Data = test_image;
    image.BytesPerPixel = 3;

	unsigned char* filtered = new unsigned char[image.Width * image.Height * 3];
	platedetection::ColourFilter(image.Data, image.Width, image.Height, filtered);

	unsigned char* mono_img = new unsigned char[image.Width * image.Height];
	for (int i = 0; i < (int)(image.Width * image.Height); i++)
	{
		mono_img[i] = filtered[i*3 + 1];
	}

	image.Data = mono_img;
	image.BytesPerPixel = 1;

	CannyEdgeDetector *edge_detector = new CannyEdgeDetector();

    // to save debugging images set this to true
    bool debug = false;

    // ideal positions of the vertices within the image
    float ideal_vertices[] = { 222.0117, 104.7593,
                               414.0189, 109.8795,
                               416.9897, 300.0131,
                               222.0117, 300.0131 };

    float maximum_aspect_ratio = 200.0f / 33.0f;
    int maximum_groups = 9999;
    int bestfit_tries = 3;
    int step_sizes[] = { 12, 4 };
    int no_of_step_sizes = 2;
    int grouping_radius_percent[] = { 0, 10, 40, 60 };
    int grouping_radius_percent_levels = 4;
    int erosion_dilation[] = { 1, 3 };
    int erosion_dilation_levels = 2;
    int accuracy_level = 0;
    int perimeter_detection_method = 1;
    int compression[] = { 7000, 6000, 5000 };
    int no_of_compressions = 3;
    int minimum_volume_percent = 3;
    int maximum_volume_percent = 20;
    bool use_perimeter_fitting = true;
    int perimeter_fit_threshold = 120;
    std::vector<int> edges;
    std::vector<float> orientation;
    std::vector<std::vector<int> > dominant_edges;
    std::vector<std::vector<std::vector<int> > > side_edges;
    unsigned char* edges_image = new unsigned char[image.Width * image.Height];
    int edges_image_width = 0;
    int edges_image_height = 0;
    std::vector<polygon2D*> rectangles;
    std::vector<unsigned char*> debug_images;
    unsigned char* erosion_dilation_buffer = new unsigned char[image.Width * image.Height];
    int* downsampling_buffer0 = new int[image.Width * image.Height];
    int* downsampling_buffer1 = new int[image.Width * image.Height];

    shapes::DetectRectangles(
        mono_img, image.Width, image.Height, 1,
    	grouping_radius_percent,
    	grouping_radius_percent_levels,
    	erosion_dilation,
    	erosion_dilation_levels,
    	false,
    	accuracy_level,
    	maximum_aspect_ratio,
    	debug,
    	0,
    	perimeter_detection_method,
    	compression,
    	no_of_compressions,
    	minimum_volume_percent,
    	maximum_volume_percent,
    	use_perimeter_fitting,
    	perimeter_fit_threshold,
    	bestfit_tries,
    	step_sizes,
    	no_of_step_sizes,
    	maximum_groups,
    	edges,
    	orientation,
    	dominant_edges,
    	side_edges,
    	edges_image,
    	edges_image_width,
    	edges_image_height,
    	edge_detector,
    	rectangles,
    	debug_images,
    	erosion_dilation_buffer,
    	downsampling_buffer0,
    	downsampling_buffer1);

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
        Bitmap *bmp_debug1 = new Bitmap(debug_images[i], edges_image_width, edges_image_height, 3);

        // save the bitmap object
        printf("Saving debug image %s\n", debug_filename.c_str());
        bmp_debug1->SavePPM(debug_filename.c_str());

        delete bmp_debug1;
    }

    if ((int)rectangles.size() == 0)
        printf("No rectangular regions were detected\n");


    unsigned char* rectangles_img = new unsigned char[image.Width * image.Height * 3];
    memcpy(rectangles_img, test_image, image.Width * image.Height * 3);
    for (int i = 0; i < (int)rectangles.size(); i++)
    {
    	polygon2D* poly = rectangles[i];
        poly->show(rectangles_img, image.Width, image.Height, 0, 255, 0, 0);
    }


    Bitmap *bmp = new Bitmap(rectangles_img, image.Width, image.Height, 3);
    bmp->SavePPM("rectangles.ppm");
    delete bmp;

    bmp = new Bitmap(mono_img, image.Width, image.Height, 1);
    bmp->SavePPM("mono.ppm");
    delete bmp;

    delete[] rectangles_img;
	delete edge_detector;
	delete[] mono_img;
	delete[] filtered;
    delete[] edges_image;
    delete[] erosion_dilation_buffer;
    delete[] downsampling_buffer0;
    delete[] downsampling_buffer1;

}

TEST (edgesTest, MyTest)
{
	int itterations = 20;
	double totalTime = 0;
	Image image;
	image.Height = 480;
	image.Width = 640;
	image.Data = raw_image4;
	image.BytesPerPixel = 3;
	timeval start,end,result;

	unsigned char* filtered = new unsigned char[image.Width * image.Height * 3];
	platedetection::ColourFilter(image.Data, image.Width, image.Height, filtered);

	unsigned char* mono_img = new unsigned char[image.Width * image.Height];
	for (int i = 0; i < (int)(image.Width * image.Height); i++)
	{
		mono_img[i] = filtered[i*3];
	}

	image.Data = mono_img;
	image.BytesPerPixel = 1;

	CannyEdgeDetector *edge_detector = new CannyEdgeDetector();
	edge_detector->Update(image);

	gettimeofday(&start, NULL);
	for(int i = 0; i < itterations; i++)
	{
		edge_detector->Update(image);
		printf(".");
	}
	gettimeofday(&end, NULL);

	timersub(&end,&start,&result);
    totalTime = result.tv_sec + (double) result.tv_usec/1000000;
    double average_time = (totalTime/itterations)*1000;
	printf("\nTime elapsed: %f Sec.\nAverage update time: %f mSec.\n",totalTime,average_time);

    //CHECK(average_time < 500);

    Bitmap *bmp = new Bitmap(edge_detector->edgesImage, image.Width, image.Height, image.BytesPerPixel);

    bmp->SavePPM("canny_edges.ppm");

    delete bmp;
	delete edge_detector;
	delete[] mono_img;
	delete[] filtered;
}


TEST (ColourFilterTest, MyTest)
{
	Image image;
	image.Height = 480;
	image.Width = 640;
	image.Data = raw_image4;
	image.BytesPerPixel = 3;

	unsigned char* filtered = new unsigned char[image.Width * image.Height * 3];
	platedetection::ColourFilter(image.Data, image.Width, image.Height, filtered);

    Bitmap *bmp = new Bitmap(filtered, image.Width, image.Height, 3);

    bmp->SavePPM("colourfilter.ppm");

    delete bmp;
    delete[] filtered;
}


TEST (detectLinesTest, MyTest)
{
    int img_width = 640;
    int img_height = 480;
    int no_of_edges = 150;
    std::vector<int> edges;

    int no_of_lines = 2;
    float ideal_x0[] = { 205, 432 };
    float ideal_y0[] = { 113, 54 };
    float ideal_x1[] = { 501, 111 };
    float ideal_y1[] = { 424, 450 };
    float* gradient = new float[no_of_lines];
    float ideal_xx0[] = { 0, 0 };
    float ideal_yy0[] = { 0, 0 };
    float ideal_xx1[] = { 0, 0 };
    float ideal_yy1[] = { 0, 0 };
    for (int i = 0; i < no_of_lines; i++)
    {
        float dx = ideal_x1[i] - ideal_x0[i];
        float dy = ideal_y1[i] - ideal_y0[i];
        gradient[i] = dy / dx;
    	geometry::intersection(ideal_x0[i], ideal_y0[i], ideal_x1[i], ideal_y1[i], 0,0,100,0, ideal_xx0[i], ideal_yy0[i]);
    	geometry::intersection(ideal_x0[i], ideal_y0[i], ideal_x1[i], ideal_y1[i], 0,img_height-1,100,img_height-1, ideal_xx1[i], ideal_yy1[i]);
    }
    float noise = 5;

    unsigned char* img = new unsigned char[img_width*img_height*3];
    for (int i = 0; i < img_width * img_height * 3; i++) img[i] = 255;

    for (int i = 0; i < no_of_edges; i++)
    {
    	int edge_x = rand() % img_width;
    	int edge_y = rand() % img_height;

    	if (rand() % 100 > 50)
    	{
    		int idx = rand() % no_of_lines;
    		int x_diff = (int)(ideal_x1[idx] - ideal_x0[idx]);
    		edge_x = ideal_x0[idx] + ( (rand() % ABS(x_diff)) * x_diff / ABS(x_diff));
    	    edge_y = ideal_y0[idx] + ((edge_x - ideal_x0[idx]) * gradient[idx]);

    	    // add some noise
    	    edge_x += ((rand() % 1000) * noise / 1000) - (noise * 0.5f);
    	    edge_y += ((rand() % 1000) * noise / 1000) - (noise * 0.5f);
    	}

    	edges.push_back(edge_x);
    	edges.push_back(edge_y);
    }

    std::vector<std::vector<float> > detected_lines;

    int no_of_samples = 100;
    int no_of_edge_samples = 100;
    int minimum_edges_per_line = 20;
    shapes::DetectLines(edges, (int)noise,
    		            detected_lines,
    		            no_of_samples,
    		            no_of_edge_samples,
    		            minimum_edges_per_line,
    		            0);

    CHECK((int)edges.size() == no_of_edges*2);
    for (int i = 0; i < (int)edges.size(); i += 2)
    {
    	// draw a circle
    	drawing::drawSpot(img, img_width, img_height, edges[i], edges[i+1], 1, 0,0,0);
    }

    for (int i = 0; i < (int)detected_lines.size(); i++)
    {
        float detected_x0 = detected_lines[i][0];
        float detected_y0 = detected_lines[i][1];
        float detected_x1 = detected_lines[i][2];
        float detected_y1 = detected_lines[i][3];
        if (detected_x0 != 9999)
        {
        	float detected_xx0 = 0;
        	float detected_yy0 = 0;
        	float detected_xx1 = 0;
        	float detected_yy1 = 0;
    	    geometry::intersection(detected_x0, detected_y0, detected_x1, detected_y1, 0,0,100,0, detected_xx0, detected_yy0);
    	    geometry::intersection(detected_x0, detected_y0, detected_x1, detected_y1, 0,img_height-1,100,img_height-1, detected_xx1, detected_yy1);

    	    drawing::drawLine(img, img_width, img_height,
            		          (int)detected_xx0, (int)detected_yy0,
            		          (int)detected_xx1, (int)detected_yy1,
            		          0, 255, 0,
            		          0, false);
        }
    }

    string debug_filename = "detectedlines.ppm";
    Bitmap *bmp_debug = new Bitmap(img, img_width, img_height, 3);
    bmp_debug->SavePPM(debug_filename.c_str());
    delete bmp_debug;
    delete[] img;
    delete[] gradient;
}


TEST (bestFitLineRANSACTest, MyTest)
{
    int img_width = 640;
    int img_height = 480;
    int no_of_edges = 100;
    std::vector<int> edges;

    float ideal_x0 = 205;
    float ideal_y0 = 113;
    float ideal_x1 = 501;
    float ideal_y1 = 424;
    float dx = ideal_x1 - ideal_x0;
    float dy = ideal_y1 - ideal_y0;
    float gradient = dy / dx;
    float noise = 5;

    float ideal_xx0 = 0;
    float ideal_yy0 = 0;
    float ideal_xx1 = 0;
    float ideal_yy1 = 0;
    geometry::intersection(ideal_x0, ideal_y0, ideal_x1, ideal_y1, 0,0,100,0, ideal_xx0, ideal_yy0);
    geometry::intersection(ideal_x0, ideal_y0, ideal_x1, ideal_y1, 0,img_height-1,100,img_height-1, ideal_xx1, ideal_yy1);

    unsigned char* img = new unsigned char[img_width*img_height*3];
    for (int i = 0; i < img_width * img_height * 3; i++) img[i] = 255;

    for (int i = 0; i < no_of_edges; i++)
    {
    	int edge_x = rand() % img_width;
    	int edge_y = rand() % img_height;

    	if (rand() % 100 > 50)
    	{
    	    edge_y = ideal_y0 + ((edge_x - ideal_x0) * gradient);

    	    // add some noise
    	    edge_x += ((rand() % 1000) * noise / 1000) - (noise * 0.5f);
    	    edge_y += ((rand() % 1000) * noise / 1000) - (noise * 0.5f);
    	}

    	edges.push_back(edge_x);
    	edges.push_back(edge_y);
    }

    float detected_x0 = 0;
    float detected_y0 = 0;
    float detected_x1 = 0;
    float detected_y1 = 0;
    int no_of_samples = 100;
    int no_of_edge_samples = 100;
    bool remove_edges = false;
    shapes::BestFitLineRANSAC(edges, (int)noise,
    		                  detected_x0, detected_y0,
    		                  detected_x1, detected_y1,
    		                  no_of_samples,
    		                  no_of_edge_samples,
    		                  remove_edges);

    for (int i = 0; i < (int)edges.size(); i += 2)
    {
    	// draw a circle
    	drawing::drawSpot(img, img_width, img_height, edges[i], edges[i+1], 1, 0,0,0);
    }

    CHECK(detected_x0 != 9999);
	float detected_xx0 = 0;
	float detected_yy0 = 0;
	float detected_xx1 = 0;
	float detected_yy1 = 0;
    if (detected_x0 != 9999)
    {
    	geometry::intersection(detected_x0, detected_y0, detected_x1, detected_y1, 0,0,100,0, detected_xx0, detected_yy0);
    	geometry::intersection(detected_x0, detected_y0, detected_x1, detected_y1, 0,img_height-1,100,img_height-1, detected_xx1, detected_yy1);

        drawing::drawLine(img, img_width, img_height,
        		          (int)detected_xx0, (int)detected_yy0,
        		          (int)detected_xx1, (int)detected_yy1,
        		          0, 255, 0,
        		          0, false);
    }

    string debug_filename = "bestfitline.ppm";
    Bitmap *bmp_debug = new Bitmap(img, img_width, img_height, 3);
    bmp_debug->SavePPM(debug_filename.c_str());
    delete bmp_debug;
    delete[] img;

    CHECK(ABS(detected_xx0 - ideal_xx0) < 4);
    CHECK(ABS(detected_xx1 - ideal_xx1) < 4);
}



TEST (geometryTest, MyTest)
{
	float x0 = 0;
	float y0 = 500;
	float x1 = 1000;
	float y1 = y0;

	float x2 = 400;
	float y2 = 0;
	float x3 = x2;
	float y3 = 1000;

	float ix=0, iy=0;
	geometry::intersection(x0,y0,x1,y1, x2,y2,x3,y3, ix, iy);

	CHECK_FLOATS_EQUAL(400, ix);
	CHECK_FLOATS_EQUAL(500, iy);

    x0 = 0;
    y0 = 500;
    x1 = 1000;
    y1 = 500;
    x2 = 500;
    y2 = 200;
    float dist_from_line = geometry::pointDistanceFromLine(x0,y0,x1,y1,x2,y2, x3,y3);
    CHECK_FLOATS_EQUAL(300, dist_from_line);
}

TEST (polygon2DTest, MyTest)
{
	polygon2D *p = new polygon2D();
	CHECK(p != NULL);

	p->Add(0,0);
	p->Add(100,0);
	p->Add(100,100);
	p->Add(0,100);

	CHECK(p->x_points.size() == 4);

	float side_length = p->getSideLength(0);
	CHECK_FLOATS_EQUAL(100, side_length);

	delete p;
}

TEST (hypergraphTest, MyTest)
{
	hypergraph *hype = new hypergraph();
	CHECK(hype != NULL);

	for (int i = 0; i < 10; i++)
	{
	    hypergraph_node *node = new hypergraph_node(1);
	    node->ID = i;
	    hype->Add(node);
    }
    CHECK_INTS_EQUAL(10, hype->Nodes.size());

	for (int i = 0; i < 10; i++)
	{
  	    for (int j = 0; j < 10; j++)
	    {
		    if (i != j) hype->LinkByIndex(j, i);
	    }
        CHECK_INTS_EQUAL(9, hype->Nodes[i]->Links.size());
    }

    hype->Remove(5);
    CHECK_INTS_EQUAL(9, hype->Nodes.size());

    delete hype;
}


TEST (fftTest, MyTest)
{
    int hits = 0;
    int misses = 0;
    for (int freq = 10; freq <= 48; freq++)
    {
        int best_frequency = FFT::Test(freq);
        if (best_frequency == freq)
            hits++;
        else
            misses++;
    }

    int hits_percent = hits * 100 / (hits + misses);
    if (hits_percent < 70)
    	printf("hits_percent = %d\n", hits_percent);
    CHECK(hits_percent > 70);
}

TEST (drawingTest, MyTest)
{
	int width = 320;
	int height = 240;
	unsigned char *img = new unsigned char[width * height * 3];
	CHECK(img != NULL);

	drawing::drawLine(img, width, height,
	                  50,70, 276,233,
	                  255,0,0,
	                  0, false);
	int n = ((70 * width) + 50) * 3;
	CHECK_INTS_EQUAL(255, img[n+2]);

    delete[] img;
}



TEST (colourToMonoTest, MyTest)
{
	int image_width = 4;
	int image_height = 4;
    unsigned char *colour_img = new unsigned char[image_width * image_height * 3];

    int n = 0;
    colour_img[n++] = 255;
    colour_img[n++] = 255;
    colour_img[n++] = 255;

    colour_img[n++] = 0;
    colour_img[n++] = 0;
    colour_img[n++] = 255;

    colour_img[n++] = 100;
    colour_img[n++] = 0;
    colour_img[n++] = 200;

    unsigned char *mono_img = new unsigned char[image_width * image_height];
    processimage::monoImage(colour_img, image_width, image_height, 0, mono_img);

    float one_third = 0.3333333333f;
    CHECK_INTS_EQUAL(255, mono_img[0]);
    CHECK_INTS_EQUAL((unsigned char)(255 * one_third), mono_img[1]);
    CHECK_INTS_EQUAL((unsigned char)((200+100) * one_third), mono_img[2]);

    delete[] mono_img;
    delete[] colour_img;
}

TEST (dilateTest, MyTest)
{
	int image_width = 6;
	int image_height = 6;
    unsigned char *mono_img = new unsigned char[image_width * image_height];
    unsigned char *buffer = new unsigned char[image_width * image_height];

    for (int i = 0; i < image_width * image_height; i++)
        mono_img[i] = 0;

    int n = (2 * image_width) + 2;
    mono_img[n] = 255;
    mono_img[n + 1] = 255;
    mono_img[n + image_width] = 255;
    mono_img[n + image_width + 1] = 255;

    unsigned char *dilated_img = new unsigned char[image_width * image_height];
    processimage::Dilate(mono_img, image_width, image_height, buffer, 1, dilated_img);
    CHECK_INTS_EQUAL(255, dilated_img[n]);
    CHECK_INTS_EQUAL(255, dilated_img[n + 1]);
    CHECK_INTS_EQUAL(255, dilated_img[n - 1]);
    CHECK_INTS_EQUAL(255, dilated_img[n - image_width]);
    CHECK_INTS_EQUAL(255, dilated_img[n + 2]);

    delete[] dilated_img;
    delete[] mono_img;
    delete[] buffer;
}

TEST (erodeTest, MyTest)
{
	int image_width = 6;
	int image_height = 6;
    unsigned char *mono_img = new unsigned char[image_width * image_height];
    unsigned char *buffer = new unsigned char[image_width * image_height];

    for (int i = 0; i < image_width * image_height; i++)
        mono_img[i] = 0;

    int n = (2 * image_width) + 2;
    mono_img[n] = 255;
    mono_img[n+1] = 255;
    mono_img[n+image_width] = 255;
    mono_img[n+image_width+1] = 255;

    unsigned char *eroded_img = new unsigned char[image_width * image_height];
    processimage::Erode(mono_img, image_width, image_height, buffer, 1, eroded_img);
    CHECK_INTS_EQUAL(0, eroded_img[n]);
    CHECK_INTS_EQUAL(0, eroded_img[n+1]);
    CHECK_INTS_EQUAL(0, eroded_img[n-1]);
    CHECK_INTS_EQUAL(0, eroded_img[n+image_width]);
    CHECK_INTS_EQUAL(0, eroded_img[n+image_width+1]);

    delete[] eroded_img;
    delete[] mono_img;
    delete[] buffer;
}


TEST (downSampleMonoTest, MyTest)
{
    int image_width = 640;
    int image_height = 480;
    int bytes_per_pixel = 1;
    unsigned char *mono_img = new unsigned char[image_width * image_height];
    int n = 0;
    for (int y = 0; y < image_height; y++)
    {
        for (int x = 0; x < image_width; x++)
        {
            mono_img[n] = 0;
            if ((x <= image_width/2) && (y <= image_height/2)) mono_img[n] = 255;
            if ((x >= image_width/2) && (y >= image_height/2)) mono_img[n] = 255;
            n++;
        }
    }
    unsigned char *downsampled_img = new unsigned char[image_width * image_height / 4];
    processimage::downSample(mono_img, image_width, image_height, bytes_per_pixel, downsampled_img);

    bool is_valid = true;
    n = image_width/2;
    for (int y = 1; y < (image_height/2)-2; y++)
    {
        for (int x = 0; x < image_width/2; x++)
        {
            if ((x < (image_width/4) - 2) && (y < (image_height/4) - 2))
            {
                if (downsampled_img[n] == 0) is_valid = false;
            }
            else
            {
                if ((x > (image_width/4) + 2) && (y > (image_height/4) + 2) &&
                    (x < (image_width/4) - 2) && (y < (image_height/4) - 2))
                {
                    if (downsampled_img[n] == 0) is_valid = false;
                }
            }
            n++;

        }
    }
    CHECK(is_valid == true);

    delete[] downsampled_img;
    delete[] mono_img;
}

#endif


// -----------------------------------------------------------------------------------------
// Medium level unit tests
// -----------------------------------------------------------------------------------------


#ifdef TEST_MEDIUM_LEVEL
#endif


// -----------------------------------------------------------------------------------------
// high level unit tests
// -----------------------------------------------------------------------------------------


#ifdef TEST_HIGH_LEVEL
#endif

#endif
