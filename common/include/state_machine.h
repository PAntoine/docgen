/**-------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: state_machine
 *  @group: draw_state_machine
 *    desc: This file holds the API for the text drawing of the state machine.
 *
 *  author: pantoine
 *    date: 17/08/2012 16:00:43
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "document_generator.h"
#include "output_format.h"

/**-------------------------------------------------------------------------------*
 * @type	TEXT_LINE_POINT
 * @desc	This holds the line and where it ends and starts on the line.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned int	rank;						/** @brief	The rank - either x or y */
	unsigned int	start;						/** @brief	The start on the rank */
	unsigned int	end;						/** @brief	The end on the rank */
} TEXT_LINE_POINT;

/**-------------------------------------------------------------------------------*
 * @type	TEXT_VERTEX_ROUTE
 * @desc	This holds the details of a vertex route.
 *          The line points are the different places where the line changes
 *          direction. For this version it assumes that the line has three
 *          points, the start the place that it bends and the end.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned int	length;						/** @brief	The length of the vertex */
	unsigned int	crossings;					/** @brief	The number of crossings the vertex makes */
	unsigned int	end_section;				/** @brief	which section does the end start on */
	unsigned int	x_direction;				/** @brief	which direction is x going ^ = 0 and v = 1 */
	unsigned int	y_direction;				/** @brief	which direction is y doing <- = 0 and -> = 1 */
	TEXT_LINE_POINT	points[2];					/** @brief	The line points for the line section */

} TEXT_VERTEX_ROUTE;


/**-------------------------------------------------------------------------------*
 * @type	TEXT_STATE_NODE
 * @desc	A node for the state machine.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	STATE*			state;					/** @brief	The state that node points to */
	unsigned int	x;						/** @brief	The virtual x offset of the node */
	unsigned int	y;						/** @brief	The virtual y offset of the node */
} TEXT_STATE_NODE;

/**-------------------------------------------------------------------------------*
 * @type	TEXT_STATE_VERTEX
 * @desc	A vertex for the state machine.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned int		from;				/** @brief	The index of the node that the message is from */
	unsigned int		to;					/** @brief	The index of the node that the message is going to */
	STATE_TRANSITION*	transition;			/** @brief	The transition that is the vertex */
	TEXT_VERTEX_ROUTE	route;				/** @brief	The route that has been laid for the vertex */

} TEXT_STATE_VERTEX;

/**-------------------------------------------------------------------------------*
 * @type	TEXT_STATE_MACHINE
 * @desc	The base type that holds the structure for the state machine.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned int		num_nodes;				/**	@brief	The number of the nodes in the state machine */
	unsigned int		num_vertices;			/** @brief	The number of vertices in the state machine */
	unsigned int		node_index;				/** @brief	The index for the next node to be added */
	unsigned int		vertex_index;			/** @brief	The index for the next vertex to be added */
	unsigned int		max_node_name_length;	/** @brief	The max name length of the nodes */
	unsigned int		x_size;					/** @brief	The biggest x value */
	unsigned int		y_size;					/** @brief	The biggest y value */
	TEXT_STATE_NODE*	node_list;				/** @brief	The list of nodes in the state machine */
	TEXT_STATE_VERTEX*	vertex_list;			/** @brief	The list of vertices in the state machine */

} TEXT_STATE_MACHINE;

/*--------------------------------------------------------------------------------*
 * The API functions for the state machine
 *--------------------------------------------------------------------------------*/
void state_machine_allocate(TEXT_STATE_MACHINE** state_machine);
void state_machine_set_size(TEXT_STATE_MACHINE* state_machine, unsigned int num_nodes, unsigned int num_vertices);
void state_machine_add_node(TEXT_STATE_MACHINE* state_machine, STATE* state, unsigned int x, unsigned int y);
void state_machine_add_vertex(TEXT_STATE_MACHINE* state_machine, STATE_TRANSITION* node, unsigned int from, unsigned int to);
void state_machine_draw(TEXT_STATE_MACHINE* state_machine, DRAW_STATE* draw_state);
void state_machine_release(TEXT_STATE_MACHINE* state_machine);

#endif

