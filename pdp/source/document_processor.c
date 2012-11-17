/**-----------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * @application	pdp
 * @section		Description
 * 				This utility is the document processor.
 *         		It will handle the processing of the structural documents with the
 *         		data recovered from the source files.
 * 
 *         		The document processor will take load a model file that have been
 *              linked with the *pdsl* application and one or more source document
 *              files. These file hold the structured document that the processor
 *              will add the elements from the module into.
 * 
 *              The processor expects the source file to be markdown formatted.
 *              *note* this application does not strictly follow the markdown
 *              specification yet, but it will. It may extend the format to be
 *              able to add new features. See *doc_gen_markdown* for details.
 *
 *              Markdown has been chosen as it is a simple markup language that
 *              leaves the original text in an almost readable state. The syntax
 *              marking does not stand out too much in the text.
 *
 * @synopsis	all		-v, -i ,-o,-?, files
 *
 * @section		Error Reporting
 * 				Please report any problems to https://github.com/PAntoine/docgen
 *
 * @section		Author 
 * 				Peter Antoine
 * @ignore
 *--------------------------------------------------------------------------------*
 *                    Copyright (c) 2012 Peter Antoine
 *                           All rights Reserved.
 *                   Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include "atoms.h"
#include "output.h"
#include "utilities.h"
#include "error_codes.h"
#include "lookup_tables.h"
#include "output_format.h"
#include "parse_markdown.h"
#include "indexing_format.h"
#include "document_generator.h"

extern char* g_input_filename;

extern unsigned char	is_valid_char[];

static unsigned char	default_config_dir[] = ".doc_gen";
static unsigned int		default_config_dir_len = sizeof(default_config_dir) - 1;

/*--------------------------------------------------------------------------------*
 * dummy format for handling indexing the input files.
 *--------------------------------------------------------------------------------*/
static OUTPUT_FORMAT	index_format = 
{
	(unsigned char*)"index",
	5,
	index_decode_flags,
	index_open,
	index_close,
	index_header,
	index_footer,
	index_raw,
	index_sample,
	index_timelines,
	index_message,
	index_state_set_size,
	index_state,
	index_transition,
	index_marker,
	index_text,
	index_section,
	index_title,
	index_block,
	index_table_start,
	index_table_header,
	index_table_row,
	index_table_end,
	index_index_chapter,
	index_index_start_sublevel,
	index_index_entry,
	index_index_end_sublevel,
	index_list_item_start,
	index_list_numeric_start,
	index_list_end,
	NULL
};

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
	name->fixed			= buffer[offset+1];
	name->name_length	= (((unsigned short)buffer[offset+2]) << 8) | buffer[offset+3];
	name->name = &buffer[offset+4];

	return (offset + 4 + name->name_length);
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
	string->fixed		= buffer[offset + 1];
	value->name_length	= buffer[offset + 2];
	string->name_length	= ((unsigned short)buffer[offset + 3] << 8) | buffer[offset + 4];

	/* get the strings */
	value->name = &buffer[offset + 5];
	string->name = &buffer[offset + 5 + value->name_length];

	return (offset + 5 + value->name_length + string->name_length);
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
	name->fixed			= 0;
	value->fixed		= 0;
	brief->fixed		= buffer[offset + 1];
	type->name_length	= buffer[offset + 2];
	name->name_length	= buffer[offset + 3];
	value->name_length	= ((unsigned short)buffer[offset + 4] << 8) | buffer[offset + 5];
	brief->name_length	= ((unsigned short)buffer[offset + 6] << 8) | buffer[offset + 7];

	/* get the strings */
	type->name = &buffer[offset + 8];
	name->name = &buffer[offset + 8 + type->name_length];
	value->name = &buffer[offset + 8 + type->name_length + name->name_length];
	brief->name = &buffer[offset + 8 + type->name_length + name->name_length + value->name_length];

	return (offset + 8 + type->name_length + name->name_length + brief->name_length + value->name_length);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_option_record
 * Desc : This function reads an option record.
 *        {short:flags,string:name,string:value,string:description}
 *--------------------------------------------------------------------------------*/
static unsigned int	read_option_record(	unsigned int	offset, 
										unsigned char*	buffer,
										unsigned short*	flags,
										NAME*			name,
										NAME*			value,
										NAME*			description)
{
	/* get the lengths */
	*flags						= ((unsigned short)buffer[offset + 1] << 8) | buffer[offset + 2];
	name->name_length			= ((unsigned short)buffer[offset + 4] << 8) | buffer[offset + 5];
	value->name_length			= ((unsigned short)buffer[offset + 6] << 8) | buffer[offset + 7];
	description->name_length	= ((unsigned short)buffer[offset + 8] << 8) | buffer[offset + 9];

	if (buffer[offset + 3] & 0x01)
	{
		name->fixed = 1;
	}

	if (buffer[offset + 3] & 0x02)
	{
		value->fixed = 1;
	}

	if (buffer[offset + 3] & 0x04)
	{
		description->fixed = 1;
	}

	/* get the strings */
	name->name = &buffer[offset + 10];
	value->name = &buffer[offset + 10 + name->name_length];
	description->name = &buffer[offset + 10 + name->name_length + value->name_length];

	return (offset + 10 + name->name_length + description->name_length + value->name_length);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_synopsis_record
 * Desc : This function reads a synopsis record.
 *        {string:name,byte_list:(option_index)}
 *--------------------------------------------------------------------------------*/
static unsigned int	read_synopsis_record(	unsigned int	offset, 
											unsigned char*	buffer,
											unsigned short*	index_length,
											unsigned char**	index,
											NAME*			name)
{
	/* get the lengths */
	name->name_length			= ((unsigned short)buffer[offset + 1] << 8) | buffer[offset + 2];
	*index_length				= ((unsigned short)buffer[offset + 3] << 8) | buffer[offset + 4];

	/* get the strings */
	name->name	= &buffer[offset + 5];
	*index		= &buffer[offset + 5 + name->name_length];

	return (offset + 5 + name->name_length + *index_length);
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

	brief->fixed	 	= buffer[offset + 1];
	type->name_length	= buffer[offset + 2];
	name->name_length	= buffer[offset + 3];
	brief->name_length	= ((unsigned short)buffer[offset + 4] << 8) | buffer[offset + 5];

	/* get the strings */
	type->name = &buffer[offset + 6];
	name->name = &buffer[offset + 6 + type->name_length];
	brief->name = &buffer[offset + 6 + type->name_length + name->name_length];

	return (offset + 6 + type->name_length + name->name_length + brief->name_length);
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
static API_CONSTANTS*	add_api_constants(GROUP* group, NAME* name, NAME* description)
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
static void	add_api_constant(API_CONSTANTS* api_constants, NAME* name, NAME* type, NAME* value, NAME* brief)
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
static API_TYPE*	add_api_type(GROUP* group, NAME* name, NAME* description)
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
static void	add_api_type_record(API_TYPE* api_type, unsigned int record_type, NAME* name, NAME* type,  NAME* brief)
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

	if (function->max_return_value_length < result->value.name_length)
	{
		function->max_return_value_length = result->value.name_length;
	}

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
 * @name: find_add_application
 * @desc: This function will add an application to the GROUP tree. 
 *        The application only lives in the root node as the application are
 *        not grouped.
 *--------------------------------------------------------------------------------*/
static APPLICATION*	find_add_application(GROUP* group, NAME* name)
{
	APPLICATION* result;

	result = group->application;

	/* find the application if it exists */
	while (result != NULL)
	{
		if (compare_name(name,&result->name) == 0)
		{
			break;
		}
		result = result->next;
	}

	if (result == NULL)
	{
		/* add a new application */
		result = calloc(1,sizeof(APPLICATION));

		/* set up the parameter */
		copy_name(name,&result->name);

		if (group->application == NULL)
		{
			group->application = result;
		}
		else
		{
			result->next = group->application;
			group->application = result;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: find_section
 * @desc: This function will find a section in the given application.
 *--------------------------------------------------------------------------------*/
static SECTION*	find_section(SECTION* section_list, NAME* name)
{
	SECTION* result = NULL;
	SECTION* current_section = section_list;

	while (current_section != NULL)
	{
		if (compare_name(name,&current_section->name) == 0)
		{
			/* found it */
			result = current_section;
			break;
		}

		current_section = current_section->next;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: add_app_section
 * @desc: This function will add a section to the current application.
 *--------------------------------------------------------------------------------*/
static SECTION*	add_app_section(APPLICATION* application,NAME* name, NAME* section_data)
{
	SECTION* temp;
	SECTION* result = NULL;
	
	if (application != NULL)
	{
		if ((result = find_section(application->section_list,name)) == NULL)
		{
			result = calloc(1,sizeof(SECTION));

			if (application->section_list == NULL)
			{
				application->section_list = result;
				application->section_list->last = result;
			}
			else
			{
				application->section_list->last->next = result;
				application->section_list->last = result;
			}
		}

		if (result != NULL)
		{
			copy_name(name,&result->name);
			copy_name(section_data,&result->section_data);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: add_app_sub_section
 * @desc: This function will add a sub-section.
 *--------------------------------------------------------------------------------*/
static void	add_app_sub_section(SECTION* section,NAME* name,NAME* section_data)
{
	SECTION*	sub_section;
	SECTION*	new_sub_section;

	if (section != NULL)
	{
		if ((sub_section = find_section(section->sub_section_list,name)) == NULL)
		{
			sub_section = calloc(1,sizeof(SECTION));

			if (section->sub_section_list == NULL)
			{
				section->sub_section_list = sub_section;
				section->sub_section_list->last = sub_section;
			}
			else
			{
				section->sub_section_list->last->next = sub_section;
				section->sub_section_list->last = sub_section;
			}
		}

		if (sub_section != NULL)
		{
			copy_name(name,&sub_section->name);
			copy_name(section_data,&sub_section->section_data);
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: add_app_option
 * @desc: This function will add an option to the application list.
 *--------------------------------------------------------------------------------*/
static void	add_app_option(APPLICATION* application,unsigned short option_flags,NAME* name,NAME* value,NAME* description)
{
	OPTION*	new_option;

	if (application != NULL)
	{
		new_option = calloc(1,sizeof(OPTION));

		if (application->option_list == NULL)
		{
			application->max_option_length = 4;
			application->option_list = new_option;
			application->option_list->last = new_option;
		}
		else
		{
			new_option->option_id = application->option_list->last->option_id + 1;
			application->option_list->last->next = new_option;
			application->option_list->last = new_option;
		}

		if (new_option != NULL)
		{
			if (name->name_length > application->max_option_length)
			{
				application->max_option_length = name->name_length;
			}

			new_option->flags = option_flags;
			copy_name(name,&new_option->name);
			copy_name(value,&new_option->value);
			copy_name(description,&new_option->description);
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: add_app_command
 * @desc: This funciton will add a command to the application.
 *--------------------------------------------------------------------------------*/
static void	add_app_command(APPLICATION* application,NAME* name,NAME* parameters,NAME* description)
{
	COMMAND*	new_command;

	if (application != NULL)
	{
		new_command = calloc(1,sizeof(COMMAND));

		if (application->command_list == NULL)
		{
			application->max_command_length = 4;
			application->command_list = new_command;
			application->command_list->last = new_command;
		}
		else
		{
			application->command_list->last->next = new_command;
			application->command_list->last = new_command;
		}

		if (new_command != NULL)
		{
			if (name->name_length > application->max_command_length)
			{
				application->max_command_length = name->name_length;
			}

			copy_name(name,&new_command->name);
			copy_name(parameters,&new_command->parameters);
			copy_name(description,&new_command->description);
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: add_app_synopsis
 * @desc: This function will add a synopsis to the list of synopsis in the
 *        application.
 *
 *        TODO: This is ridiculous MUST build an index when we load the option
 *              save having to do this recursive search rubbish.
 *--------------------------------------------------------------------------------*/
static void	add_app_synopsis(APPLICATION* application,NAME* name,unsigned char* index,unsigned short index_length)
{
	OPTION*			current_option;
	SYNOPSIS*		new_synopsis = calloc(1,sizeof(SYNOPSIS));
	unsigned int	index_item;
	unsigned int	cur_index;

	copy_name(name,&new_synopsis->name);
	new_synopsis->list_length = index_length;
	new_synopsis->list = malloc(sizeof(OPTION*)*index_length);
	
	/* now lookup all the options */
	for (index_item = 0; index_item < index_length; index_item++)
	{
		current_option = application->option_list;
		cur_index = 0;

		while (current_option != NULL)
		{
			if (cur_index == index[index_item])
			{
				new_synopsis->list[index_item] = current_option;
				break;
			}

			cur_index++;
			current_option = current_option->next;
		}
	}

	/* now add it to the list of synopsis */
	if (application->synopsis == NULL)
	{
		application->synopsis = new_synopsis;
		application->synopsis->last = new_synopsis;
	}
	else
	{
		application->synopsis->last->next = new_synopsis;
		application->synopsis->last = new_synopsis;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : input_model
 * Desc : This function will input the model.
 *--------------------------------------------------------------------------------*/
static unsigned int	input_model(char* model_file, GROUP* group_tree,unsigned short *max_state, unsigned short* max_node)
{
	int					infile;
	unsigned int		all_groups;
	unsigned int		state = MODEL_LOAD_UNKNOWN;
	unsigned int		offset = 0;
	unsigned int		result = 0;
	unsigned int		flags;
	unsigned short		index_length;
	unsigned short		id;
	unsigned short		option_flags;
	unsigned char		sender_id = 0;
	unsigned char		receiver_id = 0;
	unsigned char		signature[4] = LINKED_SOURCE_MAGIC;
	unsigned char		record[FILE_BLOCK_SIZE];
	unsigned char*		index;
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
	SECTION*			current_section;
	TIMELINE*			current_timeline;
	API_TYPE*			current_type;
	APPLICATION*		current_application;
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
									if ((current_group = find_group(group_tree,name.name,name.name_length,&all_groups)) == NULL)
									{
										current_group = add_group(group_tree,name.name,name.name_length);
									}
									break;

								case LINKER_SAMPLE:
									offset = read_pair_record(offset,record,&name,&brief);
									add_sample(group_tree,&name,&brief);
									break;
								
								case LINKER_APPLICATION_START:
									state = MODEL_LOAD_APPLICATION;
									offset = read_string_record(offset,record,&name);
									current_application = find_add_application(group_tree,&name);

									break;

								case LINKER_BLOCK_END:
									offset = FILE_BLOCK_SIZE;
									break;

								default:
									hex_dump(&record[offset],16);
									raise_warning(0,EC_UNEXPECTED_ITEM_IN_STATE,(unsigned char*)model_file,NULL);
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
									printf("api: %d\n",record[offset]);
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
									
									if ((current_group = find_group(group_tree,group.name,group.name_length,&all_groups)) == NULL)
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
									
									if ((temp_group = find_group(group_tree,group.name,group.name_length,&all_groups)) == NULL)
									{
										temp_group = add_group(group_tree,group.name,group.name_length);
									}
		
									add_trigger(current_transition,temp_group,&name);
									
									break;

								case LINKER_TRIGGERS:
									/* add the triggers to the list of triggers to the current state */
									offset = read_group_record(offset,record,&group,&name);

									if ((temp_group = find_group(group_tree,group.name,group.name_length,&all_groups)) == NULL)
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
									raise_warning(0,EC_UNEXPECTED_ITEM_IN_STATE,(unsigned char*)model_file,NULL);
									result = 1;
							}
						}

						else if (state == MODEL_LOAD_APPLICATION)
						{
							switch(record[offset])
							{
								case LINKER_APPLICATION_SECTION:
									offset = read_pair_record(offset,record,&name,&description);
									current_section = add_app_section(current_application,&name,&description);
									break;

								case LINKER_APPLICATION_SUB_SECTION:
									offset = read_pair_record(offset,record,&name,&description);
									add_app_sub_section(current_section,&name,&description);
									break;

								case LINKER_APPLICATION_OPTION:
									offset = read_option_record(offset,record,&option_flags,&name,&value,&description);
									add_app_option(current_application,option_flags,&name,&value,&description);
									break;

								case LINKER_APPLICATION_COMMAND:
									offset = read_type_record(offset,record,&name,&value,&description);
									add_app_command(current_application,&name,&value,&description);
									break;

								case LINKER_APPLICATION_SYNOPSIS:
									offset = read_synopsis_record(offset,record,&index_length,&index,&name);
									add_app_synopsis(current_application,&name,index,index_length);
									break;

								case LINKER_APPLICATION_END:
									state = MODEL_LOAD_UNKNOWN;
									offset++;
								break;

								case LINKER_BLOCK_END:
									offset = FILE_BLOCK_SIZE;
									break;

								default:
									hex_dump(&record[offset],16);
									raise_warning(0,EC_UNEXPECTED_ITEM_IN_STATE,(unsigned char*)model_file,NULL);
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
									
									if ((current_group = find_group(group_tree,group.name,group.name_length,&all_groups)) == NULL)
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
									raise_warning(0,EC_UNEXPECTED_ITEM_IN_STATE,(unsigned char*)model_file,NULL);
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
	else
	{
		raise_warning(0,EC_FAILED_TO_OPEN_INPUT_FILE,(unsigned char*)model_file,NULL);
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
 * Name : process_input
 * Desc : This function will process the input.
 *--------------------------------------------------------------------------------*/
static unsigned int	process_input(GROUP* group_tree, DRAW_STATE* draw_state, const char* file_name, char* resource_path, int resource_path_length,const char* output_directory,unsigned int output_length)
{
	unsigned int	pos = 0;
	unsigned int	format_size = 0;
	unsigned int	result = EC_OK;
	unsigned int	special_group;
	unsigned int	chapter_name_length;
	unsigned char*	chapter_name;
	INPUT_STATE		input_state;
	GROUP*			group;

	memset(&input_state,0,sizeof(input_state));

	input_state.input_name = (unsigned char*) file_name;

	draw_state->resources_path.name			= (unsigned char*) resource_path;
	draw_state->resources_path.name_length	= resource_path_length;

	draw_state->model = group_tree;
				
	level_index_set_level(&draw_state->index,0);
	
	if (draw_state->format == &index_format || (result = output_open(draw_state,&input_state,(char*)file_name,(unsigned char*)output_directory,output_length)) == EC_OK)
	{
		if (draw_state->format == &index_format)
		{
			/* don't want to create a directory tree for the index - but do need to call the open */
			get_filename(file_name,&chapter_name,&chapter_name_length);
			result = draw_state->format->output_open(draw_state,&input_state,chapter_name,chapter_name_length);
		}

		result = markdown_parse_input(draw_state,&input_state);

		output_close(draw_state,&input_state);
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
	char 			resource_path[MAX_FILENAME];
	char*			equals = NULL;
	char*			input_name = NULL;
	char*			output_name = "output";
	char*			error_param = "";
	char*			error_string = "";
	char*			resource_name = NULL;
	unsigned int	count = 0;
	unsigned int	start = 1;
	unsigned int	file_number = 0;
	unsigned int	result = EC_OK;
	unsigned int	format_pos = 0;
	unsigned int	special_group;
	unsigned int	output_length = 6;
	unsigned int	resource_length = 0;
	unsigned int	format_list_length = 5;
	unsigned int	resource_path_length = 0;
	unsigned short	max_node;
	unsigned short	max_state;
	unsigned char*	format_list_string = (unsigned char*) "text";
	unsigned char*	param_mask;
	GROUP			group_tree;
	DRAW_STATE		draw_state;

	memset(&group_tree,0,sizeof(GROUP));
	memset(&draw_state,0,sizeof(DRAW_STATE));

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
					/**-----------------------------------------------------------*
					 * @option	-v
					 *
					 * @description 
					 * This produces extra output information during the generation
					 * of the output.
					 *------------------------------------------------------------*/
					case 'v':
						verbose = 1;
						break;

					/**-----------------------------------------------------------*
					 * @option	-q
					 *
					 * @description
					 * quiet. This flag suppresses any non-error output from the
					 * application.
					 *------------------------------------------------------------*/
					case 'q':
						quiet = 1;
						break;

					/**-----------------------------------------------------------*
					 * @option		-i
					 * @value		<input_model_file_name>
					 * @required	yes
					 *
					 * @description
					 * input model filename. This is the linked model file that
					 * is used to generate the final documents.
					 *------------------------------------------------------------*/
					case 'i':
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

					/**-----------------------------------------------------------*
					 * @option	-o
					 * @value	<output directory name>
					 *
					 * @description
					 * This names the directory that the output id placed in. If
					 * this parameter is not given then the application defaults
					 * to 'output'.
					 *------------------------------------------------------------*/
					case 'o':
						if (argv[start][2] != '\0')
						{
							output_name = (char*) &argv[start][2];
							output_length = strlen(output_name);
							param_mask[start] = 1;
						}
						else if (((start + 1) < argc) && argv[start+1][0] != '-')
						{
							param_mask[start] = 1;
							start++;

							output_name = (char*) argv[start];
							output_length = strlen(output_name);
							param_mask[start] = 1;
						}
						else
						{
							error_string = "-o requires a file name\n";
							failed = 1;
						}
						break;

					/**-----------------------------------------------------------*
					 * @option	-d
					 * @value	<install directory name>
					 *
					 * @description
					 * This names the directory that the resource files and other
					 * support files are found in. If this parameter is not set
					 * then it will default to 'HOME/.doc_gen'.
					 *------------------------------------------------------------*/
					case 'd':
						if (argv[start][2] != '\0')
						{
							resource_name = (char*) &argv[start][2];
							resource_length = strlen(resource_name);
							param_mask[start] = 1;
						}
						else if (((start + 1) < argc) && argv[start+1][0] != '-')
						{
							param_mask[start] = 1;
							start++;

							resource_name = (char*) argv[start];
							resource_length = strlen(resource_name);
							param_mask[start] = 1;
						}
						else
						{
							error_string = "-d requires a file name\n";
							failed = 1;
						}
						break;

					/**-----------------------------------------------------------*
					 * @option	-f
					 * @value	<format_required>[,<format_required>[,...]]
					 *
					 * @description
					 * This option will specify which formats that the document
					 * will be produced in. If this option is not spefied it will
					 * default to "text". If this format is specifed and you 
					 * require text then this will have to be added to the format 
					 * line.
					 *
					 * The formats (with the exception of "text" that is built in)
					 * are the plugins found in the plugin directory. If the format
					 * is not found then the application will fail and no documents
					 * will be produced.
					 *------------------------------------------------------------*/
					case 'f':
						if (argv[start][2] != '\0')
						{
							format_list_string = (unsigned char*) &argv[start][2];
							format_list_length = strlen(&argv[start][2]);
							param_mask[start] = 1;
						}
						else if (((start + 1) < argc) && argv[start+1][0] != '-')
						{
							param_mask[start] = 1;
							start++;

							format_list_string = (unsigned char*) argv[start];
							format_list_length = strlen(argv[start]);
							param_mask[start] = 1;
						}
						else
						{
							error_string = "-d requires a list of formats\n";
							failed = 1;
						}
						break;

					/**-----------------------------------------------------------*
					 * @option		-D
					 * @value		MACRO_NAME=MACRO_VALUE
					 * @required	no
					 * @multiple	yes
					 *
					 * @description
					 * The macro value is used to allow for strings passed into
					 * the build to be substituted within some of the symbol names.
					 * This allows for the source document to be used to generate
					 * documents for different build options.
					 *
					 * The \$MACRO_NAME that is placed within the document will be
					 * replaced with the value that is passed in at run-time.
					 *------------------------------------------------------------*/
					case 'D':
						if (argv[start][2] != '\0')
						{
							if ((equals = index(&argv[start][2],'=')) != NULL)
							{
								if (equals[1] != '\0')
								{
									add_lookup(&draw_state.macro_lookup,&argv[start][2],equals-&argv[start][2],&equals[1],strlen(&equals[1]),0);
								}
								else if (argv[start+1][0] != '-')
								{
									add_lookup(&draw_state.macro_lookup,&argv[start][2],equals-&argv[start][2],&argv[start+1][0],strlen(&argv[start+1][0]),0);
								}
								else
								{
									failed = 1;
									error_string = "bad format for -D";
								}
							}
							else if (argv[start+1][0] == '=')
							{
								if ((argv[start+1][1] == '\0') && argv[start+2][0] != '-')
								{
									add_lookup(&draw_state.macro_lookup,argv[start],strlen(argv[start]),argv[start+2],strlen(argv[start+2]),0);
								}
								else if ((argv[start+1][0] == '=') && (argv[start+1][1] != '\0'))
								{
									add_lookup(&draw_state.macro_lookup,argv[start],strlen(argv[start]),&argv[start+1][1],strlen(&argv[start+1][1]),0);
								}
								else
								{
									failed = 1;
									error_string = "bad format for -D";
								}
							}
						}
						else if (argv[start+1][0] != '-')
						{
							if ((argv[start+2][0] == '=') && (argv[start+2][1] == '\0') && argv[start+3][0] != '-')
							{
								add_lookup(&draw_state.macro_lookup,argv[start+1],strlen(argv[start+1]),argv[start+3],strlen(argv[start+3]),0);
							}
							else if ((argv[start+2][0] == '=') && (argv[start+2][1] != '\0'))
							{
								add_lookup(&draw_state.macro_lookup,argv[start+1],strlen(argv[start+1]),&argv[start+2][1],strlen(&argv[start+2][1]),0);
							}
							else
							{
								failed = 1;
								error_string = "bad format for -D";
							}
						}
						else
						{
							error_string = "-D requires a macro name\n";
							failed = 1;
						}
						break;


					/**-----------------------------------------------------------*
					 * @option		-?
					 *
					 * @description
					 * This outputs the help message.
					 *------------------------------------------------------------*/
					case '?':
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

	if (verbose)
	{
		printf( "\n       %s version %d.%d - Copyright (c) 2012 Peter Antoine\n"
				"All Rights reserved -- Released under the conditions of the Artistic Licence\n\n",
				argv[0],VERSION_MAJOR,VERSION_MINOR);
	}

	if (failed)
	{
		/* handle failure */
	}
	else
	{

		/* set-up the path to the resources directory */
		if (resource_name == NULL)
		{
			/* use the default */
			resource_name = getenv("HOME");
			resource_path_length = strlen(resource_name);

			if ((resource_path_length + default_config_dir_len + 2) >= MAX_FILENAME)
			{
				/* stops buffer overrun - and other nasty hacks */
				resource_path[0] = '.';
				resource_path_length = 1;
			}
			else
			{
				memcpy(resource_path,resource_name,resource_path_length);
			}

			resource_path[resource_path_length++] = '/';
			memcpy(&resource_path[resource_path_length],default_config_dir,default_config_dir_len);
			resource_path_length += default_config_dir_len;
		}
		else
		{
			/* use the supplied */
			resource_path_length = strlen(resource_name);
			memcpy(resource_path,resource_name,resource_path_length);
		}

		/* init the output system */
		output_initialise(resource_path,resource_path_length);

		/* check to see if all the requested formats are supported */
		if (format_list_string != NULL)
		{
			format_pos = 0;

			for (count=0;count<format_list_length;count++)
			{
				if (format_list_string[count] == '\0' || format_list_string[count] == ',')
				{
					if (output_find_format(&format_list_string[format_pos],count - format_pos) == NULL)
					{
						format_list_string[count] = '\0';

						raise_warning(0,EC_UNSUPPORTED_OUTPUT_FORMAT,(unsigned char*)&format_list_string[format_pos],NULL);
						failed = 1;
					}

					format_pos = count+1;
				}
			}
		}

		/* input the model to process */
		if (failed == 0 && (result = input_model(input_name,&group_tree,&max_state,&max_node)) == EC_OK)
		{
			/* connect the model */
			if ((result = connect_model(&group_tree,max_state,max_node)) == EC_OK)
			{
				GROUP* temp = find_group(&group_tree,(unsigned char*)"",0,&special_group);

				/* name the default group "default" */
				memcpy(temp->name,"default",7);
				temp->name_length = 7;

				/**-----------------------------------------------------------*
				 * @option 		files
				 * @required	yes
				 * @multiple	yes
				 *
				 * @description
				 * One or more document files to produce output for. This file
				 * contains the markup that will be processed and will be used
				 * as the structure for producing the output.
				 *------------------------------------------------------------*/

				/* build the index for documents passed in */
				level_index_init(&draw_state.index);
	
				draw_state.format = &index_format;

				if (verbose)
				{
					printf("Building index for:\n");
				}

				for (start=1; start < argc; start++)
				{
					if (param_mask[start] == 0)
					{
						if (verbose)
						{
							printf("    %s\n",argv[start]);
						}
						result = process_input(&group_tree,&draw_state,argv[start],resource_path,resource_path_length,output_name,output_length);
					}
				}

				level_index_release(&draw_state.index);

				/* now produce the documentation */
				if (result == EC_OK)
				{
					format_pos = 0;
	
					/* need and index for the sections */
					level_index_init(&draw_state.index);

					/* re-parsing the string - it's not big easier to do it twice */
					for (count=0;count<format_list_length+1;count++)
					{
						if (format_list_string[count] == '\0' || format_list_string[count] == ',')
						{
							/* is the format valid */
							if ((draw_state.format = output_find_format(&format_list_string[format_pos],count - format_pos)) != NULL)
							{
								file_number = 0;
							
								if (verbose)
								{
									printf("Producing format %s\n",draw_state.format->name);
								}

								/* now run the all files again for the selected format */
								for (start=1; start < argc; start++)
								{
									if (param_mask[start] == 0)
									{
										file_number++;
										draw_state.chapter = file_number;

										result = process_input(&group_tree,&draw_state,argv[start],resource_path,resource_path_length,output_name,output_length);
									}
								}
							}
							format_pos = count+1;
						}
					}

					level_index_release(&draw_state.index);
				}
			}
		}

		/* release the plugins */
		output_release();
	}

	if (failed || result != EC_OK)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}

