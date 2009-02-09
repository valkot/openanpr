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

