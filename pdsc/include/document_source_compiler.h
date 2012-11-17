/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : document_source_compiler
 * Desc  : This is the generic header file.
 *
 * Author: peterantoine
 * Date  : 03/11/2012 16:37:14
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __DOCUMENT_SOURCE_COMPILER_H__
#define __DOCUMENT_SOURCE_COMPILER_H__

void add_pair_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned char* name, unsigned int name_length, unsigned char* string, unsigned int string_length);
void add_api_end_atom (ATOM_INDEX* list, unsigned short group_id);


#endif

