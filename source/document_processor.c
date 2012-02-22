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
#include "error_codes.h"
#include "utilities.h"
#include "supported_formats.h"
#include "document_generator.h"

extern OUTPUT_FORMATS	output_formats[];


/*--------------------------------------------------------------------------------*
 * static constant strings
 *--------------------------------------------------------------------------------*/
static unsigned char	string_none[] = "";
static unsigned char	string_state_machine[] = "state_machine";
static unsigned char	string_sequence_diagram[] = "sequence_diagram";
static unsigned char*	type_string[] = {string_none,string_state_machine,string_sequence_diagram};
static unsigned int		type_length[] = {0,sizeof(string_state_machine)-1,sizeof(string_sequence_diagram)-1};

unsigned char is_valid_char[] = 
{
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x01,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};



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
static void	generate_state_machine(DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* tree)
{
	STATE*				current_state;
	GROUP*				group;
	STATE_TRANSITION*	current_trans;

	if ((group = find_group(tree,input_state->group_name,input_state->group_length)) == NULL)
	{
		raise_warning(0,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (input_state->item_length > 0 && (current_state = find_state(group,input_state->item_name,input_state->item_length)) == NULL)
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
	NODE*				current_node;
	STATE*				current_state;
	GROUP*				temp_group;
	GROUP*				current_group;
	TIMELINE*			current_timeline;
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

								case LINKER_BLOCK_END:
									offset = FILE_BLOCK_SIZE;
									break;

								default:
									printf("bad item in no state: %d\n",record[offset]);
									hex_dump(&record[offset],16);
									raise_warning(0,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)model_file,NULL);
									result = 1;
									break;
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
									printf("bad item in state: %02x\n",record[offset]);
									hex_dump(&record[offset],16);
									raise_warning(0,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)model_file,NULL);
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
									printf("bad item in sequence_diagram: %d\n",record[offset]);
									hex_dump(&record[offset],16);
									raise_warning(0,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)model_file,NULL);
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
						current_trans->next_state = state_jump[((unsigned int)current_trans->next_state)];
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
						if (node_jump[((unsigned int)current_node->sent_message->receiver)])
						{
							node_jump[((unsigned int)current_node->sent_message->receiver)]->received_message = current_node->sent_message;
							current_node->sent_message->target_timeline = node_jump[((unsigned int)current_node->sent_message->receiver)]->timeline;
						}
						current_node->sent_message->receiver = node_jump[((unsigned int)current_node->sent_message->receiver)];
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
						if (input_state->buffer[input_state->buffer_pos] != 's')
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
						else if (input_state->buffer[input_state->buffer_pos] == 'e')
						{
							input_state->temp_type = TYPE_SEQUENCE_DIAGRAM;
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
						input_state->internal_state = INPUT_STATE_INTERNAL_DUMP_TILL_END;
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

