#include "polygon.h"


// ********** public methods **********

/*!
 * \brief creates a circle shape
 */
polygon2D *polygon2D::CreateCircle(
    float centre_x,
    float centre_y,
    float radius,
    int circumference_steps)
{
    polygon2D *circle = new polygon2D();
    for (int i = 0; i < circumference_steps; i++)
    {
        float angle = i * (float)PI * 2 / circumference_steps;
        float x = centre_x + (radius * (float)sin(angle));
        float y = centre_y + (radius * (float)cos(angle));
        circle->Add(x, y);
    }
    return(circle);
}

/*!
 * \brief returns a copy of the polygon
 * \return
 */
polygon2D *polygon2D::Copy()
{
    polygon2D *new_poly = new polygon2D();
    new_poly->name = name;
    new_poly->type = type;
    new_poly->occupied = occupied;

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        float x = x_points[i];
        float y = y_points[i];
        new_poly->Add(x, y);
    }
    return (new_poly);
}


/*!
 * \brief return a scaled version of the polygon
 * \param factor factor by which this polygon will be scaled
 * \param scaled scaled polygon to be output
 */
void polygon2D::Scale(
    float factor,
    polygon2D *scaled)
{
    scaled->Clear();

    float centre_x = 0, centre_y = 0;
    getCentreOfGravity(centre_x, centre_y);

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        float dx = x_points[i] - centre_x;
        float dy = y_points[i] - centre_y;
        float x = (float)(centre_x + (dx * factor));
        float y = (float)(centre_y + (dy * factor));
        scaled->Add(x, y);
    }
}

/*!
 * \brief scale the polygon to a new image size
 * \param original_image_width
 * \param original_image_height
 * \param new_image_width
 * \param new_image_height
 * \return
 */
polygon2D *polygon2D::Scale(
    int original_image_width,
    int original_image_height,
    int new_image_width,
    int new_image_height)
{
    polygon2D *rescaled = new polygon2D();

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        float x = x_points[i] * new_image_width / original_image_width;
        float y = y_points[i] * new_image_height / original_image_height;
        rescaled->Add(x, y);
    }
    return (rescaled);
}

/*!
 * \brief adds the given rotation angle to the orientation of the polygon #include the given centre point
 * \param rotation rotation to be added  radians
 * \param centre_x centre about which to rotate
 * \param centre_y centre about which to rotate
 */
void polygon2D::rotate(
    float rotation,
    float centre_x,
    float centre_y)
{
    for (int i = 0; i < (int)x_points.size(); i++)
    {
        float dx = x_points[i] - centre_x;
        float dy = y_points[i] - centre_y;
        float hyp = (float)sqrt((dx * dx) + (dy * dy));
        if (hyp > 0)
        {
            float angle = (float)acos(dy / hyp);
            if (dx < 0) angle = ((float)PI * 2) - angle;
            angle += (float)(PI * 3 / 2);

            angle += rotation;
            x_points[i] = centre_x + (hyp * (float)sin(angle));
            y_points[i] = centre_y + (hyp * (float)cos(angle));
        }
    }
}




/*!
 * \brief returns the centre of gravity
 * \param centre_x
 * \param centre_y
 */
void polygon2D::getCentreOfGravity(
    float& centre_x,
    float& centre_y)
{
    float x = 0, y = 0;
    for (int i = 0; i < (int)x_points.size(); i++)
    {
        x += x_points[i];
        y += y_points[i];
    }
    if (x_points.size() > 0)
    {
        x /= x_points.size();
        y /= y_points.size();
    }
    centre_x = x;
    centre_y = y;
}


/*!
 * \brief return the perimeter length of the polygon
 * \return
 */
float polygon2D::getPerimeterLength()
{
    float x0, y0, x1, y1;
    float perimeter = 0;
    int no_of_points = (int)x_points.size();

    for (int i = 0; i < no_of_points; i++)
    {
        int index1 = i;
        x0 = x_points[index1];
        y0 = y_points[index1];

        int index2 = i + 1;
        if (index2 >= no_of_points)
            index2 -= no_of_points;

        x1 = x_points[index2];
        y1 = y_points[index2];
        float dx = x1 - x0;
        float dy = y1 - y0;
        perimeter += (float)sqrt((dx * dx) + (dy * dy));
    }
    return (perimeter);
}

/*!
 * \brief return the shortest side of the polygon
 * \return
 */
float polygon2D::getShortestSide()
{
    float x0, y0, x1, y1;
    float shortest = FLT_MAX;

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        int index1 = i;
        x0 = x_points[index1];
        y0 = y_points[index1];

        int index2 = i + 1;
        if (index2 >= (int)x_points.size())
            index2 -= (int)x_points.size();

        x1 = x_points[index2];
        y1 = y_points[index2];
        float dx = x1 - x0;
        float dy = y1 - y0;
        float side_length = (float)sqrt((dx * dx) + (dy * dy));
        if (side_length < shortest)
            shortest = side_length;
    }
    if (shortest == FLT_MAX) shortest = 0;

    return (shortest);
}

/*!
 * \brief returns orientations of each side of the polygon
 * \return
 */
float* polygon2D::getOrientations()
{
    float* orientations = new float[4];

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        float x0, y0, x1, y1;

        if (i < (int)x_points.size() - 1)
        {
            x0 = x_points[i];
            y0 = y_points[i];
            x1 = x_points[i + 1];
            y1 = y_points[i + 1];
        }
        else
        {
            x0 = x_points[i];
            y0 = y_points[i];
            x1 = x_points[0];
            y1 = y_points[0];
        }

        float dx = x1 - x0;
        float dy = y1 - y0;
        float hyp = (float)sqrt((dx * dx) + (dy * dy));
        orientations[i] = (float)acos(dx / hyp);
        if (dy < 0) orientations[i] = ((float)PI * 2) - orientations[i];
    }
    return (orientations);
}

/*!
 * \brief returns gradients of each edge of the polygon
 * \return
 */
float* polygon2D::getGradients()
{
    float* gradients = new float[4];

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        float x0, y0, x1, y1;

        if (i < (int)x_points.size() - 1)
        {
            x0 = x_points[i];
            y0 = y_points[i];
            x1 = x_points[i + 1];
            y1 = y_points[i + 1];
        }
        else
        {
            x0 = x_points[i];
            y0 = y_points[i];
            x1 = x_points[0];
            y1 = y_points[0];
        }

        float dx = x1 - x0;
        float dy = y1 - y0;
        if (dx != 0) gradients[i] = dy / dx;
    }
    return (gradients);
}

/*!
 * \brief return the longest side of the polygon
 * \return
 */
float polygon2D::getLongestSide()
{
    float x0, y0, x1, y1;
    float longest = FLT_MIN;

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        int index1 = i;
        x0 = x_points[index1];
        y0 = y_points[index1];

        int index2 = i + 1;
        if (index2 >= (int)x_points.size())
            index2 -= (int)x_points.size();

        x1 = x_points[index2];
        y1 = y_points[index2];
        float dx = x1 - x0;
        float dy = y1 - y0;
        float side_length = (float)sqrt((dx * dx) + (dy * dy));
        if (side_length > longest)
            longest = side_length;
    }
    if (longest == FLT_MIN) longest = 0;

    return (longest);
}

/*!
 * \brief returns the length of one side of the polygon
 * \param index index of the side
 * \return
 */
float polygon2D::getSideLength(
    int index)
{
    int index1 = index;
    if (index1 >= (int)x_points.size())
        index1 -= x_points.size();

    float x0 = x_points[index1];
    float y0 = y_points[index1];

    int index2 = index + 1;
    if (index2 >= (int)x_points.size())
        index2 -= x_points.size();

    float x1 = x_points[index2];
    float y1 = y_points[index2];
    float dx = x1 - x0;
    float dy = y1 - y0;
    return((float)sqrt((dx * dx) + (dy * dy)));
}

/*!
 * \brief returns start and end positions for a side
 * \param index index of the side
 * \param tx
 * \param ty
 * \param bx
 * \param by
 */
void polygon2D::getSidePositions(
    int index,
    float& tx,
    float& ty,
    float& bx,
    float& by)
        {
    int index1 = index;
    if (index1 >= (int)x_points.size())
        index1 -= (int)x_points.size();

    tx = x_points[index1];
    ty = y_points[index1];

    int index2 = index + 1;
    if (index2 >= (int)x_points.size())
        index2 -= (int)x_points.size();

    bx = x_points[index2];
    by = y_points[index2];
}

/*!
 * \brief returns teh interior angle subtended at the given vertex
 */
float polygon2D::GetInteriorAngle(
    int vertex)
{
    float angle = 0;
    //if (x_points != NULL)
    {
        int previous_vertex = vertex - 1;
        if (previous_vertex < 0) previous_vertex += (int)x_points.size();

        int next_vertex = vertex + 1;
        if (next_vertex >= (int)x_points.size()) next_vertex -= (int)x_points.size();

        angle = geometry::threePointAngle(x_points[previous_vertex], y_points[previous_vertex],
                                          x_points[vertex], y_points[vertex],
                                          x_points[next_vertex], y_points[next_vertex]);

        // ensure that this  an angle less than 180 degrees
        if (angle < 0) angle = -angle;
        if (angle > PI)
            angle = (2 * (float)PI) - angle;

        //float angle_degrees = angle / (float)PI * 180.0f;
        //printf("angle = " + angle_degrees.ToString());
    }
    return(angle);
}

/*!
 * \brief returns the maximum difference from perfectly square corners
 */
float polygon2D::GetMaxDifferenceFromSquare()
{
    float max_diff_angle = 0;

    //if (x_points != NULL)
    {
        //  this a square?
        if (x_points.size() == 4)
        {
            float square_angle = (float)PI / 2.0f;

            // get the angles for each vertex
            for (int vertex = 0; vertex < 4; vertex++)
            {
                float angle_diff = GetInteriorAngle(vertex) - square_angle;
                if (angle_diff < 0) angle_diff = -angle_diff;
                if (angle_diff > max_diff_angle) max_diff_angle = angle_diff;
            }
        }
    }

    return(max_diff_angle);
}

/*!
 * \brief return the orientation of the longest side
 * \return orientation  radians
 */
float polygon2D::getLongestSideOrientation()
{
    float x0, y0, x1, y1;
    float longest_dx=0, longest_dy=0;
    float longest = 0;
    float orientation = 0;

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        int index1 = i;
        x0 = x_points[index1];
        y0 = y_points[index1];

        int index2 = i + 1;
        if (index2 >= (int)x_points.size())
            index2 -= (int)x_points.size();

        x1 = x_points[index2];
        y1 = y_points[index2];
        float dx = x1 - x0;
        float dy = y1 - y0;
        float side_length = (float)sqrt((dx * dx) + (dy * dy));
        if ((longest == 0) ||
            ((side_length > longest) && (side_length > 1)))
        {
            longest = side_length;
            longest_dx = dx;
            longest_dy = dy;
        }
    }

    if (longest_dx != 0)
    {
        orientation = (float)acos(longest_dx / longest);
        if (longest_dy < 0) orientation = ((float)PI * 2) - orientation;
        if (orientation < 0) orientation += ((float)PI*2);
        if (orientation > (float)PI) orientation -= (float)PI;
    }

    return (orientation);
}

/*!
 * \brief returns the centre point of a square shaped polygon by finding the intersection of the two diagonals
 * \param centre_x x centre of the square
 * \param centre_y y centre of the square
 */
void polygon2D::GetSquareCentre(
    float& centre_x,
    float& centre_y)
{
    if (x_points.size() == 4)
    {
        geometry::intersection(x_points[0], y_points[0],
                               x_points[2], y_points[2],
                               x_points[1], y_points[1],
                               x_points[3], y_points[3],
                               centre_x, centre_y);
    }
}

/*!
 * \brief return a value indicating how even the aspect ratio of the sides are with 1.0 being a perfect square
 * \return
 */
float polygon2D::getSquareness()
{
    float squareness = -1;

	if (x_points.size() == 4)
	{
		float x0, y0, x1, y1;
		float* side_length = new float[4];
		float perimeter = 0;

		for (int i = 0; i < (int)x_points.size(); i++)
		{
			int index1 = i;
			x0 = x_points[index1];
			y0 = y_points[index1];

			int index2 = i + 1;
			if (index2 >= (int)x_points.size())
				index2 -= (int)x_points.size();

			x1 = x_points[index2];
			y1 = y_points[index2];
			float dx = x1 - x0;
			float dy = y1 - y0;
			side_length[i] = (float)sqrt((dx * dx) + (dy * dy));
			perimeter += side_length[i];
		}

		if (perimeter > 0)
		{
			float average_side_length = perimeter / 4;
			squareness = 0;
			for (int i = 0; i < 4; i++)
			{
				squareness += ABS(side_length[i] - average_side_length);
			}
			if (squareness == 0)
				squareness = -1;
			else
				squareness = 1.0f / (1.0f + (squareness / perimeter));
		}

		delete[] side_length;
	}
    return (squareness);
}

/*!
 * \brief returns an orientation angle for a square polygon by looking at the longest vertical line
 * \return orientation of the polygon
 */
float polygon2D::GetSquareOrientation()
{
    float orientation = 0;

    //if (x_points != NULL)
    {
        float max_vertical = 0;
        for (int i = 0; i < (int)x_points.size(); i++)
        {
            float x0 = x_points[i];
            float y0 = y_points[i];
            float x1, y1;
            if (i < (int)x_points.size() - 1)
            {
                x1 = x_points[i + 1];
                y1 = y_points[i + 1];
            }
            else
            {
                x1 = x_points[0];
                y1 = y_points[0];
            }

            // keep the orientation consistent
            if (y1 < y0)
            {
                float temp = y1;
                y1 = y0;
                y0 = temp;
                temp = x1;
                x1 = x0;
                x0 = temp;
            }

            float dy = y1 - y0;
            float dx = x1 - x0;

            // vertically oriented lines
            if (ABS(dy) > ABS(dx))
            {
                float vertical = ABS(dy);
                if (vertical > max_vertical)
                {
                    max_vertical = vertical;

                    float length = (float)sqrt((dx * dx) + (dy * dy));
                    orientation = (float)acos(dy / length);
                    if (dx < 0) orientation = (float)(PI * 2) - orientation;
                }
            }
        }
    }
    return (orientation);
}


/*!
 * \brief returns an horizontal width of the square
 * \return horizontal width  pixels
 */
float polygon2D::GetSquareHorizontal()
{
    float length = 0;

    //if (x_points != NULL)
    {
        float max_horizontal = 0;
        for (int i = 0; i < (int)x_points.size(); i++)
        {
            float x0 = x_points[i];
            float y0 = y_points[i];
            float x1, y1;
            if (i < (int)x_points.size() - 1)
            {
                x1 = x_points[i + 1];
                y1 = y_points[i + 1];
            }
            else
            {
                x1 = x_points[0];
                y1 = y_points[0];
            }

            float dy = y1 - y0;
            float dx = x1 - x0;

            // horizontally oriented lines
            if (ABS(dx) > ABS(dy))
            {
                float horizontal = ABS(dx);
                if (horizontal > max_horizontal)
                {
                    max_horizontal = horizontal;

                    length = (float)sqrt((dx * dx) + (dy * dy));
                }
            }
        }
    }
    return (length);
}

/*!
 * \brief returns an vertical height of the square
 * \return vertical height  pixels
 */
float polygon2D::GetSquareVertical()
{
    float length = 0;

    //if (x_points != NULL)
    {
        float max_vertical = 0;
        for (int i = 0; i < (int)x_points.size(); i++)
        {
            float x0 = x_points[i];
            float y0 = y_points[i];
            float x1, y1;
            if (i < (int)x_points.size() - 1)
            {
                x1 = x_points[i + 1];
                y1 = y_points[i + 1];
            }
            else
            {
                x1 = x_points[0];
                y1 = y_points[0];
            }

            float dy = y1 - y0;
            float dx = x1 - x0;

            // vertically oriented lines
            if (ABS(dy) > ABS(dx))
            {
                float vertical = ABS(dy);
                if (vertical > max_vertical)
                {
                    max_vertical = vertical;

                    length = (float)sqrt((dx * dx) + (dy * dy));
                }
            }
        }
    }
    return (length);
}

/*!
 * \brief returns the bounding box of the polygon
 */
void polygon2D::BoundingBox(
    float& tx,
    float& ty,
    float& bx,
    float& by)
{
    float x,y;
    tx = FLT_MAX;
    bx = FLT_MIN;
    ty = FLT_MAX;
    by = FLT_MIN;

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        x = x_points[i];
        if (x < tx) tx = x;
        if (x > bx) bx = x;

        y = y_points[i];
        if (y < ty) ty = y;
        if (y > by) by = y;
    }
}

/*!
 * \brief returns the leftmost coordinate of the polygon
 * \return
 */
float polygon2D::left()
{
    float min_x = 0;
    float x;

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        x = x_points[i];
        if ((min_x == 0) || (x < min_x)) min_x = x;
    }
    return (min_x);
}

/*!
 * \brief returns the rightmost coordinate of the polygon
 * \return
 */
float polygon2D::right()
{
    float max_x = 0;
    float x;

    for (int i = 0; i < (int)x_points.size(); i++)
    {
        x = x_points[i];
        if ((max_x == 0) || (x > max_x)) max_x = x;
    }
    return (max_x);
}

/*!
 * \brief returns the top coordinate of the polygon
 * \return
 */
float polygon2D::top()
{
    float min_y = 0;
    float v;

    for (int i = 0; i < (int)y_points.size(); i++)
    {
        v = y_points[i];
        if ((min_y == 0) || (v < min_y)) min_y = v;
    }
    return (min_y);
}

/*!
 * \brief returns the bottom coordinate of the polygon
 * \return
 */
float polygon2D::bottom()
{
    float max_y = 0;
    float v;

    for (int i = 0; i < (int)y_points.size(); i++)
    {
        v = y_points[i];
        if ((max_y == 0) || (v > max_y)) max_y = v;
    }
    return (max_y);
}

/*!
 * \brief clear all points
 */
void polygon2D::Clear()
{
    x_points.clear();
    y_points.clear();
}

/*!
 * \brief add a new point
 * \param x x coordinate
 * \param y y coordinate
 */
void polygon2D::Add(
    float x,
    float y)
{
    x_points.push_back(x);
    y_points.push_back(y);
}

/*!
 * \brief add a new point
 * \param index position at which to insert the data
 * \param x x coordinate
 * \param y y coordinate
 */
void polygon2D::Add(
    int index,
    float x,
    float y)
{
    //if (x_points == NULL)
    {
        //vector<float> x_points; // TODO check this x_points = new vector<float>();
        x_points.clear();
        y_points.clear(); // TODO check this: y_points = new vector<float>();
    }

    // add any entries  necessary
    for (int i = x_points.size(); i <= index; i++)
    {
        x_points.push_back(0.0f);
        y_points.push_back(0.0f);
    }

    x_points[index] = x;
    y_points[index] = y;
}

/*!
 * \brief remove the last point  the list
 */
void polygon2D::Remove()
{
    //if (x_points != NULL)
    {
        int i = x_points.size()-1;
        if (i >= 0)
        {
            x_points.erase(x_points.begin() + i);
            y_points.erase(y_points.begin() + i);
        }
    }
}

/*!
 * \brief mirror the polygon with respect to the given image size
 * \param image_width width of the image within which the polygon  contained
 * \param image_height height of the image within which the polygon  contained
 */
void polygon2D::Mirror(
    int image_width,
    int image_height)
{
    //if (x_points != NULL)
    {
        for (int i = 0; i < (int)x_points.size(); i++)
            x_points[i] = image_width - 1 - x_points[i];
    }
}

/*!
 * \brief flip the polygon with respect to the given image size
 * \param image_width width of the image within which the polygon  contained
 * \param image_height height of the image within which the polygon  contained
 */
void polygon2D::Flip(
    int image_width,
    int image_height)
{
    //if (y_points != NULL)
    {
        for (int i = 0; i < (int)y_points.size(); i++)
            y_points[i] = image_height - 1 - y_points[i];
    }
}

/*!
 * \brief the given x,y point inside the polygon?
 * \param x
 * \param y
 * \return
 */
bool polygon2D::isInside(
    float x,
    float y)
{
    int i, j;
    bool c = false;
    for (i = 0, j = (int)x_points.size() - 1; i < (int)x_points.size(); j = i++)
    {
        if ((((y_points[i] <= y) && (y < y_points[j])) ||
           ((y_points[j] <= y) && (y < y_points[i]))) &&
          (x < (x_points[j] - x_points[i]) * (y - y_points[i]) / (y_points[j] - y_points[i]) + x_points[i]))
            c = !c;
    }
    return (c);
}

/*!
 * \brief return TRUE if this polygon overlaps with another
 * \param other
 * \return
 */
bool polygon2D::overlaps(
    polygon2D *other)
{
    int i;
    bool retval = false;

    i = 0;
    while ((i < (int)x_points.size()) && (retval == false))
    {
        if (other->isInside(x_points[i],y_points[i])) retval=true;
        i++;
    }

    i = 0;
    while ((i < (int)other->x_points.size()) && (retval == false))
    {
        if (isInside((float)other->x_points[i], (float)other->y_points[i])) retval = true;
        i++;
    }
    return (retval);
}

/*!
 * \brief does this polygon overlap with the other, within the given screen dimensions
 * \param other other polygon object
 * \param image_width image width
 * \param image_height image height
 * \return
 */
bool polygon2D::overlaps(
    polygon2D *other,
    int image_width,
    int image_height)
{
    int i;
    bool retval = false;

    i = 0;
    while ((i < (int)x_points.size()) && (retval == false))
    {
        if (other->isInside(x_points[i] * 1000 / image_width, y_points[i] * 1000 / image_height)) retval = true;
        i++;
    }

    i = 0;
    while ((i < (int)other->x_points.size()) && (retval == false))
    {
        if (isInside((float)other->x_points[i] * image_width / 1000, (float)other->y_points[i] * image_height / 1000)) retval = true;
        i++;
    }
    return (retval);
}

/*!
 * \brief returns the difference  the positions of vertices compared to another polygon
 * \param other the other polygon
 * \return
 */
float polygon2D::Compare(
    polygon2D *other)
{
    float difference = 0;

    // same number of vertices
    if (x_points.size() == other->x_points.size())
    {
        for (int i = 0; i < (int)x_points.size(); i++)
        {
            float dx = ABS(x_points[i] - other->x_points[i]);
            float dy = ABS(y_points[i] - other->y_points[i]);
            difference += dx + dy;
        }
    }

    return (difference);
}

/*!
 * \brief snaps the vertices of the polygon to a grid having the given spacing
 * \param grid_spacing spacing of the grid
 * \return fitted polygon
 */
polygon2D *polygon2D::SnapToGrid(
    float grid_spacing)
{
    polygon2D *result = new polygon2D();
    for (int i = 0; i < (int)x_points.size(); i++)
    {
        float x = x_points[i] / grid_spacing;
        x = (float)round(x) * grid_spacing;
        float y = y_points[i] / grid_spacing;
        y = (float)round(y) * grid_spacing;
        result->Add(x, y);
    }
    return (result);
}

/*!
 * \brief approximately calculates the area of a polygon by snapping the vertices to a grid, then applying pick's theorem to find the area
 * \param grid_spacing spacing of the grid which vertices will be snapped to
 * \return area of the polygon
 */
float polygon2D::ApproximateArea(
    float grid_spacing)
{
    float area = 0;
    float tollerance = 0.01f;

    if (x_points.size() > 2)
    {
        polygon2D *fit_to_grid = SnapToGrid(grid_spacing);
        float tx = fit_to_grid->left();
        float ty = fit_to_grid->top();
        float bx = fit_to_grid->right();
        float by = fit_to_grid->bottom();

        int points_interior = 0;
        for (float y = ty; y <= by; y += grid_spacing)
        {
            for (float x = tx; x <= bx; x += grid_spacing)
            {
                if (fit_to_grid->isInside(x, y))
                    points_interior++;
            }
        }

        int points_boundary = 0;
        for (int vertex = 0; vertex < (int)fit_to_grid->x_points.size() - 1; vertex++)
        {
            float dx = fit_to_grid->x_points[vertex + 1] - fit_to_grid->x_points[vertex];
            float dy = fit_to_grid->y_points[vertex + 1] - fit_to_grid->y_points[vertex];
            float length = (float)sqrt((dx * dx) + (dy * dy));
            int steps = (int)(length / grid_spacing);
            float start_x = fit_to_grid->x_points[vertex];
            float start_y = fit_to_grid->y_points[vertex];
            for (int i = 0; i < steps; i++)
            {
                float x = start_x + (i * dx / steps);
                float x_snapped = x / grid_spacing;
                x_snapped = (float)round(x_snapped) * grid_spacing;

                float dx2 = x_snapped - x;
                if ((dx2 > -tollerance) && (dx2 < tollerance))
                {
                    float y = start_y + (i * dy / steps);
                    float y_snapped = y / grid_spacing;
                    y_snapped = (float)round(y_snapped) * grid_spacing;
                    float dy2 = y_snapped - y;
                    if ((dy2 > -tollerance) && (dx2 < tollerance))
                        points_boundary++;
                }
            }
        }

        // calculate area by pick's theorem
        area = points_interior + (points_boundary / 2) - 1;
        area *= grid_spacing;
    }

    return area;
}

/*! \brief draw the polygon within the given image
 *  \param img image
 *  \param r red
 *  \param g green
 *  \param b blue
 *  \param lineWidth line width
 */
void polygon2D::show(unsigned char* img, int image_width, int image_height, int r, int g, int b, int lineWidth)
{
    show(img, image_width, image_height, r, g, b, lineWidth, 0, 0);
}

/*! \brief draw the polygon within the given image
 *  \param img image
 *  \param r red
 *  \param g green
 *  \param b blue
 *  \param lineWidth line width
 *  \param x_offset
 *  \param y_offset
 */
void polygon2D::show(unsigned char* img, int image_width, int image_height, int r, int g, int b, int lineWidth, int x_offset, int y_offset)
{
    for (int i = 1; i < (int)x_points.size(); i++)
    {
        drawing::drawLine(img, image_width, image_height, (int)(x_points[i - 1]+x_offset), (int)(y_points[i - 1]+y_offset), (int)(x_points[i]+x_offset), (int)(y_points[i]+y_offset), r, g, b, lineWidth, false);
        if (i == (int)x_points.size() - 1)
            drawing::drawLine(img, image_width, image_height, (int)(x_points[0]+x_offset), (int)(y_points[0]+y_offset), (int)(x_points[i]+x_offset), (int)(y_points[i]+y_offset), r, g, b, lineWidth, false);
    }
}

