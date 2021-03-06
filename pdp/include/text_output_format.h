/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: text_output_format
 *   @desc: This file holds the definitions for the text output format.
 *
 * @author: pantoine
 *   @date: 06/07/2012 06:35:41
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __TEXT_OUTPUT_FORMAT_H__
#define __TEXT_OUTPUT_FORMAT_H__

#include "output_format.h"

/*--------------------------------------------------------------------------------*
 * TEXT format function types
 *--------------------------------------------------------------------------------*/
unsigned int	text_decode_flags(DRAW_STATE* draw_state,INPUT_STATE* input_state, unsigned hash,NAME* value);
unsigned int	text_open(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned char* name, unsigned int name_length);
void			text_close(DRAW_STATE* draw_state, INPUT_STATE* input_state);
void			text_output_header(DRAW_STATE* draw_state, INPUT_STATE* input_state);
void			text_output_footer(DRAW_STATE* draw_state, INPUT_STATE* input_state);
void			text_output_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size);
void			text_output_sample(DRAW_STATE* draw_state, SAMPLE* sample);
void			text_output_message(DRAW_STATE* draw_state, MESSAGE* message);
void			text_output_timelines(DRAW_STATE* draw_state);
void			text_output_state_set_size(DRAW_STATE* draw_state, unsigned int nodes, unsigned int vertices);
void			text_output_state(DRAW_STATE* draw_state,STATE* state, unsigned int x, unsigned int y);
void			text_output_transition(DRAW_STATE* draw_state,STATE* state,STATE_TRANSITION* transition,unsigned int from_id,unsigned int to_id);
void			text_output_marker(DRAW_STATE* draw_state, unsigned int marker);
void			text_output_text(DRAW_STATE* draw_state, unsigned int text_style,NAME* text);
void			text_output_section(DRAW_STATE*	draw_state, unsigned int header_level, NAME* name, unsigned int	format, NAME* section_data); 
void			text_output_title(DRAW_STATE* draw_state, unsigned int header_level, NAME* title, NAME* name);
void			text_output_block(DRAW_STATE* draw_state, unsigned int format, NAME* block);
void			text_output_table_start(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout);
void			text_output_table_header(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row);
void			text_output_table_row(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row);
void			text_output_table_end(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout);
void			text_output_index_chapter(DRAW_STATE* draw_state, DOC_SECTION* index_item);
void			text_output_index_start_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item);
void			text_output_index_entry(DRAW_STATE* draw_state, DOC_SECTION* index_item);
void			text_output_index_end_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item);
void			text_output_list_item_start(DRAW_STATE* draw_state, unsigned int level, unsigned char marker);
void			text_output_list_numeric_start(DRAW_STATE* draw_state, unsigned int level);
void			text_output_list_end(DRAW_STATE* draw_state);


#endif

