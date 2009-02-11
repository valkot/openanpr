#include "platedetection.h"

platedetection::platedetection()
{

}

platedetection::~platedetection()
{

}

void platedetection::ColourFilter(
    unsigned char* img_colour,
    int img_width, int img_height,
    unsigned char* filtered)
{
	int* histogram = new int[256];
	int* temp_histogram = new int[256];
	memset(histogram, 0, 256*sizeof(int));
	for (int i = (img_width * img_height)-1; i >= 0; i--)
		histogram[img_colour[i]]++;

	float MeanDark = 0;
	float MeanLight = 0;
	float DarkRatio = 0;
	float threshold =
		thresholding::GetGlobalThreshold(histogram, 255, 0, temp_histogram, MeanDark, MeanLight, DarkRatio);

	float threshold_upper = MeanLight - ((MeanLight - threshold)* 0.2f);

	processimage::yellowFilter(img_colour, img_width, img_height, filtered);

	for (int i = (img_width * img_height*3)-3; i >= 0; i-=3)
	{
		if (img_colour[i] > threshold_upper)
		{
			int r = img_colour[i + 2];
			int g = img_colour[i + 1];
			int b = img_colour[i];
			int v = ((r+g+b)/3) - b - r - ((ABS(r - g) - ABS(r - b) - ABS(g - b))*10);
			if (v < 0) v = 0;
			if (v > 255) v = 255;
			if (v == 0)
			    filtered[i + 1] = (unsigned char)r;
		}
	}

	delete[] histogram;
	delete[] temp_histogram;
}

bool platedetection::Find(
    unsigned char *img_colour,
    int img_width, int img_height,
    polygon2D* plate,
    bool debug,
    std::vector<unsigned char*> &debug_images)
{
    bool found = false;

	unsigned char* filtered = new unsigned char[img_width * img_height * 3];
	unsigned char* mono_img = new unsigned char[img_width * img_height];
    unsigned char* edges_image = new unsigned char[img_width * img_height];
    unsigned char* erosion_dilation_buffer = new unsigned char[img_width * img_height];
    int* downsampling_buffer0 = new int[img_width * img_height];
    int* downsampling_buffer1 = new int[img_width * img_height];
	CannyEdgeDetector *edge_detector = new CannyEdgeDetector();

	// apply colour filters
	platedetection::ColourFilter(img_colour, img_width, img_height, filtered);

	for (int plate_colour = PLATE_YELLOW; plate_colour < PLATE_WHITE; plate_colour++)
	{
		for (int i = 0; i < img_width * img_height; i++)
			mono_img[i] = filtered[i*3 + plate_colour];

		float maximum_aspect_ratio = 200.0f / 33.0f;
		int maximum_groups = 45;
		int bestfit_tries = 3;
		int step_sizes[] = { 12, 4 };
		int no_of_step_sizes = 2;
		int grouping_radius_percent[] = { 0, 10, 40, 60 };
		int grouping_radius_percent_levels = 4;
		int erosion_dilation[] = { 1 };
		int erosion_dilation_levels = 1;
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
		int edges_image_width = 0;
		int edges_image_height = 0;
		std::vector<polygon2D*> rectangles;

		shapes::DetectRectangles(
			mono_img, img_width, img_height, 1,
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

		if (debug)
		{
			unsigned char* rectangles_img = new unsigned char[img_width * img_height * 3];
			memcpy(rectangles_img, img_colour, img_width * img_height * 3);
			for (int i = 0; i < (int)rectangles.size(); i++)
			{
				polygon2D* poly = rectangles[i];
				poly->show(rectangles_img, img_width, img_height, 0, 255, 0, 0);
			}
			debug_images.push_back(rectangles_img);
		}

	}

	for (int i = 0; i < (int)rectangles.size(); i++)
	{
		delete rectangles[i];
		rectangles[i] = NULL;
	}

	delete edge_detector;
	delete[] mono_img;
	delete[] filtered;
    delete[] edges_image;
    delete[] erosion_dilation_buffer;
    delete[] downsampling_buffer0;
    delete[] downsampling_buffer1;

	return(found);
}


