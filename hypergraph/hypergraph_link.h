#ifndef hypergraph_link_h
#define hypergraph_link_h

#include "hypergraph_node.h"

class hypergraph_node;

class hypergraph_link
{
    public:
        hypergraph_node *From;
        hypergraph_node *To;
        
        ~hypergraph_link() {}
};

#endif
