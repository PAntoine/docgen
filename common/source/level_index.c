/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *    file: level_index
 *    desc: 
 *
 *  author: 
 *    date: 17/09/2012 19:37:09
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "error_codes.h"
#include "level_index.h"

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_init
 *  desc: This function will initialise the level index.
 *        If the memory has already been allocated for the index, then it will just
 *        reset the table values.
 *--------------------------------------------------------------------------------*/
void	level_index_init(LEVEL_INDEX* index)
{
	if (index->level_index == NULL)
	{
		index->level_index  	= calloc(LEVEL_EXTEND_SIZE,sizeof(unsigned short));
		index->max_levels		= LEVEL_EXTEND_SIZE;
		index->current_level	= 0;
		index->previous_level	= 0;
		index->api_index_id		= 0;
	}
	else
	{
		memset(index->level_index,0,sizeof(unsigned short) * index->max_levels);
		index->current_level	= 0;
		index->previous_level	= 0;
		index->api_index_id		= 0;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_set_level
 *  desc: This function will set the level of the index.
 *--------------------------------------------------------------------------------*/
unsigned short	level_index_set_level(LEVEL_INDEX* index, unsigned short level)
{
	unsigned short count;

	index->previous_level = index->current_level;

	if (index->current_level < level)
	{
		/* reset all the levels from here to there */
		for (count=index->current_level+1;count <= level; count++)
		{
			index->level_index[count] = 1;
		}

		index->current_level = level;
	}
	else if (level > index->current_level)
	{
		if (level > (index->max_levels + LEVEL_EXTEND_SIZE))
		{
			/* this must be a bug - who would go up that many levels in one go? */
			raise_warning(0,EC_LARGE_JUMP_IN_INDEX_LEVELS,NULL,NULL);
		}
		else if (level > index->max_levels)
		{
			/* make the index level bigger */
			unsigned short* temp = realloc(index->level_index,sizeof(unsigned short) * (index->max_levels + LEVEL_EXTEND_SIZE));

			if (temp != NULL)
			{
				memset(&index->level_index[index->max_levels],0,(sizeof(unsigned short) * LEVEL_EXTEND_SIZE));
				index->current_level = level;
			}
		}
		
		index->current_level = level;
		index->level_index[index->current_level] = 1;
	}
	else
	{
		index->current_level = level;
		index->level_index[index->current_level]++;
	}

	return index->current_level;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_fix_level
 *  desc: This function will set the level, but without changing the index. The
 *        level cannot be set greater than the max_levels as this would cause the
 *        whole tree to break.
 *--------------------------------------------------------------------------------*/
unsigned short	level_index_fix_level(LEVEL_INDEX* index, unsigned short level)
{
	if (level < index->max_levels)
	{
		index->previous_level	= index->current_level;
		index->current_level	= level;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_increment
 *  desc: This function will increment the level index value.
 *--------------------------------------------------------------------------------*/
unsigned short	level_index_increment(LEVEL_INDEX* index)
{
	index->level_index[index->current_level]++;

	return index->level_index[index->current_level];
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_set
 *  desc: This function will set the index to the index value.
 *--------------------------------------------------------------------------------*/
unsigned short	level_index_set(LEVEL_INDEX* index, unsigned short index_level)
{
	index->level_index[index->current_level] = index_level;
	
	return index->level_index[index->current_level];
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_get_previous_level
 *  desc: This function will return the previous index level.
 *--------------------------------------------------------------------------------*/
unsigned short	level_index_get_previous_level(LEVEL_INDEX* index)
{
	return index->previous_level;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_get_level
 *  desc: This function will return the current index level.
 *--------------------------------------------------------------------------------*/
unsigned short	level_index_get_level(LEVEL_INDEX* index)
{
	return index->current_level;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_get_index
 *  desc: This function will return the value for the current index level.
 *--------------------------------------------------------------------------------*/
unsigned short	level_index_get_index(LEVEL_INDEX* index, unsigned short level)
{
	unsigned short result = 0;

	if (index->level_index != NULL)
	{
		result = index->level_index[level];
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_gen_label
 *  desc: This function will output the label for the current level of the index
 *        it will generate it in the style that is passed in.
 *--------------------------------------------------------------------------------*/
unsigned int	level_index_gen_label(LEVEL_INDEX* index, unsigned int start_level, INDEX_STYLE style, unsigned char* buffer, unsigned int buffer_size ,unsigned char delimiter)
{
	unsigned int count;
	unsigned int result = 0;

	for (count=start_level; count <= index->current_level && result < buffer_size; count++)
	{
		switch (style)
		{
			case INDEX_STYLE_BARE:
				buffer[result++] = ' ';
				break;

			case INDEX_STYLE_NUMBERS:
				result += IntToAlphaSafe(index->level_index[count],&buffer[result],buffer_size-result);
				buffer[result-1] = delimiter;
				break;
		}
	}

	if (result > 0)
	{
		result--;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: level_index_release
 *  desc: This function will release all the items allocated to the level index.
 *--------------------------------------------------------------------------------*/
void	level_index_release(LEVEL_INDEX* index)
{
	if (index->level_index != NULL)
	{
		free(index->level_index);
	}

	index->level_index		= NULL;
	index->max_levels		= 0;
	index->current_level	= 0;
	index->previous_level	= 0;
}

