/*
    ANPR
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

#ifndef ANPR_H_
#define ANPR_H_

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

#include <math.h>
#include <string>
#include <cstdlib>
#include <vector>
#include "../common.h"
#include "../utils/Image.h"
#include "../utils/polygon.h"
#include "../shapes/shapes.h"
#include "platedetection.h"
#include "platereader.h"

class anpr {
public:
	static void Read(
	    unsigned char* img_colour,
	    int img_width,
	    int img_height,
	    std::vector<polygon2D*> &plates,
	    std::vector<std::string> &numbers);
};

#endif /* ANPR_H_ */
