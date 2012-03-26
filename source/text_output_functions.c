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
		write(draw_state->output_file,"    state machine: ",sizeof("    state machine: ")-1);
	}
	else 
	{
		write(draw_state->output_file,"    sequence diagram: ",sizeof("    sequence diagram: ")-1);
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
 * Name : text_output_api_parameters_function
 * Desc : This function will output the parameter table.
 *--------------------------------------------------------------------------------*/
void	text_output_api_parameters_function(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	unsigned int	max_length;
	unsigned int	space_length;
	unsigned char	spaces[200];
	API_PARAMETER*	current_parameter;

	/* calculate the maximum number of spaces */
	if (function->max_param_type_length < 4)
		function->max_param_type_length = 4;
	
	if (function->max_param_name_length < 4)
		function->max_param_name_length = 4;

	if (function->max_param_type_length > function->max_param_name_length)
		space_length = function->max_param_type_length + 4;
	else
		space_length = function->max_param_name_length + 4;

	memset(spaces,' ',space_length<40?40:space_length);
	
	/* Ok, write the title */
	write(draw_state->output_file,"    Name",8);
	write(draw_state->output_file,spaces,function->max_param_name_length - 8 + 4);
	write(draw_state->output_file,"    Type",8);
	write(draw_state->output_file,spaces,function->max_param_type_length - 8 + 4);
	write(draw_state->output_file,"    Description",15);
	write(draw_state->output_file,spaces,40);
	write(draw_state->output_file,"\n",1);

	/* now write the underline */
	memset(spaces,'-',space_length<40?40:space_length);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,spaces,function->max_param_name_length);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,spaces,function->max_param_type_length);
	write(draw_state->output_file,"    ",4);
	write(draw_state->output_file,spaces,40);
	write(draw_state->output_file,"\n",1);

	/* clear the spaces for the write */
	memset(spaces,' ',space_length);

	current_parameter = function->parameter_list;

	while (current_parameter != NULL)
	{
		write(draw_state->output_file,"    ",4);
		
		write(draw_state->output_file,current_parameter->name.name,current_parameter->name.name_length);
		write(draw_state->output_file,spaces,function->max_param_name_length - current_parameter->name.name_length + 4);
		
		write(draw_state->output_file,current_parameter->type.name,current_parameter->type.name_length);
		write(draw_state->output_file,spaces,function->max_param_type_length - current_parameter->type.name_length + 4);
		
		write(draw_state->output_file,current_parameter->brief.name,current_parameter->brief.name_length);
			
		write(draw_state->output_file,"\n",1);

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
		write(draw_state->output_file,function->description.name,function->description.name_length);
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

