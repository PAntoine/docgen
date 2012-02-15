/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : utilities
 * Desc  : This file defines the utiltiy functions.
 *
 * Author: pantoine
 * Date  : 10/11/2011 19:39:42
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include "document_generator.h"

unsigned int	fnv_32_hash ( unsigned char *string, unsigned int length );
unsigned int	SizeToString(unsigned char* string,unsigned int size, unsigned int max_length);
void			hex_dump(unsigned char* memory, unsigned int memory_size);
unsigned int	strcpycnt( unsigned char* dest, unsigned char* src, unsigned int max);
NODE*			search_next_node(NODE* current_node, NODE** active_node);
NODE*			next_active_node(NODE* current_node);
GROUP*			add_group ( GROUP* group_tree, unsigned char* name, unsigned int name_length );
GROUP* 			find_group ( GROUP* group_tree, unsigned char* name, unsigned int name_length );
TIMELINE* 		find_timeline ( GROUP* group, unsigned char* name, unsigned int name_length );
TIMELINE* 		add_timeline ( GROUP* group, unsigned char* name, unsigned int name_length, unsigned int line_number, unsigned int flags );
STATE* 			add_state ( GROUP* group, unsigned char* name, unsigned int name_length, unsigned int line_number, unsigned int flags );
STATE* 			find_state ( GROUP* group, unsigned char* name, unsigned int name_length );

void dump(char* string,NAME* name);

#endif

