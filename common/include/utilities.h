/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : utilities
 * Desc  : This file defines the utility functions.
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

#include "output_format.h"
#include "document_generator.h"

#define	NORMAL_GROUP	(0)
#define	ALL_GROUP		(1)
#define	DOCUMENT_GROUP	(2)
#define VERSION_STRING_LENGTH	(13)	/* version string (max): MMM.mmm.bug.a/b/r */

unsigned int	fnv_32_hash( unsigned char *string, unsigned int length );
unsigned int	SizeToString(unsigned char* string,unsigned int size, unsigned int max_length);
unsigned int	IntToAlphaSafe(int value, unsigned char* buffer, unsigned int buffer_size);
void			hex_dump(unsigned char* memory, unsigned int memory_size);
unsigned int	strcpycnt( unsigned char* dest, unsigned char* src, unsigned int max);
NODE*			search_next_node(NODE* current_node, NODE** active_node);
NODE*			next_active_node(NODE* current_node);
GROUP*			add_group ( GROUP* group_tree, unsigned char* name, unsigned int name_length );
GROUP* 			find_group ( GROUP* group_tree, unsigned char* name, unsigned int name_length, unsigned int* all_groups );
TIMELINE* 		find_timeline ( GROUP* group, unsigned char* name, unsigned int name_length );
TIMELINE* 		add_timeline ( GROUP* group, unsigned char* name, unsigned int name_length, unsigned int line_number, unsigned int flags );
STATE* 			add_state ( GROUP* group, unsigned char* name, unsigned int name_length, unsigned int line_number, unsigned int flags );
STATE* 			find_state ( GROUP* group, unsigned char* name, unsigned int name_length );
void			copy_name(NAME* from, NAME* to);
int				compare_name(NAME* one, NAME* two);
void			concat_names(NAME* new_name, NAME* first_name, NAME* second_name, NAME* third_name);
void			allocate_name(NAME* new_name,unsigned int name_size);
void			append_string(NAME* name, unsigned char* string, unsigned int string_size);
void			append_name(NAME* name, NAME* append);
void			free_name(NAME* name);
unsigned int	lfsr_32(unsigned int seed);


void			dump(char* string,NAME* name);
void			dump_text(char* name,unsigned char* buffer, unsigned int length);

unsigned int	word_wrap(unsigned char* buffer, unsigned int buffer_length, unsigned int* wrapped);
void			write_text(DRAW_STATE* draw_state, NAME* text_block);
void			write_block_text(DRAW_STATE* draw_state, NAME* text_block, unsigned int flags);
unsigned int	text_punctuation(unsigned char* buffer);

unsigned int	make_version_string(unsigned int version, unsigned char* version_string);

unsigned int	index_generate_label(DOC_SECTION* section, unsigned int end_level, INDEX_STYLE style, unsigned char* buffer, unsigned int buffer_size ,unsigned char delimiter);
void			index_walk_tree(DRAW_STATE* draw_state);

void			write_text_index_entry(DRAW_STATE* draw_state, DOC_SECTION* section);

#endif

