/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : utilities
 * Desc  : This file holds the implementation of the utility functions.
 *
 * Author: pantoine
 * Date  : 10/11/2011 19:43:01
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "document_generator.h"

/*---  FUNCTION  ----------------------------------------------------------------------*
 *         Name:  fnv_32_hash
 *  Description:  This function will return a hash of the string passed in.
 *                using FNV hash.
 *-------------------------------------------------------------------------------------*/
unsigned int	fnv_32_hash ( unsigned char *string, unsigned int length )
{
	unsigned int	h = 0x811c9dc5;
	unsigned int	i;

	for ( i = 0; i < length; i++ )
	{
		h = ( h * 0x01000193 ) ^ string[i];
	}

	return h;
}

/*---  FUNCTION  ----------------------------------------------------------------------*
 *         Name:  SizeToString
 *  Description:  This function will return int as a string max of max-length  bytes.
 *  			  it will return an int of the size of the string that has been made.
 *-------------------------------------------------------------------------------------*/
unsigned int	SizeToString(unsigned char* string,unsigned int size, unsigned int max_length)
{
	unsigned int digits = 0;

	if (size == 0)
	{
		string[max_length-1] = '0';
		digits = 1;
	}
	else
	{
		while(size != 0 && digits < max_length)
		{
			string[(max_length-digits)-1] = '0' + (size % 10);
			digits++;
			size = size / 10;
		}
	}

	return digits;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : hex_dump
 * Desc : This function will dump the given buffer to the standard output.
 *--------------------------------------------------------------------------------*/
void	hex_dump(unsigned char* memory, unsigned int memory_size)
{
	int		count,count2,count3,count4;
	char	line[89];
	char	HexBit[] = "0123456789ABCDEF";

	/* write the straplines */
	printf("                     Hex Dump                                          Characters      \n");
	printf("                     ------------------------------------------------  ----------------\n");

	/* clear the space under the timestamp */
	memset(line,' ',86);
	
	/* now dump the rest */
	for (count=0;count < memory_size;count+=16)
	{
		SizeToString((unsigned char*)line,count,9);

		for(count2=count,count3=21,count4=71;count2 < count+16 && count2 < memory_size;count2++,count3+=3,count4++)
		{
			/* do the main hex conversion */
			line[count3] = HexBit[((memory[count2]&0xf0)>>4)];
			line[count3+1] = HexBit[(memory[count2]&0x0f)];
			line[count3+2] = ' ';

			/* do the end stuff */
			if (!isprint(memory[count2]))
				line[count4] = '.';
			else
				line[count4] = (char) memory[count2];

		}

		if (count2 < count+16)
		{
			for(;count2<count+16;count2++,count3+=3,count4++)
			{
				line[count3] = ' ';
				line[count3+1] = ' ';
				line[count3+2] = ' ';

				line[count4] = ' ';
			}
		}

		line[87] = '\n';
		line[88] = '\0';
		
		/*just to stop GCC whining about things it should not whine about */
		printf("%s",line);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : strcpycnt
 * Desc : String copy and count.
 *        This function will return the number of bytes copied upto a max number
 *        of bytes.
 *--------------------------------------------------------------------------------*/
unsigned int	strcpycnt( unsigned char* dest, unsigned char* src, unsigned int max)
{
	unsigned int result = 0;

	for (; result < max && src[result] != 0; result++)
	{
		dest[result] = src[result];
	}

	if (result < max)
		dest[result++] = '\0';

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
		if ((current_node->flags & FLAG_SEQUENCE_RESPONDS) == FLAG_SEQUENCE_RESPONDS)
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

NAME	du;

void dump(char* string,NAME* name)
{
	write(1,string,strlen((char*)string));
	write(1," ",1);
	write(1,name->name,name->name_length);
	write(1,"\n",1);
}


/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_group
 * Desc : This function will add the named group to the list. It should only be
 *        called after the item has been searched for, otherwise duplicates will
 *        be added to the list as this function does not check for the name
 *        previously existing.
 *--------------------------------------------------------------------------------*/
GROUP*	add_group ( GROUP* group_tree, unsigned char* name, unsigned int name_length )
{
	GROUP* result = NULL;
	GROUP* current = group_tree;

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
GROUP* find_group ( GROUP* group_tree, unsigned char* name, unsigned int name_length )
{
	GROUP* result = NULL;
	GROUP* current = group_tree;

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


