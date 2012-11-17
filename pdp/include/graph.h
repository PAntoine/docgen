/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: graph
 *   @desc: The file holds the structures that the graphs will populate.
 *
 * @author: pantoine
 *   @date: 30/07/2012 07:10:35
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __GRAPH_H__
#define __GRAPH_H__

/*#include "document_generator.h"*/

/*--------------------------------------------------------------------------------*
 * graph structures.
 *--------------------------------------------------------------------------------*/
typedef struct tag_graph_vertex GRAPH_VERTEX;

typedef struct
{
	unsigned int	x;
	unsigned int	y;

} GRAPH_POINT;

typedef struct
{
	float	x;
	float	y;

} GRAPH_POINT_FORCE;

typedef	struct tag_graph_node
{
	void*					graph_item;
	float					x;
	float					y;
	float					delta_x;
	float					delta_y;
	unsigned int			location_x;
	unsigned int			location_y;
	GRAPH_VERTEX*			vertex;

	struct tag_graph_node*	list_x;
	struct tag_graph_node*	list_y;
	struct tag_graph_node*	next;
	struct tag_graph_node*	list_next;
} GRAPH_NODE;

struct tag_graph_vertex
{
	void*			item;
	GRAPH_NODE*		to_node;
	GRAPH_NODE*		from_node;
	unsigned int	primary;			/* is this vertex a primary vertex */

	struct tag_graph_vertex*	next;
};

typedef struct
{
	unsigned int	number_nodes;
	unsigned int	number_vertex;
	GRAPH_NODE*		start_node;
	GRAPH_NODE*		last_node;
	GRAPH_NODE*		start_x;
	GRAPH_NODE*		start_y;

} GRAPH;

/*--------------------------------------------------------------------------------*
 * graph functions.
 *--------------------------------------------------------------------------------*/
void			graph_initialise(GRAPH* graph);
void			graph_add_vertex(GRAPH* graph, GRAPH_NODE* from, GRAPH_NODE* to, void* item);
GRAPH_NODE*		graph_create_node(GRAPH* graph, void* item);
void			graph_force_directed(GRAPH* graph);
void			graph_dump(GRAPH* graph);
GRAPH_NODE*		graph_get_start_node(GRAPH* graph, unsigned int* x, unsigned int* y, void** item);
GRAPH_NODE*		graph_get_next_node(GRAPH_NODE* node, unsigned int* x, unsigned int* y, void** item);
GRAPH_VERTEX*	graph_get_next_vertex(GRAPH_VERTEX* vertex, void** item);
GRAPH_VERTEX*	graph_get_start_vertex(GRAPH_NODE* node, void** item);
void			graph_release(GRAPH* graph);

#endif

