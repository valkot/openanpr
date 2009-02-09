#include "platedetection.h"

platedetection::platedetection()
{

}

platedetection::~platedetection()
{

}

bool platedetection::Find(
    unsigned char *img_mono,
    int img_width, int img_height,
    int erosion_dilation,
    polygon2D* plate,
    bool debug,
    std::vector<unsigned char*> &debug_images)
{
	bool found = false;
    bool show_lines = true;
    bool show_polygons = true;
    bool show_intercepts = false;

	// common number plate width/height aspect ratios
    float valid_aspect_ratios[] = { 18.0f / 8.0f };
    int no_of_valid_aspect_ratios = 1;

    float max_deviation = 2;
    int minimum_edges_per_line = 10;
    int maximum_no_of_lines = 20;
    int max_edges_within_image = 1000;
    int no_of_samples_per_line = max_edges_within_image;
    int no_of_edge_samples_per_line = max_edges_within_image;

    printf("Detecting...");

    std::vector<int> edges;
    std::vector<std::vector<float> > lines;
    std::vector<polygon2D*> polygons;

    unsigned char* output_img = NULL;
    if (debug) output_img = new unsigned char[img_width * img_height * 3];

    shapes::DetectPolygons(
    	img_mono,
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

    if (debug) debug_images.push_back(output_img);

    printf("Done\n");

    for (int i = 0; i < (int)polygons.size(); i++)
    {
    	delete polygons[i];
    	polygons[i] = NULL;
    }

	return(found);
}


