/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : document_processor
 * Desc  : This utility is the document processor. 
 *         It will handle the processing of the structural documents with the
 *         data recovered from the source files.
 *
 * Author: pantoine
 * Date  : 31/01/2012 09:03:17
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "atoms.h"
#include "utilities.h"
#include "error_codes.h"
#include "supported_formats.h"
#include "document_generator.h"

extern OUTPUT_FORMATS	output_formats[];

/*--------------------------------------------------------------------------------*
 * static constant strings
 *--------------------------------------------------------------------------------*/
static unsigned char	string_none[] = "";
static unsigned char	string_api[] = "api";
static unsigned char	string_state_machine[] = "state_machine";
static unsigned char	string_sequence_diagram[] = "sequence_diagram";
static unsigned char	string_sample[] = "sample";
static unsigned char*	type_string[] = {string_none,string_state_machine,string_sequence_diagram,string_api,string_sample};
static unsigned int		type_length[] = {0,sizeof(string_state_machine)-1,sizeof(string_sequence_diagram)-1,sizeof(string_api)-1,sizeof(string_sample)-1};

extern unsigned char is_valid_char[];

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : tag_all_states
 * Desc : This function will untag all states in the state_machine.
 *--------------------------------------------------------------------------------*/
static void	tag_all_states(STATE_MACHINE* state_machine)
{
	STATE*	current_state = state_machine->state_list;

	while (current_state != NULL)
	{
		current_state->flags |= (FLAG_TAGGED | FLAG_ACTIVE);

		current_state = current_state->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : untag_all_states
 * Desc : This function will untag all states in the state_machine.
 *--------------------------------------------------------------------------------*/
static void	untag_all_states(STATE_MACHINE* state_machine)
{
	STATE*	current_state = state_machine->state_list;

	while (current_state != NULL)
	{
		current_state->flags &= ~(FLAG_TAGGED | FLAG_ACTIVE);

		current_state = current_state->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : tag_state
 * Desc : This function will tag the state and all the states that communicate
 *        with it. The main state is tagged as active and the other states are
 *        tagged as tagged so the generator knows which states it has to output.
 *--------------------------------------------------------------------------------*/
static void	tag_state(STATE* state)
{
	STATE_TRANSITION*	current_transition = state->transition_list;

	state->flags |= (FLAG_ACTIVE | FLAG_TAGGED);

	while(current_transition != NULL)
	{
		if (current_transition->next_state != NULL)
		{
			current_transition->next_state->flags |= FLAG_TAGGED;
		}

		current_transition = current_transition->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_state_machine
 * Desc : This function will produce a dot file for the given state machine.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_state_machine(DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* tree)
{
	unsigned int		result = EC_OK;
	STATE*				current_state = NULL;
	GROUP*				group = NULL;
	STATE_TRANSITION*	current_trans;

	if ((group = find_group(tree,input_state->group_name,input_state->group_length)) == NULL)
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (input_state->item_length > 0 && (current_state = find_state(group,input_state->item_name,input_state->item_length)) == NULL)
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (group->state_machine == NULL)
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else
	{
		if (current_state == NULL)
		{
			tag_all_states(group->state_machine);
		}
		else
		{
			untag_all_states(group->state_machine);
			tag_state(current_state);
		}

		/* now generate the output */
		current_state = group->state_machine->state_list;
		
		output_formats[draw_state->format].output_header(draw_state,group->name,group->name_length);
		output_formats[draw_state->format].output_states(draw_state,current_state);

		while (current_state != NULL)
		{
			if ((current_state->flags & FLAG_TAGGED) == FLAG_TAGGED)
			{
				/* we have a state that we want to output */
				output_formats[draw_state->format].output_start_state(draw_state,current_state);

				current_trans = current_state->transition_list;

				while(current_trans != NULL)
				{
					/* is this transition one that we are interested in? */
					if (((current_state->flags & FLAG_ACTIVE) == FLAG_ACTIVE) || ((current_trans->next_state->flags & FLAG_ACTIVE) == FLAG_ACTIVE))
					{
						output_formats[draw_state->format].output_transition(draw_state,current_state,current_trans);
					}

					current_trans = current_trans->next;
				}

				output_formats[draw_state->format].output_end_state(draw_state,current_state);
			}

			current_state = current_state->next;
		}

		output_formats[draw_state->format].output_footer(draw_state);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : tag_all_timelines
 * Desc : This function simply untags all the timelines.
 *--------------------------------------------------------------------------------*/
static void	tag_all_timelines(SEQUENCE_DIAGRAM* sequence_diagram)
{
	TIMELINE*	current_timeline = sequence_diagram->timeline_list;

	while (current_timeline != NULL)
	{
		current_timeline->flags |= (FLAG_TAGGED | FLAG_ACTIVE);

		current_timeline = current_timeline->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : untag_timelines
 * Desc : This function simply untags all the timelines.
 *--------------------------------------------------------------------------------*/
static void	untag_timelines(SEQUENCE_DIAGRAM* sequence_diagram)
{
	TIMELINE*	current_timeline = sequence_diagram->timeline_list;

	while (current_timeline != NULL)
	{
		current_timeline->flags &= ~(FLAG_TAGGED|FLAG_ACTIVE);

		current_timeline = current_timeline->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : tag_timelines
 * Desc : This function will walk down the given timeline and tag the timelines
 *        that it sends or receives messages to/from. It does not clear down the
 *        timelines that it does not talk to.
 *--------------------------------------------------------------------------------*/
static void	tag_timelines(TIMELINE* source)
{
	NODE*	current_node = source->node;

	source->flags |= (FLAG_TAGGED | FLAG_ACTIVE);

	while (current_node != NULL)
	{
		if (current_node->sent_message != NULL && current_node->sent_message->target_timeline != NULL)
		{
			current_node->sent_message->target_timeline->flags |= FLAG_TAGGED;
		}
		else if (current_node->received_message != NULL && current_node->received_message->sending_timeline != NULL)
		{
			current_node->received_message->sending_timeline->flags |= FLAG_TAGGED;
		}

		current_node = current_node->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_columns
 * Desc : This function will count the number of columns in the sequence diagram
 *        and will calculate the text column offset for each of the timelines.
 *--------------------------------------------------------------------------------*/
static void	generate_columns(SEQUENCE_DIAGRAM* sequence_diagram, DRAW_STATE* draw_state)
{
	unsigned short	pos = 0;
	unsigned short	half_length = 0;
	TIMELINE*		current_timeline = sequence_diagram->timeline_list;

	draw_state->data.sequence.num_columns = 0;

	while (current_timeline != NULL)
	{
		if ((current_timeline->flags & FLAG_TAGGED) == FLAG_TAGGED)
		{
			current_timeline->column = draw_state->data.sequence.num_columns;
			pos += half_length + (current_timeline->name_length / 2) + 1;
			draw_state->data.sequence.column[draw_state->data.sequence.num_columns] = pos;

			half_length = (current_timeline->name_length / 2) + 1;

			draw_state->data.sequence.num_columns++;
			
			pos += current_timeline->group->max_message_length;
		}
		current_timeline = current_timeline->next;
	}

	draw_state->data.sequence.column[draw_state->data.sequence.num_columns] = pos;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_sequence_diagram
 * Desc : This function output a sequence diagram.
 *        It requires a output function so that the same code can be used to output
 *        any format. The status for the output function should be passed in the
 *        status parameter.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_sequence_diagram( DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* tree)
{
	NODE		walk_start;
	NODE*		search_node;
	NODE*		active_node;
	GROUP*		group;
	TIMELINE*	current_timeline = NULL;

	if ((group = find_group(tree,input_state->group_name,input_state->group_length)) == NULL)
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (input_state->item_length > 0 && (current_timeline = find_timeline(group,input_state->item_name,input_state->item_length)) == NULL)
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (group->sequence_diagram == NULL)
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}	
	else
	{
		if (current_timeline == NULL)
		{
			tag_all_timelines(group->sequence_diagram);
		}
		else
		{
			untag_timelines(group->sequence_diagram);
			tag_timelines(current_timeline);
		}

		memset(&walk_start,0,sizeof(NODE));

		/* first generate the column offsets for the sequence diagram */
		generate_columns(group->sequence_diagram,draw_state);
		
		current_timeline = group->sequence_diagram->timeline_list;

		output_formats[draw_state->format].output_header(draw_state,group->name,group->name_length);
		output_formats[draw_state->format].output_timelines(draw_state,current_timeline);

		if (draw_state->data.sequence.num_columns > 0)
		{
			while (current_timeline != NULL)
			{
				if (current_timeline->node != NULL && ((current_timeline->flags & FLAG_TAGGED) == FLAG_TAGGED))
				{
					walk_start.next = current_timeline->node;

					search_node = &walk_start;

					do
					{
						search_node = search_next_node(search_node,&active_node);

						if (active_node != NULL)
						{
							do
							{
								if (active_node->sent_message != NULL)
								{
									if (active_node->sent_message->receiver != NULL &&
											(((active_node->sent_message->sender->timeline->flags & FLAG_ACTIVE) == FLAG_ACTIVE) ||
											 ((active_node->sent_message->receiver->timeline->flags & FLAG_ACTIVE) == FLAG_ACTIVE))) 
									{
										output_formats[draw_state->format].output_message(draw_state,active_node->sent_message);
									}
								}
								active_node = next_active_node(active_node);
							}
							while (active_node != NULL && search_node != active_node);
						}
					}
					while (search_node != NULL);
				}
				current_timeline = current_timeline->next;
			}
		}

		output_formats[draw_state->format].output_footer(draw_state);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_api_item
 * Desc : This function will decode the api item name set to flags to the items
 *        that are required.
 *        The request format is as follows:
 *        {<type>|'*'}{/<name>|'*'}{/<part>}
 *--------------------------------------------------------------------------------*/
unsigned int	decode_api_item(unsigned char* name, unsigned int name_length, API_PARTS* api_parts)
{
	unsigned int pos = 0;
	unsigned int name_end = 0;
	unsigned int name_start = 0;
	unsigned int result = 1;
	unsigned int offset = 0;
	
	if (name_length == 0)
	{
		api_parts->flags = (OUTPUT_API_ALL					| 
							OUTPUT_API_MULTIPLE				| 
							OUTPUT_API_FUNCTION_ALL_PARTS	| 
							OUTPUT_API_TYPE_ALL_PARTS		|
							OUTPUT_API_CONSTANTS_ALL_PARTS);
		api_parts->name.name = NULL;
		api_parts->name.name_length = 0;
	}
	else
	{
		if (name[0] == '*')
		{
			api_parts->flags = OUTPUT_API_ALL;
			pos = 1;
		}
		else
		{
			switch(atoms_check_word(name))
			{
				case ATOM_FUNCTION:
					api_parts->flags = OUTPUT_API_FUNCTIONS;
					api_parts->flags |= OUTPUT_API_FUNCTION_ALL_PARTS;
					pos += atoms_get_length(ATOM_FUNCTION);
					break;

				case ATOM_TYPE:
					api_parts->flags = OUTPUT_API_TYPES;
					api_parts->flags |= OUTPUT_API_TYPE_ALL_PARTS;
					pos += atoms_get_length(ATOM_TYPE);
					break;

				case ATOM_CONSTANTS:
					api_parts->flags = OUTPUT_API_CONSTANTS;
					pos += atoms_get_length(ATOM_CONSTANTS);
					break;

				default:
					raise_warning(0,EC_BAD_NAME_FORMAT_IN_REQUEST,name,NULL);
					result = 0;
			}
		}

		/* read the name */
		if (pos < name_length)
		{
			api_parts->name.name = &name[pos+1];

			if (name[pos] != '/')
			{
				raise_warning(0,EC_BAD_NAME_FORMAT_IN_REQUEST,name,NULL);
				result  = 0;
			}
			else if (name[pos] == '*')
			{
				pos++;
				api_parts->flags |= OUTPUT_API_MULTIPLE;
			}
			else
			{
				pos++;
				name_start = pos;
				while(pos < name_length && is_valid_char[name[pos]])
				{
					pos++;
				}
				name_end = pos;
			}
		}
		else
		{
			/* if not name is specified then output all the things */
			api_parts->flags |= OUTPUT_API_MULTIPLE;
		}

		/* read the sub-parts */
		api_parts->name.name_length = name_end - name_start;

		if (pos < name_length)
		{
			if (name[pos] != '/')
			{
				raise_warning(0,EC_BAD_NAME_FORMAT_IN_REQUEST,name,NULL);
				result = 0;
			}
			else
			{
				api_parts->flags &= ~(OUTPUT_API_FUNCTION_ALL_PARTS | OUTPUT_API_TYPE_ALL_PARTS);

				if (api_parts->flags == OUTPUT_API_FUNCTIONS)
				{
					switch(atoms_check_word(&name[pos+1]))
					{
						case 	ATOM_NAME:
							pos += atoms_get_length(ATOM_NAME);
							api_parts->flags |= OUTPUT_API_FUNCTION_NAME;
							break;

						case 	ATOM_ACTION:
							pos += atoms_get_length(ATOM_ACTION);
							api_parts->flags |= OUTPUT_API_FUNCTION_ACTION;
							break;

						case 	ATOM_RETURNS:
							pos += atoms_get_length(ATOM_RETURNS);
							api_parts->flags |= OUTPUT_API_FUNCTION_RETURNS;
							break;

						case 	ATOM_PROTOTYPE:
							pos += atoms_get_length(ATOM_PROTOTYPE);
							api_parts->flags |= OUTPUT_API_FUNCTION_PROTOTYPE;
							break;

						case 	ATOM_PARAMETERS:
							pos += atoms_get_length(ATOM_PARAMETERS);
							api_parts->flags |= OUTPUT_API_FUNCTION_PARAMETERS;
							break;

						case 	ATOM_DESCRIPTION:
							pos += atoms_get_length(ATOM_DESCRIPTION);
							api_parts->flags |= OUTPUT_API_FUNCTION_DESCRIPTION;
							break;  	

						default:
							raise_warning(0,EC_BAD_NAME_FORMAT_IN_REQUEST,name,NULL);
							result = 0;
							break;
					}
				}
				else if (api_parts->flags == OUTPUT_API_TYPES)
				{
					switch(atoms_check_word(&name[pos+1]))
					{
						case	ATOM_NAME:
							pos += atoms_get_length(ATOM_NAME);
							api_parts->flags |= OUTPUT_API_TYPE_NAME;
							break;
						
						case 	ATOM_DESCRIPTION:
							pos += atoms_get_length(ATOM_DESCRIPTION);
							api_parts->flags |= OUTPUT_API_TYPE_DESCRIPTION;
							break;

						default:
							raise_warning(0,EC_BAD_NAME_FORMAT_IN_REQUEST,name,NULL);
							result = 0;
							break;
					}
				}
				else if (api_parts->flags == OUTPUT_API_CONSTANTS)
				{
					switch(atoms_check_word(&name[pos+1]))
					{

						case	ATOM_NAME:
							pos += atoms_get_length(ATOM_NAME);
							api_parts->flags |= OUTPUT_API_CONSTANTS_NAME;
							break;
						
						case 	ATOM_DESCRIPTION:
							pos += atoms_get_length(ATOM_DESCRIPTION);
							api_parts->flags |= OUTPUT_API_CONSTANTS_DESCRIPTION;
							break;

						case	ATOM_CONSTANTS:
							pos += atoms_get_length(ATOM_CONSTANTS);
							api_parts->flags |= OUTPUT_API_CONSTANTS_CONSTANT;
							break;

						default:
							raise_warning(0,EC_BAD_NAME_FORMAT_IN_REQUEST,name,NULL);
							result = 0;
							break;
					}
				}
			}
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_api
 * Desc : This function output an API.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_api( DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* tree)
{
	GROUP*				group;
	API_PARTS			item_parts;
	API_TYPE*			current_type;
	API_FUNCTION*		current_function;
	API_CONSTANTS*		current_constants;

	if ((group = find_group(tree,input_state->group_name,input_state->group_length)) == NULL)
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (!decode_api_item(input_state->item_name,input_state->item_length,&item_parts))
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (group->api == NULL)
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}	
	else
	{
		/* first generate the header for the API */
		output_formats[draw_state->format].output_header(draw_state,group->name,group->name_length);

		/* output the function parts */
		if ((item_parts.flags & OUTPUT_API_FUNCTIONS) != 0)
		{
			if (item_parts.name.name_length == 0)
			{
				current_function = group->api->function_list;
			}
			else
			{
				current_function = group->api->function_list;

				while (current_function != NULL)
				{
					if (item_parts.name.name_length == current_function->name.name_length && 
						memcmp(item_parts.name.name,current_function->name.name,current_function->name.name_length) == 0)
					{
						/* found it */
						break;
					}
					current_function = current_function->next;
				}
			}

			if (current_function == NULL)
			{
				raise_warning(0,EC_UNDEFINED_FUNCTION,item_parts.name.name,NULL);
			}
			else
			{
				do
				{
					if ((item_parts.flags & OUTPUT_API_FUNCTION_NAME) != 0)
					{
						output_formats[draw_state->format].output_function_name(draw_state,current_function);
					}

					if ((item_parts.flags & OUTPUT_API_FUNCTION_DESCRIPTION) != 0)
					{
						output_formats[draw_state->format].output_function_description(draw_state,current_function);
					}

					if ((item_parts.flags & OUTPUT_API_FUNCTION_PROTOTYPE) != 0)
					{
						output_formats[draw_state->format].output_function_prototype(draw_state,current_function);
					}

					if ((item_parts.flags & OUTPUT_API_FUNCTION_PARAMETERS) != 0)
					{
						output_formats[draw_state->format].output_function_parameters(draw_state,current_function);
					}

					if ((item_parts.flags & OUTPUT_API_FUNCTION_ACTION) != 0)
					{
						output_formats[draw_state->format].output_function_action(draw_state,current_function);
					}

					if ((item_parts.flags & OUTPUT_API_FUNCTION_RETURNS) != 0)
					{
						output_formats[draw_state->format].output_function_returns(draw_state,current_function);
					}

					current_function = current_function->next;
				}
				while (current_function != NULL && (item_parts.flags & OUTPUT_API_MULTIPLE));
			}
		}

		/* output the types */
		if ((item_parts.flags & OUTPUT_API_TYPES) != 0)
		{
			if (item_parts.name.name_length == 0)
			{
				current_type = group->api->type_list;
			}
			else
			{
				current_type = group->api->type_list;

				while (current_type != NULL)
				{
					if (item_parts.name.name_length == current_type->name.name_length && 
						memcmp(item_parts.name.name,current_type->name.name,current_type->name.name_length) == 0)
					{
						/* found it */
						break;
					}
					current_type = current_type->next;
				}
			}

			if (current_type == NULL)
			{
				/* check to see if the user specifically asked for the type or not */
				if ((item_parts.flags & OUTPUT_API_ALL) != OUTPUT_API_ALL)
				{
					raise_warning(0,EC_UNDEFINED_TYPE,item_parts.name.name,NULL);
				}
			}
			else
			{
				do
				{
					if ((item_parts.flags & OUTPUT_API_TYPE_NAME) != 0)
					{
						output_formats[draw_state->format].output_type_name(draw_state,current_type);
					}

					if ((item_parts.flags & OUTPUT_API_TYPE_RECORDS) != 0)
					{
						output_formats[draw_state->format].output_type_description(draw_state,current_type);
					}

					if ((item_parts.flags & OUTPUT_API_TYPE_DESCRIPTION) != 0)
					{
						output_formats[draw_state->format].output_type_records(draw_state,current_type);
					}

					current_type = current_type->next;
				}
				while(current_type != NULL && (item_parts.flags & OUTPUT_API_MULTIPLE));
			}
		}

		/* output the constants */
		if ((item_parts.flags & OUTPUT_API_CONSTANTS) != 0)
		{
			if (item_parts.name.name_length == 0)
			{
				current_constants = group->api->constants_list;
			}
			else
			{
				current_constants = group->api->constants_list;

				while (current_constants != NULL)
				{
					if (item_parts.name.name_length == current_constants->name.name_length && 
						memcmp(item_parts.name.name,current_constants->name.name,current_constants->name.name_length) == 0)
					{
						/* found it */
						break;
					}
					current_constants = current_constants->next;
				}
			}
			
			if (current_constants == NULL)
			{
				/* check to see if the user specifically asked for the constants or not */
				if ((item_parts.flags & OUTPUT_API_ALL) != OUTPUT_API_ALL)
				{
					raise_warning(0,EC_UNDEFINED_CONSTANTS_GROUP,item_parts.name.name,NULL);
				}
			}
			else
			{
				do
				{
					if ((item_parts.flags & OUTPUT_API_CONSTANTS_NAME) != 0)
					{
						output_formats[draw_state->format].output_constant_name(draw_state,current_constants);
					}
	
					if ((item_parts.flags & OUTPUT_API_CONSTANTS_DESCRIPTION) != 0)
					{
						output_formats[draw_state->format].output_constant_description(draw_state,current_constants);
					}

					if ((item_parts.flags & OUTPUT_API_CONSTANTS_CONSTANT) != 0)
					{
						output_formats[draw_state->format].output_constant_records(draw_state,current_constants);
					}

					current_constants = current_constants->next;
				}
				while(current_constants != NULL && (item_parts.flags & OUTPUT_API_MULTIPLE));
			}
		}

		output_formats[draw_state->format].output_footer(draw_state);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_sample
 * Desc : This function output an sample.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_sample( DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* tree)
{
	SAMPLE*	current_sample = NULL;

	/* only, support non-grouped samples are the moment */
	if (input_state->group_length != sizeof("default")-1 && memcmp(input_state->group_name,"default",sizeof("default")-1))
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else
	{
		current_sample = tree->sample_list->next;

		while (current_sample != NULL)
		{
			if (input_state->item_length == current_sample->name.name_length &&
				memcmp(current_sample->name.name,input_state->item_name,input_state->item_length) == 0)
			{
				output_formats[draw_state->format].output_sample(draw_state,current_sample);
				break;
			}

			current_sample = current_sample->next;
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_numeric_record
 * Desc : This function will read a string record.
 *--------------------------------------------------------------------------------*/
static unsigned int	read_numeric_record(unsigned int offset, unsigned char* buffer, unsigned short *parameter)
{
	*parameter = (((unsigned short)buffer[offset + 1]) << 8) | buffer[offset+2];

	return (offset + 3);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_numerics_record
 * Desc : This function will read a string record.
 *--------------------------------------------------------------------------------*/
static unsigned int	read_numerics_record(unsigned int offset, unsigned char* buffer, unsigned short *parameter1, unsigned int *parameter2)
{
	*parameter1 = (((unsigned short)buffer[offset + 1]) << 8) | buffer[offset+2];
	*parameter2 = (((	((unsigned int)buffer[offset + 3]) << 24) | 
						((unsigned int)buffer[offset + 4]) << 16) | 
						((unsigned int)buffer[offset + 5]) << 8)  | 
						buffer[offset+6];

	return (offset + 3 + 4);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_string_record
 * Desc : This function will read a string record.
 *--------------------------------------------------------------------------------*/
static unsigned int	read_string_record(unsigned int offset, unsigned char* buffer, NAME* name)
{
	name->name_length = buffer[offset+1];
	name->name = &buffer[offset+2];

	return (offset + 2 + buffer[offset+1]);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_group_record
 * Desc : This function reads a group record.
 *--------------------------------------------------------------------------------*/
static unsigned int	read_group_record(	unsigned int	offset, 
										unsigned char*	buffer, 
										NAME*			group,
										NAME*			name)
{
	unsigned char	length = buffer[offset+1];

	/* get the group name */
	group->name_length = buffer[offset+1];
	group->name = &buffer[offset+2];

	/* get the name */
	name->name_length = buffer[offset + length + 2];
	name->name = &buffer[offset + length + 3];

	return (offset + length + 3 + buffer[offset + length + 2]);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_group_id_record
 * Desc : This function reads a group record and an id.
 *--------------------------------------------------------------------------------*/
static unsigned int	read_group_id_record(	unsigned int	offset, 
											unsigned char*	buffer, 
											NAME*			group,
											NAME*			name,
											unsigned short*	id)
{
	unsigned char	length = buffer[offset+3];

	*id = (((unsigned short)buffer[offset+1] << 8) | buffer[offset+2]);

	/* get the group name */
	group->name_length = buffer[offset+3];
	group->name = &buffer[offset+4];

	/* get the name */
	name->name_length = buffer[offset + length + 4];
	name->name = &buffer[offset + length + 5];

	return (offset + length + 5 + buffer[offset + length + 4]);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_message_record
 * Desc : This function writes a group record.
 *--------------------------------------------------------------------------------*/
static unsigned int	read_message_record(	unsigned int	offset, 
											unsigned char*	buffer,
											unsigned char*	sender_id,
											unsigned char*	receiver_id,
											NAME*			receiver_timeline, 
											NAME*			message)
{
	/* get the id's */
	*sender_id = buffer[offset+1];
	*receiver_id = buffer[offset+2];

	/* get the receiver_timeline */
	receiver_timeline->name = &buffer[offset+4];
	receiver_timeline->name_length = buffer[offset+3];

	/* get the message_name */
	message->name = &buffer[offset + buffer[offset+3] + 5];
	message->name_length = buffer[offset + buffer[offset+3] + 4];

	return (offset + buffer[offset+3] + 5 + buffer[offset+buffer[offset+3]+4]);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_pair_record
 * Desc : This function reads a pair record.
 *--------------------------------------------------------------------------------*/
static unsigned int	read_pair_record(	unsigned int	offset, 
										unsigned char*	buffer,
										NAME*			value,
										NAME*			string)
{
	/* get the lengths */
	value->name_length	= buffer[offset + 1];
	string->name_length	= ((unsigned short)buffer[offset + 2] << 8) | buffer[offset + 3];

	/* get the strings */
	value->name = &buffer[offset + 4];
	string->name = &buffer[offset + 4 + value->name_length];

	return (offset + 4 + value->name_length + string->name_length);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_constant_record
 * Desc : This function reads a constant record.
 *--------------------------------------------------------------------------------*/
static unsigned int	read_constant_record(	unsigned int	offset, 
											unsigned char*	buffer,
											NAME*			type, 
											NAME*			name,
											NAME*			value,
											NAME*			brief)
{
	/* get the lengths */
	type->name_length	= buffer[offset + 1];
	name->name_length	= buffer[offset + 2];
	value->name_length	= ((unsigned short)buffer[offset + 3] << 8) | buffer[offset + 4];
	brief->name_length	= ((unsigned short)buffer[offset + 5] << 8) | buffer[offset + 6];

	/* get the strings */
	type->name = &buffer[offset + 7];
	name->name = &buffer[offset + 7 + type->name_length];
	value->name = &buffer[offset + 7 + type->name_length + name->name_length];
	brief->name = &buffer[offset + 7 + type->name_length + name->name_length + value->name_length];

	return (offset + 7 + type->name_length + name->name_length + brief->name_length + value->name_length);
}


/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_type_record
 * Desc : This function reads a type record.
 *--------------------------------------------------------------------------------*/
static unsigned int	read_type_record(	unsigned int	offset, 
										unsigned char*	buffer,
										NAME*			type, 
										NAME*			name,
										NAME*			brief)
{
	/* get the lengths */
	type->name_length	= buffer[offset + 1];
	name->name_length	= buffer[offset + 2];
	brief->name_length	= ((unsigned short)buffer[offset + 3] << 8) | buffer[offset + 4];

	/* get the strings */
	type->name = &buffer[offset + 5];
	name->name = &buffer[offset + 5 + type->name_length];
	brief->name = &buffer[offset + 5 + type->name_length + name->name_length];

	return (offset + 5 + type->name_length + name->name_length + brief->name_length);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_transition
 * Desc : This function will add a new transition to the given state. It uses the
 *        id of the state as the "STATE". This is dangerous as the NULL pointer
 *        check wont work. But, as we assume there will be less than 2^16 states
 *        we can use that check to post validate the states and transitions later.
 *--------------------------------------------------------------------------------*/
static STATE_TRANSITION*	add_transition(STATE* state, unsigned short to_id)
{
	STATE_TRANSITION*	temp;
	STATE_TRANSITION*	new_transition = NULL;

	if (state != NULL)
	{
		new_transition = calloc(1,sizeof(STATE_TRANSITION));

		temp = state->transition_list;
		state->transition_list = new_transition;
		new_transition->next = temp;

		new_transition->next_state = (STATE*) ((unsigned long)to_id);
	}

	return new_transition;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_trigger
 * Desc : This function will add a trigger to a transition.
 *--------------------------------------------------------------------------------*/
static	void	add_trigger(STATE_TRANSITION* transition,GROUP* group, NAME* trigger_name)
{
	transition->trigger = calloc(1,sizeof(TRIGGER));
	transition->trigger->group = group;
	transition->trigger->name_length = trigger_name->name_length;
	memcpy(transition->trigger->name,trigger_name->name,trigger_name->name_length);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_triggers
 * Desc : This function will add a trigger to list of triggers that are generated
 *        by this node.
 *--------------------------------------------------------------------------------*/
static void	add_triggers(STATE_TRANSITION* transition,GROUP* group, NAME* trigger_name)
{
	TRIGGERS*	new_triggers;

	new_triggers = malloc(sizeof(TRIGGERS));
	new_triggers->trigger = calloc(1,sizeof(TRIGGER));
	
	/* set the value of the trigger */
	new_triggers->trigger->group = group;
	new_triggers->trigger->name_length = trigger_name->name_length;
	memcpy(new_triggers->trigger->name,trigger_name->name,trigger_name->name_length);

	if (transition->triggers == NULL)
	{
		transition->triggers = new_triggers;
	}
	else
	{
		new_triggers->next = transition->triggers;
		transition->triggers = new_triggers;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_message
 * Desc : This function will add a message to the given timeline.
 *--------------------------------------------------------------------------------*/
static void	add_message(NODE* sender, TIMELINE* destination, BLOCK_NAME* name)
{
	MESSAGE* new_message = calloc(1,sizeof(MESSAGE));

	if (sender->timeline->group->max_message_length < name->name_length)
	{
		sender->timeline->group->max_message_length = name->name_length;
	}

	/* set the values in new message */
	memcpy(new_message->name,name->name,name->name_length);
	new_message->sender = sender;
	new_message->name_length = name->name_length;
	new_message->target_timeline = destination;
	new_message->sending_timeline = sender->timeline;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_node
 * Desc : This function will add a node to the timeline.
 *--------------------------------------------------------------------------------*/
static NODE*	add_node(TIMELINE* timeline, unsigned short id, unsigned int flags)
{
	NODE* result = calloc(1,sizeof(NODE));

	result->level = id;
	result->flags = flags;
	result->timeline = timeline;

	if (timeline->node == NULL)
	{
		timeline->node = result;
		timeline->last_node = result;
	}
	else
	{
		timeline->last_node->next = result;
		timeline->last_node = result;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : create_message
 * Desc : This function will create a message.
 *--------------------------------------------------------------------------------*/
static MESSAGE*	create_message(unsigned short sender_id, unsigned short receiver_id, NAME* message_name)
{
	MESSAGE* result = calloc(1,sizeof(MESSAGE));

	memcpy(result->name,message_name->name,message_name->name_length);
	result->name_length = message_name->name_length;
	result->receiver = (NODE*) ((unsigned long)receiver_id);
	result->sender = (NODE*) ((unsigned long)sender_id);

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_constants
 * Desc : This function will add an cosntants group to the API.
 *--------------------------------------------------------------------------------*/
API_CONSTANTS*	add_api_constants(GROUP* group, NAME* name, NAME* description)
{
	API_CONSTANTS* result = calloc(1,sizeof(API_CONSTANTS));
	API_CONSTANTS* current_constants;

	copy_name(name,&result->name);
	copy_name(description,&result->description);

	if (group->api == NULL)
	{
		group->api = calloc(1,sizeof(API));
		group->api->group = group;
	}

	/* move the constants to the correct API */
	if (group->api->constants_list == NULL)
	{
		group->api->constants_list = result;
	}
	else
	{
		current_constants = group->api->constants_list;

		while (current_constants != NULL)
		{
			if (current_constants->next == NULL)
			{
				current_constants->next = result;
				break;
			}

			current_constants = current_constants->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_constant
 * Desc : This function will add a constant to the constant group.
 *--------------------------------------------------------------------------------*/
void	add_api_constant(API_CONSTANTS* api_constants, NAME* name, NAME* type, NAME* value, NAME* brief)
{
	API_CONSTANT*	new_constant = malloc(sizeof(API_CONSTANT));

	/* initialise the new constant */
	copy_name(type,&new_constant->type);
	copy_name(name,&new_constant->name);
	copy_name(value,&new_constant->value);
	copy_name(brief,&new_constant->brief);

	if (api_constants->max_name_length < name->name_length)
	{
		api_constants->max_name_length = name->name_length;
	}

	if (api_constants->max_value_length < value->name_length)
	{
		api_constants->max_value_length = value->name_length;
	}

	/* add it to the constant list */
	if (api_constants->constant_list == NULL)
	{
		api_constants->constant_list = new_constant;
	}
	else
	{
		api_constants->last_constant->next = new_constant;
	}

	api_constants->last_constant = new_constant;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_type
 * Desc : This function will add an type to the API.
 *--------------------------------------------------------------------------------*/
API_TYPE*	add_api_type(GROUP* group, NAME* name, NAME* description)
{
	API_TYPE* result = calloc(1,sizeof(API_TYPE));
	API_TYPE* current_type;

	copy_name(name,&result->name);
	copy_name(description,&result->description);

	if (group->api == NULL)
	{
		group->api = calloc(1,sizeof(API));
		group->api->group = group;
	}

	/* move the type to the correct API */
	if (group->api->type_list == NULL)
	{
		group->api->type_list = result;
	}
	else
	{
		current_type = group->api->type_list;

		while (current_type != NULL)
		{
			if (current_type->next == NULL)
			{
				current_type->next = result;
				break;
			}

			current_type = current_type->next;
		}
	}


	result->max_type_item_length = 4;
	result->max_name_value_length = 4;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_type_record
 * Desc : This function will add a type record to the api type.
 *--------------------------------------------------------------------------------*/
void	add_api_type_record(API_TYPE* api_type, unsigned int record_type, NAME* name, NAME* type,  NAME* brief)
{
	API_TYPE_RECORD*	new_record = malloc(sizeof(API_TYPE_RECORD));

	/* initialise the new record */
	new_record->record_type = record_type;
	copy_name(type,&new_record->type_item);
	copy_name(name,&new_record->name_value);
	copy_name(brief,&new_record->brief);

	if (api_type->max_type_item_length < type->name_length)
	{
		api_type->max_type_item_length = type->name_length;
	}

	if (api_type->max_name_value_length < name->name_length)
	{
		api_type->max_name_value_length = name->name_length;
	}

	/* add it to the record list */
	if (api_type->record_list == NULL)
	{
		api_type->record_list = new_record;
	}
	else
	{
		api_type->last_record->next = new_record;
	}

	api_type->last_record = new_record;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_function
 * Desc : This function will add a named function in the list of functions.
 *--------------------------------------------------------------------------------*/
static API_FUNCTION*	add_api_function(GROUP* group, NAME* type, NAME* name)
{
	API_FUNCTION* 	result = calloc(1,sizeof(API_FUNCTION));
	API_FUNCTION*	current_func;

	if (group->api == NULL)
	{
		/* first definition of an api */
		group->api = calloc(1,sizeof(API));
		group->api->function_list = result;
		group->api->group = group;
	}
	else
	{
		/* add it to the end of the list */
		current_func = group->api->function_list;

		while (current_func != NULL)
		{
			if (current_func->next == NULL)
			{
				current_func->next = result;
				break;
			}

			current_func = current_func->next;
		}
	}

	/* set the values of the new function */
	result->name.name_length = name->name_length;
	result->name.name = malloc(name->name_length);
	memcpy(result->name.name,name->name,name->name_length);

	result->return_type.name_length = type->name_length;
	result->return_type.name = malloc(type->name_length);
	memcpy(result->return_type.name,type->name,type->name_length);
		
	/* set the minimum name sizes */
	result->max_param_name_length = 4;
	result->max_param_type_length = 4;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_parameter
 * Desc : This function will add a parameter in to the list of functions.
 *--------------------------------------------------------------------------------*/
static void	add_api_parameter(API_FUNCTION* function, NAME* type, NAME* name, NAME* brief)
{
	API_PARAMETER* 	result = calloc(1,sizeof(API_PARAMETER));
	API_PARAMETER*	current_parameter;

	if (function->max_param_type_length < type->name_length)
	{
		function->max_param_type_length = type->name_length;
	}

	if (function->max_param_name_length < name->name_length)
	{
		function->max_param_name_length = name->name_length;
	}

	/* set up the parameter */
	copy_name(type,&result->type);
	copy_name(name,&result->name);
	copy_name(brief,&result->brief);

	if (function->parameter_list == NULL)
	{
		function->parameter_list = result;
	}
	else
	{
		/* add it to the end of the list */
		current_parameter = function->parameter_list;

		while (current_parameter != NULL)
		{
			if (current_parameter->next == NULL)
			{
				current_parameter->next = result;
				break;
			}

			current_parameter = current_parameter->next;
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_returns
 * Desc : This function will add a return value to function.
 *--------------------------------------------------------------------------------*/
static void	add_api_returns(API_FUNCTION* function, NAME* value, NAME* brief)
{
	API_RETURNS* 	result = calloc(1,sizeof(API_RETURNS));
	API_RETURNS*	current_returns;

	/* set up the parameter */
	copy_name(value,&result->value);
	copy_name(brief,&result->brief);

	if (function->returns_list == NULL)
	{
		function->returns_list = result;
	}
	else
	{
		/* add it to the end of the list */
		current_returns = function->returns_list;

		while (current_returns != NULL)
		{
			if (current_returns->next == NULL)
			{
				current_returns->next = result;
				break;
			}
			current_returns = current_returns->next;
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_sample
 * Desc : This function will sample to the sample list.
 *--------------------------------------------------------------------------------*/
static void	add_sample(GROUP* group, NAME* name, NAME* payload)
{
	SAMPLE* 	result = calloc(1,sizeof(SAMPLE));

	/* set up the parameter */
	copy_name(name,&result->name);
	copy_name(payload,&result->sample);

	if (group->sample_list == NULL)
	{
		group->sample_list = result;
	}
	else
	{
		result->next = group->sample_list;
		group->sample_list = result;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : input_model
 * Desc : This function will input the model.
 *--------------------------------------------------------------------------------*/
static unsigned int	input_model(char* model_file, GROUP* group_tree,unsigned short *max_state, unsigned short* max_node)
{
	int					infile;
	unsigned int		state = MODEL_LOAD_UNKNOWN;
	unsigned int		offset = 0;
	unsigned int		result = 0;
	unsigned int		flags;
	unsigned short		id;
	unsigned char		sender_id = 0;
	unsigned char		receiver_id = 0;
	unsigned char		signature[4] = LINKED_SOURCE_MAGIC;
	unsigned char		record[FILE_BLOCK_SIZE];
	NAME				timeline;
	NAME				group;
	NAME				name;
	NAME				type;
	NAME				brief;
	NAME				value;
	NAME				description;
	NODE*				current_node;
	STATE*				current_state;
	GROUP*				temp_group;
	GROUP*				current_group;
	TIMELINE*			current_timeline;
	API_TYPE*			current_type;
	API_FUNCTION*		current_function;
	API_CONSTANTS*		current_constants;
	API_PARAMETER*		current_parameter;
	STATE_TRANSITION*	current_transition;

	if ((infile = open(model_file,READ_FILE_STATUS)) != -1)
	{
		if (read(infile,record,LINKER_HEADER_SIZE) == LINKER_HEADER_SIZE)
		{
			/* check to see if the file is the correct format */
			if (record[0] == signature[0] &&
				record[1] == signature[1] &&	
				record[2] == signature[2] &&	
				record[3] == signature[3])
			{
				/* valid formatted file */
				if (record[FILE_VERSION_MAJOR] > VERSION_MAJOR)
				{
					raise_warning(0,EC_INPUT_FILE_BUILT_WITH_LATER_MAJOR_VERSION,(unsigned char*)model_file,NULL);
				}

				/* now load the model */
				while(result == 0 && read(infile,record,FILE_BLOCK_SIZE) == FILE_BLOCK_SIZE)
				{
					offset = 0;

					while(offset < FILE_BLOCK_SIZE && result == 0)
					{
						if (state == MODEL_LOAD_UNKNOWN)
						{
							switch(record[offset])
							{
								case LINKER_STATE_MACHINE_START:
									state = MODEL_LOAD_STATE;
									offset = read_string_record(offset,record,&name);
									break;

								case LINKER_SEQUENCE_START:
									state = MODEL_LOAD_SEQUENCE;
									offset = read_string_record(offset,record,&name);
									break;

								case LINKER_API_START:
									state = MODEL_LOAD_API;
									offset = read_string_record(offset,record,&name);
									
									/* init the holders */
									current_type = NULL;
									current_function = NULL;
									current_constants = NULL;

									/* get the group that the API belongs to */
									if ((current_group = find_group(group_tree,name.name,name.name_length)) == NULL)
									{
										current_group = add_group(group_tree,name.name,name.name_length);
									}
									break;

								case LINKER_SAMPLE:
									offset = read_pair_record(offset,record,&name,&brief);
									add_sample(group_tree,&name,&brief);
									break;

								case LINKER_BLOCK_END:
									offset = FILE_BLOCK_SIZE;
									break;

								default:
									hex_dump(&record[offset],16);
									raise_warning(0,EC_UNXPECTED_ITEM_IN_STATE,(unsigned char*)model_file,NULL);
									result = 1;
									break;
							}
						}
						else if (state == MODEL_LOAD_API)
						{
							switch(record[offset])
							{
								case LINKER_API_FUNCTION:
									offset = read_type_record(offset,record,&type,&name,&brief);
									current_function = add_api_function(current_group,&type,&name);
									break;

								case LINKER_API_ACTION:
									offset = read_string_record(offset,record,&name);
									if (current_function != NULL)
									{
										copy_name(&name,&current_function->action);
									}					
									else
									{
										raise_warning(0,EC_ATOM_MUST_BE_DEFINED_WITHIN_A_FUNCTION,NULL,NULL);
										result = 1;
									}
								break;

								case LINKER_API_DESCRIPTION:
									offset = read_string_record(offset,record,&name);
									if (current_function != NULL)
									{
										copy_name(&name,&current_function->description);
									}					
									else
									{
										raise_warning(0,EC_ATOM_MUST_BE_DEFINED_WITHIN_A_FUNCTION,NULL,NULL);
										result = 1;
									}
								break;

								case LINKER_API_PARAMETER:
									offset = read_type_record(offset,record,&type,&name,&brief);
									if (current_function != NULL)
									{
										add_api_parameter(current_function,&type,&name,&brief);
									}
									else
									{
										raise_warning(0,EC_ATOM_MUST_BE_DEFINED_WITHIN_A_FUNCTION,NULL,NULL);
										result = 1;
									}
								break;

								case LINKER_API_RETURNS:
									offset = read_pair_record(offset,record,&name,&brief);	
									if (current_function != NULL)
									{
										add_api_returns(current_function,&name,&brief);
									}
									else
									{
										raise_warning(0,EC_ATOM_MUST_BE_DEFINED_WITHIN_A_FUNCTION,NULL,NULL);
										result = 1;
									}
								break;

								case LINKER_API_FUNCTION_END:
									current_function = NULL;
									offset++;
								break;

								case LINKER_API_TYPE_START:
									offset = read_pair_record(offset,record,&name,&description);
									current_type = add_api_type(current_group,&name,&description);
								break;
								
								case LINKER_API_TYPE_FIELD:
									offset = read_type_record(offset,record,&type,&name,&brief);
									add_api_type_record(current_type,LINKER_API_TYPE_FIELD,&name,&type,&brief);
								break;
								
								case LINKER_API_TYPE_END:
									current_type = NULL;
									offset++;
								break;

								case LINKER_API_END:
									state = MODEL_LOAD_UNKNOWN;
									offset++;
								break;

								case LINKER_API_CONSTANTS_START:
									offset = read_pair_record(offset,record,&name,&description);
									current_constants = add_api_constants(current_group,&name,&description);
								break;

								case LINKER_API_CONSTANT:
									offset = read_constant_record(offset,record,&type,&name,&value,&brief);
									add_api_constant(current_constants,&name,&type,&value,&brief);
								break;

								case LINKER_API_CONSTANTS_END:
									current_constants = NULL;
									offset += 1;
								break;

								case LINKER_END:
									current_state = NULL;
									offset += 1;
									break;

								case LINKER_BLOCK_END:
									offset = FILE_BLOCK_SIZE;
									break;

								default:
									hex_dump(&record[offset],16);
									raise_warning(0,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)model_file,NULL);
									result = 1;
							}
						}
						else if (state == MODEL_LOAD_STATE)
						{
							switch(record[offset])
							{
								case LINKER_STATE_MACHINE_END:
									state = MODEL_LOAD_UNKNOWN;
									offset++;
									break;

								case LINKER_STATE:
									offset = read_group_id_record(offset,record,&group,&name,&id);
									
									if ((current_group = find_group(group_tree,group.name,group.name_length)) == NULL)
									{
										current_group = add_group(group_tree,group.name,group.name_length);
									}
									
									current_state = add_state(current_group,name.name,name.name_length,0,0);
									current_state->tag_id = id;

									current_transition = NULL;

									if (id > *max_state)
										*max_state = id;

									break;
								
								case LINKER_TRANSITON:
									/* add the transition to the current_state */
									offset = read_numeric_record(offset,record,&id);
									
									current_transition = add_transition(current_state,id);
									break;

								case LINKER_TRIGGER:
									/* add the trigger to the current transition */
									offset = read_group_record(offset,record,&group,&name);
									
									if ((temp_group = find_group(group_tree,group.name,group.name_length)) == NULL)
									{
										temp_group = add_group(group_tree,group.name,group.name_length);
									}
		
									add_trigger(current_transition,temp_group,&name);
									
									break;

								case LINKER_TRIGGERS:
									/* add the triggers to the list of triggers to the current state */
									offset = read_group_record(offset,record,&group,&name);

									if ((temp_group = find_group(group_tree,group.name,group.name_length)) == NULL)
									{
										temp_group = add_group(group_tree,group.name,group.name_length);
									}
	
									add_triggers(current_transition,temp_group,&name);
									break;

								case LINKER_CONDITION:
									/* add the condition to the current transition */
									offset = read_string_record(offset,record,&name);
									current_transition->condition = malloc(name.name_length);
									current_transition->condition_length = name.name_length;
									memcpy(current_transition->condition,name.name,name.name_length);
									break;

								case LINKER_END:
									current_state = NULL;
									offset += 1;
									break;

								case LINKER_BLOCK_END:
									offset = FILE_BLOCK_SIZE;
									break;

								default:
									hex_dump(&record[offset],16);
									raise_warning(0,EC_UNXPECTED_ITEM_IN_STATE,(unsigned char*)model_file,NULL);
									result = 1;
							}
						}
						else
						{
							switch(record[offset])
							{
								case LINKER_SEQUENCE_END:
									state = MODEL_LOAD_UNKNOWN;
									offset++;
									break;

								case LINKER_TIMELINE:
									offset = read_group_record(offset,record,&group,&name);
									
									if ((current_group = find_group(group_tree,group.name,group.name_length)) == NULL)
									{
										current_group = add_group(group_tree,group.name,group.name_length);
									}

									current_timeline = add_timeline(current_group,name.name,name.name_length,0,0);
									break;
	
								case LINKER_NODE_START:
									offset = read_numerics_record(offset,record,&id,&flags);
									
									current_node = add_node(current_timeline,id,flags);

									if (id > *max_node)
										*max_node = id;
									break;

								case LINKER_SENT_MESSAGE:
									offset = read_message_record(	offset,
																	record,
																	&sender_id,
																	&receiver_id,
																	&timeline,
																	&name);

									current_node->sent_message = create_message(sender_id,receiver_id,&name);
									current_node->sent_message->sending_timeline = current_timeline;
									current_node->sent_message->sender = current_node;
	
									if (current_timeline->group->max_message_length < name.name_length)
									{
										current_timeline->group->max_message_length = name.name_length;
									}
	
									break;

								case LINKER_CONDITION:
									offset = read_string_record(offset,record,&name);
									current_node->condition = malloc(name.name_length);
									current_node->condition_length = name.name_length;
									memcpy(current_node->condition,name.name,name.name_length);
									break;

								case LINKER_NODE_END:
									current_node = NULL;
									offset++;
									break;

								case LINKER_BLOCK_END:
									offset = FILE_BLOCK_SIZE;
									break;

								default:
									hex_dump(&record[offset],16);
									raise_warning(0,EC_UNXPECTED_ITEM_IN_STATE,(unsigned char*)model_file,NULL);
									result = 1;
							}
						}
					}
				}
			}
			else
			{
				raise_warning(0,EC_INPUT_FILE_SIGNATURE_INCORRECT,(unsigned char*)model_file,NULL);
				result = 1;
			}
		}

		close(infile);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : connect_model
 * Desc : This function will connect the loaded model. It will attach all the
 *        messages and state transitions to the target state/nodes.
 *--------------------------------------------------------------------------------*/
static unsigned int	connect_model(GROUP* group_tree, unsigned short max_state, unsigned short max_node)
{
	unsigned int		result = EC_OK;
	unsigned short		count;
	NODE*				current_node;
	NODE*				function_start;
	NODE**				node_jump;
	GROUP*				current = group_tree;
	STATE*				current_state;
	STATE**				state_jump;
	TRIGGERS* 			current_triggers;
	TIMELINE*			current_timeline;
	STATE_TRANSITION*	current_trans;

	state_jump = malloc((1+max_state) * sizeof(STATE*));
	node_jump = calloc(1,(1+max_node) * sizeof(NODE*));

	/* ok, we have an open file */
	while (current != NULL)
	{
		if (current->state_machine != NULL)
		{
			current_state = current->state_machine->state_list;
	
			count = 0;

			/* build state jump table */
			while (current_state != NULL)
			{
				/* nodes will be in id order */
				state_jump[count++] = current_state;

				current_state = current_state->next;
			}

			current_state = current->state_machine->state_list;
			
			/* connect the states */
			while (current_state != NULL)
			{
				current_trans = current_state->transition_list;

				if (current_state->name_length > 0)
				{
					while(current_trans != NULL)
					{
						current_trans->next_state = state_jump[((unsigned long)current_trans->next_state)];
						current_trans = current_trans->next;
					}
				}
				current_state = current_state->next;
			}
		}
		
		if (current->sequence_diagram != NULL)
		{
			/* build the jump table first */
			count = 1;
			node_jump[0] = NULL;
			
			current_timeline = current->sequence_diagram->timeline_list;
			
			while (current_timeline != NULL)
			{
				current_node = current_timeline->node;

				while (current_node != NULL)
				{
					if (current_node->flags & FLAG_FUNCTION)
					{
						function_start = current_node;
					}
					else if (current_node->flags & FLAG_FUNCTION_END)
					{
						function_start->function_end = current_node;
						current_node->return_node = function_start;
					}

					node_jump[count] = current_node;
					count++;

					current_node = current_node->next;
				}

				current_timeline = current_timeline->next;
			}

			/* now join up the messages */
			current_timeline = current->sequence_diagram->timeline_list;

			while (current_timeline != NULL)
			{
				current_node = current_timeline->node;

				while (current_node != NULL)
				{
					if (current_node->sent_message != NULL)
					{
						if (node_jump[((unsigned long)current_node->sent_message->receiver)])
						{
							node_jump[((unsigned long)current_node->sent_message->receiver)]->received_message = current_node->sent_message;
							current_node->sent_message->target_timeline = node_jump[((unsigned long)current_node->sent_message->receiver)]->timeline;
						}
						current_node->sent_message->receiver = node_jump[((unsigned long)current_node->sent_message->receiver)];
					}
					current_node = current_node->next;
				}

				current_timeline = current_timeline->next;
			}
		}

		current = current->next;
	}

	free(node_jump);
	free(state_jump);

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : parse_input
 * Desc : This function will parse the input and locate the markers that we are
 *        interested in.
 *--------------------------------------------------------------------------------*/
unsigned int	parse_input(INPUT_STATE* input_state)
{
	unsigned int 			result = 1;
	unsigned int			keep_looking = 1;
	static unsigned char	model_name[] = "model:";
	static unsigned int		model_length = sizeof(model_name) - 1;

	/* default state - dump the data into the output */
	input_state->state = TYPE_TEXT;
	input_state->output_end = input_state->buffer_pos;
	input_state->output_start = input_state->buffer_pos;

	if (input_state->buffer_pos >= input_state->bytes_read)
	{
		result = 0;
	}
	else
	{
		while (keep_looking && input_state->buffer_pos < input_state->bytes_read)
		{
			switch(input_state->internal_state)
			{
				case INPUT_STATE_INTERNAL_SEARCHING:
					while(input_state->buffer_pos < input_state->bytes_read)
					{
						if (input_state->buffer[input_state->buffer_pos++] == '[')
						{
							input_state->model_pos = 0;
							input_state->internal_state = INPUT_STATE_INTERNAL_SCHEME;
							break;
						}
					}

					input_state->output_end = input_state->buffer_pos;
					break;

				case INPUT_STATE_INTERNAL_SCHEME:
					if (input_state->buffer[input_state->buffer_pos++] != model_name[input_state->model_pos++])
					{
						input_state->internal_state = INPUT_STATE_INTERNAL_SEARCHING;
					}

					if (input_state->model_pos == model_length)
					{
						/* next time around handle do this */
						input_state->internal_state = INPUT_STATE_INTERNAL_GROUP_COLLECT;
						input_state->item_length = 0;
						input_state->group_length = 0;

						/* remove the '[' from the output and exit and write the output */
						input_state->output_end--;
						keep_looking = 0;
					}
					break;

				case INPUT_STATE_INTERNAL_GROUP_COLLECT:
					if (is_valid_char[input_state->buffer[input_state->buffer_pos]])
					{
						input_state->group_name[input_state->group_length++] = input_state->buffer[input_state->buffer_pos];
					}
					else if (input_state->buffer[input_state->buffer_pos] == '/')
					{
						/* leading '/' are ignored but ones in the middle are separators */
						if (input_state->group_length > 0)
						{
							input_state->internal_state = INPUT_STATE_INTERNAL_TYPE_COLLECT;
							input_state->count = 0;
						}
					}
					else if (input_state->buffer[input_state->buffer_pos] == ']')
					{
						/* ok, we have finished looking at the name */
						input_state->internal_state = INPUT_STATE_INTERNAL_SEARCHING;
						input_state->output_end = input_state->buffer_pos;
						input_state->output_start = input_state->buffer_pos;
						keep_looking = 0;
					}
					else
					{
						input_state->internal_state = INPUT_STATE_INTERNAL_DUMP_TILL_END;
					}
					
					input_state->buffer_pos++;
					break;

				case INPUT_STATE_INTERNAL_TYPE_COLLECT:
					
					if (input_state->count == 0)
					{
						if ((input_state->buffer[input_state->buffer_pos] != 's') && (input_state->buffer[input_state->buffer_pos] != 'a'))
						{
							input_state->internal_state = INPUT_STATE_INTERNAL_DUMP_TILL_END;
						}
						else
						{
							input_state->count++;
						}
					}
					else if (input_state->count == 1)
					{
						if (input_state->buffer[input_state->buffer_pos] == 't')
						{
							input_state->temp_type = TYPE_STATE_MACHINE;
						}
						else if (input_state->buffer[input_state->buffer_pos] == 'p')
						{
							input_state->temp_type = TYPE_API;
						}
						else if (input_state->buffer[input_state->buffer_pos] == 'e')
						{
							input_state->temp_type = TYPE_SEQUENCE_DIAGRAM;
						}
						else if (input_state->buffer[input_state->buffer_pos] == 'a')
						{
							input_state->temp_type = TYPE_SAMPLE;
						}
						else
						{
							input_state->internal_state = INPUT_STATE_INTERNAL_DUMP_TILL_END;
						}

						input_state->count++;
					}
					else if (type_string[input_state->temp_type][input_state->count] == input_state->buffer[input_state->buffer_pos])
					{
						input_state->count++;
					}
					else if (type_length[input_state->temp_type] == input_state->count)
					{
						if (input_state->buffer[input_state->buffer_pos] == '/')
						{
							input_state->internal_state = INPUT_STATE_INTERNAL_ITEM_COLLECT;
							input_state->item_length = 0;
						}
						else if (input_state->buffer[input_state->buffer_pos] == ']')
						{
							input_state->state = input_state->temp_type;
							input_state->internal_state = INPUT_STATE_INTERNAL_SEARCHING;
							input_state->output_end = input_state->buffer_pos+1;
							input_state->output_start = input_state->buffer_pos+1;
							keep_looking = 0;
						}
						else
						{
							input_state->internal_state = INPUT_STATE_INTERNAL_DUMP_TILL_END;
						}
					}
					else
					{
						input_state->internal_state = INPUT_STATE_INTERNAL_DUMP_TILL_END;
					}
					
					input_state->buffer_pos++;
					break;

				case INPUT_STATE_INTERNAL_ITEM_COLLECT:
					if (is_valid_char[input_state->buffer[input_state->buffer_pos]])
					{
						input_state->item_name[input_state->item_length++] = input_state->buffer[input_state->buffer_pos];
					}
					else if (input_state->buffer[input_state->buffer_pos] == '/')
					{
						input_state->state = TYPE_TEXT;
						input_state->item_name[input_state->item_length++] = input_state->buffer[input_state->buffer_pos];
					}
					else if (input_state->buffer[input_state->buffer_pos] == ']')
					{
						/* ok, we have finished looking at the name */
						input_state->state = input_state->temp_type;
						input_state->internal_state = INPUT_STATE_INTERNAL_SEARCHING;
						input_state->output_end = input_state->buffer_pos+1;
						input_state->output_start = input_state->buffer_pos+1;
						keep_looking = 0;
					}
					else
					{
						input_state->internal_state = INPUT_STATE_INTERNAL_DUMP_TILL_END;
					}
					
					input_state->buffer_pos++;
					break;

				case INPUT_STATE_INTERNAL_DUMP_TILL_END:
					if (input_state->buffer[input_state->buffer_pos++] == ']')
					{
						/* ok, we have finished looking at the name */
						input_state->state = TYPE_TEXT;
						input_state->internal_state = INPUT_STATE_INTERNAL_SEARCHING;
						input_state->output_end = input_state->buffer_pos;
						input_state->output_start = input_state->buffer_pos;
					}
					break;
			}
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : process_input
 * Desc : This function will process the given input file and produce the genrated
 *        documents for the system.
 *--------------------------------------------------------------------------------*/
unsigned int	process_input(GROUP* group_tree, const char* file_name,const char* output_directory,unsigned int output_length)
{
	unsigned int	result = EC_OK;
	DRAW_STATE		draw_state;
	INPUT_STATE		input_state;

	input_state.input_name = (unsigned char*) file_name;

	if ((input_state.input_file = open(file_name,READ_FILE_STATUS)) == -1)
	{
		result = EC_PROBLEM_WITH_INPUT_FILE;
		raise_warning(0,result,(unsigned char*)file_name,NULL);
	}
	else
	{
		if ((result = output_open(&draw_state,(char*)file_name,(unsigned char*)output_directory,output_length)) == EC_OK)
		{
			while ((input_state.bytes_read = read(input_state.input_file,input_state.buffer,FILE_BLOCK_SIZE)) > 0)
			{
				input_state.buffer_pos = 0;

				/* now check the read in data for the markers that we are interested in */
				while(parse_input(&input_state))
				{
					/* action the found markers */
					switch (input_state.state)
					{
						case TYPE_TEXT:
							output_formats[draw_state.format].output_raw(	&draw_state,
																			&input_state.buffer[input_state.output_start],
																			input_state.output_end - input_state.output_start);
							break;

						case TYPE_STATE_MACHINE:
							generate_state_machine(&draw_state,&input_state,group_tree);
							break;

						case TYPE_SEQUENCE_DIAGRAM:
							generate_sequence_diagram(&draw_state,&input_state,group_tree);
							break;

						case TYPE_API:
							generate_api(&draw_state,&input_state,group_tree);
							break;

						case TYPE_SAMPLE:
							generate_sample(&draw_state,&input_state,group_tree);
							break;

						default:
							raise_warning(0,EC_INTERNAL_ERROR_INPUT_BAD_STATE,(unsigned char*)file_name,0);
					}
				}
			}

			output_close(&draw_state);
		}

		close(input_state.input_file);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : main
 *--------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
	int				quiet = 0;
	int				failed = 0;
	int				verbose = 0;
	char*			input_name = NULL;
	char*			output_name = "output";
	char*			error_param = "";
	char*			error_string = "";
	unsigned int	result = EC_OK;
	unsigned int	start = 1;
	unsigned int	output_length = 6;
	unsigned short	max_node;
	unsigned short	max_state;
	unsigned char*	param_mask;
	GROUP			group_tree;

	memset(&group_tree,0,sizeof(GROUP));

	param_mask = calloc(argc,1);

	if (argc < 2)
	{
		failed = 1;
	}
	else
	{
		/* first parse the command line and remove all the flags and the
		 * output files from the list and leave just the input file to
		 * be handled.
		 */
		do
		{
			if (argv[start][0] == '-')
			{
				/* mask the parameters that are not document files */
				param_mask[start] = 1;
				
				switch (argv[start][1])
				{
					case 'v':	/* verbose - add extra comments to the output */
						verbose = 1;
						break;

					case 'q':	/* quiet - suppress non error outputs */
						quiet = 1;
						break;

					case 'i':	/* input model file */
						if (argv[start][2] != '\0')
						{
							input_name = (char*) &argv[start][2];
							param_mask[start] = 1;
						}
						else if (((start + 1) < argc) && argv[start+1][0] != '-')
						{
							param_mask[start] = 1;
							start++;

							input_name = (char*) argv[start];
							param_mask[start] = 1;
						}
						else
						{
							error_string = "-i requires a file name\n";
							failed = 1;
						}
						break;

					case 'o':	/* output directory */
						if (argv[start][2] != '\0')
						{
							output_name = (char*) &argv[start][2];
							param_mask[start] = 1;
						}
						else if (((start + 1) < argc) && argv[start+1][0] != '-')
						{
							param_mask[start] = 1;
							start++;

							output_name = (char*) argv[start];
							param_mask[start] = 1;
						}
						else
						{
							error_string = "-o requires a file name\n";
							failed = 1;
						}
						break;

					case '?':	/* help  - just fail! */
						failed = 1;
						break;

					default:
						failed = 1;
						error_string = "Unknown parameter\n";
				}		
			}

			start++;
		}
		while(start < argc);
	}

	if (failed)
	{
		/* handle failure */
	}
	else
	{
		/* input the model to process */
		if ((result = input_model(input_name,&group_tree,&max_state,&max_node)) == EC_OK)
		{
			/* connect the model */
			if ((result = connect_model(&group_tree,max_state,max_node)) == EC_OK)
			{
				GROUP* temp = find_group(&group_tree,(unsigned char*)"",0);
				
				/* name the default group "default" */
				memcpy(temp->name,"default",7);
				temp->name_length = 7;

				for (start=1; start < argc; start++)
				{
					if (param_mask[start] == 0)
					{
						result = process_input(&group_tree,argv[start],output_name,output_length);
					}
				}
			}
		}
	}

	if (failed || result != EC_OK)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}

