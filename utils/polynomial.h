#ifndef polynomial_h
#define polynomial_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "drawing.h"

#ifndef ABS
    #define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

class polynomial
{
    public:

        polynomial();
        ~polynomial();

        void Solve();
        void Init();
        void SetCoeff(int Exponent, float value);
        float Coeff(int Exponent);
        int GetDegree();
        void SetDegree(int NewVal);
        int XYCount();
        void AddPoint(float x, float y);
        float RegVal(float x);
        float GetRMSerror();
        void Show(unsigned char *img, int width, int height);


    private:
        int MaxO;            // max polynomial degree
        int GlobalO;         // degree of the polynomial expected
        bool Finished;
        float* SumX;
        float* SumYX;
        float** M;
        float* C; // coefficients
        std::vector<float> Xpoints;
        std::vector<float> Ypoints;

        void GaussSolve(int O);
        void BuildMatrix(int O);
        void FinalizeMatrix(int O);


};

#endif
