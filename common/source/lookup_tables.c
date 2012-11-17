/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *    file: lookup_tables
 *    desc: This file holds the functions that handle the look-up tables.
 *
 *  author: 
 *    date: 12/10/2012 18:35:59
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "document_generator.h"
#include "lookup_tables.h"

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_lookup
 * Desc : This function will find a lookup in the lookup table.
 *--------------------------------------------------------------------------------*/
LOOKUP_ITEM* find_lookup ( LOOKUP_LIST* lookup_list, unsigned char* name, unsigned int name_length )
{
	unsigned int	hash = fnv_32_hash(name,name_length);
	unsigned int	count;
	unsigned int	index_count = 0;
	LOOKUP_ITEM*	result = NULL;
	LOOKUP_LIST*	current = lookup_list;
	LOOKUP_LIST*	previous = current;
	
	while (current != NULL && result == NULL)
	{
		for (count=0;count<current->num_items;count++)
		{
			if (current->lookup[count].hash == hash)
			{
				result = &current->lookup[count];
				break;
			}
		}

		index_count++;
		previous = current;
		current = current->next;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : set_lookup_group
 * Desc : This function will set the lookup items group field.
 *--------------------------------------------------------------------------------*/
void	set_lookup_group(LOOKUP_ITEM *item, unsigned short group_id)
{
	item->group_id = group_id;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : set_lookup_name
 * Desc : This function will set the lookup name.
 *--------------------------------------------------------------------------------*/
void	set_lookup_name(LOOKUP_ITEM *item, NAME* name, unsigned short line_num)
{
	unsigned int	hash = fnv_32_hash(name->name,name->name_length);
	
	item->hash = hash;
	item->line_num = line_num;
	item->name_length = name->name_length;
	
	item->name = malloc(name->name_length);
	memcpy(item->name,name->name,name->name_length);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : new_lookup
 * Desc : This function will create a new lookup slot.
 *--------------------------------------------------------------------------------*/
unsigned int new_lookup ( LOOKUP_LIST* lookup_list )
{
	unsigned int	result = INVALID_ITEM;
	unsigned int	index_count = 0;
	LOOKUP_LIST*	current = lookup_list;
	LOOKUP_LIST*	previous = current;
	
	/* find the end of the index */
	while (current != NULL && result == INVALID_ITEM)
	{
		index_count++;
		previous = current;
		current = current->next;
	}

	if (previous->num_items < LOOKUP_INDEX_SIZE)
	{
		/*OK, we have space in the first block */
		index_count--;
		
		result = (index_count * LOOKUP_INDEX_SIZE) + previous->num_items;
		previous->num_items++;
	}
	else
	{
		/* need to add a block to the lookup table */
		LOOKUP_LIST* temp = calloc(1,sizeof(LOOKUP_LIST));
		previous->next = temp;

		result = ((index_count) * LOOKUP_INDEX_SIZE);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_lookup
 * Desc : This function will find a lookup if it exists and if it does not then it
 *        adds it.
 *--------------------------------------------------------------------------------*/
unsigned int add_lookup ( LOOKUP_LIST* lookup_list, const char* name, unsigned int name_length, const char* payload, unsigned int payload_length, unsigned short line_num )
{
	unsigned int	hash = fnv_32_hash(name,name_length);
	unsigned int	count;
	unsigned int	result = INVALID_ITEM;
	unsigned int	index_count = 0;
	LOOKUP_LIST*	current = lookup_list;
	LOOKUP_LIST*	previous = current;
	
	while (current != NULL && result == INVALID_ITEM)
	{
		for (count=0;count<current->num_items;count++)
		{
			if (current->lookup[count].hash == hash)
			{
				result = (index_count * LOOKUP_INDEX_SIZE) + count;
				break;
			}
		}

		index_count++;
		previous = current;
		current = current->next;
	}

	if (result == INVALID_ITEM)
	{
		if (previous->num_items < LOOKUP_INDEX_SIZE)
		{
			index_count--;
			
			previous->lookup[previous->num_items].hash = hash;
			previous->lookup[previous->num_items].line_num = line_num;
			previous->lookup[previous->num_items].name_length = name_length;
			
			previous->lookup[previous->num_items].name = malloc(name_length);
			memcpy(previous->lookup[previous->num_items].name,name,name_length);

			previous->lookup[previous->num_items].payload = malloc(payload_length);
			previous->lookup[previous->num_items].payload_length = payload_length;
			memcpy(previous->lookup[previous->num_items].payload,payload,payload_length);

			result = (index_count * LOOKUP_INDEX_SIZE) + previous->num_items;
			previous->num_items++;
		}
		else
		{
			LOOKUP_LIST* temp = calloc(1,sizeof(LOOKUP_LIST));
			previous->next = temp;
	
			temp->lookup[0].hash = hash;
			temp->lookup[0].line_num = line_num;
			temp->lookup[0].name_length = name_length;
			
			temp->lookup[0].name = malloc(name_length);
			memcpy(temp->lookup[0].name,name,name_length);
	
			temp->lookup[0].payload = malloc(payload_length);
			temp->lookup[0].payload_length = payload_length;
			memcpy(previous->lookup[0].payload,payload,payload_length);

			temp->num_items = 1;
			
			result = (index_count * LOOKUP_INDEX_SIZE);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_add_lookup
 * Desc : This function will find a lookup if it exists and if it does not then it
 *        adds it.
 *--------------------------------------------------------------------------------*/
unsigned int find_add_lookup ( LOOKUP_LIST* lookup_list, unsigned char* name, unsigned int name_length )
{
	unsigned int	hash = fnv_32_hash(name,name_length);
	unsigned int	count;
	unsigned int	result = INVALID_ITEM;
	unsigned int	index_count = 0;
	LOOKUP_LIST*	current = lookup_list;
	LOOKUP_LIST*	previous = current;
	
	while (current != NULL && result == INVALID_ITEM)
	{
		for (count=0;count<current->num_items;count++)
		{
			if (current->lookup[count].hash == hash)
			{
				result = (index_count * LOOKUP_INDEX_SIZE) + count;
				break;
			}
		}

		index_count++;
		previous = current;
		current = current->next;
	}
	
	if (result == INVALID_ITEM)
	{
		if (previous->num_items < LOOKUP_INDEX_SIZE)
		{
			index_count--;

			previous->lookup[previous->num_items].hash = hash;
			previous->lookup[previous->num_items].name = malloc(name_length);
			previous->lookup[previous->num_items].name_length = name_length;
			memcpy(previous->lookup[previous->num_items].name,name,name_length);

			result = (index_count * LOOKUP_INDEX_SIZE) + previous->num_items;
			previous->num_items++;
		}
		else
		{
			LOOKUP_LIST* temp = calloc(1,sizeof(LOOKUP_LIST));
			previous->next = temp;
	
			temp->lookup[0].hash = hash;
			temp->lookup[0].name = malloc(name_length);
			temp->lookup[0].name_length = name_length;
			memcpy(temp->lookup[0].name,name,name_length);
			temp->num_items = 1;
			
			result = (index_count * LOOKUP_INDEX_SIZE);
		}
	}

	return result;
}


