/**-------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : document_linker
 * Desc  : This file holds the code the handles linking the source files that
 *         have been compiled with the document source compiler.
 *
 * Author: pantoine
 * Date  : 15/11/2011 08:59:42
 * 
 * @application	pdsl
 *
 * @section		Description
 *
 * Document Source Linker
 *
 * This application will take all the specified input_files that have been
 * generated from the document compiler *pdsc* and link them together as a 
 * single object file.
 *
 * This object will have all the components for each of the compiled source
 * files, it will link all objects that are either grouped together or reference
 * each other (i.e. state_machines or sequence_diagrams).
 *
 * It produces an output file that can be passed to the document processor (*pdp*)
 * that can be used as the model to be used to generate final documents.
 *
 * @synopsis	all		-v, -q, -o, input_files
 *
 * @section		Error Reporting
 * 				Please report any problems to https://github.com/PAntoine/docgen
 *
 * @section		Author 
 * 				Peter Antoine
 *
 * @ignore
*--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "utilities.h"
#include "error_codes.h"
#include "document_generator.h"

static unsigned char	record_buffer[MAX_RECORD_SIZE];

extern unsigned char*	g_input_filename;
extern unsigned char*	g_source_filename;
extern unsigned int		g_source_filename_length;

unsigned char	finish_state[] = "finish";
unsigned int	finish_state_length = sizeof(finish_state);

/*--------------------------------------------------------------------------------*
 * The linking options
 *--------------------------------------------------------------------------------*/
static unsigned int	g_max_call_depth	= 10;

/*--------------------------------------------------------------------------------*
 * Linking Structures
 *--------------------------------------------------------------------------------*/
GROUP 				g_group_tree;
static SAMPLE		g_sample_list;
static FUNCTION		g_function_list;
static APPLICATION	g_application_list;

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : test_walk_node_tree
 * Desc : This function will walk the tree as a test.
 *--------------------------------------------------------------------------------*/
static void	test_walk_node_tree(NODE* start)
{
	NODE	walk_start;
	NODE*	search_node;
	NODE*	active_node;

	memset(&walk_start,0,sizeof(NODE));
	walk_start.next = start;

	search_node = &walk_start;
	
	do
	{
		search_node = search_next_node(search_node,&active_node);
		
		if (active_node != NULL)
		{
			do
			{
				active_node = next_active_node(active_node);
			}
			while (active_node != NULL && search_node != active_node);
		}
	}
	while (search_node != NULL);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_trigger
 * Desc : This function will find the trigger in the given groups trigger list.
 *--------------------------------------------------------------------------------*/
static TRIGGER* find_trigger ( GROUP* group, unsigned char* name, unsigned int name_length )
{
	TRIGGER*	result = NULL;
	TRIGGER*	current_trigger = NULL;

	if (group != NULL)
	{
		if (group->trigger_list != NULL)
		{
			current_trigger = group->trigger_list;

			do
			{
				if (current_trigger->name_length == name_length && memcmp(current_trigger->name,name,name_length) == 0)
				{
					result = current_trigger;
					break;
				}

				current_trigger = current_trigger->next;
			}
			while (current_trigger != NULL);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_trigger
 * Desc : This function will add the trigger to the trigger list. It will not check to
 *        see if the trigger already exists, so this function MUST be called after
 *        the find has been called or duplicate triggers will occur and the results
 *        will be unpredictable.
 *--------------------------------------------------------------------------------*/
static TRIGGER* add_trigger ( GROUP* group, unsigned char* name, unsigned int name_length, unsigned int flags )
{
	TRIGGER*	result = NULL;
	TRIGGER*	current_trigger = NULL;

	if (group != NULL)
	{
		if (group->trigger_list == NULL)
		{
			/* ok, first trigger */
			group->trigger_list = calloc(1,sizeof(TRIGGER));
			current_trigger = group->trigger_list;
		}
		else
		{
			current_trigger = group->trigger_list;

			while (current_trigger != NULL)
			{
				if (current_trigger->next == NULL)
				{
					current_trigger->next = calloc(1,sizeof(TRIGGER));
					current_trigger = current_trigger->next; 
					break;
				}

				current_trigger = current_trigger->next;
			}
		}

		/* Ok, current trigger is the new trigger - now fill it in */
		current_trigger->flags = flags;
		current_trigger->name_length = name_length;
		current_trigger->group = group;

		memcpy(current_trigger->name,name,name_length);
		
		result = current_trigger;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_triggers
 * Desc : This function will add the triggers to the transition. It will look down
 *        a TRIGGERS_LIST item and find all the triggers that are defined in that
 *        list and add them.
 *--------------------------------------------------------------------------------*/
static void	add_triggers(TRIGGERS** triggers, BLOCK_NAME* triggers_list, GROUP* group)
{
	GROUP*		owing_group = NULL;
	TRIGGER*	trigger = NULL;
	BLOCK_NAME*	current = triggers_list;

	while(current != NULL)
	{
		if (current->owner.group == NULL)
		{
			current->owner.group = group;
		}

		if ((trigger = find_trigger(current->owner.group,current->name,current->name_length)) == NULL)
		{
			/* add the new trigger */
			trigger = add_trigger(current->owner.group,current->name,current->name_length,0);
		}
		else
		{
			/* make sure the ghost flag is clear */
			trigger->flags &= ~FLAG_GHOST;
		}

		if (*triggers == NULL)
		{
			*triggers = calloc(1,sizeof(TRIGGERS));
			(*triggers)->trigger = trigger;
		}
		else
		{
			TRIGGERS* current_triggers = *triggers;

			do
			{
				if (current_triggers->next == NULL)
				{
					current_triggers->next = calloc(1,sizeof(TRIGGERS));
					current_triggers->next->trigger = trigger;
					break;
				}

				current_triggers = current_triggers->next;
			}
			while (current_triggers != NULL);
		}

		current = current->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : create_message
 * Desc : This function will add a message to the given timeline.
 *--------------------------------------------------------------------------------*/
static MESSAGE*	create_message(NODE* sender, TIMELINE* destination, BLOCK_NAME* name)
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

	return new_message;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_api_function
 * Desc : This function will find a named api function in the list of functions.
 *--------------------------------------------------------------------------------*/
static API_FUNCTION*	find_api_function(unsigned char* name, unsigned int name_length, GROUP* group)
{
	API_FUNCTION* 	result = NULL;
	API_FUNCTION*	current_func;

	if (group->api != NULL)
	{
		/* add it to the end of the list */
		current_func = group->api->function_list;

		while (current_func != NULL)
		{
			if (current_func->name.name_length == name_length && memcmp(name,current_func->name.name,current_func->name.name_length) == 0)
			{
				result = current_func;
				break;
			}

			current_func = current_func->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_type_record
 * Desc : This function will add a type record to the api type.
 *--------------------------------------------------------------------------------*/
static void	add_api_type_record(API_TYPE* api_type, unsigned int record_type,NAME* name, NAME* type,  NAME* brief)
{
	API_TYPE_RECORD*	new_record = malloc(sizeof(API_TYPE_RECORD));

	/* initialise the new record */
	new_record->record_type = record_type;
	copy_name(type,&new_record->type_item);
	copy_name(name,&new_record->name_value);
	copy_name(brief,&new_record->brief);

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
 * Name : add_api_constant_record
 * Desc : This function will add a type record to the api type.
 *--------------------------------------------------------------------------------*/
static void	add_api_constant_record(API_CONSTANTS* api_constants, unsigned int record_type,NAME* type, NAME* name,  NAME* brief)
{
	API_CONSTANT*	new_constant = malloc(sizeof(API_CONSTANT));

	/* initialise the new constant */
	copy_name(name,&new_constant->name);
	copy_name(type,&new_constant->type);
	copy_name(brief,&new_constant->brief);

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
	new_constant->constant = 1;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_constant
 * Desc : This function will add a constant to the api constants group list.
 *--------------------------------------------------------------------------------*/
static void	add_api_constant(API_CONSTANTS* api_constant, NAME* name, NAME* value)
{
	API_CONSTANT*	new_record = malloc(sizeof(API_CONSTANT));

	/* initialise the new record */
	copy_name(name,&new_record->name);
	copy_name(value,&new_record->value);

	/* add it to the record list */
	if (api_constant->constant_list == NULL)
	{
		api_constant->constant_list = new_record;
	}
	else
	{
		api_constant->last_constant->next = new_record;
	}

	api_constant->last_constant = new_record;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_function
 * Desc : This function will add a named function in the list of functions.
 *--------------------------------------------------------------------------------*/
static API_FUNCTION*	add_api_function(unsigned char* name, unsigned int name_length, GROUP* group)
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
	result->name.name_length = name_length;
	
	result->name.name = malloc(name_length);
	memcpy(result->name.name,name,name_length);
			
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_function
 * Desc : This function will find a named function in the list of functions.
 *--------------------------------------------------------------------------------*/
static FUNCTION*	find_function(unsigned char* name, unsigned int name_length)
{
	FUNCTION* result = &g_function_list;

	while (result != NULL)
	{
		if (name_length == result->name_length && memcmp(name,result->name,name_length) == 0)
		{
			break;
		}

		result = result->next;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_add_application
 * Desc : This function will find or add an application from the list of
 *        applications.
 *--------------------------------------------------------------------------------*/
static APPLICATION*	find_add_application(unsigned char* name, unsigned int name_length)
{
	APPLICATION* 	result = NULL;
	APPLICATION*	current_appl;

	/* add it to the end of the list */
	current_appl = &g_application_list;

	while (current_appl != NULL)
	{
		if (current_appl->next == NULL)
		{
			/* we have reached the end of the list -- add new application */
			result = calloc(1,sizeof(APPLICATION));

			result->name.name_length = name_length;
			result->name.name = malloc(name_length);
			memcpy(result->name.name,name,name_length);

			current_appl->next = result;
			break;
		}
		else if (current_appl->name.name_length == name_length && memcmp(current_appl->name.name,name,name_length) == 0)
		{
			/* found the item */
			result = current_appl;
			break;
		}

		current_appl = current_appl->next;
	}


	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : handle_trigger
 * Desc : This function will handle the adding of the trigger to the node.
 *--------------------------------------------------------------------------------*/
static TRIGGER*	handle_trigger(BLOCK_NAME* trigger, GROUP* group)
{
	TRIGGER* result = NULL;

	if (trigger->owner.group == NULL)
	{
		trigger->owner.group = group;
	}

	if ((result = find_trigger(trigger->owner.group,trigger->name,trigger->name_length)) == NULL)
	{
		/* add ghost trigger */
		result = add_trigger(trigger->owner.group,trigger->name,trigger->name_length,FLAG_GHOST);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_function
 * Desc : This function will add a named function in the list of functions.
 *--------------------------------------------------------------------------------*/
static FUNCTION*	add_function(unsigned char* name, unsigned int name_length, unsigned int flags)
{
	FUNCTION* result = calloc(1,sizeof(FUNCTION));
	FUNCTION* current_func = &g_function_list;

	while (current_func != NULL)
	{
		if (current_func->next == NULL)
		{
			current_func->next = result;
			break;
		}

		current_func = current_func->next;
	}

	/* set the values of the new function */
	result->flags = flags;
	result->name_length = name_length;
	memcpy(result->name,name,name_length);

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_sample
 * Desc : This function will add a named sample in the list of samples.
 *--------------------------------------------------------------------------------*/
static void	add_sample(unsigned char* data, unsigned int data_length)
{
	SAMPLE* result = calloc(1,sizeof(SAMPLE));
	unsigned int	name_length;
	unsigned int	sample_length;

	name_length = ((((unsigned int)data[0]) << 8) | data[1]);
	sample_length = ((((unsigned int)data[2]) << 8) | data[3]);

	/* add it to the front, these are unordered */
	result->next = g_sample_list.next;
	g_sample_list.next = result;

	/* set the values of the new sample */
	result->name.name_length = name_length;
	result->name.name = malloc(name_length);
	memcpy(result->name.name,&data[4],name_length);

	result->sample.name_length = sample_length;
	result->sample.name = malloc(sample_length);
	memcpy(result->sample.name,&data[4+name_length],sample_length);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_block_node
 * Desc : This function will add a sequence diagram block to the diagram tree.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_block_node(BLOCK_NODE* block, GROUP** local_group_list)
{
	NODE*			after;
	NODE*			sequence;
	NODE*			new_node;
	GROUP*			owing_group = NULL;
	TRIGGER*		trigger = NULL;
	MESSAGE*		message = NULL;
	MESSAGE*		new_message = NULL;
	MESSAGE*		wait_message = NULL;
	FUNCTION*		function = NULL;
	unsigned int	result = EC_OK;
	
	if (block->timeline == NULL && block->to_timeline == NULL)
	{
		result = EC_SEQUENCE_NODE_MISSING_TIMELINE;
	}
	else
	{
		/* create the new node */
		new_node = calloc(1,sizeof(NODE));
		new_node->flags = block->flags;
		new_node->group = block->group;
		new_node->timeline = block->timeline;

		/* decode the trigger */
		if (block->trigger != NULL)
		{
			if ((new_node->trigger = find_trigger(block->trigger->owner.group,block->trigger->name,block->trigger->name_length)) == NULL)
			{
				/* add ghost trigger */
				new_node->trigger = add_trigger(block->trigger->owner.group,block->trigger->name,block->trigger->name_length,FLAG_GHOST);
			}
		}

		if (block->message.name_length > 0)
		{
			/* does the block call a function or send a message */
			if ((block->flags & FLAG_MESSAGE_CALL) == FLAG_MESSAGE_CALL)
			{
				if((new_node->function = find_function(block->message.name,block->message.name_length)) == NULL)
				{
					/* Ok, we have a function that we don't know about yet */
					new_node->function = add_function(block->message.name,block->message.name_length,FLAG_GHOST);
				}
			}
			else
			{
				new_node->sent_message = create_message(new_node,block->to_timeline,&block->message);
				new_node->sent_message->flags = block->flags;
			}
		}

		/* wait- this needs to fixed-up after the functions are */
		if (block->sequence.name_length > 0)
		{
			memcpy(&new_node->wait_message,&block->sequence,sizeof(BLOCK_NAME));
		}

		/* handle the sequence */
		if (block->after.name_length > 0)
		{
			memcpy(&new_node->after,&block->after,sizeof(BLOCK_NAME));
		}

		/* handle the trigger wait */
		if (block->trigger != NULL)
		{
			new_node->trigger = handle_trigger(block->trigger,block->group);
		}

		/* add the generated triggers to the group */
		if (block->triggers_list != NULL)
		{
			add_triggers(&new_node->triggers,block->triggers_list,block->group);
		}

		/* insert at the end */
		if (block->timeline->last_node == NULL)
		{
			block->timeline->node = new_node;
			block->timeline->last_node = new_node;
		}
		else
		{
			block->timeline->last_node->next = new_node;
			block->timeline->last_node = new_node;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_block_type
 * Desc : This function will add a type block to the diagram tree.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_block_type(BLOCK_NODE* block, GROUP** local_group_list)
{
	GROUP*			owing_group = NULL;
	API_TYPE*		current_type;
	unsigned int	result = EC_OK;
	
	if (block->description.name_length > 0)
	{
		block->api_type->description.fixed = block->description.fixed;
		block->api_type->description.name = block->description.name;
		block->api_type->description.name_length = block->description.name_length;
	}

	if (block->group == NULL)
	{
		/* if no group defined, default to the default */
		block->group = local_group_list[0];
	}

	owing_group = block->group;

	/* create the API group if required */
	if (owing_group->api == NULL)
	{
		owing_group->api = calloc(1,sizeof(API));
		owing_group->api->group = block->group;
	}

	/* move the type to the correct API */
	if (owing_group->api->type_list == NULL)
	{
		owing_group->api->type_list = block->api_type;
	}
	else
	{
		current_type = owing_group->api->type_list;

		while (current_type != NULL)
		{
			if (current_type->next == NULL)
			{
				current_type->next = block->api_type;
				block->api_type->next = NULL;
				break;
			}

			current_type = current_type->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_block_constants
 * Desc : This function will add a constants block to the diagram tree.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_block_constants(BLOCK_NODE* block, GROUP** local_group_list)
{
	GROUP*			owing_group = NULL;
	API_CONSTANTS*	current_constants;
	unsigned int	result = EC_OK;
	
	if (block->description.name_length > 0)
	{
		block->api_type->description.fixed = block->description.fixed;
		block->api_type->description.name  = block->description.name;
		block->api_type->description.name_length = block->description.name_length;
	}

	if (block->group == NULL)
	{
		/* if no group defined, default to the default */
		block->group = local_group_list[0];
	}

	owing_group = block->group;

	/* create the API group if required */
	if (owing_group->api == NULL)
	{
		owing_group->api = calloc(1,sizeof(API));
		owing_group->api->group = block->group;
	}

	/* move the type to the correct API */
	if (owing_group->api->constants_list == NULL)
	{
		owing_group->api->constants_list = block->api_constants;
	}
	else
	{
		current_constants = owing_group->api->constants_list;

		while (current_constants != NULL)
		{
			if (current_constants->next == NULL)
			{
				current_constants->next = block->api_constants;
				block->api_constants->next = NULL;
				break;
			}

			current_constants = current_constants->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_block_function
 * Desc : This function will add a function block.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_block_function(BLOCK_NODE* block, FUNCTION** local_function_list)
{
	unsigned int		result = EC_OK;
	unsigned int		function_id = 0;
	FUNCTION_NODE*		current = NULL;
	FUNCTION_NODE*		function_node = NULL;

	if (block->message.name_length == 0 && block->sequence.name_length == 0)
	{
		result = EC_BLOCK_MUST_HAVE_DEFINER;
		raise_warning(block->line_number,result,NULL,NULL);
	}
	else
	{
		/* create the new node for the function */
		function_node = calloc(1,sizeof(FUNCTION_NODE));

		function_node->flags = block->flags;

		/* handle the send/call */
		if (block->message.name_length > 0)
		{
			/* add a send or a call */
			if ((block->flags & FLAG_MESSAGE_CALL) == FLAG_MESSAGE_CALL)
			{
				if((function_node->call = find_function(block->message.name,block->message.name_length)) == NULL)
				{
					/* Ok, we have a function that we don't know about yet */
					function_node->call = add_function(block->message.name,block->message.name_length,FLAG_GHOST);
				}
			}
			else
			{
				memcpy(&function_node->message,&block->message,sizeof(BLOCK_NAME));
			}
		}

		/* handle the after */
		if (block->after.name_length > 0)
		{
			memcpy(&function_node->after,&block->after,sizeof(BLOCK_NAME));
		}

		/* handle the condition */
		if (block->condition.name_length > 0)
		{
			memcpy(&function_node->condition,&block->condition,sizeof(BLOCK_NAME));
		}

		/* handle the waitfor/responds */
		if (block->sequence.name_length > 0)
		{
			memcpy(&function_node->sequence,&block->sequence,sizeof(BLOCK_NAME));
		}

		/* handle function @to timeline */
		if (block->function_to_timeline.name_length > 0)
		{
			memcpy(&function_node->to_timeline_name,&block->function_to_timeline,sizeof(BLOCK_NAME));
		}

		/* copy the triggers across */
		function_node->trigger = block->trigger;
		function_node->triggers_list = block->triggers_list;

		/* stop the memory from being released */
		block->trigger = NULL;
		block->triggers_list = NULL;

		/* add node to the end of the function */
		if ((current = block->function->node) == NULL)
		{
			block->function->node = function_node;
		}
		else
		{
			do
			{
				if (current->next == NULL)
				{
					current->next = function_node;
					break;
				}

				current = current->next;
			}
			while (current != NULL);
		}
	}
	
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_block_file
 * Desc : This will add the block for the file. It's major function is to set the
 *        defaults that are require while processing this file.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_block_file(BLOCK_NODE* block, GROUP** local_group_list)
{
	unsigned int result = EC_OK;

	/* TODO: handle the other fields except the ones required for processing */

	if (block->group != NULL)
	{
		local_group_list[0] = block->group;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_block_application
 * Desc : This will add the block for the application.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_block_application(BLOCK_NODE* block, GROUP** local_group_list, APPLICATION** local_application_list)
{
	unsigned int 	result = EC_OK;
	OPTION*			current_option = NULL;
	APPLICATION*	application	= NULL;
	
	if (block->application_id != 0)
	{
		application = local_application_list[block->application_id];
	}

	if (block->group != NULL)
	{
		local_group_list[0] = block->group;
	}

	if (block->option.name_length > 0)
	{
		if (application != NULL)
		{
			/* create the new option */
			current_option = calloc(1,sizeof(OPTION));
			current_option->name.name = block->option.name;
			current_option->name.name_length = block->option.name_length;
			current_option->flags = block->application_flag;

			if (application->option_list == NULL)
			{
				/* first element of the list */
				application->option_list = current_option;
				application->option_list->last = current_option;
			}
			else
			{
				/* add to the end */
				current_option->option_id = application->option_list->last->option_id + 1;
				application->option_list->last->next = current_option;
				application->option_list->last = current_option;
			}
		}
		else
		{
			result = EC_OPTION_MUST_HAVE_APPLICATION_SCOPE;
			raise_warning(block->line_number,result,NULL,NULL);
		}
	}

	if (block->value.name_length > 0)
	{
		if (current_option == NULL)
		{
			result = EC_VALUE_MUST_BE_ASSOCIATED_WITH_OPTION;
			raise_warning(block->line_number,result,g_source_filename,NULL);
		}
		else if (current_option->value.name_length > 0)
		{
			result = EC_OPTION_VALUE_REDEFINED;
			raise_warning(block->line_number,result,NULL,NULL);
		}
		else
		{
			current_option->value.name = block->value.name;
			current_option->value.fixed = block->value.fixed;
			current_option->value.name_length = block->value.name_length;
		}
	}

	/* description is multi-purpose so wont be attached to and application without
	 * an application atom.
	 */
	if (block->description.name_length > 0 && current_option != NULL)
	{
		if (current_option->description.name_length > 0)
		{
			result = EC_DOUBLE_DESCRIPTOR_FOUND;
			raise_warning(block->line_number,result,g_source_filename,NULL);
		}
		else
		{
			current_option->description.fixed = block->description.fixed;
			current_option->description.name  = block->description.name;
			current_option->description.name_length = block->description.name_length;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_block_api
 * Desc : This will add the block for the api. This can add fields to the parts
 *        of the API.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_block_api(BLOCK_NODE* block, GROUP** local_group_list)
{
	unsigned int		result = EC_OK;
	API_RETURNS*		current_return;
	API_PARAMETER*		current_parameter;
	NAME_PAIRS_LIST*	current_item;

	if (block->group == NULL)
	{
		/* if no group defined, default to the default */
		block->group = local_group_list[0];
	}

	if (block->description.name_length > 0)
	{
		block->api_function->description.name = block->description.name;
		block->api_function->description.fixed = block->description.fixed;
		block->api_function->description.name_length = block->description.name_length;
	}

	if (block->action.name_length > 0)
	{
		block->api_function->action.name = block->action.name;
		block->api_function->action.fixed = block->action.fixed;
		block->api_function->action.name_length = block->action.name_length;
	}

	if (block->num_parameters > 0)
	{
		current_item = &block->parameters;

		while (current_item != NULL)
		{
			current_parameter = block->api_function->parameter_list;

			/* find the parameter in the api's function list */
			while(current_parameter != NULL)
			{
				if (current_item->name.name_length == current_parameter->name.name_length && 
					memcmp(current_item->name.name,current_parameter->name.name,current_item->name.name_length) == 0)
				{
					/* found the item - set the brief */
					if (current_parameter->brief.name_length > 0)
					{
						/* already has a parameter */
						result = EC_DUPLICATE_PARAMETER;
						raise_warning(block->line_number,result,current_parameter->brief.name,NULL);
					}
					else
					{
						current_parameter->brief.name = current_item->string.name;
						current_parameter->brief.fixed = current_item->string.fixed;
						current_parameter->brief.name_length = current_item->string.name_length;
					}
					break;
				}
				
				current_parameter = current_parameter->next;
			}

			current_item = current_item->next;
		}
	}

	/* add the return values to the function */
	if (block->num_returns > 0)
	{
		current_item = &block->returns;

		while (current_item != NULL)
		{
			if (block->api_function->returns_list == NULL)
			{
				block->api_function->returns_list = malloc(sizeof(API_RETURNS));
						
				block->api_function->returns_list->value.name = current_item->name.name;
				block->api_function->returns_list->value.name_length = current_item->name.name_length;
				
				block->api_function->returns_list->brief.name = current_item->string.name;
				block->api_function->returns_list->brief.fixed = current_item->string.fixed;
				block->api_function->returns_list->brief.name_length = current_item->string.name_length;
				
				block->api_function->returns_list->next = NULL;
			}
			else
			{
				current_return = block->api_function->returns_list;

				while(current_return != NULL)
				{
					if (current_return->next == NULL)
					{
						current_return->next = malloc(sizeof(API_RETURNS));

						current_return->next->value.name = current_item->name.name;
						current_return->next->value.name_length = current_item->name.name_length;

						current_return->next->brief.name = current_item->string.name;
						current_return->next->brief.fixed = current_item->string.fixed;
						current_return->next->brief.name_length = current_item->string.name_length;

						current_return->next->next = NULL;
						break;
					}

					current_return = current_return->next;
				}
			}


			current_item = current_item->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_block_state
 * Desc : This function will add a state block to the diagram tree.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_block_state(BLOCK_NODE* block, GROUP** local_group_list)
{
	STATE*				to_state = NULL;
	GROUP*				owing_group = NULL;
	TRIGGER*			trigger = NULL;
	STATE_TRANSITION*	current_trans  = NULL;
	STATE_TRANSITION*	new_transition = NULL;
	unsigned int		result = EC_OK;

	if (block->transition.name_length == 0 && (block->state->name_length == 6 && memcmp(block->state->name,"finish",6) != 0))
	{
		result = EC_STATE_MUST_HAVE_TRANSITION;
	}
	else
	{
		/* ok, looks valid lets add it together */
		if ((to_state = find_state(block->state->group,block->transition.name,block->transition.name_length)) == NULL)
		{
			/* cannot find the state, so add a ghost state */
			to_state = add_state(block->state->group,block->transition.name,block->transition.name_length,block->line_number,FLAG_GHOST);
		}

		/* decode the trigger */
		if (block->trigger != NULL)
		{
			trigger = handle_trigger(block->trigger,block->group);
		}

		/* create the transition */
		if (block->state->transition_list == NULL)
		{
			block->state->transition_list = calloc(1,sizeof(STATE_TRANSITION));
			new_transition = block->state->transition_list;
		}
		else
		{
			current_trans = block->state->transition_list;

			/* add to the end of the transition list, and check to see if the trigger has
			 * already been used in this state.
			 */
			while(current_trans != NULL)
			{
				if (current_trans->next == NULL)
				{
					current_trans->next = calloc(1,sizeof(STATE_TRANSITION));
					new_transition = current_trans->next;
					break;
				}
				else if (trigger != NULL && current_trans->trigger == trigger)
				{
					result = EC_DUPLICATE_TRIGGER;
					break;
				}
				current_trans = current_trans->next;
			}
		}

		if (new_transition != NULL)
		{
			new_transition->next_state = to_state;
			new_transition->trigger = trigger;

			if (block->condition.name_length > 0)
			{
				new_transition->condition = malloc(block->condition.name_length);
				new_transition->condition_length = block->condition.name_length;
				memcpy(new_transition->condition,block->condition.name,block->condition.name_length);
			}
		}

		/* add the generated triggers to the group */
		if (block->triggers_list != NULL)
		{
			add_triggers(&new_transition->triggers,block->triggers_list,block->group);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_block
 * Desc : This function will add a block to the diagram tree.
 *--------------------------------------------------------------------------------*/
static unsigned int add_block(BLOCK_NODE* block, GROUP** local_group_list, FUNCTION** local_function_list, APPLICATION** local_application_list )
{
	STATE*				to_state = NULL;
	GROUP*				owing_group = NULL;
	TRIGGER*			trigger = NULL;
	STATE_TRANSITION*	current_trans  = NULL;
	STATE_TRANSITION*	new_transition = NULL;
	unsigned int		temp = EC_OK;
	unsigned int		result = EC_OK;

	/* validate the node first */
	if (block->type == ATOM_BLOCK_FILE)
	{
		result = add_block_file(block,local_group_list);
	}
	else if (block->state != NULL)
	{
		result = add_block_state(block,local_group_list);

	}
	else if (block->timeline != NULL)
	{
		result = add_block_node(block,local_group_list);
	}
	else if (block->api_constants != NULL)
	{
		result = add_block_constants(block,local_group_list);
	}
	else if (block->api_type != NULL)
	{
		result = add_block_type(block,local_group_list);
	}
	else if (block->application_id != 0)
	{
		result = add_block_application(block,local_group_list,local_application_list);
	}
	else if (block->api_function != NULL || (block->flags & FLAG_IN_FUNCTION) == FLAG_IN_FUNCTION)
	{
		/* block can both be an API and a function */
		if (block->api_function != NULL)
		{
			result = add_block_api(block,local_group_list);
		}

		if ((block->flags & FLAG_IN_FUNCTION) == FLAG_IN_FUNCTION)
		{
			if ((temp = add_block_function(block,local_function_list)) != EC_OK)
			{
				result = temp;
			}
		}
	}
	else
	{
		result = EC_BLOCK_MUST_HAVE_DEFINER;
	}

	if (result != EC_OK)
	{
		raise_warning(block->line_number,result,NULL,NULL);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_numeric_to_block
 * Desc : This function will add the numeric atom to the block.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_numeric_to_block (	GROUP**			local_group_list,
											FUNCTION**		local_function_list,
											API_FUNCTION**	local_api_function_list,
											BLOCK_NODE*		node,
											unsigned char*	record,
											unsigned char*	payload,
											unsigned int	payload_length )
{
	unsigned int	result = EC_OK;
	unsigned int	line_number = line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
	unsigned int	numeric_value;
	
	numeric_value =  ((((unsigned int) payload[0])<< 24) |
					  (((unsigned int) payload[1])<< 16) |
					  (((unsigned int) payload[2])<< 16) |
					   ((unsigned int) payload[3]));

	switch(record[RECORD_ATOM])
	{
		case ATOM_API:
				if (numeric_value > MAX_GROUPS_PER_FILE)
				{
					result = EC_UNDEFINED_API;
					raise_warning(line_number,result,NULL,NULL);
				}
				else
				{
					node->api_function = local_api_function_list[numeric_value];
				}
			break;

		case ATOM_GROUP:
				if (numeric_value > MAX_GROUPS_PER_FILE)
				{
					result = EC_UNDEFINED_GROUP;
					raise_warning(line_number,result,NULL,NULL);
				}
				else
				{
					node->group = local_group_list[numeric_value];
				}
			break;

		default:
			result = EC_UNKNOWN_ATOM;
			raise_warning(line_number,result,NULL,NULL);
			break;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_to_pair_list
 * Desc : This function will add a pair to the pair list.
 *--------------------------------------------------------------------------------*/
static void	add_to_pair_list(NAME_PAIRS_LIST* list, NAME* name, NAME* string)
{
	NAME_PAIRS_LIST*	current_item;

	if (list->name.name_length == 0)
	{
		copy_name(name,&list->name);
		copy_name(string,&list->string);
	}
	else
	{
		current_item = list;

		while (current_item != NULL)
		{
			if (current_item->next == NULL)
			{
				current_item->next = malloc(sizeof(NAME_PAIRS_LIST));
				copy_name(name,&current_item->next->name);
				copy_name(string,&current_item->next->string);
				break;
			}

			current_item = current_item->next;
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_pair_to_block
 * Desc : This function will add the numeric atom to the block.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_pair_to_block (	GROUP**			local_group_list,
										FUNCTION**		local_function_list,
										APPLICATION**	local_application_list,
										API_FUNCTION**	local_api_function_list,
										BLOCK_NODE*		node,
										unsigned char*	record,
										unsigned char*	payload,
										unsigned int	payload_length )
{
	unsigned int	result = EC_OK;
	unsigned int	line_number = line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
	NAME			name;
	NAME			string;

	name.name			= &payload[2];
	name.name_length	= ((((unsigned int)payload[0]) << 8) | payload[1]);

	string.name			= &payload[2 + 2 + name.name_length];
	string.name_length	= ((((unsigned int)payload[2 + name.name_length]) << 8) | payload[1 + 2 + name.name_length]);
	
	switch(record[RECORD_ATOM])
	{
		case ATOM_PARAMETER:
			node->num_parameters++;
			add_to_pair_list(&node->parameters,&name,&string);
			break;

		case ATOM_RETURNS:
			node->num_returns++;
			add_to_pair_list(&node->returns,&name,&string);
			break;

		case ATOM_RECORD:
			if (node->api_constants != NULL)
			{
				add_api_constant(node->api_constants,&name,&string);
			}
			break;

		case ATOM_TYPE:
			if (node->api_type != NULL)
			{
				add_api_type_record(node->api_type,ATOM_TYPE_FIELD,&string,&name,NULL);
			}
			break;

		case ATOM_SYNOPSIS:
			if (node->application_id > 0)
			{
				APPLICATION*	application = local_application_list[node->application_id];

				if (application != NULL)
				{
					if (application->synopsis_list.name.name_length == 0)
					{
						/* ad to start */
						copy_name(&name,&application->synopsis_list.name);
						copy_name(&string,&application->synopsis_list.items);
						application->synopsis_list.last = &application->synopsis_list;
					}
					else
					{
						/* add to end */
						SYNOPSIS_LIST*	new_synopsis = calloc(1,sizeof(SYNOPSIS_LIST));

						copy_name(&name,&new_synopsis->name);
						copy_name(&string,&new_synopsis->items);

						application->synopsis_list.last->next = new_synopsis;
						application->synopsis_list.last = new_synopsis;
					}
				}
			}
			break;

		default:
			result = EC_UNKNOWN_ATOM;
			raise_warning(line_number,result,NULL,NULL);
			break;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_atom_to_constant
 * Desc : This function will add the atom to the block. 
  *--------------------------------------------------------------------------------*/
static unsigned int	add_atom_to_constant(	GROUP** 		local_group_list,
										FUNCTION**		local_function_list,
										API_FUNCTION**	local_api_function_list,
										BLOCK_NODE*		node,
										unsigned char*	record,
										unsigned char*	payload,
										unsigned int	payload_length )
{
	NAME			temp;
	unsigned int	result = EC_OK;
	unsigned char	fixed = payload[0];

	temp.name = &payload[1];
	temp.fixed = fixed;
	temp.name_length = payload_length - 1;

	switch(record[RECORD_ATOM])
	{
		case ATOM_DESC:
		case ATOM_DESCRIPTION:
			node->api_constants->description.fixed |= fixed;
			node->api_constants->description.name = realloc(node->api_constants->description.name,
															node->api_constants->description.name_length+temp.name_length+1);
			node->api_constants->description.name[node->api_constants->description.name_length] = 0x20;
			memcpy(&node->api_constants->description.name[node->api_constants->description.name_length+1],temp.name,temp.name_length);
			node->api_constants->description.name_length += temp.name_length + 1;
			break;

		case ATOM_BRIEF:
			if (node->api_constants->last_constant != NULL)
			{
				if (node->api_constants->last_constant->brief.name_length == 0)
				{
					copy_name(&temp,&node->api_constants->last_constant->brief);
				}
				else
				{
					result = EC_DOUBLE_DESCRIPTOR_FOUND;
					raise_warning (0,result,node->api_constants->last_constant->brief.name,NULL);
				}
			}
			else
			{
				result = EC_BRIEF_WITHOUT_A_TYPE;
				raise_warning (0,result,node->api_constants->last_constant->brief.name,NULL);
			}
			break;

		case ATOM_CONSTANTS:
			if (node->api_constants != NULL && node->api_constants->last_constant != NULL)
			{
				/* add the new data to the end of the old */
				unsigned char* new_val;
				void* old = node->api_constants->last_constant->value.name;
				unsigned int old_size = node->api_constants->last_constant->value.name_length;
					
				node->api_constants->last_constant->value.fixed |= fixed;

				if ((new_val = realloc(old,old_size+temp.name_length)) != NULL)
				{
					memcpy(&new_val[old_size],temp.name,temp.name_length);

					node->api_constants->last_constant->value.name = new_val;
					node->api_constants->last_constant->value.name_length += temp.name_length;
				}
			}
			break;

		default:
			result = EC_UNEXPECTED_ATOM;
			raise_warning (0,result,node->api_constants->last_constant->brief.name,NULL);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: new_section
 * @desc: This function will add a section to application.
 *--------------------------------------------------------------------------------*/
static unsigned int	new_section(APPLICATION* application, unsigned char* name, unsigned int name_length, unsigned int fixed)
{
	unsigned int	result = 0;
	SECTION*		new_section = calloc(1,sizeof(SECTION));
			
	if (new_section != NULL && application != NULL)
	{
		/* allocate the section_list */
		new_section->name.name			= malloc(name_length);
		new_section->name.fixed			= fixed;
		new_section->name.name_length	= name_length;
		memcpy(new_section->name.name,name,name_length);

		if (application->section_list == NULL)
		{
			/* Ok, the first part of the section_list will have the section_list name */
			application->section_list		= new_section;
			application->section_list->last	= new_section;
		}
		else
		{
			application->section_list->last->next	= new_section;
			application->section_list->last			= new_section;
		}

		result = 1;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: extend_section
 * @desc: This function will extend the section data.
 *
 * TODO: make this concatenate strings - this also is probably a leak source
 *--------------------------------------------------------------------------------*/
static unsigned int	extend_section(APPLICATION* application, unsigned char* name, unsigned int name_length, unsigned int fixed)
{
	unsigned int	result = 0;
	unsigned char*	temp;

	if (application != NULL && name_length > 0)
	{
		temp = realloc(	application->section_list->last->section_data.name,
						application->section_list->last->section_data.name_length + name_length + 1);

		if (temp != NULL)
		{
			if (application->section_list->last->section_data.name_length > 0)
			{
				application->section_list->last->section_data.name[application->section_list->last->section_data.name_length++] = 0x20;
			}

			memcpy(&temp[application->section_list->last->section_data.name_length],name,name_length);
			
			application->section_list->last->section_data.name = temp;
			application->section_list->last->section_data.name_length += name_length;
		
			application->section_list->last->section_data.fixed |= fixed;
		}

		result = 1;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_atom_to_type
 * Desc : This function will add the atom to the block. 
  *--------------------------------------------------------------------------------*/
static unsigned int	add_atom_to_type(	GROUP** 		local_group_list,
									FUNCTION**		local_function_list,
									API_FUNCTION**	local_api_function_list,
									BLOCK_NODE*		node,
									unsigned char*	record,
									unsigned char*	payload,
									unsigned int	payload_length )
{
	NAME			temp;
	unsigned int	result = EC_OK;
	unsigned char	fixed = payload[0];

	temp.name = &payload[1];
	temp.fixed = fixed;
	temp.name_length = payload_length - 1;

	switch(record[RECORD_ATOM])
	{
		case ATOM_NAME:
			if (node->api_type->depth > 0)
			{
				add_api_type_record(node->api_type,ATOM_NAME,&temp,NULL,NULL);
				node->api_type->depth--;
			}
			else
			{
				copy_name(&temp,&node->api_type->name);
				node->api_type->depth--;
			}
			break;

		case ATOM_RECORD:
			node->api_type->depth++;
			add_api_type_record(node->api_type,ATOM_RECORD,NULL,&temp,NULL);
			break;

		case ATOM_TYPE:
			add_api_type_record(node->api_type,record[RECORD_ATOM],NULL,&temp,NULL);
			break;

		case ATOM_BRIEF:
			if (node->api_type->last_record != NULL)
			{
				if (node->api_type->last_record->brief.name_length == 0)
				{
					copy_name(&temp,&node->api_type->last_record->brief);
				}
				else
				{
					result = EC_DOUBLE_DESCRIPTOR_FOUND;
					raise_warning (0,result,node->api_constants->last_constant->brief.name,NULL);
				}
			}
			else
			{
				result = EC_BRIEF_WITHOUT_A_TYPE;
				raise_warning (0,result,node->api_constants->last_constant->brief.name,NULL);
			}
			break;

		default:
			result = EC_UNEXPECTED_ATOM;
			raise_warning (0,result,node->api_constants->last_constant->brief.name,NULL);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_atom_to_block
 * Desc : This function will add the atom to the block. 
 *
 * TODO: fix this code it knows the record format, and it should not. Means that
 *       too many functions have to be changed when the format changes.
 *--------------------------------------------------------------------------------*/
static unsigned int	add_atom_to_block ( GROUP** 		local_group_list,
									FUNCTION**		local_function_list,
									APPLICATION**	local_application_list,
									API_FUNCTION**	local_api_function_list,
									BLOCK_NODE*		node,
									unsigned char*	record,
									unsigned char*	payload,
									unsigned int	payload_length )
{
	unsigned int	in_function = 0;
	unsigned int	result = EC_OK;
	unsigned int	line_number = 0;
	unsigned int	group_id = ((record[RECORD_GROUP]<< 8) | record[RECORD_GROUP+1]);
	unsigned int	application_id = group_id;
	unsigned int	string_length = payload_length - 1;
	unsigned char	fixed = payload[0];
	unsigned char*	string = &payload[1];
	NAME			temp;
	TRIGGER*		trigger;
	FUNCTION*		function;

	temp.name	= string;
	temp.fixed	= fixed;
	temp.name_length = string_length;

	if (group_id != DEFAULT_GROUP && (group_id & RECORD_FUNC_API_MASK) == RECORD_FUNCTION_FLAG)
	{
		/* the atom is within a function */
		node->flags |= FLAG_IN_FUNCTION;
		group_id &= ~RECORD_FUNC_API_MASK;
	
		node->function = local_function_list[group_id+1];

		in_function = 1;
	}

	if (group_id != DEFAULT_GROUP && ((group_id & RECORD_FUNC_API_MASK) == RECORD_API_FLAG))
	{
		node->api_function = local_api_function_list[(~RECORD_FUNC_API_MASK) & group_id];
	}
	
	switch(record[RECORD_ATOM])
	{
		case ATOM_FILE:
			node->type = ATOM_BLOCK_FILE;
			break;
	
		case ATOM_AUTHOR:
			if (node->author.name_length > 0)
			{
				result = EC_MULTIPLE_AUTHORS_IN_ONE_BLOCK;
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				raise_warning(line_number,result,NULL,NULL);
			}
			else
			{
				node->author.name_length = string_length;
				memcpy(node->author.name,string,string_length);
			}
			break;

		case ATOM_ACTION:
			node->action.fixed |= fixed;
			node->action.name = realloc(node->action.name,node->action.name_length+string_length+1);
			node->action.name[node->action.name_length] = 0x20;
			memcpy(&node->action.name[node->action.name_length+1],string,string_length);
			node->action.name_length += string_length + 1;
			break;

		case ATOM_DESC:
		case ATOM_DESCRIPTION:
			node->description.fixed |= fixed;
			node->description.name = realloc(node->description.name,node->description.name_length+string_length+1);

			if (node->description.name_length > 0)
			{
				node->description.name[node->description.name_length++] = 0x20;
			}
			memcpy(&node->description.name[node->description.name_length],string,string_length);
			node->description.name_length += string_length;

			break;

		case ATOM_STATE:
			if (group_id == DEFAULT_GROUP)
			{
				group_id = 0;
			}
				
			node->group = local_group_list[group_id];

			if (node->state != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				raise_warning(line_number,EC_MULTIPLE_STATES_IN_ONE_BLOCK,NULL,NULL);
				result = EC_MULTIPLE_STATES_IN_ONE_BLOCK;
			}
			else if (node->timeline != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				raise_warning(line_number,EC_MIXED_DIAGRAM_TYPES_IN_BLOCK,NULL,NULL);
				result = EC_MIXED_DIAGRAM_TYPES_IN_BLOCK;
			}
			else if ((node->state = find_state(local_group_list[group_id],string,string_length)) == NULL)
			{
				node->state = add_state(local_group_list[group_id],string,string_length,node->line_number,0);
			}
			else if ((node->state->flags & FLAG_GHOST) == FLAG_GHOST)
			{
				node->state->flags &= ~FLAG_GHOST;
				node->state->line_number = node->line_number;
			}
			break;

		case ATOM_NEXT:
			if (node->timeline != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				raise_warning(line_number,EC_WRONG_ATOM_IN_BLOCK,NULL,NULL);
				result = EC_WRONG_ATOM_IN_BLOCK;
			}
			else
			{
				if (node->transition.name_length > 0)
				{
					line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
					result = EC_DUPLICATE_TRANSITION;
					raise_warning(line_number,result,NULL,NULL);
				}
				else
				{
					memcpy(node->transition.name,string,string_length);
					node->transition.owner.group = local_group_list[group_id];
					node->transition.name_length = string_length;
				}
			}
			break;

		case ATOM_CONDITION:
			/* condition for a state condition */
			if (node->condition.name_length > 0)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				result = EC_DUPLICATE_CONDITION_IN_TRANSITION;
				raise_warning(line_number,result,NULL,NULL);
			}
			else if (node->trigger != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				result = EC_CANNOT_HAVE_TRIGGER_AND_CONDITION_IN_TRANSITION;
				raise_warning(line_number,result,NULL,NULL);
			}
			else
			{
				memcpy(node->condition.name,string,string_length);
				node->condition.owner.group = local_group_list[group_id];
				node->condition.name_length = string_length;
			}
			break;

		case ATOM_TRIGGER:
			if (node->condition.name_length > 0)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				result = EC_CANNOT_HAVE_TRIGGER_AND_CONDITION_IN_TRANSITION;
				raise_warning(line_number,result,NULL,NULL);
			}
			else if (node->trigger != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				result = EC_MULTIPLE_TRIGGER;
				raise_warning(line_number,result,NULL,NULL);
			}
			else 
			{
				/* add the trigger reference */
				node->trigger = calloc(1,sizeof(BLOCK_NAME));
				node->trigger->owner.group = local_group_list[group_id];
				node->trigger->name_length = string_length;
				memcpy(node->trigger->name,string,string_length);
			}
			break;

		case ATOM_TRIGGERS:
			{
				BLOCK_NAME* temp = node->triggers_list;
				node->triggers_list = calloc(1,sizeof(BLOCK_NAME));
				node->triggers_list->owner.group = local_group_list[group_id];
				node->triggers_list->name_length = string_length;
				memcpy(node->triggers_list->name,string,string_length);
				node->triggers_list->next = temp;
			}
			break;

		case ATOM_TIMELINE:
			if (group_id == DEFAULT_GROUP)
			{
				group_id = 0;
			}

			if (in_function)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				result = EC_CANNOT_CALL_HAVE_THESE_ATOMS_IN_A_FUNCTION;
				raise_warning(line_number,result,NULL,NULL);
			}
			if (node->timeline != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				result = EC_MULTIPLE_TIMELINES_IN_ONE_BLOCK;
				raise_warning(line_number,result,NULL,NULL);
			}
			else if (node->state != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				result = EC_MIXED_DIAGRAM_TYPES_IN_BLOCK;
				raise_warning(line_number,result,NULL,NULL);
			}
			else if ((node->timeline = find_timeline(local_group_list[group_id],string,string_length)) == NULL)
			{
				node->timeline = add_timeline(local_group_list[group_id],string,string_length,node->line_number,0);
			}

			node->group = node->timeline->group;
			break;

		case ATOM_CALL:
		case ATOM_SEND:
		case ATOM_RESPONDS:
			if (node->state != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				raise_warning(line_number,EC_WRONG_ATOM_IN_BLOCK,NULL,NULL);
				result = EC_WRONG_ATOM_IN_BLOCK;
			}
			else
			{
				if (node->message.name_length > 0)
				{
					line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
					result = EC_DUPLICATE_MESSAGE;
					raise_warning(line_number,result,NULL,NULL);
				}
				else
				{
					if (record[RECORD_ATOM] == ATOM_SEND)
						node->flags |= FLAG_MESSAGE_SEND;
					else if (record[RECORD_ATOM] == ATOM_RESPONDS)
						node->flags |= FLAG_SEQUENCE_RESPONDS;
					else
						node->flags |= FLAG_MESSAGE_CALL;

					node->message.owner.group = local_group_list[group_id];
					memcpy(node->message.name,string,string_length);
					node->message.name_length = string_length;
				}
			}
			break;

		case ATOM_AFTER:
			if (node->state != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				raise_warning(line_number,EC_WRONG_ATOM_IN_BLOCK,NULL,NULL);
				result = EC_WRONG_ATOM_IN_BLOCK;
			}
			else
			{
				if (node->after.name_length > 0)
				{
					line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
					result = EC_DUPLICATE_AFTER;
					raise_warning(line_number,result,NULL,NULL);
				}
				else
				{
					node->after.owner.group = local_group_list[group_id];
					memcpy(node->after.name,string,string_length);
					node->after.name_length = string_length;
				}
			}
			break;

		case ATOM_WAITFOR:
			if (node->state != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				raise_warning(line_number,EC_WRONG_ATOM_IN_BLOCK,NULL,NULL);
				result = EC_WRONG_ATOM_IN_BLOCK;
			}
			else
			{
				if (node->sequence.name_length > 0)
				{
					line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
					result = EC_DUPLICATE_SEQUENCE;
					raise_warning(line_number,result,NULL,NULL);
				}
				else
				{
					node->flags |= FLAG_SEQUENCE_WAITFOR;

					node->sequence.owner.group = local_group_list[group_id];
					memcpy(node->sequence.name,string,string_length);
					node->sequence.name_length = string_length;
				}
			}
			break;

		case ATOM_TO:
			if (group_id == DEFAULT_GROUP)
			{
				group_id = 0;
			}

			if (node->to_timeline != NULL || node->function_to_timeline.name_length > 0)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				result = EC_MULTIPLE_TO_TIMELINES_IN_ONE_BLOCK;
				raise_warning(line_number,result,NULL,NULL);
			}
			else if (node->state != NULL)
			{
				line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
				result = EC_MIXED_DIAGRAM_TYPES_IN_BLOCK;
				raise_warning(line_number,result,NULL,NULL);
			}
			else if (in_function)
			{
				/* have to differ the lookup till we know what the group the function will be unpacked in
				 * will be, so just save the name.
				 */
				memcpy(node->function_to_timeline.name,string,string_length);
				node->function_to_timeline.name_length = string_length;
			}
			else if ((node->to_timeline = find_timeline(local_group_list[group_id],string,string_length)) == NULL)
			{
				node->to_timeline = add_timeline(local_group_list[group_id],string,string_length,node->line_number,0);
			}
			break;

			/* handle atoms that take a name */
		case ATOM_ACTIVATION:
			printf("ignoring activation\n");
			break;

			/* string atoms */
		case ATOM_REPEATS:
			printf("ignoring repeat\n");
			break;

		case ATOM_COPYRIGHT:
			printf("ignoring copyright\n");
			break;

		case ATOM_SECTION:
			if (application_id != 0)
			{
				node->application_id = application_id; 
				new_section(local_application_list[application_id],string,string_length,fixed);
			}
			else if (node->application_id != 0)
			{
				/* need to extend the section data */
				extend_section(local_application_list[node->application_id],string,string_length,fixed);
			}
			else
			{
				result = EC_ATOM_REQUIRES_APPLICATION_ATOM;
				raise_warning(line_number,result,NULL,NULL);
			}
			break;

		case ATOM_OPTION:
			if (node->option.name != NULL)
			{
				result = EC_MULTIPLE_DEFINITION_OPTION;
				raise_warning(line_number,result,NULL,NULL);
			}
			else
			{
				copy_name(&temp,&node->option);
			}

			if (application_id != 0)
			{
				node->application_id = application_id;
			}
			break;

		case ATOM_VALUE:
			if (node->value.name != NULL)
			{
				result = EC_MULTIPLE_DEFINITION_VALUE;
				raise_warning(line_number,result,NULL,NULL);
			}
			else
			{
				copy_name(&temp,&node->value);
			}

			if (application_id != 0)
			{
				node->application_id = application_id;
			}
			break;

		default:
			printf("atom: %d\n",record[RECORD_ATOM]);
			line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
			raise_warning(line_number,EC_UNKNOWN_ATOM,NULL,NULL);
			result = EC_UNKNOWN_ATOM;
			break;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_api_function_type
 * Desc : This function will decode the record and add it to the current api
 *        function.
 *--------------------------------------------------------------------------------*/
static unsigned int	decode_api_function_type(API_FUNCTION* function, ATOM_ATOMS atom, NAME* type, NAME* name, NAME* brief)
{
	unsigned int	result = 0;

	if (atom == ATOM_API)
	{
		/* set the API's name and type */
		function->name.name = name->name;
		function->name.name_length = name->name_length;
	
		function->return_type.name = type->name;
		function->return_type.name_length = type->name_length;
	}
	else if (atom == ATOM_PARAMETER)
	{
		API_PARAMETER* new_parameter = calloc(1,sizeof(API_PARAMETER));

		new_parameter->name.name = name->name;
		new_parameter->name.name_length = name->name_length;
		new_parameter->type.name = type->name;
		new_parameter->type.name_length = type->name_length;
		new_parameter->brief.name = brief->name;
		new_parameter->brief.name_length = brief->name_length;

		new_parameter->next = function->parameter_list;
		function->parameter_list = new_parameter;
	}
	else
	{
		/* TODO: fix raise_warning to allow for other types */
		result = EC_UNEXPECTED_ATOM;
		raise_warning (0,result,NULL,NULL);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_type
 * Desc : This function will decode the type record.
 *--------------------------------------------------------------------------------*/
static unsigned int	decode_type(unsigned char* record, unsigned int record_length, NAME* type, NAME* name, NAME* brief)
{
	unsigned int 	pos = 0;
	unsigned int 	result = 0;
	unsigned short	length;

	type->fixed = 0;
	name->fixed = 0;
	brief->fixed = 0;

	/* get the type */
	length = ((((unsigned int)record[pos]) << 8) | record[pos+1]);
	type->name_length = length;

	if (length > 0)
	{
		type->name = malloc(length);
		memcpy(type->name,&record[2],length);
	}

	/* get the name */
	pos += 2 + length;
	
	length = ((((unsigned int)record[pos]) << 8) | record[pos+1]);
	name->name_length = length;

	if (length > 0)
	{
		name->name = malloc(length);
		memcpy(name->name,&record[2+pos],length);
	}

	/* get the name */
	pos += 2 + length;
	
	length = ((((unsigned int)record[pos]) << 8) | record[pos+1]);
	brief->name_length = length;

	if (length > 0)
	{
		brief->name = malloc(length);
		memcpy(brief->name,&record[2+pos],length);
	}

	return result;
}


/*----- FUNCTION -----------------------------------------------------------------*
 * Name : new_constant
 * Desc : This function will create a new constant and add it to the end of the
 *        current constant list.
 *--------------------------------------------------------------------------------*/
static void	new_constant(API_CONSTANTS* constants)
{
	API_CONSTANT*	result = calloc(1,sizeof(API_CONSTANT));

	if (constants->constant_list == NULL)
	{
		constants->constant_list = result;
		constants->last_constant = result;
	}
	else
	{
		constants->last_constant->next = result;
	}

	/* it is a constant from a type not a define */
	result->constant = 1;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : process_input
 * Desc : This function will read an input file and add the atoms to the set if
 *        atoms. It will also handle the first parse on the building of the 
 *        connections.
 *--------------------------------------------------------------------------------*/
static unsigned int	process_input(const char* filename)
{
	int				infile;
	int				bytes_read;
	unsigned int 	result = 0;
	unsigned int	special_group;
	unsigned int	in_api_group = 0;
	unsigned int	line_number = 0;
	unsigned int	num_groups = 1;
	unsigned int	num_functions = 1;
	unsigned int	num_applications = 0;
	unsigned int	num_api_functions = 0;
	unsigned int	record_size;
	unsigned int	file_name_size;
	unsigned int	new_block_number;
	unsigned int	current_api_function = MAX_GROUPS_PER_FILE;
	unsigned char	signature[4] = COMPILED_SOURCE_MAGIC;
	unsigned char	record[RECORD_DATA_START];
	unsigned char	header[FILE_HEADER_SIZE];
	unsigned short	group;
	NAME			type;
	NAME			name;
	NAME			brief;
	BLOCK_NODE		block_node;

	static GROUP*			local_group[MAX_GROUPS_PER_FILE];
	static FUNCTION*		local_functions[MAX_GROUPS_PER_FILE];
	static APPLICATION*		local_applications[MAX_GROUPS_PER_FILE];
	static API_FUNCTION*	local_api_functions[MAX_GROUPS_PER_FILE];
	static unsigned char	source_file_name[MAX_FILENAME+1];

	memset(&block_node,0,sizeof(block_node));
	block_node.line_number = 0;

	local_group[0] = &g_group_tree;

	g_input_filename = NULL;
	g_source_filename = source_file_name;
	g_source_filename[0] = '\0';

	if ((infile = open(filename,READ_FILE_STATUS,0)) != -1)
	{
		/* reset the local list */
		num_groups = 1;
		num_functions = 1;

		if (read(infile,header,FILE_HEADER_SIZE) == FILE_HEADER_SIZE)
		{
			if(memcmp(header,signature,4) == 0)
			{
				/* we have a object file -- we are ignoring all other files at this pass */
				if (header[FILE_VERSION_MAJOR] > VERSION_MAJOR)
				{
					raise_warning(0,EC_INPUT_FILE_BUILT_WITH_LATER_MAJOR_VERSION,NULL,NULL);
					result = 1;
				}

				/* read the input file name */
				file_name_size = ((((unsigned int)header[FILE_NAME_START]) << 8) | header[FILE_NAME_START+1]);

				if (file_name_size < MAX_FILENAME)
				{
					bytes_read = read(infile,source_file_name,file_name_size);
					g_source_filename_length = file_name_size;
				}
				else
				{
					while(file_name_size > MAX_FILENAME)
					{
						read(infile,source_file_name,MAX_FILENAME);
						file_name_size -= MAX_FILENAME;
					}

					bytes_read = read(infile,source_file_name,MAX_FILENAME);
				}

				source_file_name[bytes_read] = '\0';

				/* now read the atoms */
				while ((bytes_read = read(infile,record,RECORD_DATA_START)) == RECORD_DATA_START)
				{
					line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
					record_size = ((((unsigned int)record[RECORD_DATA_SIZE]) << 8) | record[RECORD_DATA_SIZE+1]);
					new_block_number = ((((unsigned int)record[RECORD_BLOCK_NUM]) << 8) | record[RECORD_BLOCK_NUM+1]);

					/* check to see if the block number changes */
					if (new_block_number != block_node.block_number && !in_api_group)
					{
						/* ignore all blocks that are found while in a type */
						if (block_node.line_number != 0)
						{
							/* add the block to the output */
							add_block(&block_node,local_group,local_functions,local_applications);
						}

						memset(&block_node,0,sizeof(block_node));
						block_node.line_number = line_number;
						block_node.block_number = new_block_number;
					}

					if (record_size > 0 && read(infile,record_buffer,record_size) != record_size)
					{
						/* failed to read the record --- problem with the file */
						raise_warning(0,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)filename,NULL);
						result = 1;
					}
					else
					{
						switch(record[RECORD_TYPE])
						{
							case INTERMEDIATE_RECORD_EMPTY:
								if (record[RECORD_ATOM] == ATOM_API)
								{
									current_api_function = MAX_GROUPS_PER_FILE;
								}
								else if (record[RECORD_ATOM] == ATOM_CONSTANTS)
								{
									/* Start the CONSTANT grouping */
									if (block_node.api_constants != NULL)
									{
										result = EC_MULTIPLE_CONSTANTS_REQUEST_WITHOUT_END;
										raise_warning(line_number,result,NULL,NULL);
									}
									else
									{
										block_node.api_constants = calloc(1,sizeof(API_CONSTANTS));
										in_api_group = 1;
									}
								}
								else if (record[RECORD_ATOM] == ATOM_END_CONSTANTS)
								{
									/* End the CONSTANT grouping */
									in_api_group = 0;
								}
								break;

							case INTERMEDIATE_RECORD_FUNCTION:
								if ((local_functions[num_functions] = find_function(record_buffer,record_size)) == NULL)
								{
									local_functions[num_functions] = add_function(record_buffer,record_size,0);
								}
								num_functions++;
								break;

							case INTERMEDIATE_RECORD_APPLICATION:
								local_applications[num_applications] = find_add_application(record_buffer,record_size);
								num_applications++;
								break;

							case INTERMEDIATE_RECORD_SAMPLE:
								 add_sample(record_buffer,record_size);
								break;

							case INTERMEDIATE_RECORD_API:
								group = (((unsigned int)(record[RECORD_GROUP]) << 8) | record[RECORD_GROUP+1]);

								if (group == DEFAULT_GROUP)
								{
									group = 0;
								}

								if (current_api_function != MAX_GROUPS_PER_FILE)
								{
									result = EC_NESTED_FUNCTION_DEFINITIONS_NOT_ALLOWED;
									raise_warning(line_number,result,NULL,NULL);
								}

								if ((local_api_functions[num_api_functions] = find_api_function(record_buffer,record_size,local_group[group])) == NULL)
								{
									local_api_functions[num_api_functions] = add_api_function(record_buffer,record_size,local_group[group]);
								}

								current_api_function = num_api_functions;
								num_api_functions++;
								break;

							case INTERMEDIATE_RECORD_TYPE:
								result = decode_type(record_buffer,record_size,&type,&name,&brief);
	
								if (block_node.api_type != NULL)
								{
									add_api_type_record(block_node.api_type,ATOM_TYPE_RECORD,&type,&name,&brief);
								}
								else if (block_node.api_constants != NULL)
								{
									add_api_constant_record(block_node.api_constants,ATOM_TYPE_RECORD,&type,&name,&brief);
								}
								else if (current_api_function != MAX_GROUPS_PER_FILE)
								{
									decode_api_function_type(local_api_functions[current_api_function],record[RECORD_ATOM],&type,&name,&brief);
								}
								else
								{
									result = EC_TYPE_DEFINITION_IN_INVALID_PLACE;
									raise_warning(line_number,result,NULL,NULL);
								}
								break;

							case INTERMEDIATE_RECORD_GROUP:
								/* does not have a name then it should map to the default */
								if (record_size > 0)
								{
									if ((local_group[num_groups] = find_group(&g_group_tree,record_buffer,record_size,&special_group)) == NULL)
									{
										if (special_group != NORMAL_GROUP)
										{
											result = EC_SPECIFIED_SPECIAL_GROUP_IN_MODEL;
											raise_warning(line_number,result,NULL,NULL);
										}

										local_group[num_groups] = add_group(&g_group_tree,record_buffer,record_size);
									}

									num_groups++;
								}
								break;

							case INTERMEDIATE_RECORD_NAME:
							case INTERMEDIATE_RECORD_STRING:
							case INTERMEDIATE_RECORD_MULTILINE:
								if (block_node.api_constants != NULL)
								{
									add_atom_to_constant( 	local_group,
															local_functions,
															local_api_functions,
															&block_node,
															record,
															record_buffer,
															record_size);
								}
								else if (block_node.api_type != NULL)
								{
									add_atom_to_type( 	local_group,
														local_functions,
														local_api_functions,
														&block_node,
														record,
														record_buffer,
														record_size);
								}
								else
								{
									add_atom_to_block(	local_group,
														local_functions,
														local_applications,
														local_api_functions,
														&block_node,
														record,
														record_buffer,
														record_size);
								}
								break;

							case INTERMEDIATE_RECORD_NUMBERIC:
								add_numeric_to_block(	local_group,
														local_functions,
														local_api_functions,
														&block_node,
														record,
														record_buffer,
														record_size);
								break;

							case INTERMEDIATE_RECORD_PAIR:
								add_pair_to_block(	local_group,
													local_functions,
													local_applications,
													local_api_functions,
													&block_node,
													record,
													record_buffer,
													record_size);
								break;

							case INTERMEDIATE_RECORD_START:
								group = (((unsigned int)(record[RECORD_GROUP]) << 8) | record[RECORD_GROUP+1]);

								if (group == DEFAULT_GROUP)
								{
									group = 0;
								}

								/* types can be created here */
								/* TODO: this is wrong -- all types must have a type block - originally coded badly.
								 * so this should be fixed.
								 *
								 * The constants are correct already.
								 */
								if (record[RECORD_ATOM] != RECORD_GROUP_CONSTANT)
								{
									block_node.api_type = calloc(1,sizeof(API_TYPE));
									in_api_group = 1;
								}
								else if (block_node.api_constants == NULL)
								{
									raise_warning(0,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)filename,NULL);
								}
								else
								{
									new_constant(block_node.api_constants);
								}

								break;

							case INTERMEDIATE_RECORD_END:
								if (block_node.api_type != NULL)
								{
									in_api_group = 0;
								}
								break;

							case INTERMEDIATE_RECORD_BOOLEAN:
								{
									unsigned int	flags = 0;

									switch(record[RECORD_ATOM])
									{
										case ATOM_REQUIRED: flags = OPTION_FLAG_REQUIRED; break;
										case ATOM_MULTIPLE: flags = OPTION_FLAG_MULTIPLE; break;
									}

									block_node.application_flag |= flags;
								}
								break;

							default:
								printf("%d\n",record[RECORD_TYPE]);
								raise_warning(0,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)filename,NULL);
								result = 1;
						}
					}
				}

				if (block_node.line_number != 0)
				{
					add_block(&block_node,local_group,local_functions,local_applications);
				}
			}
			else
			{
				raise_warning(0,EC_INPUT_FILE_SIGNATURE_INCORRECT,(unsigned char*)filename,NULL);
				result = 1;
			}
		}

		close(infile);
	}

	g_source_filename = NULL;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : check_state_machine
 * Desc : This function will check to see if the state machine is valid.
 *--------------------------------------------------------------------------------*/
static unsigned int	check_state_machine(STATE_MACHINE* state_machine)
{
	unsigned int		result = EC_OK;
	STATE*				current_state = NULL;
	STATE_TRANSITION*	current_trans = NULL;
	
	current_state = state_machine->state_list;

	while (current_state != NULL)
	{
		if (current_state->name[0] == 's' && current_state->name_length == 5 && memcmp(current_state->name,"start",5) == 0)
		{
			/* found the start state */
			state_machine->init_state = current_state;
		}

		if ((current_state->flags & FLAG_GHOST) == FLAG_GHOST)
		{
			if (current_state->name_length == finish_state_length && memcmp(current_state->name,finish_state,finish_state_length) != 0)
			{
				result = EC_UNDEFINDED_REFERENCED_STATE_FOUND;
				raise_warning(current_state->line_number,result,NULL,NULL);
			}
		}

		current_state = current_state->next;
	}

	if (state_machine->init_state == NULL)
	{
		result = EC_STATE_MACHINE_DOES_NOT_HAVE_START;
		raise_warning(0,result,state_machine->group->name,NULL);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : handle_reorder
 * Desc : This function handles the reordering of the node. If the node is either
 *        part of a function or has a sub-wait these are moved to.
 *--------------------------------------------------------------------------------*/
static NODE*	handle_reorder(NODE* search_node, NODE* psearch_node, NODE* previous_node, TIMELINE* timeline)
{
	search_node->flags |= FLAG_DEPENDENCY;

	/* start the insert */
	if (previous_node == NULL)
	{
		timeline->node = search_node;
	}
	else
	{
		previous_node->next = search_node;
	}

	/* is there more that one item to copy 
	 * we need to copy all subsequent waits and the items within a function */
	do
	{
		if (search_node->next != NULL)
		{
			if ((search_node->next->flags & (FLAG_WAIT |FLAG_IN_FUNCTION)) != 0)
			{
				search_node = search_node->next;
			}
			else
			{
				break;
			}
		}
	}
	while (search_node != NULL);

	/* set the end of the insert */
	if (psearch_node == NULL)
	{
		timeline->node = search_node->next;
	}
	else
	{
		psearch_node->next = search_node->next;
	}

	return search_node;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : reorder_nodes
 * Desc : This function will reorder the nodes in the specific timeline.
 *--------------------------------------------------------------------------------*/
static ERROR_CODES	reorder_nodes(TIMELINE* timeline)
{
	int			found = 0;
	NODE*		temp_node;
	NODE*		search_node;
	NODE*		psearch_node;
	NODE*		current_node;
	NODE*		previous_node = NULL;
	ERROR_CODES result = EC_OK;

	if (timeline->node != NULL)
	{
		current_node = timeline->node;

		do
		{
			if (current_node->after.name_length > 0)
			{
				/* we have a relocation */
				psearch_node = NULL;
				search_node = timeline->node;
				found = 0;

				do
				{
					if ((search_node->flags & FLAG_WAIT) == 0)
					{
						if (search_node->function != NULL)
						{
							if (search_node->function->name_length == current_node->after.name_length &&
								memcmp(search_node->function->name,current_node->after.name,search_node->after.name_length) == 0)
							{
								search_node = handle_reorder(search_node,psearch_node,previous_node,timeline);
								search_node->next = current_node;
								found = 1;
								break;
							}
						}
						else if (search_node->sent_message != NULL && search_node->sent_message->name_length > 0)
						{
							if (search_node->sent_message->name_length == current_node->after.name_length &&
								memcmp(search_node->sent_message->name,current_node->after.name,current_node->after.name_length) == 0)
							{
								search_node = handle_reorder(search_node,psearch_node,previous_node,timeline);
								search_node->next = current_node;
								found = 1;
								break;
							}
						}
					}

					psearch_node = search_node;
					search_node = search_node->next;
				}
				while(search_node != NULL);

				if (!found)
				{
					result = EC_AFTER_NODE_DOES_NOT_EXIST;
					raise_warning(current_node->line_number,result,NULL,NULL);
				}
			}

			previous_node = current_node;
			current_node = current_node->next;
		}
		while (current_node != NULL);
	}

	return result;
}


/*----- FUNCTION -----------------------------------------------------------------*
 * Name : insert_function
 * Desc : This function will insert the parts of a function into the current
 *        timeline after the given node. It will also do the basic checks that
 *        are required. Note: this call recursively call itself if the code is
 *        a function.
 *
 *        The depth parameter is used to depth check the call list to stop 
 *        recursive call breaking it.
 *--------------------------------------------------------------------------------*/
static unsigned int	insert_function(TIMELINE* timeline, NODE** start, FUNCTION* function, unsigned int depth)
{
	NODE*			current_node = *start;
	NODE*			new_node = *start;
	MESSAGE*		message = NULL;
	MESSAGE*		new_message = NULL;
	MESSAGE*		wait_message = NULL;
	TIMELINE*		target = NULL;
	FUNCTION_NODE*	function_node = function->node;
	unsigned int	result = EC_OK;

	current_node->flags |= FLAG_FUNCTION;

	if (depth > g_max_call_depth)
	{
		result = EC_MAX_FUNCTION_DEPTH_REACHED;
	}
	else
	{
		while (function_node != NULL)
		{
			new_node = calloc(1,sizeof(NODE));

			/* add the new node into the node list */
			new_node->next = current_node->next;
			new_node->timeline = timeline;
			current_node->next = new_node;
			current_node = new_node;
			new_node->flags = (FLAG_IN_FUNCTION | FLAG_DEPENDENCY);
			target = function_node->to_timeline;

			if (function_node->to_timeline_name.name_length > 0)
			{
				/* now we can fixup the @to timeline */
				if ((target = find_timeline(timeline->group,
											function_node->to_timeline_name.name,
											function_node->to_timeline_name.name_length)) == NULL)
				{
					target = add_timeline(timeline->group,
											function_node->to_timeline_name.name,
											function_node->to_timeline_name.name_length,
											0,
											FLAG_GHOST);
				}
			}

			if (function_node->call != NULL)
			{
				result = insert_function(timeline,&current_node,function_node->call,depth + 1);
				new_node->function = find_function(function_node->call->name,function_node->call->name_length);
			}

			/* handle the send */
			if (function_node->message.name_length > 0)
			{
				new_node->sent_message = create_message(new_node,target,&function_node->message);
				new_node->sent_message->flags = function_node->flags;
			}

			/* "wait" - this needs to fixed-up after the functions are */
			if (function_node->sequence.name_length > 0)
			{
				memcpy(&new_node->wait_message,&function_node->sequence,sizeof(BLOCK_NAME));
			}

			/* handle the sequence */
			if (function_node->after.name_length > 0)
			{
				memcpy(&new_node->after,&function_node->after,sizeof(BLOCK_NAME));
			}

			/* trigger is attached to the node */
			if (function_node->trigger != NULL)
			{
				new_node->trigger = handle_trigger(function_node->trigger,timeline->group);
			}

			/* trigger is attached to the node */
			if (function_node->triggers_list != NULL)
			{
				add_triggers(&new_node->triggers,function_node->triggers_list,timeline->group);
			}

			if (function_node->condition.name_length > 0)
			{
				if (function_node->condition.name_length > 0)
				{
					new_node->condition = malloc(function_node->condition.name_length);
					memcpy(new_node->condition,function_node->condition.name,function_node->condition.name_length);
					new_node->condition_length = function_node->condition.name_length;
				}
			}

			/* build the new node */
			new_node->group = timeline->group;

			/* next function node */
			function_node = function_node->next;
		}

		/* add the end of function node - must have one as the last node may have received a message */
		new_node = calloc(1,sizeof(NODE));
		new_node->flags = FLAG_FUNCTION_END;
		new_node->timeline = timeline;
		new_node->return_node = *start;
		new_node->next = current_node->next;
		current_node->next = new_node;
		
		(*start)->function_end = new_node;

		if (timeline->last_node == *start)
		{
			timeline->last_node = new_node;
		}

		*start = new_node;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : node_reorder
 * Desc : This function will reorder the nodes. If the node has an after then it
 *        will search the list of nodes to find out where the after message is
 *        sent or waited for and place the node there.
 *--------------------------------------------------------------------------------*/
static ERROR_CODES	node_reorder(SEQUENCE_DIAGRAM* sequence_diagram)
{
	NODE*			current_node;
	TIMELINE*		current_timeline;
	ERROR_CODES		error;
	unsigned int	result = EC_OK;
			
	if (sequence_diagram != NULL)
	{
		current_timeline = sequence_diagram->timeline_list;
		
		while (current_timeline != NULL)
		{
			if ((error = reorder_nodes(current_timeline)) != EC_OK)
			{
				result = EC_FAILED;
			}

			current_timeline = current_timeline->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : function_fixup
 * Desc : This function will fixup the function references in the tree. It will
 *        just add the functions and all the atoms that are associated with the
 *        function into the tree at the current location.
 *--------------------------------------------------------------------------------*/
static unsigned int	function_fixup(SEQUENCE_DIAGRAM* sequence_diagram)
{
	NODE*			current_node;
	TIMELINE*		current_timeline;
	unsigned int	error;
	unsigned int	result = EC_OK;

	if (sequence_diagram != NULL)
	{
		current_timeline = sequence_diagram->timeline_list;
		
		while (current_timeline != NULL)
		{
			current_node = current_timeline->node;

			while (current_node != NULL)
			{
				if (current_node->function != NULL)
				{
					if ((current_node->function->flags & FLAG_GHOST) != 0)
					{
						result = EC_UNDEFINED_FUNCTION;
						raise_warning(0,result,current_node->function->name,NULL);
					}
					else
					{
						/* ok, now we have a function its contents will have to be inserted here */
						if ((error = insert_function(current_timeline,&current_node,current_node->function,0)) != EC_OK)
						{
							result = error;
						}
					}
				}

				current_node = current_node->next;
			}

			current_timeline = current_timeline->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : copy_node
 * Desc : This function this will copy a node and add it to the list of nodes
 *        that have been copied.
 *--------------------------------------------------------------------------------*/
static NODE*	copy_node(NODE* node, NODE_LIST* list)
{
	NODE*		result;
	NODE_LIST*	list_end = list;
	NODE_LIST*	list_entry = list;

	result = malloc(sizeof(NODE));
	memcpy(result,node,sizeof(NODE));
	result->next = NULL;

	while (list_entry != NULL)
	{
		if (list_entry->timeline == node->timeline)
		{
			/* found the list that it belongs to */
			list_entry->last->next = result;
			list_entry->last = list_entry->last->next;
			break;
		}

		list_end = list_entry;
		list_entry = list_entry->next;
	}

	if (list_entry == NULL)
	{
		/* ok, did not find the time line so add it to the end */
		list_end->next = calloc(1,sizeof(NODE_LIST));
		list_end->next->last = result;
		list_end->next->timeline = node->timeline;
		list_end->next->node = result;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : merge_nodes
 * Desc : This function will merge the copied list of nodes into the sequence
 *        diagram.
 *
 *        This function will also tidy up the function pointers for the copied
 *        nodes.
 *--------------------------------------------------------------------------------*/
static void	merge_nodes(NODE_LIST* list, NODE* insert_node)
{
	NODE*		end_node;
	NODE*		start_node;
	NODE*		current_node;
	NODE*		function_start;
	NODE_LIST*	current_list = list->next;

	while (current_list != NULL)
	{
		current_node = current_list->node;
		start_node = current_node;
		end_node = current_node;

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

			end_node = current_node;
			current_node = current_node->next;
		}

		/* add the copied node to the correct timeline */
		if (insert_node->timeline == current_list->timeline)
		{
			/* ok, we have the copied node here */
			end_node->next = insert_node->next;
			insert_node->next = start_node;
		}
		else
		{
			/* add to the end of the relevant timeline */
			current_list->timeline->last_node->next = start_node;
			current_list->timeline->last_node = end_node;
		}

		current_list = current_list->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : duplicate_function
 * Desc : This function will duplicate a node and all off the sub-nodes and the
 *        messages that the node has. It will add the duplicate nodes to the end 
 *        of the timelines that they belong in.
 *--------------------------------------------------------------------------------*/
static NODE*	duplicate_function(NODE* start_node)
{
	NODE* 		result = start_node;
	NODE* 		copy;
	NODE* 		prev_node;
	NODE* 		stop_node = start_node->function_end;
	NODE* 		active_node = start_node;
	MESSAGE*	new_message;
	NODE_LIST	insert_list = {NULL,NULL,NULL,NULL};

	start_node->return_node = NULL;
	
	copy = copy_node(active_node,&insert_list);
	result = copy;

	while (active_node != NULL && stop_node != active_node)
	{
		prev_node = copy;
		active_node = next_active_node(active_node);

		copy = copy_node(active_node,&insert_list);

		if (prev_node->sent_message != NULL)
		{
			new_message = malloc(sizeof(MESSAGE));
			memcpy(new_message,prev_node->sent_message,sizeof(MESSAGE));
			
			prev_node->sent_message = new_message;
			new_message->sender = prev_node;

			if (prev_node->sent_message->receiver != NULL)
			{
				new_message->receiver = copy;
			}
		}
	}

	merge_nodes(&insert_list,stop_node);

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_wait_node
 * Desc : This function will find the waiting message that matches the send in
 *        the given node. It will return the node that it finds.
 *--------------------------------------------------------------------------------*/
static NODE*	find_wait_node(MESSAGE* search_message, MESSAGE* sent_message)
{
	NODE*			result = NULL;
	NODE*			temp_node;
	NODE*			start_node;
	NODE*			insert_node;
	NODE*			current_node = search_message->sender->timeline->node;
	TIMELINE*		target_timeline = search_message->sender->timeline;

	while (current_node != NULL)
	{
		if (sent_message->name_length == current_node->wait_message.name_length &&
			memcmp(sent_message->name,current_node->wait_message.name,sent_message->name_length) == 0)
		{
			if (current_node->received_message == NULL)
			{
				result = current_node;
				break;
			}
			else if ((search_message->flags & FLAG_BROADCAST) != 0)
			{
				if (current_node->flags & FLAG_FUNCTION)
				{
					result = duplicate_function(current_node);
				}
				else if (((current_node->flags & FLAG_IN_FUNCTION) == 0))
				{
					/* message to a non-function node */
					temp_node = malloc(sizeof(NODE));
					memcpy(temp_node,current_node,sizeof(NODE));
					temp_node->next = current_node->next;
					current_node->next = temp_node;
					result = temp_node;
				}
				else
				{
					printf("failed ---- currently not supported - messages to inside a function -- s/b reponses \n");
				}

				break;
			}
		}
		current_node = current_node->next;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_wait
 * Desc : This function will find the waiting message that matches the send in
 *        the given node. It will then attach the waiting message to sending 
 *        message.
 *--------------------------------------------------------------------------------*/
static unsigned int	find_wait(TIMELINE* search_timeline, TIMELINE* timeline, NODE** sending_node, unsigned int broadcast)
{
	NODE*			temp_node;
	NODE*			current_node;
	MESSAGE*		new_message;
	MESSAGE*		temp_message;
	MESSAGE*		sent_message = (*sending_node)->sent_message;
	TIMELINE*		target_timeline = search_timeline;
	unsigned int	result = EC_FAILED;

	if (search_timeline == NULL)
	{
		target_timeline = timeline;
	}

	current_node = target_timeline->node;

	while (current_node != NULL)
	{
		if (current_node->wait_message.name_length > 0 && 
			sent_message->name_length == current_node->wait_message.name_length &&
			memcmp(sent_message->name,current_node->wait_message.name,sent_message->name_length) == 0)
		{
			/* add to the list of received messages */
			if (current_node->received_message != NULL)
			{
				if (sent_message->receiver != NULL)
				{
					printf("supported cascade copy required\n");
				}
				else
				{
					/* copy the node as the message needs to be unique */
					temp_node = malloc(sizeof(NODE));
					memcpy(temp_node,current_node,sizeof(NODE));

					/* link them in the chain */
					temp_node->next = current_node->next;
					current_node->next = temp_node;
					current_node = temp_node;

					current_node->received_message = sent_message;
					sent_message->target_timeline = target_timeline;
					sent_message->receiver = current_node;
				}
				result = EC_OK;
				break;
			}
			else if (current_node->received_message == NULL)
			{
				if (sent_message->receiver != NULL)
				{
					TIMELINE* sending_timeline = sent_message->sending_timeline;

					/* copy the node as the message needs to be unique */
					temp_node = malloc(sizeof(NODE));
					memcpy(temp_node,*sending_node,sizeof(NODE));

					/* copy the message */
					temp_message = malloc(sizeof(MESSAGE));
					memcpy(temp_message,sent_message,sizeof(MESSAGE));

					temp_message->sender = temp_node;
					temp_message->receiver = current_node;
					temp_message->target_timeline = target_timeline;

					temp_node->sent_message = temp_message;

					/* link them in the chain */
					(*sending_node)->next = temp_node;
					*sending_node = temp_node;

					if (*sending_node == sending_timeline->last_node)
					{
						sending_timeline->last_node = temp_node;
					}

					current_node->received_message = temp_message;
				}
				else
				{
					/* make the connection between the two nodes */
					sent_message->receiver = current_node;
					sent_message->target_timeline = target_timeline;
					current_node->received_message = sent_message;
				}

				result = EC_OK;
				break;
			}
		}

		current_node = current_node->next;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : connect_responses
 * Desc : This function will connect the messages that are responses together to
 *        create the sequence diagram.
 *
 *        This also as a side effect (to save a couple of cycles) also sets the
 *        function end message to the same one as the function start.
 *--------------------------------------------------------------------------------*/
static unsigned int	connect_responses(SEQUENCE_DIAGRAM* sequence_diagram)
{
	NODE*			target_node;
	NODE*			current_node;
	NODE*			function_node = NULL;
	MESSAGE*		current_message;
	TIMELINE*		broadcast;
	TIMELINE*		search_timeline;
	TIMELINE*		current_timeline;
	FUNCTION*		current_function = NULL;
	DEFFERED_LIST*	temp = NULL;
	DEFFERED_LIST*	current_deferred = NULL;
	DEFFERED_LIST*	deferred_message = NULL;
	unsigned int	error;
	unsigned int	satisfied;
	unsigned int	is_broadcast;
	unsigned int	result = EC_OK;

	/* TODO: replace this function will one based on the walker code. This will solve the 
	 *       problem of nested functions.
	 */
	if (sequence_diagram != NULL)
	{
		broadcast = find_timeline(sequence_diagram->group,(unsigned char*)"broadcast",sizeof("broadcast")-1);

		current_timeline = sequence_diagram->timeline_list;
		
		while (current_timeline != NULL)
		{
			current_node = current_timeline->node;

			while (current_node != NULL)
			{
				if ((current_node->flags & FLAG_FUNCTION) != 0)
				{
					/* if walking into a function remember then entry as any responses
					 * within a function go to the sending timeline.
					 */
					/* TODO: need to handle nested functions. Will have to set the function_end
					 *       node to the function that called the function.
					 */
					if (function_node != NULL)
					{
						printf("function not null ---- NESTED FUNCITON not currently handled \n");
					}
					else
					{
						function_node = current_node;
					}
				}
				else if ((current_node->flags & FLAG_FUNCTION_END) != 0)
				{
					function_node = NULL;
				}
				else if (current_node->sent_message != NULL && (current_node->sent_message->flags & FLAG_SEQUENCE_RESPONDS) != 0)
				{
					target_node = NULL;

					if (current_node->received_message != NULL)
					{
						target_node = find_wait_node(current_node->received_message,current_node->sent_message);
					}
					else if (function_node != NULL && function_node->received_message != NULL)
					{
						target_node = find_wait_node(function_node->received_message,current_node->sent_message);
					}

					if (target_node != NULL)
					{
						target_node->received_message = current_node->sent_message;
						current_node->sent_message->receiver = target_node;
						current_node->sent_message->target_timeline = target_node->timeline;
					}
					else
					{
						/*TODO: handle responses to reponses */
						printf("deferred: %s ---- NOT CURRENTLY HANDLED \n",current_node->sent_message->name);
					}
				}

				current_node = current_node->next;
			}
			current_timeline = current_timeline->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : connect_sequence
 * Desc : This function will connect the messages from the different timelines
 *        together to create the sequence diagram.
 *
 *        SIDE-EFFECT WARNING: this code also flags any broadcast message as a 
 *        broadcast. Done here to safe some cycles.
 *--------------------------------------------------------------------------------*/
static unsigned int	connect_sequence(SEQUENCE_DIAGRAM* sequence_diagram)
{
	NODE*			current_node;
	TIMELINE*		broadcast;
	TIMELINE*		search_timeline;
	TIMELINE*		current_timeline;
	unsigned int	error;
	unsigned int	result = EC_OK;

	if (sequence_diagram != NULL)
	{
		broadcast = find_timeline(sequence_diagram->group,(unsigned char*)"broadcast",sizeof("broadcast")-1);

		if (broadcast != NULL)
		{
			broadcast->flags |= FLAG_BROADCAST;
		}

		current_timeline = sequence_diagram->timeline_list;
		
		while (current_timeline != NULL)
		{
			current_node = current_timeline->node;

			while (current_node != NULL)
			{
				if (current_node->sent_message != NULL)
				{
					if (current_node->sent_message->target_timeline == broadcast)
					{
						current_node->sent_message->flags |= FLAG_BROADCAST;
					}

					if ((current_node->sent_message->flags & FLAG_SEQUENCE_RESPONDS) == 0)
					{
						if (current_node->sent_message->target_timeline == broadcast)
						{
							/* message to broadcast */
							search_timeline = sequence_diagram->timeline_list;
						
							while(search_timeline != NULL)
							{
								find_wait(search_timeline,current_timeline,&current_node,1);

								search_timeline = search_timeline->next;
							}
						}
						else if (find_wait(current_node->sent_message->target_timeline,current_timeline,&current_node,0) != EC_OK)
						{
							raise_warning(0,EC_SEND_AND_NO_WAIT,current_node->sent_message->name,NULL);
							result = EC_FAILED;
						}
					}
				}

				current_node = current_node->next;
			}

			current_timeline = current_timeline->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : semantic_check
 * Desc : This function will check that the files input are sensible and all the
 *        nodes that have to be defined are properly defined.
 *--------------------------------------------------------------------------------*/
static unsigned int semantic_check ( void )
{
	GROUP*				current = &g_group_tree;
	TRIGGER*			current_trigger = NULL;
	unsigned int		result = EC_OK;

	while (current != NULL)
	{
		if (current->state_machine != NULL)
		{
			result = check_state_machine(current->state_machine);
		}
		
		if (current->sequence_diagram != NULL)
		{
			/* handle sequence diagram */
			if ((result = node_reorder(current->sequence_diagram)) == EC_OK)
			{
				if ((result = function_fixup(current->sequence_diagram)) == EC_OK)
				{
					if ((result = connect_sequence(current->sequence_diagram)) == EC_OK)
					{
						result = connect_responses(current->sequence_diagram);
					
					}
				}
			}
		}

		/* check the triggers are valid */
		if (current->trigger_list != NULL)
		{
			current_trigger = current->trigger_list;

			while (current_trigger != NULL)
			{
				if ((current_trigger->flags & FLAG_GHOST) == FLAG_GHOST)
				{
					result = EC_UNDEFINED_TRIGGER;
					raise_warning(0,result,current_trigger->name,NULL);
				}

				current_trigger = current_trigger->next;
			}
		}

		current = current->next;
	}

	return result;
}



/*----- FUNCTION -----------------------------------------------------------------*
 * Name : open_file
 * Desc : This function will open the output file and initialise the output
 *        structure for the file.
 *--------------------------------------------------------------------------------*/
static int	open_file(OUTPUT_FILE* file, char* file_name)
{
	int result = 0;

	if ((file->outfile = open(file_name,WRITE_FILE_STATUS,WRITE_FILE_PERM)) != -1)
	{
		file->buffer = malloc(FILE_BLOCK_SIZE);
		file->offset = 0;

		result = 1;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_to_file
 * Desc : This function will write the specific record to the file. It will handle
 *        the block structure and padding.
 *--------------------------------------------------------------------------------*/
static void	write_to_file(OUTPUT_FILE* file)
{
	unsigned int count = 0;

	if ((file->record_size + file->offset) > FILE_BLOCK_SIZE)
	{
		if (file->offset < FILE_BLOCK_SIZE)
		{
			file->buffer[file->offset++] = LINKER_BLOCK_END;
		}

		write(file->outfile,file->buffer,FILE_BLOCK_SIZE);
		file->offset = 0;
	}

	/* Ok, copy the parts to the output */
	for (count=0;count < file->parts; count++)
	{
		if (file->offset >= FILE_BLOCK_SIZE)
		{
			raise_warning(0,EC_DATA_TOO_LARGE,NULL,NULL);
		}
		else
		{
			memcpy(&file->buffer[file->offset],file->buffer_list[count].buffer,file->buffer_list[count].size);
			file->offset += file->buffer_list[count].size;
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : close_file
 * Desc : This function will close the output file and flush the last write
 *        block to the file.
 *--------------------------------------------------------------------------------*/
static void	close_file(OUTPUT_FILE* file)
{
	if (file->offset > 0)
	{
		file->buffer[file->offset++] = LINKER_BLOCK_END;
		write(file->outfile,file->buffer,FILE_BLOCK_SIZE);
	}
	
	free(file->buffer);
	file->buffer = NULL;

	close(file->outfile);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_empty_record
 * Desc : This function writes empty record to the file.
 *--------------------------------------------------------------------------------*/
static void	write_empty_record(unsigned char type, OUTPUT_FILE* file)
{
	unsigned char	buffer[1];

	buffer[0] = type;

	file->parts = 1;
	file->record_size = 1;
	file->buffer_list[0].size = 1;
	file->buffer_list[0].buffer = buffer;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_numerics_record
 * Desc : This function writes a short and a int to the file.
 *--------------------------------------------------------------------------------*/
static void	write_numerics_record(unsigned char type, OUTPUT_FILE* file, unsigned short param1, unsigned int param2)
{
	unsigned char	buffer[7];

	buffer[0] = type;
	buffer[1] = (param1 >> 8) & 0xff;
	buffer[2] = param1 & 0xff;
	
	buffer[3] = (param2 & 0xff000000) >> 24;
	buffer[4] = (param2 & 0x00ff0000) >> 16;
	buffer[5] = (param2 & 0x0000ff00) >> 8;
	buffer[6] = (param2 & 0xff);

	file->parts = 1;
	file->record_size = 7;
	file->buffer_list[0].size = 7;
	file->buffer_list[0].buffer = buffer;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_numeric_record
 * Desc : This function writes a short to the file.
 *--------------------------------------------------------------------------------*/
static void	write_numeric_record(unsigned char type, OUTPUT_FILE* file, unsigned short param)
{
	unsigned char	buffer[3];

	buffer[0] = type;
	buffer[1] = (param >> 8) & 0xff;
	buffer[2] = param & 0xff;

	file->parts = 1;
	file->record_size = 3;
	file->buffer_list[0].size = 3;
	file->buffer_list[0].buffer = buffer;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_string_param_record
 * Desc : This function writes a string record to the file. With a parameter.
 *--------------------------------------------------------------------------------*/
static void	write_string_param_record(unsigned char type, OUTPUT_FILE* file,unsigned char* string,unsigned short string_length, unsigned short param)
{
	unsigned char	buffer[4];

	buffer[0] = type;
	buffer[1] = (param >> 8) & 0xff;
	buffer[2] = param & 0xff;
	buffer[3] = string_length & 0xff;

	file->parts = 2;
	file->record_size = 4 + string_length;
	file->buffer_list[0].size = 4;
	file->buffer_list[0].buffer = buffer;
	file->buffer_list[1].size = string_length;
	file->buffer_list[1].buffer = string;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_string_record
 * Desc : This function writes a string record to the file.
 *--------------------------------------------------------------------------------*/
static void	write_string_record(unsigned char type, OUTPUT_FILE* file, NAME* string)
{
	unsigned char	buffer[4];

	buffer[0] = type;
	buffer[1] = string->fixed;
	buffer[2] = (string->name_length >> 8) & 0xff;
	buffer[3] = string->name_length & 0xff;

	file->parts = 2;
	file->record_size = 4 + string->name_length;
	file->buffer_list[0].size = 4;
	file->buffer_list[0].buffer = buffer;
	file->buffer_list[1].size = string->name_length;
	file->buffer_list[1].buffer = string->name;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_group_id_record
 * Desc : This function writes a group record with an id attached..
 *--------------------------------------------------------------------------------*/
static void	write_group_id_record(	unsigned char	type, 
								OUTPUT_FILE*	file, 
								GROUP*			group, 
								unsigned char*	string,
								unsigned short	string_length,
								unsigned short	id)
{
	unsigned char	buffer[5];

	buffer[0] = type;
	buffer[1] = (id >> 8) & 0xff;
	buffer[2] = (id & 0xff);
	buffer[3] = group->name_length & 0xff;
	buffer[4] = string_length & 0xff;

	file->parts = 4;
	file->record_size = 4 + group->name_length + 1 + string_length;
	file->buffer_list[0].size = 4;
	file->buffer_list[0].buffer = buffer;
	file->buffer_list[1].size = group->name_length;
	file->buffer_list[1].buffer = group->name;
	file->buffer_list[2].size = 1;
	file->buffer_list[2].buffer = &buffer[4];
	file->buffer_list[3].size = string_length;
	file->buffer_list[3].buffer = string;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_option_record
 * Desc : This function writes a group option record.
 *--------------------------------------------------------------------------------*/
static void	write_option_record( unsigned char type, OUTPUT_FILE* file, OPTION* option)
{
	unsigned char	fixed = 0;
	unsigned char	buffer[10];

	if (option->name.fixed)
	{
		fixed = 0x01;
	}

	if (option->value.fixed)
	{
		fixed |= 0x02;
	}

	if (option->description.fixed)
	{
		fixed |= 0x04;
	}

	buffer[0] = type;
	buffer[1] = (option->flags & 0xff00) >> 8;
	buffer[2] = option->flags & 0x00ff;
	buffer[3] = fixed;
	buffer[4] = (option->name.name_length & 0xff00) >> 8;
	buffer[5] = option->name.name_length & 0xff;
	buffer[6] = (option->value.name_length & 0xff00) >> 8;
	buffer[7] = option->value.name_length & 0xff;
	buffer[8] = (option->description.name_length & 0xff00) >> 8;
	buffer[9] = option->description.name_length & 0xff;

	file->parts = 4;
	file->record_size = sizeof(buffer) + option->name.name_length + option->value.name_length + option->description.name_length;
	file->buffer_list[0].size = sizeof(buffer);
	file->buffer_list[0].buffer	= buffer;
	file->buffer_list[1].size	= option->name.name_length;
	file->buffer_list[1].buffer = option->name.name;
	file->buffer_list[2].size	= option->value.name_length;
	file->buffer_list[2].buffer = option->value.name;
	file->buffer_list[3].size	= option->description.name_length;
	file->buffer_list[3].buffer = option->description.name;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: write_synopsis_record
 * @desc: This function will write te synopsis record to the output file.
 *--------------------------------------------------------------------------------*/
static void	write_synopsis_record(unsigned char type, OUTPUT_FILE* file, NAME* name, unsigned char* synopsis_index, unsigned int index_size)
{
	unsigned char	buffer[5];

	buffer[0] = type;
	buffer[1] = (name->name_length & 0xff00) >> 8;
	buffer[2] = name->name_length & 0xff;
	buffer[3] = (index_size & 0xff00) >> 8;
	buffer[4] = index_size & 0xff;

	file->parts = 3;
	file->record_size = sizeof(buffer) + name->name_length + index_size;
	file->buffer_list[0].size = sizeof(buffer);
	file->buffer_list[0].buffer	= buffer;
	file->buffer_list[1].size	= name->name_length;
	file->buffer_list[1].buffer = name->name;
	file->buffer_list[2].size	= index_size;
	file->buffer_list[2].buffer = synopsis_index;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_group_record
 * Desc : This function writes a group record.
 *--------------------------------------------------------------------------------*/
static void	write_group_record(unsigned char type, OUTPUT_FILE* file, GROUP* group, unsigned char* string, unsigned short string_length)
{
	unsigned char	buffer[3];

	buffer[0] = type;
	buffer[1] = group->name_length & 0xff;
	buffer[2] = string_length & 0xff;

	file->parts = 4;
	file->record_size = 2 + group->name_length + 1 + string_length;
	file->buffer_list[0].size = 2;
	file->buffer_list[0].buffer = buffer;
	file->buffer_list[1].size = group->name_length;
	file->buffer_list[1].buffer = group->name;
	file->buffer_list[2].size = 1;
	file->buffer_list[2].buffer = &buffer[2];
	file->buffer_list[3].size = string_length;
	file->buffer_list[3].buffer = string;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_message_record
 * Desc : This function writes a message record.
 *--------------------------------------------------------------------------------*/
static void	write_message_record(	unsigned char	type,
								OUTPUT_FILE*	file,
								NODE*			current_node,
								NODE*			other_node,
								TIMELINE*		other_timeline,
								unsigned char*	message_name,
								unsigned int	message_length)
{
	unsigned char	buffer[5];
	unsigned short	other_id = 0;
	unsigned short	current_id = 0;
	unsigned short	timeline_length = sizeof("unknown")-1;
	unsigned char*	timeline_name = (unsigned char*) "Unknown";

	if (current_node != NULL)
	{
		current_id = current_node->level;
	}

	if (other_node != NULL)
	{
		other_id = other_node->level;
	}

	if (other_timeline != NULL)
	{
		timeline_name = other_timeline->name;
		timeline_length = other_timeline->name_length;
	}

	buffer[0] = type;
	buffer[1] = current_id & 0xff;
	buffer[2] = other_id & 0xff;

	buffer[3] = timeline_length & 0xff;
	
	buffer[4] = message_length & 0xff;

	file->parts = 4;
	file->record_size = 4 + timeline_length + 1 + message_length;
	file->buffer_list[0].size = 4;
	file->buffer_list[0].buffer = buffer;
	file->buffer_list[1].size = timeline_length;
	file->buffer_list[1].buffer = timeline_name;
	file->buffer_list[2].size = 1;
	file->buffer_list[2].buffer = &buffer[4];
	file->buffer_list[3].size = message_length;
	file->buffer_list[3].buffer = message_name;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_type_record
 * Desc : This function writes a api type record to the file.
 *--------------------------------------------------------------------------------*/
static void	write_type_record(unsigned char type, OUTPUT_FILE* file, NAME* type_type, NAME* name, NAME* brief)
{
	unsigned char	buffer[6];

	buffer[0] = type;
	buffer[1] = brief->fixed;
	buffer[2] = type_type->name_length & 0xff;
	buffer[3] = name->name_length & 0xff;
	buffer[4] = (brief->name_length & 0xff00) >> 8;
	buffer[5] = brief->name_length & 0xff;

	file->parts = 4;
	file->record_size = 6 + type_type->name_length + name->name_length + brief->name_length;
	file->buffer_list[0].size = 6;
	file->buffer_list[0].buffer = buffer;
	file->buffer_list[1].size = 	type_type->name_length;
	file->buffer_list[1].buffer =	type_type->name;
	file->buffer_list[2].size = 	name->name_length;
	file->buffer_list[2].buffer =	name->name;
	file->buffer_list[3].size =		brief->name_length;
	file->buffer_list[3].buffer =	brief->name;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_constant_record
 * Desc : This function writes a api constant record to the file.
 *--------------------------------------------------------------------------------*/
static void	write_constant_record(unsigned char type, OUTPUT_FILE* file, NAME* type_type, NAME* name, NAME* value, NAME* brief)
{
	unsigned char	buffer[8];

	buffer[0] = type;
	buffer[1] = brief->fixed;
	buffer[2] = type_type->name_length & 0xff;
	buffer[3] = name->name_length & 0xff;
	buffer[4] = (value->name_length & 0xff00) >> 8;
	buffer[5] = value->name_length & 0xff;
	buffer[6] = (brief->name_length & 0xff00) >> 8;
	buffer[7] = brief->name_length & 0xff;

	file->parts = 5;
	file->record_size = 8 + type_type->name_length + name->name_length + brief->name_length + value->name_length;
	file->buffer_list[0].size = 8;
	file->buffer_list[0].buffer	= buffer;
	file->buffer_list[1].size	= type_type->name_length;
	file->buffer_list[1].buffer = type_type->name;
	file->buffer_list[2].size	= name->name_length;
	file->buffer_list[2].buffer = name->name;
	file->buffer_list[3].size	= value->name_length;
	file->buffer_list[3].buffer = value->name;
	file->buffer_list[4].size	= brief->name_length;
	file->buffer_list[4].buffer = brief->name;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_pair_record
 * Desc : This function writes a api pair record to the file.
 *--------------------------------------------------------------------------------*/
static void	write_pair_record(unsigned char type, OUTPUT_FILE* file, NAME* value, NAME* string)
{
	unsigned char	buffer[5];

	buffer[0] = type;
	buffer[1] = string->fixed;
	buffer[2] = value->name_length & 0xff;
	buffer[3] = (string->name_length & 0xff00) >> 8;
	buffer[4] = string->name_length & 0xff;

	file->parts = 3;
	file->record_size = 5 + value->name_length + string->name_length;
	file->buffer_list[0].size = 5;
	file->buffer_list[0].buffer = buffer;
	file->buffer_list[1].size = 	value->name_length;
	file->buffer_list[1].buffer =	value->name;
	file->buffer_list[2].size = 	string->name_length;
	file->buffer_list[2].buffer =	string->name;

	write_to_file(file);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_api
 * Desc : This function will output the api to the open file.
 *--------------------------------------------------------------------------------*/
static void	output_api(OUTPUT_FILE* outfile, API* api)
{
	NAME				empty = {NULL,0,0,0};
	NAME				group_name;
	API_TYPE*			current_type;
	API_RETURNS*		current_returns;
	API_FUNCTION*		current_function;
	API_CONSTANT*		current_constant;
	API_CONSTANTS*		current_constants;
	API_PARAMETER*		current_parameter;
	API_TYPE_RECORD*	current_type_record;

	/* let the file know that we are starting a new api */
	group_name.name			= api->group->name;
	group_name.fixed 		= 0;
	group_name.name_length	= api->group->name_length;
	write_string_record(LINKER_API_START,outfile,&group_name);
	
	/* write all the functions to the file */
	current_function = api->function_list;

	while(current_function != NULL)
	{
		write_type_record(LINKER_API_FUNCTION,outfile,&current_function->return_type,&current_function->name,&empty);
		write_string_record(LINKER_API_DESCRIPTION,outfile,&current_function->description);
		write_string_record(LINKER_API_ACTION,outfile,&current_function->action);
			
		/* dump parameter */
		current_parameter = current_function->parameter_list;

		while(current_parameter != NULL)
		{
			write_type_record(LINKER_API_PARAMETER,outfile,&current_parameter->type,&current_parameter->name,&current_parameter->brief);

			current_parameter = current_parameter->next;
		}

		/* dump returns */
		current_returns = current_function->returns_list;

		while(current_returns != NULL)
		{
			write_pair_record(LINKER_API_RETURNS,outfile,&current_returns->value,&current_returns->brief);

			current_returns = current_returns->next;
		}
		
		write_empty_record(LINKER_API_FUNCTION_END,outfile);

		current_function = current_function->next;
	}

	/* now output the types for the API */
	current_type = api->type_list;

	while (current_type != NULL)
	{
		write_pair_record(LINKER_API_TYPE_START,outfile,&current_type->name,&current_type->description);
		
		current_type_record = current_type->record_list;

		while (current_type_record != NULL)
		{
			write_type_record(	LINKER_API_TYPE_FIELD,
								outfile,
								&current_type_record->type_item,
								&current_type_record->name_value,
								&current_type_record->brief);

			current_type_record = current_type_record->next;
		}

		write_empty_record(LINKER_API_TYPE_END,outfile);

		current_type = current_type->next;
	}

	/* now output the constantss for the API */
	current_constants = api->constants_list;

	while (current_constants != NULL)
	{
		write_pair_record(LINKER_API_CONSTANTS_START,outfile,&current_constants->name,&current_constants->description);
		
		current_constant = current_constants->constant_list;

		while (current_constant != NULL)
		{
			write_constant_record(	LINKER_API_CONSTANT,
									outfile,
									&current_constant->type,
									&current_constant->name,
									&current_constant->value,
									&current_constant->brief);

			current_constant = current_constant->next;
		}

		write_empty_record(LINKER_API_CONSTANTS_END,outfile);

		current_constants = current_constants->next;
	}

	/* Ok, we have finished outputting the API */
	write_empty_record(LINKER_API_END,outfile);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_state_machine
 * Desc : This function will output the state machine to the open file.
 *--------------------------------------------------------------------------------*/
static void	output_state_machine(OUTPUT_FILE* outfile,STATE_MACHINE* state_machine)
{
	unsigned int		node_id = 0;
	NAME				temp_name;
	STATE*				current_state = state_machine->state_list;
	TRIGGERS* 			current_triggers;
	STATE_TRANSITION*	current_trans;
	
	/* enumerate all the states first first */
	while (current_state != NULL)
	{
		current_state->tag_id = node_id++;

		current_state = current_state->next;
	}

	current_state = state_machine->state_list;
	
	temp_name.name			= state_machine->group->name;
	temp_name.fixed 		= 0;
	temp_name.name_length	= state_machine->group->name_length;

	write_string_record(LINKER_STATE_MACHINE_START,outfile,&temp_name);

	while (current_state != NULL)
	{
		current_trans = current_state->transition_list;

		if (current_state->name_length > 0)
		{
			write_group_id_record(LINKER_STATE,outfile,current_state->group,current_state->name,current_state->name_length,current_state->tag_id);

			while(current_trans != NULL)
			{
				write_numeric_record(LINKER_TRANSITON,outfile,current_trans->next_state->tag_id);

				if (current_trans->trigger != NULL)
				{
					write_group_record(	LINKER_TRIGGER,
										outfile,
										current_trans->trigger->group,
										current_trans->trigger->name,
										current_trans->trigger->name_length);
				}
				else if (current_trans->condition != NULL)
				{
					temp_name.name			= current_trans->condition;
					temp_name.name_length	= current_trans->condition_length;
					write_string_record(LINKER_CONDITION,outfile,&temp_name);
				}

				if (current_trans->triggers != NULL)
				{
					current_triggers = current_trans->triggers;

					do
					{
						write_group_record(	LINKER_TRIGGERS,
											outfile,
											current_triggers->trigger->group,
											current_triggers->trigger->name,
											current_triggers->trigger->name_length);

						current_triggers = current_triggers->next;
					} 
					while (current_triggers != NULL);
				}

				current_trans = current_trans->next;
			}

			/* end the state */
			write_empty_record(LINKER_END,outfile);
		}
		current_state = current_state->next;
	}

	write_empty_record(LINKER_STATE_MACHINE_END,outfile);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_sequence_diagram
 * Desc : This function will output the sequence diagram.
 *--------------------------------------------------------------------------------*/
static void	output_sequence_diagram(OUTPUT_FILE* outfile, SEQUENCE_DIAGRAM* sequence_diagram)
{
	NAME		group_name;
	NODE*		current_node;
	MESSAGE*	current_message;
	TIMELINE*	current_timeline;
	unsigned int	node_id = 1;
	unsigned int 	result = EC_OK;

	current_timeline = sequence_diagram->timeline_list;
	
	group_name.name			= sequence_diagram->group->name;
	group_name.fixed 		= 0;
	group_name.name_length	= sequence_diagram->group->name_length;
	write_string_record(LINKER_SEQUENCE_START,outfile,&group_name);

	/* enumerate all the nodes first */
	while (current_timeline != NULL)
	{
		current_node = current_timeline->node;

		while (current_node != NULL)
		{
			current_node->level = node_id++;
			current_node = current_node->next;
		}

		current_timeline = current_timeline->next;
	}

	/* now dump the nodes */
	current_timeline = sequence_diagram->timeline_list;

	while (current_timeline != NULL)
	{
		if ((current_timeline->flags & FLAG_BROADCAST) != FLAG_BROADCAST)
		{
			current_node = current_timeline->node;

			write_group_record(LINKER_TIMELINE,outfile,current_timeline->group,current_timeline->name,current_timeline->name_length);

			while (current_node != NULL)
			{
				/* start the node */
				write_numerics_record(LINKER_NODE_START,outfile,current_node->level,current_node->flags);

				/* check the messages that have been sent from the current node */
				current_message = current_node->sent_message;

				if (current_message != NULL)
				{
					write_message_record(	LINKER_SENT_MESSAGE,
											outfile,
											current_node,
											current_message->receiver,
											current_message->target_timeline,
											current_message->name,
											current_message->name_length);
				}

				if (current_node->received_message == NULL && current_node->wait_message.name_length > 0)
				{
					/* TODO: not sure if this is required to be output */
					printf("found a recives and waits -- need to fix w: %s (%p %p)\n",current_node->wait_message.name,(void*)current_node,(void*)current_node->sent_message);
				}

				write_empty_record(LINKER_NODE_END,outfile);

				current_node = current_node->next;
			}
		}

		current_timeline = current_timeline->next;
	}

	write_empty_record(LINKER_SEQUENCE_END,outfile);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_samples
 * Desc : This function will output all the samples collected to the output
 *        file.
 *--------------------------------------------------------------------------------*/
static void	output_samples(OUTPUT_FILE* outfile, SAMPLE* list)
{
	SAMPLE*	current_sample = list;

	while (current_sample != NULL)
	{
		write_pair_record(LINKER_SAMPLE,outfile,&current_sample->name,&current_sample->sample);
		current_sample = current_sample->next;
	}
}

#if (MAX_NUM_OPTIONS > 255)
#error Need to fix the following code to allow for greater the 255 - also will need to fix the processor
#endif

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_synopsis
 * @desc: This function will output the synopsis. As part of this it will try
 *        and validate the options specified in the synopsis list. it will list
 *        the synopsis in the numeric order that they appear in the applications
 *        list.
 *--------------------------------------------------------------------------------*/
static void	output_synopsis(OUTPUT_FILE* outfile, SYNOPSIS_LIST* synopsys, OPTION* option_list)
{
	unsigned int	end;
	unsigned int	start;
	unsigned int	item;
	unsigned int	index = 0;
	unsigned int	position = 0;
	unsigned int	name_length;
	unsigned char	synopsis_index[MAX_NUM_OPTIONS];
	OPTION*			option;

	while(position < synopsys->items.name_length && index < MAX_NUM_OPTIONS)
	{
		start = position;

		/* remove white space */
		while ((start < synopsys->items.name_length) && (synopsys->items.name[start] < 0x0f || synopsys->items.name[start] == 0x20))
		{
			start++;
		}

		/* find the string */
		while (synopsys->items.name[position] != ',' && (position < synopsys->items.name_length))
		{
			position++;
		}

		/* trim non-valid chars */
		end = position;
		while ((end-1 > start) && (synopsys->items.name[end-1] < 0x0f || synopsys->items.name[end-1] == 0x20))
		{
			end--;
		}

		/* now find the option in the option list */
		option = option_list;

		name_length = end - start;

		while (option != NULL)
		{
			if (name_length == option->name.name_length && memcmp(&synopsys->items.name[start],option->name.name,name_length) == 0)
			{
				/* Ok, found it */
				synopsis_index[index++] = option->option_id;
				break;
			}

			option = option->next;
		}

		if (option == NULL)
		{
			unsigned char	hold = synopsys->items.name[end];

			synopsys->items.name[end] = '\0';
			raise_warning(0,EC_OPTION_REQUIRED_BY_SYNOPSIS_MISSING,&synopsys->items.name[start],NULL);
			synopsys->items.name[end] = hold;
		}

		position++;
	}

	/* output the synopsis to the file */
	write_synopsis_record(LINKER_APPLICATION_SYNOPSIS,outfile,&synopsys->name,synopsis_index,index);

	if (index == MAX_NUM_OPTIONS)
	{
		raise_warning(0,EC_MAX_NUMBER_OF_OPTIONS_IN_SYNOPSIS,NULL,NULL);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_applications
 * Desc : This function will output all the applications collected to the output
 *        file.
 *--------------------------------------------------------------------------------*/
static void	output_applications(OUTPUT_FILE* outfile, APPLICATION* list)
{
	OPTION*			option;
	SECTION*		section;
	COMMAND*		command;
	SECTION*		sub_section;
	APPLICATION*	application = list;
	SYNOPSIS_LIST*	synopsis;

	while (application != NULL)
	{
		if (application->name.name_length > 0)
		{
			write_string_record(LINKER_APPLICATION_START,outfile,&application->name);
	
			/* now output the options */
			option = application->option_list;

			while (option != NULL)
			{
				write_option_record(LINKER_APPLICATION_OPTION,outfile,option);

				option = option->next;
			}

			/* now write the synposis */
			synopsis = &application->synopsis_list;

			while (synopsis != NULL)
			{
				if (synopsis->name.name_length > 0)
				{
					/* we have a synopsis and now need to process the list */
					output_synopsis(outfile,synopsis,application->option_list);
				}

				synopsis = synopsis->next;
			}

			/* now output the commands */
			command = application->command_list;

			while (command != NULL)
			{
				write_type_record(LINKER_APPLICATION_COMMAND,outfile,&command->name,&command->parameters,&command->description);

				command = command->next;
			}

			/* now write the sections */
			section = application->section_list;
			
			while (section != NULL)
			{
				/* now write the sections to the file */
				write_pair_record(LINKER_APPLICATION_SECTION,outfile,&section->name,&section->section_data);
				
				sub_section = section->sub_section_list;

				while (sub_section != NULL)
				{
					write_pair_record(LINKER_APPLICATION_SUB_SECTION,outfile,&sub_section->name,&sub_section->section_data);
				
					sub_section = sub_section->next;
				}

				section = section->next;
			}

			write_empty_record(LINKER_APPLICATION_END,outfile);
		
		}
		application = application->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : produce_output
 * Desc : This function will output the resulting linked object file.
 *--------------------------------------------------------------------------------*/
static unsigned int	produce_output(char* output_name)
{
	time_t			now = time(NULL);
	struct tm*		curr_time = gmtime(&now);
	unsigned int	in_size;
	unsigned int	result = EC_OK;
	unsigned char	signature[4] = LINKED_SOURCE_MAGIC;
	unsigned char	file_header[FILE_HEADER_SIZE];
	GROUP*			current = &g_group_tree;
	OUTPUT_FILE		outfile;

	if (open_file(&outfile,output_name))
	{
		/* ok, we have created the output file */
		file_header[0] = signature[0];
		file_header[1] = signature[1];
		file_header[2] = signature[2];
		file_header[3] = signature[3];

		file_header[FILE_VERSION_MAJOR] = VERSION_MAJOR;
		file_header[FILE_VERSION_MINOR] = VERSION_MINOR;

		file_header[FILE_DAY_OFF    ] = (unsigned char) curr_time->tm_mday;
		file_header[FILE_MONTH_OFF  ] = (unsigned char) curr_time->tm_mon;
		file_header[FILE_YEAR_OFF   ] = (unsigned char) curr_time->tm_year;
		file_header[FILE_HOUR_OFF   ] = (unsigned char) curr_time->tm_hour;
		file_header[FILE_MINUTE_OFF ] = (unsigned char) curr_time->tm_min;
		file_header[FILE_SECONDS_OFF] = (unsigned char) curr_time->tm_sec;

		/* more than one imput file name so set this to zero */
		file_header[FILE_NAME_LENGTH  ] = 0;
		file_header[FILE_NAME_LENGTH+1] = 0;

		write(outfile.outfile,file_header,FILE_HEADER_SIZE);

		/* first dump all the samples */
		output_samples(&outfile,g_sample_list.next);

		/* second dump the applications to the file */
		output_applications(&outfile,g_application_list.next);

		/* ok, we have an open file */
		while (current != NULL)
		{
			if (current->state_machine != NULL)
			{
				output_state_machine(&outfile,current->state_machine);
			}

			if (current->sequence_diagram != NULL)
			{
				output_sequence_diagram(&outfile,current->sequence_diagram);
			}

			if (current->api != NULL)
			{
				output_api(&outfile,current->api);
			}

			current = current->next;
		}

		close_file(&outfile);
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
	char*			output_name = "doc.gout";
	char*			infile_name = NULL;
	char*			error_param = "";
	char*			error_string = "";
	unsigned int	start = 1;
	unsigned int	output_length = sizeof("doc.gout")-1;
	unsigned char*	param_mask;

	param_mask = calloc(argc,1);

	/* initialise some things */
	memset(&g_group_tree,0,sizeof(GROUP));
	memset(&g_sample_list,0,sizeof(SAMPLE));
	memset(&g_function_list,0,sizeof(FUNCTION));
	memset(&g_application_list,0,sizeof(APPLICATION));

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
					case 'v':	/* verbose - add extra comments to the output */
						verbose = 1;
						break;

					/**-----------------------------------------------------------*
					 * @option	-q
					 *
					 * @description
					 * quiet. This flag suppresses any non-error output from the
					 * application.
					 *------------------------------------------------------------*/
					case 'q':	/* quiet - suppress non error outputs */
						quiet = 1;
						break;

					/**-----------------------------------------------------------*
					 * @option		-o
					 * @value		<output file name>
					 * @required	no
					 *
					 * @description
					 *
					 * This function defines the name of the output file produced
					 * by this application. If the name is not given then it will
					 * default to "doc.gout".
					 *------------------------------------------------------------*/
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

					/**-----------------------------------------------------------*
					 * @option		-?
					 *
					 * @description
					 * This outputs the help message.
					 *------------------------------------------------------------*/
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
		/**-----------------------------------------------------------*
		 * @option 		input_files
		 * @required	yes
		 * @multiple	yes
		 *
		 * @description
		 *
		 * The object file(s) to link together.
		 *------------------------------------------------------------*/
		g_input_filename = NULL;

		for (start = 1; start < argc; start++)
		{
			if (param_mask[start] == 0)
			{
				/* ok, this should be an input file */
				failed |= process_input(argv[start]);
			}
		}

		/* change the filename that the error reports are going to be reported against */
		g_input_filename = (unsigned char*) output_name;

		if (!failed)
		{
			if ((failed = semantic_check()) == EC_OK)
			{
				failed = produce_output(output_name);
			}
		}
	}

	if (failed)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}

