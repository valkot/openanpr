/*
    computational geometry
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

#include "geometry.h"


// ********** public methods **********


/*!
 * \brief returns the angle subtended by three points
 * \param x0 line first point x coordinate
 * \param y0 line first point y coordinate
 * \param x1 line second point x coordinate
 * \param y1 line second point y coordinate
 * \param x2 line third point x coordinate
 * \param y2 line third point y coordinate
 * \return angle  radians
 */
float geometry::threePointAngle(
    float x0,
    float y0,
    float x1,
    float y1,
    float x2,
    float y2)
{
    float pt1 = x0 - x1;
    float pt2 = y0 - y1;
    float pt3 = x2 - x1;
    float pt4 = y2 - y1;

    float angle = ((pt1 * pt3) + (pt2 * pt4)) /
                   (((float)sqrt((pt1*pt1) + (pt2*pt2))) *
                    ((float)sqrt((pt3*pt3) + (pt4*pt4))));

    angle = (float)acos(angle);
    return(angle);
}

/*!
 * \brief returns the perpendicular distance of a line from the centre of a circle
 * \param x0 line first point x coordinate
 * \param y0 line first point y coordinate
 * \param x1 line second point x coordinate
 * \param y1 line second point y coordinate
 * \param circle_x circle centre x coordinate
 * \param circle_y circle centre y coordinate
 * \param circle_radius circle radius
 * \return
 */
float geometry::circleDistanceFromLine(
    float x0,
    float y0,
    float x1,
    float y1,
    float circle_x,
    float circle_y,
    float circle_radius)
{
    float perpendicular_dist = 999999;

    float dx = x1 - x0;
    float dy = y1 - y0;
    float line_length = (float)sqrt((dx * dx) + (dy * dy));

    if (line_length > 0)
    {
        // perpendicular line, spanning the circle
        float perp_line_x0 = circle_x - dy;
        float perp_line_y0 = circle_y - dx;
        float perp_line_x1 = circle_x + dy;
        float perp_line_y1 = circle_y + dx;

        float ix = GEOM_INVALID;
        float iy = 0;
        intersection(x0, y0, x1, y1,
                     perp_line_x0, perp_line_y0, perp_line_x1, perp_line_y1,
                     ix, iy);

        if (ix != GEOM_INVALID)
        {
            dx = ix - circle_x;
            dy = iy - circle_y;
            perpendicular_dist = (float)sqrt((dx * dx) + (dy * dy));
        }
    }

    return (perpendicular_dist);
}

/*!
 * \brief returns the perpendicular distance of a point from a line
 * \param x0 line first point x coordinate
 * \param y0 line first point y coordinate
 * \param x1 line second point x coordinate
 * \param y1 line second point y coordinate
 * \param point_x point x coordinate
 * \param point_y point y coordinate
 * \param line_intersection_x returned perpendicular x intersection point on the line
 * \param line_intersection_y returned perpendicular y intersection point on the line
 * \return distance of the point from the line
 */
float geometry::pointDistanceFromLine(
    float x0,
    float y0,
    float x1,
    float y1,
    float point_x,
    float point_y,
    float& line_intersection_x,
    float& line_intersection_y)
{
    float perpendicular_dist = 999999;

    float dx = x1 - x0;
    float dy = y1 - y0;
    float line_length = (float)sqrt((dx * dx) + (dy * dy));

    if (line_length > 0)
    {
        // perpendicular line, spanning the circle
        float perp_line_x0 = point_x - dy;
        float perp_line_y0 = point_y - dx;
        float perp_line_x1 = point_x + dy;
        float perp_line_y1 = point_y + dx;

        float ix = GEOM_INVALID;
        float iy = 0;
        intersection(x0, y0, x1, y1,
                     perp_line_x0, perp_line_y0, perp_line_x1, perp_line_y1,
                     ix, iy);

        if (ix != GEOM_INVALID)
        {
            line_intersection_x = ix;
            line_intersection_y = iy;
            dx = ix - point_x;
            dy = iy - point_y;
            perpendicular_dist = (float)sqrt((dx * dx) + (dy * dy));
        }
    }

    return (perpendicular_dist);
}


/*!
 * \brief does the line intersect with the given line?
 * \param x0 first line top x
 * \param y0 first line top y
 * \param x1 first line bottom x
 * \param y1 first line bottom y
 * \param x2 second line top x
 * \param y2 second line top y
 * \param x3 second line bottom x
 * \param y3 second line bottom y
 * \param xi intersection x coordinate
 * \param yi intersection y coordinate
 * \return true if the lines intersect
 */
bool geometry::intersection(
    float x0,
    float y0,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float& xi,
    float& yi)
{
    float a1, b1, c1,         //constants of linear equations
          a2, b2, c2,
          det_inv,            //the inverse of the determinant of the coefficient
          m1, m2, dm;         //the gradients of each line
    bool insideLine = false;  //is the intersection along the lines given, or outside them
    float tx, ty, bx, by;

    //compute gradients, note the cludge for infinity, however, this will
    //be close enough
    if ((x1 - x0) != 0)
        m1 = (y1 - y0) / (x1 - x0);
    else
        m1 = (float)1e+10;   //close, but no cigar

    if ((x3 - x2) != 0)
        m2 = (y3 - y2) / (x3 - x2);
    else
        m2 = (float)1e+10;   //close, but no cigar

    dm = (float)ABS(m1 - m2);
    if (dm > 0.000001f)
    {
        //compute constants
        a1 = m1;
        a2 = m2;

        b1 = -1;
        b2 = -1;

        c1 = (y0 - m1 * x0);
        c2 = (y2 - m2 * x2);

        //compute the inverse of the determinate
        det_inv = 1 / (a1 * b2 - a2 * b1);

        //use Kramers rule to compute xi and yi
        xi = ((b1 * c2 - b2 * c1) * det_inv);
        yi = ((a2 * c1 - a1 * c2) * det_inv);

        //is the intersection inside the line or outside it?
        if (x0 < x1) { tx = x0; bx = x1; } else { tx = x1; bx = x0; }
        if (y0 < y1) { ty = y0; by = y1; } else { ty = y1; by = y0; }
        if ((xi >= tx) && (xi <= bx) && (yi >= ty) && (yi <= by))
        {
            if (x2 < x3) { tx = x2; bx = x3; } else { tx = x3; bx = x2; }
            if (y2 < y3) { ty = y2; by = y3; } else { ty = y3; by = y2; }
            if ((xi >= tx) && (xi <= bx) && (yi >= ty) && (yi <= by))
            {
                insideLine = true;
            }
        }
    }
    else
    {
        //parallel (or parallelish) lines, return some indicative value
        xi = GEOM_INVALID;
    }

    return (insideLine);
}

/*!
 * \brief returns the signed perpendicular distance of a point from a line
 * \param x0 line first point x coordinate
 * \param y0 line first point y coordinate
 * \param x1 line second point x coordinate
 * \param y1 line second point y coordinate
 * \param point_x point x coordinate
 * \param point_y point y coordinate
 * \return signed distance of the point from the line
 */
float geometry::pointDistanceFromLine(
    float x0,
    float y0,
    float x1,
    float y1,
    float point_x,
    float point_y)
{
    float perpendicular_dist = 0;

    // compute the dot product AB . BC
    float* AB = new float[2];
    float* BC = new float[2];
    AB[0] = x1 - x0;
    AB[1] = y1 - x1;
    BC[0] = point_x - x1;
    BC[1] = point_y - y1;
    //float dot = AB[0] * BC[0] + AB[1] * BC[1];

    // compute the cross product AB x AC
    float* AC = new float[2];
    AC[0] = point_x - x0;
    AC[1] = point_y - y0;
    float cross = AB[0] * AC[1] - AB[1] * AC[0];

    // compute the distance from A to B
    float d1 = x0 - x1;
    float d2 = y0 - y1;
    float distance = (float)sqrt(d1*d1 + d2*d2);

    //Compute the distance from AB to C
    if (distance > 0)
        perpendicular_dist = cross / distance;

    delete[] AB;
    delete[] BC;
    delete[] AC;

    return (perpendicular_dist);
}

