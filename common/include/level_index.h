/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *    file: level_index
 *    desc: This file holds the level index functions.
 *          These functions help build the file indexes and is also used during 
 *          the rendering of pages.
 *
 *  author: 
 *    date: 17/09/2012 19:08:23
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __LEVEL_INDEX_H__
#define __LEVEL_INDEX_H__

#define LEVEL_EXTEND_SIZE	(10)

/*--------------------------------------------------------------------------------*
 * Index Styles Supported
 *--------------------------------------------------------------------------------*/

typedef enum
{
	INDEX_STYLE_BARE,
	INDEX_STYLE_NUMBERS,

	MAX_INDEX_STYLES

} INDEX_STYLE; 


/*--------------------------------------------------------------------------------*
 * structures for building the index.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned int	max_levels;
	unsigned int	current_level;
	unsigned int	previous_level;
	unsigned int	api_index_id;
	unsigned short*	level_index;
} LEVEL_INDEX;

/*--------------------------------------------------------------------------------*
 * Level Index Functions.
 *--------------------------------------------------------------------------------*/
void			level_index_init(LEVEL_INDEX* index);
unsigned short	level_index_set_level(LEVEL_INDEX* index,unsigned short level);
unsigned short	level_index_increment(LEVEL_INDEX* index);
unsigned short	level_index_set(LEVEL_INDEX* index,unsigned short index_level);
unsigned short	level_index_fix_level(LEVEL_INDEX* index, unsigned short level);
unsigned short	level_index_get_level(LEVEL_INDEX* index);
unsigned short	level_index_get_previous_level(LEVEL_INDEX* index);
unsigned short	level_index_get_index(LEVEL_INDEX* index, unsigned short level);
unsigned int	level_index_gen_label(LEVEL_INDEX* index, unsigned int start_level, INDEX_STYLE style, unsigned char* buffer, unsigned int buffer_size ,unsigned char delimiter);
void			level_index_release(LEVEL_INDEX* index);

#endif

