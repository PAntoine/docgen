/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : text_output_functions
 * Desc  : This file holds the function definitions and tables for the generation
 *         of the output for the document generator.
 *
 * Author: pantoine
 * Date  : 07/01/2012 15:17:08
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "error_codes.h"
#include "supported_formats.h"
#include "document_generator.h"

#define MAX_OUTPUT_TEXT_LEGNTH	(2048)
#define DEFAULT_PAGE_WIDTH		(90)			/* A4 - 2cm margins - 10pt text (2.5mm) = 230/2.5 ~= 90 */

/*--------------------------------------------------------------------------------*
 * local defines.
 *--------------------------------------------------------------------------------*/
#define	COMPLEX_LOOKING		(0)
#define	COMPLEX_OTHER		(1)
#define	COMPLEX_ARRAY		(2)

/*--------------------------------------------------------------------------------*
 * Structure for the tables.
 *--------------------------------------------------------------------------------*/
#define MAX_TABLE_SIZE	(5)

typedef struct
{
	unsigned int	flags;
	unsigned int	offset;
	unsigned int	width;
} COLUMN;

typedef struct
{
	unsigned int	num_columns;
	COLUMN			column[MAX_TABLE_SIZE];
} TABLE_LAYOUT;

typedef struct
{
	NAME*			row[MAX_TABLE_SIZE];
	unsigned int	copied[MAX_TABLE_SIZE];
	unsigned int	remaining[MAX_TABLE_SIZE];
} TABLE_ROW;

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: handle_multiline
 * @desc: This function will handle the mutliline code case.
 *--------------------------------------------------------------------------------*/
unsigned int	handle_multiline(	DRAW_STATE* 	draw_state, 
									NAME* 			text_block, 
									unsigned int	flags,
									unsigned int	text_block_copied,
									unsigned int	text_block_offset,
									unsigned int	text_block_remaining)
{
	unsigned int clip_point = 0;

	/* multiline, need to clip to the page width, but skip the end of the page width to the next newline */
	while (	clip_point < draw_state->page_width &&
			(text_block->name[text_block_copied + clip_point] != 0x0a && text_block->name[text_block_copied + clip_point] != 0x0d))
	{
		clip_point++;
	}

	if (clip_point > 0 && (text_block->name[text_block_copied + clip_point] == 0x0a || text_block->name[text_block_copied + clip_point] == 0x0d))
	{
		/* ok, lets backup one */
		clip_point--;
	}

	/* copy the contents for line */
	memcpy(&draw_state->output_buffer[text_block_offset],&text_block->name[text_block_copied],clip_point);
	memset(&draw_state->output_buffer[text_block_offset+clip_point],' ',draw_state->page_width - clip_point);

	/* now, find the end of the line */
	while (	clip_point < text_block_remaining &&
			(text_block->name[text_block_copied + clip_point] != 0x0a && text_block->name[text_block_copied + clip_point] != 0x0d))
	{
		clip_point++;
	}

	/* windows/mac line ending? */
	if ((text_block->name[text_block_copied + clip_point] == 0x0a || text_block->name[text_block_copied + clip_point] == 0x0d))
	{
		/* ok, lets backup one */
		clip_point++;
	}


	return clip_point;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: handle_wrapping
 * @desc: This function will handle the wrapping of the text.
 *--------------------------------------------------------------------------------*/
unsigned int	handle_wrapping(	DRAW_STATE* 	draw_state, 
									NAME* 			text_block, 
									unsigned int	flags,
									unsigned int	text_block_copied,
									unsigned int	text_block_offset,
									unsigned int	text_block_remaining)
{
	/* as the brief is a text description then word wrapping should be observed */
	unsigned int wrap_point = draw_state->page_width;

	if ((flags & FORMAT_WORD_WRAP) != 0)
	{
		/* first check for word wrapping */
		while(wrap_point > 0 && text_block->name[text_block_copied + wrap_point - 1] != 0x20)
		{
			wrap_point--;
		}

		/* if no place to word wrap then clip it hard */
		if (wrap_point == 0)
		{
			wrap_point = draw_state->page_width;
		}
	}

	memcpy(&draw_state->output_buffer[text_block_offset],&text_block->name[text_block_copied],wrap_point);

	return wrap_point;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: write_complex
 * @desc: This function will handle the complex wrapping of the text. It will
 *        try and work out what the text type is and then wrap it accordingly.
 *--------------------------------------------------------------------------------*/
unsigned int	write_complex (	DRAW_STATE* draw_state, NAME* text_block, unsigned int margin )
{
	unsigned int	size = 0;
	unsigned int	state = COMPLEX_LOOKING;
	unsigned int	level = 0;
	unsigned int	group = 0;
	unsigned int	result = 0;
	unsigned int	instring = 0;
	unsigned int	last_comma = 0;
	unsigned int	out_pos  = margin;
	unsigned int	position = 0;
	
	/*clear the margin */
	memset(draw_state->output_buffer,' ',margin);

	/* white space remove */
	while (text_block->name[position] == 0x20 || text_block->name[position] < 0x0f)
	{
		position++;
	}

	while (position < text_block->name_length)
	{
		switch(state)
		{
			case COMPLEX_LOOKING:
				if (text_block->name[position] == '{')
				{
					do
					{
						draw_state->output_buffer[out_pos++] = text_block->name[position++];

						/* white space remove */
						while (text_block->name[position] == 0x20 || text_block->name[position] < 0x0f)
						{
							position++;
						}

						level++;
					}
					while (text_block->name[position] == '{');

					group = level - 1;
					state = COMPLEX_ARRAY;
				}
				else
				{
					state = COMPLEX_OTHER;
				}
				break;

			case COMPLEX_OTHER:
				/* need to clip the data into the line width */
				position = text_block->name_length;
				break;

			case COMPLEX_ARRAY:

				while (position < text_block->name_length)
				{
					if (text_block->name[position] == 0)
					{
						position++;
					}
					else if (instring)
					{
						/* skip the escaped end " */
						if ((text_block->name[position] == '\\') && (text_block->name[position+1] == '"'))
						{
							draw_state->output_buffer[out_pos++] = text_block->name[position++];
						}
						else if (text_block->name[position] == '"')
						{
							instring = 0;
						}

						draw_state->output_buffer[out_pos++] = text_block->name[position++];
					}
					else
					{
						switch(text_block->name[position])
						{
							case '{':
								/* white space remove */
								while (text_block->name[position] == 0x20 || text_block->name[position] < 0x0f)
								{
									position++;
								}

								level++;
								size = 0;
								draw_state->output_buffer[out_pos++] = text_block->name[position++];
								break;

							case '"':
								instring = 1;
								draw_state->output_buffer[out_pos++] = text_block->name[position++];
								break;

							case ',':
								/* need to reverse remove whitespace from the output */
								if (out_pos > 0)
								{
									if ((draw_state->output_buffer[out_pos-1] == 0x20 || draw_state->output_buffer[out_pos-1] < 0x0f))
									{
										while (	out_pos > 0 && 
												(draw_state->output_buffer[out_pos-1] == 0x20 || draw_state->output_buffer[out_pos-1] < 0x0f))
										{
											out_pos--;
										}
									}
								}
								draw_state->output_buffer[out_pos++] = text_block->name[position++];
								last_comma = out_pos;

								if (level == group)
								{
									/* write it to the output file */
									draw_state->output_buffer[out_pos++] = '\n';
									write(draw_state->output_file,draw_state->output_buffer,out_pos);
									memset(draw_state->output_buffer,' ',margin+level);
									out_pos = margin+level;
									last_comma = 0;
								}

								/* now check for post comma whitespace */
								while (	position < text_block->name_length && 
										(text_block->name[position] == 0x20 || text_block->name[position] < 0x0f))
								{
									position++;
								}

								break;

							case '}':
								level--;
								if (level < group)
								{
									draw_state->output_buffer[out_pos++] = '\n';
									write(draw_state->output_file,draw_state->output_buffer,out_pos);
									memset(draw_state->output_buffer,' ',margin+level);
									out_pos = margin+level;
									last_comma = 0;
								}

								draw_state->output_buffer[out_pos++] = text_block->name[position++];
								while (	position < text_block->name_length && 
										(text_block->name[position] == 0x20 || text_block->name[position] < 0x0f))
								{
									position++;
								}
								break;

							default:
								size++;
								draw_state->output_buffer[out_pos++] = text_block->name[position++];
						}
					}

					if (out_pos == draw_state->page_width)
					{
						if (last_comma > 0)
						{
							unsigned char temp = draw_state->output_buffer[last_comma];
							draw_state->output_buffer[last_comma] = '\n';
							write(draw_state->output_file,draw_state->output_buffer,last_comma+1);
							
							draw_state->output_buffer[last_comma] = temp;
							memcpy(&draw_state->output_buffer[margin + level],&draw_state->output_buffer[last_comma],(out_pos - last_comma));
							out_pos = margin + level + (out_pos - last_comma);
						}
						else
						{
							draw_state->output_buffer[out_pos] = '\n';
							write(draw_state->output_file,draw_state->output_buffer,out_pos+1);

							out_pos = margin+level;
						}
						memset(draw_state->output_buffer,' ',margin+level);
						last_comma = 0;
					}
				}

				draw_state->output_buffer[out_pos] = '\n';
				write(draw_state->output_file,draw_state->output_buffer,out_pos+1);
				break;
		}
	}

	return result;
} 

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: write_block_text
 * @desc: This function will output a block of text.
 *        it will use the flags passed in to format the text.
 *--------------------------------------------------------------------------------*/
static void	write_block_text(DRAW_STATE* draw_state, NAME* text_block, unsigned int flags)
{
	unsigned int clipped;
	unsigned int text_used = 0;
	unsigned int text_block_copied = 0;
	unsigned int text_block_offset = 0;
	unsigned int text_block_remaining = text_block->name_length;
	
	do
	{
		clipped = 0;
	
		if (text_block_remaining > draw_state->page_width)
		{
			if ((flags & FORMAT_MULTILINE) != 0)
			{
				text_used = handle_multiline(draw_state,text_block,flags,text_block_copied,text_block_offset,text_block_remaining);
			}
			else
			{
				text_used = handle_wrapping(draw_state,text_block,flags,text_block_copied,text_block_offset,text_block_remaining);
			}

			text_block_copied 		+= text_used;
			text_block_remaining	-= text_used;
			clipped = 1;
		}
		else
		{
			memcpy(&draw_state->output_buffer[text_block_offset],&text_block->name[text_block_copied],text_block_remaining);
			memset(&draw_state->output_buffer[text_block_offset+text_block_remaining],' ',draw_state->page_width - text_block_remaining);
			text_block_remaining = 0;
		}

		write(draw_state->output_file,draw_state->output_buffer,draw_state->page_width+1);
	}
	while (clipped && ((flags & (FORMAT_WRAP|FORMAT_MULTILINE)) != 0));
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_open
 * Desc : This function will open the text file.
 *--------------------------------------------------------------------------------*/
unsigned int	text_open(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length)
{
	unsigned int result = EC_FAILED;
	unsigned int length = name_length + draw_state->path_length;

	if ((length + 4) < FILENAME_MAX)
	{
		memcpy(&draw_state->path[draw_state->path_length],name,name_length);
		draw_state->path[length++] = '.';
		draw_state->path[length++] = 't';
		draw_state->path[length++] = 'x';
		draw_state->path[length++] = 't';
		draw_state->path[length] = '\0';
	
		draw_state->page_width = DEFAULT_PAGE_WIDTH;

		draw_state->output_buffer = malloc(draw_state->page_width + 1);

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
void	text_close(DRAW_STATE* draw_state)
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
void	text_output_header(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length)
{
	if (name_length == 0)
	{
		name = (unsigned char*) "default";
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_footer
 * Desc : This adds the footer to the output.
 *--------------------------------------------------------------------------------*/
void	text_output_footer(DRAW_STATE* draw_state)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_raw
 * Desc : This function will output to the text file the raw unprocessed data
 *        from the file as read in. As in text mode there is no processing of
 *        the MarkDown required, we will just write the data to the file.
 *--------------------------------------------------------------------------------*/
void	text_output_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size)
{
	write(draw_state->output_file,buffer,buffer_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_sample
 * Desc : This function will output to the text file the sample data.
 *--------------------------------------------------------------------------------*/
void	text_output_sample(DRAW_STATE* draw_state, SAMPLE* sample)
{
	write_block_text(draw_state,&sample->sample,FORMAT_MULTILINE);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_timelines
 * Desc : This function will output the list of timelines.
 *--------------------------------------------------------------------------------*/
void	text_output_timelines(DRAW_STATE* draw_state, TIMELINE* timeline)
{
	unsigned int	offset = 0;
	unsigned int	write_size = 0;
	unsigned char	buffer[1024];
	TIMELINE* 		current = timeline;
	
	offset = 4;
	write_size = 4;

	buffer[0] = ' ';
	buffer[1] = ' ';
	buffer[2] = ' ';
	buffer[3] = ' ';

	while (current != NULL)
	{
		if ((current->flags & FLAG_TAGGED) == FLAG_TAGGED)
		{
			buffer[offset++] = '[';
			memcpy(&buffer[offset],current->name,current->name_length);
			offset += current->name_length;
			buffer[offset++] = ']';

			write_size = offset;

			memset(&buffer[offset],' ',current->group->max_message_length);
			offset += current->group->max_message_length;
		}

		current = current->next;
	}
			
	buffer[write_size++] = '\n';
	write(draw_state->output_file,buffer,write_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_message
 * Desc : This function will output the message connection for the text output.
 *--------------------------------------------------------------------------------*/
void	text_output_message(DRAW_STATE* draw_state, MESSAGE* message)
{
	unsigned int	end;
	unsigned int	count;
	unsigned int	start;
	unsigned int	offset = 4;
	unsigned char	special = '<';
	unsigned char	out_buffer[MAX_OUTPUT_TEXT_LEGNTH];
	unsigned char*	buffer = &out_buffer[4];

	if (message->receiver == NULL)
	{
		special = '?';
	}

	/* clear the line */
	memset(out_buffer,' ',draw_state->data.sequence.column[draw_state->data.sequence.num_columns]);

	/* now fix the end point */
	out_buffer[draw_state->data.sequence.column[draw_state->data.sequence.num_columns-1]+1+4] = '\n';

	if (message->sending_timeline != NULL)
	{
		start = draw_state->data.sequence.column[message->sending_timeline->column];
		end = start;

		if (message->receiver != NULL)
		{
			end = draw_state->data.sequence.column[message->receiver->timeline->column];
		}

		if (end < start)
		{
			memset(&buffer[end+1],'-',start-end-1);
			buffer[end+1] = '<';
			offset = end + (start - end - message->name_length)/2;
		}
		else if ((end-start) > 0)
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
	
		for (count=0;count<draw_state->data.sequence.num_columns;count++)
		{

			if (buffer[draw_state->data.sequence.column[count]] == '-')
				buffer[draw_state->data.sequence.column[count]] = '+';
			else if (buffer[draw_state->data.sequence.column[count]] == ' ')
				buffer[draw_state->data.sequence.column[count]] = '|';
		}

		memcpy(&buffer[offset],message->name,message->name_length);
		offset += message->name_length;
	}

	write(draw_state->output_file,out_buffer,draw_state->data.sequence.column[draw_state->data.sequence.num_columns-1]+2+4);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_states
 * Desc : This function outputs the state list, which we do not need to do for
 *        the text version.
 *--------------------------------------------------------------------------------*/
void	text_output_states(DRAW_STATE* draw_state, STATE* list)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : test_output_start_state
 * Desc : This function starts the output of the state.
 *--------------------------------------------------------------------------------*/
void	text_output_start_state(DRAW_STATE* draw_state,STATE* state)
{
	if (state->name_length > 0)
	{
		write(draw_state->output_file,"    ",4);
		write(draw_state->output_file,state->name,state->name_length);
		write(draw_state->output_file,":\n",2);
	}
	else
	{
		write(draw_state->output_file,"    <unknown>:\n",sizeof("    <unknown>:\n")-1);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_transition
 * Desc : This function will output the transition details.
 *--------------------------------------------------------------------------------*/
void	text_output_transition(DRAW_STATE* draw_state,STATE* state, STATE_TRANSITION* transition)
{
	TRIGGERS*			current_triggers;

	write(draw_state->output_file,"        -> ",sizeof("        -> ")-1);
	write(draw_state->output_file,transition->next_state->name,transition->next_state->name_length);

	if (transition->trigger != NULL)
	{
		write(draw_state->output_file," when ",sizeof(" when ")-1);

		if (state->group != transition->trigger->group)
		{
			write(draw_state->output_file,transition->trigger->group->name,transition->trigger->group->name_length);
			write(draw_state->output_file,":",1);
		}

		write(draw_state->output_file,transition->trigger->name,transition->trigger->name_length);
	}
	else if (transition->condition != NULL)
	{
		write(draw_state->output_file," when '",sizeof(" when '")-1);
		write(draw_state->output_file,transition->condition,transition->condition_length);
	}

	if (transition->triggers != NULL)
	{
		write(draw_state->output_file," triggering ",sizeof(" triggering ")-1);

		current_triggers = transition->triggers;

		do
		{
			if (state->group != current_triggers->trigger->group)
			{
				write(draw_state->output_file,current_triggers->trigger->group->name,current_triggers->trigger->group->name_length);
				write(draw_state->output_file,":",1);
			}

			write(draw_state->output_file,current_triggers->trigger->name,current_triggers->trigger->name_length);

			if (current_triggers->next != NULL)
			{
				write(draw_state->output_file,",",1);
			}
			current_triggers = current_triggers->next;
		} 
		while (current_triggers != NULL);
	}

	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : test_output_end_state
 * Desc : This function does nothing for the text format as the state does not
 *        need ending.
 *--------------------------------------------------------------------------------*/
void	text_output_end_state(DRAW_STATE* draw_state,STATE* state)
{

}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_api_returns_function
 * Desc : 
 *--------------------------------------------------------------------------------*/
void	text_output_api_returns_function(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	API_RETURNS*	current_returns = function->returns_list;

	write(draw_state->output_file,"returns:\n",sizeof("returns:\n")-1);

	while (current_returns != NULL)
	{
		write(draw_state->output_file,current_returns->value.name,current_returns->value.name_length);
		write(draw_state->output_file," ",1);
		write(draw_state->output_file,current_returns->brief.name,current_returns->brief.name_length);
		write(draw_state->output_file,"\n",1);

		current_returns = current_returns->next;
	}
	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_api_action_function
 * Desc : This function will output the formatted action.
 *--------------------------------------------------------------------------------*/
void	text_output_api_action_function(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	write(draw_state->output_file,"action:\n",sizeof("action:\n")-1);

	if (function->action.name_length > 0)
	{
		write(draw_state->output_file,function->action.name,function->action.name_length);
		write(draw_state->output_file,"\n",1);
	}
	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_draw_table_row
 * Desc : This function will draw a table row.
 *        This expects the third column to be a text column that is word wrapped.
 *--------------------------------------------------------------------------------*/
void	text_draw_table_row(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout,TABLE_ROW* table_rows)
{
	unsigned int	column;
	unsigned int	clipped = 0;
	
	for (column = 0; column < table_layout->num_columns; column++)
	{
		table_rows->remaining[column]	= table_rows->row[column]->name_length;
		table_rows->copied[column]		= 0;
	}
	
	do
	{
		clipped = 0;
	
		for (column = 0; column < table_layout->num_columns; column++)
		{
			if (table_rows->remaining[column] > table_layout->column[column].width)
			{
				/* handle the word wrapped columns */
				if ((table_layout->column[column].flags & FORMAT_WRAP) != 0)
				{
					unsigned int wrap_point = table_layout->column[column].width;
				
					/* does it need word wrapping */
					if ((table_layout->column[column].flags & FORMAT_WORD_WRAP) != 0)
					{
						/* first check for word wrapping */
						while(wrap_point > 0 && table_rows->row[column]->name[table_rows->copied[column] + wrap_point - 1] != 0x20)
						{
							wrap_point--;
						}

						/* if no place to word wrap then clip it hard */
						if (wrap_point == 0)
						{
							wrap_point = table_layout->column[column].width;
						}
					}

					/* copy to the output */
					memcpy(	&draw_state->output_buffer[table_layout->column[column].offset],
							&table_rows->row[column]->name[table_rows->copied[column]],
							wrap_point);
				
					memset(	&draw_state->output_buffer[table_layout->column[column].offset+wrap_point],' ',
							table_layout->column[column].width - wrap_point);

					table_rows->copied[column] 		+= wrap_point;
					table_rows->remaining[column]	-= wrap_point;
					clipped = 1;
				}
				else
				{
					/* default is to hard clip to width */
					memcpy(	&draw_state->output_buffer[table_layout->column[column].offset],
							&table_rows->row[column]->name[table_rows->copied[column]],
							table_layout->column[column].width);

					table_rows->copied[column] 		+= table_layout->column[column].width;
					table_rows->remaining[column]	-= table_layout->column[column].width;
					clipped = 1;
				}
			}
			else
			{
				/* column fits */
				memcpy(	&draw_state->output_buffer[table_layout->column[column].offset],
						&table_rows->row[column]->name[table_rows->copied[column]],
						table_rows->remaining[column]);
				
				memset(	&draw_state->output_buffer[table_layout->column[column].offset+table_rows->remaining[column]],' ',
						table_layout->column[column].width - table_rows->remaining[column]);
				
				table_rows->remaining[column] = 0;
			}
		}

		write(draw_state->output_file,draw_state->output_buffer,draw_state->page_width+1);
	}
	while (clipped);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_api_parameters_function
 * Desc : This function will output the parameter table.
 *--------------------------------------------------------------------------------*/
void	text_output_api_parameters_function(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	unsigned int	max_length;
	TABLE_ROW		table_row;
	TABLE_LAYOUT	table_layout;
	API_PARAMETER*	current_parameter;

	/* ok. where do the columns start */
	table_layout.num_columns = 3;
	table_layout.column[0].flags = 0;
	table_layout.column[1].flags = 0;
	table_layout.column[2].flags = (FORMAT_WRAP|FORMAT_WORD_WRAP);

	table_layout.column[0].width = function->max_param_name_length;
	table_layout.column[1].width = function->max_param_type_length;

	if ((table_layout.column[0].width + table_layout.column[1].width + 15 + 12) > draw_state->page_width) 
	{
		/* columns too wide, need to clip them. A simple ratio of 20/20/60 */
		unsigned int ten_percent = (draw_state->page_width - 12) / 10;
		table_layout.column[0].width = ten_percent * 2;
		table_layout.column[1].width = ten_percent * 2;
	}

	table_layout.column[0].offset = 4;
	table_layout.column[1].offset = table_layout.column[0].offset + table_layout.column[0].width + 4;
	table_layout.column[2].offset = table_layout.column[1].offset + table_layout.column[1].width + 4;
	table_layout.column[2].width  = draw_state->page_width - table_layout.column[2].offset;

	/* Ok, write the title */
	memset(draw_state->output_buffer,' ',table_layout.column[2].width);
	write(draw_state->output_file,"    Name",8);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[0].width - 8 + 4);
	write(draw_state->output_file,"    Type",8);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[1].width - 8 + 4);
	write(draw_state->output_file,"    Description",15);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[2].width - 15 + 4);
	write(draw_state->output_file,"\n",1);

	/* now write the underline */
	memset(draw_state->output_buffer,'-',table_layout.column[2].width);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[0].width);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[1].width);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[2].width);
	write(draw_state->output_file,"\n",1);


	/* clear the draw_state->output_buffer for the write */
	memset(draw_state->output_buffer,' ',draw_state->page_width);
	draw_state->output_buffer[draw_state->page_width] = '\n';

	current_parameter = function->parameter_list;

	while (current_parameter != NULL)
	{
		table_row.row[0] = &current_parameter->name;
		table_row.row[1] = &current_parameter->type;
		table_row.row[2] = &current_parameter->brief;

		text_draw_table_row(draw_state,&table_layout,&table_row);
	
		current_parameter = current_parameter->next;
	}

	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_api_prototype_function
 * Desc : This function will output the prototype for the function.
 *--------------------------------------------------------------------------------*/
void	text_output_api_prototype_function(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	unsigned int	space_length;
	unsigned char	spaces[200];
	API_PARAMETER*	current_parameter;

	write(draw_state->output_file,function->return_type.name,function->return_type.name_length);
	write(draw_state->output_file," ",1);
	write(draw_state->output_file,function->name.name,function->name.name_length);
	write(draw_state->output_file,"( ",2);

	current_parameter = function->parameter_list;

	space_length = function->return_type.name_length+3+function->name.name_length;

	memset(spaces,' ',space_length);

	while (current_parameter != NULL)
	{
		write(draw_state->output_file,current_parameter->type.name,current_parameter->type.name_length);
		write(draw_state->output_file,spaces,function->max_param_type_length - current_parameter->type.name_length + 1);
		write(draw_state->output_file,current_parameter->name.name,current_parameter->name.name_length);

		if (current_parameter->next != NULL)
		{
			write(draw_state->output_file,",\n",sizeof(",\n")-1);
			write(draw_state->output_file,spaces,space_length);
		}

		current_parameter = current_parameter->next;
	}

	write(draw_state->output_file,")\n\n",sizeof(")\n\n")-1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_api_description_function
 * Desc : This function will output the description.
 *--------------------------------------------------------------------------------*/
void	text_output_api_description_function(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	write(draw_state->output_file,"description:\n",sizeof("description:\n")-1);

	if (function->description.name_length > 0)
	{
		write_block_text(draw_state,&function->description,(FORMAT_WRAP|FORMAT_WORD_WRAP));
		write(draw_state->output_file,"\n",1);
	}
	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_api_name_function
 * Desc : This function will output the function name.
 *--------------------------------------------------------------------------------*/
void	text_output_api_name_function(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	write(draw_state->output_file,"function name: ",sizeof("function name: ")-1);
	write(draw_state->output_file,function->name.name,function->name.name_length);
	write(draw_state->output_file,"\n\n",2);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_type_name_function
 * Desc : This function will output the type name.
 *--------------------------------------------------------------------------------*/
void	text_output_type_name_function(DRAW_STATE* draw_state, API_TYPE* type)
{
	write(draw_state->output_file,"type name: ",sizeof("type name: ")-1);
	write(draw_state->output_file,type->name.name,type->name.name_length);
	write(draw_state->output_file,"\n\n",2);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_type_description_function
 * Desc : This function will output the type description.
 *--------------------------------------------------------------------------------*/
void	text_output_type_description_function(DRAW_STATE* draw_state, API_TYPE* type)
{
	write(draw_state->output_file,"description:\n",sizeof("description:\n")-1);

	if (type->description.name_length > 0)
	{
		write_block_text(draw_state,&type->description,(FORMAT_WRAP|FORMAT_WORD_WRAP));
		write(draw_state->output_file,"\n",1);
	}
	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_type_records_function
 * Desc : This function will output the records that make up the type.
 *--------------------------------------------------------------------------------*/
void	text_output_type_records_function(DRAW_STATE* draw_state, API_TYPE* type)
{
	TABLE_ROW			table_row;
	TABLE_LAYOUT		table_layout;
	API_TYPE_RECORD*	current_record;

	/* ok. where do the columns start */
	table_layout.num_columns = 3;
	table_layout.column[0].flags = 0;
	table_layout.column[1].flags = 0;
	table_layout.column[2].flags = (FORMAT_WRAP|FORMAT_WORD_WRAP);

	/* ok. where do the columns start */
	table_layout.column[0].offset = 4;
	
	table_layout.column[0].width  = type->max_name_value_length;
	table_layout.column[1].width  = type->max_type_item_length;
	
	if ((table_layout.column[0].width + table_layout.column[1].width + 15 + 12) > draw_state->page_width) 
	{
		/* columns too wide, need to clip them. A simple ratio of 20/20/60 */
		unsigned int ten_percent = (draw_state->page_width - 12) / 10;
		table_layout.column[0].width = ten_percent * 3;
		table_layout.column[1].width = ten_percent * 3;
	}

	table_layout.column[1].offset = 4;
	table_layout.column[0].offset = table_layout.column[1].offset + table_layout.column[1].width + 4;
	table_layout.column[2].offset = table_layout.column[0].offset + table_layout.column[0].width + 4;
	table_layout.column[2].width  = draw_state->page_width - table_layout.column[2].offset;

	/* Ok, write the title */
	memset(draw_state->output_buffer,' ',table_layout.column[2].width);
	write(draw_state->output_file,"    Name",8);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[1].width - 8 + 4);
	write(draw_state->output_file,"    Type",8);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[0].width - 8 + 4);
	write(draw_state->output_file,"    Description",15);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[2].width - 15 + 4);
	write(draw_state->output_file,"\n",1);

	/* now write the underline */
	memset(draw_state->output_buffer,'-',table_layout.column[2].width);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[1].width);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[0].width);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[2].width);
	write(draw_state->output_file,"\n",1);

	/* clear the draw_state->output_buffer for the write */
	memset(draw_state->output_buffer,' ',draw_state->page_width);
	draw_state->output_buffer[draw_state->page_width] = '\n';

	current_record = type->record_list;

	while (current_record != NULL)
	{
		if (current_record->name_value.name_length > 0 && current_record->type_item.name_length > 0)
		{
			table_row.row[0] = &current_record->name_value;
			table_row.row[1] = &current_record->type_item;
			table_row.row[2] = &current_record->brief;

			text_draw_table_row(draw_state,&table_layout,&table_row);
		}
		current_record = current_record->next;
	}
	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_constant_name_function
 * Desc : This function will output the constant name.
 *--------------------------------------------------------------------------------*/
void	text_output_constant_name_function(DRAW_STATE* draw_state, API_CONSTANTS* constants)
{
	write(draw_state->output_file,"constant name: ",sizeof("constant name: ")-1);
	write(draw_state->output_file,constants->name.name,constants->name.name_length);
	write(draw_state->output_file,"\n\n",2);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_constants_description_function
 * Desc : This function will output the constant description.
 *--------------------------------------------------------------------------------*/
void	text_output_constants_description_function(DRAW_STATE* draw_state, API_CONSTANTS* constants)
{
	write(draw_state->output_file,"description:\n",sizeof("description:\n")-1);

	if (constants->description.name_length > 0)
	{
		write_block_text(draw_state,&constants->description,(FORMAT_WRAP|FORMAT_WORD_WRAP));
		write(draw_state->output_file,"\n",1);
	}
	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_constants_records_function
 * Desc : This function will output the records that make up the constant.
 *--------------------------------------------------------------------------------*/
void	text_output_constants_records_function(DRAW_STATE* draw_state, API_CONSTANTS* constants)
{
	TABLE_ROW		table_row;
	TABLE_LAYOUT	table_layout;
	API_CONSTANT*	current_constant;

	/* ok. where do the columns start */
	table_layout.num_columns = 3;
	table_layout.column[0].flags = 0;
	table_layout.column[1].flags = 0;
	table_layout.column[2].flags = (FORMAT_WRAP|FORMAT_WORD_WRAP);
	
	/* ok, lets start with the min name length */
	current_constant = constants->constant_list;

	table_layout.column[0].width = 0;
	table_layout.column[1].width = 0;
	table_layout.column[2].width = 0;

	while (current_constant != NULL)
	{
		/* does it fit? */
		if ((current_constant->name.name_length + current_constant->value.name_length + 15 + 12) < draw_state->page_width) 
		{
			/* if it fits, then it defines the name size, if its bigger */
			if (table_layout.column[0].width < current_constant->name.name_length)
			{
				table_layout.column[0].width = current_constant->name.name_length;
			}

			if (table_layout.column[1].width < current_constant->value.name_length)
			{
				table_layout.column[1].width = current_constant->value.name_length;
			}
		}
		current_constant = current_constant->next;
	}
	
	if ((table_layout.column[0].width + table_layout.column[1].width + 15 + 12) > draw_state->page_width) 
	{
		/* columns too wide, need to clip the value column to max size that we can fit */
		table_layout.column[1].width = draw_state->page_width - (table_layout.column[0].width + 15 + 12);
	}

	/* set out the columns */
	table_layout.column[0].offset = 4;
	table_layout.column[1].offset = table_layout.column[0].offset + table_layout.column[0].width + 4;
	table_layout.column[2].offset = table_layout.column[1].offset + table_layout.column[1].width + 4;
	table_layout.column[2].width  = draw_state->page_width - table_layout.column[2].offset;

	/* Ok, write the title */
	memset(draw_state->output_buffer,' ',draw_state->page_width);
	write(draw_state->output_file,"    Name",8);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[0].width - 8 + 4);
	write(draw_state->output_file,"    constant",12);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[1].width - 12 + 4);
	write(draw_state->output_file,"    Description",15);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[2].width - 15 + 4);
	write(draw_state->output_file,"\n",1);

	/* now write the underline */
	memset(draw_state->output_buffer,'-',draw_state->page_width);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[0].width);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[1].width);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,draw_state->output_buffer,table_layout.column[2].width);
	write(draw_state->output_file,"\n",1);

	/* clear the draw_state->output_buffer for the write */
	memset(draw_state->output_buffer,' ',draw_state->page_width);
	draw_state->output_buffer[draw_state->page_width] = '\n';

	/* do the items that fit in the table */
	current_constant = constants->constant_list;

	while (current_constant != NULL)
	{
		/* large constants need to handled differently */
		if (current_constant->value.name_length < table_layout.column[1].width )
		{
			/* don't print the special fields in the table - only the actual records */
			if (current_constant->name.name_length > 0 && current_constant->value.name_length > 0)
			{
				table_row.row[0] = &current_constant->name;
				table_row.row[1] = &current_constant->value;
				table_row.row[2] = &current_constant->brief;

				text_draw_table_row(draw_state,&table_layout,&table_row);
			}
		}

		current_constant = current_constant->next;
	}

	/* now do this items that do not fit in a table format */
	table_layout.num_columns = 2;
	table_layout.column[0].flags = 0;
	table_layout.column[1].flags = (FORMAT_WRAP|FORMAT_WORD_WRAP);
	
	if (constants->max_name_length < 32)
	{
		table_layout.column[0].width = constants->max_name_length;
		table_layout.column[1].width = draw_state->page_width - constants->max_name_length - 4 - 4;
	}
	else
	{
		table_layout.column[0].width = 32;
		table_layout.column[1].width = draw_state->page_width - 32 - 4 - 4;
	}
	
	current_constant = constants->constant_list;

	while (current_constant != NULL)
	{
		if (current_constant->value.name_length >= table_layout.column[1].width )
		{
			/* don't print the special fields in the table - only the actual records */
			if (current_constant->name.name_length > 0 && current_constant->value.name_length > 0)
			{
				/* only interested in the long constants */
				memset(draw_state->output_buffer,' ',draw_state->page_width);
				write(draw_state->output_file,"\n",1);
				
				write(draw_state->output_file,"    Name",8);
				write(draw_state->output_file,draw_state->output_buffer,table_layout.column[0].width - 8 + 4);
				write(draw_state->output_file,"    Description",15);
				write(draw_state->output_file,draw_state->output_buffer,table_layout.column[1].width - 15 + 4);
				write(draw_state->output_file,"\n",1);

				/* now write the underline */
				memset(draw_state->output_buffer,'-',draw_state->page_width);
				write(draw_state->output_file,"    ",4);
				write(draw_state->output_file,draw_state->output_buffer,table_layout.column[0].width);
				write(draw_state->output_file,"    ",4);
				write(draw_state->output_file,draw_state->output_buffer,table_layout.column[1].width);
				write(draw_state->output_file,"\n",1);

				/*clear the margins */
				draw_state->output_buffer[0] = ' ';
				draw_state->output_buffer[1] = ' ';
				draw_state->output_buffer[2] = ' ';
				draw_state->output_buffer[3] = ' ';

				/*clear the margins */
				draw_state->output_buffer[draw_state->page_width-1] = ' ';
				draw_state->output_buffer[draw_state->page_width-2] = ' ';
				draw_state->output_buffer[draw_state->page_width-3] = ' ';
				draw_state->output_buffer[draw_state->page_width-4] = ' ';

				/* write the table */
				table_row.row[0] = &current_constant->name;
				table_row.row[1] = &current_constant->brief;

				text_draw_table_row(draw_state,&table_layout,&table_row);

				write(draw_state->output_file,"\n",1);
				
				/* now dump the value to the output */
				write_complex(draw_state,&current_constant->value,4);
			}
		}

		current_constant = current_constant->next;
	}
			
	write(draw_state->output_file,"\n",1);
}


