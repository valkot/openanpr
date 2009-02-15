/*
    hypergraph
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

#include "hypergraph.h"

// ********** constructors / destructors **********


hypergraph::hypergraph()
{
}

/*! \brief constructor
 *  \param no_of_nodes number of nodes in the graph
 *  \param no_of_flags number of flags per node
 */
hypergraph::hypergraph(
    int no_of_nodes,
    int no_of_flags)
{
    for (int i = 0; i < no_of_nodes; i++)
    {
        hypergraph_node *node = new hypergraph_node(no_of_flags);
        node->ID = i;
        char nodenumber[10];
        sprintf(nodenumber,"%d",i);
        node->Name = std::string(nodenumber);
        Add(node);
    }
}

/*! \brief destructor
 */
hypergraph::~hypergraph()
{
	for (int i = (int)Nodes.size() - 1; i >= 0; i--)
		delete Nodes[i];
}


// ********** public methods **********


/*!
 * \brief Add adds a new node to the graph
 * \param node node to be added
 */
void hypergraph::Add(
    hypergraph_node *node)
{
    Nodes.push_back(node);
}

/*!
 * \brief Remove removes a node from the graph
 * \param node node to be removed
 */
void hypergraph::Remove(
    hypergraph_node *node)
{
	int i = 0;
	while (i < (int)Nodes.size())
	{
        if (Nodes[i] == node)
        {
	        std::vector<hypergraph_node*>::iterator itt = Nodes.begin();
	        advance(itt, i);
            Nodes.erase(itt);
            delete node;
            break;
        }
        i++;
    }
}

/*!
 * \brief Remove removes a node from the graph
 * \param ID identifier of the node to be removed
 */
void hypergraph::Remove(
    int ID)
{
    int index = IndexOf(ID);
    std::vector<hypergraph_node*>::iterator victim;
    victim = Nodes.begin();
    advance(victim, index);

    if (index > -1)
    {
        delete Nodes[index];
        Nodes.erase(victim);
    }
}

/*!
 * \brief Remove removes a node from the graph
 * \param name name of the node to be removed
 */
void hypergraph::Remove(
    std::string name)
{
    int index = IndexOf(name);
    std::vector<hypergraph_node*>::iterator victim;
    victim = Nodes.begin();
    advance(victim, index);
    if (index > -1)
    {
        delete Nodes[index];
        Nodes.erase(victim);
    }
}

/*!
 * \brief IndexOf returns the vector index of the node with the given identifier
 * \param ID identifier number
 * \return vector index number (-1 if not found)
 */
int hypergraph::IndexOf(
    int ID)
{
    int index = -1;
    int i = 0;
    int max = (int)Nodes.size();
    while ((i < max) && (index == -1))
    {
	    hypergraph_node *n = Nodes[i];
        if (n->ID == ID) index = i;
        i++;
    }
    return(index);
}

/*!
 * \brief IndexOf returns the vector index of the node with the given name
 * \param name name of the node
 * \return vector index number (-1 if not found)
 */
int hypergraph::IndexOf(
    std::string name)
{
    int index = -1;
    int i = 0;
    int max = (int)Nodes.size();
    while ((i < max) && (index == -1))
    {
        if (Nodes[i]->Name.compare(name) == 0) index = i;
        i++;
    }
    return(index);
}

/*!
 * \brief GetNode
 * \param ID
 * \return
 */
hypergraph_node *hypergraph::GetNode(
    int ID)
{
    hypergraph_node *node = NULL;
    int index = IndexOf(ID);
    if (index > -1) node = Nodes[index];
    return(node);
}

/*!
 * \brief GetNode
 * \param name
 * \return
 */
hypergraph_node *hypergraph::GetNode(
    std::string name)
{
    hypergraph_node *node = NULL;
    int index = IndexOf(name);
    if (index > -1) node = Nodes[index];
    return(node);
}

/*!
 * \brief LinkByIndex
 * \param from_node_index
 * \param to_node_index
 */
void hypergraph::LinkByIndex(
    int from_node_index,
    int to_node_index)
{
    hypergraph_link *link = new hypergraph_link();
    link->From = Nodes[from_node_index];
    link->To = Nodes[to_node_index];
    link->To->Add(link);
}

/*!
 * \brief LinkByID
 * \param from_node_ID
 * \param to_node_ID
 */
void hypergraph::LinkByID(
    int from_node_ID,
    int to_node_ID)
{
    hypergraph_link *link = new hypergraph_link();
    link->From = GetNode(from_node_ID);
    link->To = GetNode(to_node_ID);
    link->To->Add(link);
}

/*!
 * \brief LinkByName
 * \param from_node_name
 * \param to_node_name
 */
void hypergraph::LinkByName(
    std::string from_node_name,
    std::string to_node_name)
{
    hypergraph_link *link = new hypergraph_link();
    link->From = GetNode(from_node_name);
    link->To = GetNode(to_node_name);
    link->To->Add(link);
}

/*!
 * \brief SetFlagByIndex
 * \param node_index
 * \param flag_index
 * \param flag_state
 */
void hypergraph::SetFlagByIndex(
    int node_index,
    int flag_index,
    bool flag_state)
{
    Nodes[node_index]->Flags[flag_index] = flag_state;
}

/*!
 * \brief GetFlagByIndex returns the state of flag for the given node
 * \param node_index
 * \param flag_index
 * \return
 */
bool hypergraph::GetFlagByIndex(
    int node_index,
    int flag_index)
{
    return(Nodes[node_index]->Flags[flag_index]);
}

/*!
 * \brief SetFlagByID sets the state of a flag
 * \param node_ID identifier of the node
 * \param flag_index index number of the flag
 * \param flag_state state to which the flag will be set
 */
void hypergraph::SetFlagByID(
    int node_ID,
    int flag_index,
    bool flag_state)
{
    hypergraph_node *node = GetNode(node_ID);
    node->Flags[flag_index] = flag_state;
}

/*!
 * \brief GetFlagByID returns the state of a flag
 * \param node_ID identifier of the node
 * \param flag_index index number of the flag
 * \return flag state
 */
bool hypergraph::GetFlagByID(
    int node_ID,
    int flag_index)
{
    hypergraph_node *node = GetNode(node_ID);
    return(node->Flags[flag_index]);
}

/*!
 * \brief SetFlagByName sets the state of a flag
 * \param node_name name of the node
 * \param flag_index index number of the flag
 * \param flag_state state to which the flag will be set
 */
void hypergraph::SetFlagByName(
    std::string node_name,
    int flag_index,
    bool flag_state)
{
    hypergraph_node *node = GetNode(node_name);
    node->Flags[flag_index] = flag_state;
}

/*!
 * \brief GetFlagByName gets the state of a flag
 * \param node_name name of the node
 * \param flag_index index number of the flag
 * \return flag state
 */
bool hypergraph::GetFlagByName(
    std::string node_name,
    int flag_index)
{
    hypergraph_node *node = GetNode(node_name);
    return(node->Flags[flag_index]);
}

/*!
 * \brief clear the state of all flags
 */
void hypergraph::ClearFlags()
{
    for (int i = 0; i < (int)Nodes.size(); i++)
    {
        for (int j = 0; j < (int)Nodes[i]->Flags.size(); j++)
            Nodes[i]->Flags[j] = false;
    }
}

/*!
 * \brief propogates the given flag index from the given node
 * \param node_index vector index number of the node
 * \param flag_index flag index to use as the propogation channel
 * \param members returned set of nodes
 */
void hypergraph::PropogateFlagFromIndex(
    int node_index,
    int flag_index,
    std::vector<hypergraph_node *> &members,
    int maximum_depth)
{
	hypergraph_node *node = Nodes[node_index];
	if (node->Flags[flag_index] == false)
	{
		node->Flags[flag_index] = true;
		node->depth = 1;
		members.push_back(node);
		for (int i = 0; i < (int)node->Links.size(); i++)
		{
			hypergraph_link *link = node->Links[i];
			PropogateFlag(link->From, flag_index, members, maximum_depth, 2);
		}
	}
}


/*!
 * \brief propogates the given flag index from the given node
 * \param node node object
 * \param flag_index flag index to use as the propogation channel
 * \param members returned set of nodes
 */
void hypergraph::PropogateFlag(
    hypergraph_node *node,
    int flag_index,
    std::vector<hypergraph_node *> &members,
    int maximum_depth,
    int depth)
{
    if (node->Flags[flag_index] == false)
	{
		node->Flags[flag_index] = true;
		node->depth = depth;
		members.push_back(node);
		if (depth < maximum_depth)
		{
			for (int i = 0; i < (int)node->Links.size(); i++)
			{
				hypergraph_link *link = node->Links[i];
				PropogateFlag(link->From, flag_index, members, maximum_depth, depth + 1);
			}
		}
	}
}

/*! \brief returns true if the given list contains the given node
 *  \param lst list to be searched
 *  \param node to be searched for
 *  \return true if the node is in the given list
 */
bool hypergraph::ContainsNode(std::vector<hypergraph_node*> &lst,
                              hypergraph_node *n)
{
	int i = 0;
	while (i < (int)lst.size())
	{
		if (lst[i] == n) break;
		i++;
	}
	if (i < (int)lst.size())
	    return(true);
	else
	    return(false);
}

