/*
    image thresholding
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

#ifndef THRESHOLDING_H_
#define THRESHOLDING_H_

class thresholding
{
public:
    static float GetGlobalThreshold(float* histogram, int histogram_length, float& MeanDark, float& MeanLight, float& DarkRatio);
    static float GetGlobalThreshold(int* histogram, int histogram_length, int histogram_start_index, int* temp_histogram_buffer, float& MeanDark, float& MeanLight, float& DarkRatio);
};

#endif /*THRESHOLDING_H_*/
