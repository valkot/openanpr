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

void platedetection::MergeRectangles(std::vector<polygon2D*> &rectangles)
{
	float similarity_threshold_percent = 30;

	float* centres = new float[(int)rectangles.size()*2];
	float* lengths = new float[(int)rectangles.size()*2];

	for (int i = 0; i < (int)rectangles.size(); i++)
	{
		polygon2D* rect = rectangles[i];
		rect->GetSquareCentre(centres[i*2], centres[(i*2) + 1]);
		lengths[i*2] = rect->getLongestSide();
		lengths[(i*2)+1] = rect->getShortestSide();
	}

    hypergraph *hype = new hypergraph((int)rectangles.size(), 1);
	for (int i = 0; i < (int)rectangles.size() - 1; i++)
	{
		float length0 = lengths[i*2];
		float length1 = lengths[(i*2)+1];
		for (int j = i + 1; j < (int)rectangles.size(); j++)
		{
			float length_diff_percent = ABS(length0 - lengths[j*2]) * 100 / length0;
			if (length_diff_percent < similarity_threshold_percent)
			{
				length_diff_percent = ABS(length1 - lengths[(j*2)+1]) * 100 / length1;
				if (length_diff_percent < similarity_threshold_percent)
				{
					float dx = ABS(centres[i*2] - centres[j*2]);
					if (dx < length1 * 0.4f)
					{
						float dy = ABS(centres[(i*2)+1] - centres[(j*2)+1]);
						if (dy < length1 * 0.4f)
						{
				            hype->LinkByIndex(i, j);
				            hype->LinkByIndex(j, i);
						}
					}
				}
			}

		}
	}

	std::vector<polygon2D*> merged;
    float* vertex_x = new float[4];
    float* vertex_y = new float[4];
	for (int i = 0; i < (int)rectangles.size(); i++)
	{
		if (hype->GetFlagByIndex(i, 0) == false)
		{
		    std::vector<hypergraph_node *> members;
            hype->PropogateFlagFromIndex(i, 0, members, (int)rectangles.size()*10);
            if ((int)members.size() > 0)
            {
            	for (int j = 0; j < 4; j++)
            	{
            	    vertex_x[j] = 0;
            	    vertex_y[j] = 0;
            	}
            	for (int j = 0; j < (int)members.size(); j++)
            	{
            		int index = members[j]->ID;
            		for (int vertex = 0; vertex < 4; vertex++)
            		{
            		    vertex_x[vertex] += rectangles[index]->x_points[vertex];
            		    vertex_y[vertex] += rectangles[index]->y_points[vertex];
            		}
            	}
            	for (int j = 0; j < 4; j++)
            	{
            		vertex_x[j] /= (int)members.size();
            		vertex_y[j] /= (int)members.size();
            	}
            	polygon2D* merged_polygon = new polygon2D();
            	for (int j = 0; j < 4; j++)
            	{
            	    merged_polygon->Add(vertex_x[j], vertex_y[j]);
            	}
            	merged.push_back(merged_polygon);
            }
		}
	}

	for (int i = 0; i < (int)rectangles.size(); i++)
	{
	    delete rectangles[i];
	    rectangles[i] = NULL;
	}
	rectangles.clear();
	for (int i = 0; i < (int)merged.size(); i++)
	{
		rectangles.push_back(merged[i]);
		merged[i] = NULL;
	}

	delete[] vertex_x;
	delete[] vertex_y;
    delete hype;
	delete[] lengths;
	delete[] centres;
}

/*!
 * \brief extracts mono images for each candidate number plate
 * \param img_colour image data
 * \param img_width width of the image
 * \param img_height height of the image
 * \param plates candidate number plate perimeters
 * \param plate_image_width a specified fixed width for all extracted images
 * \param plate_image_height height values for the extracted number plate images
 * \param plate_image extracted mono images
 */
void platedetection::ExtractPlateImages(
    unsigned char *img_colour,
    int img_width, int img_height,
    std::vector<polygon2D*> &plates,
    int plate_image_width,
    std::vector<int> &plate_image_height,
    std::vector<unsigned char*> &plate_images)
{
	int pixels = (img_width * img_height * 3) - 3;

	polygon2D* number_plate = new polygon2D();
	for (int vertex = 0; vertex < 4; vertex++) number_plate->Add(0, 0);

	for (int p = 0; p < (int)plates.size(); p++)
	{
		polygon2D* number_plate = plates[p];

		float origin_x = img_width;
		float origin_y = img_height;
		int origin_index = 0;

		for (int vertex = 0; vertex < 4; vertex++)
		{
            if (number_plate->x_points[vertex] + number_plate->y_points[vertex] < origin_x + origin_y)
            {
           	    origin_x = number_plate->x_points[vertex];
           	    origin_y = number_plate->y_points[vertex];
           	    origin_index = vertex;
            }
		}
		int next_index = origin_index + 1;
		if (next_index >= 4) next_index -= 4;
		int prev_index = origin_index - 1;
		if (prev_index < 0) prev_index += 4;

		if (number_plate->getSideLength(origin_index) < number_plate->getSideLength(prev_index))
		{
			int temp_index = next_index;
			next_index = prev_index;
			prev_index = temp_index;
		}

		float dx_horizontal = number_plate->x_points[next_index] - origin_x;
		float dy_horizontal = number_plate->y_points[next_index] - origin_y;
		int w = (float)sqrt(dx_horizontal*dx_horizontal + dy_horizontal*dy_horizontal);
		float dx_vertical = number_plate->x_points[prev_index] - origin_x;
		float dy_vertical = number_plate->y_points[prev_index] - origin_y;
		int h = (float)sqrt(dx_vertical*dx_vertical + dy_vertical*dy_vertical);
		int height = (int)(h * plate_image_width / w);

		unsigned char* plate_image = new unsigned char[plate_image_width * height];
		plate_image_height.push_back(height);
		plate_images.push_back(plate_image);
		float mult_0 = dx_horizontal / plate_image_width;
		float mult_1 = dy_horizontal / plate_image_width;
		float mult_2 = dx_vertical / height;
		float mult_3 = dy_vertical / height;
		int n = 0;
	    for (int y = 0; y < height; y++)
		{
			float image_x = origin_x + (y * mult_2);
			float image_y = origin_y + (y * mult_3);

			for (int x = 0; x < plate_image_width; x++, n++, image_x += mult_0, image_y += mult_1)
			{
                int n2 = ((((int)image_y * img_width) + (int)image_x) * 3) + 2;
                if ((n2 >-1) && (n2 < pixels))
                {
                    plate_image[n] = img_colour[n2];
                }
			}
		}
	}
}

bool platedetection::Find(
    unsigned char *img_colour,
    int img_width, int img_height,
    std::vector<polygon2D*> &plates,
    bool debug,
    std::vector<unsigned char*> &debug_images,
	int &debug_image_width,
	int &debug_image_height)
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

	for (int plate_colour = PLATE_YELLOW; plate_colour <= PLATE_WHITE; plate_colour++)
	{
		for (int i = 0; i < img_width * img_height; i++)
			mono_img[i] = filtered[(i*3) + plate_colour];

		std::vector<polygon2D*> rectangles;
		std::vector<unsigned char*> temp_debug_images;
		float maximum_aspect_ratio = 200.0f / 33.0f;
		int maximum_groups = 45;
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
			debug_image_width,
			debug_image_height,
			edge_detector,
			rectangles,
			temp_debug_images,
			erosion_dilation_buffer,
			downsampling_buffer0,
			downsampling_buffer1);


		for (int i = 0; i < (int)temp_debug_images.size(); i++)
			debug_images.push_back(temp_debug_images[i]);

		for (int i = 0; i < (int)rectangles.size(); i++)
		{
			float tx = img_width;
			float ty = img_height;
			float bx = 0;
			float by = 0;
			for (int vertex = 0; vertex < 4; vertex++)
			{
				if (rectangles[i]->x_points[vertex] < tx) tx = rectangles[i]->x_points[vertex];
				if (rectangles[i]->y_points[vertex] < ty) ty = rectangles[i]->y_points[vertex];
				if (rectangles[i]->x_points[vertex] > bx) bx = rectangles[i]->x_points[vertex];
				if (rectangles[i]->y_points[vertex] > by) by = rectangles[i]->y_points[vertex];
			}
			int w = bx - tx;
			int h = by - ty;
			if (w > h)
			{
			    plates.push_back(rectangles[i]);
			}
			else
			{
				delete rectangles[i];
				rectangles[i] = NULL;
			}
		}
	}

	/*
	if (debug)
	{
		printf("debug_image_width: %d\n", debug_image_width);
		unsigned char* rectangles_img = new unsigned char[debug_image_width * debug_image_height * 3];
		memcpy(rectangles_img, img_colour, debug_image_width * debug_image_height * 3);
		for (int i = 0; i < (int)plates.size(); i++)
		{
			polygon2D* poly = plates[i];
			poly->show(rectangles_img, debug_image_width, debug_image_height, 0, 255, 0, 0);
		}
		debug_images.push_back(rectangles_img);
	}
	*/

	MergeRectangles(plates);

	delete edge_detector;
	delete[] mono_img;
	delete[] filtered;
    delete[] edges_image;
    delete[] erosion_dilation_buffer;
    delete[] downsampling_buffer0;
    delete[] downsampling_buffer1;

    if ((int)plates.size() > 0) found = true;

	return(found);
}


