#include "shapes.h"

// ********** public methods **********



/*!
 * \brief performs a simple kind of binarization on the entire image
 * \param img image data
 * \param img_width width of the image
 * \param img_height height of the image
 * \param colour is this a colour image
 * \return mono binarized image
 */
void shapes::BinarizeSimple(
    unsigned char* img,
    int img_width,
    int img_height,
    int vertical_integration_percent,
    bool colour,
    unsigned char* binary_image)
{
    // get a mono version of the image
    unsigned char* img_mono = img;
    if (colour)
    {
        img_mono = new unsigned char[img_width * img_height];
        int bytes_per_pixel = 3;
        int n = 0;
        for (int i = 0; i < img_width * img_height * 3; i += bytes_per_pixel)
        {
            int intensity = 0;
            for (int b = 0; b < bytes_per_pixel; b++)
                intensity = img[i + b];
            img_mono[n++] = (unsigned char)(intensity / bytes_per_pixel);
        }
    }
    else
    {
        img_mono = img;
    }

    float mean_light = 0;
    float mean_dark = 0;
    float dark_ratio = 0;
    unsigned char threshold = 0;
    int w = img_width * (img_height * vertical_integration_percent / 100);
    int w2 = (img_width * img_height) - 1 - w;
    int histogram_sampling_step = 5;
    float* histogram = new float[256];
    for (int y = 0; y < img_height; y++)
    {
        int n1 = (y * img_width);
        int n2 = n1 + img_width;

        // create a histogram of the image row
        for (int i = 255; i >= 0; i--) histogram[i] = 0;
        for (int i = n1; i < n2; i += histogram_sampling_step)
        {
            histogram[img_mono[i]]++;
            if (i > w) histogram[img_mono[i - w]]++;
            if (i < w2) histogram[img_mono[i + w]]++;
        }

        // find the threshold for this row
        threshold = (unsigned char)thresholding::GetGlobalThreshold(histogram, 256, mean_dark, mean_light, dark_ratio);

        for (int i = n1; i < n2; i++)
            if (img_mono[i] > threshold)
                binary_image[i] = (unsigned char)255;
    }

    w = img_width * vertical_integration_percent / 100;
    w2 = img_width - 1 - w;
    for (int x = 0; x < img_width; x++)
    {
        // create a histogram of the image row
        for (int i = 255; i >= 0; i--) histogram[i] = 0;
        for (int y = 0; y < img_height; y += histogram_sampling_step)
        {
            int n = (y * img_width) + x;
            histogram[img_mono[n]]++;
            if (x > w) histogram[img_mono[n - w]]++;
            if (x < w2) histogram[img_mono[n + w]]++;
        }

        // find the threshold for this row
        threshold = (unsigned char)thresholding::GetGlobalThreshold(histogram, 256, mean_dark, mean_light, dark_ratio);

        for (int y = 0; y < img_height; y++)
        {
            int n = (y * img_width) + x;
            if (img_mono[n] > threshold)
                binary_image[n] = (unsigned char)255;
        }
    }

    delete[] histogram;
    if (colour) delete[] img_mono;
}

/*!
 * \brief RemoveSurroundingBlob
 * \param img_width
 * \param img_height
 * \param black_on_white
 * \param colour_image
 */
void shapes::RemoveSurroundingBlob(
    unsigned char* img,
    int img_width,
    int img_height,
    bool black_on_white,
    bool colour_image)
{
    unsigned char* blob_image = new unsigned char[img_width * img_height];
    BinarizeSimple(img, img_width, img_height, 20, colour_image, blob_image);

    int n, black_to_white_transition;
    int maximum_continuous_white = img_width / 12;

    int pixels = img_width * img_height;
    bool* surrounding_blob_image = new bool[pixels];
    for (int i = 0; i < pixels; i++)
        if (blob_image[i] == 255)
            surrounding_blob_image[i] = true;
        else
            surrounding_blob_image[i] = false;

    // remove vertical sections
    for (int x = 0; x < img_width; x++)
    {
        black_to_white_transition = -1;
        for (int y = 1; y < img_height; y++)
        {
            n = (y * img_width) + x;

            if (surrounding_blob_image[n] != surrounding_blob_image[n - img_width])
            {
                if ((surrounding_blob_image[n] != black_on_white) &&
                    (surrounding_blob_image[n - img_width] == black_on_white))
                {
                    // notice transitions from black to white
                    black_to_white_transition = y;
                }
                else
                {
                    if (black_to_white_transition > -1)
                    {
                        int y2 = y;
                        int continuous_white = y2 - black_to_white_transition;
                        if (continuous_white < maximum_continuous_white)
                        {
                            // fill   black
                            for (int yy = black_to_white_transition; yy < y2; yy++)
                            {
                                int n2 = (yy * img_width) + x;
                                surrounding_blob_image[n2] = black_on_white;
                            }
                        }
                    }
                }
            }
        }
    }

    // remove horizontal sections
    for (int y = 0; y < img_height; y++)
    {
        black_to_white_transition = -1;
        for (int x = 1; x < img_width; x++)
        {
            n = (y * img_width) + x;

            if (surrounding_blob_image[n] != surrounding_blob_image[n - 1])
            {
                if ((surrounding_blob_image[n] != black_on_white) &&
                    (surrounding_blob_image[n - 1] == black_on_white))
                {
                    // notice transitions from black to white
                    black_to_white_transition = x;
                }
                else
                {
                    if (black_to_white_transition > -1)
                    {
                        int x2 = x;
                        int continuous_white = x2 - black_to_white_transition;
                        if (continuous_white < maximum_continuous_white)
                        {
                            // fill   black
                            for (int xx = black_to_white_transition; xx < x2; xx++)
                            {
                                int n2 = (y * img_width) + xx;
                                surrounding_blob_image[n2] = black_on_white;
                            }
                        }
                    }
                }
            }
        }
    }

    // erode or dilate the surrounding blob
    bool high = true;
    bool low = false;
    if (black_on_white)
    {
        high = false;
        low = true;
    }
    int erode_dilate = 10;
    int max = (img_width * (img_height - 1)) - 1;
    bool* new_surrounding_blob_image = new bool[pixels];
    memcpy(new_surrounding_blob_image, surrounding_blob_image, pixels);
    for (int i = 0; i < erode_dilate; i++)
    {
        if (i > 0)
        {
            for (int j = 0; j < pixels; j++)
                new_surrounding_blob_image[j] = surrounding_blob_image[j];
        }

        for (int j = 1; j < pixels - 1; j++)
        {
            if (surrounding_blob_image[j] == low)
            {
                if ((surrounding_blob_image[j - 1] == high) ||
                    (surrounding_blob_image[j + 1] == high))
                {
                    new_surrounding_blob_image[j] = high;
                }
                else
                {
                    if (j < max)
                    {
                        if (surrounding_blob_image[j + img_width] == high)
                            new_surrounding_blob_image[j] = high;
                    }
                    if (j < max - 1)
                    {
                        if (surrounding_blob_image[j + img_width + 1] == high)
                            new_surrounding_blob_image[j] = high;
                    }
                    if (j < max + 1)
                    {
                        if (surrounding_blob_image[j + img_width - 1] == high)
                            new_surrounding_blob_image[j] = high;
                    }
                    if (j > img_width)
                    {
                        if (surrounding_blob_image[j - img_width] == high)
                            new_surrounding_blob_image[j] = high;
                    }
                    if (j > img_width + 1)
                    {
                        if (surrounding_blob_image[j - img_width - 1] == high)
                            new_surrounding_blob_image[j] = high;
                    }
                    if (j > img_width - 1)
                    {
                        if (surrounding_blob_image[j - img_width + 1] == high)
                            new_surrounding_blob_image[j] = high;
                    }
                }
            }
        }

        for (int j = 0; j < pixels; j++)
            surrounding_blob_image[j] = new_surrounding_blob_image[j];
    }


    //remove the background from the original image
    n = 0;
    unsigned char background_intensity = 0;
    if (black_on_white) background_intensity = 255;
    int bytes_per_pixel = 1;
    if (colour_image) bytes_per_pixel = 3;
    for (int y = 0; y < img_height; y++)
    {
        for (int x = 0; x < img_width; x++)
        {
            if (surrounding_blob_image[n] == true)
            {
                for (int b = 0; b < bytes_per_pixel; b++)
                    img[(n * bytes_per_pixel) + b] = background_intensity;
            }
            n++;
        }
    }

    delete[] blob_image;
    delete[] new_surrounding_blob_image;
    delete[] surrounding_blob_image;
}

/*!
 * \brief DetectSquaresInsideCircles
 * \param img_width
 * \param img_height
 * \param bytes_per_pixel
 * \param circular_ROI_radius
 * \param perimeter_detection_method
 * \param square_black_on_white
 * \param grouping_radius_percent
 * \param grouping_radius_percent_levels
 * \param erosion_dilation
 * \param erosion_dilation_levels
 * \param compression
 * \param no_of_compressions
 * \param minimum_volume_percent minimum volume of the square as a percentage of the image volume (prevents very small stuff being detected)
 * \param maximum_volume_percent maximum volume of the square as a percentage of the image volume (prevents very large stuff being detected)
 * \param use_perimeter_fitting whether to enable an additional attempt to fit the perimeter as closely as possible
 * \param perimeter_fit_threshold a threshold value used to fit the perimeter more closely
 * \param bestfit_tries number of tries when fitting teh perimeter using different parameters
 * \param step_sizes array containing step sizes (in pixels) used when connecting line segments with GetGroups
 * \param no_of_step_sizes the number of step sizes in the array
 */
void shapes::DetectSquaresInsideCircles(
    unsigned char* img,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int circular_ROI_radius,
    int perimeter_detection_method,
    bool square_black_on_white,
    int* grouping_radius_percent,
    int grouping_radius_percent_levels,
    int* erosion_dilation,
    int erosion_dilation_levels,
    int* compression,
    int no_of_compressions,
    int minimum_volume_percent,
    int maximum_volume_percent,
    bool use_perimeter_fitting,
    int perimeter_fit_threshold,
    int bestfit_tries,
    int* step_sizes,
    int no_of_step_sizes,
    int maximum_groups,
    std::vector<float>& circles,
    std::vector<int>& edges,
    unsigned char* edges_image,
    int& edges_image_width,
    int& edges_image_height,
    CannyEdgeDetector *edge_detector,
    std::vector<polygon2D*>& squares,
    std::vector<unsigned char*>& debug_images,
    unsigned char* erosion_dilation_buffer,
    int* downsampling_buffer0,
    int* downsampling_buffer1)
{
    squares.erase(squares.begin(), squares.end());

    DetectCircle(img, img_width, img_height, bytes_per_pixel, circular_ROI_radius, circles);

    for (int i = 0; i < (int)circles.size(); i += 3)
    {
        float centre_x = circles[i];
        float centre_y = circles[i + 1];
        float radius = circles[i + 2];

        int tx = (int)(centre_x - radius);
        int ty = (int)(centre_y - radius);
        int bx = (int)(centre_x + radius);
        int by = (int)(centre_y + radius);

        // create a smaller image of the region
        int w = bx - tx;
        int h = by - ty;

        // ensure that the cropped dimensions are divisible by 2
        w = (w/2) * 2;
        h = (h/2) * 2;
        bx = tx + w;
        by = ty + h;
        unsigned char* small = new unsigned char[w * h * bytes_per_pixel];

        processimage::cropImage(img, img_width, img_height, bytes_per_pixel,
                                tx, ty, bx, by, small);

        // now detect a square inside the smaller image
        std::vector<float> orientation;
        std::vector<std::vector<int> > dominant_edges;
        std::vector<std::vector<std::vector<int> > > side_edges;

        DetectSquares(
            small, w, h, bytes_per_pixel,
            grouping_radius_percent, grouping_radius_percent_levels,
            erosion_dilation, erosion_dilation_levels,
            square_black_on_white, -1,
            false, h * 50 / 100,
            perimeter_detection_method,
            compression, no_of_compressions,
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
            edges_image_width, edges_image_height,
            edge_detector,
            squares,
            debug_images,
            erosion_dilation_buffer,
            downsampling_buffer0,
            downsampling_buffer1);

        for (int j = 0; j < (int)edges.size(); j += 2)
        {
            edges[j] += tx;
            edges[j + 1] += ty;
        }

        for (int j = 0; j < (int)squares.size(); j++)
        {
            for (int vertex = 0; vertex < 4; vertex++)
            {
                squares[j]->x_points[vertex] += tx;
                squares[j]->y_points[vertex] += ty;
            }
        }

        delete[] small;
    }

}

/*!
 * \brief DetectCircle
 * \param img
 * \param img_width
 * \param img_height
 * \param circular_ROI_radius
 * \param circles returned circles
 */
void shapes::DetectCircle(
    unsigned char* img,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int circular_ROI_radius,
    std::vector<float>& circles)
{
	if (bytes_per_pixel > 1)
	{
		unsigned char* img_mono = new unsigned char[img_width * img_height];
		processimage::monoImage(img, img_width, img_height, 0, img_mono);
		DetectCircleMono(img_mono, img_width, img_height,
						 circular_ROI_radius,
						 circles);
		delete[] img_mono;
	}
	else
	{
		DetectCircleMono(img, img_width, img_height,
						 circular_ROI_radius,
						 circles);
	}
}

/*!
 * \brief DetectCircleMono
 * \param img_width
 * \param img_height
 * \param circular_ROI_radius
 */
void shapes::DetectCircleMono(
    unsigned char* mono_img,
    int img_width,
    int img_height,
    int circular_ROI_radius,
    std::vector<float>& circles)
{
    circles.erase(circles.begin(), circles.end());

    // create edge detection object
    CannyEdgeDetector *edge_detector = new CannyEdgeDetector();

    // copy the original image
    //unsigned char* img_mono = new unsigned char[img_width * img_height];
    //memcpy(img_mono, mono_img, img_width * img_height);

    // detect edges with canny algorithm
    edge_detector->Update(mono_img, img_width, img_height, 1);

    if (edge_detector->edges.size() > 0)
    {
        int half_width = img_width / 2;
        int half_height = img_height / 2;
        int circular_ROI_radius_sqr = circular_ROI_radius * circular_ROI_radius;

        int* Left = new int[img_height];
        int* Right = new int[img_height];
        int* Top = new int[img_width];
        int* Bottom = new int[img_width];

        for (int i = img_height-1; i >= 0; i--)
        {
            Left[i] = 0;
            Right[i] = 0;
        }
        for (int i = img_width-1; i >= 0; i--)
        {
            Top[i] = 0;
            Bottom[i] = 0;
        }

        for (int i = 0; i < (int)edge_detector->edges.size(); i += 2)
        {
            int x = edge_detector->edges[i];
            int y = edge_detector->edges[i + 1];

            bool valid_point = true;
            if (circular_ROI_radius > 0)
            {
                //  this point within the region of interest ?
                int dx = x - half_width;
                int dy = y - half_height;
                int dist_sqr = (dx * dx) + (dy * dy);
                if (dist_sqr > circular_ROI_radius_sqr) valid_point = false;
            }

            if (valid_point)
            {
                if ((Left[y] == 0) ||
                    (x < Left[y])) Left[y] = x;
                if ((Right[y] == 0) ||
                    (x > Right[y])) Right[y] = x;
                if ((Top[x] == 0) ||
                    (y < Top[x])) Top[x] = y;
                if ((Bottom[x] == 0) ||
                    (y > Bottom[x])) Bottom[x] = y;
            }
        }


        // convert integer list into floats
        std::vector<float> edges;
        int step_size = 4;
        int prev_x = -1;
        int prev_y = -1;
        for (int y = 0; y < img_height; y += step_size)
        {
            if (Left[y] > 0)
            {
                if (prev_y > -1)
                {
                    edges.push_back((float)prev_x);
                    edges.push_back((float)prev_y);
                    edges.push_back((float)Left[y]);
                    edges.push_back((float)y);
                }
                prev_x = Left[y];
                prev_y = y;
            }
        }
        prev_x = -1;
        prev_y = -1;
        for (int y = 0; y < img_height; y += step_size)
        {
            if (Right[y] > 0)
            {
                if (prev_y > -1)
                {
                    edges.push_back((float)prev_x);
                    edges.push_back((float)prev_y);
                    edges.push_back((float)Right[y]);
                    edges.push_back((float)y);
                }
                prev_x = Right[y];
                prev_y = y;
            }
        }
        prev_x = -1;
        prev_y = -1;
        for (int x = 0; x < img_width; x += step_size)
        {
            if (Top[x] > 0)
            {
                if (prev_x > -1)
                {
                    edges.push_back((float)prev_x);
                    edges.push_back((float)prev_y);
                    edges.push_back((float)x);
                    edges.push_back((float)Top[x]);
                }
                prev_x = x;
                prev_y = Top[x];
            }
        }
        prev_x = -1;
        prev_y = -1;
        for (int x = 0; x < img_width; x += step_size)
        {
            if (Bottom[x] > 0)
            {
                if (prev_x > -1)
                {
                    edges.push_back((float)prev_x);
                    edges.push_back((float)prev_y);
                    edges.push_back((float)x);
                    edges.push_back((float)Bottom[x]);
                }
                prev_x = x;
                prev_y = Bottom[x];
            }
        }

        circleDetector *circle_detector = new circleDetector();
        if (circle_detector->Find(edges, img_width, img_height))
        {
            circles.push_back(circle_detector->centre_x);
            circles.push_back(circle_detector->centre_y);
            circles.push_back(circle_detector->radius);
        }

        delete circle_detector;
        delete[] Left;
        delete[] Right;
        delete[] Top;
        delete[] Bottom;
    }

    //delete[] img_mono;
    delete edge_detector;
}

/*!
 * \brief DetectRectangles
 * \param img_colour colour image
 * \param img_width
 * \param img_height
 * \param bytes_per_pixel number of bytes per pixel
 * \param grouping_radius_percent
 * \param grouping_radius_percent_levels
 * \param erosion_dilation
 * \param erosion_dilation_levels
 * \param black_on_white
 * \param accuracy_level
 * \param debug
 * \param perimeter_detection_method
 * \param compression
 * \param no_of_compressions
 * \param use_perimeter_fitting whether to enable an additional attempt to fit the perimeter as closely as possible
 * \param perimeter_fit_threshold a threshold value used to fit the perimeter more closely
 * \param bestfit_tries number of tries when fitting the perimeter using different parameters
 * \param step_sizes array containing step sizes (in pixels) used when connecting line segments with GetGroups
 * \param no_of_step_sizes the number of step sizes in the array
 */
void shapes::DetectRectangles(
    unsigned char* img_colour,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int* grouping_radius_percent,
    int grouping_radius_percent_levels,
    int* erosion_dilation,
    int erosion_dilation_levels,
    bool black_on_white,
    int accuracy_level,
    float maximum_aspect_ratio,
    bool debug,
    int circular_ROI_radius,
    int perimeter_detection_method,
    int* compression,
    int no_of_compressions,
    int minimum_volume_percent,
    int maximum_volume_percent,
    bool use_perimeter_fitting,
    int perimeter_fit_threshold,
    int bestfit_tries,
    int* step_sizes,
    int no_of_step_sizes,
    int maximum_groups,
    std::vector<int>& edges,
    std::vector<float>& orientation,
    std::vector<std::vector<int> >& dominant_edges,
    std::vector<std::vector<std::vector<int> > >& side_edges,
    unsigned char* edges_image,
    int& edges_image_width,
    int& edges_image_height,
    CannyEdgeDetector *edge_detector,
    std::vector<polygon2D*>& rectangles,
    std::vector<unsigned char*>& debug_images,
    unsigned char* erosion_dilation_buffer,
    int* downsampling_buffer0,
    int* downsampling_buffer1)
{
    int downsampled_width = img_width;
    switch (accuracy_level)
    {
        case 0: { downsampled_width = 320; break; }
        case 1: { downsampled_width = 480; break; }
        case 2: { downsampled_width = 640; break; }
    }
    DetectSquares(
        img_colour, img_width, img_height, bytes_per_pixel,
        false, 0,
        grouping_radius_percent, grouping_radius_percent_levels,
        erosion_dilation, erosion_dilation_levels,
        black_on_white,
        1.0f / maximum_aspect_ratio, maximum_aspect_ratio,
        downsampled_width, false, debug,
        circular_ROI_radius,
        perimeter_detection_method,
        compression, no_of_compressions,
        minimum_volume_percent,
        maximum_volume_percent,
        use_perimeter_fitting,
        perimeter_fit_threshold,
        bestfit_tries,
        step_sizes,
        no_of_step_sizes,
        maximum_groups,
        edges, orientation,
        dominant_edges,
        side_edges,
        edge_detector,
        edges_image, edges_image_width, edges_image_height,
        rectangles,
        debug_images,
        erosion_dilation_buffer,
        downsampling_buffer0,
        downsampling_buffer1);
}

/*!
 * \brief detects square regions within the given colour image
 * \param img_colour colour image data
 * \param img_width image width
 * \param img_height image height
 * \param bytes_per_pixel number of bytes per pixel
 * \param grouping_radius_percent radius used to group adjacent edge features
 * \param erosion_dilation erosion dilation level
 * \param black_on_white whether this image contains dark markings on a lighter background
 * \param accuracy_level desired level of accuracy  the range 0-2.  0 = least accurate but fastest
 * \param debug save extra debugging info
 * \param circular_ROI_radius radius of a circular region of interest
 * \param perimeter_detection_method method used to detect the perimeter (0 or 1)
 * \param minimum_volume_percent minimum volume of the square as a percentage of the image volume (prevents very small stuff being detected)
 * \param maximum_volume_percent maximum volume of the square as a percentage of the image volume (prevents very large stuff being detected)
 * \param use_perimeter_fitting whether to enable an additional attempt to fit the perimeter as closely as possible
 * \param perimeter_fit_threshold a threshold value used to fit the perimeter more closely
 * \param bestfit_tries number of tries when fitting the perimeter using different parameters
 * \param step_sizes array containing step sizes (in pixels) used when connecting line segments with GetGroups
 * \param no_of_step_sizes the number of step sizes in the array
 * \param edges edges within the image
 * \param orientation dominant orientation of the detected squares
 */
void shapes::DetectSquares(
    unsigned char* img_colour,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int* grouping_radius_percent,
    int grouping_radius_percent_levels,
    int* erosion_dilation,
    int erosion_dilation_levels,
    bool black_on_white,
    int accuracy_level,
    bool debug,
    int circular_ROI_radius,
    int perimeter_detection_method,
    int* compression,
    int no_of_compressions,
    int minimum_volume_percent,
    int maximum_volume_percent,
    bool use_perimeter_fitting,
    int perimeter_fit_threshold,
    int bestfit_tries,
    int* step_sizes,
    int no_of_step_sizes,
    int maximum_groups,
    std::vector<int>& edges,
    std::vector<float>& orientation,
    std::vector<std::vector<int> >& dominant_edges,
    std::vector<std::vector<std::vector<int> > >& side_edges,
    unsigned char* edges_image,
    int& edges_image_width,
    int& edges_image_height,
    CannyEdgeDetector *edge_detector,
    std::vector<polygon2D*>& squares,
    std::vector<unsigned char*>& debug_images,
    unsigned char* erosion_dilation_buffer,
    int* downsampling_buffer0,
    int* downsampling_buffer1)
{
    int downsampled_width = img_width;

    int minimum_width = 320;
    switch (accuracy_level)
    {
        case 0: { minimum_width = 320; break; }
        case 1: { minimum_width = 480; break; }
        case 2: { minimum_width = 640; break; }
    }
    int i = 0;
    while ((downsampled_width > minimum_width) &&
           (i <= accuracy_level))
    {
        downsampled_width /= 2;
        i++;
    }

    float minimum_aspect_ratio = 0.7f;
    float maximum_aspect_ratio = 1.3f;

    const bool ignore_periphery = true;
    const int image_border_percent = 1;

    DetectSquares(
        img_colour, img_width, img_height, bytes_per_pixel,
        ignore_periphery, image_border_percent,
        grouping_radius_percent, grouping_radius_percent_levels,
        erosion_dilation, erosion_dilation_levels,
        black_on_white,
        minimum_aspect_ratio, maximum_aspect_ratio,
        downsampled_width, true, debug,
        circular_ROI_radius,
        perimeter_detection_method,
        compression, no_of_compressions,
        minimum_volume_percent,
        maximum_volume_percent,
        use_perimeter_fitting,
        perimeter_fit_threshold,
        bestfit_tries,
        step_sizes,
        no_of_step_sizes,
        maximum_groups,
        edges, orientation,
        dominant_edges,
        side_edges,
        edge_detector,
        edges_image, edges_image_width, edges_image_height,
        squares,
        debug_images,
        erosion_dilation_buffer,
        downsampling_buffer0,
        downsampling_buffer1);
}

/*!
 * \brief detects square regions within the given colour image
 * \param img_colour colour image data
 * \param img_width image width
 * \param img_height image height
 * \param bytes_per_pixel number of bytes per pixel
 * \param ignore_periphery whether to ignore edge features which stray into the border of the image
 * \param image_border_percent percentage of the image considred to be the surrounding border
 * \param grouping_radius_percent radius used to group adjacent edge features
 * \param erosion_dilation erosion dilation level
 * \param black_on_white whether this image contains dark markings on a lighter background
 * \param minimum_aspect_ratio minimum aspect ratio when searching for valid regions
 * \param maximum_aspect_ratio maximum aspect ratio when searching for valid regions
 * \param downsampled_width width of the downsampled image
 * \param squares_only only search for areas with square aspect ratios
 * \param debug save extra debugging info
 * \param circular_ROI_radius radius of the circular region of interest
 * \param perimeter_detection_method method used to detect the perimeter (0 or 1)
 * \param minimum_volume_percent minimum volume of the square as a percentage of the image volume (prevents very small stuff being detected)
 * \param maximum_volume_percent maximum volume of the square as a percentage of the image volume (prevents very large stuff being detected)
 * \param use_perimeter_fitting whether to enable an additional attempt to fit the perimeter as closely as possible
 * \param perimeter_fit_threshold a threshold value used to fit the perimeter more closely
 * \param bestfit_tries number of tries when fitting the perimeter using different parameters
 * \param step_sizes array containing step sizes (in pixels) used when connecting line segments with GetGroups
 * \param no_of_step_sizes the number of step sizes in the array
 * \param edges edges detected within the image
 * \param orientation dominant orientations of detected squares
 */
void shapes::DetectSquares(
    unsigned char* img_colour,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    bool ignore_periphery,
    int image_border_percent,
    int* grouping_radius_percent,
    int grouping_radius_percent_levels,
    int* erosion_dilation,
    int erosion_dilation_levels,
    bool black_on_white,
    float minimum_aspect_ratio,
    float maximum_aspect_ratio,
    int downsampled_width,
    bool squares_only,
    bool debug,
    int circular_ROI_radius,
    int perimeter_detection_method,
    int* compression,
    int no_of_compressions,
    int minimum_volume_percent,
    int maximum_volume_percent,
    bool use_perimeter_fitting,
    int perimeter_fit_threshold,
    int bestfit_tries,
    int* step_sizes,
    int no_of_step_sizes,
    int maximum_groups,
    std::vector<int>& edges,
    std::vector<float>& orientation,
    std::vector<std::vector<int> >& dominant_edges,
    std::vector<std::vector<std::vector<int> > >& side_edges,
    CannyEdgeDetector *edge_detector,
    unsigned char* edges_image,
    int& edges_image_width,
    int& edges_image_height,
    std::vector<polygon2D*>& squares,
    std::vector<unsigned char*>& debug_images,
    unsigned char* erosion_dilation_buffer,
    int* downsampling_buffer0,
    int* downsampling_buffer1)
 {
    int original_img_width = img_width;

    unsigned char *img = img_colour;

    //printf("downsampled width: %d\n", downsampled_width);
    //printf("img width: %d\n", img_width);

    // downsample the original image
    bool remove_img = false;
    if (downsampled_width < img_width)
    {
        int downsampled_height = img_height * downsampled_width / img_width;
        img = new unsigned char[downsampled_width * downsampled_height * 3];
        remove_img = true;
        int factor = img_width / downsampled_width;

        if (img_width % downsampled_width == 0)
        {
            // if the desired downsampled width  an exact multiple of the
            // original width then use a simple downsampling method
            processimage::downSample(img_colour, img_width, img_height, bytes_per_pixel, factor, downsampling_buffer0, downsampling_buffer1, img);
        }
        else
        {
            processimage::downSample(img_colour, img_width, img_height, bytes_per_pixel,
                                     downsampled_width, img_height * downsampled_width / img_width, img);
        }

        img_height = img_height * downsampled_width / img_width;
        img_width = downsampled_width;
    }

    // convert the colour image to mono
    unsigned char* img_mono = new unsigned char[img_width * img_height];
    if (bytes_per_pixel == 1)
        memcpy(img_mono, img, img_width * img_height);
    else
        processimage::monoImage(img, img_width, img_height, 0, img_mono);

    edges_image_width = img_width;
    edges_image_height = img_height;

    DetectSquaresMono(
        img_mono, img_width, img_height,
        ignore_periphery,
        image_border_percent,
        grouping_radius_percent, grouping_radius_percent_levels,
        erosion_dilation, erosion_dilation_levels,
        black_on_white,
        true,
        minimum_aspect_ratio, maximum_aspect_ratio,
        squares_only, debug,
        circular_ROI_radius,
        perimeter_detection_method,
        compression, no_of_compressions,
        minimum_volume_percent, maximum_volume_percent,
        use_perimeter_fitting, perimeter_fit_threshold,
        bestfit_tries,
        step_sizes,
        no_of_step_sizes,
        maximum_groups,
        edges,
        orientation,
        dominant_edges,
        side_edges,
        edge_detector,
        edges_image,
        squares,
        debug_images,
        erosion_dilation_buffer);

    // re-scale points back into the original image resolution
    if (img_width < original_img_width)
    {
        for (int i = 0; i < (int)squares.size(); i++)
        {
            for (int vertex = 0; vertex < 4; vertex++)
            {
                squares[i]->x_points[vertex] = squares[i]->x_points[vertex] * original_img_width / img_width;
                squares[i]->y_points[vertex] = squares[i]->y_points[vertex] * original_img_width / img_width;
            }
        }
    }

    if (remove_img) delete[] img;
    delete[] img_mono;
}

int shapes::EdgeMagnitude(
	unsigned char* img_mono, int img_width, int img_height,
	float x0, float y0, float x1, float y1,
	float x2, float y2, float x3, float y3,
	int no_of_samples)
{
	int diff, magnitude = 0;
	float dx1 = x1 - x0;
	float dy1 = y1 - y0;
	float dx2 = x3 - x2;
	float dy2 = y3 - y2;

	int pixels = img_width * img_height;
	float incr = 0.5f / no_of_samples;

	for (int i = 0; i < no_of_samples; i++)
	{
		int sample_x = (int)(x0 + (dx1 * i * incr));
		int sample_y = (int)(y0 + (dy1 * i * incr));
		int n = (sample_y * img_width) + sample_x;
		if ((n > -1) && (n < pixels))
		{
			diff = img_mono[n];

			sample_x = (int)(x2 + (dx2 * i * incr));
			sample_y = (int)(y2 + (dy2 * i * incr));
			n = (sample_y * img_width) + sample_x;
			if ((n > -1) && (n < pixels))
			{
				diff -= img_mono[n];
				magnitude += diff*diff;
			}
		}
	}

	return(magnitude);
}

bool shapes::FitPerimeter(
	unsigned char* img_mono, int img_width, int img_height,
	polygon2D *perimeter,
	float fit_radius_pixels,
	int no_of_samples,
	int magnitude_threshold,
	float shrink_factor,
	float perimeter_fit_step_size)
{
	bool fitted = false;
	magnitude_threshold *= no_of_samples*2;

	// get the centre of the perimeter
	float centre_x=0, centre_y = 0;
	perimeter->GetSquareCentre(centre_x, centre_y);

	float next_x, next_y;

	float prev_x = perimeter->x_points[3];
	float prev_y = perimeter->y_points[3];
	for (int vertex = 0; vertex < 4; vertex++)
	{
		float x = perimeter->x_points[vertex];
		float y = perimeter->y_points[vertex];

		if (vertex < 3)
		{
			next_x = perimeter->x_points[vertex + 1];
			next_y = perimeter->y_points[vertex + 1];
		}
		else
		{
			next_x = perimeter->x_points[0];
			next_y = perimeter->y_points[0];
		}

		// search for a closer fit to the edge data
		int max_magnitude = 0;
		float best_x = x;
		float best_y = y;
		for (float yy = y - fit_radius_pixels; yy <= y + fit_radius_pixels; yy += perimeter_fit_step_size)
		{
			float dy = yy - centre_y;
			float yy2 = centre_y + (dy * shrink_factor);
			float dy2 = yy2 - yy;
			for (float xx = x - fit_radius_pixels; xx <= x + fit_radius_pixels; xx += perimeter_fit_step_size)
			{
				float dx = xx - centre_x;
				float xx2 = centre_x + (dx * shrink_factor);
				float dx2 = xx2 - xx;

				int magnitude = EdgeMagnitude(
					img_mono, img_width, img_height,
					xx, yy, next_x, next_y,
					xx2, yy2, next_x + dx2, next_y + dy2,
					no_of_samples);
				magnitude += EdgeMagnitude(
					img_mono, img_width, img_height,
					xx, yy, prev_x, prev_y,
					xx2, yy2, prev_x + dx2, prev_y + dy2,
					no_of_samples);

				if (magnitude > max_magnitude)
				{
					max_magnitude = magnitude;
					best_x = xx;
					best_y = yy;
				}
			}
		}

		// if there is a reasonable fit then re-position the vertex
		if (max_magnitude > magnitude_threshold)
		{
			perimeter->x_points[vertex] = best_x;
			perimeter->y_points[vertex] = best_y;
			fitted = true;
		}
		prev_x = x;
		prev_y = y;
	}
	return(fitted);
}

/*!
 * \brief Given three vertices predict the position of the fourth
 * \param perimeter perimeter to be used
 * \param vertex index of the vertex to be predicted (0-3)
 * \param vertex_x predicted x position of the vertex
 * \param vertex_y predicted y position of the vertex
 */
void shapes::PredictSquareVertex(
    polygon2D *perimeter, int vertex,
	float &vertex_x,
	float &vertex_y)
{
	// two baseline vertices
	int index1 = vertex + 1;
	int index2 = vertex + 3;

	// opposite vertex
	int index_opposite = vertex + 2;

	// range check
	if (index1 >= 4) index1 -= 4;
	if (index2 >= 4) index2 -= 4;
	if (index_opposite >= 4) index_opposite -= 4;

	// find the baseline centre point
	float dx = perimeter->x_points[index2] - perimeter->x_points[index1];
	float dy = perimeter->y_points[index2] - perimeter->y_points[index1];
	float centre_x = perimeter->x_points[index1] + (dx * 0.5f);
	float centre_y = perimeter->y_points[index1] + (dy * 0.5f);

	// distance of the opposite vertex from the centre
	dx = centre_x - perimeter->x_points[index_opposite];
	dy = centre_y - perimeter->y_points[index_opposite];

	// predict vertex position
	vertex_x = centre_x + dx;
	vertex_y = centre_y + dy;
}

/*!
 * \brief Finds the vertex which least resembles a right angle and attempts to correct it
 * \param perimeter original perimeter
 * \param approximated_perimeter corrected perimeter
 */
bool shapes::ApproximateSquare(
    polygon2D *perimeter,
	polygon2D *approximated_perimeter)
{
	bool approximated = true;
	const float vertex_adjustment_threshold_pixels = 1.0f;

	polygon2D *temp_perimeter = perimeter->Copy();

	float max_score = 0;
	int modified_vertex = -1;
	for (int vertex = 0; vertex < 4; vertex++)
	{
		float vertex_x = 0;
		float vertex_y = 0;
		PredictSquareVertex(perimeter, vertex, vertex_x, vertex_y);

		for (int vertex2 = 0; vertex2 < 4; vertex2++)
		{
		    temp_perimeter->x_points[vertex2] = perimeter->x_points[vertex2];
		    temp_perimeter->y_points[vertex2] = perimeter->y_points[vertex2];
		}
		temp_perimeter->x_points[vertex] = vertex_x;
		temp_perimeter->y_points[vertex] = vertex_y;
		float score = temp_perimeter->getSquareness();
		if (score > max_score)
		{
			modified_vertex = vertex;
			max_score = score;
			for (int vertex2 = 0; vertex2 < 4; vertex2++)
			{
				approximated_perimeter->x_points[vertex2] = temp_perimeter->x_points[vertex2];
				approximated_perimeter->y_points[vertex2] = temp_perimeter->y_points[vertex2];
			}
		}
	}
	delete temp_perimeter;

    // has the position of the modified vertex changed significantly?
    // if not then this is effectively the same as the original perimeter
	if (modified_vertex > -1)
	{
        float dx = ABS(approximated_perimeter->x_points[modified_vertex] - perimeter->x_points[modified_vertex]);
		if (dx < vertex_adjustment_threshold_pixels)
		{
		    approximated = false;
		}
		else
		{
            float dy = ABS(approximated_perimeter->y_points[modified_vertex] - perimeter->y_points[modified_vertex]);
		    if (dy < vertex_adjustment_threshold_pixels)
		    {
		        approximated = false;
		    }
		}
	}
	return(approximated);
}

void shapes::MostSquare(
    std::vector<polygon2D*> &square_shapes,
    std::vector<float>& orientation,
    int max_squares)
{
	float* squareness;

	if ((int)square_shapes.size() > max_squares)
	{
		// array to store squareness value for each shape
		squareness = new float[square_shapes.size()];

		// update squareness
		for (int i = (int)square_shapes.size()-1; i >= 0; i--)
			squareness[i] = square_shapes[i]->getSquareness();

		int squares_remaining = (int)square_shapes.size();
		while (squares_remaining > max_squares)
		{
			float min_squareness = 999999; // some large value
			int victim_index = 0;

			// find the shape most different from a square
			for (int i = (int)square_shapes.size()-1; i >= 0; i--)
			{
				if (squareness[i] > -1)
				{
					if (squareness[i] < min_squareness)
					{
						victim_index = i;
						min_squareness = squareness[i];
					}
				}
			}
			squareness[victim_index] = -1;
			squares_remaining--;
		}

		// remove shapes with low squareness values
		for (int i = (int)square_shapes.size()-1; i >= 0; i--)
		{
			if (squareness[i] < 0)
			{
                square_shapes.erase(square_shapes.begin()+i);
                orientation.erase(orientation.begin()+i);
			}
		}

		delete[] squareness;
	}

	// sort by squareness
	squareness = new float[square_shapes.size()];

	// update squareness values
	for (int i = (int)square_shapes.size()-1; i >= 0; i--)
		squareness[i] = square_shapes[i]->getSquareness();

	for (int i = 0; i < (int)square_shapes.size()-1; i++)
	{
		float max_squareness = squareness[i];
		int index = -1;
		for (int j = i+1; j < (int)square_shapes.size()-1; j++)
		{
			if (squareness[j] > squareness[i])
			{
				max_squareness = squareness[j];
				index = j;
			}
		}
		if (index > -1)
		{
			polygon2D *temp_shape = square_shapes[i];
			float temp_orient = orientation[i];
			float temp_sq = squareness[i];
			square_shapes[i] = square_shapes[index];
			square_shapes[index] = temp_shape;
			orientation[i] = orientation[index];
			orientation[index] = temp_orient;
			squareness[i] = squareness[index];
			squareness[index] = temp_sq;
		}
	}

	delete[] squareness;
}

/*!
 * \brief sorts perimeters into order of squareness
 * \param perimeters detected possible perimeters
 * \param orientation perimeter orientations
 */
void shapes::SortPerimeters(
    std::vector<polygon2D*> perimeters,
    std::vector<float> orientation)
{
    float* squareness = new float[(int)perimeters.size()];

    // calculate the squareness of each perimeter
    for (int i = (int)perimeters.size()-1; i >= 0; i--)
    	squareness[i] = perimeters[i]->getSquareness();

    // sort by squareness (most square first)
    for (int i = 0; i < (int)perimeters.size()-1; i++)
    {
    	float max_squareness = -1;
    	int index = -1;
    	for (int j = i+1; j < (int)perimeters.size(); j++)
    	{
    	    if (squareness[j] > max_squareness)
    	    {
    	    	max_squareness = j;
    	    	index = j;
    	    }
    	}
    	if (index > -1)
    	{
    	    polygon2D* temp_perimeter = perimeters[i];
    	    perimeters[i] = perimeters[index];
    	    perimeters[index] = temp_perimeter;
    	    float temp_orientation = orientation[i];
    	    orientation[i] = orientation[index];
    	    orientation[index] = temp_orientation;
    	}
    }

    delete[] squareness;
}



/*!
 * \brief detects square shapes within the given mono image
 * \param img_mono mono image data
 * \param img_width image width
 * \param img_height image height
 * \param ignore_periphery whether to ignore edge features which stray into the border of the image
 * \param image_border_percent percentage of the image considred to be the surrounding border
 * \param grouping_radius_percent radius used to group adjacent edge features
 * \param erosion_dilation erosion dilation level
 * \param black_on_white whether this image contains dark markings on a lighter background
 * \param use_original_image whether to allow img_mono to be altered
 * \param minimum_aspect_ratio minimum aspect ratio when searching for valid regions
 * \param maximum_aspect_ratio maximum aspect ratio when searching for valid regions
 * \param squares_only only look for square aspect ratios
 * \param debug save extra debugging info
 * \param circular_ROI_radius radius of a circular region of interest
 * \param perimeter_detection_method the method used to detect the perimeter (0 or 1)
 * \param minimum_volume_percent minimum volume of the square as a percentage of the image volume (prevents very small stuff being detected)
 * \param maximum_volume_percent maximum volume of the square as a percentage of the image volume (prevents very large stuff being detected)
 * \param use_perimeter_fitting whether to enable an additional attempt to fit the perimeter as closely as possible
 * \param perimeter_fit_threshold a threshold value used to fit the perimeter more closely
 * \param bestfit_tries number of tries when fitting a perimeter using different fitting parameters
 * \param step_sizes array containing step sizes (in pixels) used when connecting line segments with GetGroups
 * \param no_of_step_sizes the number of step sizes in the array
 * \param edges edges detected within the image
 * \param orientation dominant orientations
 */
void shapes::DetectSquaresMono(
    unsigned char* mono_img,
    int img_width,
    int img_height,
    bool ignore_periphery,
    int image_border_percent,
    int* grouping_radius_percent,
    int grouping_radius_percent_levels,
    int* erosion_dilation,
    int erosion_dilation_levels,
    bool black_on_white,
    bool use_original_image,
    float minimum_aspect_ratio,
    float maximum_aspect_ratio,
    bool squares_only,
    bool debug,
    int circular_ROI_radius,
    int perimeter_detection_method,
    int* compression,
    int no_of_compressions,
    int minimum_volume_percent,
    int maximum_volume_percent,
    bool use_perimeter_fitting,
    int perimeter_fit_threshold,
    int bestfit_tries,
    int* step_sizes,
    int no_of_step_sizes,
    int maximum_groups,
    std::vector<int>& edges,
    std::vector<float>& orientation,
    std::vector<std::vector<int> >& dominant_edges,
    std::vector<std::vector<std::vector<int> > >& side_edges,
    CannyEdgeDetector *edge_detector,
    unsigned char* edges_image,
    std::vector<polygon2D*>& square_shapes,
    std::vector<unsigned char*>& debug_images,
    unsigned char* erosion_dilation_buffer)
{
	const float orientation_histogram_quantization_degrees = 3.5f;
	int no_of_perimeter_sampling_step_sizes = 2;
	int perimeter_sampling_step_sizes[] = { 6, 5 };

	// clear lists
    edges.erase(edges.begin(), edges.end());
    orientation.erase(orientation.begin(), orientation.end());
    dominant_edges.erase(dominant_edges.begin(), dominant_edges.end());
    side_edges.erase(side_edges.begin(), side_edges.end());
    square_shapes.erase(square_shapes.begin(), square_shapes.end());

	int *left = new int[img_height];
	int *right = new int[img_height];
	int *top = new int[img_width];
	int *bottom = new int[img_width];

	int *histogram = new int[360/2];
	const int histogram_members_length = (360/2) * 4;
	histogram_element** histogram_members = new histogram_element*[histogram_members_length];
	for (int i = histogram_members_length-1; i >= 0; i--)
		histogram_members[i] = new histogram_element();

    float best_fit_max_deviation_pixels = 6;
    int best_fit_baseline_pixels = 5;

    // create some buffers which will be used by GetGroups
    // this saves having to repeatedly allocate and deallocate memory
    int least_compression = 99999999;
    for (int i = 0; i < no_of_compressions; i++)
    	if (compression[i] < least_compression) least_compression = compression[i];
    int* line_segment_map_buffer = new int[(((img_width * least_compression / 1000) + 2) *
    		                               ((img_height * least_compression / 1000) + 2)) * 3];

    int pixels = img_width * img_height;

    const int minimum_size_percent = 5;
    int connect_edges_radius = 5;

    dominant_edges.erase(dominant_edges.begin(), dominant_edges.end());
    side_edges.erase(side_edges.begin(), side_edges.end());

    unsigned char* img_mono = mono_img;
    if (!use_original_image)
    {
        // make a copy of the original image (we don't want to alter it)
        img_mono = new unsigned char[img_width * img_height];
        memcpy(img_mono, mono_img, img_width * img_height);
    }

    // if the image contains light markings on a darker background invert the pixels
    if (!black_on_white)
    {
        for (int i = pixels - 1; i >= 0; i--)
        {
            img_mono[i] = (unsigned char)(255 - img_mono[i]);
        }
    }

    // for debugging purposes store the original image
    if (debug)
    {
        unsigned char *img_debug_colour = new unsigned char[img_width * img_height * 3];
        processimage::colourImage(img_mono, img_width, img_height, img_debug_colour);
        debug_images.push_back(img_debug_colour);
    }

    int image_border = img_width * image_border_percent / 100;

    unsigned char* img_mono2 = NULL;
    unsigned char* eroded_dilated = NULL;
    bool previous_eroded = false;
    bool previous_dilated = false;

    unsigned char *buffer = erosion_dilation_buffer;

    //  each erosion/dilation level
    for (int erosion_dilation_level = 0; erosion_dilation_level < erosion_dilation_levels; erosion_dilation_level++)
    {
        // erode
        if (erosion_dilation[erosion_dilation_level] > 0)
        {
            if (previous_eroded)
            {
                processimage::Erode(eroded_dilated, img_width, img_height,
                                    buffer,
                                    erosion_dilation[erosion_dilation_level] - erosion_dilation[erosion_dilation_level - 1],
                                    eroded_dilated);
            }
            else
            {
                if (eroded_dilated == NULL) eroded_dilated = new unsigned char[img_width * img_height];
                processimage::Erode(img_mono, img_width, img_height,
                                    buffer,
                                    erosion_dilation[erosion_dilation_level],
                                    eroded_dilated);
            }

            img_mono2 = eroded_dilated;
            previous_eroded = true;
        }

        // dilate
        if (erosion_dilation[erosion_dilation_level] < 0)
        {
            if (previous_dilated)
            {
                processimage::Dilate(eroded_dilated, img_width, img_height,
                                     buffer,
                                     -erosion_dilation[erosion_dilation_level] + erosion_dilation[erosion_dilation_level - 1],
                                     eroded_dilated);
            }
            else
            {
                if (eroded_dilated == NULL) eroded_dilated = new unsigned char[img_width * img_height];
                processimage::Dilate(img_mono, img_width, img_height,
                                     buffer,
                                     -erosion_dilation[erosion_dilation_level],
                                     eroded_dilated);
            }
            img_mono2 = eroded_dilated;
            previous_dilated = true;
        }

        // just copy the original image
        if (erosion_dilation[erosion_dilation_level] == 0)
        {
            img_mono2 = img_mono;
            previous_eroded = false;
            previous_dilated = false;
        }

        // for debugging purposes store the image after erosion / dilation
        if (debug)
        {
            unsigned char *img_debug_colour = new unsigned char[img_width * img_height * 3];
            processimage::colourImage(img_mono2, img_width, img_height, img_debug_colour);
            debug_images.push_back(img_debug_colour);
        }

        // detect edges with canny algorithm
        edges_image = edge_detector->Update(img_mono2, img_width, img_height, 1);

        // for debugging purposes store the edges image
        if (debug)
        {
            unsigned char *img_debug_colour = new unsigned char[img_width * img_height * 3];
            processimage::colourImage(edges_image, img_width, img_height, img_debug_colour);
            debug_images.push_back(img_debug_colour);
        }

        // connect edges which are a short distance apart
        edge_detector->ConnectBrokenEdges(connect_edges_radius, img_width, img_height, 1);

        // remove edges which are outside a circular region of interest
        if (circular_ROI_radius > 0)
        {
            int circular_ROI_radius_sqr = circular_ROI_radius * circular_ROI_radius;
            int half_width = img_width / 2;
            int half_height = img_height / 2;
            for (int i = (int)edge_detector->edges.size() - 2; i >= 0; i -= 2)
            {
                int x = edge_detector->edges[i];
                int y = edge_detector->edges[i + 1];
                int dx = x - half_width;
                int dy = y - half_height;
                int dist_sqr = (dx * dx) + (dy * dy);
                if (dist_sqr > circular_ROI_radius_sqr)
                {
                    edge_detector->edges.erase(edge_detector->edges.begin() + i + 1);
                    edge_detector->edges.erase(edge_detector->edges.begin() + i);
                }
            }
        }
        edges = edge_detector->edges;

        //  different groupings
        for (int group_radius_index = 0; group_radius_index < grouping_radius_percent_levels; group_radius_index++)
        {
            // group edges together into objects
            std::vector<std::vector<int> > groups;
            GetGroups(
                edge_detector->edges,
                img_width, img_height, image_border,
                minimum_size_percent,
                false, maximum_aspect_ratio,
                ignore_periphery,
                grouping_radius_percent[group_radius_index],
                compression, no_of_compressions,
                groups,
                line_segment_map_buffer,
                step_sizes,
                no_of_step_sizes);

            int no_of_groups = (int)groups.size();

            if ((no_of_groups > 0) && (no_of_groups < maximum_groups))
            {
                // for debugging purposes show the detected groups
                if (debug)
                {
                    unsigned char *img_debug_colour = new unsigned char[img_width * img_height * 3];
                    ShowGroups(groups, img_width, img_height, img_debug_colour);
                    debug_images.push_back(img_debug_colour);
                }

                // get the set of edges with aspect ratio closest to square
                std::vector<std::vector<int> > squares;
                GetValidGroups(
                    groups,
                    img_width, img_height,
                    minimum_size_percent,
                    squares);

                unsigned char *img_debug_squares = NULL;
                if (debug)
                {
                    img_debug_squares = new unsigned char[img_width * img_height * 3];

                    for (int i = (img_width * img_height * 3)-1; i >= 0; i--)
                        img_debug_squares[i] = (unsigned char)255;

                    debug_images.push_back(img_debug_squares);
                }

                int detected_squares = (int)squares.size();
                for (int i = detected_squares - 1; i >= 0; i--)
                {
                    // display the square-looking areas under consideration
                    if (debug)
                    {
                        // use different colours to distinguish each square region
                        unsigned char r=0, g=0, b=0;
                        switch(i % 6)
                        {
                            case 0: { r = 255; g = 0; b = 0; break; }
                            case 1: { r = 0; g = 255; b = 0; break; }
                            case 2: { r = 0; g = 0; b = 255; break; }
                            case 3: { r = 255; g = 255; b = 0; break; }
                            case 4: { r = 0; g = 255; b = 255; break; }
                            case 5: { r = 255; g = 0; b = 255; break; }
                        }

                        for (int j = (int)squares[i].size() - 2; j >= 0; j -= 2)
                        {
                            int x = squares[i][j];
                            int y = squares[i][j + 1];
                            int n = ((y * img_width) + x) * 3;
                            img_debug_squares[n] = b;
                            img_debug_squares[n+1] = g;
                            img_debug_squares[n+2] = r;
                        }
                    }

                    // get the bounding box coordinates
                    // and edge positions along the left, right, top and bottom sides
					int tx = 0, ty = 0, bx = 0, by = 0;
					GetPeripheralEdges(
						squares[i],
						tx, ty, bx, by,
						left, right, top, bottom);

					// width and height of the bounding box around the edges
			        int horizontal_length = bx - tx + 1;
			        int vertical_length = by - ty + 1;

			        // try detecting the dominant orientation and side edges using
			        // a few different sampling step sizes
                    for (int dorient = 0; dorient < no_of_perimeter_sampling_step_sizes; dorient++)
                    {
    					std::vector<int> curr_dominant_edges;
    					std::vector<int> curr_non_dominant_edges;
    					std::vector<std::vector<int> > curr_side_edges;
						float dominant_orientation =
							DominantOrientation(
							    tx, ty,
								left, vertical_length,
								right, vertical_length,
								top, horizontal_length,
								bottom, horizontal_length,
								curr_dominant_edges,
								curr_non_dominant_edges,
								curr_side_edges,
								orientation_histogram_quantization_degrees,
								perimeter_sampling_step_sizes[dorient],
								histogram,
								histogram_members);

						// try a couple of different settings
						// for the best fit line parameters
						for (int bestfit = 0; bestfit < bestfit_tries; bestfit++)
						{
							switch(bestfit)
							{
								case 0:
								{
									// parameter set 3
									best_fit_max_deviation_pixels = 8;
									best_fit_baseline_pixels = 10;
									break;
								}
							    case 1:
							    {
									// parameter set 0
									best_fit_max_deviation_pixels = 6;
									best_fit_baseline_pixels = 5;
							    	break;
							    }
							    case 2:
							    {
								    // parameter set 1
								    best_fit_max_deviation_pixels = 1;
								    best_fit_baseline_pixels = 4;
								    break;
							    }
							    case 3:
							    {
								    // parameter set 2
								    best_fit_max_deviation_pixels = 2;
								    best_fit_baseline_pixels = 5;
								    break;
							    }
							}

							// fit lines to the edges along each side
							// and return a perimeter polygon
							polygon2D *perim = new polygon2D();
							std::vector<std::vector<int> > periphery;
							GetPeripheral(
								perimeter_detection_method,
								erosion_dilation[erosion_dilation_level],
								best_fit_max_deviation_pixels,
								best_fit_baseline_pixels,
								tx, ty, bx, by,
								left, right,
								top, bottom,
								perim,
								curr_side_edges,
								periphery);

							dominant_edges.push_back(curr_dominant_edges);
							dominant_edges.push_back(curr_non_dominant_edges);
							side_edges.push_back(curr_side_edges);

							int perim_points = (int)perim->x_points.size();
							bool added = false;
							if (perim_points == 4)
							{
								float longest_side = perim->getLongestSide();
								float shortest_side = perim->getShortestSide();

								float aspect = shortest_side / longest_side;

								if ((aspect > minimum_aspect_ratio) &&
									(aspect < maximum_aspect_ratio))
								{
									float volume_percent = shortest_side * 100 / img_width;

									// not too small
									if ((volume_percent > minimum_volume_percent) &&
										(volume_percent < maximum_volume_percent))
									{
										// check the angles
										bool angle_out_of_range = false;
										int vertex = 0;
										while ((vertex < (int)perim->x_points.size()) &&
											   (!angle_out_of_range) && (vertex < 4))
										{
											float angle = perim->GetInteriorAngle(vertex);
											angle = angle / (float)PI * 180;
											if ((angle < 70) || (angle > 110)) angle_out_of_range = true;
											//if ((angle < 85) || (angle > 95)) angle_out_of_range = true;
											vertex++;
										}

										if (!angle_out_of_range)
										{
											float aspect1 = perim->getSideLength(0) / perim->getSideLength(2);
											if ((aspect1 > minimum_aspect_ratio) && (aspect1 < maximum_aspect_ratio))
											{
												float aspect2 = perim->getSideLength(1) / perim->getSideLength(3);
												if ((aspect2 > minimum_aspect_ratio) && (aspect2 < maximum_aspect_ratio))
												{
													// try to fit a more accurate perimeter
													if (use_perimeter_fitting)
													{
														polygon2D *perim_fitted = perim->Copy();

														// additional perimeter fit to the image data
														float perimeter_fit_step_size = 0.5f;
														int perimeter_fit_no_of_samples = 10;
														int perimeter_fit_radius_pixels = 2;
														float perimeter_fit_shrink_factor = 2.0f / (int)perim->getLongestSide();

														if (FitPerimeter(
															mono_img, img_width, img_height,
															perim_fitted,
															perimeter_fit_radius_pixels,
															perimeter_fit_no_of_samples,
															perimeter_fit_threshold,
															perimeter_fit_shrink_factor,
															perimeter_fit_step_size))
														{
															square_shapes.push_back(perim_fitted);
															orientation.push_back(dominant_orientation);

															// try to fix situations where one corner of the square
															// has not been properly detected
															polygon2D *perim_corrected = perim_fitted->Copy();
															if (ApproximateSquare(perim_fitted, perim_corrected))
															{
																square_shapes.push_back(perim_corrected);
																orientation.push_back(dominant_orientation);
															}
															else
															{
																delete perim_corrected;
															}
														}
														else
														{
															delete perim_fitted;
														}

														// try to fix situations where one corner of the square
														// has not been properly detected
														polygon2D *perim_corrected2 = perim->Copy();
														if (ApproximateSquare(perim, perim_corrected2))
														{
															square_shapes.push_back(perim_corrected2);
															orientation.push_back(dominant_orientation);
														}
														else
														{
															delete perim_corrected2;
														}
													}

													// perimeter without any additional fitting
													square_shapes.push_back(perim);
													orientation.push_back(dominant_orientation);
													added = true;
												}
											}

										}
									}
								}
							}

							if (!added)
							{
								delete perim;
							}
						}
                    }
                }

                if (debug)
                {
                    debug_images.push_back(img_debug_squares);
                }

            }

            // clear out the contents of the group
            for (int i = (int)groups.size() - 1; i >= 0; i--)
                groups[i].clear();
            groups.clear();
        }
    }

    // limit the number of square shapes returned to those which look most like squares
    //MostSquare(square_shapes, orientation, 100);

    // destroy all monsters
    if (eroded_dilated != NULL) delete[] eroded_dilated;
    if (!use_original_image) delete[] img_mono;
    delete[] line_segment_map_buffer;
    delete[] left;
    delete[] right;
    delete[] top;
    delete[] bottom;
    delete[] histogram;
    for (int i = histogram_members_length-1; i >= 0; i--)
    	delete histogram_members[i];
    delete[] histogram_members;
}

/*!
 * \brief GetAspectRange
 * \param groups
 * \param img_width
 * \param img_height
 * \param minimum_aspect
 * \param maximum_aspect
 * \param minimum_size_percent
 * \param squares_only
 */
void shapes::GetAspectRange(
    std::vector<std::vector<int> > &groups,
    int img_width,
    int img_height,
    float minimum_aspect,
    float maximum_aspect,
    int minimum_size_percent,
    bool squares_only,
    std::vector<std::vector<int> >& results)
{
    results.erase(results.begin(), results.end());

    // minimum size  pixels
    int minimum_size = minimum_size_percent * img_width / 100;

    //float ideal_aspect = minimum_aspect + ((maximum_aspect - minimum_aspect) / 2.0f);
    //float minimum_diff_from_ideal = 9999;
    //float maximum_volume = 0;

    for (int i = 0; i < (int)groups.size(); i++)
    {
        int tx = img_width - 1;
        int ty = img_height - 1;
        int bx = 0;
        int by = 0;
        for (int j = 0; j < (int)groups[i].size(); j += 2)
        {
            int x = groups[i][j];
            int y = groups[i][j + 1];
            if (x < tx) tx = x;
            if (y < ty) ty = y;
            if (x > bx) bx = x;
            if (y > by) by = y;
        }

        int dx = bx - tx;
        int dy = by - ty;

        if ((dx > minimum_size) &&
            (dy > minimum_size))
        {
            float aspect_ratio = 0;

            if (dy > dx)
                aspect_ratio = dx / (float)dy;
            else
                aspect_ratio = dy / (float)dx;

            if ((aspect_ratio > minimum_aspect) &&
                (aspect_ratio < maximum_aspect))
            {
                bool is_valid = true;
                if ((!squares_only) && (aspect_ratio > 0.8f) && (aspect_ratio < 1.2f))
                    is_valid = false;

                if (is_valid)
                {
                    results.push_back(groups[i]);
                }
            }
        }

    }
}

/*!
 * \brief GetGroups
 * \param edges
 * \param img_width
 * \param img_height
 * \param image_border
 * \param minimum_size_percent
 * \param squares_only
 * \param ignore_periphery
 * \param grouping_radius_percent
 * \param compression
 * \param no_of_compressions
 */
void shapes::GetValidGroups(
    std::vector<std::vector<int> > &groups,
    int img_width,
    int img_height,
    int minimum_size_percent,
    std::vector<std::vector<int> >& results)
{
    results.erase(results.begin(), results.end());

    // minimum size in pixels
    int minimum_size = minimum_size_percent * img_width / 100;

    for (int i = 0; i < (int)groups.size(); i++)
    {
        int tx = img_width - 1;
        int ty = img_height - 1;
        int bx = 0;
        int by = 0;
        for (int j = 0; j < (int)groups[i].size(); j += 2)
        {
            int x = groups[i][j];
            int y = groups[i][j + 1];
            if (x < tx) tx = x;
            if (y < ty) ty = y;
            if (x > bx) bx = x;
            if (y > by) by = y;
        }

        int dx = bx - tx;
        int dy = by - ty;

        if ((dx > minimum_size) &&
            (dy > minimum_size))
        {
            results.push_back(groups[i]);
        }

    }
}

/*!
 * \brief turns edges into line segments, then groups the line segments together into distinct objects
 * \param edges detected edges
 * \param image_width width of the image
 * \param image_height height of the image
 * \param image_border border around the image
 * \param minimum_size_percent minimum group size as a percent of the max size found
 * \param squares_only limit only to square aspect ratios
 * \param max_rectangular_aspect largest aspect ratio to use when searching for rectangular shapes
 * \param ignore_periphery ignore line segments which stray into the border
 * \param grouping_radius_percent radius to use for grouping as a percentage of the image width in the range 0-1000
 * \param compression compression factors x 1000
 * \param no_of_compressions number of compression factors
 * \param groups returned groups
 * \param line_segment_map_buffer
 */
void shapes::GetGroups(
    std::vector<int> &edges,
    int img_width,
    int img_height,
    int image_border,
    int minimum_size_percent,
    bool squares_only,
    float max_rectangular_aspect,
    bool ignore_periphery,
    int grouping_radius_percent,
    int* compression,
    int no_of_compressions,
    std::vector<std::vector<int> >& groups,
    int* line_segment_map_buffer,
    int* step_sizes,
    int no_of_step_sizes)
{
    // ensure that the list is empty
    groups.erase(groups.begin(), groups.end());

    // unique identifier for each group
    // this is used to ensure that there are no duplicates
    std::vector<std::string> groups_identifiers;

    // find line segments of significant length
    std::vector<float> centres;
    std::vector<float> bounding_boxes;
    std::vector<std::vector<int> > line_segments;
    DetectLongestPerimeters(
        edges,
        img_width, img_height,
        image_border,
        minimum_size_percent,
        squares_only,
        max_rectangular_aspect,
        ignore_periphery,
        centres,
        bounding_boxes,
        line_segments);

    int grouping_matrix_dimension = line_segments.size();

    int grouping_matrix_length = grouping_matrix_dimension * grouping_matrix_dimension;
    bool* grouping_matrix  = new bool[grouping_matrix_length];
    int prev_grouping_radius = -1;

    // for each possible compression value
    for (int c = 0; c < no_of_compressions; c++)
    {
    	float compression_factor = compression[c] / 1000.0f;
    	float inv_compression_factor = 1.0f / compression_factor;

        // calculate a grouping radius in pixels based upon the compression value
        int grouping_radius = (int)(grouping_radius_percent * img_width / (1000.0f * compression_factor));
        if (grouping_radius < 1) grouping_radius = 1;

        if (grouping_radius != prev_grouping_radius)
        {
            for (int s = 0; s < no_of_step_sizes; s++)
            {
				// stepping speeds things up, at the cost of some accuracy
				int step_size = step_sizes[s];
				//if (grouping_matrix_dimension < 20) step_size = 2;
				if ((grouping_matrix_dimension >= 20) && (step_size < 4)) step_size = 4;

				// clear grouping
				for (int i = grouping_matrix_length-1; i >= 0; i--)
					grouping_matrix[i] = false;

				// map the line segments
				int ty = img_height - 1;
				int by = 0;
				int tx = img_width - 1;
				int bx = 0;

				// the line segment map is a 2D array, smaller
				// than the original image containing
				// line segment IDs
				int lsm_width = (int)(img_width / compression_factor) + 2;
				int lsm_height = (int)(img_height / compression_factor) + 2;
				int line_segment_map_length = lsm_width * lsm_height * 3;
				int* line_segment_map = line_segment_map_buffer;
				if (line_segment_map_buffer == NULL)
					line_segment_map = new int[line_segment_map_length];
				for (int i = line_segment_map_length-1; i >= 0; i--)
					line_segment_map[i] = 0;

				int edge_x, edge_y;

				for (int i = grouping_matrix_dimension - 1; i >= 0; i--)
				{
					int line_segment_ID = i + 1;

					int segment_len = (int)line_segments[i].size();
					for (int j = segment_len - step_size; j >= 0; j -= step_size)
					{
						if (j - step_size >= 0)
						{
							edge_x = line_segments[i][j];
							edge_y = line_segments[i][j + 1];
						}
						else
						{
							edge_x = line_segments[i][0];
							edge_y = line_segments[i][1];
						}

						int x = (int)((edge_x + 0.5f) * inv_compression_factor);
						int y = (int)((edge_y + 0.5f) * inv_compression_factor);

						// a zero value on the line segment map indicates
						// that no line segment ID has been assigned at this coordinate
						int nn = ((y * lsm_width) + x) * 3;
						if (nn < line_segment_map_length-3)
						{
							int k = 0;
							while (k < 3)
							{
								if (line_segment_map[nn + k] > 0)
								{
									// the ID should not be the same as the current one
									if (line_segment_map[nn + k] != line_segment_ID)
									{
										// array indexes of the line segments to be linked
										int segment_index1 = line_segment_map[nn + k] - 1;
										int segment_index2 = i;

										// link the two segments by setting bits within the grouping matrix
										int n = (segment_index2 * grouping_matrix_dimension) + segment_index1;
										grouping_matrix[n] = true;

										int n2 = (segment_index1 * grouping_matrix_dimension) + segment_index2;
										grouping_matrix[n2] = true;
									}
								}
								else break;
								k++;
							}

							// update the matrix
							if ((line_segment_map[nn] == 0) ||
								(line_segment_map[nn] == line_segment_ID))
								line_segment_map[nn] = line_segment_ID;
							else
							{
								if ((line_segment_map[nn + 1] == 0) || (line_segment_map[nn + 1] == line_segment_ID))
								{
									line_segment_map[nn + 1] = line_segment_ID;
								}
								else
								{
								    line_segment_map[nn + 2] = line_segment_ID;  // an ID has already been assigned here
								}
							}

							// update the bounding box
							if (x < tx) tx = x;
							if (x > bx) bx = x;
							if (y < ty) ty = y;
							if (y > by) by = y;
						}
					}
				}

				if (grouping_radius > 0)
				{

					// horizontal grouping
					for (int y = ty; y <= by; y++)
					{
						int prev_line_segment_ID = -1;
						int prev_line_segment_ID2 = -1;
						int prev_line_segment_ID3 = -1;
						int prev_segment_x = -1;
						int prev_segment_x2 = -1;
						int prev_segment_x3 = -1;
						for (int x = tx; x <= bx; x++)
						{
							int nn = ((y * lsm_width) + x) * 3;
							if (line_segment_map[nn] > 0)
							{
								int k = 0;
								while (k < 3)
								{
									int line_segment_ID = line_segment_map[nn + k];
									if (line_segment_ID > 0)
									{
										if (prev_segment_x > -1)
										{
											int dx = x - prev_segment_x;
											if (dx < grouping_radius)
											{
												if (line_segment_ID != prev_line_segment_ID)
												{
													// link the two segments by setting bits within the grouping matrix
													int n = ((prev_line_segment_ID-1) * grouping_matrix_dimension) + (line_segment_ID-1);
													grouping_matrix[n] = true;

													int n2 = ((line_segment_ID-1) * grouping_matrix_dimension) + (prev_line_segment_ID-1);
													grouping_matrix[n2] = true;
												}
											}
										}
										if (k == 0)
										{
											prev_segment_x = x;
											prev_line_segment_ID = line_segment_ID;
										}

										if (prev_segment_x2 > -1)
										{
											int dx = x - prev_segment_x2;
											if (dx < grouping_radius)
											{
												if (line_segment_ID != prev_line_segment_ID2)
												{
													// link the two segments by setting bits within the grouping matrix
													int n = ((prev_line_segment_ID2-1) * grouping_matrix_dimension) + (line_segment_ID-1);
													grouping_matrix[n] = true;

													int n2 = ((line_segment_ID-1) * grouping_matrix_dimension) + (prev_line_segment_ID2-1);
													grouping_matrix[n2] = true;
												}
											}
										}
										if (k == 1)
										{
											prev_segment_x2 = x;
											prev_line_segment_ID2 = line_segment_ID;
										}


										if (prev_segment_x3 > -1)
										{
											int dx = x - prev_segment_x3;
											if (dx < grouping_radius)
											{
												if (line_segment_ID != prev_line_segment_ID3)
												{
													// link the two segments by setting bits within the grouping matrix
													int n = ((prev_line_segment_ID3-1) * grouping_matrix_dimension) + (line_segment_ID-1);
													grouping_matrix[n] = true;

													int n2 = ((line_segment_ID-1) * grouping_matrix_dimension) + (prev_line_segment_ID3-1);
													grouping_matrix[n2] = true;
												}
											}
										}
										if (k == 2)
										{
											prev_segment_x3 = x;
											prev_line_segment_ID3 = line_segment_ID;
										}
									}
									else break;
									k++;
								}
							}
						}
					}

					// vertical grouping
					for (int x = tx; x <= bx; x++)
					{
						int prev_segment_y = -1;
						int prev_segment_y2 = -1;
						int prev_segment_y3 = -1;
						int prev_line_segment_ID = -1;
						int prev_line_segment_ID2 = -1;
						int prev_line_segment_ID3 = -1;
						for (int y = ty; y <= by; y++)
						{
							int nn = ((y * lsm_width) + x) * 3;
							if (line_segment_map[nn] > 0)
							{
								int k = 0;
								while (k < 3)
								{
									int line_segment_ID = line_segment_map[nn + k];
									if (line_segment_ID > 0)
									{
										if (prev_segment_y > -1)
										{
											int dy = y - prev_segment_y;
											if (dy < grouping_radius)
											{
												int n = ((prev_line_segment_ID-1) * grouping_matrix_dimension) + (line_segment_ID-1);
												if (line_segment_ID != prev_line_segment_ID)
												{
													// link the two segments by setting bits within the grouping matrix
													grouping_matrix[n] = true;

													int n2 = ((line_segment_ID-1) * grouping_matrix_dimension) + (prev_line_segment_ID-1);
													grouping_matrix[n2] = true;
												}
											}
										}
										if (k == 0)
										{
											prev_segment_y = y;
											prev_line_segment_ID = line_segment_ID;
										}

										if (prev_segment_y2 > -1)
										{
											int dy = y - prev_segment_y2;
											if (dy < grouping_radius)
											{
												if (line_segment_ID != prev_line_segment_ID2)
												{
													// link the two segments by setting bits within the grouping matrix
													int n = ((prev_line_segment_ID2-1) * grouping_matrix_dimension) + (line_segment_ID-1);
													grouping_matrix[n] = true;

													int n2 = ((line_segment_ID-1) * grouping_matrix_dimension) + (prev_line_segment_ID2-1);
													grouping_matrix[n2] = true;
												}
											}
										}
										if (k == 1)
										{
											prev_segment_y2 = y;
											prev_line_segment_ID2 = line_segment_ID;
										}

										if (prev_segment_y3 > -1)
										{
											int dy = y - prev_segment_y3;
											if (dy < grouping_radius)
											{
												if (line_segment_ID != prev_line_segment_ID3)
												{
													// link the two segments by setting bits within the grouping matrix
													int n = ((prev_line_segment_ID3-1) * grouping_matrix_dimension) + (line_segment_ID-1);
													grouping_matrix[n] = true;

													int n2 = ((line_segment_ID-1) * grouping_matrix_dimension) + (prev_line_segment_ID3-1);
													grouping_matrix[n2] = true;
												}
											}
										}
										if (k == 2)
										{
											prev_segment_y3 = y;
											prev_line_segment_ID3 = line_segment_ID;
										}
									}
									else break;
									k++;
								}
							}
						}
					}

				}

				// turn grouping matrix into a hypergraph
				int n = 0;
				hypergraph *graph = new hypergraph(grouping_matrix_dimension, 1);
				// note: these loops must be in ascending order
				for (int i = 0; i < grouping_matrix_dimension; i++)
				{
					for (int j = 0; j < grouping_matrix_dimension; j++, n++)
					{
						if (grouping_matrix[n])
							graph->LinkByIndex(j, i);
					}
				}

				// detect connected sets within the hypergraph
				int no_of_nodes = (int)graph->Nodes.size();
				for (int i = no_of_nodes - 1; i >= 0; i--)
				{
					bool already_grouped = graph->GetFlagByIndex(i, 0);
					if (!already_grouped)
					{
						std::vector<hypergraph_node*> connected_set;
						graph->PropogateFlagFromIndex(i, 0, connected_set, 9999);

						std::vector<int> member_IDs;
						std::vector<hypergraph_node*> members;
						int set_size = (int)connected_set.size();
						for (int j = 0; j < set_size; j++)
						{
							if (!VectorContains(members, connected_set[j]))
							{
								members.push_back(connected_set[j]);
								member_IDs.push_back(connected_set[j]->ID);
							}
						}

						// sort the line segment IDs into order
						std::sort(member_IDs.rbegin(), member_IDs.rend());

						// convert the ID numbers into an identifier string
						std::string group_identifier = "";
						for (int j = 0; j < (int)member_IDs.size(); j++)
						{
							std::stringstream str;
						    str << member_IDs[j];
						    string temp_id = "";
						    str >> temp_id;
						    group_identifier += temp_id;
						}

						// here we avoid creating duplicate groups by checking
						// that the group identifier does not already exist
						if (!VectorContains(groups_identifiers, group_identifier))
						{
							std::vector<int> group_members;
							int no_of_members = (int)members.size();
							for (int j = 0; j < no_of_members; j++)
							{
								int line_segment_index = members[j]->ID;

								// note that this loop must be in ascending order
								int len = (int)line_segments[line_segment_index].size();
								for (int k = 0; k < len; k += 2)
								{
									int x = line_segments[line_segment_index][k];
									int y = line_segments[line_segment_index][k + 1];

									group_members.push_back(x);
									group_members.push_back(y);
								}
							}

							groups_identifiers.push_back(group_identifier);
							groups.push_back(group_members);
						}
					}

				}
				delete graph;
				if (line_segment_map_buffer == NULL)
					delete[] line_segment_map;
            }
        }
        prev_grouping_radius = grouping_radius;

    }

   	delete[] grouping_matrix;
}

/*!
 * \brief ShowLongestPerimeters
 * \param edges
 * \param img_width
 * \param img_height
 * \param image_border
 * \param minimum_size_percent
 * \param squares_only
 * \param max_rectangular_aspect
 * \param max_search_depth
 * \param ignore_periphery
 * \param show_centres
 */
void shapes::ShowLongestPerimeters(
    std::vector<int> &edges,
    int img_width,
    int img_height,
    int image_border,
    int minimum_size_percent,
    bool squares_only,
    float max_rectangular_aspect,
    int max_search_depth,
    bool ignore_periphery,
    bool show_centres,
    unsigned char* result)
{
    for (int i = (img_width * img_height * 3) - 1; i >= 0; i--) result[i] = 255;

    std::vector<float> centres;
    std::vector<float> bounding_boxes;

    std::vector<std::vector<int> > perimeters;
    DetectLongestPerimeters(
        edges,
        img_width, img_height,
        image_border,
        minimum_size_percent,
        squares_only,
        max_rectangular_aspect,
        ignore_periphery,
        centres,
        bounding_boxes,
        perimeters);

    for (int i = 0; i < (int)perimeters.size(); i++)
    {
        for (int j = 0; j < (int)perimeters[i].size(); j += 2)
        {
            int x = perimeters[i][j];
            int y = perimeters[i][j + 1];
            int n = ((y * img_width) + x) * 3;
            for (int col = 0; col < 3; col++)
                result[n + col] = 0;
        }

        if (show_centres)
        {
            float x = centres[i * 2];
            float y = centres[(i * 2) + 1];
            drawing::drawCross(result, img_width, img_height,
                               (int)x, (int)y, 5,
                               255, 0, 0,
                               0);
        }
    }
}

/*!
 * \brief ShowEdges
 * \param edges
 * \param img_width
 * \param img_height
 */
void shapes::ShowEdges(
    std::vector<int> &edges,
    int img_width,
    int img_height,
    unsigned char* result)
{
    for (int i = (img_width * img_height * 3) - 1; i >= 0; i--) result[i] = 255;

    for (int i = 0; i < (int)edges.size(); i += 2)
    {
        int x = edges[i];
        int y = edges[i + 1];
        int n = ((y * img_width) + x) * 3;
        result[n] = 0;
        result[n + 1] = 0;
        result[n + 2] = 0;
    }
}

/*!
 * \brief ShowEdges
 * \param edges
 * \param img_width
 * \param img_height
 */
void shapes::ShowEdges(
    std::vector<float> &edges,
    int img_width,
    int img_height,
    unsigned char* result)
{
    int pixels = img_width * img_height * 3;
    for (int i = pixels - 1; i >= 0; i--) result[i] = 255;

    for (int i = 0; i < (int)edges.size(); i += 2)
    {
        float x = edges[i];
        float y = edges[i + 1];
        int n = (((int)y * img_width) + (int)x) * 3;
        if (n < pixels - 3)
        {
            result[n] = 0;
            result[n + 1] = 0;
            result[n + 2] = 0;
        }
    }
}

/*!
 * \brief ShowEdges
 * \param edges
 * \param img_width
 * \param img_height
 * \param r
 * \param g
 * \param b
 */
void shapes::ShowEdges(
    std::vector<int> &edges,
    unsigned char* img,
    int img_width,
    int img_height,
    int r, int g, int b)
{
    int pixels = img_width * img_height * 3;
    for (int i = 0; i < (int)edges.size(); i += 2)
    {
        int x = edges[i];
        int y = edges[i + 1];
        int n = ((y * img_width) + x) * 3;
        if (n < pixels - 3)
        {
            img[n] = (unsigned char)b;
            img[n + 1] = (unsigned char)g;
            img[n + 2] = (unsigned char)r;
        }
    }
}

/*!
 * \brief ShowRawImagePerimeter
 * \param img_width
 * \param img_height
 * \param perim
 * \param r
 * \param g
 * \param b
 * \param line_width
 */
void shapes::ShowRawImagePerimeter(
    unsigned char* img,
    int img_width,
    int img_height,
    polygon2D *perim,
    int r,
    int g,
    int b,
    int line_width,
    unsigned char* result)
{
    memcpy(result, img, img_width * img_height * 3);
    perim->show(result, img_width, img_height,
                r, g, b, line_width);
}

/*!
 * \brief ShowEdges
 * \param groups
 * \param img_width
 * \param img_height
 */
void shapes::ShowEdges(
    std::vector<std::vector<int> > &groups,
    int img_width,
    int img_height,
    unsigned char* result)
{
    for (int i = (img_width * img_height * 3) - 1; i >= 0; i--) result[i] = 255;

    unsigned char r = 0, g = 0, b = 0;
    for (int i = 0; i < (int)groups.size(); i++)
    {
        int ii = i % 4;
        switch (ii)
        {
            case 0:
                {
                    r = 0;
                    g = 255;
                    b = 0;
                    break;
                }
            case 1:
                {
                    r = 255;
                    g = 0;
                    b = 0;
                    break;
                }
            case 2:
                {
                    r = 0;
                    g = 0;
                    b = 255;
                    break;
                }
            case 3:
                {
                    r = 255;
                    g = 0;
                    b = 255;
                    break;
                }
        }

        for (int j = 0; j < (int)groups[i].size(); j += 2)
        {
            int x = groups[i][j];
            int y = groups[i][j + 1];
            int n = ((y * img_width) + x) * 3;

            result[n] = b;
            result[n + 1] = g;
            result[n + 2] = r;
        }
    }
}

/*!
 * \brief ShowGroups
 * \param edges
 * \param img_width
 * \param img_height
 * \param image_border
 * \param minimum_size_percent
 * \param squares_only
 * \param max_search_depth
 * \param ignore_periphery
 * \param grouping_radius_percent
 */
void shapes::ShowGroups(
    std::vector<int> &edges,
    int img_width,
    int img_height,
    int image_border,
    int minimum_size_percent,
    bool squares_only,
    float maximum_aspect_ratio,
    int max_search_depth,
    bool ignore_periphery,
    int grouping_radius_percent,
    std::vector<std::vector<int> >& groups,
    unsigned char* result)
{
    int no_of_compressions = 1;
    int compression[] = { 6 };

    int step_sizes[] = { 12, 4 };
    int no_of_step_sizes = 2;

    for (int i = (img_width * img_height * 3)-1; i >= 0; i++) result[i] = 255;

    GetGroups(
        edges, img_width, img_height, image_border, minimum_size_percent,
        squares_only,
        maximum_aspect_ratio,
        ignore_periphery,
        grouping_radius_percent,
        compression, no_of_compressions,
        groups, NULL,
        step_sizes,
        no_of_step_sizes);

    unsigned char r = 0, g = 0, b = 0;
    for (int i = 0; i < (int)groups.size(); i++)
    {
        int ii = i % 12;
        switch (ii)
        {
			case 0: { r = 255; g = 0; b = 0; break; }
			case 1: { r = 0; g = 255; b = 0; break; }
			case 2: { r = 0; g = 0; b = 255; break; }
			case 3: { r = 255; g = 0; b = 255; break; }
			case 4: { r = 0; g = 255; b = 255; break; }
			case 5: { r = 255; g = 255; b = 0; break; }

			case 6: { r = 155; g = 0; b = 0; break; }
			case 7: { r = 0; g = 155; b = 0; break; }
			case 8: { r = 0; g = 0; b = 155; break; }
			case 9: { r = 255; g = 0; b = 155; break; }
			case 10: { r = 0; g = 155; b = 255; break; }
			case 11: { r = 155; g = 255; b = 0; break; }
        }
        for (int j = 0; j < (int)groups[i].size(); j += 2)
        {
            int x = groups[i][j];
            int y = groups[i][j + 1];
            int n = ((y * img_width) + x) * 3;
            result[n] = b;
            result[n + 1] = g;
            result[n + 2] = r;
        }
    }
}

/*!
 * \brief ShowGroups
 * \param groups
 * \param img_width
 * \param img_height
 */
void shapes::ShowGroups(
    std::vector<std::vector<int> > &groups,
    int img_width,
    int img_height,
    unsigned char* result)
{
    for (int i = (img_width * img_height * 3)-1; i >= 0; i--) result[i] = 255;

    unsigned char r = 0, g = 0, b = 0;
    for (int i = 0; i < (int)groups.size(); i++)
    {
        int ii = i % 12;
        switch (ii)
        {
			case 0: { r = 255; g = 0; b = 0; break; }
			case 1: { r = 0; g = 255; b = 0; break; }
			case 2: { r = 0; g = 0; b = 255; break; }
			case 3: { r = 255; g = 0; b = 255; break; }
			case 4: { r = 0; g = 255; b = 255; break; }
			case 5: { r = 255; g = 255; b = 0; break; }
			case 6: { r = 155; g = 0; b = 0; break; }
			case 7: { r = 0; g = 155; b = 0; break; }
			case 8: { r = 0; g = 0; b = 155; break; }
			case 9: { r = 255; g = 0; b = 155; break; }
			case 10: { r = 0; g = 155; b = 255; break; }
			case 11: { r = 155; g = 255; b = 0; break; }
        }
        for (int j = 0; j < (int)groups[i].size(); j += 2)
        {
            int x = groups[i][j];
            int y = groups[i][j + 1];
            int n = ((y * img_width) + x) * 3;
            result[n] = b;
            result[n + 1] = g;
            result[n + 2] = r;
        }
    }
}

/*!
 * \brief detect connected sets of edges which correspond to the perimeters of shapes
 * \param edgemap
 * \param image_border
 * \param minimum_size_percent the minimum size of a perimeter relative to the largest found
 * \param squares_only only look for square regions
 * \param max_rectangular_aspect the maximum rectangular aspect ratio
 * \param max_search_depth when tracing edges recursively this  the maximum search depth
 * \param ignore_periphery don't trace edges which are within the border
 * \param centres average pixel position
 * \param bounding_boxes bounding boxes for the perimeters
 * \return list of perimeters
 */
void shapes::DetectLongestPerimeters(
    std::vector<int> &edges,
    int img_width,
    int img_height,
    int image_border,
    int minimum_size_percent,
    bool squares_only,
    float max_rectangular_aspect,
    bool ignore_periphery,
    std::vector<float>& centres,
    std::vector<float>& bounding_boxes,
    std::vector<std::vector<int> >& longestPerimeters)
{
    centres.erase(centres.begin(), centres.end());
    bounding_boxes.erase(bounding_boxes.begin(), bounding_boxes.end());
    longestPerimeters.erase(longestPerimeters.begin(), longestPerimeters.end());

    bool** edgemap = new bool*[img_width];
    for (int x = img_width-1; x >= 0; x--)
    {
        edgemap[x] = new bool[img_height];
        for (int y = img_height-1; y >= 0; y--)
            edgemap[x][y] = false;
    }

    // update the map
    for (int i = (int)edges.size() - 2; i >= 0; i -= 2)
    {
        int x = edges[i];
        int y = edges[i + 1];
        if ((x > -1) && (x < img_width) &&
            (y > -1) && (y < img_height))
            edgemap[x][y] = true;
        else
        {
            edges.erase(edges.begin() + i + 1);
            edges.erase(edges.begin() + i);
        }
    }

    // maximum perimeter length found
    int max_length = 0;

    // make a copy of the original edges image
    // because we'll remove edges as they're counted
    bool** edges_img = new bool*[img_width];
    for (int x = img_width-1; x >= 0; x--)
    {
        edges_img[x] = new bool[img_height];
        for (int y = img_height-1; y >= 0; y--)
            edges_img[x][y] = edgemap[x][y];
    }

    // list of perimeters found
    std::vector<std::vector<int> > temp_perimeters;

    // list of the lengths of the perimeters found
    std::vector<int> temp_perimeter_lengths;

    // list of the centre points of the perimeters found
    std::vector<float> temp_perimeter_centres;

    // list of the bounding boxes of the perimeters found
    std::vector<float> temp_bounding_boxes;

    // max and min rectangular size
    int max_rectangular_width = img_width * 70 / 100;
    int max_rectangular_height = img_height * 70 / 100;

    // the maximum and minimum rectangular aspect ratios
    if (max_rectangular_aspect == 0) max_rectangular_aspect = 100;
    float min_rectangular_aspect = 1.0f / max_rectangular_aspect;

    // examine the edges image
    for (int i = 0; i < (int)edges.size(); i += 2)
    {
        int x = edges[i];
        int y = edges[i + 1];

        //  this an edge?
        if (edges_img[x][y])
        {
            // an edge has been found - begin tracing
            std::vector<int> members;

            // trace along the edge to form a perimeter
            int length = 0;
            bool isValid = true;

            int perimeter_tx = img_width;
            int perimeter_ty = img_height;
            int perimeter_bx = 0;
            int perimeter_by = 0;
            int centre_x = 0;
            int centre_y = 0;
            TraceEdge(edges_img, img_width, img_height,
                      x, y, length, members, image_border, isValid,
                      perimeter_tx, perimeter_ty, perimeter_bx, perimeter_by,
                      ignore_periphery,
                      centre_x, centre_y);

            if (squares_only)
            {
                if (perimeter_by != perimeter_ty)
                {
                    float aspect_ratio = (perimeter_bx - perimeter_tx) /
                                         (float)(perimeter_by - perimeter_ty);
                    if ((aspect_ratio < 0.7f) ||
                        (aspect_ratio > 1.3f))
                        isValid = false;
                }
            }
            else
            {
            	// rectangular shapes
            	if ((perimeter_bx - perimeter_tx > max_rectangular_width) ||
            		(perimeter_by - perimeter_ty > max_rectangular_height))
            	{
            		if (perimeter_by != perimeter_ty)
            		{
                        float aspect_ratio = (perimeter_bx - perimeter_tx) /
                                             (float)(perimeter_by - perimeter_ty);
                        if ((aspect_ratio > max_rectangular_aspect) ||
                            (aspect_ratio < min_rectangular_aspect))
                            isValid = false;
            		}
            	}
            }


            //  the perimeter above some small size ?
            // (we're not interested  noise)
            if (isValid)
            {
                if (length > 20)
                {
                    // add this perimeter to the list
                    temp_perimeters.push_back(members);
                    temp_perimeter_lengths.push_back(length);
                    temp_perimeter_centres.push_back(centre_x / (float)length);
                    temp_perimeter_centres.push_back(centre_y / (float)length);
                    temp_bounding_boxes.push_back(perimeter_tx);
                    temp_bounding_boxes.push_back(perimeter_ty);
                    temp_bounding_boxes.push_back(perimeter_bx);
                    temp_bounding_boxes.push_back(perimeter_by);
                }

                // was this the longest perimeter ?
                if (length > max_length)
                    max_length = length;
            }
        }
    }

    // minimum length of the perimeter in pixels
    int minimum_length_pixels = minimum_size_percent * max_length / 100;

    // of all the perimeters detected select the longest
    centres.erase(centres.begin(), centres.end());
    bounding_boxes.erase(bounding_boxes.begin(), bounding_boxes.end());
    if (max_length > 0)
    {
        for (int i = 0; i < (int)temp_perimeters.size(); i++)
        {
            int length = temp_perimeter_lengths[i];

            //  this perimeter longer than the minimum ?
            if (length > minimum_length_pixels)
            {
                // add this perimeter to the list
                longestPerimeters.push_back(temp_perimeters[i]);
                centres.push_back(temp_perimeter_centres[i * 2]);
                centres.push_back(temp_perimeter_centres[(i * 2) + 1]);
                for (int j = 0; j < 4; j++)
                    bounding_boxes.push_back(temp_bounding_boxes[(i * 4) + j]);
            }
        }
    }

    for (int x = img_width-1; x >= 0; x--)
    {
        delete[] edgemap[x];
        delete[] edges_img[x];
    }
    delete[] edgemap;
    delete[] edges_img;
}



// ********** private methods **********


/*!
 * \brief returns a value indicating how square the given region is
 */
float shapes::Squareness(
    polygon2D *square)
{
    float measure1 = square->getSideLength(0) / square->getSideLength(1);
    measure1 = ABS(1.0f - measure1);

    float measure2 = square->getSideLength(0) / square->getSideLength(2);
    measure2 = ABS(1.0f - measure2);

    float measure3 = square->getSideLength(1) / square->getSideLength(3);
    measure3 = ABS(1.0f - measure3);

    float result = 1.0f / (1.0f + (measure1 + measure2 + measure3));
    return (result);
}

/*!
 * \brief searches for a given polygon within a vector of polygons
 * \param vect list of polygons
 * \param item polygon to be searched for
 * \return true if the polygon was found
 */
bool shapes::VectorContains(std::vector<polygon2D*> &vect, polygon2D *item)
{
    int i = 0;
    int max = (int)vect.size();
    while (i < max)
    {
        if (vect[i] != item)
        	i++;
        else
            i = max+1;
    }
    if (i > max)
        return(true);
    else
        return(false);
}

/*!
 * \brief searches for a given node within a vector of nodes
 * \param vect list of nodes
 * \param item node to be searched for
 * \return true if the node was found
 */
bool shapes::VectorContains(std::vector<hypergraph_node*> &vect, hypergraph_node *item)
{
    int i = 0;
    int max = (int)vect.size();
    while (i < max)
    {
        if (vect[i] != item)
        	i++;
        else
            i = max+1;
    }
    if (i > max)
        return(true);
    else
        return(false);
}

/*!
 * \brief searches for a given string within a vector of strings
 * \param vect list of strings
 * \param item string to be searched for
 * \return true if the string was found
 */
bool shapes::VectorContains(std::vector<std::string> &vect, std::string item)
{
    int i = 0;
    int max = (int)vect.size();
    while (i < max)
    {
        if (vect[i] != item)
        	i++;
        else
            i = max+1;
    }
    if (i > max)
        return(true);
    else
        return(false);
}

/*!
 * \brief removes a given node from a vector of nodes
 * \param vect list of nodes
 * \param item node to be searched for
 * \return true if the node was removed
 */
bool shapes::VectorRemove(std::vector<polygon2D*> &vect, polygon2D *item)
{
    int i = 0;
    while (i < (int)vect.size())
    {
        if (vect[i] == item)
        {
            vect.erase(vect.begin() + i);
            i = 999999;
        }
        else
        {
            i++;
        }
    }
    if (i == 999999)
        return(true);
    else
        return(false);
}


/*!
 * \brief non-recursively trace along an edge
 * \param edges_img edges image
 * \param img_width edges image width
 * \param img_height edges image height
 * \param x current x coordinate
 * \param y current y coordinate
 * \param length length of the perimeter
 * \param members list of points belonging to the perimeter
 * \param image_border border around the image  pixels
 * \param isValid whether this traced set of edges  valid
 * \param perimeter_tx top left x coordinate of the bounding box
 * \param perimeter_ty top left y coordinate of the bounding box
 * \param perimeter_bx bottom right x coordinate of the bounding box
 * \param perimeter_by bottom right y coordinate of the bounding box
 * \param ignore_periphery whether to ignore any edges which trace into teh border region
 * \param centre_x centre of gravity x coordinate
 * \param centre_y centre of gravity y coordinate
 */
void shapes::TraceEdge(
    bool** edges_img,
    int& img_width,
    int& img_height,
    int x,
    int y,
    int& length,
    std::vector<int> &members,
    int& image_border,
    bool& isValid,
    int& perimeter_tx,
    int& perimeter_ty,
    int& perimeter_bx,
    int& perimeter_by,
    bool& ignore_periphery,
    int& centre_x,
    int& centre_y)
{
    members.erase(members.begin(), members.end());

    bool following = true;
    while (following)
    {
        following = false;

        // if we encounter the periphery of the image then abandon the trace
        if ((!ignore_periphery) ||
            ((ignore_periphery) &&
             ((x >= image_border) && (x <= img_width - 1 - image_border) &&
              (y >= image_border) && (y <= img_height - 1 - image_border))))
        {

            // this edge has been followed
            edges_img[x][y] = false;

            // add this point to the list
            //int size = (int)members.size();
            members.push_back(x);
            members.push_back(y);

            // keep track of the centre of gravity
            centre_x += x;
            centre_y += y;

            // increase the length of the line
            length++;

            // update the bounding box
            if (x < perimeter_tx) perimeter_tx = x;
            if (y < perimeter_ty) perimeter_ty = y;
            if (x > perimeter_bx) perimeter_bx = x;
            if (y > perimeter_by) perimeter_by = y;

            // peruse the hood
            int yy = y - 1;
            while ((yy <= y + 1) && (!following))
            {
                if ((yy > -1) && (y < img_height))
                {
                    int xx = x - 1;
                    while ((xx <= x + 1) && (!following))
                    {
                        if ((xx > -1) && (x < img_width))
                        {
                            if (edges_img[xx][yy])
                            {
                                x = xx;
                                y = yy;
                                following = true;
                            }
                        }
                        xx++;
                    }
                }
                yy++;
            }
        }
    }
}

/*!
 * \brief returns the dominant orientation, based upon the given edge positions around teh perimeter of a square shape
 * \param tx
 * \param ty
 * \param left_edges edge positions along the left side of the perimeter
 * \param left_edges_length number of edges along the left perimeter (x2)
 * \param right_edges edge positions along the right side of the perimeter
 * \param right_edges_length number of edges along the right perimeter (x2)
 * \param top_edges edge positions along the top side of the perimeter
 * \param top_edges_length number of edges along the top perimeter (x2)
 * \param bottom_edges edge positions along the bottom side of the perimeter
 * \param bottom_edges_length number of edges along the bottom perimeter (x2)
 * \param dominant_edges edges found oriented with the dominant orientation
 * \param non_dominant_edges edges found which are not oriented with the dominant orientation
 * \param side_edges edges along each side of the perimeter
 * \param quantization_degrees quantization to use when creating orientation histograms
 * \param step_size step size to be used when sampling edge positions
 */
float shapes::DominantOrientation(
    int tx,
    int ty,
    int* left_edges,
    int left_edges_length,
    int* right_edges,
    int right_edges_length,
    int* top_edges,
    int top_edges_length,
    int* bottom_edges,
    int bottom_edges_length,
    std::vector<int>& dominant_edges,
    std::vector<int>& non_dominant_edges,
    std::vector<std::vector<int> >& side_edges,
    float quantization_degrees,
    int step_size,
    int *histogram,
    histogram_element** histogram_members)
{
    float orientation = 0;
    float centre_x = 0, centre_y = 0, no_of_edges = 0;
    float pi = (float)PI;
    float two_pi = pi * 2;
    int no_of_buckets = (int)(360 / quantization_degrees);
    int hist_length = no_of_buckets + 1;

    // clear histogram arrays
    bool* histogram_members_active = new bool[hist_length * 4];
    for (int i = hist_length-1; i >= 0; i--) histogram[i] = 0;
    for (int i = (hist_length * 4)-1; i >= 0; i--)
    {
    	histogram_members[i]->histogram.clear();
    	histogram_members_active[i] = false;
    }

    dominant_edges.erase(dominant_edges.begin(), dominant_edges.end());
    non_dominant_edges.erase(non_dominant_edges.begin(), non_dominant_edges.end());
    side_edges.erase(side_edges.begin(), side_edges.end());

    std::vector<int> s_edges0;
    side_edges.push_back(s_edges0);
    std::vector<int> s_edges1;
    side_edges.push_back(s_edges1);
    std::vector<int> s_edges2;
    side_edges.push_back(s_edges2);
    std::vector<int> s_edges3;
    side_edges.push_back(s_edges3);

    // for each side
    for (int i = 0; i < 4; i++)
    {
        // use the appropriate array
        int* edges = NULL;
        int edges_length = 0;
        switch (i)
        {
            case 0: { edges = left_edges; edges_length = left_edges_length; break; }
            case 1: { edges = right_edges; edges_length = right_edges_length; break; }
            case 2: { edges = top_edges; edges_length = top_edges_length; break; }
            case 3: { edges = bottom_edges; edges_length = bottom_edges_length; break; }
        }

        int step_size2 = edges_length / step_size;
        if (step_size2 < 1) step_size2 = 1;

        // only use non zero values
        std::vector<int> nonzero;
        for (int j = 0; j < edges_length; j++)
            if (edges[j] > 0) nonzero.push_back(j);

        // update the orientation histogram
        int prev_x, prev_y;
        int x, y;
        for (int j = 0; j < (int)nonzero.size() - step_size2; j++)
        {
            if (i < 2)
            {
                prev_y = nonzero[j];
                prev_x = edges[prev_y];
                y = nonzero[j + step_size2];
                x = edges[y];
                prev_y += ty;
                y += ty;
            }
            else
            {
                prev_x = nonzero[j];
                prev_y = edges[prev_x];
                x = nonzero[j + step_size2];
                y = edges[x];
                prev_x += tx;
                x += tx;
            }

            centre_x += prev_x;
            centre_y += prev_y;
            centre_x += x;
            centre_y += y;
            no_of_edges += 2;

            float dx = x - prev_x;
            float dy = y - prev_y;
            float dist = (float)sqrt((dx * dx) + (dy * dy));
            if (dist > 0)
            {
                float angle = (float)acos(dy / dist);
                if (dx < 0) angle = two_pi - angle;
                float angle_degrees = angle / pi * 180.0f;
                int bucket = (int)(angle_degrees / quantization_degrees);
                histogram[bucket]++;

                // which edges are members of this histogram entry
                int index = (i * hist_length) + bucket;

                histogram_element *elem = histogram_members[index];
                elem->histogram.push_back(prev_x);
                elem->histogram.push_back(prev_y);
                elem->histogram.push_back(x);
                elem->histogram.push_back(y);
                histogram_members_active[index] = true;
            }
        }
    }

    // search for the dominant orientation
    int half_histogram = (int)(180 / quantization_degrees);
    int max_hits = 0;
    for (int i = 0; i < half_histogram; i++)
    {
        int hits = histogram[i] + histogram[i + half_histogram];
        if (hits > max_hits)
        {
            max_hits = hits;
            orientation = i;
        }
    }

    // get the centre of edges
    if (no_of_edges > 0)
    {
        centre_x /= no_of_edges;
        centre_y /= no_of_edges;
    }

    // dominant orientation in radians
    float orientation_radians = (orientation * quantization_degrees) * pi / 180.0f;

    // detect edges belonging to the dominant orientation
    int orient;
    for (int b = 0; b < 2; b++)
    {
        if (b == 0)
            orient = (int)orientation;
        else
            orient = (int)orientation + (no_of_buckets / 4);

        // separator
        float separator_x0 = centre_x;
        float separator_y0 = centre_y;
        float separator_x1 = centre_x - (100 * (float)sin(orientation_radians + (b * ((float)PI * 0.5f))));
        float separator_y1 = centre_y - (100 * (float)cos(orientation_radians + (b * ((float)PI * 0.5f))));

        // two opposite sides
        for (int p = 0; p < 2; p++)
        {
            if (p > 0) orient += (no_of_buckets / 2);
            for (int i = orient - 1; i <= orient + 1; i++)
            {
                int j = i;
                if (j < 0) j += no_of_buckets;
                if (j >= no_of_buckets) j -= no_of_buckets;
                for (int side = 0; side < 4; side++)
                {
                    int index = (side * hist_length) + j;
                    if (histogram_members_active[index] > 0)
                    {
                    	int size = (int)(histogram_members[index]->histogram.size());
						for (int k = 0; k < size; k += 4)
						{
							int prev_x = histogram_members[index]->histogram.at(k);
							int prev_y = histogram_members[index]->histogram.at(k + 1);
							int x = histogram_members[index]->histogram.at(k + 2);
							int y = histogram_members[index]->histogram.at(k + 3);

							// perpendicular distance of this point to the separator line
							float dist_to_separator =
								geometry::pointDistanceFromLine(
									separator_x0, separator_y0,
									separator_x1, separator_y1,
									x, y);

							if (b == 0)
							{
								dominant_edges.push_back(prev_x);
								dominant_edges.push_back(prev_y);
								dominant_edges.push_back(x);
								dominant_edges.push_back(y);

								int idx = 0;
								if (dist_to_separator > 0)
									idx = 1;

								side_edges[idx].push_back(prev_x);
								side_edges[idx].push_back(prev_y);
								side_edges[idx].push_back(x);
								side_edges[idx].push_back(y);
							}
							else
							{
								non_dominant_edges.push_back(prev_x);
								non_dominant_edges.push_back(prev_y);
								non_dominant_edges.push_back(x);
								non_dominant_edges.push_back(y);

								int idx = 2;
								if (dist_to_separator > 0)
									idx = 3;


								side_edges[idx].push_back(prev_x);
								side_edges[idx].push_back(prev_y);
								side_edges[idx].push_back(x);
								side_edges[idx].push_back(y);

							}
						}
                    }
                }
            }
        }
    }

    delete[] histogram_members_active;

    return (orientation_radians);
}

/*!
 * \brief detect all polygons within a set of edges
 * \param edges
 * \param max_deviation maximum deviation of edge positions from the ideal best fit line in pixels
 * \param lines returned lines
 * \param intercepts returned interception points between lines
 * \param polygons returned polygons
 * \param no_of_samples_per_line number of samples at each step used to detect lines
 * \param no_of_edge_samples_per_line number of edges to sample for each candidate line
 * \param minimum_edges_per_line the minimum number of edges on the detected line
 * \param maximum_no_of_lines the maximum number of lines to be returned
 * \param suppression_radius radius for non-maximal suppression
 * \param edges_per_row ideal number of edges per row
 * \param max_edges_within_image maximum number of edges per image (typically around 200)
 * \param valid_aspect_ratios a list of valid aspect ratios
 */
void shapes::DetectPolygons(
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
    bool show_intercepts)
{
    if (output_img != NULL)
    	memset(output_img, 255, img_width * img_height * 3 * sizeof(unsigned char));

	// define a bounding box suitable for the image
    int boundingbox_tx = -img_width;
    int boundingbox_ty = -img_height;
    int boundingbox_bx = img_width * 2;
    int boundingbox_by = img_height * 2;

    std::vector<float> intercepts;

    unsigned char *eroded_dilated_img = img_mono;
    unsigned char *buffer = NULL;

    if (erosion_dilation != 0)
    {
    	buffer = new unsigned char[img_width * img_height];
    	eroded_dilated_img = new unsigned char[img_width * img_height];
    }
    if (erosion_dilation > 0)
        processimage::Erode(img_mono, img_width, img_height, buffer, erosion_dilation, eroded_dilated_img);
    if (erosion_dilation < 0)
        processimage::Dilate(img_mono, img_width, img_height, buffer, -erosion_dilation, eroded_dilated_img);

    CannyEdgeDetector *edge_detector = new CannyEdgeDetector();
    edge_detector->Update(eroded_dilated_img, img_width, img_height, 1);

    shapes::DetectPolygons(
        edge_detector->edges,
        max_deviation,
        lines,
        intercepts,
        polygons,
        max_edges_within_image,
        max_edges_within_image,
        minimum_edges_per_line,
        maximum_no_of_lines,
        boundingbox_tx, boundingbox_ty, boundingbox_bx, boundingbox_by,
        valid_aspect_ratios,
        no_of_valid_aspect_ratios,
        true);

    if (output_img != NULL)
    {
		// draw edges
		for (int i = 0; i < (int)edge_detector->edges.size(); i += 2)
			drawing::drawSpot(output_img, img_width, img_height, edge_detector->edges[i], edge_detector->edges[i+1], 1, 0,0,0);

		if (show_lines)
		{
			for (int i = 0; i < (int)lines.size(); i++)
			{
				float detected_xx0 = lines[i][0];
				float detected_yy0 = lines[i][1];
				float detected_xx1 = lines[i][2];
				float detected_yy1 = lines[i][3];
				drawing::drawLine(output_img, img_width, img_height,
								  (int)detected_xx0, (int)detected_yy0,
								  (int)detected_xx1, (int)detected_yy1,
								  0, 255, 0,
								  0, false);
			}
		}

		if (show_polygons)
		{
		    for (int i = 0; i < (int)polygons.size(); i++)
			    polygons[i]->show(output_img, img_width, img_height, 0,0,255, 0);
		}

		if (show_intercepts)
		{
			for (int i = 0; i < (int)intercepts.size(); i += 2)
				drawing::drawCircle(output_img, img_width, img_height, (int)intercepts[i], (int)intercepts[i+1], 5, 255,0,0, 0);
		}
    }

    if (buffer != NULL)
    {
    	delete[] buffer;
    	delete[] eroded_dilated_img;
    }
    delete edge_detector;

}

bool shapes::ValidPolygon(
    polygon2D* poly,
    float* valid_aspect_ratios,
    int no_of_valid_aspect_ratios)
{
	bool is_valid = false;

	float squareness = poly->getSquareness();
	if ((squareness > 0.7f) && (squareness < 1.3f))
	{
		float aspect = poly->getLongestSide() / poly->getShortestSide();

		bool aspect_valid = false;
		int ctr = 0;
		while ((ctr < no_of_valid_aspect_ratios) && (!aspect_valid))
		{
			if ((aspect > valid_aspect_ratios[ctr] - 0.2f) &&
				(aspect < valid_aspect_ratios[ctr] + 0.2f))
				aspect_valid = true;
		    ctr++;
		}

		if (aspect_valid)
		{
			bool square_corners = true;
			int corner = 0;
			while ((corner < 4) && (square_corners))
			{
                if ((ABS(poly->x_points[1] - poly->x_points[0])) >
                    (ABS(poly->y_points[1] - poly->y_points[0])))
                {
					if (poly->x_points[0] < poly->x_points[1])
					{
						if (poly->x_points[3] > poly->x_points[2])
							square_corners = false;
					}
					else
					{
						if (poly->x_points[3] < poly->x_points[2])
							square_corners = false;
					}
                }
                else
                {
					if (poly->y_points[0] < poly->y_points[1])
					{
						if (poly->y_points[3] > poly->y_points[2])
							square_corners = false;
					}
					else
					{
						if (poly->y_points[3] < poly->y_points[2])
							square_corners = false;
					}
                }

                if (square_corners)
                {
				    float angle = poly->GetInteriorAngle(corner) * 180.0f / PI;
				    //printf("angle: %f\n", angle);
				    if ((angle < 60) || (angle > 120)) square_corners = false;
                }
				corner++;
			}
			if (square_corners) is_valid = true;
		}
	}
    return(is_valid);
}

/*!
 * \brief detect all polygons within a set of edges
 * \param edges
 * \param max_deviation maximum deviation of edge positions from the ideal best fit line in pixels
 * \param lines returned lines
 * \param intercepts returned interception points between lines
 * \param polygons returned polygons
 * \param no_of_samples_per_line number of samples at each step used to detect lines
 * \param no_of_edge_samples_per_line number of edges to sample for each candidate line
 * \param minimum_edges_per_line the minimum number of edges on the detected line
 * \param maximum_no_of_lines the maximum number of lines to be returned
 * \param valid_aspect_ratios a list of valid aspect ratios
 * \param no_of_valid_aspect_ratios number of valid aspect ratios
 * \param fit_to_edge_map fit polygons to the known edges
 */
void shapes::DetectPolygons(
    std::vector<int> &edges,
    float max_deviation,
    std::vector<std::vector<float> > &lines,
    std::vector<float> &intercepts,
    std::vector<polygon2D*> &polygons,
    int no_of_samples_per_line,
    int no_of_edge_samples_per_line,
    int minimum_edges_per_line,
    int maximum_no_of_lines,
    int boundingbox_tx, int boundingbox_ty,
    int boundingbox_bx, int boundingbox_by,
    float* valid_aspect_ratios,
    int no_of_valid_aspect_ratios,
    bool fit_to_edge_map)
{
	// seed random number generator
	srand(0);

	// find lines
	DetectLines(
        edges,
        max_deviation,
        lines,
        no_of_samples_per_line,
        no_of_edge_samples_per_line,
        minimum_edges_per_line,
        maximum_no_of_lines);

	if (lines.size() > 0)
	{
		// create a hypergraph with some nodes representing the lines
		hypergraph* primal_sketch = new hypergraph((int)lines.size(), 1);

		// welcome to duh matrix
		float* line_connection_matrix = new float[(int)lines.size() * (int)lines.size() * 2];

		// find intercepts between lines
		for (int i = 0; i < (int)lines.size()-1; i++)
		{
			float x0 = lines[i][0];
			float y0 = lines[i][1];
			float x1 = lines[i][2];
			float y1 = lines[i][3];
			for (int j = i + 1; j < (int)lines.size(); j++)
			{
				float x2 = lines[j][0];
				float y2 = lines[j][1];
				float x3 = lines[j][2];
				float y3 = lines[j][3];

				float ix = 0, iy = 0;
				geometry::intersection(x0, y0, x1, y1, x2, y2, x3, y3, ix, iy);

				// is the interception inside the bounding box ?
				if ((ix > boundingbox_tx) &&
					(iy > boundingbox_ty) &&
					(ix < boundingbox_bx) &&
					(iy < boundingbox_by))
				{
					intercepts.push_back(ix);
					intercepts.push_back(iy);

					// store the intercept position
					int index = ((i * (int)lines.size()) + j) * 2;
					line_connection_matrix[index] = ix;
					line_connection_matrix[index + 1] = iy;
					index = ((j * (int)lines.size()) + i) * 2;
					line_connection_matrix[index] = ix;
					line_connection_matrix[index + 1] = iy;

					primal_sketch->LinkByIndex(i, j);
					primal_sketch->LinkByIndex(j, i);
				}

			}
		}

		// find closed sets
		for (int i = 0; i < (int)lines.size(); i++)
		{
		    hypergraph_node* node = primal_sketch->Nodes[i];
		    if (node->Links.size() > 0)
		    {
		    	// clear all flags
		    	for (int j = 0; j < (int)lines.size(); j++)
		    		primal_sketch->SetFlagByIndex(j, 0, false);

		    	// propogate for 4 itterations
		    	std:: vector<hypergraph_node*> members;
		    	primal_sketch->PropogateFlagFromIndex(i, 0, members, 4);

                for (int j = 0; j < (int)members.size(); j++)
                {
                    if (members[j]->depth == 4)
                    {
                    	hypergraph_node *n1 = members[j];

                    	// is there a link between the node and this one?
                    	bool link_exists = false;
                    	for (int j1 = 0; j1 < (int)node->Links.size(); j1++)
                    	{
                    	    if (node->Links[j1]->From == n1)
                    	    {
                    	    	link_exists = true;
                    	    	break;
                    	    }
                    	}

                    	if (link_exists)
                    	{
							for (int j2 = 0; j2 < (int)n1->Links.size(); j2++)
							{
								hypergraph_node *n2 = n1->Links[j2]->From;
								if (n2->depth == 3)
								{
									for (int j3 = 0; j3 < (int)n2->Links.size(); j3++)
									{
										hypergraph_node *n3 = n2->Links[j3]->From;
										if (n3->depth == 2)
										{
											int idx = ((node->ID * (int)lines.size()) + n1->ID) * 2;
											float poly_x0 = line_connection_matrix[idx];
											float poly_y0 = line_connection_matrix[idx + 1];
											idx = ((n1->ID * (int)lines.size()) + n2->ID) * 2;
											float poly_x1 = line_connection_matrix[idx];
											float poly_y1 = line_connection_matrix[idx + 1];
											idx = ((n2->ID * (int)lines.size()) + n3->ID) * 2;
											float poly_x2 = line_connection_matrix[idx];
											float poly_y2 = line_connection_matrix[idx + 1];
											idx = ((node->ID * (int)lines.size()) + n3->ID) * 2;
											float poly_x3 = line_connection_matrix[idx];
											float poly_y3 = line_connection_matrix[idx + 1];

											polygon2D *poly = new polygon2D();
											poly->Add(poly_x0, poly_y0);
											poly->Add(poly_x1, poly_y1);
											poly->Add(poly_x2, poly_y2);
											poly->Add(poly_x3, poly_y3);

											if (ValidPolygon(poly, valid_aspect_ratios, no_of_valid_aspect_ratios))
											{
												polygons.push_back(poly);
											}
											else
											{
											    delete poly;
											}
										}
									}
								}
							}
                    	}
                    }
                }
		    }
		}

		// remove duplicates
		for (int i = 0; i < (int)polygons.size()-1; i++)
		{
            polygon2D *poly0 = polygons[i];
            float centre0_x = 0, centre0_y = 0;
            poly0->GetSquareCentre(centre0_x, centre0_y);
            float length0 = poly0->getPerimeterLength();

    		for (int j = (int)polygons.size()-1; j >= i + 1 ; j--)
    		{
                polygon2D *poly1 = polygons[j];
                float centre1_x = 0, centre1_y = 0;
                poly1->GetSquareCentre(centre1_x, centre1_y);

                float dx = centre1_x - centre0_x;
                if (ABS(dx) < 2)
                {
                	float dy = centre1_y - centre0_y;
                	if (ABS(dy) < 2)
                	{
                		float length1 = poly1->getPerimeterLength();
                		if (ABS(length1 - length0) < 2)
                		{
                			// exterminate!
                			polygons.erase(polygons.begin() + j);
                		}
                	}
                }
    		}
		}

		delete[] line_connection_matrix;
		delete primal_sketch;

		// create a binary 2D map from the edge features and use
		// this to eliminate polygons which do not have many edges
		// along their sides
		if (fit_to_edge_map)
		{
			// create an edge map
			int radius = 2;
			int edge_map_width = (boundingbox_bx - boundingbox_tx) / 3;
			int edge_map_height = (boundingbox_by - boundingbox_ty) / 3;
			bool* edge_map = new bool[edge_map_width * edge_map_height];
			memset(edge_map, 0, edge_map_width * edge_map_height * sizeof(bool));
			for (int i = 0; i < (int)edges.size(); i += 2)
			{
				int edge_x = (int)((edges[i] - boundingbox_tx + 0.5f) * 0.333f);
				int edge_y = (int)((edges[i+1] - boundingbox_ty + 0.5f) * 0.333f);
				if ((edge_x > 1) &&
					(edge_y > 1) &&
					(edge_x < edge_map_width-2) &&
					(edge_y < edge_map_height-2))
				{
					for (int edge_y2 = edge_y - radius; edge_y2 <= edge_y + radius; edge_y2++)
					{
						int n = (edge_y2 * edge_map_width) + edge_x - radius;
  					    for (int edge_x2 = edge_x - radius; edge_x2 <= edge_x + radius; edge_x2++, n++)
					    {
  					    	edge_map[n] = true;
						}
					}
				}
			}

			// remove polygons if they do not have sufficient evidence on the edge map
			int no_of_samples_per_side = 16;
			for (int i = (int)polygons.size()-1; i >= 0; i--)
			{
				polygon2D* poly = polygons[i];
				bool valid_polygon = true;

				for (int vertex = 0; vertex < 4; vertex++)
				{
					int sample_hits = 0;
					float x0 = (poly->x_points[vertex] - boundingbox_tx + 0.5f) * 0.333f;
					float y0 = (poly->y_points[vertex] - boundingbox_ty + 0.5f) * 0.333f;
					int next_vertex = vertex + 1;
					if (next_vertex == 4) next_vertex = 0;
					float x1 = (poly->x_points[next_vertex] - boundingbox_tx + 0.5f) * 0.333f;
					float y1 = (poly->y_points[next_vertex] - boundingbox_ty + 0.5f) * 0.333f;
					float dx = x1 - x0;
					float dy = y1 - y0;
					for (int sample = 0; sample < no_of_samples_per_side; sample++)
					{
						float fraction  = sample / (float)no_of_samples_per_side;
						int sample_x = (int)(x0 + (dx * fraction));
						if ((sample_x > 1) && (sample_x < edge_map_width-1))
						{
							int sample_y = (int)(y0 + (dy * fraction));
							if ((sample_y > 1) && (sample_y < edge_map_height-1))
							{
								int n2 = (sample_y * edge_map_width) + sample_x;
								if (edge_map[n2]) sample_hits++;
							}
						}
					}

					//printf("HITS: %d\n", sample_hits);
					if (sample_hits < no_of_samples_per_side * 50 / 100)
					{
						valid_polygon = false;
						vertex = 4;
					}

				}

				if (!valid_polygon)
				{
					polygons.erase(polygons.begin() + i);
				}
			}

			delete[] edge_map;
		}
	}
}

/*!
 * \brief detect all lines within a set of edges
 * \param edges
 * \param max_deviation maximum deviation of edge positions from the ideal best fit line in pixels
 * \param lines returned lines
 * \param no_of_samples_per_line number of samples at each step used to detect lines
 * \param no_of_edge_samples_per_line number of edges to sample for each candidate line
 * \param minimum_edges_per_line the minimum number of edges on the detected line
 * \param maximum_no_of_lines the maximum number of lines to be returned
 */
void shapes::DetectLines(
    std::vector<int> &edges,
    float max_deviation,
    std::vector<std::vector<float> > &lines,
    int no_of_samples_per_line,
    int no_of_edge_samples_per_line,
    int minimum_edges_per_line,
    int maximum_no_of_lines)
{
	// make a copy of the edges
	std::vector<int> temp_edges;
	for (int i = 0; i < (int)edges.size(); i++) temp_edges.push_back(edges[i]);

	int max_edges = 999999;
	int line_count = 0;
	while ((max_edges > minimum_edges_per_line) &&
		   ((line_count < maximum_no_of_lines) || (maximum_no_of_lines <= 0)))
	{
		float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	    max_edges =
	    	BestFitLineRANSAC(
	            edges,
	            max_deviation,
	            x0, y0, x1, y1,
	            no_of_samples_per_line,
	            no_of_edge_samples_per_line,
	            true);

	    if (no_of_samples_per_line > (int)edges.size() / 2)
	        no_of_samples_per_line = (int)edges.size()/2;

	    if (max_edges > minimum_edges_per_line)
	    {
            std::vector<float> line;
            line.push_back(x0);
            line.push_back(y0);
            line.push_back(x1);
            line.push_back(y1);
            lines.push_back(line);
	    }
	    line_count++;
	}

	// restore edges to their former glory
	edges.clear();
	for (int i = 0; i < (int)temp_edges.size(); i++) edges.push_back(temp_edges[i]);
}


/*!
 * \brief BestFitLine
 * \param edges
 * \param max_deviation maximum deviation of edge positions from the ideal best fit line in pixels
 * \param x0 start x coordinate of the best fit line
 * \param y0 start y coordinate of the best fit line
 * \param x1 end x coordinate of the best fit line
 * \param y1 end y coordinate of the best fit line
 * \param no_of_samples number of possible lines to consider
 * \param no_of_edge_samples number of edges to sample for each candidate line
 * \param remove_edges removes edges if they belong to the detected line
 * \return number of edges along the line
 */
int shapes::BestFitLineRANSAC(
    std::vector<int> &edges,
    float max_deviation,
    float& x0,
    float& y0,
    float& x1,
    float& y1,
    int no_of_samples,
    int no_of_edge_samples,
    bool remove_edges)
{
	int max_hits = 0;

	int no_of_edges = (int)edges.size() / 2;
	if (no_of_edges > 10)
	{
		float gradient;
		float min_deviation = 999999;
		int best_index0=0;
		int best_index1=1;

		for (int sample = 0; sample < no_of_samples; sample++)
		{
			// randomly pick a baseline
			int index0 = rand() % no_of_edges;
			int index1 = rand() % no_of_edges;
			if (index0 != index1)
			{
				int hits = 0;
				float deviation_sum = 0;
				int idx0 = index0 * 2;
				int idx1 = index1 * 2;
				int xx0 = edges[idx0];
				int yy0 = edges[idx0 + 1];
				int xx1 = edges[idx1];
				int yy1 = edges[idx1 + 1];
				int dx = xx1 - xx0;
				int dy = yy1 - yy0;

				// is the baseline horizontally oriented ?
				bool horizontal = true;
				if (ABS(dy) > ABS(dx))
				{
					horizontal = false;
					gradient = dx / (float)dy;
				}
				else
				{
					gradient = dy / (float)dx;
				}

				for (int edge_sample = 0; edge_sample < no_of_edge_samples; edge_sample++)
				{
					// randomly pick an edge
					int index = rand() % no_of_edges;
					int edge_x = edges[index * 2];
					int edge_y = edges[(index * 2) + 1];
					float deviation = 0;

					if (horizontal)
					{
						float predicted_edge_y = yy0 + ((edge_x - xx0) * gradient);
						deviation = ABS(predicted_edge_y - edge_y);
					}
					else
					{
						float predicted_edge_x = xx0 + ((edge_y - yy0) * gradient);
						deviation = ABS(predicted_edge_x - edge_x);
					}

					if (deviation < max_deviation)
					{
						hits++;
						deviation_sum += deviation;
					}
				}

				if (hits > 0)
				{
					// pick the line with the maximum number of edges within the max deviation range
					if (hits > max_hits)
					{
						best_index0 = index0;
						best_index1 = index1;
						max_hits = hits;
						min_deviation = deviation_sum;
					}
					else
					{
						// if there is a tie choose the result with the lowest deviation
						if (hits == max_hits)
						{
							if (deviation_sum < min_deviation)
							{
								best_index0 = index0;
								best_index1 = index1;
								min_deviation = deviation_sum;
							}
						}
					}
				}
			}
		}

		// try to find the line parameters as accurately as possible
		if (max_hits > 0)
		{
			std::vector<int> line;
			int hits0 = 0;
			int hits1 = 0;
			float av_x = 0;
			float av_y = 0;
			x0 = 0;
			y0 = 0;
			x1 = 0;
			y1 = 0;
			int idx0 = best_index0 * 2;
			int idx1 = best_index1 * 2;
			int xx0 = edges[idx0];
			int yy0 = edges[idx0 + 1];
			int xx1 = edges[idx1];
			int yy1 = edges[idx1 + 1];
			int dx = xx1 - xx0;
			int dy = yy1 - yy0;

			// is the baseline horizontally oriented ?
			bool horizontal = true;
			if (ABS(dy) > ABS(dx))
			{
				horizontal = false;
				gradient = dx / (float)dy;
			}
			else
			{
				gradient = dy / (float)dx;
			}

			for (int edge_sample = no_of_edges-1; edge_sample >= 0; edge_sample--)
			{
				// randomly pick an edge
				int edge_x = edges[edge_sample * 2];
				int edge_y = edges[(edge_sample * 2) + 1];
				float deviation = 0;

				if (horizontal)
				{
					float predicted_edge_y = yy0 + ((edge_x - xx0) * gradient);
					deviation = ABS(predicted_edge_y - edge_y);
				}
				else
				{
					float predicted_edge_x = xx0 + ((edge_y - yy0) * gradient);
					deviation = ABS(predicted_edge_x - edge_x);
				}

				if (deviation < max_deviation)
				{
					line.push_back(edge_sample * 2);
					av_x += edge_x;
					av_y += edge_y;
				}
			}

			if (line.size() > 0)
			{
				// average edge position
				int edges_on_line = (int)line.size();
				av_x /= edges_on_line;
				av_y /= edges_on_line;

				for (int i = 0; i < (int)line.size(); i++)
				{
					int idx = line[i];
					if (horizontal)
					{
						if (edges[idx] < av_x)
						{
							x0 += edges[idx];
							y0 += edges[idx + 1];
							hits0++;
						}
						else
						{
							x1 += edges[idx];
							y1 += edges[idx + 1];
							hits1++;
						}
					}
					else
					{
						if (edges[idx+1] < av_y)
						{
							x0 += edges[idx];
							y0 += edges[idx + 1];
							hits0++;
						}
						else
						{
							x1 += edges[idx];
							y1 += edges[idx + 1];
							hits1++;
						}
					}

					if (remove_edges)
					{
						edges.erase(edges.begin() + idx + 1);
						edges.erase(edges.begin() + idx);
					}
				}

				x0 /= hits0;
				y0 /= hits0;
				x1 /= hits1;
				y1 /= hits1;
			}
		}
		else x0 = 9999;
	}

	return(max_hits);
}

/*!
 * \brief BestFitLine
 * \param edges
 * \param max_deviation maximum deviation of edge positions from the ideal best fit line in pixels
 * \param baseline_length_pixels baseline distance in pixels to be used when considering possible best fit lines
 * \param x0 start x coordinate of the best fit line
 * \param y0 start y coordinate of the best fit line
 * \param x1 end x coordinate of the best fit line
 * \param y1 end y coordinate of the best fit line
 * \return
 */
float shapes::BestFitLine(
    std::vector<int> &edges,
    float max_deviation,
    int baseline_length_pixels,
    float& x0,
    float& y0,
    float& x1,
    float& y1)
{
    int increment1 = 4; //2;
    int increment2 = 5; //4;

    float max_hits = 0;
    bool horizontal = false;
    float grad, error, hits;
    float dx, dy, dx2, dy2;
    float abs_dx, abs_dy;
    int baseline_offset = baseline_length_pixels * 2;
    int endpoint = edges.size() / 2;
    std::vector<int> hits_index;
    std::vector<float> weights;
    std::vector<int> best_hits_index;
    std::vector<float> best_weights;

    // for each possible baseline
    for (int i = (int)edges.size() - baseline_offset - 2; i >= endpoint; i -= increment1)
    {
    	hits_index.clear();
    	weights.clear();

    	// baseline start
        float prev_xx0 = edges[i] + 0.5f;
        float prev_yy0 = edges[i + 1] + 0.5f;

        // baseline end
        float xx0 = edges[edges.size() - 2 - i] + 0.5f;
        float yy0 = edges[edges.size() - 1 - i] + 0.5f;

        // is this baseline horizontally oriented?
        dx = xx0 - prev_xx0;
        dy = yy0 - prev_yy0;
        if (dx >= 0)
            abs_dx = dx;
        else
            abs_dx = -dx;
        if (dy >= 0)
            abs_dy = dy;
        else
            abs_dy = -dy;

        if (abs_dx > abs_dy)
        {
        	// gradient of the baseline
            grad = dy / dx;
            horizontal = true;
        }
        else
        {
        	// gradient of the baseline
            grad = dx / dy;
            horizontal = false;
        }

        // check all edges for interceptions with this baseline
        hits = 0;
        for (int j = (int)edges.size() - 2; j >= 0; j -= increment2)
        {
            float xx1 = edges[j] + 0.5f;
            float yy1 = edges[j + 1] + 0.5f;
            dx2 = xx1 - prev_xx0;
            dy2 = yy1 - prev_yy0;
            if (horizontal)
                error = dy2 - (dx2 * grad);
            else
                error = dx2 - (dy2 * grad);

            // is the edge close to the ideal line through the current baseline?
            if ((error > -max_deviation) &&
                (error < max_deviation))
            {
            	float weight = 1.0f / (1.0f + (error*error));
            	hits_index.push_back(j);
            	weights.push_back(weight);
                hits += weight;
            }
        }

        // store the best results
        if (hits > max_hits)
        {
        	best_hits_index.clear();
        	best_weights.clear();
        	for (int j = hits_index.size()-1; j >= 0; j--)
        	{
        		best_weights.push_back(weights[j]);
        	    best_hits_index.push_back(i);
        	    best_hits_index.push_back(hits_index[j]);
        	}

            max_hits = hits;
            x0 = prev_xx0;
            y0 = prev_yy0;
            x1 = xx0;
            y1 = yy0;
        }
    }


/*
    // for each possible baseline
    for (int i = (int)edges.size() - baseline_offset - 4; i >= 0; i -= increment1)
    {
    	hits_index.clear();
    	weights.clear();

    	// baseline start
        float prev_xx0 = edges[i] + 0.5f;
        float prev_yy0 = edges[i + 1] + 0.5f;

        // baseline end
        float xx0 = edges[i + baseline_offset] + 0.5f;
        float yy0 = edges[i + baseline_offset + 1] + 0.5f;

        // is this baseline horizontally oriented?
        dx = xx0 - prev_xx0;
        dy = yy0 - prev_yy0;
        if (dx >= 0)
            abs_dx = dx;
        else
            abs_dx = -dx;
        if (dy >= 0)
            abs_dy = dy;
        else
            abs_dy = -dy;

        if (abs_dx > abs_dy)
        {
        	// gradient of the baseline
            grad = dy / dx;
            horizontal = true;
        }
        else
        {
        	// gradient of the baseline
            grad = dx / dy;
            horizontal = false;
        }

        // check all edges for interceptions with this baseline
        hits = 0;
        for (int j = (int)edges.size() - 2; j >= 0; j -= increment2)
        {
        	// edge position
            float xx1 = edges[j] + 0.5f;
            float yy1 = edges[j + 1] + 0.5f;

            dx2 = xx1 - prev_xx0;
            dy2 = yy1 - prev_yy0;
            if (horizontal)
                error = dy2 - (dx2 * grad);
            else
                error = dx2 - (dy2 * grad);

            // is the edge close to the ideal line through the current baseline?
            if ((error > -max_deviation) &&
                (error < max_deviation))
            {
            	float weight = 1.0f / (1.0f + (error*error));
            	hits_index.push_back(j);
            	weights.push_back(weight);
            	hits+= weight;
            }
        }

        if (hits > max_hits)
        {
        	best_hits_index.clear();
        	best_weights.clear();
        	for (int j = hits_index.size()-1; j >= 0; j--)
        	{
        		best_weights.push_back(weights[j]);
        	    best_hits_index.push_back(i);
        	    best_hits_index.push_back(hits_index[j]);
        	}

        	max_hits = hits;
            x0 = prev_xx0;
            y0 = prev_yy0;
            x1 = xx0;
            y1 = yy0;
        }
    }
*/


    if (best_hits_index.size() > 2)
    {
    	// get the average edge position
    	float av_x = 0;
    	float av_y = 0;
    	int av_hits = 0;
    	int horizontals = 0;
    	int verticals = 0;
    	float total_weight = 0;
    	int w = 0;
		for (int k = 0; k < (int)best_hits_index.size(); k += 2, w++)
		{
			int i = best_hits_index[k];
			int j = best_hits_index[k + 1];
			float weight = best_weights[w];
			float xx0 = edges[i] + 0.5f;
			float yy0 = edges[i + 1] + 0.5f;
			float xx1 = edges[j] + 0.5f;
			float yy1 = edges[j + 1] + 0.5f;

			dx = xx1 - xx0;
			dy = yy1 - yy0;
			abs_dx = ABS(dx);
			abs_dy = ABS(dy);

			if (abs_dx > abs_dy)
				horizontals++;
			else
				verticals++;

			av_x += xx0*weight;
			av_y += yy0*weight;
			av_x += xx1*weight;
			av_y += yy1*weight;
			av_hits += 2;
			total_weight += weight*2;
		}
		av_x /= total_weight;
		av_y /= total_weight;

    	float xxx1 = 0;
    	float yyy1 = 0;
    	float hits1 = 0;

    	float xxx2 = 0;
    	float yyy2 = 0;
    	float hits2 = 0;

    	if (horizontals > verticals)
    		horizontal = true;
    	else
    		horizontal = false;

    	w = 0;
		for (int k = 0; k < (int)best_hits_index.size(); k += 2, w++)
		{
			int i = best_hits_index[k];
			int j = best_hits_index[k + 1];
			float weight = best_weights[w];
			float xx0 = edges[i] + 0.5f;
			float yy0 = edges[i + 1] + 0.5f;
			float xx1 = edges[j] + 0.5f;
			float yy1 = edges[j + 1] + 0.5f;

			if (horizontal)
			{
				if (xx0 < av_x)
				{
				    xxx1 += xx0 * weight;
				    yyy1 += yy0 * weight;
				    hits1 += weight;
				}
				else
				{
				    xxx2 += xx0 * weight;
				    yyy2 += yy0 * weight;
				    hits2 += weight;
				}
				if (xx1 < av_x)
				{
				    xxx1 += xx1 * weight;
				    yyy1 += yy1 * weight;
				    hits1 += weight;
				}
				else
				{
				    xxx2 += xx1 * weight;
				    yyy2 += yy1 * weight;
				    hits2 += weight;
				}
			}
			else
			{
				if (yy0 < av_y)
				{
				    xxx1 += xx0 * weight;
				    yyy1 += yy0 * weight;
				    hits1 += weight;
				}
				else
				{
				    xxx2 += xx0 * weight;
				    yyy2 += yy0 * weight;
				    hits2 += weight;
				}
				if (yy1 < av_y)
				{
				    xxx1 += xx1 * weight;
				    yyy1 += yy1 * weight;
				    hits1 += weight;
				}
				else
				{
				    xxx2 += xx1 * weight;
				    yyy2 += yy1 * weight;
				    hits2 += weight;
				}
			}
		}

		if ((hits1 > 0) && (hits2 > 0))
		{
    	    x0 = xxx1 / hits1;
    	    y0 = yyy1 / hits1;
    	    x1 = xxx2 / hits2;
    	    y1 = yyy2 / hits2;
		}
    }

    return (max_hits);
}


/*!
 * \brief rotates the given set of edges around the given origin
 * \param edges list of edge points
 * \param centre_x x coordinate of the origin
 * \param centre_y y coordinate of the origin
 * \param rotate_angle angle to rotate  radians
 * \return
 */
void shapes::RotateEdges(
    std::vector<int> &edges,
    int centre_x,
    int centre_y,
    float rotate_angle,
    std::vector<int>& rotated)
{
    rotated.erase(rotated.begin(), rotated.end());

    for (int i = 0; i < (int)edges.size(); i += 2)
    {
        int dx = edges[i] - centre_x;
        int dy = edges[i + 1] - centre_y;
        float hyp = (float)sqrt((dx * dx) + (dy * dy));
        if ((int)hyp != 0)
        {
            float angle = (float)acos(dx / hyp);
            rotated.push_back(centre_x + (int)(hyp * (float)sin(angle + rotate_angle)));
            rotated.push_back(centre_y + (int)(hyp * (float)cos(angle + rotate_angle)));
        }
    }
}

/*!
 * \brief returns edges around the periphery of the shape
 * \param edges edges within the square region
 * \param tx returned top x position of the bounding box for the given edges
 * \param ty returned top y position of the bounding box for the given edges
 * \param bx returned bottom x position of the bounding box for the given edges
 * \param by returned bottom y position of the bounding box for the given edges
 * \param left edge positions along the left side
 * \param right edge positions along the right side
 * \param top edge positions along the top side
 * \param bottom edge positions along the bottom side
 * \param orientation_histogram_quantization_degrees quantization in degrees to be used when creating orientation histograms
 * \param perimeter_sampling_step_size step size to be used when sampling the perimeter
 * \param dominant_orientation main orientation in radians
 * \param dominant_edges
 * \param non_dominant_edges
 * \param side_edges
 */
void shapes::GetPeripheralEdges(
	std::vector<int> &edges,
	int &tx, int &ty, int &bx, int &by,
	int* left, int* right, int* top, int* bottom)
{
    // find the bounding box for all edges
    tx = 99999;
    ty = 99999;
    bx = -99999;
    by = -99999;
    for (int i = (int)edges.size() - 2; i >= 0; i -= 2)
    {
        int x = edges[i];
        int y = edges[i + 1];
        if (x < tx) tx = x;
        if (y < ty) ty = y;
        if (x > bx) bx = x;
        if (y > by) by = y;
    }

    int w = bx - tx;
    int h = by - ty;

    if ((w > 0) && (h > 0))
    {
        int horizontal_length = w + 1;
        int vertical_length = h + 1;

        // clear arrays
        memset(left, 0, vertical_length*sizeof(int));
        memset(right, 0, vertical_length*sizeof(int));
        memset(top, 0, horizontal_length*sizeof(int));
        memset(bottom, 0, horizontal_length*sizeof(int));

        for (int i = (int)edges.size() - 2; i >= 0; i -= 2)
        {
            int x = edges[i];
            int x2 = x - tx;
            int y = edges[i + 1];
            int y2 = y - ty;

            // left side
            if ((left[y2] == 0) ||
                (x < left[y2]))
                left[y2] = x;

            // right side
            if ((right[y2] == 0) ||
                (x > right[y2]))
                right[y2] = x;

            // top
            if ((top[x2] == 0) ||
                (y < top[x2]))
                top[x2] = y;

            // bottom
            if ((bottom[x2] == 0) ||
                (y > bottom[x2]))
                bottom[x2] = y;
        }
    }
}

/*!
 * \brief returns a polygon representing the periphery of a square region
 * \param detection_method method used to detect the periphery
 * \param erode_dilate erosion or dilation level
 * \param best_fit_max_deviation_pixels maximum deviation in pixels when fitting lines
 * \param best_fit_baseline_pixels baseline distance in pixels to be used when fitting lines
 * \param perim returned periphery
 */
void shapes::GetPeripheral(
	int detection_method,
	int erode_dilate,
	float best_fit_max_deviation_pixels,
	int best_fit_baseline_pixels,
	int tx, int ty, int bx, int by,
	int* left, int* right,
	int* top, int* bottom,
	polygon2D *perim,
	std::vector<std::vector<int> > &side_edges,
	std::vector<std::vector<int> > &result)
{
	const int minimum_interior_angle = 80;
	const int maximum_interior_angle = 100;
	int w = bx - tx;
	int h = by - ty;

	if ((w > 0) && (h > 0))
	{
		float left_x0 = 0, left_y0 = 0;
		float left_x1 = 0, left_y1 = 0;
		float right_x0 = 0, right_y0 = 0;
		float right_x1 = 0, right_y1 = 0;
		float top_x0 = 0, top_y0 = 0;
		float top_x1 = 0, top_y1 = 0;
		float bottom_x0 = 0, bottom_y0 = 0;
		float bottom_x1 = 0, bottom_y1 = 0;

		BestFitLine(side_edges[0],
					best_fit_max_deviation_pixels,
					best_fit_baseline_pixels,
					left_x0, left_y0,
					left_x1, left_y1);

		BestFitLine(side_edges[2],
					best_fit_max_deviation_pixels,
					best_fit_baseline_pixels,
					top_x0, top_y0,
					top_x1, top_y1);

		bool is_valid = true;

		// find the intersection between the left side and the top side
		float ix = 0;
		float iy = 0;
		geometry::intersection(left_x1, left_y1, left_x0, left_y0,
							   top_x1, top_y1, top_x0, top_y0,
							   ix, iy);
		if (ix == GEOM_INVALID)
		{
			is_valid = false;
		}
		else
		{
			// get the interior angle between these lines
			float interior_angle =
				geometry::threePointAngle(
					left_x0, left_y0,
					ix, iy,
					top_x1, top_y1);

			// does this vaguely resemble a right angle ?
			float interior_angle_degrees = interior_angle * 180 / PI;
			if ((interior_angle_degrees > minimum_interior_angle) &&
				(interior_angle_degrees < maximum_interior_angle))
			{
				perim->Add(ix, iy);

				BestFitLine(
				    side_edges[1],
					best_fit_max_deviation_pixels,
					best_fit_baseline_pixels,
					right_x0, right_y0,
					right_x1, right_y1);

				// find the intersection between the right side and the top side
				ix = 0;
				iy = 0;
				geometry::intersection(right_x1, right_y1, right_x0, right_y0,
									   top_x0, top_y0, top_x1, top_y1,
									   ix, iy);
				if (ix == GEOM_INVALID)
				{
					is_valid = false;
				}
				else
				{
					// get the interior angle between these lines
					interior_angle =
						geometry::threePointAngle(
							right_x0, right_y0,
							ix, iy,
							top_x0, top_y0);

					// does this vaguely resemble a right angle ?
					interior_angle_degrees = interior_angle * 180 / PI;
					if ((interior_angle_degrees > minimum_interior_angle) &&
						(interior_angle_degrees < maximum_interior_angle))
					{
						perim->Add(ix, iy);

						BestFitLine(
						    side_edges[3],
							best_fit_max_deviation_pixels,
							best_fit_baseline_pixels,
							bottom_x0, bottom_y0,
							bottom_x1, bottom_y1);

						// find the intersection between the right side and the bottom side
						ix = 0;
						iy = 0;
						geometry::intersection(right_x1, right_y1, right_x0, right_y0,
											  bottom_x0, bottom_y0, bottom_x1, bottom_y1,
											  ix, iy);
						if (ix == GEOM_INVALID)
						{
							is_valid = false;
						}
						else
						{
							// get the interior angle between these lines
							interior_angle =
								geometry::threePointAngle(
									right_x0, right_y0,
									ix, iy,
									bottom_x0, bottom_y0);

							// does this vaguely resemble a right angle ?
							interior_angle_degrees = interior_angle * 180 / PI;
							if ((interior_angle_degrees > minimum_interior_angle) &&
								(interior_angle_degrees < maximum_interior_angle))
							{
								perim->Add(ix, iy);

								// find the intersection between the left side and the bottom side
								ix = 0;
								iy = 0;
								geometry::intersection(left_x1, left_y1, left_x0, left_y0,
													   bottom_x0, bottom_y0, bottom_x1, bottom_y1,
													   ix, iy);
								if (ix == GEOM_INVALID)
									is_valid = false;
								else
									perim->Add(ix, iy);
							}
							else
							{
								is_valid = false;
							}
						}
					}
					else
					{
						is_valid = false;
					}
				}
			}
			else
		    {
				is_valid = false;
		    }
		}

		if (is_valid)
		{
			// left and right
			std::vector<int> left_edges;
			std::vector<int> right_edges;
			for (int y = h; y >= 0; y--)
			{
				if (left[y] != 0)
				{
					left_edges.push_back(left[y]);
					left_edges.push_back(ty + y);
				}
				if (right[y] != 0)
				{
					right_edges.push_back(right[y]);
					right_edges.push_back(ty + y);
				}
			}

			// top and bottom
			std::vector<int> top_edges;
			std::vector<int> bottom_edges;
			for (int x = w; x >= 0; x--)
			{
				if (top[x] != 0)
				{
					top_edges.push_back(tx + x);
					top_edges.push_back(top[x]);
				}
				if (bottom[x] != 0)
				{
					bottom_edges.push_back(tx + x);
					bottom_edges.push_back(bottom[x]);
				}
			}

			float aspect_check = perim->getShortestSide() / perim->getLongestSide();
			if (aspect_check > 0.2f)
			{
				result.push_back(left_edges);
				result.push_back(right_edges);
				result.push_back(top_edges);
				result.push_back(bottom_edges);
			}
			else perim->Clear();
		}

	    // shrink the perimeter according to the erosion/dilation value
	    if ((is_valid) && (erode_dilate != 0))
	    {
	        if ((int)perim->x_points.size() == 4)
	        {
	            float shrink_percent = (erode_dilate * 2) / (perim->getPerimeterLength() / 4.0f);
	            polygon2D* shrunken_perim = new polygon2D();
	            perim->Scale(1.0f - shrink_percent, shrunken_perim);

	            for (int vertex = 0; vertex < 4; vertex++)
	            {
	                perim->x_points[vertex] = shrunken_perim->x_points[vertex];
	                perim->y_points[vertex] = shrunken_perim->y_points[vertex];
	            }
	            delete shrunken_perim;
	        }
	        else perim->Clear();
	    }
	}
}
