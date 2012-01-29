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
#include "supported_formats.h"
#include "document_generator.h"

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_open
 * Desc : This function will open the text file.
 *--------------------------------------------------------------------------------*/
unsigned int	text_open(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length)
{
	unsigned int result = 0;
	unsigned int length = name_length + draw_state->path_length;

	if ((length + 4) < FILENAME_MAX)
	{
		memcpy(&draw_state->path[draw_state->path_length],name,name_length);
		draw_state->path[length++] = '.';
		draw_state->path[length++] = 't';
		draw_state->path[length++] = 'x';
		draw_state->path[length++] = 't';
		draw_state->path[length] = '\0';

		if ((draw_state->output_file = open((char*)draw_state->path,O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) != -1)
		{
			/* file successfully opened */
			result = 1;
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

	if (draw_state->type == DIAGRAM_TYPE_STATE_MACHINE)
	{
		write(draw_state->output_file,"state machine: ",sizeof("state machine: ")-1);
	}
	else 
	{
		write(draw_state->output_file,"sequence diagram: ",sizeof("sequence diagram: ")-1);
	}
	
	write(draw_state->output_file,name,name_length);
	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_footer
 * Desc : This adds the footer to the output.
 *--------------------------------------------------------------------------------*/
void	text_output_footer(DRAW_STATE* draw_state)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_output_timelines
 * Desc : This function will output the list of timelines.
 *--------------------------------------------------------------------------------*/
void	text_output_timelines(DRAW_STATE* draw_state, TIMELINE* timeline)
{
	unsigned int	offset = 0;
	TIMELINE* 		current = timeline;
	
	draw_state->offset = 0;

	while (current != NULL)
	{
		if (memcmp("broadcast",current->name,sizeof("broadcast")-1) != 0)
		{
			draw_state->buffer[draw_state->offset++] = '[';
			memcpy(&draw_state->buffer[draw_state->offset],current->name,current->name_length);
			draw_state->offset += current->name_length;
			draw_state->buffer[draw_state->offset++] = ']';

			offset = draw_state->offset;

			memset(&draw_state->buffer[draw_state->offset],' ',current->group->max_message_length);
			draw_state->offset += current->group->max_message_length;
		}

		current = current->next;
	}
			
	draw_state->buffer[offset++] = '\n';
	write(draw_state->output_file,draw_state->buffer,offset);
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
	unsigned char	special = '<';

	if (message->receiver == NULL)
	{
		special = '?';
	}

	/* clear the line */
	memset(draw_state->buffer,' ',draw_state->data.sequence.column[draw_state->data.sequence.num_columns]);

	/* now fix the end point */
	draw_state->buffer[draw_state->data.sequence.column[draw_state->data.sequence.num_columns-1]+1] = '\n';

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
			memset(&draw_state->buffer[end+1],'-',start-end-1);
			draw_state->buffer[end+1] = '<';
			draw_state->offset = end + (start - end - message->name_length)/2;
		}
		else if ((end-start) > 0)
		{
			memset(&draw_state->buffer[start+1],'-',end-start-1);
			draw_state->buffer[end-1] = '>';
			draw_state->offset = start + (end - start - message->name_length)/2;
		}
		else
		{
			draw_state->buffer[end]   = special;
			draw_state->offset = end - (message->name_length);
		}
	
		for (count=0;count<draw_state->data.sequence.num_columns;count++)
		{

			if (draw_state->buffer[draw_state->data.sequence.column[count]] == '-')
				draw_state->buffer[draw_state->data.sequence.column[count]] = '+';
			else if (draw_state->buffer[draw_state->data.sequence.column[count]] == ' ')
				draw_state->buffer[draw_state->data.sequence.column[count]] = '|';
		}

		memcpy(&draw_state->buffer[draw_state->offset],message->name,message->name_length);
		draw_state->offset += message->name_length;
	}

	write(draw_state->output_file,draw_state->buffer,draw_state->data.sequence.column[draw_state->data.sequence.num_columns-1]+2);
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
 * Name : text_output_state
 * Desc : This function outputs a single state.
 *--------------------------------------------------------------------------------*/
void	text_output_state(DRAW_STATE* draw_state, STATE* state)
{
	TRIGGERS*			current_triggers;
	STATE_TRANSITION*	current_trans = state->transition_list;

	current_trans = state->transition_list;

	if (state->name_length > 0)
	{
		write(draw_state->output_file,state->name,state->name_length);
		write(draw_state->output_file,":\n",2);
		
		while(current_trans != NULL)
		{
			write(draw_state->output_file,"    -> ",sizeof("    -> ")-1);
			write(draw_state->output_file,current_trans->next_state->name,current_trans->next_state->name_length);

			if (current_trans->trigger != NULL)
			{
				write(draw_state->output_file," when ",sizeof(" when ")-1);

				if (state->group != current_trans->trigger->group)
				{
					write(draw_state->output_file,current_trans->trigger->group->name,current_trans->trigger->group->name_length);
					write(draw_state->output_file,":",1);
				}

				write(draw_state->output_file,current_trans->trigger->name,current_trans->trigger->name_length);
				write(draw_state->output_file," ",1);
			}
			else if (current_trans->condition != NULL)
			{
				write(draw_state->output_file," when '",sizeof(" when '")-1);
				write(draw_state->output_file,current_trans->condition,current_trans->condition_length);
				write(draw_state->output_file,"' ",sizeof("' ")-1);
			}

			if (current_trans->triggers != NULL)
			{
				write(draw_state->output_file,"triggering ",sizeof("triggering ")-1);

				current_triggers = current_trans->triggers;

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
			current_trans = current_trans->next;
		}
	}
}

