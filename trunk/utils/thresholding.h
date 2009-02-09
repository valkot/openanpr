#ifndef THRESHOLDING_H_
#define THRESHOLDING_H_

class thresholding
{
public:
    static float GetGlobalThreshold(float* histogram, int histogram_length, float& MeanDark, float& MeanLight, float& DarkRatio);
    static float GetGlobalThreshold(int* histogram, int histogram_length, int histogram_start_index, int* temp_histogram_buffer, float& MeanDark, float& MeanLight, float& DarkRatio);
};

#endif /*THRESHOLDING_H_*/
