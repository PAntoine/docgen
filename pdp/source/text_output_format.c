/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: text_output
 *   @desc: This file holds the text output functions for the default sudo plugin.
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
#include "level_index.h"
#include "error_codes.h"
#include "state_machine.h"
#include "document_generator.h"
#include "text_output_format.h"

#define DEFAULT_PAGE_WIDTH		(120)
#define DEFAULT_COLUMN_SPACING	(2)

/*--------------------------------------------------------------------------------*
 * static structures.
 *--------------------------------------------------------------------------------*/
static unsigned char	text_fmt[] = "text";

OUTPUT_FORMAT	output_formats = 
{
	text_fmt,
	sizeof(text_fmt)-1,
	text_decode_flags,
	text_open,
	text_close,
	text_output_header,
	text_output_footer,
	text_output_raw,
	text_output_sample,
	text_output_timelines,
	text_output_message,
	text_output_state_set_size,
	text_output_state,
	text_output_transition,
	text_output_marker,
	text_output_text,
	text_output_section,
	text_output_title,
	text_output_block,
	text_output_table_start,
	text_output_table_header,
	text_output_table_row,
	text_output_table_end,
	text_output_index_chapter,
	text_output_index_start_sublevel,
	text_output_index_entry,	
	text_output_index_end_sublevel,
	text_output_list_item_start,
	text_output_list_numeric_start,
	text_output_list_end,

	NULL
};

/*--------------------------------------------------------------------------------*
 * String Constants.
 *--------------------------------------------------------------------------------*/
static unsigned char	list_level_string[]	= "                                ";
static unsigned char	chapter_string[]	= "Chapter";

#define	CHAPTER_LENGTH	(sizeof(chapter_string)-1)

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_decode_flags
 * @desc: This function will decode the flags that are local to text functions.
 *--------------------------------------------------------------------------------*/
unsigned int	text_decode_flags(DRAW_STATE* draw_state,INPUT_STATE* input_state, unsigned hash,NAME* value)
{
	unsigned int result = 0;

	/* there are not any flags for text functions */

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_open
 * Desc : This function will open the text file.
 *--------------------------------------------------------------------------------*/
unsigned int	text_open(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned char* name, unsigned int name_length)
{
	unsigned int result = EC_FAILED;
	unsigned int length = name_length + draw_state->path_length;

	if ((length + 4) < MAX_FILENAME)
	{
		memcpy(&draw_state->path[draw_state->path_length],name,name_length);
		draw_state->path[length++] = '.';
		draw_state->path[length++] = 't';
		draw_state->path[length++] = 'x';
		draw_state->path[length++] = 't';
		draw_state->path[length] = '\0';
	
		draw_state->page_width = DEFAULT_PAGE_WIDTH;
		draw_state->margin_width = 4;
		draw_state->global_margin_width = 4;
		draw_state->global_max_constant = 50;

		draw_state->output_buffer = malloc(draw_state->page_width + 1);
		draw_state->output_buffer[draw_state->page_width] = '\n';
	
		if ((draw_state->output_file = open((char*)draw_state->path,O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) != -1)
		{
			/* file successfully opened */
			result = EC_OK;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_close
 * Desc : This function will close the output file.
 *--------------------------------------------------------------------------------*/
void	text_close(DRAW_STATE* draw_state,INPUT_STATE* input_state)
{
	if (draw_state->output_file != -1)
	{
		close(draw_state->output_file);
		free(draw_state->output_buffer);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_header
 * Desc : This function will output the file header.
 *--------------------------------------------------------------------------------*/
void	text_output_header(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	NAME	text = {NULL,0,0,0};

	switch(input_state->state)
	{
		case TYPE_STATE_MACHINE:
			state_machine_allocate((TEXT_STATE_MACHINE**)&draw_state->data.state_machine.data);
			break;

		case TYPE_QUOTE_BLOCK: 
			break;

		case TYPE_LIST:
			level_index_init(&draw_state->list_index);
			break;

		default:
			if ((draw_state->format_flags & OUTPUT_FORMAT_INLINE) == 0)
			{
				write(draw_state->output_file,"\n",1);
			}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_footer
 * Desc : This adds the footer to the output.
 *--------------------------------------------------------------------------------*/
void	text_output_footer(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	NAME	text = {NULL,0,0,0};

	if (input_state->state == TYPE_STATE_MACHINE)
	{
		if (draw_state->data.state_machine.data != NULL)
		{
			state_machine_draw(draw_state->data.state_machine.data,draw_state);
			state_machine_release(draw_state->data.state_machine.data);
			draw_state->data.state_machine.data = NULL;
		}
	}

	else if (input_state->state == TYPE_LIST)
	{
		/* list changes the margin - so reset it */
		draw_state->margin_width = draw_state->global_margin_width;
		level_index_release(&draw_state->list_index);
	}
	
	if ((draw_state->format_flags & OUTPUT_FORMAT_INLINE) == 0)
	{
		text_output_marker(draw_state,OUTPUT_MARKER_PARAGRAPH_BREAK);

		level_index_release(&draw_state->list_index);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_raw
 * Desc : This function will output to the text file the raw unprocessed data
 *        from the file as read in. As in text mode there is no processing of
 *        the MarkDown required, we will just write the data to the file.
 *
 *        But we will clip it to line width.
 *--------------------------------------------------------------------------------*/
    void	text_output_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size)
{
	unsigned int offset = 0;
	unsigned int start_point = 0;

	while (offset < buffer_size)
	{
		if (buffer[offset] == 0x0a || buffer[offset] == 0x0d)
		{
			if (offset - start_point > 1)
			{
				write(draw_state->output_file,"\n",1);
				write(draw_state->output_file,&buffer[start_point],offset-start_point);
			}
			start_point = offset + 1;
		}
		offset++;
	}

	if (start_point < offset)
	{
		write(draw_state->output_file,"\n",1);
		write(draw_state->output_file,&buffer[start_point],offset-start_point);
	}
		
	draw_state->offset = 0;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_output_sample
 * @desc: This function writes a pre-formatted lump of text to the output. It needs
 *        to be clipped to fit the maximum page size.
 *--------------------------------------------------------------------------------*/
void	text_output_sample(DRAW_STATE* draw_state, SAMPLE* sample)
{
	/* text version ignores the header level */
	write(draw_state->output_file,"\n",1);
	write(draw_state->output_file,sample->name.name,sample->name.name_length);
	write(draw_state->output_file,"\n",1);

	/* now output the block */
	write_block_text(draw_state,&sample->sample,OUTPUT_COLUMN_FORMAT_LINE_CLIP);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_timelines
 * Desc : This function will output the list of timelines.
 *--------------------------------------------------------------------------------*/
void	text_output_timelines(DRAW_STATE* draw_state)
{
	unsigned int	offset = 0;
	unsigned int	write_size = 0;
	TIMELINE* 		current = draw_state->data.sequence.start_timeline;
	
	offset = draw_state->data.sequence.column[current->column] - draw_state->data.sequence.window_start;
	write_size = write_size;

	memset(draw_state->output_buffer,' ',offset);

	while (current != draw_state->data.sequence.end_timeline)
	{
		if ((current->flags & FLAG_TAGGED) == FLAG_TAGGED)
		{
			offset = draw_state->data.sequence.column[current->column] - draw_state->data.sequence.window_start;
			draw_state->output_buffer[offset++] = '[';
			memcpy(&draw_state->output_buffer[offset],current->name,current->name_length);
			offset += current->name_length;
			draw_state->output_buffer[offset++] = ']';

			write_size = offset;

			memset(&draw_state->output_buffer[offset],' ',current->group->max_message_length+2);
			offset += current->group->max_message_length;
		}

		current = current->next;
	}
			
	draw_state->output_buffer[write_size++] = '\n';
	write(draw_state->output_file,draw_state->output_buffer,write_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_message
 * Desc : This function will output the message connection for the text output.
 *        This function only draws the messages that originate from the current
 *        timeline. The walking code handles the receiver messages.
 *--------------------------------------------------------------------------------*/
void	text_output_message(DRAW_STATE* draw_state, MESSAGE* message)
{
	unsigned int	end;
	unsigned int	pos;
	unsigned int	count;
	unsigned int	start;
	unsigned int	offset = draw_state->margin_width;
	unsigned char	special = '<';
	unsigned char	start_char = '-';
	unsigned char*	buffer;

	buffer = &draw_state->output_buffer[draw_state->margin_width];

	if (message->receiver == NULL)
	{
		special = '?';
	}
	
	/* clear the line */
	memset(draw_state->output_buffer,' ',draw_state->page_width);

	if (message->sending_timeline != NULL)
	{
		/* put the start within the window, we know it must be inside the window */
		start = draw_state->data.sequence.column[message->sending_timeline->column];

		if (start < draw_state->data.sequence.window_start)
		{
			start = 0;
		}
		else if (start > draw_state->data.sequence.window_end)
		{
			start = draw_state->page_width;
		}
		else
		{
			start -= draw_state->data.sequence.window_start;
		}

		end = start;

		if (message->receiver != NULL)
		{
			/* fit end to the window */
			end = draw_state->data.sequence.column[message->receiver->timeline->column];

			if (end < draw_state->data.sequence.window_start)
			{
				end = 0;
			}
			else if (end > draw_state->data.sequence.window_end)
			{
				end = draw_state->page_width;
			}
			else
			{
				end -= draw_state->data.sequence.window_start;
			}
		}

		/* if it is out of the window at either side skip */
		if (!(end == start && (end == 0 || end >= draw_state->data.sequence.window_end)))
		{
			if (end < start)
			{
				memset(&buffer[end+1],'-',start-end-1);
				buffer[end+1] = '<';
				offset = end + (start - end - message->name_length)/2;
			}
			else if (end > start)
			{
				memset(&buffer[start+1],'-',end-start-1);
				buffer[end-1] = '>';
				offset = start + (end - start - message->name_length)/2;
			}
			else
			{
				buffer[end]   = special;
				offset = end - (message->name_length);
			}

			for (count=draw_state->data.sequence.first_column;count<draw_state->data.sequence.last_column;count++)
			{
				pos = draw_state->data.sequence.column[count] - draw_state->data.sequence.window_start;

				if (buffer[pos] == '-')
					buffer[pos] = '+';
				else if (buffer[pos] == ' ')
					buffer[pos] = '|';
			}

			/* set the start of the window */
			if (draw_state->data.sequence.first_column != 0)
			{
				if (buffer[0] == '-')
					buffer[0] = '+';
				else if (buffer[0] == ' ')
					buffer[0] = '|';
			}

			if (draw_state->data.sequence.last_column != draw_state->data.sequence.num_columns)
			{
				if (buffer[draw_state->data.sequence.window_end] == '-')
					buffer[draw_state->data.sequence.window_end] = '+';
				else if (buffer[draw_state->data.sequence.window_end] == ' ')
					buffer[draw_state->data.sequence.window_end] = '|';
			}


			memcpy(&buffer[offset],message->name,message->name_length);
			offset += message->name_length;
			pos = draw_state->data.sequence.column[count] - draw_state->data.sequence.window_start;
		
			/* now write the line to the file */
			write(draw_state->output_file,draw_state->output_buffer,draw_state->page_width+1);
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: text_output_state_set_size
 *  desc: This function will set the size of the graph to draw.
 *--------------------------------------------------------------------------------*/
void	text_output_state_set_size(DRAW_STATE* draw_state, unsigned int nodes, unsigned int vertices)
{
	state_machine_set_size(draw_state->data.state_machine.data,nodes,vertices);
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: text_output_state
 *  desc: This function will add a state to the state_machine. This function
 *        will add the state and the location of the node into the drawing code.
 *
 *        Note: the x and y are hints for drawing based on a virtual layout.
 *--------------------------------------------------------------------------------*/
void	text_output_state(DRAW_STATE* draw_state,STATE* state, unsigned int x, unsigned int y)
{
	state_machine_add_node(draw_state->data.state_machine.data,state,x,y);
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: text_output_transition
 *  desc: This function will add a transition to the output.
 *--------------------------------------------------------------------------------*/
void	text_output_transition(DRAW_STATE* draw_state,STATE* state, STATE_TRANSITION* transition,unsigned int from_id,unsigned int to_id)
{
	state_machine_add_vertex(draw_state->data.state_machine.data,transition,from_id,to_id);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_output_marker
 * @desc: This function mostly does nothing for most of the markers that are
 *        passed to it (for the text version).
 *--------------------------------------------------------------------------------*/
void	text_output_marker(DRAW_STATE* draw_state, unsigned int marker)
{
	unsigned char array[1];
	unsigned char	level = marker & ~(OUTPUT_MARKER_MASK);

	switch ((marker & OUTPUT_MARKER_MASK))
	{
		case OUTPUT_MARKER_CODE_START:
		case OUTPUT_MARKER_CODE_END:
		case OUTPUT_MARKER_BLOCK_START:
		case OUTPUT_MARKER_BLOCK_END:
		case OUTPUT_MARKER_EMP_START:
		case OUTPUT_MARKER_EMP_END:
		case OUTPUT_MARKER_HEADER_END:
			/* all the above do nothing */
			break;

		case OUTPUT_MARKER_HEADER_START:
			if ((marker & OUTPUT_MARKER_NO_INDEX) == 0)
			{
				level_index_set_level(&draw_state->index,level);
			}
			break;

		case OUTPUT_MARKER_PARAGRAPH_BREAK:
			draw_state->offset = 0;
			write(draw_state->output_file,"\n\n",2);
			break;

		case OUTPUT_MARKER_LINE_BREAK:
			draw_state->offset = 0;
			write(draw_state->output_file,"\n",1);
			break;

		case OUTPUT_MARKER_ASCII_CHAR:
			array[0] = (unsigned char) marker & 0x7f;
			draw_state->offset++;
			write(draw_state->output_file,array,1);
			break;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_output_text
 * @desc: This function will output the text to the output file.
 *--------------------------------------------------------------------------------*/
void	text_output_text(DRAW_STATE* draw_state, unsigned int text_style, NAME* text)
{
	if (text_style & OUTPUT_TEXT_STYLE_L_NEWLINE)
	{
		write(draw_state->output_file,"\n",1);
		draw_state->offset = 0;
	}

	if (text_style & (OUTPUT_TEXT_STYLE_L_SPACE | OUTPUT_TEXT_STYLE_SPACED))
	{
		write(draw_state->output_file," ",1);
		draw_state->offset++;
	}

	if (text_style & OUTPUT_TEXT_STYLE_ITALIC)
	{
		write(draw_state->output_file,"*",1);
		draw_state->offset++;
	}

	if (text != NULL)
	{
		write_text(draw_state,text);
	}

	if (text_style & OUTPUT_TEXT_STYLE_ITALIC)
	{
		write(draw_state->output_file,"*",1);
		draw_state->offset++;
	}

	if (text_style & (OUTPUT_TEXT_STYLE_T_SPACE | OUTPUT_TEXT_STYLE_SPACED))
	{
		write(draw_state->output_file," ",1);
		draw_state->offset++;
	}

	if (text_style & OUTPUT_TEXT_STYLE_T_NEWLINE)
	{
		write(draw_state->output_file,"\n",1);
		draw_state->offset = 0;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_output_section
 * @desc: This function will output a section and the headers. It will output
 *        the section data block to the format specified in the format fields.
 *--------------------------------------------------------------------------------*/
void	text_output_section( DRAW_STATE* draw_state, unsigned int header_level, NAME* name, unsigned int format, NAME* section_data)
{
	/* text version ignores the header level */
	write(draw_state->output_file,"\n",1);
	write(draw_state->output_file,name->name,name->name_length);
	write(draw_state->output_file,"\n",1);

	/* now output the block */
	write_block_text(draw_state,section_data,format);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_output_title
 * @desc: This function will output a title.
 *--------------------------------------------------------------------------------*/
void	text_output_title(DRAW_STATE* draw_state, unsigned int header_level, NAME* title, NAME* name)
{
	/* text functions ignore header level */
	write(draw_state->output_file,"\n",1);
	write(draw_state->output_file,title->name,title->name_length);

	if (name != NULL)
	{
		write(draw_state->output_file,"    ",4);
		write(draw_state->output_file,name->name,name->name_length);
	}
	
	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_output_block
 * @desc: This function will output a block of text.
 *--------------------------------------------------------------------------------*/
void	text_output_block(DRAW_STATE* draw_state, unsigned int format, NAME* block)
{
	write_block_text(draw_state,block,format);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_output_table_start
 * @desc: This function will handle the laying out of the columns. It will try
 *        and fit the table to the size of the line as page_width supplies. It
 *        will maximise the columns sizes, but if the columns don't fit then it
 *        will resize them, it will leave the columns marked as no_truncate if
 *        it can, else it will make the columns all the same size.
 *--------------------------------------------------------------------------------*/
void	text_output_table_start(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout)
{
	unsigned int count = 0;
	unsigned int per_column;
	unsigned int fixed_size = 0;
	unsigned int total_size = 0;
	unsigned int given_size = 0;
	unsigned int total_column_spacing;
	unsigned int adjustable_columns = 0;
	unsigned int variable_width_columns = 0;
	unsigned int num_fixed_width = 0;

	table_layout->valid = 0;

	/* must have some space between columns */
	if (table_layout->column_spacing == 0)
	{
		table_layout->column_spacing = DEFAULT_COLUMN_SPACING;
	}

	/* sanity check the table first - at least one char per column */
	total_column_spacing = table_layout->column_spacing * (table_layout->num_columns - 1);
	
	if ((total_column_spacing + draw_state->margin_width + table_layout->num_columns) <= draw_state->page_width)
	{
		/* ok, the table can possibly fit the page */
		if (table_layout->num_columns > 0)
		{
			/* the table has a possible layout */
			table_layout->valid = 1;

			/* naive layout - trust the setter */
			total_size = draw_state->margin_width;
			
			for (count=0;count < table_layout->num_columns; count++)
			{
				if (table_layout->column[count].width == 0)
				{
					variable_width_columns++;
				}
				else if ((table_layout->column[count].flags & OUTPUT_COLUMN_FORMAT_NO_TRUNCATE) != 0)
				{
					num_fixed_width++;
					fixed_size += table_layout->column[count].width;
				}
				else
				{
					given_size += table_layout->column[count].width;
					adjustable_columns++;
				}

				table_layout->column[count].offset = total_size;
				total_size += table_layout->column[count].width + table_layout->column_spacing;
			}

			/* remove the last column spacing */
			total_size -= table_layout->column_spacing;

			/* fill the variable width columns to pad out the table */
			if (total_size < draw_state->page_width && variable_width_columns > 0)
			{
				per_column = (draw_state->page_width - total_column_spacing - draw_state->margin_width - fixed_size - given_size) /
								(variable_width_columns);
					
				total_size = draw_state->margin_width;
					
				for (count=0; count < table_layout->num_columns; count++)
				{
					if (table_layout->column[count].width == 0)
					{
						table_layout->column[count].width = per_column;
					}
						
					/* now adjust the offset */
					table_layout->column[count].offset = total_size;
					total_size += table_layout->column[count].width + table_layout->column_spacing;
				}
			}
			else if (total_size > draw_state->page_width || variable_width_columns > 0)
			{
				/* ok, naive layout does not work - or we have variable width columns */
				if ((fixed_size + total_column_spacing) < draw_state->page_width && (variable_width_columns || adjustable_columns))
				{
					write(draw_state->output_file,"with\n",5);
					/* ok, the table will fit with the fixed columns */
					per_column = (draw_state->page_width - total_column_spacing - draw_state->margin_width - fixed_size) 
									/ (table_layout->num_columns - num_fixed_width);

					total_size = draw_state->margin_width;

					for (count=0; count < table_layout->num_columns; count++)
					{
						if (table_layout->column[count].width == 0)
						{
							table_layout->column[count].width = per_column;
						}

						/* now adjust the offset */
						table_layout->column[count].offset = total_size;
						total_size += table_layout->column[count].width + table_layout->column_spacing;
					}
				}
				else
				{
					/* Ok, we cannot fit the table columns with the sizes given in the fixed column 
					 * we have to just fix the size of the columns.
					 */
					per_column = (draw_state->page_width - total_column_spacing - draw_state->margin_width) / table_layout->num_columns;
					total_size = draw_state->margin_width;

					for (count=0; count < table_layout->num_columns; count++)
					{
						table_layout->column[count].width = per_column;
						table_layout->column[count].offset = total_size;
						total_size += per_column + table_layout->column_spacing;
					}
				}
			}
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_output_table_header
 * @desc: This function will output the header for the column. As this is the text
 *        version then it will output the titles then underline them on two lines.
 *--------------------------------------------------------------------------------*/
void	text_output_table_header(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row)
{
	unsigned int	count;
	unsigned int	offset;

	if (table_layout->valid)
	{
		memset(draw_state->output_buffer,' ',draw_state->page_width);

		/* ok, we have a valid table */
		write(draw_state->output_file,draw_state->output_buffer,draw_state->margin_width);
		
		for (count=0; count < table_layout->num_columns; count++)
		{
			if (table_row->row[count]->name_length < table_layout->column[count].width)
			{
				/* write the text and pad with spaces */
				write(draw_state->output_file,table_row->row[count]->name,table_row->row[count]->name_length);
				write(draw_state->output_file,draw_state->output_buffer,table_layout->column[count].width - table_row->row[count]->name_length);
			}
			else
			{
				/* write the title clipped */
				write(draw_state->output_file,table_row->row[count]->name,table_layout->column[count].width);
			}

			write(draw_state->output_file,draw_state->output_buffer,table_layout->column_spacing);
		}
		write(draw_state->output_file,"\n",1);
		
		/* now write the underlines */
		for (count=0; count < table_layout->num_columns; count++)
		{
			memset(&draw_state->output_buffer[table_layout->column[count].offset],'-',table_layout->column[count].width);
		}
		write(draw_state->output_file,draw_state->output_buffer,draw_state->page_width);
		write(draw_state->output_file,"\n",1);
	}

}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_table_row
 * Desc : This function will draw a table row.
 *        This expects the third column to be a text column that is word wrapped.
 *--------------------------------------------------------------------------------*/
void	text_output_table_row(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout,TABLE_ROW* table_rows)
{
	unsigned int	column;
	unsigned int	wrapped = 0;
	unsigned int	clipped = 0;
	unsigned int	write_size = 0;
		
	if (table_layout->valid)
	{
		memset(draw_state->output_buffer,' ',draw_state->page_width);

		/* initialise the rows */
		for (column = 0; column < table_layout->num_columns; column++)
		{
			if (table_rows->row[column] != NULL)
			{
				table_rows->remaining[column]	= table_rows->row[column]->name_length;
				table_rows->copied[column]		= 0;
			}
		}
					
		do
		{
			clipped = 0;
		
			write(draw_state->output_file,draw_state->output_buffer,draw_state->margin_width);

			/* write each column */
			for (column = 0; column < table_layout->num_columns; column++)

			{
				if (table_rows->row[column] != NULL && table_rows->row[column]->name)
				{
					if (table_rows->remaining[column] > table_layout->column[column].width)
					{
						write_size = table_layout->column[column].width;

						if ((table_layout->column[column].flags & OUTPUT_COLUMN_FORMAT_WORD_WRAP) != 0)
						{
							write_size = word_wrap(&table_rows->row[column]->name[table_rows->copied[column]],write_size,&wrapped);
						}

						write(draw_state->output_file,&table_rows->row[column]->name[table_rows->copied[column]],write_size);

						/* adjust the counts */
						table_rows->remaining[column] -= write_size + wrapped;
						table_rows->copied[column]    += write_size + wrapped;

						clipped = 1;
					}
					else
					{
						write_size = table_rows->remaining[column];
						write(draw_state->output_file,&table_rows->row[column]->name[table_rows->copied[column]],write_size);
						write(draw_state->output_file,draw_state->output_buffer,table_layout->column[column].width - write_size);

						table_rows->remaining[column] -= write_size;
						table_rows->copied[column]    += write_size;
					}
				}
				else
				{
					write(draw_state->output_file,draw_state->output_buffer,table_layout->column[column].width);
				}
		
				write(draw_state->output_file,draw_state->output_buffer,table_layout->column_spacing);
			}
				
			/* need a new line */
			write(draw_state->output_file,"\n",1);
		}
		while (clipped);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: text_output_table_end
 * @desc: The text output essentially does nothing.
 *--------------------------------------------------------------------------------*/
void	text_output_table_end(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_index_chapter
 * Desc : 
 *--------------------------------------------------------------------------------*/
void	text_output_index_chapter(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_index_start_sublevel
 * Desc : This function will write the sub-level.
 *--------------------------------------------------------------------------------*/
void	text_output_index_start_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
	write_text_index_entry(draw_state,index_item);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_index_entry
 * Desc : This function will write the index entry.
 *--------------------------------------------------------------------------------*/
void	text_output_index_entry(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
	write_text_index_entry(draw_state,index_item);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_index_end_sublevel
 * Desc : This function will write the index entry.
 *--------------------------------------------------------------------------------*/
void	text_output_index_end_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_list_start
 * Desc : This function starts the list items for items that have a symbolic
 *        start.
 *--------------------------------------------------------------------------------*/
void	text_output_list_item_start(DRAW_STATE* draw_state, unsigned int level, unsigned char marker)
{
	NAME	marker_str;
	
	marker_str.name = &marker;
	marker_str.name_length = 1;

	draw_state->margin_width = draw_state->global_margin_width * (level + 2) - 2;
	text_output_text(draw_state,OUTPUT_TEXT_STYLE_NORMAL,&marker_str);
	draw_state->margin_width += 2;

	level_index_set_level(&draw_state->list_index,level);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_list_numeric_start
 * Desc : This handles the list items that have a numeric index.
 *--------------------------------------------------------------------------------*/
void	text_output_list_numeric_start(DRAW_STATE* draw_state, unsigned int level)
{
	NAME			number = {NULL,0,0,0};
	unsigned int	index;
	unsigned char	number_buffer[40];

	level_index_set_level(&draw_state->list_index,level);
	index = level_index_get_index(&draw_state->list_index,level);

	/* convert and output the number */
	number.name = number_buffer;
	number.name_length = IntToAlphaSafe(index,number_buffer,sizeof(number_buffer));
	
	draw_state->margin_width = draw_state->global_margin_width * (level + 2) - number.name_length - 1;

	number_buffer[number.name_length-1] = '.';
	text_output_text(draw_state,OUTPUT_TEXT_STYLE_NORMAL,&number);
	draw_state->margin_width = draw_state->global_margin_width * (level + 2);

}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_list_end
 * Desc : This function will end the list items.
 *--------------------------------------------------------------------------------*/
void	text_output_list_end(DRAW_STATE* draw_state)
{
	text_output_marker(draw_state,OUTPUT_MARKER_LINE_BREAK);
}

