/*
    detect circles
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

#include "circledetector.h"

/*!
 * \brief constructor
 */
circleDetector::circleDetector()
{
    maximum_possible_centre_separation = 20;
    minimum_centre_response = 15;
    dilation = 3;
}

/*!
 * \brief destructor
 */
circleDetector::~circleDetector()
{
}

// ********** public methods **********

/*!
 * \brief ShowPossibleCentres
 * \param img_width
 * \param img_height
 * \param clear_image
 */
void circleDetector::ShowPossibleCentres(
    unsigned char* img,
    int img_width,
    int img_height,
    bool clear_image)
{
    // clear the image
    if (clear_image)
    {
	    int pixels = img_width*img_height*3;
        for (int i = 0; i < pixels; i++) img[i] = 255;
    }

    for (int i = 0; i < (int)possible_centres.size(); i += 3)
    {
        float x = possible_centres[i] * img_width / image_width;
        float y = possible_centres[i + 1] * img_height / image_height;
        drawing::drawCross(img, img_width, img_height,
                           (int)x, (int)y, 3, 150, 150, 150, 0);
    }

    if (centre_x != 9999)
    {
        float x = centre_x * img_width / image_width;
        float y = centre_y * img_height / image_height;
        float r = radius * img_width / image_width;
        drawing::drawCircle(img, img_width, img_height,
                            x, y, r, 255, 0, 0, 0);
    }
}

/*!
 * \brief ShowCircle
 * \param img_width
 * \param img_height
 */
void circleDetector::ShowCircle(
    unsigned char* img,
    int img_width,
    int img_height)
{
    if (centre_x != 9999)
    {
        float x = centre_x * img_width / image_width;
        float y = centre_y * img_height / image_height;
        float r = radius * img_width / image_width;
        drawing::drawCircle(img, img_width, img_height,
                            x, y, r, 255, 255, 0, 1);
    }
}

/*!
 * \brief Find
 * \param connected_points
 * \param image_width
 * \param image_height
 * \return
 */
bool circleDetector::Find(
    std::vector<float> &connected_points,
    int image_width,
    int image_height)
{
    this->image_width = image_width;
    this->image_height = image_height;
    centre_x = 9999;

    bool circle_found = false;

    intersections.clear();
    radii.clear();

    int step_size = 5;

    float max_radius = 0;
    for (int i = 0; i < (int)connected_points.size(); i += 2)
    {
        float prev_x = connected_points[i];
        float prev_y = connected_points[i + 1];

        if (i + 1 + (step_size * 2) < (int)connected_points.size())
        {
            float x = connected_points[i + (step_size * 2)];
            float y = connected_points[i + 1 + (step_size * 2)];

            float dx = x - prev_x;
            float dy = y - prev_y;
            float cx = prev_x + (dx / 2);
            float cy = prev_y + (dy / 2);
            float x0 = cx + dy;
            float y0 = cy - dx;
            float x1 = cx - dy;
            float y1 = cy + dx;

            int max_j = (connected_points.size() / 2) - step_size - 2;
            int j = (int)((rand() / (float)RAND_MAX) * max_j) * 2;
            float prev_x2 = connected_points[j];
            float prev_y2 = connected_points[j + 1];
            if (j + 1 + (step_size * 2) < (int)connected_points.size())
            {
                float x2 = connected_points[j + (step_size * 2)];
                float y2 = connected_points[j + 1 + (step_size * 2)];

                float dx2 = x2 - prev_x2;
                float dy2 = y2 - prev_y2;
                float cx2 = prev_x2 + (dx2 / 2);
                float cy2 = prev_y2 + (dy2 / 2);
                float prev_x0 = cx2 + dy2;
                float prev_y0 = cy2 - dx2;
                float prev_x1 = cx2 - dy2;
                float prev_y1 = cy2 + dx2;

                //if (prev_x0 != 9999)
                {
                    // find the intersection between the two lines
                    float ix = 0;
                    float iy = 0;
                    geometry::intersection(x0, y0, x1, y1,
                                           prev_x0, prev_y0, prev_x1, prev_y1,
                                           ix, iy);
                    intersections.push_back(ix);
                    intersections.push_back(iy);

                    // find the radii distances
                    dx = cx - ix;
                    dy = cy - iy;
                    if ((ABS(dx) < image_width) && (ABS(dy) < image_height))
                    {
                        float r = (float)sqrt((dx * dx) + (dy * dy));
                        if (r > max_radius) max_radius = r;
                        radii.push_back(r);
                        radii.push_back(ix);
                        radii.push_back(iy);
                    }
                }

                //prev_x0 = x0;
                //prev_y0 = y0;
                //prev_x1 = x1;
                //prev_y1 = y1;
            }
        }
    }

    // create a histogram of radii
    if (max_radius > image_width) max_radius = image_width;
    int quantisation = 5 * image_width / 320;
    int histogram_length = (int)(max_radius / quantisation) + 1;
    int* histogram = new int[histogram_length];
    int max_response = 0;
    for (int i = 0; i < (int)radii.size()-3; i += 3)
    {
        float r = radii[i];
        int bucket = (int)(r / quantisation);
        if (bucket < histogram_length)
        {
            histogram[bucket]++;
            if (histogram[bucket] > max_response)
            {
                max_response = histogram[bucket];
            }
        }
    }

    // find peaks within the histogram, corresponding to circular regions
    int minimum_hits = 3;
    possible_centres.clear();
    if (max_response > minimum_hits)
    {
        // the peak must be greater than some fraction of
        // the maximum response
        int minimum_response = max_response * 80 / 100;

        for (int i = 0; i < (int)radii.size(); i += 3)
        {
            float r = radii[i];
            int bucket = (int)(r / quantisation);
            if (bucket < histogram_length)
            {
                if (histogram[bucket] > minimum_response)
                {
                    // store the possible centre and radius of the circle
                    float x = radii[i + 1];
                    float y = radii[i + 2];
                    possible_centres.push_back(x);
                    possible_centres.push_back(y);
                    possible_centres.push_back(r);
                }
            }
        }
    }

    // distill the possible centres into single points
    int max_possible_centre_hits = minimum_centre_response;
    for (int i = 0; i < (int)possible_centres.size(); i += 3)
    {
        float x1 = possible_centres[i];
        float y1 = possible_centres[i + 1];
        float r = possible_centres[i + 2];

        float mean_x = x1;
        float mean_y = y1;
        float mean_radius = r;
        int hits = 1;

        for (int j = i + 3; j < (int)possible_centres.size(); j += 3)
        {
            float x2 = possible_centres[j];
            float y2 = possible_centres[j + 1];

            float dx = x2 - x1;
            float dy = y2 - y1;
            if ((ABS(dx) < maximum_possible_centre_separation) &&
                (ABS(dy) < maximum_possible_centre_separation))
            {
                mean_x += x2;
                mean_y += y2;
                mean_radius += possible_centres[j + 2];
                hits++;
            }
        }

        // record the most likely circle shape
        if (hits > max_possible_centre_hits)
        {
            max_possible_centre_hits = hits;
            centre_x = mean_x / hits;
            centre_y = mean_y / hits;
            radius = (mean_radius / hits) - dilation;
            circle_found = true;
        }
    }

    if (!circle_found) centre_x = 9999;
    delete[] histogram;

    return (circle_found);
}
