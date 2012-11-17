/**-------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: state_machine
 *  @group: draw_state_machine
 *    desc: This file holds the functions for rendering the state machine in 
 *          ascii art form.
 *
 *          It expects the graph to be laid out in a grid form, this code does not
 *          handle that part. See the graph.c that will generate a layout in the
 *          correct format.
 *
 *  author: pantoine
 *    date: 17/08/2012 15:56:27
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include "state_machine.h"

#include <malloc.h>
#include <memory.h>

/**---- FUNCTION -----------------------------------------------------------------*
 *  @api	state_machine_allocate
 *  @desc	This function will allocate the state machine structure.
 *
 *  @action	This function simply allocates the space for the *TEXT_STATE_MACHINE*
 *          structure that will hold the nodes and vertices that will be added to
 *          the state machine before rendering.
 *
 *  @parameter	state_machine	A pointer to a pointer that will have the state
 *  							machine structure in it.
 *--------------------------------------------------------------------------------*/
void	state_machine_allocate(TEXT_STATE_MACHINE** state_machine)
{
	*state_machine = calloc(1,sizeof(TEXT_STATE_MACHINE));
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  @api	state_machine_set_size
 *  @desc	The function will set the size of the graph. 
 *
 *  @action	This function will create the two arrays that hold the nodes and
 *          vertices lists.
 *
 *  @parameter	state_machine	A pointer to a pointer that will have the state
 *  							machine structure in it.
 *  @parameter  num_nodes       The number of nodes.
 *  @parameter  num_vertices	The number of vertices.
 *--------------------------------------------------------------------------------*/
void	state_machine_set_size(TEXT_STATE_MACHINE* state_machine, unsigned int num_nodes, unsigned int num_vertices)
{
	state_machine->num_nodes	= num_nodes;
	state_machine->num_vertices = num_vertices;
	state_machine->node_index	= 0;
	state_machine->node_list	= calloc(num_nodes,sizeof(TEXT_STATE_NODE));
	state_machine->vertex_list	= calloc(num_vertices,sizeof(TEXT_STATE_VERTEX));
}

/**---- FUNCTION -----------------------------------------------------------------*
 *  @api        state_machine_add_node
 *  @desc       This function will add a node to the *state_machine* structure.
 *
 *  @action	    This function will create and add a node to the list of nodes that
 *              require rendering.
 *
 *  @parameter  state_machine   A pointer to the state machine that the node will
 *                              be added to.
 *  @parameter  state           A pointer to the state that will be added to the
 *                              state machine.
 *  @parameter  x               The virtual x co-ordinate of the node.
 *  @parameter  y               The virtual y co-ordinate of the node.
 *--------------------------------------------------------------------------------*/
void	state_machine_add_node(TEXT_STATE_MACHINE* state_machine, STATE* state, unsigned int x, unsigned int y)
{
	if (state_machine->max_node_name_length < state->name_length)
	{
		state_machine->max_node_name_length = state->name_length;
	}

	if (state_machine->x_size < x)
	{
		state_machine->x_size = x;
	}

	if (state_machine->y_size < y)
	{
		state_machine->y_size = y;
	}

	state_machine->node_list[state_machine->node_index].x		= x;
	state_machine->node_list[state_machine->node_index].y		= y;
	state_machine->node_list[state_machine->node_index].state	= state;

	state_machine->node_index++;
}

/**---- FUNCTION -----------------------------------------------------------------*
 *  @api        state_machine_add_vertex
 *  @desc       This function will add a vertex to the state machine.
 *  @action     This simply creates a new vertex and adds it to the list of
 *              vertices in the state machine.
 *  @parameter  state_machine   A pointer to the state machine that the node will
 *                              be added to.
 *  @parameter  node            A pointer to the transition.
 *  @parameter  from_id         The id (index number) of the sending node.
 *  @parameter  to_id           The id (index number) of the receiving node.
 *--------------------------------------------------------------------------------*/
void	state_machine_add_vertex(TEXT_STATE_MACHINE* state_machine, STATE_TRANSITION* transition, unsigned int from_id, unsigned int to_id)
{
	state_machine->vertex_list[state_machine->vertex_index].from		= from_id;
	state_machine->vertex_list[state_machine->vertex_index].to			= to_id;
	state_machine->vertex_list[state_machine->vertex_index].transition	= transition;

	state_machine->vertex_index++;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: state_machine_check_route
 *  desc: This function will check the route and count the number of crossings
 *        that the route makes. It does not lift any tracks so does not do a 
 *        recursive search, it justs works out the bend and counts the number of
 *        objects are are in the way of the track.
 *--------------------------------------------------------------------------------*/
static void	state_machine_check_route(TEXT_STATE_MACHINE* state_machine, TEXT_VERTEX_ROUTE* route, unsigned int start_id, unsigned int end_id, unsigned int x_or_y_primary)
{
	unsigned int	count;
	unsigned int	length_x = abs(state_machine->node_list[start_id].x - state_machine->node_list[end_id].x);
	unsigned int	length_y = abs(state_machine->node_list[start_id].y - state_machine->node_list[end_id].y);
	unsigned int	section[2][3];

	if (x_or_y_primary == 1)
	{
		/* from start -> bend_x -> end */
		section[0][0] = state_machine->node_list[end_id].x;
		section[1][0] = state_machine->node_list[start_id].y;
		route->end_section = 0;
	}
	else
	{
		/* from start -> bend_y -> end */
		section[0][0] = state_machine->node_list[start_id].x;
		section[1][0] = state_machine->node_list[end_id].y;
		route->end_section = 1;
	}

	/* in the x axis */
	if (state_machine->node_list[start_id].y < state_machine->node_list[end_id].y)
	{
		section[0][1] = state_machine->node_list[start_id].y;
		section[0][2] = state_machine->node_list[end_id].y;
		route->y_direction = 1;
	}
	else
	{
		section[0][1] = state_machine->node_list[end_id].y;
		section[0][2] = state_machine->node_list[start_id].y;
		route->y_direction = 0;
	}

	/* on the y-axis */
	if (state_machine->node_list[start_id].x < state_machine->node_list[end_id].x)
	{
		section[1][1] = state_machine->node_list[start_id].x;
		section[1][2] = state_machine->node_list[end_id].x;
		route->x_direction = 1;
	}
	else
	{
		section[1][1] = state_machine->node_list[end_id].x;
		section[1][2] = state_machine->node_list[start_id].x;
		route->x_direction = 0;
	}

	/* now check to see if there are any nodes in the way of this vertex */
	for (count=0; count < state_machine->num_nodes; count++)
	{
		if (count != start_id && count != end_id)
		{
			if (section[0][0] == state_machine->node_list[count].x && section[0][1] <= state_machine->node_list[count].y && state_machine->node_list[count].y <= section[0][2])
			{
				route->crossings++;
			}
			
			if (section[1][0] == state_machine->node_list[count].y && section[1][1] <= state_machine->node_list[count].x && state_machine->node_list[count].x <= section[1][2])
			{
				route->crossings++;
			}
		}
	}
	
	/* set the routing values */
	route->length = length_x + length_y;
	
	/* x */
	route->points[0].rank	= section[0][0];
	route->points[0].start	= section[0][1];
	route->points[0].end	= section[0][2];
		
	/* y */
	route->points[1].rank	= section[1][0];
	route->points[1].start	= section[1][1];
	route->points[1].end	= section[1][2];
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: route_vertex
 *  desc: This function will route a vertex.
 *        It will try and find the vertex with the minimum number of crossings.
 *        It will add a section route list to the list.
 *
 *        The algorithm it uses is a simple single bend search. It assumes that
 *        the target node is one major bend away. It will use the under and over
 *        "channels" to avoid nodes that it needs to cross.
 *
 *        This model maybe to simplistic, but lets see if it works well enought,
 *        else will have to use a full path search algorithm, and that seems a
 *        bit excessive for the number of nodes expected.
 *--------------------------------------------------------------------------------*/
static	void	route_vertex(TEXT_STATE_MACHINE* state_machine, unsigned int vertex)
{
	unsigned int		count;
	unsigned int		winner = 0;
	unsigned int		quad = 0;
	unsigned int		clock;
	unsigned int		min_length = 0;
	unsigned int		min_crossings = UINT_MAX;
	unsigned int		start_id;
	unsigned int		end_id;
	TEXT_VERTEX_ROUTE	searches[2];

	memset(searches,0,sizeof(searches));

	start_id	= state_machine->vertex_list[vertex].from;
	end_id		= state_machine->vertex_list[vertex].to;

	if (state_machine->node_list[start_id].x > state_machine->node_list[end_id].x)
	{
		/* if start > target = bottom quad pair */
		quad = 2;
	}

	if (state_machine->node_list[start_id].y > state_machine->node_list[end_id].y)
	{
		/* if start > target = left quad pair */
		quad++;
	}

	/* check the routes, 4 cardinal points, x_primary and y_primary */
	state_machine_check_route(state_machine,&searches[0],start_id,end_id,0);
	state_machine_check_route(state_machine,&searches[1],start_id,end_id,1);

	/* now find the winner */
	for (count=0; count < 2; count++)
	{
		if (searches[count].crossings <= min_crossings)
		{
			if (searches[count].crossings == min_crossings)
			{
				/* shortest path should always come first */
				if (searches[count].length < min_length)
				{
					min_crossings	= searches[count].crossings;
					min_length		= searches[count].length;
					winner			= count;
				}
			}
			else
			{
				/* minimum number of crossings */
				min_crossings	= searches[count].crossings;
				min_length		= searches[count].length;
				winner			= count;
			}
		}
	}
	
	/* copy the winner to the route */
	memcpy(&state_machine->vertex_list[vertex].route,&searches[winner],sizeof(TEXT_VERTEX_ROUTE));
}

/**---- FUNCTION -----------------------------------------------------------------*
 *  @api        state_machine_draw
 *  @desc       This function will render the state machine.
 *  @action     This function will handle the *text* rendering of the state machine
 *              it will use the text functions of the format to actually write to 
 *              the output file.
 *
 *  @parameter  state_machine   A pointer to the state machine that will be drawn.
 *--------------------------------------------------------------------------------*/
void	state_machine_draw(TEXT_STATE_MACHINE* state_machine, DRAW_STATE* draw_state)
{
	int				x_line_offset;
	NAME			draw_line[7];
	unsigned char	arrow;
	unsigned int	x;
	unsigned int	y;
	unsigned int	count;
	unsigned int	y_line;
	unsigned int	y_coord;
	unsigned int	x_size;
	unsigned int	x_offset;
	unsigned int	x_center;
	unsigned int	node_size = state_machine->max_node_name_length + 4;
	unsigned int	node_spacing = node_size + 2;
	unsigned int	line_size = (node_spacing + 1) * (state_machine->x_size + 1) + 1 + draw_state->margin_width;
	unsigned char*	line[7];

	/* create the drawing lines */
	for (count=0;count<7;count++)
	{
		line[count] = malloc(line_size);
		draw_line[count].name = line[count];
		draw_line[count].name_length = line_size;
	}

	/* route the vertices */
	for (count=0; count < state_machine->num_vertices; count++)
	{
		route_vertex(state_machine,count);
	}

	/* now draw the grids */
	for (y_coord = 1; y_coord < state_machine->y_size+1; y_coord++)
	{
		for (count=0;count<7;count++)
		{
			memset(line[count],' ',line_size);
		}


		for (count=0; count < state_machine->num_vertices; count++)
		{
			x_offset = node_spacing * (state_machine->vertex_list[count].route.points[0].rank - 1) + node_size/2 + 1 + draw_state->margin_width;

			/* check to see what direction it is coming from in x */
			if (state_machine->vertex_list[count].route.points[0].rank < state_machine->vertex_list[count].route.points[1].end)
			{
				x_offset++;
				x_line_offset = 1;
			}
			else if (state_machine->vertex_list[count].route.points[0].rank > state_machine->vertex_list[count].route.points[1].start)
			{
				x_line_offset = -1;
				x_offset--;
			}
			else
			{
				x_line_offset = 0;
			}

			/* check if any lines cross the y_axis */
			if ((state_machine->vertex_list[count].route.points[0].start == y_coord) &&
				(state_machine->vertex_list[count].route.points[0].end == y_coord))
			{
				y_line = 3;
			}
			else if (state_machine->vertex_list[count].route.points[0].start == y_coord)
			{
				line[5][x_offset] = '|';

				if (line[6][x_offset] != '^')
					line[6][x_offset] = '|';

				y_line = 4;
			}
			else if (state_machine->vertex_list[count].route.points[0].end == y_coord)
			{
				line[0][x_offset] = '|';
				line[1][x_offset] = '|';

				y_line = 2;
			}
			else if (state_machine->vertex_list[count].route.points[0].end > y_coord && state_machine->vertex_list[count].route.points[0].start < y_coord)
			{

				line[0][x_offset] = '|';
				line[1][x_offset] = '|';
				line[2][x_offset] = '|';
				line[3][x_offset] = '|';
				line[4][x_offset] = '|';
				line[5][x_offset] = '|';
				line[6][x_offset] = '|';
				y_line = 3;
			}

			/* handle arrows along the y-axis */
			if (state_machine->vertex_list[count].route.end_section == 0)
			{
				if (state_machine->vertex_list[count].route.points[0].start == y_coord && state_machine->vertex_list[count].route.y_direction == 0)
				{
					line[6][x_offset] = '^';
				}
				else if (state_machine->vertex_list[count].route.points[0].start == y_coord && state_machine->vertex_list[count].route.y_direction == 1)
				{
					line[0][x_offset] = 'v';
				}
			}

			/* check to see if any lines run along the y_axis - check the bend */
			if (y_coord == state_machine->vertex_list[count].route.points[1].rank && state_machine->vertex_list[count].route.points[1].end != state_machine->vertex_list[count].route.points[1].start)
			{
				x_offset = (node_spacing * (state_machine->vertex_list[count].route.points[1].start - 1) + node_size/2) + x_line_offset + 1 + draw_state->margin_width;
				x_size   = node_spacing * (state_machine->vertex_list[count].route.points[1].end - state_machine->vertex_list[count].route.points[1].start);
				memset(&line[y_line][x_offset+1],'-',x_size-1);
			}

			/* handle the arrows alone the x-axis */
			if (state_machine->vertex_list[count].route.end_section == 1 && state_machine->vertex_list[count].route.points[1].rank == y_coord)
			{
				/* the end is on the y-axis */
				y = state_machine->vertex_list[count].route.points[1].rank;

				if (state_machine->vertex_list[count].route.x_direction == 0)
				{
					x_offset = node_spacing * (state_machine->vertex_list[count].route.points[1].start-1) + node_size + 1 + draw_state->margin_width;
					arrow = '<';
				}
				else
				{
					x_offset = node_spacing * (state_machine->vertex_list[count].route.points[1].end-1) + draw_state->margin_width;
					arrow = '>';
				}
			
				line[y_line][x_offset] = arrow;
			}
		}

		/* now check to see if any boxes are on the line */
		for (count=0; count < state_machine->num_nodes; count++)
		{
			if (y_coord == state_machine->node_list[count].y)
			{
				x_offset = node_spacing * (state_machine->node_list[count].x - 1) + 1 + draw_state->margin_width;
				x_center = (node_size - state_machine->node_list[count].state->name_length) / 2;

				line[2][x_offset] = '|';
				line[3][x_offset] = '|';
				line[4][x_offset] = '|';

				memset(&line[1][x_offset+1],'-',node_size-2);
				memset(&line[5][x_offset+1],'-',node_size-2);

				memset(&line[2][x_offset+1],' ',node_size-2);
				memset(&line[3][x_offset+1],' ',node_size-2);
				memset(&line[4][x_offset+1],' ',node_size-2);

				memcpy(&line[3][x_offset+x_center],state_machine->node_list[count].state->name,state_machine->node_list[count].state->name_length);

				line[2][x_offset + node_size-1] = '|';
				line[3][x_offset + node_size-1] = '|';
				line[4][x_offset + node_size-1] = '|';
			}
		}

		/* write the lines out */
		for (count=0;count<7;count++)
		{
			draw_state->format->output_raw(draw_state,draw_line[count].name,draw_line[count].name_length);
		}
	}

	/* release all the allocations */
	for (count=0;count<7;count++)
	{
		free(line[count]);
	}
}

/**---- FUNCTION -----------------------------------------------------------------*
 *  @api        state_machine_release
 *  @desc       This function will release the state machine.
 *  @action     It releases all the memory that has been allocated to the text
 *              state machine structures.
 *
 *  @parameter  state_machine   A pointer to the state machine that will be drawn.
 *--------------------------------------------------------------------------------*/
void	state_machine_release(TEXT_STATE_MACHINE* state_machine)
{
	free(state_machine->node_list);
	free(state_machine->vertex_list);

	state_machine->node_list	= NULL;
	state_machine->vertex_list	= NULL;

	free(state_machine);
}

