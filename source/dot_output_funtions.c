/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : dot_output_functions
 * Desc  : This file holds the functions for generating the dot formatted output.
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


/*--------------------------------------------------------------------------------*
 * static definitons needed for the dot files.
 *--------------------------------------------------------------------------------*/
static char			label[] = 				" [ label = \"";
static char			label_end[] = 			"\" ]";
static char			file_header[] =		"digraph ";
static char			state_machine_hdr[] =	" {\n"
											"rankdir=LR;\n"
											"node [shape = doublecircle]; start finish;\n"
											"node [shape = circle];\n";
static char			sequence_diag_hdr[] =	" {\n"
											"node [shape = box];\n";
static char			timeline_hdr[] =		"{\n"
											"  rank=same;\n  ";
static char			timeline_ftr[] =		"\n}\n";
static char			footer[] = 				"}\n";


/*----- FUNCTION -----------------------------------------------------------------*
 * Name : dot_output_header
 * Desc : This function will output the file header.
 *--------------------------------------------------------------------------------*/
void	dot_output_header(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length)
{
	write(draw_state->output_file,file_header,sizeof(file_header)-1);

	if (name_length == 0)
	{
		write(draw_state->output_file,"default",sizeof("default")-1);
	}
	else
	{
		write(draw_state->output_file,name,name_length);
	}

	if (draw_state->type == DIAGRAM_TYPE_STATE_MACHINE)
		write(draw_state->output_file,state_machine_hdr,sizeof(state_machine_hdr)-1);
	else 
		write(draw_state->output_file,sequence_diag_hdr,sizeof(sequence_diag_hdr)-1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : dot_output_footer
 * Desc : This adds the footer to the output.
 *--------------------------------------------------------------------------------*/
void	dot_output_footer(DRAW_STATE* draw_state)
{
	write(draw_state->output_file,footer,sizeof(footer)-1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : dot_output_timelines
 * Desc : This function will output the list of timelines.
 *--------------------------------------------------------------------------------*/
void	dot_output_timelines(DRAW_STATE* draw_state, TIMELINE* timeline)
{
	TIMELINE* current = timeline;
	
	write(draw_state->output_file,timeline_hdr,sizeof(timeline_hdr)-1);
	
	while (current != NULL)
	{
		if (memcmp("broadcast",current->name,sizeof("broadcast")-1) != 0)
		{
			write(draw_state->output_file,current->name,current->name_length);
			write(draw_state->output_file," ",1);
		}

		current = current->next;
	}

	write(draw_state->output_file,timeline_ftr,sizeof(timeline_ftr)-1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : dot_output_message
 * Desc : This function will output the message connection for the dot output.
 *--------------------------------------------------------------------------------*/
void	dot_output_message(DRAW_STATE* draw_state, MESSAGE* message)
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
	draw_state->buffer[draw_state->data.sequence.column[draw_state->data.sequence.num_columns-1]+1] = '\0';

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
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : dot_output_states
 * Desc : This function outputs the state list, which we do not need to do for
 *        the dot version.
 *--------------------------------------------------------------------------------*/
void	dot_output_states(DRAW_STATE* draw_state, STATE* list)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : dot_output_state
 * Desc : This function outputs a single state.
 *--------------------------------------------------------------------------------*/
void	dot_output_state(DRAW_STATE* draw_state, STATE* state)
{
	TRIGGERS*			current_triggers;
	unsigned int		need_end = 0;
	unsigned int		no_label = 1;
	STATE_TRANSITION*	current_trans = state->transition_list;

	current_trans = state->transition_list;

	if (state->name_length > 0)
	{
		current_trans = state->transition_list;

		while(current_trans != NULL)
		{
			need_end = 0;
			no_label = 1;

			write(draw_state->output_file,state->name,state->name_length);
		
			if (current_trans->next_state->name_length > 0)
			{
				write(draw_state->output_file," -> ",4);
				write(draw_state->output_file,current_trans->next_state->name,current_trans->next_state->name_length);
			}

			if (current_trans->trigger != NULL)
			{
				write(draw_state->output_file,label,sizeof(label)-1);

				if (state->group != current_trans->trigger->group)
				{
					write(draw_state->output_file,current_trans->trigger->group->name,current_trans->trigger->group->name_length);
					write(draw_state->output_file,".",1);
				}

				write(draw_state->output_file,current_trans->trigger->name,current_trans->trigger->name_length);

				no_label = 0;
				need_end = 1;
			}
			else if (current_trans->condition != NULL)
			{
				write(draw_state->output_file,label,sizeof(label)-1);
				write(draw_state->output_file,current_trans->condition,current_trans->condition_length);
				no_label = 0;
				need_end = 1;
			}
			
			if (current_trans->triggers != NULL)
			{
				TRIGGERS*	current_triggers = current_trans->triggers;
				
				if (no_label)
				{
					write(draw_state->output_file,label,sizeof(label)-1);
				}

				write(draw_state->output_file," -> ",4);
	
				do
				{
					if (state->group != current_triggers->trigger->group)
					{
						write(draw_state->output_file,current_triggers->trigger->group->name,current_triggers->trigger->group->name_length);
						write(draw_state->output_file,".",1);
					}

					write(draw_state->output_file,current_triggers->trigger->name,current_triggers->trigger->name_length);
					if (current_triggers->next != NULL)
					{
						write(draw_state->output_file,", ",2);
					}
					current_triggers = current_triggers->next;
				} 
				while (current_triggers != NULL);
				
				need_end = 1;
			}

			if (need_end)
			{
				write(draw_state->output_file,label_end,sizeof(label_end)-1);
			}

			write(draw_state->output_file,";\n",2);
			
			current_trans = current_trans->next;
		}

		state = state->next;
	}
}


