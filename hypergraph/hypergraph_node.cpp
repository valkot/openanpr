/*
    hypergraph node
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

#include "hypergraph_node.h"

// ********** constructors / destructors **********

/*!
 * \brief hypergraph_node
 * \param no_of_flags
 * \return
 */
hypergraph_node::hypergraph_node(
    int no_of_flags)
{
    for (int i = 0; i < no_of_flags; i++) Flags.push_back(false);
}


hypergraph_node::~hypergraph_node()
{
    for (int i = 0; i < (int)Links.size(); i++) delete Links[i];
}


// ********** public methods **********


/*!
 * \brief Add
 * \param link
 */
void hypergraph_node::Add(
    hypergraph_link *link)
{
    Links.push_back(link);
}

