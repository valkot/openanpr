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

#include "cppunitlite/TestHarness.h"

#define DEBUG 1

// -----------------------------------------------------------------------------------------
// low level unit tests
// -----------------------------------------------------------------------------------------

#ifdef TEST_LOW_LEVEL

TEST (edgesTest, MyTest)
{
	int itterations = 20;
	double totalTime = 0;
	Image image;
	image.Height = 480;
	image.Width = 640;
	image.Data = raw_image;
	image.BytesPerPixel = 1;
	timeval start,end,result;

	int* histogram = new int[256];
	int* temp_histogram = new int[256];
	memset(histogram, 0, 256*sizeof(int));
	for (int i = (image.Width * image.Height)-1; i >= 0; i--)
		histogram[image.Data[i]]++;

	float MeanDark = 0;
	float MeanLight = 0;
	float DarkRatio = 0;
	float threshold =
		thresholding::GetGlobalThreshold(histogram, 255, 0, temp_histogram, MeanDark, MeanLight, DarkRatio);

	threshold = MeanLight - ((MeanLight - threshold)* 0.1f);

	unsigned char* filtered = new unsigned char[image.Width * image.Height];
	memset(filtered, 0, image.Width * image.Height * sizeof(unsigned char));
	for (int i = (image.Width * image.Height)-1; i >= 0; i--)
	{
		if (image.Data[i] > threshold)
			filtered[i] = image.Data[i];
	}
	image.Data = filtered;

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
	delete[] histogram;
	delete[] temp_histogram;
	delete[] filtered;
}


TEST (thresholdTest, MyTest)
{
	Image image;
	image.Height = 480;
	image.Width = 640;
	image.Data = raw_image;
	image.BytesPerPixel = 1;

	int* histogram = new int[256];
	int* temp_histogram = new int[256];
	memset(histogram, 0, 256*sizeof(int));
	for (int i = (image.Width * image.Height)-1; i >= 0; i--)
		histogram[image.Data[i]]++;

	float MeanDark = 0;
	float MeanLight = 0;
	float DarkRatio = 0;
	float threshold =
		thresholding::GetGlobalThreshold(histogram, 255, 0, temp_histogram, MeanDark, MeanLight, DarkRatio);

	float threshold_upper = MeanLight - ((MeanLight - threshold)* 0.1f);
	float threshold_lower = MeanDark - ((threshold - MeanDark)* 0.1f);

	unsigned char* filtered = new unsigned char[image.Width * image.Height * 3];
	memset(filtered, 0, image.Width * image.Height * 3 * sizeof(unsigned char));
	for (int i = (image.Width * image.Height)-1; i >= 0; i--)
	{
		if (image.Data[i] > threshold_upper)
		{
			filtered[i*3] = image.Data[i];
			filtered[(i*3) + 1] = image.Data[i];
			filtered[(i*3) + 2] = image.Data[i];
		}
		if (image.Data[i] < threshold_lower)
		{
			filtered[i*3] = 0;
			filtered[(i*3) + 1] = 0;
			filtered[(i*3) + 2] = 255;
		}
	}

    Bitmap *bmp = new Bitmap(filtered, image.Width, image.Height, 3);

    bmp->SavePPM("histogram.ppm");

    delete bmp;
    delete[] filtered;
	delete[] histogram;
	delete[] temp_histogram;
}


TEST (detectPolygonsFromImageTest, MyTest)
{
    int img_width = 640;
    int img_height = 480;

    unsigned char* img = raw_image;
    int erosion_dilation = 0;
    float max_deviation = 2;
    int minimum_edges_per_line = 20;
    int maximum_no_of_lines = 100;
    float valid_aspect_ratios[] = { 222.0f / 47.0f };
    int no_of_valid_aspect_ratios = 1;
    unsigned char* output_img = new unsigned char[img_width * img_height * 3];
    bool show_lines = true;
    bool show_polygons = false;
    bool show_intercepts = true;

    int max_edges_within_image = 1000;
    int no_of_samples_per_line = max_edges_within_image;
    int no_of_edge_samples_per_line = max_edges_within_image;

    printf("Detecting...");

    std::vector<int> edges;
    std::vector<std::vector<float> > lines;
    std::vector<polygon2D*> polygons;

    shapes::DetectPolygons(
    	img,
    	img_width, img_height,
    	erosion_dilation,
        edges,
        max_deviation,
        lines,
        polygons,
        no_of_samples_per_line,
        no_of_edge_samples_per_line,
        minimum_edges_per_line,
        maximum_no_of_lines,
        max_edges_within_image,
        valid_aspect_ratios,
        no_of_valid_aspect_ratios,
        output_img,
        show_lines,
        show_polygons,
        show_intercepts);

    printf("Done\n");

    string debug_filename = "detectedpolygons_image.ppm";
    Bitmap *bmp_debug = new Bitmap(output_img, img_width, img_height, 3);
    bmp_debug->SavePPM(debug_filename.c_str());
    delete bmp_debug;
    delete[] output_img;

    for (int i = 0; i < (int)polygons.size(); i++)
    {
    	delete polygons[i];
    	polygons[i] = NULL;
    }
}

TEST (detectPolygonsTest, MyTest)
{
    int img_width = 640;
    int img_height = 480;
    unsigned char* img = new unsigned char[img_width*img_height*3];
    for (int i = 0; i < img_width * img_height * 3; i++) img[i] = 255;

    float valid_aspect_ratios[] = { 1.0f, 18.0f / 8.0f };
    int no_of_valid_aspect_ratios = 2;
    float rotation = -30;
    int no_of_edges = 200;
    float noise = 3;
    std::vector<int> edges;
    int shape_width = 300;
    int shape_height = 300;
    int square_tx = 150;
    int square_ty = 150;
    int square_bx = square_tx + shape_width;
    int square_by = square_ty + shape_height;
    float square_cx = square_tx + ((square_bx - square_tx)/2.0f);
    float square_cy = square_ty + ((square_by - square_ty)/2.0f);

    for (int i = 0; i < no_of_edges; i++)
    {
    	int edge_x = rand() % img_width;
    	int edge_y = rand() % img_height;

    	if (rand() % 100 > 40)
    	{
    		int side = rand() % 4;
    		int x0 = 0;
    		int y0 = 0;
    		int x1 = 0;
    		int y1 = 0;
    		switch(side)
    		{
				case 0:
				{
					x0 = square_tx;
					y0 = square_ty;
					x1 = square_bx;
					y1 = square_ty;
					break;
				}
				case 1:
				{
					x0 = square_bx;
					y0 = square_ty;
					x1 = square_bx;
					y1 = square_by;
					break;
				}
				case 2:
				{
					x0 = square_bx;
					y0 = square_by;
					x1 = square_tx;
					y1 = square_by;
					break;
				}
				case 3:
				{
					x0 = square_tx;
					y0 = square_by;
					x1 = square_tx;
					y1 = square_ty;
					break;
				}
    		}

    		float dx,dy,dist,angle;

    		dx = x1 - x0;
    		dy = y1 - y0;
    		edge_x = x0 + (int)((rand() % 10000) * dx / 10000);
    		edge_y = y0 + (int)((rand() % 10000) * dy / 10000);

    		dx = edge_x - square_cx;
    		dy = edge_y - square_cy;
    		dist = (float)sqrt(dx*dx + dy*dy);
    		angle = (float)acos(dy / dist);
    		if (dx < 0) angle = (3.1415927f * 2) - angle;
    		angle += rotation * 3.1415927f / 180.0f;
    		edge_x = (int)(square_cx + (dist * (float)sin(angle)));
    		edge_y = (int)(square_cy + (dist * (float)cos(angle)));

    		// add some noise
    	    edge_x += ((rand() % 1000) * noise / 1000) - (noise * 0.5f);
    	    edge_y += ((rand() % 1000) * noise / 1000) - (noise * 0.5f);
    	}

    	edges.push_back(edge_x);
    	edges.push_back(edge_y);
    }

    int no_of_samples = no_of_edges;
    int no_of_edge_samples = no_of_edges;
    int minimum_edges_per_line = 10;
    int maximum_no_of_lines = 4;
    std::vector<std::vector<float> > lines;
    std::vector<float> intercepts;
    std::vector<polygon2D*> polygons;

    //printf("detecting...");

    shapes::DetectPolygons(
        edges,
        noise,
        lines,
        intercepts,
        polygons,
        no_of_samples,
        no_of_edge_samples,
        minimum_edges_per_line,
        maximum_no_of_lines,
        -200, -200, img_width+200, img_height+200,
        valid_aspect_ratios,
        no_of_valid_aspect_ratios,
        false);

    CHECK(polygons.size() == 1);

    //printf("%d polygons detected\n", polygons.size());

    for (int i = 0; i < (int)edges.size(); i += 2)
    {
    	// draw a circle
    	drawing::drawSpot(img, img_width, img_height, edges[i], edges[i+1], 1, 0,0,0);
    }

    for (int i = 0; i < (int)intercepts.size(); i += 2)
    	drawing::drawCircle(img, img_width, img_height, (int)intercepts[i], (int)intercepts[i+1], 5, 255,0,0, 0);

    for (int i = 0; i < (int)polygons.size(); i++)
    {
        polygons[i]->show(img, img_width, img_height, 0,0,255, 0);
    }

    string debug_filename = "detectedpolygons_lines.ppm";
    Bitmap *bmp_debug = new Bitmap(img, img_width, img_height, 3);
    bmp_debug->SavePPM(debug_filename.c_str());
    delete bmp_debug;
    delete[] img;

    for (int i = 0; i < (int)polygons.size(); i++)
    {
    	delete polygons[i];
    	polygons[i] = NULL;
    }
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
