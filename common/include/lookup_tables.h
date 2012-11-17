/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *    file: lookup_tables
 *    desc: This file describles the the structures and functions that are used 
 *          in creating the lookup tables.
 *
 *  author: 
 *    date: 12/10/2012 16:38:09
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __LOOKUP_TABLES_H__
#define __LOOKUP_TABLES_H__

/*--------------------------------------------------------------------------------*
 * The Group lookup list structures.
 *--------------------------------------------------------------------------------*/
#define	LOOKUP_INDEX_SIZE	(100)

typedef struct
{
	unsigned int	hash;
	unsigned int	name_length;
	unsigned int	payload_length;
	unsigned short	line_num;
	unsigned short	group_id;
	unsigned char*	name;
	unsigned char*	payload;

} LOOKUP_ITEM;

typedef struct tag_lookup_list
{
	unsigned int	num_items;
	LOOKUP_ITEM		lookup[LOOKUP_INDEX_SIZE];

	struct tag_lookup_list*	next;

} LOOKUP_LIST;

/*--------------------------------------------------------------------------------*
 * functions.
 *--------------------------------------------------------------------------------*/

LOOKUP_ITEM* 	find_lookup ( LOOKUP_LIST* lookup_list, unsigned char* name, unsigned int name_length );
void			set_lookup_group(LOOKUP_ITEM *item, unsigned short group_id);
void			set_lookup_name(LOOKUP_ITEM *item, NAME* name, unsigned short line_num);
unsigned int	new_lookup ( LOOKUP_LIST* lookup_list );
unsigned int	add_lookup ( LOOKUP_LIST* lookup_list, const char* name, unsigned int name_length, const char* payload, unsigned int payload_length, unsigned short line_num );
unsigned int	find_add_lookup ( LOOKUP_LIST* lookup_list, unsigned char* name, unsigned int name_length );

#endif

