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
