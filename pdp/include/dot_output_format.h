/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: dot_output_format
 *   @desc: This file holds the definition for outputting to the dot formatted
 *          files.
 *
 * @author: pantoine
 *   @date: 06/07/2012 06:37:34
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __DOT_OUTPUT_FORMAT_H__
#define __DOT_OUTPUT_FORMAT_H__

/*--------------------------------------------------------------------------------*
 * DOT format function types
 *--------------------------------------------------------------------------------*/
void	dot_output_header(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
void	dot_output_footer(DRAW_STATE* draw_state);
void	dot_output_message(DRAW_STATE* draw_state, MESSAGE* message);
void	dot_output_timelines(DRAW_STATE* draw_state, TIMELINE* timeline);
void	dot_output_states(DRAW_STATE* draw_state, STATE* list);
void	dot_output_state(DRAW_STATE* draw_state, STATE* state);

#endif

