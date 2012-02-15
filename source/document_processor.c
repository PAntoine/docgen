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

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_state_machine
 * Desc : This function will produce a dot file for the given state machine.
 *--------------------------------------------------------------------------------*/
static void	generate_state_machine(DRAW_STATE* draw_state, STATE_MACHINE* state_machine)
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
static void	generate_columns(SEQUENCE_DIAGRAM* sequence_diagram, DRAW_STATE* draw_state)
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
static unsigned int	generate_sequence_diagram( DRAW_STATE* draw_state, SEQUENCE_DIAGRAM* sequence_diagram)
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

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : produce_output
 * Desc : This function will generate the output.
 *--------------------------------------------------------------------------------*/
unsigned int	produce_output(GROUP* group_tree, unsigned char* output_directory, unsigned int name_length, unsigned int format)
{
	DRAW_STATE*			draw_state;
	GROUP*				current = group_tree;
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
			draw_state->type = DIAGRAM_TYPE_STATE_MACHINE;
			
			if (output_open(draw_state,current->state_machine->group,output_directory,name_length))
			{
				generate_state_machine(draw_state,current->state_machine);
				output_close(draw_state);
			}
		}

		if (current->sequence_diagram != NULL)
		{
			draw_state->type = DIAGRAM_TYPE_SEQUENCE_DIAGRAM;
			
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
 * Name : read_numeric_record
 * Desc : This function will read a string record.
 *--------------------------------------------------------------------------------*/
unsigned int	read_numeric_record(unsigned int offset, unsigned char* buffer, unsigned short *parameter)
{
	*parameter = (((unsigned short)buffer[offset + 1]) << 8) | buffer[offset+2];

	return (offset + 3);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_numerics_record
 * Desc : This function will read a string record.
 *--------------------------------------------------------------------------------*/
unsigned int	read_numerics_record(unsigned int offset, unsigned char* buffer, unsigned short *parameter1, unsigned int *parameter2)
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
unsigned int	read_string_record(unsigned int offset, unsigned char* buffer, NAME* name)
{
	name->name_length = buffer[offset+1];
	name->name = &buffer[offset+2];

	return (offset + 2 + buffer[offset+1]);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : read_group_record
 * Desc : This function reads a group record.
 *--------------------------------------------------------------------------------*/
unsigned int	read_group_record(	unsigned int	offset, 
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
unsigned int	read_group_id_record(	unsigned int	offset, 
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
unsigned int	read_message_record(	unsigned int	offset, 
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
STATE_TRANSITION*	add_transition(STATE* state, unsigned short to_id)
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
void	add_trigger(STATE_TRANSITION* transition,GROUP* group, NAME* trigger_name)
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
void	add_triggers(STATE_TRANSITION* transition,GROUP* group, NAME* trigger_name)
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
void	add_message(NODE* sender, TIMELINE* destination, BLOCK_NAME* name)
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
NODE*	add_node(TIMELINE* timeline, unsigned short id, unsigned int flags)
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
MESSAGE*	create_message(unsigned short sender_id, unsigned short receiver_id, NAME* message_name)
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
unsigned int	input_model(char* model_file, GROUP* group_tree,unsigned short *max_state, unsigned short* max_node)
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
 *        messages and state taransitions to the target state/nodes.
 *--------------------------------------------------------------------------------*/
unsigned int	connect_model(GROUP* group_tree, unsigned short max_state, unsigned short max_node)
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
 * Name : main
 *--------------------------------------------------------------------------------*/
int main(int argc, const char *argv[])
{
	int				quiet = 0;
	int				failed = 0;
	int				verbose = 0;
	char*			input_name = NULL;
	char*			output_name = "doc.gdo";
	char*			error_param = "";
	char*			error_string = "";
	unsigned int	result = EC_OK;
	unsigned int	start = 1;
	unsigned int	output_length = 4;
	unsigned short	max_node;
	unsigned short	max_state;
	unsigned char*	param_mask;
	GROUP			group_tree;

	memset(&group_tree,0,sizeof(GROUP));
	memcpy(output_name,output_name,4);

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
				/* now load and process the input files */
				produce_output(&group_tree,(unsigned char*)"out_test",sizeof("out_test")-1, OUTPUT_TEXT);
			}
		}
	}

	if (failed || result != EC_OK)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}

