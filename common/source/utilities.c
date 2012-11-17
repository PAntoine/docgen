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
#include "utilities.h"
#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * Standard expectable char table
 *
 * This is the escapable chars for the atom names in the fields, and for most
 * c-like languages.
 *--------------------------------------------------------------------------------*/
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
 * Name : IntToAlphaSafe
 * Desc : This function is a itoa implementation. Can't be arsed working out which
 *        flavour of this function to use, so it's getting the peter treatment. As
 *        want the code to have as little external dependencies as possible to 
 *        stop maintenance problems.
 *
 *        Index To Alpha Safe. The String to size function is obviously quicker
 *        but more difficult to use in code.
 *
 *        It returns the number of chars written to the array.
 *--------------------------------------------------------------------------------*/
unsigned int	IntToAlphaSafe(int value, unsigned char* buffer, unsigned int buffer_size)
{
	unsigned int count;
	unsigned int result = 1;
	unsigned int negative = 0;
	unsigned char temp;

	if (buffer != NULL)
	{
		if (value == 0)
		{
			buffer[0] = '0';
			buffer[1] = '\0';
			result = 2;
		}
		else
		{
			if (value < 0)
			{
				buffer[1] = '-';
				result++;
				value = 0 - value;
			}

			while (value > 0 && result < buffer_size)
			{
				buffer[result] = '0' + (value % 10);
				result++;
				value = value / 10;
			}

			/* swap it around - if odd no need to swap the middle digit */
			for (count=0;count<result/2;count++)
			{
				temp = buffer[count];
				buffer[count] = buffer[result-count-1];
				buffer[result-count-1] = temp;
			}

			buffer[result-1] = '\0';
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : dump_text
 * Desc : This is a simple piece of debug that will dump a section of text.
 *--------------------------------------------------------------------------------*/
void	dump_text(char* name, unsigned char* buffer, unsigned int length)
{
	printf("%s:",name);
	fwrite((char*)buffer,length,1,stdout);
	printf("\n");
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
	GROUP* 			result = NULL;
	GROUP* 			current = group_tree;

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
GROUP* find_group ( GROUP* group_tree, unsigned char* name, unsigned int name_length, unsigned int* special_group)
{
	GROUP* result = NULL;
	GROUP* current = group_tree;
			
	if (name_length == 3 && (memcmp("all",name,name_length) == 0))
	{
		/* user wants the "all" group */
		result = group_tree->next;
		*special_group = ALL_GROUP;
	}
	else if (name_length == 8 && memcmp("document",name,name_length) == 0)
	{
		result = group_tree;
		*special_group = DOCUMENT_GROUP;
	}
	else
	{
		while (current != NULL)
		{
			if (current->name_length == name_length && memcmp(current->name,name,name_length) == 0)
			{
				result = current;
				break;
			}
			current = current->next;
		}
	
		*special_group = NORMAL_GROUP;
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

		/* Ok, name length for rendering */
		if (name_length > group->sequence_diagram->max_name_length)
		{
			group->sequence_diagram->max_name_length = name_length;
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

		/* for drawing needs to know the max state length */
		if (group->state_machine->max_state_length < name_length)
		{
			group->state_machine->max_state_length = name_length;
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
 * Name : copy_name
 * Desc : This function will copy a name and create a new item.
 *--------------------------------------------------------------------------------*/
void	copy_name(NAME* from, NAME* to)
{
	if (from != NULL)
	{
		to->name = malloc(from->name_length);
		to->name_length = from->name_length;
		to->fixed = from->fixed;
		to->allocated_size = from->name_length;

		memcpy(to->name,from->name,from->name_length);
	}
	else
	{
		to->name = NULL;
		to->fixed = 0;
		to->name_length = 0;
		to->allocated_size = 0;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: compare_name
 * @desc: This function will compare two NAMEs.
 *--------------------------------------------------------------------------------*/
int	compare_name(NAME* one, NAME* two)
{
	int result;

	if ((result = one->name_length - two->name_length) == 0)
	{
		result = memcmp(one->name,two->name,one->name_length);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: concat_names
 * @desc: This function will concatenate three NAMEs into a new NAME. This name
 *        will have to be released to avoid memory leaks. This new name will
 *        be of the same size as the two names added together. Any of the names
 *        can be null and the function will handle it.
 *--------------------------------------------------------------------------------*/
void	concat_names(NAME* new_name, NAME* first_name, NAME* second_name, NAME* third_name)
{
	unsigned int size = 0;
	unsigned int offset = 0;

	new_name->name = NULL;
	new_name->name_length = 0;

	if (first_name != NULL)
	{
		size += first_name->name_length;
		new_name->fixed = first_name->fixed;
	}

	if (second_name != NULL)
	{
		size += second_name->name_length;
		new_name->fixed |= second_name->fixed;
	}

	if (third_name != NULL)
	{
		size += third_name->name_length;
		new_name->fixed |= third_name->fixed;
	}

	if (size > 0)
	{
		new_name->name = malloc(size);
		new_name->name_length = size;
		new_name->allocated_size = size;
		
		if (first_name != NULL)
		{
			memcpy(new_name->name,first_name->name,first_name->name_length);
			offset = first_name->name_length;
		}

		if (second_name != NULL)
		{
			memcpy(&new_name->name[offset],second_name->name,second_name->name_length);
			offset += second_name->name_length;
		}

		if (third_name != NULL)
		{
			memcpy(&new_name->name[offset],third_name->name,third_name->name_length);
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: allocate_name
 * @desc: This function will allocate a new name. Obviously you should call free
 *        on the new_name created or memory will leak.
 *--------------------------------------------------------------------------------*/
void	allocate_name(NAME* new_name,unsigned int name_size)
{
	new_name->name = malloc(name_size);
	new_name->name_length = 0;
	new_name->fixed = 0;
	new_name->allocated_size = name_size;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: append_string
 * @desc: This function will append data into the name, but it will only do this
 *        if allocated size > 0 and name_length < allocate_size.
 *--------------------------------------------------------------------------------*/
void	append_string(NAME* name, unsigned char* string, unsigned int string_size)
{
	if (name->allocated_size > 0 && (name->name_length + string_size) <= name->allocated_size)
	{
		memcpy(&name->name[name->name_length],string,string_size);
		name->name_length += string_size;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: append_name
 * @desc: This function will append data into the name, but it will only do this
 *        if allocated size > 0 and name_length < allocate_size.
 *--------------------------------------------------------------------------------*/
void	append_name(NAME* name, NAME* append)
{
	if (name->allocated_size > 0 && (name->name_length + append->name_length) <= name->allocated_size)
	{
		memcpy(&name->name[name->name_length],append->name,append->name_length);
		name->name_length += append->name_length;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: free_name
 * @desc: This function will only free a NAME that has been allocated by one of
 *        the allocation function - this is because the name may have been set
 *        from a constant in BSS or off the stack.
 *--------------------------------------------------------------------------------*/
void	free_name(NAME* name)
{
	if (name->allocated_size > 0 && name->name != NULL)
	{
		free(name->name);
		name->name = NULL;
	}

	/* always safe to zero the name */
	name->fixed = 0;
	name->name_length = 0;
	name->allocated_size = 0;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : lfsr_32
 * Desc : This function is a basic LFSR. 
 *        It used the following poly: 32,31,16,2.
 *--------------------------------------------------------------------------------*/
unsigned int	lfsr_32(unsigned int seed)
{
	unsigned int bit = (	((seed      ) ^
							 (seed << 1 ) ^
							 (seed << 15) ^
							 (seed << 30) ^
							 (seed << 31)) & (1 << 31));

	return ((seed >> 1) | bit);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_generate_label
 * Desc : This function will generate an index table from a DOC_SECTION* in the
 *        linked table. This will walk up the table to build the string.
 *        Because the string is being built from the end to the front, it needs
 *        a max length and will return the offset of the buffer that the string
 *        started at.
 *--------------------------------------------------------------------------------*/
unsigned int	index_generate_label(DOC_SECTION* section, unsigned int end_level, INDEX_STYLE style, unsigned char* buffer, unsigned int buffer_size ,unsigned char delimiter)
{
	unsigned int	result = buffer_size;
	DOC_SECTION*	current_section = section;

	while(current_section != NULL && result > 0 && current_section->parent != NULL && current_section->level > end_level)
	{
		switch (style)
		{
			case INDEX_STYLE_BARE:
				result--;
				buffer[result] = ' ';
				break;

			case INDEX_STYLE_NUMBERS:
				result -= SizeToString(buffer,current_section->index,result);
				buffer[result-1] = delimiter;
				result--;
				break;
		}

		current_section = current_section->parent;
	}

	if (buffer[result] == delimiter)
	{
		result++;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: write_text_index_entry
 *  desc: This function will output the index line.
 *--------------------------------------------------------------------------------*/
void	write_text_index_entry(DRAW_STATE* draw_state, DOC_SECTION* section)
{
	unsigned int	buffer_offset = 0;
	unsigned int	temp_size;
	unsigned int	fill_size;
	unsigned int	start = 0;
	unsigned char*	buffer;
	unsigned char	temp[200];

	buffer = draw_state->output_buffer;
	buffer_offset = draw_state->margin_width;

	if ((draw_state->format_flags & OUTPUT_FORMAT_FLAT) == 0)
	{
		start = index_generate_label(section,0,INDEX_STYLE_BARE,temp,200,'.');
		memcpy(&buffer[buffer_offset],&temp[start],200-start);
		buffer_offset += 200 - start;
	}
	
	/* produce the numbers if we need them */
	if ((draw_state->format_flags & (OUTPUT_FORMAT_REFERENCE | OUTPUT_FORMAT_NUMBERED)) != 0)
	{
		start = index_generate_label(section,0,INDEX_STYLE_NUMBERS,temp,200,'.');
		temp_size = 200 - start;
	}

	if ((draw_state->format_flags & OUTPUT_FORMAT_NUMBERED) != 0)
	{
		memcpy(&buffer[buffer_offset],&temp[start],temp_size);
		buffer_offset += temp_size;

		buffer[buffer_offset++] = ' ';
		buffer[buffer_offset++] = ' ';
		buffer[buffer_offset++] = ' ';
	}

	memcpy(&buffer[buffer_offset],section->section_title.name,section->section_title.name_length);
	buffer_offset += section->section_title.name_length;

	if ((draw_state->format_flags & OUTPUT_FORMAT_REFERENCE) != 0)
	{
		/* calc the size of the reference */
		buffer[buffer_offset++] = ' ';

		/* calc and fill the gap */
		fill_size = draw_state->page_width - buffer_offset - temp_size - 2;

		if ((draw_state->format_flags & OUTPUT_FORMAT_LINED) != 0)
		{
			memset(&buffer[buffer_offset],'.',fill_size);
		}
		else
		{
			memset(&buffer[buffer_offset],' ',fill_size);
		}

		/* write the reference */
		buffer_offset += fill_size;
		buffer[buffer_offset++] = ' ';
		memcpy(&buffer[buffer_offset],&temp[start],temp_size);
		buffer_offset += temp_size;
	}
		
	draw_state->format->output_raw(draw_state,buffer,buffer_offset);
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: make_version_string
 *  desc: This function makes a version string from the version passed in.
 *        The version is 32bit value that has the following format:
 *           MMmmbbXX
 *        where:
 *           MM = Major version
 *           mm = Minor version
 *           bb = bug release version
 *           XX = type of release (alpha,beta,debug,test,release)
 *--------------------------------------------------------------------------------*/
unsigned int	make_version_string(unsigned int version, unsigned char* version_string)
{
	unsigned int result = 0;
	
	version_string[11] = '.';

	switch(version & 0x000000ff)
	{
		case 1:	version_string[12] = 'a';	break;
		case 2:	version_string[12] = 'b';	break;
		case 3:	version_string[12] = 'd';	break;
		case 4:	version_string[12] = 't';	break;
		default:
			version_string[12] = ' ';
			version_string[11] = ' ';
			break;
	}

	result = 10;
	result -= SizeToString(version_string,((version & 0x0000ff00) >> 8),result);
	version_string[result-1] = '.';
	result--;
	result -= SizeToString(version_string,((version & 0x00ff0000) >> 16),result);
	version_string[result-1] = '.';
	result--;
	result -= SizeToString(version_string,((version & 0xff000000) >> 24),result);

	return result;
}

