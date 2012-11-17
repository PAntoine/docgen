/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: indexing_format
 *   @desc: This file holds the functions for the pseudo format index.
 *    
 *          This format is used to index the files. It is called by the markdown
 *          parser to generate the index. This is the simplest way of generating
 *          the index as it means that there is no need for two parsers for
 *          handling the same code (and the parser does not need any special 
 *          states to handle the indexing mode).
 *
 *
 * @author: pantoine
 *   @date: 08/07/2012 19:04:54
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <string.h>
#include <malloc.h>
#include "output.h"
#include "utilities.h"
#include "error_codes.h"
#include "indexing_format.h"
#include "document_generator.h"

static unsigned int g_capture_title = 0;

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_add_section
 * Desc : This function adds a section to the index.
 *--------------------------------------------------------------------------------*/
DOC_SECTION*	index_add_section(DRAW_STATE* draw_state)
{
	DOC_SECTION*	new_parent;
	DOC_SECTION*	new_section;
	unsigned short	new_level;
	unsigned short	current_level;
	unsigned short	previous_level;

	/* create the new section */
	new_section = calloc(1,sizeof(DOC_SECTION));
	
	/* at this point should add the new level to the index */

	/* work out the new level */
	current_level	= level_index_get_level(&draw_state->index);
	previous_level	= level_index_get_previous_level(&draw_state->index);

	if (current_level > previous_level)
	{
		/* child section */
		new_section->parent = (DOC_SECTION*)draw_state->format_state;
		new_section->parent->child = new_section;
	}
	else if (current_level < previous_level)
	{
		/* go, up levels */
		new_parent = ((DOC_SECTION*)draw_state->format_state)->parent;

		while (new_parent->level >= current_level)
		{
			new_parent = new_parent->parent;
		}

		new_section->parent = new_parent;
		new_section->parent->child->last->next = new_section;
	}
	else
	{
		new_section->parent = ((DOC_SECTION*)draw_state->format_state)->parent;
		new_section->parent->child->last->next = new_section;
	}

	draw_state->format_state = new_section;
	new_section->parent->child->last = new_section;

	/* add the new section for the header */
	new_section->level = current_level;
	new_section->index = level_index_get_index(&draw_state->index,new_section->level);
	
	return new_section;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_decode_flags
 * @desc: This function will decode the flags that are local to text functions.
 *        Set the index-only flag as this lets things know that the indexer is
 *        running.
 *--------------------------------------------------------------------------------*/
unsigned int	index_decode_flags(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned hash, NAME* value)
{
	unsigned int result = OUTPUT_FLAG_TYPE_BOOLEAN;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_open
 * Desc : This function will increment the root section number.
 *--------------------------------------------------------------------------------*/
unsigned int	index_open(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned char* name, unsigned int name_length)
{
	NAME		chapter_name;
	CHAPTER*	new_chapter = calloc(1,sizeof(CHAPTER));

	/* create the root document if required */
	if (draw_state->model->document == NULL)
	{
		draw_state->model->document = calloc(1,sizeof(DOCUMENT));
		draw_state->model->document->last_chapter = &draw_state->model->document->chapter_list;
	}

	/* create the new chapter */
	chapter_name.name = name;
	chapter_name.name_length = name_length;

	draw_state->global_format_flags = OUTPUT_FORMAT_INDEX_ONLY;

	copy_name(&chapter_name,&new_chapter->file_name);

	/* add it to the list of chapters */
	draw_state->model->document->chapter_count++;
	draw_state->model->document->last_chapter->next	= new_chapter;
	draw_state->model->document->last_chapter		= new_chapter;
	
	/* set the root of the index */
	draw_state->format_state = &new_chapter->root;
	new_chapter->root.level = 0;
	new_chapter->root.index = draw_state->model->document->chapter_count;

	new_chapter->index = draw_state->model->document->chapter_count;

	return EC_OK;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_close
 * Desc : This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_close(DRAW_STATE* draw_state,INPUT_STATE* input_state)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_header
 * Desc : This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_header(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_footer
 * Desc : nothing.
 *--------------------------------------------------------------------------------*/
void	index_footer(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_raw
 * Desc : nothing.
 *--------------------------------------------------------------------------------*/
void	index_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size)
{
	NAME	string;

	string.name = buffer;
	string.name_length = buffer_size;

	if (g_capture_title)
	{
		copy_name(&string,&((DOC_SECTION*)draw_state->format_state)->section_title);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_sample
 * @desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_sample(DRAW_STATE* draw_state, SAMPLE* sample)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_timelines
 * Desc : This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_timelines(DRAW_STATE* draw_state)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_message
 * Desc : This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_message(DRAW_STATE* draw_state, MESSAGE* message)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: index_state_set_size
 *  desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_state_set_size(DRAW_STATE* draw_state, unsigned int nodes, unsigned int vertices)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: index_state
 *  desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_state(DRAW_STATE* draw_state,STATE* state, unsigned int x, unsigned int y)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: index_transition
 *  desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_transition(DRAW_STATE* draw_state,STATE* state, STATE_TRANSITION* transition,unsigned int from_id,unsigned int to_id)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_marker
 * @desc: This function mostly does nothing for most of the markers that are
 *        passed to it (for the text version).
 *--------------------------------------------------------------------------------*/
void	index_marker(DRAW_STATE* draw_state, unsigned int marker)
{
	unsigned char	level = marker & 0x0000ffff;

	switch ((marker & 0xffff0000))
	{
		case OUTPUT_MARKER_CODE_START:
		case OUTPUT_MARKER_CODE_END:
		case OUTPUT_MARKER_BLOCK_START:
		case OUTPUT_MARKER_BLOCK_END:
		case OUTPUT_MARKER_EMP_START:
		case OUTPUT_MARKER_EMP_END:
		case OUTPUT_MARKER_ASCII_CHAR:
		case OUTPUT_MARKER_LINE_BREAK:
			/* all the above do nothing */
			break;

		case OUTPUT_MARKER_HEADER_START:
			if ((marker & OUTPUT_MARKER_NO_INDEX) == 0)
			{
				g_capture_title = 1;
				level_index_set_level(&draw_state->index,level);
				index_add_section(draw_state);		
			}
			break;

		case OUTPUT_MARKER_HEADER_END:
			g_capture_title = 0;
			break;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_text
 * @desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_text(DRAW_STATE* draw_state, unsigned int index_style, NAME* text)
{
	if (g_capture_title)
	{
		copy_name(text,&((DOC_SECTION*)draw_state->format_state)->section_title);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_section
 * @desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_section( DRAW_STATE* draw_state, unsigned int header_level, NAME* name, unsigned int format, NAME* section_data)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_title
 * @desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_title(DRAW_STATE* draw_state, unsigned int header_level, NAME* title, NAME* name)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_block
 * @desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_block(DRAW_STATE* draw_state, unsigned int format, NAME* block)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_table_start
 * @desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_table_start(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_table_header
 * @desc: This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_table_header(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_table_row
 * Desc : This function does nothing.
 *--------------------------------------------------------------------------------*/
void	index_table_row(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout,TABLE_ROW* table_rows)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: index_table_end
 * @desc: The text output essentially does nothing.
 *--------------------------------------------------------------------------------*/
void	index_table_end(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_index_chapter
 * Desc : This index chapter.
 *--------------------------------------------------------------------------------*/
void	index_index_chapter(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_index_start_sublevel
 * Desc : The start sub-level
 *--------------------------------------------------------------------------------*/
void	index_index_start_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_index_entry
 * Desc : The same level.
 *--------------------------------------------------------------------------------*/
void	index_index_entry(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_index_end_sublevel
 * Desc : end the sub level.
 *--------------------------------------------------------------------------------*/
void	index_index_end_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: index_list_start
 * desc: The index output essentially does nothing.
 *--------------------------------------------------------------------------------*/
void	index_list_item_start(DRAW_STATE* draw_state, unsigned int level, unsigned char marker)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: index_list_numeric_start
 * desc: The index output essentially does nothing.
 *--------------------------------------------------------------------------------*/
void	index_list_numeric_start(DRAW_STATE* draw_state, unsigned int level)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: index_list_end
 * desc: The index output essentially does nothing.
 *--------------------------------------------------------------------------------*/
void	index_list_end(DRAW_STATE* draw_state)
{
}

