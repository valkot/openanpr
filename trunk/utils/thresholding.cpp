#include "thresholding.h"

/*!
 * \brief calculate a global threshold based upon an intensity histogram
 * \param histogram histogram data
 * \param MeanDark returned mean dark value
 * \param MeanLight returned mean light value
 * \param DarkRatio ratio of dark pixels to light
 * \return global threshold value
 */
float thresholding::GetGlobalThreshold(
    float* histogram,
    int histogram_length,
    float& MeanDark,
    float& MeanLight,
    float& DarkRatio)
{
    float global_threshold = 0;

    float Tmin = 0;
    float Tmax = 0;
    float MinVariance = 999999;  // some large figure
    float currMeanDark, currMeanLight, VarianceDark, VarianceLight;

    float DarkHits = 0;
    float LightHits = 0;
    float BestDarkHits = 0;
    float BestLightHits = 0;
    MeanDark = 0;
    MeanLight = 0;

    // calculate squared magnitudes
    // this avoids unnecessary multiplications later on
    int* histogram_squared_magnitude = new int[histogram_length];
    int b = histogram_length - 1;
    for (int i = histogram_length-1; i >= 0; i--, b--)
        histogram_squared_magnitude[i] = histogram[b] * histogram[b];

    int h;
    float magnitude_sqr, Variance;

    // evaluate all possible thresholds
    for (int grey_level = 255; grey_level >= 0; grey_level--)
    {
        // compute mean and variance for this threshold
        //  a struggle between light and darkness
        DarkHits = 0;
        LightHits = 0;
        currMeanDark = 0;
        currMeanLight = 0;
        VarianceDark = 0;
        VarianceLight = 0;

        for (h = histogram_length-1; h >= 0; h--)
        {
            magnitude_sqr = histogram[h];
            if (h < grey_level)
            {
                currMeanDark += h * magnitude_sqr;
                VarianceDark += (grey_level - h) * magnitude_sqr;
                DarkHits += magnitude_sqr;
            }
            else
            {
                currMeanLight += h * magnitude_sqr;
                VarianceLight += (grey_level - h) * magnitude_sqr;
                LightHits += magnitude_sqr;
            }
        }

        // compute means
        if (DarkHits > 0)
        {
            // rescale into 0-255 range
            currMeanDark = currMeanDark / DarkHits;
            VarianceDark = VarianceDark / DarkHits;
        }
        if (LightHits > 0)
        {
            // rescale into 0-255 range
            currMeanLight = currMeanLight / LightHits;
            VarianceLight = VarianceLight / LightHits;
        }

        Variance = VarianceDark + VarianceLight;
        if (Variance < 0) Variance = -Variance;

        if (Variance < MinVariance)
        {
            MinVariance = Variance;
            Tmin = grey_level;
            MeanDark = currMeanDark;
            MeanLight = currMeanLight;
            BestDarkHits = DarkHits;
            BestLightHits = LightHits;
        }
        if ((int)(Variance * 1000) == (int)(MinVariance * 1000))
        {
            Tmax = grey_level;
            MeanLight = currMeanLight;
            BestLightHits = LightHits;
        }
    }

    global_threshold = (Tmin + Tmax) / 2;

    if (BestLightHits + BestDarkHits > 0)
        DarkRatio = BestDarkHits * 100 / (BestLightHits + BestDarkHits);

    delete[] histogram_squared_magnitude;

    return (global_threshold);
}

/*!
 * \brief calculate a global threshold based upon an intensity histogram
 * \param histogram histogram data
 * \param histogram_length length of the histogram data
 * \param histogram_start_index starting index for the histogram data within the array (there might be multiple histograms  a 1D array)
 * \param temp_histogram_buffer buffer used to store squared magnitudes
 * \param MeanDark returned mean dark value
 * \param MeanLight returned mean light value
 * \param DarkRatio ratio of dark pixels to light
 * \return global threshold value
 */
float thresholding::GetGlobalThreshold(
    int* histogram,
    int histogram_length,
    int histogram_start_index,
    int* temp_histogram_buffer,
    float& MeanDark,
    float& MeanLight,
    float& DarkRatio)
{
    float global_threshold = 0;

    float Tmin = 0;
    float Tmax = 0;
    float MinVariance = 999999;  // some large figure
    float currMeanDark, currMeanLight, VarianceDark, VarianceLight;

    float DarkHits = 0;
    float LightHits = 0;
    float BestDarkHits = 0;
    float BestLightHits = 0;
    MeanDark = 0;
    MeanLight = 0;

    // calculate squared magnitudes
    // this avoids unnecessary multiplications later on
    int* histogram_squared_magnitude = temp_histogram_buffer;
    int b = histogram_length - 1 + histogram_start_index;
    for (int i = histogram_length-1; i >= 0; i--, b--)
        histogram_squared_magnitude[i] = histogram[b] * histogram[b];

    int h;
    float magnitude_sqr, Variance, divisor;

    // evaluate all possible thresholds
    for (int grey_level = 255; grey_level >= 0; grey_level--)
    {
        // compute mean and variance for this threshold
        //  a struggle between light and darkness
        DarkHits = 0;
        LightHits = 0;
        currMeanDark = 0;
        currMeanLight = 0;
        VarianceDark = 0;
        VarianceLight = 0;

        for (h = histogram_length-1; h >= 0; h--)
        {
            magnitude_sqr = histogram[h];
            if (h < grey_level)
            {
                currMeanDark += h * magnitude_sqr;
                VarianceDark += (grey_level - h) * magnitude_sqr;
                DarkHits += magnitude_sqr;
            }
            else
            {
                currMeanLight += h * magnitude_sqr;
                VarianceLight += (grey_level - h) * magnitude_sqr;
                LightHits += magnitude_sqr;
            }
        }

        // compute means
        if (DarkHits > 0)
        {
            // rescale into 0-255 range
            currMeanDark = currMeanDark / DarkHits;
            VarianceDark = VarianceDark / DarkHits;
        }
        if (LightHits > 0)
        {
            // rescale into 0-255 range
            divisor = LightHits * histogram_length;
            currMeanLight = currMeanLight / LightHits;
            VarianceLight = VarianceLight / LightHits;
        }

        Variance = VarianceDark + VarianceLight;
        if (Variance < 0) Variance = -Variance;

        if (Variance < MinVariance)
        {
            MinVariance = Variance;
            Tmin = grey_level;
            MeanDark = currMeanDark;
            MeanLight = currMeanLight;
            BestDarkHits = DarkHits;
            BestLightHits = LightHits;
        }
        if ((int)(Variance * 1000) == (int)(MinVariance * 1000))
        {
            Tmax = grey_level;
            MeanLight = currMeanLight;
            BestLightHits = LightHits;
        }
    }

    global_threshold = (Tmin + Tmax) / 2;

    if (BestLightHits + BestDarkHits > 0)
        DarkRatio = BestDarkHits * 100 / (BestLightHits + BestDarkHits);

    //delete[] histogram_squared_magnitude;

    return (global_threshold);
}
