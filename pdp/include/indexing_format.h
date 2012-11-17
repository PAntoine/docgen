/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: indexing_format
 *   @desc: This file holds the definitions for the indexing format.
 *
 * @author: pantoine
 *   @date: 06/07/2012 06:35:41
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef _INDEXING_FORMAT_H__
#define _INDEXING_FORMAT_H__

#include "output_format.h"

/*--------------------------------------------------------------------------------*
 * TEXT format function types
 *--------------------------------------------------------------------------------*/
unsigned int	index_decode_flags(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned hash, NAME* value);
unsigned int	index_open(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned char* name, unsigned int name_length);
void			index_close(DRAW_STATE* draw_state, INPUT_STATE* input_state);
void			index_header(DRAW_STATE* draw_state, INPUT_STATE* input_state);
void			index_footer(DRAW_STATE* draw_state, INPUT_STATE* input_state);
void			index_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size);
void			index_sample(DRAW_STATE* draw_state, SAMPLE* sample);
void			index_message(DRAW_STATE* draw_state, MESSAGE* message);
void			index_timelines(DRAW_STATE* draw_state);
void			index_state_set_size(DRAW_STATE* draw_state, unsigned int nodes, unsigned int vertices);
void			index_state(DRAW_STATE* draw_state,STATE* state, unsigned int x, unsigned int y);
void			index_transition(DRAW_STATE* draw_state,STATE* state,STATE_TRANSITION* transition,unsigned int from_id,unsigned int to_id);
void			index_marker(DRAW_STATE* draw_state, unsigned int marker);
void			index_text(DRAW_STATE* draw_state, unsigned int index_style,NAME* text);
void			index_section(DRAW_STATE*	draw_state, unsigned int header_level, NAME* name, unsigned int	format, NAME* section_data); 
void			index_title(DRAW_STATE* draw_state, unsigned int header_level, NAME* title, NAME* name);
void			index_block(DRAW_STATE* draw_state, unsigned int format, NAME* block);
void			index_table_start(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout);
void			index_table_header(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row);
void			index_table_row(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row);
void			index_table_end(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout);
void			index_chapter(DRAW_STATE* draw_state, LEVEL_INDEX* level_index, NAME* chapter_name);
void			index_entry(DRAW_STATE* draw_state, LEVEL_INDEX* level_index, NAME* entry);
void			index_index_chapter(DRAW_STATE* draw_state, DOC_SECTION* index_item);
void			index_index_start_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item);
void			index_index_entry(DRAW_STATE* draw_state, DOC_SECTION* index_item);
void			index_index_end_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item);
void			index_list_item_start(DRAW_STATE* draw_state, unsigned int level, unsigned char marker);
void			index_list_numeric_start(DRAW_STATE* draw_state, unsigned int level);
void			index_list_end(DRAW_STATE* draw_state);

/*--------------------------------------------------------------------------------*
 * Other functions that are referenced
 *--------------------------------------------------------------------------------*/
DOC_SECTION*	index_add_section(DRAW_STATE* draw_state);

#endif

