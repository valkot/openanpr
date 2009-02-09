#ifndef hypergraph_h
#define hypergraph_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string.h>
#include "hypergraph_link.h"
#include "hypergraph_node.h"

class hypergraph
{
    public:
        std::vector<hypergraph_node*> Nodes;

        hypergraph();
        hypergraph(int no_of_nodes, int no_of_flags);
        ~hypergraph();

        void Add(hypergraph_node *node);
        void Remove(hypergraph_node *node);
        void Remove(int ID);
        void Remove(std::string name);
        int IndexOf(int ID);
        int IndexOf(std::string name);
        hypergraph_node *GetNode(int ID);
        hypergraph_node *GetNode(std::string name);
        void LinkByIndex(int from_node_index, int to_node_index);
        void LinkByID(int from_node_ID, int to_node_ID);
        void LinkByName(std::string from_node_name, std::string to_node_name);
        void SetFlagByIndex(int node_index, int flag_index, bool flag_state);
        bool GetFlagByIndex(int node_index, int flag_index);
        void SetFlagByID(int node_ID, int flag_index, bool flag_state);
        bool GetFlagByID(int node_ID, int flag_index);
        void SetFlagByName(std::string node_name, int flag_index, bool flag_state);
        bool GetFlagByName(std::string node_name, int flag_index);
        void ClearFlags();
        void PropogateFlagFromIndex(int node_index, int flag_index, std::vector<hypergraph_node *> &members, int maximum_depth);
        void PropogateFlag(hypergraph_node *node, int flag_index, std::vector<hypergraph_node *> &members, int maximum_depth, int depth);
        static bool ContainsNode(std::vector<hypergraph_node*> &lst, hypergraph_node *n);
};

#endif
