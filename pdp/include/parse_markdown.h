/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : parse_markdown
 * Desc  : This holds the external definition of the markdown parse functions.
 *
 * Author: peterantoine
 * Date  : 16/10/2012 19:25:19
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __PARSE_MARKDOWN_H__
#define __PARSE_MARKDOWN_H__

unsigned int	markdown_parse_input(DRAW_STATE* draw_state,INPUT_STATE* input_state);
unsigned int	markdown_parse_buffer(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned char* buffer, unsigned int buffer_size, unsigned int in_block);

#endif

