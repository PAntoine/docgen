/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : document_linker
 * Desc  : This file holds the code the handles linking the source files that
 *         have been compiled with the document source compiler.
 *
 * Author: pantoine
 * Date  : 15/11/2011 08:59:42
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "error_codes.h"
#include "supported_formats.h"
#include "document_generator.h"

static unsigned char	record_buffer[MAX_RECORD_SIZE];

extern unsigned char*	g_input_filename;
extern unsigned char*	g_source_filename;
extern unsigned char	g_file_header[];
extern unsigned int		g_source_filename_length;

unsigned char	finish_state[] = "finish";
unsigned int	finish_state_length = sizeof(finish_state);

extern OUTPUT_FORMATS	output_formats[];

/*--------------------------------------------------------------------------------*
 * The linking options
 *--------------------------------------------------------------------------------*/
static unsigned int	g_max_call_depth	= 10;

/*--------------------------------------------------------------------------------*
 * Linking Structures
 *--------------------------------------------------------------------------------*/
static GROUP 	g_group_tree = {{0x00},0,0,NULL,NULL,NULL,NULL};
static FUNCTION	g_function_list = {0,0,{0x00},NULL,NULL};

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : debug_dump_node
 * Desc : thid function will dump the node.
 *--------------------------------------------------------------------------------*/
void	debug_dump_node(NODE* current_node)
{
	if (current_node == NULL)
		return;

		printf("flags: %c%c%c%c%c ",
				(current_node->flags & FLAG_SEQUENCE_RESPONDS)?'r':' ',
				(current_node->flags & FLAG_FUNCTION)?'f':' ',
				(current_node->flags & FLAG_IN_FUNCTION)?'i':' ',
				(current_node->flags & FLAG_FUNCTION_END)?'e':' ',
				(current_node->flags & FLAG_BROADCAST)?'b':' ',
				(current_node->flags & FLAG_WAIT)?'w':' ');

		printf("cn: %p ",current_node);

		if (current_node->timeline != NULL)
			printf("t: %s ",current_node->timeline->name);

		if (current_node->sent_message != NULL)
			printf("s: %s ",current_node->sent_message->name);
		
		if (current_node->received_message != NULL)
			printf("r: %s ",current_node->received_message->name);

		if (current_node->sent_message != NULL && current_node->sent_message->receiver != NULL)
			printf(" (%s) ",current_node->sent_message->receiver->timeline->name);

		if (current_node->wait_message.name_length > 0)
			printf(" w: %s ",current_node->wait_message.name);

		printf("next: %p ",current_node->next);

		printf("\n");
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : next_active_node
 * Desc : This function handles the active part of the search.
 *--------------------------------------------------------------------------------*/
NODE*	next_active_node(NODE* current_node)
{
	NODE* result = NULL;

	if (current_node->sent_message != NULL)
	{
		result = current_node->sent_message->receiver;

		/* if there is a target and it is not a message to self */
		if (result != NULL && result->timeline != current_node->timeline)
		{
			result->return_node = current_node;
		}
		else
		{
			result = current_node->next;
		}
	}
	else
	{
		/* ok, goto to the next item in the tree */
		if (current_node->flags & (FLAG_FUNCTION | FLAG_IN_FUNCTION))
		{
			result = current_node->next;
		}
		else if ((current_node->flags & FLAG_FUNCTION_END))
		{
			/* check to see if the function was called */
			if (current_node->return_node->return_node != NULL)
			{
				result = current_node->return_node->return_node->next;
			}
			else
			{
				result = current_node->next;
			}
		}
		else if (current_node->return_node)
		{
			/* only if the function was called */
			result = current_node->return_node->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : search_next_node
 * Desc : This function will search for an active node. This is a node that should
 *        be actioned. Which will be a node that does a send without a receive.
 *--------------------------------------------------------------------------------*/
NODE*	search_next_node(NODE* current_node, NODE** active_node)
{
	NODE* result = NULL;
	
	*active_node = NULL;

	while (current_node != NULL)
	{
		if (current_node->wait_message.name_length > 0)
		{
			if (current_node->flags & FLAG_FUNCTION)
			{
				current_node = current_node->function_end;
			}
			else
			{
				current_node = current_node->next;
			}
		}
		else if (current_node->sent_message != NULL)
		{
			*active_node = current_node;
			
			result = current_node->next;
			break;
		}
		else
		{
			current_node = current_node->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : test_walk_node_tree
 * Desc : This function will walk the tree as a test.
 *--------------------------------------------------------------------------------*/
void	test_walk_node_tree(NODE* start)
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
 * Name : add_group
 * Desc : This function will add the named group to the list. It should only be
 *        called after the item has been searched for, otherwise duplicates will
 *        be added to the list as this function does not check for the name
 *        previously existing.
 *--------------------------------------------------------------------------------*/
GROUP*	add_group ( unsigned char* name, unsigned int name_length )
{
	GROUP* result = NULL;
	GROUP* current = &g_group_tree;

	while (current != NULL)
	{
		if (current->next == NULL)
		{
			current->next = calloc(1,sizeof(GROUP));
			memcpy(current->next->name,name,name_length);
			current->next->name_length = name_length;
			result = current->next;
			break;
		}

		current = current->next;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_group
 * Desc : This function will lookup the GROUP in the group list. If the group list
 *        has the named group it will return the first one it finds. Else, it will
 *        return NULL.
 *--------------------------------------------------------------------------------*/
GROUP* find_group ( unsigned char* name, unsigned int name_length )
{
	GROUP* result = NULL;
	GROUP* current = &g_group_tree;

	while (current != NULL)
	{
		if (current->name_length == name_length && memcmp(current->name,name,name_length) == 0)
		{
			result = current;
			break;
		}
		current = current->next;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_state
 * Desc : This function will find the state in the state tree for the given
 *        group.
 *--------------------------------------------------------------------------------*/
STATE* find_state ( GROUP* group, unsigned char* name, unsigned int name_length )
{
	STATE*	result = NULL;
	STATE*	current_state = NULL;

	if (group != NULL)
	{
		if (group->state_machine != NULL && group->state_machine->state_list != NULL)
		{
			current_state = group->state_machine->state_list;

			do
			{
				if (current_state->name_length == name_length && memcmp(current_state->name,name,name_length) == 0)
				{
					result = current_state;
					break;
				}

				current_state = current_state->next;
			}
			while (current_state != NULL);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_state
 * Desc : This function will add the state to the state list. It will not check to
 *        see if the state already exists, so this function MUST be called after
 *        the find has been called or duplicate states will occur and the results
 *        will be unpredictable.
 *--------------------------------------------------------------------------------*/
STATE* add_state ( GROUP* group, unsigned char* name, unsigned int name_length, unsigned int line_number, unsigned int flags )
{
	STATE*	result = NULL;
	STATE*	current_state = NULL;

	if (group != NULL)
	{
		if (group->state_machine == NULL)
		{
			/* ok, first state */
			group->state_machine = calloc(1,sizeof(STATE_MACHINE));
			group->state_machine->state_list = calloc(1,sizeof(STATE));

			current_state = group->state_machine->state_list;
			group->state_machine->group = group;
		}
		else
		{
			current_state = group->state_machine->state_list;

			while (current_state != NULL)
			{
				if (current_state->next == NULL)
				{
					current_state->next = calloc(1,sizeof(STATE));
					current_state = current_state->next; 
					break;
				}

				current_state = current_state->next;
			}
		}

		/* Ok, current state is the new state - now fill it in */
		current_state->name = malloc(name_length);
		current_state->flags = flags;
		current_state->name_length = name_length;
		current_state->group = group;
		current_state->line_number = line_number;

		memcpy(current_state->name,name,name_length);
		
		result = current_state;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_timeline
 * Desc : This function will find the timeline in the timeline tree for the given
 *        group.
 *--------------------------------------------------------------------------------*/
TIMELINE* find_timeline ( GROUP* group, unsigned char* name, unsigned int name_length )
{
	TIMELINE*	result = NULL;
	TIMELINE*	current_timeline = NULL;

	if (group != NULL)
	{
		if (group->sequence_diagram != NULL && group->sequence_diagram->timeline_list != NULL)
		{
			current_timeline = group->sequence_diagram->timeline_list;

			do
			{
				if (current_timeline->name_length == name_length && memcmp(current_timeline->name,name,name_length) == 0)
				{
					result = current_timeline;
					break;
				}

				current_timeline = current_timeline->next;
			}
			while (current_timeline != NULL);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_timeline
 * Desc : This function will add the timeline to the timeline list. It will not check to
 *        see if the timeline already exists, so this function MUST be called after
 *        the find has been called or duplicate timelines will occur and the results
 *        will be unpredictable.
 *--------------------------------------------------------------------------------*/
TIMELINE* add_timeline ( GROUP* group, unsigned char* name, unsigned int name_length, unsigned int line_number, unsigned int flags )
{
	TIMELINE*	result = NULL;
	TIMELINE*	current_timeline = NULL;

	if (group != NULL)
	{
		if (group->sequence_diagram == NULL)
		{
			/* ok, first timeline */
			group->sequence_diagram = calloc(1,sizeof(SEQUENCE_DIAGRAM));
			group->sequence_diagram->timeline_list = calloc(1,sizeof(TIMELINE));

			current_timeline = group->sequence_diagram->timeline_list;
			group->sequence_diagram->group = group;
		}
		else
		{
			current_timeline = group->sequence_diagram->timeline_list;

			while (current_timeline != NULL)
			{
				if (current_timeline->next == NULL)
				{
					current_timeline->next = calloc(1,sizeof(TIMELINE));
					current_timeline = current_timeline->next; 
					break;
				}

				current_timeline = current_timeline->next;
			}
		}

		/* Ok, current timeline is the new timeline - now fill it in */
		current_timeline->name = malloc(name_length);
		current_timeline->flags = flags;
		current_timeline->name_length = name_length;
		current_timeline->group = group;
		current_timeline->line_number = line_number;

		memcpy(current_timeline->name,name,name_length);
		
		result = current_timeline;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_trigger
 * Desc : This function will find the trigger in the given groups trigger list.
 *--------------------------------------------------------------------------------*/
TRIGGER* find_trigger ( GROUP* group, unsigned char* name, unsigned int name_length )
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
TRIGGER* add_trigger ( GROUP* group, unsigned char* name, unsigned int name_length, unsigned int flags )
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
void	add_triggers(TRIGGERS** triggers, BLOCK_NAME* triggers_list, GROUP* group)
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
MESSAGE*	create_message(NODE* sender, TIMELINE* destination, BLOCK_NAME* name)
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
 * Name : find_function
 * Desc : This function will find a named function in the list of functions.
 *--------------------------------------------------------------------------------*/
FUNCTION*	find_function(unsigned char* name, unsigned int name_length)
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
 * Name : handle_trigger
 * Desc : This function will handle the adding of the trigger to the node.
 *--------------------------------------------------------------------------------*/
TRIGGER*	handle_trigger(BLOCK_NAME* trigger, GROUP* group)
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
FUNCTION*	add_function(unsigned char* name, unsigned int name_length, unsigned int flags)
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
 * Name : add_block_node
 * Desc : This function will add a sequence diagram block to the diagram tree.
 *--------------------------------------------------------------------------------*/
unsigned int	add_block_node(BLOCK_NODE* block, GROUP** local_group_list)
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
 * Name : add_block_function
 * Desc : This function will add a function block.
 *--------------------------------------------------------------------------------*/
unsigned int	add_block_function(BLOCK_NODE* block, FUNCTION** local_function_list)
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
 * Name : add_block_state
 * Desc : This function will add a state block to the diagram tree.
 *--------------------------------------------------------------------------------*/
unsigned int	add_block_state(BLOCK_NODE* block, GROUP** local_group_list)
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
unsigned int add_block(BLOCK_NODE* block, GROUP** local_group_list, FUNCTION** local_function_list )
{
	STATE*				to_state = NULL;
	GROUP*				owing_group = NULL;
	TRIGGER*			trigger = NULL;
	STATE_TRANSITION*	current_trans  = NULL;
	STATE_TRANSITION*	new_transition = NULL;
	unsigned int		result = EC_OK;

	/* validate the node first */
	if (block->state != NULL)
	{
		result = add_block_state(block,local_group_list);

	}
	else if (block->timeline != NULL)
	{
		result = add_block_node(block,local_group_list);
	}
	else if ((block->flags & FLAG_IN_FUNCTION) == FLAG_IN_FUNCTION)
	{
		result = add_block_function(block,local_function_list);
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
 * Name : add_atom_to_block
 * Desc : This function will add the atom to the block. 
 *--------------------------------------------------------------------------------*/
unsigned int	add_atom_to_block ( GROUP** local_group_list, FUNCTION** local_function_list, BLOCK_NODE* node, unsigned char* record, unsigned char* payload, unsigned int payload_length )
{
	unsigned int	in_function = 0;
	unsigned int	result = EC_OK;
	unsigned int	line_number = 0;
	unsigned int	group_id = ((record[RECORD_GROUP]<< 8) | record[RECORD_GROUP+1]);
	TRIGGER*		trigger;
	FUNCTION*		function;

	if (group_id != DEFAULT_GROUP && (group_id & RECORD_FUNCTION_MASK) == RECORD_FUNCTION_MASK)
	{
		/* the atom is within a function */
		node->flags |= FLAG_IN_FUNCTION;
		group_id &= ~RECORD_FUNCTION_MASK;
	
		node->function = local_function_list[group_id+1];

		in_function = 1;
	}

	switch(record[RECORD_ATOM])
	{
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
			else if ((node->state = find_state(local_group_list[group_id],payload,payload_length)) == NULL)
			{
				node->state = add_state(local_group_list[group_id],payload,payload_length,node->line_number,0);
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
					memcpy(node->transition.name,payload,payload_length);
					node->transition.owner.group = local_group_list[group_id];
					node->transition.name_length = payload_length;
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
				memcpy(node->condition.name,payload,payload_length);
				node->condition.owner.group = local_group_list[group_id];
				node->condition.name_length = payload_length;
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
				node->trigger->name_length = payload_length;
				memcpy(node->trigger->name,payload,payload_length);
			}
			break;

		case ATOM_TRIGGERS:
			{
				BLOCK_NAME* temp = node->triggers_list;
				node->triggers_list = calloc(1,sizeof(BLOCK_NAME));
				node->triggers_list->owner.group = local_group_list[group_id];
				node->triggers_list->name_length = payload_length;
				memcpy(node->triggers_list->name,payload,payload_length);
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
			else if ((node->timeline = find_timeline(local_group_list[group_id],payload,payload_length)) == NULL)
			{
				node->timeline = add_timeline(local_group_list[group_id],payload,payload_length,node->line_number,0);
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
					memcpy(node->message.name,payload,payload_length);
					node->message.name_length = payload_length;
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
					memcpy(node->after.name,payload,payload_length);
					node->after.name_length = payload_length;
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
					memcpy(node->sequence.name,payload,payload_length);
					node->sequence.name_length = payload_length;
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
				memcpy(node->function_to_timeline.name,payload,payload_length);
				node->function_to_timeline.name_length = payload_length;
			}
			else if ((node->to_timeline = find_timeline(local_group_list[group_id],payload,payload_length)) == NULL)
			{
				node->to_timeline = add_timeline(local_group_list[group_id],payload,payload_length,node->line_number,0);
			}
			break;

		case ATOM_PARAMETER:
			/* add parameter to parameter list for block node */
			break;

			/* handle atoms that take a name */
		case ATOM_ACTIVATION:
			break;

			/* string atoms */
		case ATOM_REPEATS:
			break;

		default:
			line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
			raise_warning(line_number,EC_UNKNOWN_ATOM,NULL,NULL);
			result = EC_UNKNOWN_ATOM;
			break;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : process_input
 * Desc : This function will read an input file and add the atoms to the set if
 *        atoms. It will also handle the first parse on the building of the 
 *        connections.
 *--------------------------------------------------------------------------------*/
unsigned int	process_input(const char* filename)
{
	int				infile;
	int				bytes_read;
	unsigned int 	result = 0;
	unsigned int	num_groups = 1;
	unsigned int	num_functions = 1;
	unsigned int	record_size;
	unsigned int	file_name_size;
	unsigned int	new_block_number;
	unsigned char	record[RECORD_DATA_START];
	unsigned char	header[INTERMEDIATE_HEADER_SIZE];
	BLOCK_NODE		block_node;
	
	static GROUP*		local_group[MAX_GROUPS_PER_FILE];
	static FUNCTION*	local_functions[MAX_GROUPS_PER_FILE];
	static unsigned char	source_file_name[FILENAME_MAX+1];

	memset(&block_node,0,sizeof(block_node));
	block_node.line_number = 0;

	local_group[0] = &g_group_tree;
	local_group[DEFAULT_GROUP] = NULL;

	g_input_filename = NULL;
	g_source_filename = source_file_name;
	g_source_filename[0] = '\0';

	if ((infile = open(filename,READ_FILE_STATUS,0)) != -1)
	{
		g_input_filename = (unsigned char*) filename;
		
		/* reset the local list */
		num_groups = 1;
		num_functions = 1;

		if (read(infile,header,INTERMEDIATE_HEADER_SIZE) == INTERMEDIATE_HEADER_SIZE)
		{
			if(memcmp(header,g_file_header,4) == 0)
			{
				/* we have a object file -- we are ignoring all other files at this pass */
				if (header[INTERMEDIATE_VERSION_MAJOR_OFF] > VERSION_MAJOR)
				{
					raise_warning(0,EC_INPUT_FILE_BUILT_WITH_LATER_MAJOR_VERSION,NULL,NULL);
				}

				/* read the input file name */
				file_name_size = ((((unsigned int)header[INTERMEDIATE_NAME_START_OFF]) << 8) | header[INTERMEDIATE_NAME_START_OFF+1]);

				if (file_name_size < FILENAME_MAX)
				{
					bytes_read = read(infile,source_file_name,file_name_size);
					g_source_filename_length = file_name_size;
				}
				else
				{
					while(file_name_size > FILENAME_MAX)
					{
						read(infile,source_file_name,FILENAME_MAX);
						file_name_size -= FILENAME_MAX;
					}

					bytes_read = read(infile,source_file_name,FILENAME_MAX);
				}

				source_file_name[bytes_read] = '\0';

				/* now read the atoms */
				while ((bytes_read = read(infile,record,RECORD_DATA_START)) == RECORD_DATA_START)
				{
					record_size = ((((unsigned int)record[RECORD_DATA_SIZE]) << 8) | record[RECORD_DATA_SIZE+1]);
					new_block_number = ((((unsigned int)record[RECORD_BLOCK_NUM]) << 8) | record[RECORD_BLOCK_NUM+1]);

					if (new_block_number != block_node.block_number)
					{
						if (block_node.line_number != 0)
						{
							add_block(&block_node,local_group,local_functions);
						}

						memset(&block_node,0,sizeof(block_node));
						block_node.line_number = ((((unsigned int)record[RECORD_LINE_NUM]) << 8) | record[RECORD_LINE_NUM+1]);
						block_node.block_number = new_block_number;
					}

					if (record_size > 0)
					{
						if (read(infile,record_buffer,record_size) != record_size)
						{
							/* failed to read the record --- problem with the file */
							raise_warning(0,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)filename,NULL);
						}
						else
						{
							switch(record[RECORD_TYPE])
							{
								case INTERMEDIATE_RECORD_FUNCTION:
									if ((local_functions[num_functions] = find_function(record_buffer,record_size)) == NULL)
									{
										local_functions[num_functions] = add_function(record_buffer,record_size,0);
									}
									num_functions++;
									break;

								case INTERMEDIATE_RECORD_GROUP:
									if ((local_group[num_groups] = find_group(record_buffer,record_size)) == NULL)
									{
										local_group[num_groups] = add_group(record_buffer,record_size);
									}

									num_groups++;
									break;

								case INTERMEDIATE_RECORD_NAME:
								case INTERMEDIATE_RECORD_STRING:
									add_atom_to_block(local_group,local_functions,&block_node,record,record_buffer,record_size);
									break;

								default:
									raise_warning(0,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)filename,NULL);
							}
						}
					}
				}

				if (block_node.line_number != 0)
				{
					add_block(&block_node,local_group,local_functions);
				}
			}
		}

		close(infile);
	}

	g_source_filename = NULL;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_state_machine
 * Desc : This function will produce a dot file for the given state machine.
 *--------------------------------------------------------------------------------*/
void	generate_state_machine(DRAW_STATE* draw_state, STATE_MACHINE* state_machine)
{
	STATE*				current_state;

	output_formats[draw_state->format].output_header(draw_state,state_machine->group->name,state_machine->group->name_length);
	output_formats[draw_state->format].output_states(draw_state,state_machine->state_list);
	
	current_state = state_machine->state_list;

	while (current_state != NULL)
	{
		output_formats[draw_state->format].output_state(draw_state,current_state);

		current_state = current_state->next;
	}

	output_formats[draw_state->format].output_footer(draw_state);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_columns
 * Desc : This function will count the number of columns in the sequence diagram
 *        and will calculate the text column offset for each of the timelines.
 *--------------------------------------------------------------------------------*/
void	generate_columns(SEQUENCE_DIAGRAM* sequence_diagram, DRAW_STATE* draw_state)
{
	unsigned short	pos = 0;
	unsigned short	half_length = 0;
	TIMELINE*		current_timeline = sequence_diagram->timeline_list;

	unsigned char	debug[2048];

	draw_state->data.sequence.num_columns = 0;

	while (current_timeline != NULL)
	{
		if (memcmp("broadcast",current_timeline->name,sizeof("broadcast")-1) != 0)
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
unsigned int	generate_sequence_diagram( DRAW_STATE* draw_state, SEQUENCE_DIAGRAM* sequence_diagram)
{
	NODE	walk_start;
	NODE*	search_node;
	NODE*	active_node;
	TIMELINE* current_timeline = sequence_diagram->timeline_list;

	memset(&walk_start,0,sizeof(NODE));

	/* first generate the column offsets for the sequence diagram */
	generate_columns(sequence_diagram,draw_state);
	
	output_formats[draw_state->format].output_header(draw_state,sequence_diagram->group->name,sequence_diagram->group->name_length);
	output_formats[draw_state->format].output_timelines(draw_state,sequence_diagram->timeline_list);

	while (current_timeline != NULL)
	{
		if (current_timeline->node != NULL)
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
							output_formats[draw_state->format].output_message(draw_state,active_node->sent_message);
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
	
	output_formats[draw_state->format].output_footer(draw_state);
}

/* TODO: remove the debug --- */
#define NAME_WIDTH	(22)

unsigned int	dump_sequence(SEQUENCE_DIAGRAM* sequence_diagram)
{
	NODE*		current_node;
	MESSAGE*	current_message;
	TIMELINE*	current_timeline;
	unsigned int result = EC_OK;
	unsigned char	to_name[NAME_WIDTH+1];
	unsigned char	message_name[NAME_WIDTH+1];
	unsigned char	from_name[NAME_WIDTH+1];
	
	to_name[NAME_WIDTH] = '\0';
	from_name[NAME_WIDTH] = '\0';
	message_name[NAME_WIDTH] = '\0';

	printf("-----------------------------------------------------------------------------\n");
	printf("----------------------------- DUMP SEQUENCE ---------------------------------\n");
	printf("-----------------------------------------------------------------------------\n");

	if (sequence_diagram != NULL)
	{
		current_timeline = sequence_diagram->timeline_list;

		while (current_timeline != NULL)
		{
			current_node = current_timeline->node;

			printf("timeline: %s\n",current_timeline->name);

			while (current_node != NULL)
			{
				/* check the messages that have been sent from the current node */
				current_message = current_node->sent_message;

				memset(from_name,' ',NAME_WIDTH);
				memset(to_name,' ',NAME_WIDTH);
				memset(message_name,' ',NAME_WIDTH);

				if (current_message != NULL)
				{
					if (current_message->sending_timeline != NULL)
					{
						memcpy(from_name,current_message->sending_timeline->name,current_message->sending_timeline->name_length);
					}

					memcpy(message_name,current_message->name,current_message->name_length);

					if (current_message->target_timeline != NULL)
					{
						memcpy(to_name,current_message->target_timeline->name,current_message->target_timeline->name_length);
					}

					to_name[NAME_WIDTH] = '\0';
					from_name[NAME_WIDTH] = '\0';
					message_name[NAME_WIDTH] = '\0';
					printf("s: %s ---> %p %s%p ---> %s (%p)\n",from_name,current_message->sender,message_name,current_message->receiver,to_name,current_message);
				}

				/* check for the messages that have been received. */
				current_message = current_node->received_message;

				memset(from_name,' ',NAME_WIDTH);
				memset(to_name,' ',NAME_WIDTH);
				memset(message_name,' ',NAME_WIDTH);

				if (current_message != NULL)
				{
					if (current_message->sending_timeline != NULL)
					{
						memcpy(from_name,current_message->sending_timeline->name,current_message->sending_timeline->name_length);
					}

					memcpy(message_name,current_message->name,current_message->name_length);

					if (current_message->receiver != NULL)
					{
						memcpy(to_name,current_message->target_timeline->name,current_message->target_timeline->name_length);
					}
					to_name[NAME_WIDTH] = '\0';
					from_name[NAME_WIDTH] = '\0';
					message_name[NAME_WIDTH] = '\0';

					printf("r: %s <--- %p %s%p <--- %s (%p)\n",to_name,current_message->receiver,message_name,current_message->sender,from_name,current_message);
				}
				else if (current_node->wait_message.name_length > 0)
				{
					memcpy(from_name,current_node->wait_message.name,current_node->wait_message.name_length);
					printf("w: %s (%p %p)\n",from_name,current_node,current_node->sent_message);
				}

				current_node = current_node->next;
			}

			current_timeline = current_timeline->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : produce_output
 * Desc : This function will generate the output.
 *--------------------------------------------------------------------------------*/
unsigned int	produce_output(char* output_directory, unsigned int name_length, unsigned int format)
{
	DRAW_STATE*			draw_state;
	GROUP*				current = &g_group_tree;
	unsigned int		result = EC_OK;

	draw_state = calloc(1,sizeof(DRAW_STATE));

	/* initialise the draw state */
	draw_state->offset = 0;
	draw_state->format = format;
	draw_state->buffer = malloc(2048);
	draw_state->buffer_size = 2048;
	
	while (current != NULL)
	{
		if (current->state_machine != NULL)
		{
			draw_state->type = DIAGRAM_TYPE_STATE_MACHINE;				/* TODO: debug */
			if (output_open(draw_state,current->state_machine->group,output_directory,name_length))
			{
				generate_state_machine(draw_state,current->state_machine);
				output_close(draw_state);
			}
		}

		if (current->sequence_diagram != NULL)
		{
			draw_state->type = DIAGRAM_TYPE_SEQUENCE_DIAGRAM;			/* TODO: debug */
			if (output_open(draw_state,current->state_machine->group,output_directory,name_length))
			{
				generate_sequence_diagram(draw_state,current->sequence_diagram);
				output_close(draw_state);
			}
		}
		current = current->next;
	}

	free(draw_state->buffer);

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : check_state_machine
 * Desc : This function will check to see if the state machine is valid.
 *--------------------------------------------------------------------------------*/
unsigned int	check_state_machine(STATE_MACHINE* state_machine)
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
NODE*	handle_reorder(NODE* search_node, NODE* psearch_node, NODE* previous_node, TIMELINE* timeline)
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
ERROR_CODES	reorder_nodes(TIMELINE* timeline)
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
unsigned int	insert_function(TIMELINE* timeline, NODE** start, FUNCTION* function, unsigned int depth)
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
ERROR_CODES	node_reorder(SEQUENCE_DIAGRAM* sequence_diagram)
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
unsigned int	function_fixup(SEQUENCE_DIAGRAM* sequence_diagram)
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
NODE*	copy_node(NODE* node, NODE_LIST* list)
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
void	merge_nodes(NODE_LIST* list, NODE* insert_node)
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
NODE*	duplicate_function(NODE* start_node)
{
	NODE* 		result = start_node;
	NODE* 		copy;
	NODE* 		prev_node;
	NODE* 		stop_node = start_node->function_end;
	NODE* 		active_node = start_node;
	MESSAGE*	new_message;
	NODE_LIST	insert_list = {NULL,NULL,NULL};

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
NODE*	find_wait_node(MESSAGE* search_message, MESSAGE* sent_message)
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
unsigned int	find_wait(TIMELINE* search_timeline, TIMELINE* timeline, NODE** sending_node, unsigned int broadcast)
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
					printf("1 needs dup: %s\n",sent_message->name);
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

					printf("%p %p\n",current_node,current_node->received_message);

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
unsigned int	connect_responses(SEQUENCE_DIAGRAM* sequence_diagram)
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
unsigned int	connect_sequence(SEQUENCE_DIAGRAM* sequence_diagram)
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
unsigned int semantic_check ( void )
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

static	unsigned char	output_name[] = {'o','u','t',PATH_SEPARATOR,'\0'};

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : main
 *--------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
	int				quiet = 0;
	int				failed = 0;
	int				verbose = 0;
	char*			infile_name = NULL;
	char*			error_param = "";
	char*			error_string = "";
	unsigned char	output_directory[FILENAME_MAX];
	unsigned int	start = 1;
	unsigned int	output_format = OUTPUT_TEXT;
	unsigned int	output_length = 4;
	unsigned char*	param_mask;

	memcpy(output_directory,output_name,4);

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
				param_mask[start] = 1;
				
				switch (argv[start][1])
				{
					case 'v':	/* verbose - add extra comments to the output */
						verbose = 1;
						break;

					case 'q':	/* quiet - suppress non error outputs */
						quiet = 1;
						break;

					case 'o':	/* output directory */
						if (argv[start][2] != '\0')
						{
							output_length = strcpycnt(output_directory,&argv[start][2],FILENAME_MAX);
						}
						else if (((start + 1) < argc) && argv[start+1][0] != '-')
						{
							start++;

							output_length = strcpycnt(output_directory,argv[start],FILENAME_MAX);
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
		if (output_directory[output_length-2] != PATH_SEPARATOR)
		{
			if (output_length < FILENAME_MAX)
			{
				output_directory[output_length-1] = PATH_SEPARATOR;
				output_directory[output_length] = '\0';
			}
			else
			{
				output_directory[output_length++] = PATH_SEPARATOR;
			}
		}

		for (start = 1; start < argc; start++)
		{
			if (param_mask[start] == 0)
			{
				/* ok, this should be an input file */
				failed |= process_input(argv[start]);
			}
		}

		/* change the filename that the error reports are going to be reported against */
		g_input_filename = (unsigned char*) output_directory;

		if (!failed)
		{
			if ((failed = semantic_check()) == EC_OK)
			{
				failed = produce_output(output_directory,output_length,output_format);
			}
		}
	}

	if (failed)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}

