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


#ifndef hypergraph_node_h
#define hypergraph_node_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include "hypergraph_link.h"

class hypergraph_link;

class hypergraph_node
{
    public:
        std::string Name;
        int ID;
        int depth;
        std::vector<hypergraph_link *> Links;
        std::vector<bool> Flags;
        void Add(hypergraph_link *link);

        hypergraph_node(int no_of_flags);
        ~hypergraph_node();
};

#endif
