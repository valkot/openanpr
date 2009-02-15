/*
    shape detection
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


#ifndef shapes_h
#define shapes_h

#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <vector>
#include "circledetector.h"
#include "../common.h"
#include "../utils/geometry.h"
#include "../utils/polygon.h"
#include "../utils/polynomial.h"
#include "../utils/processimage.h"
#include "../utils/thresholding.h"
#include "../utils/bitmap.h"
#include "../edgedetection/CannyEdgeDetector.h"
#include "../hypergraph/hypergraph.h"

#ifndef ABS
    #define ABS(a) (((a) < 0) ? -(a) : (a))
#endif
#ifndef PI
    #define PI 3.14159265358979323846264338327950288419716939937510
#endif

class histogram_element
{
    public:
    	std::vector<int> histogram;
};

class shapes
{
    public:

    	static void SortPerimeters(std::vector<polygon2D*> perimeters, std::vector<float> orientation);
        static void BinarizeSimple(unsigned char* img, int img_width, int img_height, int vertical_integration_percent, bool colour, unsigned char* binary_image);
        static void RemoveSurroundingBlob(unsigned char* img, int img_width, int img_height, bool black_on_white, bool colour_image);
        static void DetectSquaresInsideCircles(unsigned char* img, int img_width, int img_height, int bytes_per_pixel, int circular_ROI_radius, int perimeter_detection_method, bool square_black_on_white, int* grouping_radius_percent, int grouping_radius_percent_levels, int* erosion_dilation, int erosion_dilation_levels, int* compression, int no_of_compressions, int minimum_volume_percent, int maximum_volume_percent, bool use_perimeter_fitting, int perimeter_fit_threshold, int bestfit_tries, int* step_sizes, int no_of_step_sizes, int maximum_groups, std::vector<float>& circles, std::vector<int>& edges, unsigned char* edges_image, int& edges_image_width, int& edges_image_height, CannyEdgeDetector *edge_detector, std::vector<polygon2D*>& squares, std::vector<unsigned char*>& debug_images, unsigned char* erosion_dilation_buffer, int* downsampling_buffer0, int* downsampling_buffer1);
        static void DetectCircle(unsigned char* img, int img_width, int img_height, int bytes_per_pixel, int circular_ROI_radius, std::vector<float>& circles);
        static void DetectCircleMono(unsigned char* mono_img, int img_width, int img_height, int circular_ROI_radius, std::vector<float>& circles);
        static void DetectRectangles(unsigned char* img_colour, int img_width, int img_height, int bytes_per_pixel, int* grouping_radius_percent, int grouping_radius_percent_levels, int* erosion_dilation, int erosion_dilation_levels, bool black_on_white, int accuracy_level, float maximum_aspect_ratio, bool debug, int circular_ROI_radius, int perimeter_detection_method, int* compression, int no_of_compressions, int minimum_volume_percent, int maximum_volume_percent, bool use_perimeter_fitting, int perimeter_fit_threshold, int bestfit_tries, int* step_sizes, int no_of_step_sizes, int maximum_groups, std::vector<int>& edges, std::vector<float>& orientation, std::vector<std::vector<int> >& dominant_edges, std::vector<std::vector<std::vector<int> > >& side_edges, unsigned char* edges_image, int& edges_image_width, int& edges_image_height, CannyEdgeDetector *edge_detector, std::vector<polygon2D*>& rectangles, std::vector<unsigned char*>& debug_images, unsigned char* erosion_dilation_buffer, int* downsampling_buffer0, int* downsampling_buffer1);
        static void DetectSquares   (unsigned char* img_colour, int img_width, int img_height, int bytes_per_pixel, int* grouping_radius_percent, int grouping_radius_percent_levels, int* erosion_dilation, int erosion_dilation_levels, bool black_on_white, int accuracy_level, bool debug, int circular_ROI_radius, int perimeter_detection_method, int* compression, int no_of_compressions, int minimum_volume_percent, int maximum_volume_percent, bool use_perimeter_fitting, int perimeter_fit_threshold, int bestfit_tries, int* step_sizes, int no_of_step_sizes, int maximum_groups, std::vector<int>& edges, std::vector<float>& orientation, std::vector<std::vector<int> >& dominant_edges, std::vector<std::vector<std::vector<int> > >& side_edges, unsigned char* edges_image, int& edges_image_width, int& edges_image_height, CannyEdgeDetector *edge_detector, std::vector<polygon2D*>& squares, std::vector<unsigned char*>& debug_images, unsigned char* erosion_dilation_buffer, int* downsampling_buffer0, int* downsampling_buffer1);
        static void DetectSquares   (unsigned char* img_colour, int img_width, int img_height, int bytes_per_pixel, bool ignore_periphery, int image_border_percent, int* grouping_radius_percent, int grouping_radius_percent_levels, int* erosion_dilation, int erosion_dilation_levels, bool black_on_white, float minimum_aspect_ratio, float maximum_aspect_ratio, int downsampled_width, bool squares_only, bool debug, int circular_ROI_radius, int perimeter_detection_method, int* compression, int no_of_compressions, int minimum_volume_percent, int maximum_volume_percent, bool use_perimeter_fitting, int perimeter_fit_threshold, int bestfit_tries, int* step_sizes, int no_of_step_sizes, int maximum_groups, std::vector<int>& edges, std::vector<float>& orientation, std::vector<std::vector<int> >& dominant_edges, std::vector<std::vector<std::vector<int> > >& side_edges, CannyEdgeDetector *edge_detector, unsigned char* edges_image, int& edges_image_width, int& edges_image_height, std::vector<polygon2D*>& squares, std::vector<unsigned char*>& debug_images, unsigned char* erosion_dilation_buffer, int* downsampling_buffer0, int* downsampling_buffer1);
        static void DetectSquaresMono(unsigned char* mono_img, int img_width, int img_height, bool ignore_periphery, int image_border_percent, int* grouping_radius_percent, int grouping_radius_percent_levels, int* erosion_dilation, int erosion_dilation_levels, bool black_on_white, bool use_original_image, float minimum_aspect_ratio, float maximum_aspect_ratio, bool squares_only, bool debug, int circular_ROI_radius, int perimeter_detection_method, int* compression, int no_of_compressions, int minimum_volume_percent, int maximum_volume_percent, bool use_perimeter_fitting, int perimeter_fit_threshold, int bestfit_tries, int* step_sizes, int no_of_step_sizes, int maximum_groups, std::vector<int>& edges, std::vector<float>& orientation, std::vector<std::vector<int> >& dominant_edges, std::vector<std::vector<std::vector<int> > >& side_edges, CannyEdgeDetector *edge_detector, unsigned char* edges_image, std::vector<polygon2D*>& square_shapes, std::vector<unsigned char*>& debug_images, unsigned char* erosion_dilation_buffer);
        static void GetValidGroups(std::vector<std::vector<int> > &groups, int img_width, int img_height, int minimum_size_percent, std::vector<std::vector<int> >& results);
        static void GetAspectRange(std::vector<std::vector<int> > &groups, int img_width, int img_height, float minimum_aspect, float maximum_aspect, int minimum_size_percent, bool squares_only, std::vector<std::vector<int> >& results);
        static void GetGroups(std::vector<int> &edges, int img_width, int img_height, int image_border, int minimum_size_percent, bool squares_only, float max_rectangular_aspect, bool ignore_periphery, int grouping_radius_percent, int* compression, int no_of_compressions, std::vector<std::vector<int> >& groups, int* line_segment_map_buffer, int* step_sizes, int no_of_step_sizes);
        static void ShowLongestPerimeters(std::vector<int> &edges, int img_width, int img_height, int image_border, int minimum_size_percent, bool squares_only, float max_rectangular_aspect, int max_search_depth, bool ignore_periphery, bool show_centres, unsigned char* result);
        static void ShowEdges(std::vector<int> &edges, int img_width, int img_height, unsigned char* result);
        static void ShowEdges(std::vector<float> &edges, int img_width, int img_height, unsigned char* result);
        static void ShowEdges(std::vector<int> &edges, unsigned char* img, int img_width, int img_height, int r, int g, int b);
        static void ShowRawImagePerimeter(unsigned char* img, int img_width, int img_height, polygon2D *perim, int r, int g, int b, int line_width, unsigned char* result);
        static void ShowEdges(std::vector<std::vector<int> > &groups, int img_width, int img_height, unsigned char* result);
        static void ShowGroups(std::vector<int> &edges, int img_width, int img_height, int image_border, int minimum_size_percent, bool squares_only, float maximum_aspect_ratio, int max_search_depth, bool ignore_periphery, int grouping_radius_percent, std::vector<std::vector<int> >& groups, unsigned char* result);
        static void ShowGroups(std::vector<std::vector<int> > &groups, int img_width, int img_height, unsigned char* result);
        static void DetectLongestPerimeters(std::vector<int> &edges, int img_width, int img_height, int image_border, int minimum_size_percent, bool squares_only, float max_rectangular_aspect, bool ignore_periphery, std::vector<float>& centres, std::vector<float>& bounding_boxes, std::vector<std::vector<int> >& longestPerimeters);
        static bool ApproximateSquare(polygon2D *perimeter, polygon2D *approximated_perimeter);

        // experimental functions
        static bool ValidPolygon(polygon2D* poly, float* valid_aspect_ratios, int no_of_valid_aspect_ratios);
        static int BestFitLineRANSAC(std::vector<int> &edges, float max_deviation, float& x0, float& y0, float& x1, float& y1, int no_of_samples, int no_of_edge_samples, bool remove_edges);
        static void DetectLines(std::vector<int> &edges, float max_deviation, std::vector<std::vector<float> > &lines, int no_of_samples_per_line, int no_of_edge_samples_per_line, int minimum_edges_per_line, int maximum_no_of_lines);
        static void DetectPolygons(std::vector<int> &edges, float max_deviation, std::vector<std::vector<float> > &lines, std::vector<float> &intercepts, std::vector<polygon2D*> &polygons, int no_of_samples_per_line, int no_of_edge_samples_per_line, int minimum_edges_per_line, int maximum_no_of_lines, int boundingbox_tx, int boundingbox_ty, int boundingbox_bx, int boundingbox_by, float* valid_aspect_ratios, int no_of_valid_aspect_ratios, bool fit_to_edge_map);
        static void DetectPolygons(
        	unsigned char* img_mono,
        	int img_width, int img_height,
        	int erosion_dilation,
            std::vector<int> &edges,
            float max_deviation,
            std::vector<std::vector<float> > &lines,
            std::vector<polygon2D*> &polygons,
            int no_of_samples_per_line,
            int no_of_edge_samples_per_line,
            int minimum_edges_per_line,
            int maximum_no_of_lines,
            int max_edges_within_image,
            float* valid_aspect_ratios,
            int no_of_valid_aspect_ratios,
            unsigned char* output_img,
            bool show_lines,
            bool show_polygons,
            bool show_intercepts);

    private:

        static void PredictSquareVertex(polygon2D *perimeter, int vertex, float &vertex_x, float &vertex_y);
    	static int EdgeMagnitude(unsigned char* img_mono, int img_width, int img_height, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, int no_of_samples);
    	static bool FitPerimeter(unsigned char* img_mono, int img_width, int img_height, polygon2D *perimeter, float fit_radius_pixels, int no_of_samples, int magnitude_threshold, float shrink_factor, float perimeter_fit_step_size);
        static bool VectorContains(std::vector<polygon2D*> &vect, polygon2D *item);
        static bool VectorContains(std::vector<hypergraph_node*> &vect, hypergraph_node *item);
        static bool VectorContains(std::vector<std::string> &vect, std::string item);
        static bool VectorRemove(std::vector<polygon2D*> &vect, polygon2D *item);
        static float Squareness(polygon2D *square);
        static void TraceEdge(bool** edges_img, int& img_width, int& img_height, int x, int y, int& length, std::vector<int> &members, int& image_border, bool& isValid, int& perimeter_tx, int& perimeter_ty, int& perimeter_bx, int& perimeter_by, bool& ignore_periphery, int& centre_x, int& centre_y);
        static float DominantOrientation(int tx, int ty, int* left_edges, int left_edges_length, int* right_edges, int right_edges_length, int* top_edges, int top_edges_length, int* bottom_edges, int bottom_edges_length, std::vector<int>& dominant_edges, std::vector<int>& non_dominant_edges, std::vector<std::vector<int> >& side_edges, float quantization_degrees, int step_size, int *histogram, histogram_element** histogram_members);
        static float BestFitLine(std::vector<int> &edges, float max_deviation, int baseline_length_pixels, float& x0, float& y0, float& x1, float& y1);
        void RotateEdges(std::vector<int> &edges, int centre_x, int centre_y, float rotate_angle, std::vector<int>& rotated);
        static void MostSquare(std::vector<polygon2D*> &square_shapes, std::vector<float>& orientation, int max_squares);

        static void GetPeripheralEdges(
        	std::vector<int> &edges,
        	int &tx, int &ty, int &bx, int &by,
        	int* left, int* right, int* top, int* bottom);

        static void GetPeripheral(
        	int detection_method,
        	int erode_dilate,
        	float best_fit_max_deviation_pixels,
        	int best_fit_baseline_pixels,
        	int tx, int ty, int bx, int by,
        	int* left, int* right,
        	int* top, int* bottom,
        	polygon2D *perim,
        	std::vector<std::vector<int> > &side_edges,
        	std::vector<std::vector<int> > &result);
};

#endif
