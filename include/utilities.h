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

unsigned int	fnv_32_hash ( unsigned char *string, unsigned int length );
unsigned int	SizeToString(unsigned char* string,unsigned int size, unsigned int max_length);
void			hex_dump(unsigned char* memory, unsigned int memory_size);
unsigned int	strcpycnt( unsigned char* dest, unsigned char* src, unsigned int max);

#endif

