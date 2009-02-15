/*
    image processing functions
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

#include "processimage.h"

// ********** public methods **********

/*!
 * \brief apply a yellow filter to the image
 * \param img colour image data
 * \param img_width width of the image
 * \param img_height height of the image
 * \param filtered returned filtered image
 */
void processimage::yellowFilter(
    unsigned char* img,
    int img_width,
    int img_height,
    unsigned char* filtered)
{
	// clear the filtered image
	memset(filtered, 0, img_width * img_height * 3 * sizeof(unsigned char));

	int* histogram = new int[256];
	int* temp_buffer = new int[256];
	memset(histogram, 0, 256*sizeof(int));

	// apply filter
	for (int i = (img_width * img_height * 3)-3; i >= 0; i -= 3)
	{
		int r = img[i+2];
		int g = img[i+1];
		int b = img[i];

		int v = r - g;
		if (v < 0) v = -v;
        int yellow = (int)((r + g) - ((b + v)*2));
        if (yellow > 0)
        {
        	if (yellow > 255) yellow = 255;
        	if (yellow < 0) yellow = 0;

        	unsigned char y = (unsigned char)yellow;
        	filtered[i] = y;
        	filtered[i+1] = y;
        	filtered[i+2] = y;

            histogram[yellow]++;
        }
	}
	float MeanDark = 0;
	float MeanLight = 0;
	float DarkRatio = 0;
	float threshold = thresholding::GetGlobalThreshold(histogram, 256, 0, temp_buffer, MeanDark, MeanLight, DarkRatio);

	for (int i = (img_width * img_height * 3)-3; i >= 0; i -= 3)
	{
        if (filtered[i] > 0)
        {
        	if (filtered[i] < threshold)
        	{
        	    filtered[i] = 0;
        	    filtered[i + 1] = 0;
        	    filtered[i + 2] = 0;
        	}
        }
	}

	delete[] histogram;
	delete[] temp_buffer;
}


/*!
 * \brief convert a mono image to a colour image
 * \param img_mono mono image data
 * \param img_width image width
 * \param img_height image height
 * \param output optional colour image buffer
 * \return colour image data
 */
void processimage::colourImage(
    unsigned char* img_mono,
    int img_width,
    int img_height,
    unsigned char* colour_image)
{
    int n = 0;
    for (int i = 0; i < img_width * img_height; i++)
    {
        unsigned char b = img_mono[i];
        colour_image[n++] = b;
        colour_image[n++] = b;
        colour_image[n++] = b;
    }
}

/*!
 * \brief convert the given colour image to mono
 * \param img_colour
 * \param img_width
 * \param img_height
 * \param conversion_type method for converting to mono
 * \param mono_image output mono image
 * \return
 */
void processimage::monoImage(
    unsigned char* img_colour,
    int img_width,
    int img_height,
    int conversion_type,
    unsigned char* mono_image)
{
    int n = 0;
    int tot = 0;
    int luminence = 0;

    for (int i = 0; i < img_width * img_height * 3; i += 3, n++)
    {
        switch (conversion_type)
        {
            case 0: // magnitude
                {
                    tot = 0;
                    for (int col = 0; col < 3; col++)
                        tot += img_colour[i + col];

                    mono_image[n] = (unsigned char)(tot * 0.3333333333f);
                    break;
                }
            case 1: // luminance
                {
                    luminence = ((img_colour[i + 2] * 299) +
                                 (img_colour[i + 1] * 587) +
                                 (img_colour[i] * 114)) / 1000;
                    //if (luminence > 255) luminence = 255;
                    mono_image[n] = (unsigned char)luminence;
                    break;
                }
        }
    }
}

/*!
 * \brief sub-sample a mono image
 * \param img image pixel data (one byte per pixel)
 * \param img_width width of the image
 * \param img_height height of the image
 * \return
 */
void processimage::downSample(
    unsigned char* img,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int new_width,
    int new_height,
    unsigned char *result)
{
    if (!((new_width == img_width) && (new_height == img_height)))
    {
        int n = 0;
        int pixels = img_width * img_height * bytes_per_pixel;

        for (int y = 0; y < new_height; y++)
        {
            int yy = y * (img_height - 1) / new_height;
            for (int x = 0; x < new_width; x++)
            {
                int xx = x * (img_width - 1) / new_width;
                int n2 = ((yy * img_width) + xx) * bytes_per_pixel;
                if (n2 < pixels - bytes_per_pixel) result[n] = img[n2];
                n++;
            }
        }
    }
    else
    {
        memcpy(result, img, img_width * img_height * bytes_per_pixel);
    }
}

/*!
 * \brief sub-sample a mono image
 * \param img image pixel data (one byte per pixel)
 * \param img_width width of the image
 * \param img_height height of the image
 * \param bytes_per_pixel
 * \param new_width
 * \param new_height
 * \param new_bytes_per_pixel
 * \return
 */
void processimage::downSample(
    unsigned char* img,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int new_width,
    int new_height,
    int new_bytes_per_pixel,
    unsigned char *result)
{
    bool convert_to_mono = false;
    if ((bytes_per_pixel == 3) && (new_bytes_per_pixel == 1))
        convert_to_mono = true;

    bool same_bytes_per_pixel = false;
    if (bytes_per_pixel == new_bytes_per_pixel)
        same_bytes_per_pixel = true;

    bool convert_to_colour = false;
    if ((bytes_per_pixel == 1) && (new_bytes_per_pixel == 3))
        convert_to_colour = true;

    int n = 0;
    int pixels = img_width * img_height * bytes_per_pixel;

    for (int y = 0; y < new_height; y++)
    {
        int yy = y * (img_height - 1) / new_height;
        for (int x = 0; x < new_width; x++)
        {
            int xx = x * (img_width - 1) / new_width;

            if (convert_to_mono)
            {
                int n2 = ((yy * img_width) + xx) * bytes_per_pixel;
                if (n2 < pixels - bytes_per_pixel) result[n] = img[n2];
                n++;
            }
            if (same_bytes_per_pixel)
            {
                if (bytes_per_pixel == 1)
                {
                    int n2 = (yy * img_width) + xx;
                    result[n++] = img[n2++];
                }
                else
                {
                    int n2 = ((yy * img_width) + xx) * bytes_per_pixel;
                    for (int col = 0; col < bytes_per_pixel; col++)
                        result[n++] = img[n2++];
                }
            }
            if (convert_to_colour)
            {
                int n2 = (yy * img_width) + xx;
                if (n2 < pixels - 1)
                {
                    result[n++] = img[n2];
                    result[n++] = img[n2];
                    result[n++] = img[n2];
                }
            }
        }
    }
}

/*!
 * \brief sub-sample a mono image to half its original size
 * \param img image data (one byte per pixel)
 * \param img_width width of the image
 * \param img_height height of the image
 * \return image of half the size
 */
void processimage::downSample(
    unsigned char* img,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    unsigned char *result)
{
    int new_width = img_width / 2;
    int new_height = img_height / 2;

    int pixels = img_width * img_height;

    if (bytes_per_pixel == 1)
    {
        int yy = 0;
        int stride = img_width;
        for (int y = 0; y < new_height-1; y++)
        {
            int n = y * new_width;
            int xx = yy * stride;
            int xx2 = xx + stride;
            for (int x = 0; x < new_width-1; x++)
            {
                if (xx2 + 1 < pixels)
                {
                    float v = img[xx] + img[xx + 1] +
                              img[xx2] + img[xx2 + 1];

                    result[n] = (unsigned char)(v * 0.25f);
                }
                n++;
                xx += 2;
                xx2 += 2;
            }
            yy += 2;
        }
    }
    else
    {
        pixels *= 3;

        int yy = 0;
        int stride = img_width * 3;
        for (int y = 0; y < new_height-1; y++)
        {
            int n = y * new_width * 3;
            int xx = yy * stride;
            int xx2 = xx + stride;
            for (int x = 0; x < new_width-1; x++)
            {
                if (xx2 + 3 < pixels)
                {
                    for (int col = 0; col < 3; col++)
                    {
                        float v = img[xx + col] + img[xx + 3 + col] +
                                  img[xx2 + col] + img[xx2 + 3 + col];

                        result[n + col] = (unsigned char)(v * 0.25f);
                    }
                }
                n += 3;
                xx += 6;
                xx2 += 6;
            }
            yy += 2;
        }
    }
}


/*!
 * \brief sub-sample a mono image by a certain factor
 * \param img mono image data (one byte per pixel)
 * \param img_width width of the image
 * \param img_height height of the image
 * \param bytes_per_pixel number of bytes per pixel
 * \param buffer temporary buffer used for downsampling
 * \param factor downsampling factor
 * \param result downsampled result
 */
void processimage::downSample(
    unsigned char* img,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int factor,
    int *buffer0,
    int *buffer1,
    unsigned char *result)
{
    if (factor <= 2)
    {
        if (factor == 2)
        {
            // do a single downsample (half original image size)
            downSample(img, img_width, img_height, bytes_per_pixel, result);
        }
    }
    else
    {
        // do multiple downsamples
        // note here that we sum pixels and then divide at
        // the end for greater accuracy
        int pixels = img_width * img_height;
        int *img2 = buffer0; //new int[pixels];
        for (int i = pixels - 1; i >= 0; i--) img2[i] = img[i];

        int target_img_width = img_width / factor;

        int* img3 = buffer1;
        int *temp_img = NULL;
        int grouped_pixels = 2;
        while (img_width > target_img_width)
        {
            if (temp_img != NULL) delete[] temp_img;
            //img3 = buffer1; //new int[(img_width/2)*(img_height/2)];
            downSampleSum(img2, img_width, img_height, bytes_per_pixel, img3);
            img_width /= 2;
            img_height /= 2;
            temp_img = img2;
            img2 = img3;
            img3 = temp_img;
            grouped_pixels *= grouped_pixels;
        }

        // divide the summed pixel values
        float divisor = 1.0f / grouped_pixels;
        for (int i = (img_width * img_height) - 1; i >= 0; i--) result[i] = (unsigned char)((float)img2[i] * divisor);

        //if (temp_img != NULL)
        //    delete[] temp_img;
        //else
        //    delete[] img2;

        //if (img3 != NULL) delete[] img3;
    }
}



/*!
 * \brief mirror the given image
 * \param bmp image data
 * \param wdth width of the image
 * \param hght height of the image
 * \param bytes_per_pixel number of bytes per pixel
 * \param mirrored mirrored version
 */
void processimage::Mirror(
    unsigned char* bmp,
    int wdth,
    int hght,
    int bytes_per_pixel,
    unsigned char* mirrored)
{
    //unsigned char* mirrored = new unsigned char[wdth * hght * bytes_per_pixel];

    for (int y = 0; y < hght; y++)
    {
        int n0 = (y * wdth);
        for (int x = 0; x < wdth; x++)
        {
            int n1 = (n0 + x) * bytes_per_pixel;
            int x2 = wdth - 1 - x;
            int n2 = (n0 + x2) * bytes_per_pixel;
            for (int col = 0; col < bytes_per_pixel; col++)
                mirrored[n2 + col] = bmp[n1 + col];
        }
    }
}

/*!
 * \brief flip the given image
 * \param bmp image data
 * \param wdth width of the image
 * \param hght height of the image
 * \param bytes_per_pixel number of bytes per pixel
 * \return flipped varesion
 */
void processimage::Flip(
    unsigned char* bmp,
    int wdth,
    int hght,
    int bytes_per_pixel,
    unsigned char* flipped)
{
    //unsigned char* flipped = new unsigned char[wdth * hght * bytes_per_pixel];

    for (int y = 0; y < hght; y++)
    {
        int n0 = (y * wdth);
        for (int x = 0; x < wdth; x++)
        {
            int n1 = (n0 + x) * bytes_per_pixel;
            int n2 = (((hght - 1 - y) * wdth) + x) * bytes_per_pixel;
            for (int col = 0; col < bytes_per_pixel; col++)
                flipped[n2 + col] = bmp[n1 + col];
        }
    }
}


/*!
 * \brief Dilate
 * \param width
 * \param height
 * \param bytes_per_pixel
 * \param radius
 * \return
 */
/*
void processimage::Dilate(
    unsigned char* bmp,
    int width,
    int height,
    int bytes_per_pixel,
    int radius,
    unsigned char* result)
{
    int intensity = 0;
    int max_intensity = 0;
    int n, n2, n3;
    int pixels = width * height * bytes_per_pixel;

    unsigned char *source = new unsigned char[pixels];
    memcpy(source, bmp, pixels);

    int r = 1;

    for (int j = 0; j < radius; j++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int best_n = 0;
                max_intensity = 0;

                for (int yy = y - r; yy <= y + r; yy++)
                {
                    if ((yy > -1) && (yy < height))
                    {
                        n2 = yy * width * bytes_per_pixel;
                        for (int xx = x - r; xx <= x + r; xx++)
                        {
                            if ((xx > -1) && (xx < width))
                            {
                                n3 = n2 + (xx * bytes_per_pixel);

                                // get the intensity value of this pixel
                                intensity = 0;
                                for (int i = 0; i < bytes_per_pixel; i++)
                                    intensity += source[n3 + i];

                                //  this the biggest intensity ?
                                if (intensity > max_intensity)
                                {
                                    max_intensity = intensity;
                                    best_n = n3;
                                }

                            }
                        }
                    }
                }

                // result pixel
                n = (((y * width) + x) * bytes_per_pixel);
                for (int i = 0; i < bytes_per_pixel; i++)
                    result[n + i] = source[best_n + i];
            }
        }

        if (j < radius-1)
            memcpy(source, result, pixels);
    }
    delete[] source;
}
*/

/*!
 * \brief ErodeDilate
 * \param width
 * \param height
 * \param radius
 */
void processimage::ErodeDilate(
    unsigned char* bmp_mono,
    int width,
    int height,
    int radius,
    unsigned char* result_erode,
    unsigned char* result_dilate)
{
	int pixels = width * height;
    unsigned char *source_erode = new unsigned char[pixels];
    memcpy(source_erode, bmp_mono, pixels);

    unsigned char *source_dilate = new unsigned char[pixels];
    memcpy(source_dilate, bmp_mono, pixels);

    for (int r = 0; r < radius; r++)
    {
        int n = 0;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                for (int i = 0; i < 2; i++)
                {
                    if (i == 0)
                    {
                        if (x > 0)
                        {
                            // left
                            if (source_dilate[n - 1] > source_dilate[n])
                                result_dilate[n] = source_dilate[n - 1];
                            else
                                result_dilate[n] = source_dilate[n];

                            // top left
                            if (y > 0)
                                if (source_dilate[n - width - 1] > result_dilate[n]) result_dilate[n] = source_dilate[n - width - 1];

                            // bottom left
                            if (y < height - 1)
                                if (source_dilate[n + width - 1] > result_dilate[n]) result_dilate[n] = source_dilate[n + width - 1];
                        }

                        if (x < width - 1)
                        {
                            // right
                            if (source_dilate[n + 1] > result_dilate[n]) result_dilate[n] = source_dilate[n + 1];

                            // top right
                            if (y > 0)
                                if (source_dilate[n - width + 1] > result_dilate[n]) result_dilate[n] = source_dilate[n - width + 1];

                            // bottom right
                            if (y < height - 1)
                                if (source_dilate[n + width + 1] > result_dilate[n]) result_dilate[n] = source_dilate[n + width + 1];
                        }

                        // above
                        if (y > 0)
                            if (source_dilate[n - width] > result_dilate[n]) result_dilate[n] = source_dilate[n - width];

                        // below
                        if (y < height - 1)
                            if (source_dilate[n + width] > result_dilate[n]) result_dilate[n] = source_dilate[n + width];
                    }
                    else
                    {
                        if (x > 0)
                        {
                            if (source_erode[n - 1] < source_erode[n])
                                result_erode[n] = source_erode[n - 1];
                            else
                                result_erode[n] = source_erode[n];

                            if (y > 0)
                                if (source_erode[n - width - 1] < result_erode[n]) result_erode[n] = source_erode[n - width - 1];
                            if (y < height - 1)
                                if (source_erode[n + width - 1] < result_erode[n]) result_erode[n] = source_erode[n + width - 1];
                        }

                        if (x < width - 1)
                        {
                            if (source_erode[n + 1] < result_erode[n]) result_erode[n] = source_erode[n + 1];

                            if (y > 0)
                                if (source_erode[n - width + 1] < result_erode[n]) result_erode[n] = source_erode[n - width + 1];
                            if (y < height - 1)
                                if (source_erode[n + width + 1] < result_erode[n]) result_erode[n] = source_erode[n + width + 1];
                        }

                        if (y > 0)
                            if (source_erode[n - width] < result_erode[n]) result_erode[n] = source_erode[n - width];
                        if (y < height - 1)
                            if (source_erode[n + width] < result_erode[n]) result_erode[n] = source_erode[n + width];
                    }

                }

                n++;
            }
        }

        if (r < radius - 1)
        {
	        memcpy(source_erode, result_erode, pixels);
	        memcpy(source_dilate, result_dilate, pixels);
        }
    }
    delete[] source_erode;
    delete[] source_dilate;
}

/*!
 * \brief Opening
 * \param width
 * \param height
 * \param radius
 * \return
 */
void processimage::Opening(
    unsigned char* bmp,
    int width,
    int height,
    unsigned char* buffer,
    int radius,
    unsigned char* result)
{
	unsigned char *img1 = new unsigned char[width * height];
    Erode(bmp, width, height, buffer, radius, img1);
    Dilate(img1, width, height, buffer, radius, result);
    delete[] img1;
}

/*!
 * \brief Closing
 * \param width
 * \param height
 * \param radius
 * \return
 */
void processimage::Closing(
    unsigned char* bmp,
    int width,
    int height,
    unsigned char* buffer,
    int radius,
    unsigned char* result)
{
	unsigned char *img1 = new unsigned char[width * height];
    Dilate(bmp, width, height, buffer, radius, img1);
    Erode(img1, width, height, buffer, radius, result);
    delete[] img1;
}

/*!
 * \brief Dilate
 * \param width
 * \param height
 * \param radius
 * \return
 */
void processimage::Dilate(
    unsigned char* bmp,
    int width,
    int height,
    unsigned char* buffer,
    int radius,
    unsigned char* result)
{
    int pixels = width * height;

    memcpy(buffer, bmp, pixels);

    int n;
    unsigned char v;
    int min = width + 1;
    int max = pixels - width - 1;
    for (int r = 0; r < radius; r++)
    {
        for (int i = min; i < max; i++)
        {
            v = buffer[i];

            // same row
            if (buffer[i - 1] > v) v = buffer[i - 1];
            if (buffer[i + 1] > v) v = buffer[i + 1];

            // row above
            n = i - width - 1;
            if (buffer[n] > v) v = buffer[n];
            n++;
            if (buffer[n] > v) v = buffer[n];
            n++;
            if (buffer[n] > v) v = buffer[n];

            // row below
            n = i + width - 1;
            if (buffer[n] > v) v = buffer[n];
            n++;
            if (buffer[n] > v) v = buffer[n];
            n++;
            if (buffer[n] > v) v = buffer[n];

            result[i] = v;
        }

        if (r < radius - 1)
	        memcpy(buffer, result, pixels);
    }
}

/*!
 * \brief Erode
 * \param width
 * \param height
 * \param radius
 * \return
 */
void processimage::Erode(
    unsigned char* bmp,
    int width,
    int height,
    unsigned char* buffer,
    int radius,
    unsigned char* result)
{
	int pixels = width * height;

    memcpy(buffer, bmp, pixels);

    int n;
    unsigned char v;
    int min = width + 1;
    int max = pixels - width - 1;
    for (int r = 0; r < radius; r++)
    {
        for (int i = min; i < max; i++)
        {
            v = buffer[i];

            // same row
            if (buffer[i - 1] < v) v = buffer[i - 1];
            if (buffer[i + 1] < v) v = buffer[i + 1];

            // row above
            n = i - width - 1;
            if (buffer[n] < v) v = buffer[n];
            n++;
            if (buffer[n] < v) v = buffer[n];
            n++;
            if (buffer[n] < v) v = buffer[n];

            // row below
            n = i + width - 1;
            if (buffer[n] < v) v = buffer[n];
            n++;
            if (buffer[n] < v) v = buffer[n];
            n++;
            if (buffer[n] < v) v = buffer[n];

            result[i] = v;
        }

        if (r < radius - 1)
        {
	        memcpy(buffer, result, pixels);
        }
    }
}


/*!
 * \brief returns a sub image from a larger image
 * \param img large image
 * \param img_width width of the large image
 * \param img_height height of the large image
 * \param bytes_per_pixel bytes per pixel
 * \param tx sub image top left x
 * \param ty sub image top left y
 * \param bx sub image bottom right x
 * \param by sub image bottom right y
 * \return
 */
void processimage::createSubImage(
    unsigned char* img,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int tx,
    int ty,
    int bx,
    int by,
    unsigned char* subimage)
{
	int pixels = img_width * img_height * bytes_per_pixel;
    int sub_width = bx - tx;
    int sub_height = by - ty;
    for (int y = 0; y < sub_height; y++)
    {
        for (int x = 0; x < sub_width; x++)
        {
            int n1 = ((y * sub_width) + x) * bytes_per_pixel;
            int n2 = (((y+ty) * img_width) + (x+tx)) * bytes_per_pixel;

            if ((n2 > -1) && (n2 < pixels - 3))
            {
                for (int col = 0; col < bytes_per_pixel; col++)
                    subimage[n1 + col] = img[n2 + col];
            }
        }
    }
}

/*!
 * \brief returns a sub image from a larger image
 * \param img large image
 * \param img_width width of the large image
 * \param img_height height of the large image
 * \param bytes_per_pixel bytes per pixel
 * \param tx sub image top left x
 * \param ty sub image top left y
 * \param bx sub image bottom right x
 * \param by sub image bottom right y
 * \param subimage cropped image
 */
void processimage::cropImage(
    unsigned char* img,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int tx,
    int ty,
    int bx,
    int by,
    unsigned char* subimage)
{
    int sub_width = bx - tx;
    int sub_height = by - ty;
	int pixels = img_width * img_height * bytes_per_pixel;

    for (int y = 0; y < sub_height; y++)
    {
        for (int x = 0; x < sub_width; x++)
        {
            int n1 = ((y * sub_width) + x) * bytes_per_pixel;
            int n2 = (((y + ty) * img_width) + (x + tx)) * bytes_per_pixel;

            if ((n2 > -1) && (n2 < pixels - bytes_per_pixel))
            {
                for (int col = 0; col < bytes_per_pixel; col++)
                    subimage[n1 + col] = img[n2 + col];
            }
        }
    }
}


/*!
 * \brief returns TRUE if the given image  blank
 * \param img image data to be examined
 * \param step_size step size with which to sample the image
 * \return true if the image  blank
 */
bool processimage::IsBlank(
    unsigned char* img, int img_width, int img_height, int bytes_per_pixel,
    int step_size)
{
	int pixels = img_width * img_height * bytes_per_pixel;
    bool is_blank = true;
    int i = 0;
    while ((i < pixels) && (is_blank))
    {
        if (img[i] > 0) is_blank = false;
        i += step_size;
    }
    return (is_blank);
}



// ********** private methods **********



/*!
 * \brief sub-sample a mono image to half its original size, summing pixel values
 * \param img image data (one byte per pixel)
 * \param img_width width of the image
 * \param img_height height of the image
 * \param bytes_per_pixel number of bytes per pixel
 * \return image of half the size
 */
void processimage::downSampleSum(
    int* img,
    int img_width,
    int img_height,
    int bytes_per_pixel,
    int *result)
{
    int new_width = img_width / 2;
    int new_height = img_height / 2;

    if (!((new_width == img_width) && (new_height == img_height)))
    {
        if (bytes_per_pixel == 1)
        {
            // mono image
            int yy = 0;
            //int stride = img_width;
            int tot_pixels = img_width * img_height;
            for (int y = 0; y < new_height - 1; y++)
            {
                int n = y * new_width;
                int xx = yy * img_width;
                int xx2 = xx + img_width;
                for (int x = 0; x < new_width - 1; x++)
                {
                    if (xx2 + 1 < tot_pixels)
                    {
                        int v = img[xx] + img[xx + 1] +
                                img[xx2] + img[xx2 + 1];

                        result[n] = v;
                    }
                    n++;
                    xx += 2;
                    xx2 += 2;
                }
                yy += 2;
            }
        }
    }

}

