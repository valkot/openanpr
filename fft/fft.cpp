#include "fft.h"

// ********** public methods **********


/*!
 * \brief one dimensional fast fourier transform
 * \param data waveform with values  the range 0.0 - 255.0
 * \param data_length number of elements  the waveform - must be a power of 2
 * \param forward forward or inverse transform
 * \param min_frequency minimum frequency
 * \param max_frequency maximum frequency
 * \return best responding frequency
 */
int FFT::fft_1D(
    double* data,
    int data_length,
    bool forward,
    int min_frequency,
    int max_frequency)
{
    int best_frequency = 0;
    double* data_dbl = new double[data_length * 2];
    int j = 0;
    for (int i = 0; i < data_length; i++)
    {
        data_dbl[j++] = data[i]; // real
        data_dbl[j++] = 0;       // imaginary
    }

    // transform
    fft_1D(data_dbl, data_length, forward);

    // find the best frequency
    j = min_frequency * 2;
    double max_response = DBL_MIN;
    for (int freq = min_frequency; freq < max_frequency; freq++, j += 2)
    {
        if (data_dbl[j] > max_response)
        {
            max_response = data_dbl[j];
            best_frequency = freq;
        }
    }

    delete[] data_dbl;

    return (best_frequency);
}

/*!
 * \brief test function
 */
int FFT::Test(
    int frequency)
{
    const int dimension = 256;

    // create a test waveform
    double* test_waveform = new double[dimension];

    for (int i = 0; i < dimension; i++)
    {
        float offset = i * ((float)PI * 2 * frequency) / (float)dimension;
        test_waveform[i] = (unsigned char)(255 * (cos(offset) + 1) / 2);
    }

    int best_frequency = fft_1D(test_waveform, dimension, true, 10, 48);
    delete[] test_waveform;
    //printf("Best frequency = %d\n", best_frequency);
    return(best_frequency);
}



// ********** private methods **********


/*!
 * \brief fft_1D
 * \param data
 * \param no_of_elements
 * \param forward
 */
void FFT::fft_1D(
    double* data,
    int no_of_elements,
    bool forward)
{
    int isign = 1;
    if (!forward) isign = -1;

    int n = no_of_elements << 1;
    int i, j, k, m;
    double t;

    // bit reversal section
    j = 0;
    for (i = 0; i < n; i += 2)
    {
	    if (j > i)
        {
            for (k = j; k <= j + 1; k++)
            {
                t = data[k];
                data[k] = data[k - j + i];
                data[k - j + i] = t;
            }
	    }
        m = no_of_elements;
	    while (m >= 2 && j >= m)
        {
	        j -= m;
	        m >>= 1;
	    }
	    j += m;
    }

    // Danielson-Lanczo theorem
    double theta, wr, wpr, wpi, wi, wtemp=0;
    double tempr, tempi, v;
    for (int mmax = 2; n > mmax;)
    {
        int istep = mmax << 1;
	    theta = isign * (2.0 * PI / mmax);
        v = sin(0.5 * theta);
	    wpr = -2.0 * v * v;
	    wpi = sin (theta);
	    wr = 1.0;
	    wi = 0.0;
	    for (m = 0; m < mmax; m += 2)
        {
	        for (i = m; i < n; i += istep)
            {
		        j = i + mmax;
		        tempr = wr * data[j] - wi * data[j + 1];
		        tempi = wr * data[j + 1] + wi * data[j];
		        data[j] = data[i] - tempr;
		        data[j + 1] = data[i + 1] - tempi;
		        data[i] += tempr;
		        data[i + 1] += tempi;
	        }
	        wtemp = wr;
	        wr = wtemp * wpr - wi * wpi + wr;
	        wi = wi * wpr + wtemp * wpi + wi;
	    }
	    mmax = istep;
    }
}
