/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *    file: graph
 *    desc: This function holds the functions for creating the graph and populating
 *          the graph structure.
 *
 *          This graph drawing code holds the support structures for 
 *
 *  author: pantoine
 *    date: 30/07/2012 07:09:44
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include "graph.h"
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include "document_generator.h" 		/* DEBUG --- remove */


#define GRAPH_ENERGY_MINIMUM	(0.1)

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: graph_initialise
 *  desc: This function will initialise a graph.
 *--------------------------------------------------------------------------------*/
void	graph_initialise(GRAPH* graph)
{
	graph->number_nodes  = 0;
	graph->number_vertex = 0;
	graph->start_node = NULL;
	graph->last_node = NULL;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: graph_create_node
 *  desc: This function will add a new node to the tree.
 *--------------------------------------------------------------------------------*/
GRAPH_NODE*	graph_create_node(GRAPH* graph, void* item)
{
	GRAPH_NODE* result = calloc(1,sizeof(GRAPH_NODE));

	result->graph_item = item;
	graph->number_nodes++;
	
	/* add to the list of nodes */
	if (graph->start_node == NULL)
	{
		graph->start_node = result;
	}
	else
	{
		graph->last_node->list_next = result;
	}

	graph->last_node = result;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: graph_add_vertex
 *  desc: This function will add a vertex to the given node, this should be done
 *        to both sides of the message.
 *--------------------------------------------------------------------------------*/
static void	graph_insert_vertex(GRAPH* graph, GRAPH_NODE* node, GRAPH_VERTEX* new_vertex)
{
	GRAPH_VERTEX*	current_vertex = node->vertex;

	/* add to the graph */
	if (current_vertex == NULL)
	{
		node->vertex = new_vertex;
	}
	else
	{
		while (current_vertex != NULL)
		{
			if (current_vertex->next == NULL)
			{
				/* Ok, we are at the end of the chain */
				current_vertex->next = new_vertex;
				break;
			}

			current_vertex = current_vertex->next;
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: graph_add_vertex
 *  desc: This function will add a vertex to the graph.
 *--------------------------------------------------------------------------------*/
void	graph_add_vertex(GRAPH* graph, GRAPH_NODE* from, GRAPH_NODE* to, void* item)
{
	GRAPH_VERTEX*	from_vertex = calloc(1,sizeof(GRAPH_VERTEX));
	GRAPH_VERTEX*	to_vertex = calloc(1,sizeof(GRAPH_VERTEX));

	from_vertex->from_node	= from;
	from_vertex->to_node	= to;
	from_vertex->item		= item;
	from_vertex->primary	= 1;

	to_vertex->from_node	= to;
	to_vertex->to_node		= from;
	to_vertex->item			= item;
	to_vertex->primary		= 0;
	
	graph->number_vertex++;

	graph_insert_vertex(graph,from,from_vertex);
	graph_insert_vertex(graph,to,to_vertex);
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: calculate_repulsion
 *  desc: This function calculates the repulsion that nodes have on each other.
 *--------------------------------------------------------------------------------*/
static void	calculate_repulsion(GRAPH_POINT_FORCE* point_force, GRAPH_NODE* target_node, GRAPH_NODE* other_node)
{
	float	x_dist = target_node->x - other_node->x;
	float	y_dist = target_node->y - other_node->y;
	float	distance;
	float	force_ratio;

	/* square of the line distance between points (pythag) */
	distance = (x_dist * x_dist) + (y_dist * y_dist); 
	force_ratio = 1/distance;

	/* counting the repulsion between two vertices */
	point_force->x += x_dist * force_ratio;
	point_force->y += y_dist * force_ratio;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: calculate_attraction
 *  desc: This function will work out the spring attraction force between the
 *        two given nodes.
 *--------------------------------------------------------------------------------*/
static void	calculate_attraction(GRAPH_POINT_FORCE* point_force, GRAPH_VERTEX* vertex)
{
	point_force->x += 0.09 * (vertex->to_node->x - vertex->from_node->x);
	point_force->y += 0.09 * (vertex->to_node->y - vertex->from_node->y);
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: calculate_velocity
 *  desc: This function will calculate the velocity. It will also return the 
 *        current energy of the spring.
 *--------------------------------------------------------------------------------*/
static float	calculate_velocity(GRAPH_NODE* node, GRAPH_POINT_FORCE* force)
{
	float	result = 0;

	node->delta_x = (node->delta_x + force->x) * 0.90;
	node->delta_y = (node->delta_y + force->y) * 0.90;

	/* a bit of pythag to calculate the actual force length 
	 * assume modern processors have sqrt in the floating point unit. 
	 * (still feels dirty to use sqrt. :) )
	 */
	result = (node->delta_y * node->delta_y) + (node->delta_x * node->delta_x);

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: calculate_position
 *  desc: This function will handle the last position calculation.
 *--------------------------------------------------------------------------------*/
static void	calculate_position(GRAPH_NODE* node)
{
	node->x += node->delta_x;
	node->y += node->delta_y;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: randomise_nodes
 *  desc: This function will set the initial locations of the nodes, they should
 *        not have the same position. So just doing a random for now without
 *        checking for overlaps.
 *
 *        TODO: use a better algo, prob use LFSR as that will do what I need.
 *--------------------------------------------------------------------------------*/
static void	randomise_nodes(GRAPH* graph)
{
	GRAPH_NODE*		current_node;
	unsigned int	seed = 0xa9385323;

	/* move the items */
	current_node = graph->start_node;

	while (current_node != NULL)
	{
		seed = lfsr_32(seed);
		current_node->x = (0x0f & seed)+10;
		seed = lfsr_32(seed);
		current_node->y = (0x0f & seed)+30;

		current_node = current_node->list_next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: graph_normalise
 *  desc: This function will normalise the graph and make it sit as square as 
 *        possible to aid drawing.
 *
 *        Basic algorithm is to dived the graph up into grid and then place the
 *        items in to the slots, then remove all the rows of the grid that do not
 *        have any items in them.
 *--------------------------------------------------------------------------------*/
static void	graph_normalise(GRAPH* graph)
{
	GRAPH_NODE*		node;
	GRAPH_NODE*		search;
	unsigned int	count_x;
	unsigned int	count_y;
	unsigned int	last_pos_x = 0;
	unsigned int	last_pos_y = 0;

	/* now calculate the repulsion */
	node = graph->start_node;

	graph->start_x = node;
	graph->start_y = node;

	node = node->list_next;

	/* sort the items along the x and y axis */
	while (node != NULL)
	{
		/* sort in x */
		if (graph->start_x->x > node->x)
		{
			node->list_x = graph->start_x;
			graph->start_x = node;
		}
		else
		{
			search = graph->start_x;

			while (search != NULL)
			{
				if (search->list_x == NULL)
				{
					search->list_x = node;
					break;
				}
				else if (search->list_x->x > node->x)
				{
					node->list_x = search->list_x;
					search->list_x = node;
					break;
				}

				search = search->list_x;
			}
		}

		/* sort in y */
		if (graph->start_y->y > node->y)
		{
			node->list_y = graph->start_y;
			graph->start_y = node;
		}
		else
		{
			search = graph->start_y;

			while (search != NULL)
			{
				if (search->list_y == NULL)
				{
					search->list_y = node;
					break;
				}
				else if (search->list_y->y > node->y)
				{
					node->list_y = search->list_y;
					search->list_y = node;
					break;
				}

				search = search->list_y;
			}
		}

		node = node->list_next;
	}

	/* now do the normalisation in x */
	count_x = 0;
	search = graph->start_x;

	while (search != NULL)
	{
		if (last_pos_x != ((unsigned int) search->x))
		{
			last_pos_x = ((unsigned int) search->x);
			count_x++;
		}
		
		search->location_x = count_x;

		search = search->list_x;
	}

	/* now do the normalisation in y */
	count_y = 0;
	search = graph->start_y;

	while (search != NULL)
	{
		if (last_pos_y != ((unsigned int) search->y))
		{
			last_pos_y = ((unsigned int) search->y);
			count_y++;
		}
		
		search->location_y = count_y;

		search = search->list_y;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: directed_graph_force
 *  desc: This function will calculate the virtual layout of the graph using a
 *        force directed algorithm.
 *--------------------------------------------------------------------------------*/
void	graph_force_directed(GRAPH* graph)
{
	GRAPH_NODE*			current_node;
	GRAPH_NODE*			repulsion_node;
	GRAPH_VERTEX*		current_vertex;
	GRAPH_POINT_FORCE	point_force;
	float				total_energy = 0;

	/* need to randomise the node locations as to try and avoid local minima */
	randomise_nodes(graph);
	
	do
	{
		current_node = graph->start_node;
		total_energy = 0;

		while (current_node != NULL)
		{
			point_force.x = 0;
			point_force.y = 0;

			/* now calculate the repulsion */
			repulsion_node = graph->start_node;

			while (repulsion_node != NULL)
			{
				/* only want other nodes */
				if (repulsion_node != current_node)
				{
					calculate_repulsion(&point_force,current_node,repulsion_node);
				}

				repulsion_node = repulsion_node->list_next;
			}

			/* now calculate the vertex (spring) forces */
			current_vertex = current_node->vertex;

			while (current_vertex != NULL)
			{
				calculate_attraction(&point_force,current_vertex);

				current_vertex = current_vertex->next;
			}

			/* now need to work out the velocity of the item */
			total_energy += calculate_velocity(current_node,&point_force);
			current_node = current_node->list_next;
		}

		/* move the items */
		current_node = graph->start_node;
		
		while (current_node != NULL)
		{
			calculate_position(current_node);

			current_node = current_node->list_next;
		}
	} 
	while (total_energy > GRAPH_ENERGY_MINIMUM);
	
	/* normalise the graph for drawing */
	graph_normalise(graph);
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: graph_get_start_node
 *  desc: This function returns the first node in the graph.
 *--------------------------------------------------------------------------------*/
GRAPH_NODE*	graph_get_start_node(GRAPH* graph, unsigned int* x, unsigned int* y, void** item)
{
	GRAPH_NODE*	result = graph->start_node;

	if (result != NULL)
	{
		*x = result->location_x;
		*y = result->location_y;
		*item = result->graph_item;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: graph_get_next
 *  desc: This function returns the next node in the graph list.
 *--------------------------------------------------------------------------------*/
GRAPH_NODE*	graph_get_next_node(GRAPH_NODE* node, unsigned int* x, unsigned int* y, void** item)
{
	GRAPH_NODE*	result = node->list_next;
	
	if (result != NULL)
	{
		*x = result->location_x;
		*y = result->location_y;
		*item = result->graph_item;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: get_next_vertex
 *  desc: This function will get the next vertex from the graph, it is only
 *        interested in the primary vertices as there other are constructs that
 *        are used for the drawing algorithm(s).
 *--------------------------------------------------------------------------------*/
GRAPH_VERTEX*	graph_get_next_vertex(GRAPH_VERTEX* vertex, void** item)
{
	GRAPH_VERTEX* result = NULL;
	GRAPH_VERTEX* current_vertex = vertex->next;

	while (current_vertex != NULL)
	{
		if (current_vertex->primary)
		{
			result = current_vertex;
			*item  = current_vertex->item;
			break;
		}

		current_vertex = current_vertex->next;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: get_start_vertex
 *  desc: This function will get the first primary vertex for the node.
 *--------------------------------------------------------------------------------*/
GRAPH_VERTEX*	graph_get_start_vertex(GRAPH_NODE* node, void** item)
{
	GRAPH_VERTEX* result = NULL;
	GRAPH_VERTEX* current_vertex = node->vertex;

	while (current_vertex != NULL)
	{
		if (current_vertex->primary)
		{
			result = current_vertex;
			*item  = current_vertex->item;
			break;
		}

		current_vertex = current_vertex->next;
	}
	
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: graph_release
 *  desc: This function will release all the nodes used in the graph.
 *--------------------------------------------------------------------------------*/
void	graph_release(GRAPH* graph)
{
	GRAPH_NODE*		temp;
	GRAPH_NODE*		current_node;

	/* move the items */
	current_node = graph->start_node;

	while (current_node != NULL)
	{
		temp = current_node->list_next;

		current_node->list_next = NULL;
		
		free(current_node);
		
		current_node = temp;
	}
}


