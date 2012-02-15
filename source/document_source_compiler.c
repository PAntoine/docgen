/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : document_source_compiler
 * Desc  : This file holds the main loop for the document compiler.
 *
 * Author: pantoine
 * Date  : 07/11/2011 08:13:08
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#include "atoms.h"
#include "error_codes.h"
#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * Static String Tables
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

/*--------------------------------------------------------------------------------*
 * Global Settings (with defaults)
 *--------------------------------------------------------------------------------*/
static unsigned char	g_marker = '@';
static unsigned char	g_comment_start		= '/';
static unsigned char	g_comment_start_end	= '*';
static unsigned char	g_comment_end_start	= '*';
static unsigned char	g_comment_end_end	= '/';
static unsigned char	g_function_start	= '{';
static unsigned char	g_function_end		= '}';

/*--------------------------------------------------------------------------------*
 * Global Structures.
 *--------------------------------------------------------------------------------*/
static LOOKUP_LIST	g_group_lookup;
static LOOKUP_LIST	g_macro_lookup;
static LOOKUP_LIST	g_functions;

extern char* g_input_filename;


/*--------------------------------------------------------------------------------*
 * Externals
 *--------------------------------------------------------------------------------*/
extern unsigned char	g_file_header[];
extern unsigned int		g_file_header_size;
extern unsigned int		g_date_offset;
extern unsigned int		g_number_records_offset;
extern unsigned int		g_date_offset;

/*--------------------------------------------------------------------------------*
 * Global Flags
 *--------------------------------------------------------------------------------*/
unsigned char	g_looking_for_comment = 1;
unsigned char	g_looking_for_function = 0;
unsigned int	g_in_function = 0;
unsigned int	g_current_function = INVALID_ITEM;

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_atoms
 * Desc : This function will output a lookup table to the output file.
 *--------------------------------------------------------------------------------*/
void output_atoms (int output_file, ATOM_INDEX* index)
{
	unsigned int	count;
	unsigned char	record[RECORD_DATA_START];
	ATOM_BLOCK*		current = &index->index;
	
	while (current != NULL)
	{
		for (count=0;count<current->num_items;count++)
		{
			record[RECORD_TYPE] = current->index[count].any.type;
			record[RECORD_ATOM] = current->index[count].any.atom;
			record[RECORD_GROUP] = current->index[count].any.function;
			record[RECORD_LINE_NUM  ] = (current->index[count].any.line & 0xff00) >> 8;
			record[RECORD_LINE_NUM+1] = current->index[count].any.line & 0xff;
			record[RECORD_BLOCK_NUM  ] = (current->index[count].any.block & 0xff00) >> 8;
			record[RECORD_BLOCK_NUM+1] = current->index[count].any.block & 0xff;

			current->index[count].any.function |= RECORD_FUNCTION_MASK;
			record[RECORD_GROUP  ] = (current->index[count].any.function & 0xff00) >> 8;
			record[RECORD_GROUP+1] = current->index[count].any.function & 0xff;

			if (current->index[count].any.type == INTERMEDIATE_RECORD_NAME)
			{
				if (current->index[count].any.function == INVALID_ITEM)
				{
					record[RECORD_GROUP  ] = (current->index[count].name.group & 0xff00) >> 8;
					record[RECORD_GROUP+1] = current->index[count].name.group & 0xff;
				}
				record[RECORD_DATA_SIZE  ] = (current->index[count].name.name_length & 0xff00) >> 8;
				record[RECORD_DATA_SIZE+1] = current->index[count].name.name_length & 0xff;

				write(output_file,record,RECORD_DATA_START);
				write(output_file,current->index[count].name.name,current->index[count].name.name_length);
			}
			else if (current->index[count].any.type == INTERMEDIATE_RECORD_STRING)
			{
				record[RECORD_DATA_SIZE  ] = (current->index[count].string.string_length & 0xff00) >> 8;
				record[RECORD_DATA_SIZE+1] = current->index[count].string.string_length & 0xff;

				write(output_file,record,RECORD_DATA_START);
				write(output_file,current->index[count].string.string,current->index[count].string.string_length);
			}
			else
			{
				raise_warning(0,EC_INTERNAL_ERROR_UNKNOWN_RECORD_TYPE,NULL,NULL);
			}
		}

		current = current->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_lookup
 * Desc : This function will output a lookup table to the output file.
 *--------------------------------------------------------------------------------*/
void output_lookup (int output_file, LOOKUP_LIST* lookup_list, unsigned int type)
{
	unsigned int	count;
	unsigned char	record[RECORD_DATA_START];
	LOOKUP_LIST*	current = lookup_list;
	
	while (current != NULL)
	{
		for (count=0;count<current->num_items;count++)
		{
			record[RECORD_TYPE] = type;
			record[RECORD_ATOM] = 0;
			record[RECORD_GROUP] = 0;
			record[RECORD_BLOCK_NUM  ] = 0;
			record[RECORD_BLOCK_NUM+1] = 0;
			record[RECORD_DATA_SIZE  ] = (current->lookup[count].name_length & 0xff00) >> 8;
			record[RECORD_DATA_SIZE+1] = current->lookup[count].name_length & 0xff;
		
			write(output_file,record,RECORD_DATA_START);
			write(output_file,current->lookup[count].name,current->lookup[count].name_length);
		}

		current = current->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_output
 * Desc : This function will create the object file that holds all the atoms
 *        that have been recovered from the input file.
 *--------------------------------------------------------------------------------*/
unsigned int	generate_output(ATOM_INDEX* atom_index, char* filename, char* input_name)
{
	int				outfile;
	unsigned int	in_size;
	unsigned int	result = 0;
	unsigned char	signature[4] = COMPILED_SOURCE_MAGIC;
	time_t			now = time(NULL);
	struct tm*		curr_time = gmtime(&now);
	unsigned char	file_header[FILE_HEADER_SIZE];
	
	if ((outfile = open(filename,WRITE_FILE_STATUS,WRITE_FILE_PERM)) != -1)
	{
		/* ok, we have created the output file */
		file_header[0] = signature[0];
		file_header[1] = signature[1];
		file_header[2] = signature[2];
		file_header[3] = signature[3];
		file_header[4] = VERSION_MAJOR;
		file_header[5] = VERSION_MINOR;

		/* time stamp the file */
		file_header[FILE_DAY_OFF	  ] = (unsigned char) curr_time->tm_mday;
		file_header[FILE_MONTH_OFF	  ] = (unsigned char) curr_time->tm_mon;
		file_header[FILE_YEAR_OFF	  ] = (unsigned char) curr_time->tm_year;
		file_header[FILE_HOUR_OFF	  ] = (unsigned char) curr_time->tm_hour;
		file_header[FILE_MINUTE_OFF   ] = (unsigned char) curr_time->tm_min;
		file_header[FILE_SECONDS_OFF  ] = (unsigned char) curr_time->tm_sec;

		/* put the number of records in the header */
		in_size = g_group_lookup.num_items + atom_index->number_atoms;
		file_header[FILE_NUMBER_RECORDS+0] = (unsigned char)((in_size & 0xff00) >>8);
		file_header[FILE_NUMBER_RECORDS+1] = (unsigned char)(in_size & 0xff);

		/* put the size of the input name in the header */
		in_size = strlen(input_name);
		file_header[FILE_NAME_START+0] = (unsigned char)((in_size & 0xff00) >>8);
		file_header[FILE_NAME_START+1] = (unsigned char)(in_size & 0xff);

		write(outfile,file_header,FILE_HEADER_SIZE);
		write(outfile,input_name,in_size);

		/* dump the groups first */
		output_lookup(outfile,&g_group_lookup,INTERMEDIATE_RECORD_GROUP);

		/* dump the functions next */
		output_lookup(outfile,&g_functions,INTERMEDIATE_RECORD_FUNCTION);

		/* dump the atoms */
		output_atoms (outfile,atom_index);

		result = 1;

		close(outfile);
	}
	else
	{
		raise_warning(0,EC_FAILED_TO_OPEN_OUTPUT_FILE,NULL,NULL);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : init_atoms_index
 * Desc : This function will initialise an ATOMS_index item.
 *--------------------------------------------------------------------------------*/
void init_atoms_index ( ATOM_INDEX* index )
{
	memset(index,0,sizeof(ATOM_INDEX));
	index->last = &index->index;
}

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
 * Name : add_lookup
 * Desc : This function will find a lookup if it exists and if it does not then it
 *        adds it.
 *--------------------------------------------------------------------------------*/
unsigned int add_lookup ( LOOKUP_LIST* lookup_list, char* name, unsigned int name_length, char* payload, unsigned int payload_length )
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
		index_count--;

		if (previous->num_items < LOOKUP_INDEX_SIZE)
		{
			previous->lookup[previous->num_items].hash = hash;
			previous->lookup[previous->num_items].name = malloc(name_length);
			previous->lookup[previous->num_items].name_length = name_length;
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
	
			temp->lookup[temp->num_items].hash = hash;
			temp->lookup[temp->num_items].name = malloc(name_length);
			temp->lookup[temp->num_items].name_length = name_length;
			memcpy(temp->lookup[0].name,name,name_length);
	
			previous->lookup[previous->num_items].payload = malloc(payload_length);
			previous->lookup[previous->num_items].payload_length = payload_length;
			memcpy(previous->lookup[previous->num_items].payload,payload,payload_length);

			previous->num_items = 1;
			
			result = ((index_count + 1) * LOOKUP_INDEX_SIZE);
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
		index_count--;

		if (previous->num_items < LOOKUP_INDEX_SIZE)
		{
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
	
			temp->lookup[temp->num_items].hash = hash;
			temp->lookup[temp->num_items].name = malloc(name_length);
			temp->lookup[temp->num_items].name_length = name_length;
			memcpy(temp->lookup[0].name,name,name_length);
			previous->num_items = 1;
			
			result = ((index_count + 1) * LOOKUP_INDEX_SIZE);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : remove_white_space
 * Desc : This function will remove the whitespace from a string and return the
 *        position of the first non-white char.
 *--------------------------------------------------------------------------------*/
static unsigned int	remove_white_space(unsigned int position, unsigned char* buffer)
{
	unsigned int result = position;

	while(buffer[result] == '\t' || buffer[result] == ' ')
	{
		result++;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : get_name
 * Desc : This function will retrieve a name from the given buffer.
 *--------------------------------------------------------------------------------*/
static unsigned int	get_name(unsigned char* buffer, unsigned char** name)
{
	unsigned char	result = 0;
	unsigned char*	start = buffer;

	start = &buffer[remove_white_space(0,buffer)];

	while(is_valid_char[start[result]])
	{
		result++;
	}

	if (result > 0)
	{
		*name = start;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : get_macro_name
 * Desc : This function will find lookup the macro name, if it is found then it
 *        will return the payload as the name.
 *--------------------------------------------------------------------------------*/
static unsigned int get_macro_name ( unsigned char* buffer, unsigned char** name, unsigned int* name_length )
{
	unsigned int	in_length;
	unsigned int	result = 0;
	unsigned char*	macro_name;
	LOOKUP_ITEM*	macro;

	in_length = get_name(buffer,&macro_name);
	macro = find_lookup(&g_macro_lookup,macro_name,in_length);

	if (macro != NULL)
	{
		*name			= macro->payload;
		*name_length	= macro->payload_length;
		result			= in_length;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_atom
 * Desc : This function will add an atom to the atom index.
 *--------------------------------------------------------------------------------*/
ATOM_ITEM* add_atom ( ATOM_INDEX* list, unsigned int type, ATOM_ATOMS atom )
{
	ATOM_ITEM*	result = NULL;
	
	if (list->last->num_items >= ATOM_BLOCK_SIZE)
	{
		/* need to increase the size of the index */
		list->last->next = calloc(1,ATOM_BLOCK_SIZE);
		list->last = list->last->next;
	}

	result = &list->last->index[list->last->num_items++];
	result->any.type	= type;
	result->any.atom	= atom;
	result->any.block	= list->block_count;
	result->any.line	= list->line_number;

	list->number_atoms++;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_name_atom
 * Desc : This function will add an atom_name item to th
 *--------------------------------------------------------------------------------*/
static unsigned int add_name_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned char* atom_group, unsigned int atom_group_length, unsigned char* atom_name, unsigned int atom_name_length, unsigned int function )
{
	unsigned int	result = EC_OK;
	ATOM_ITEM*		item = add_atom(list,INTERMEDIATE_RECORD_NAME,atom);
	
	if (atom_group_length > 255)
	{
		result = EC_GROUP_NAME_TOO_LONG;
	}
	else if (atom_name_length > 255)
	{
		result = EC_ATOM_NAME_TOO_LONG;
	}
	else
	{
		/* do we have a group */
		if (atom_group != NULL)
		{
			item->name.group = find_add_lookup(&g_group_lookup,atom_group,atom_group_length);
		}
		else
		{
			item->name.group = DEFAULT_GROUP;
		}

		/* set the atoms details */
		item->name.function		= function;
		item->name.name 		= malloc(atom_name_length);
		item->name.name_length	= atom_name_length;
		memcpy(item->name.name,atom_name,atom_name_length);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_string_atom
 * Desc : This function will add the string literal item to the atom index.
 *--------------------------------------------------------------------------------*/
void add_string_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned char* string, unsigned int string_length, unsigned int function )
{
	ATOM_ITEM*	item = add_atom(list,INTERMEDIATE_RECORD_STRING,atom);
	
	item->string.function		= function;
	item->string.string			= malloc(string_length);
	item->string.string_length	= string_length;
	memcpy(item->string.string,string,string_length);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_name
 * Desc : This function will take in a string and find a name within it. It will
 *        handle the MACRO values. i.e. $name and handle the string replacement.
 *
 *        The format of a name must be one of the following:
 *  		
 *  	  alpha_num_string
 *  	  alpha_num_string.alpha_num_string
 *  	  $(alpha_num_string).alpha_num_string
 *--------------------------------------------------------------------------------*/
static unsigned int decode_name(unsigned char* buffer, ATOM_INDEX* atom_list, ATOM_ATOMS atom, unsigned int function)
{
	unsigned int	result = EC_OK;
	unsigned int	buff_pos;
	unsigned int	item_length = 0;
	unsigned int	first_length = 0;
	unsigned char*	first_part = NULL;
	unsigned char*	item_name = NULL;

	if (buffer[0] == '$')
	{
		if ((buff_pos = get_macro_name(&buffer[1],&first_part,&first_length)))
		{
			/* macros must be a atom_group and precede an atom_item */
			if (buffer[buff_pos + 1] == '.' && (item_length = get_name(&buffer[buff_pos + 2],&item_name)))
			{
				if (atom == ATOM_STATE || atom == ATOM_TRIGGERS || atom == ATOM_TRIGGER || atom == ATOM_TIMELINE)
				{
					if ((result = add_name_atom(atom_list,atom,first_part,first_length,item_name,item_length,function)) != EC_OK)
					{
						raise_warning(atom_list->line_number,result,NULL,NULL);
					}
				}
				else
				{
					result = EC_ATOM_NOT_ALLOWED_TO_HAVE_GROUP;
					raise_warning(atom_list->line_number,result,NULL,NULL);
				}
			}
			else
			{
				result = EC_MACRO_NOT_GROUP;
				raise_warning(atom_list->line_number,result,NULL,NULL);
			}
		}
		else
		{
			result = EC_UNKNOWN_MACRO;
			raise_warning(atom_list->line_number,EC_UNKNOWN_MACRO,NULL,NULL);
		}
	}
	else if ((first_length = get_name(buffer,&first_part)))
	{
		if (buffer[first_length] == '.')
		{
			if ((item_length = get_name(&buffer[first_length + 1],&item_name)))
			{
				if (atom == ATOM_STATE || atom == ATOM_TRIGGERS || atom == ATOM_TRIGGER || atom == ATOM_TIMELINE)
				{
					if ((result = add_name_atom(atom_list,atom,first_part,first_length,item_name,item_length,function)) != EC_OK)
					{
						raise_warning(atom_list->line_number,result,NULL,NULL);
					}
				}
				else
				{
					result = EC_ATOM_NOT_ALLOWED_TO_HAVE_GROUP;
					raise_warning(atom_list->line_number,result,NULL,NULL);
				}
			}
			else
			{
				result = EC_INVALID_ITEM_NAME;
				raise_warning(atom_list->line_number,EC_INVALID_ITEM_NAME,NULL,NULL);
			}
		}
		else
		{
			/* add a simple item - no atom_group */
			if ((result = add_name_atom(atom_list,atom,NULL,0,first_part,first_length,function)) != EC_OK)
			{
				raise_warning(atom_list->line_number,result,NULL,NULL);
			}
		}
	}
	else
	{
		result = EC_INVALID_NAME_OR_NAME_NOT_FOUND;
		raise_warning(atom_list->line_number,EC_INVALID_NAME_OR_NAME_NOT_FOUND,NULL,NULL);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_string
 * Desc : This function will decode a string atom and add it to the atom index.
 *        Actually there is not much to do, as the string will be all data on the
 *        line to the end, so just add it to the atom list.
 *--------------------------------------------------------------------------------*/
void decode_string (unsigned char* buffer, unsigned int buffer_length, ATOM_INDEX* atom_list, ATOM_ATOMS atom, unsigned int function )
{
	unsigned int count;

	/* trim any unsightly characters */
	for (count=buffer_length-1;count>0 && buffer[count] < 0x0f ;count--)
	{
	}

	buffer_length = count+1;

	add_string_atom(atom_list,atom,buffer,buffer_length,function);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : parse_line
 * Desc : This function will parse the line and extract any atoms from it and add
 *        them to the atom list.
 *--------------------------------------------------------------------------------*/
static unsigned int parse_line ( unsigned char* line, unsigned int line_length, ATOM_INDEX* atom_list )
{
	unsigned int pos = 0;
	unsigned int atom = -1;
	unsigned int err = EC_OK;
	unsigned int result = EC_OK;

	/* must be more then one char before the end of the line */
	while(pos < line_length+1)
	{
		if (g_looking_for_comment)
		{
			if (line[pos] == g_comment_start && line[pos+1] == g_comment_start_end && line[pos+2] == g_comment_start_end)
			{
				/* we have found the comment we are looking for */
				g_looking_for_comment = 0;
				atom_list->block_count++;
			}
			else if (g_looking_for_function)
			{
				if (line[pos] == g_function_start)
				{
					/* ok, we have a function start marker */
					g_in_function++;
				}
				else if (line[pos] == g_function_end)
				{
					if (g_in_function > 0)
					{
						g_in_function--;

						if (g_in_function == 0)
						{
							g_looking_for_function = 0;
							g_current_function = INVALID_ITEM;
						}
					}
				}
			}
		}
		else if (line[pos] == g_marker)
		{
			if (line[pos+1] == g_marker)
			{
				/* g_marker escapes itself */
				pos++;
			}
			else if ((atom = atoms_check_word(&line[pos+1])) != -1)
			{
				pos += atoms_get_length(atom) + 1;

				if (line[pos] == '-')
				{
					atom_list->multiline = 1;
				}
				else if (line[pos] != ':')
				{
					result = EC_WEIRD_STUFF_AFTER_COMMAND;
					raise_warning(atom_list->line_number,EC_WEIRD_STUFF_AFTER_COMMAND,NULL,NULL);
					break;
				}

				pos++;

				pos = remove_white_space(pos,line);

				/* ok, we have a good start char */
				switch(atom)
				{
					/* handle atoms that take a name */
					case ATOM_NEXT:
					case ATOM_STATE:
					case ATOM_TIMELINE:
					case ATOM_ACTIVATION:
						if (g_looking_for_function)
						{
							result = EC_CANNOT_CALL_HAVE_THESE_ATOMS_IN_A_FUNCTION;
							raise_warning(atom_list->line_number,result,NULL,NULL);
						}
						else if ((err = decode_name(&line[pos],atom_list,atom,INVALID_ITEM)) != EC_OK)
						{
							result = err;
						}
						break;

					case ATOM_TO:
					case ATOM_CALL:
					case ATOM_SEND:
					case ATOM_AFTER:
					case ATOM_WAITFOR:
					case ATOM_TRIGGER:
					case ATOM_TRIGGERS:
					case ATOM_RESPONDS:
						if ((err = decode_name(&line[pos],atom_list,atom,g_current_function)) != EC_OK)
						{
							result = err;
						}
						break;

						/* function atom, starts a function lookup and a creates a function */
					case ATOM_FUNCTION:
						if (g_looking_for_function)
						{
							result = EC_MULTIPLE_FUNCTION_ATOMS_WITHOUT_FUNCTION;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else if (g_in_function)
						{
							result = EC_NESTED_FUNCTION_DEFINITIONS_NOT_ALLOWED;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else if (find_lookup(&g_functions,&line[pos],line_length-pos) != NULL)
						{
							/* multiple definition of function has been found */
							result = EC_MULTIPLE_DEFINITIONS_OF_FUNCTION;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else
						{
							g_current_function = add_lookup(&g_functions,(char*)&line[pos],line_length-pos,NULL,0);
							g_looking_for_function = 1;
						}
						break;
					
					case ATOM_PARAMETER:
						if (g_looking_for_function)
						{
							result = EC_CANNOT_CALL_HAVE_THESE_ATOMS_IN_A_FUNCTION;
							raise_warning(atom_list->line_number,result,NULL,NULL);
						}
						else 
						{
							decode_string(&line[pos],line_length-pos,atom_list,atom,g_current_function);
						}
						break;

						/* string atoms */
					case ATOM_REPEATS:
					case ATOM_CONDITION:
						decode_string(&line[pos],line_length-pos,atom_list,atom,g_current_function);
						break;

					default:	
						result = EC_UNKNOWN_COMMAND;
						raise_warning(atom_list->line_number,EC_UNKNOWN_COMMAND,NULL,NULL);
						break;
				}
				break;
			}
			else
			{
				/* The atom was not found --- raise an error */
				result = EC_UNKNOWN_COMMAND;
				raise_warning(atom_list->line_number,EC_UNKNOWN_COMMAND,NULL,NULL);
				break;
			}
		}
		else if (line[pos] == g_comment_end_start && line[pos+1] == g_comment_end_end)
		{
			g_looking_for_comment = 1;
			break;
		}

		pos++;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : main
 * Desc : This is the main function.
 *--------------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int				quiet = 0;
	int				failed = 0;
	int				verbose = 0;
	int				exit_code = 0;
	int				buffer_size = 100;
	int				have_filename = 0;
	char*			equals = NULL;
	char*			infile_name = NULL;
	char*			error_param = "";
	char*			error_string = "";
	char*			output_filename = "doc.pdso";
	unsigned int	start = 1;
	unsigned int	result = 0;
	unsigned int	linesize = 0;
	unsigned char*	line_buffer;
	FILE*			input_file;
	ATOM_INDEX		raw_atoms;

	memset(&g_group_lookup,0, sizeof(g_group_lookup));
	memset(&g_macro_lookup,0, sizeof(g_macro_lookup));
	memset(&g_functions,0, sizeof(g_functions));

	raw_atoms.line_number = 0;

	if (argc < 2)
	{
		failed = 1;
	}
	else
	{
		do
		{
			if (argv[start][0] == '-')
			{
				switch (argv[start][1])
				{
					case 'v':	/* verbose - add extra comments to the output */
						verbose = 1;
						break;

					case 'q':	/* quiet - suppress non error outputs */
						quiet = 1;
						break;

					case 'o':	/* output file */
						if (argv[start][2] != '\0')
						{
							output_filename = &argv[start][2];
						}
						else if (((start + 1) < argc) && argv[start+1][0] != '-')
						{
							start++;
							output_filename = argv[start];
						}
						else
						{
							error_string = "-o requires a file name\n";
							failed = 1;
						}
						break;

					case 'D':	/* macro definition */
						if (argv[start][2] != '\0')
						{
							if ((equals = index(&argv[start][2],'=')) != NULL)
							{
								if (equals[1] != '\0')
								{
									add_lookup(&g_macro_lookup,&argv[start][2],equals-&argv[start][2],&equals[1],strlen(&equals[1]));
								}
								else if (argv[start+1][0] != '-')
								{
									add_lookup(&g_macro_lookup,&argv[start][2],equals-&argv[start][2],&argv[start+1][0],strlen(&argv[start+1][0]));
								}
								else
								{
									failed = 1;
									error_string = "bad format for -D";
								}
							}
							else if (argv[start+1][0] == '=')
							{
								if ((argv[start+1][1] == '\0') && argv[start+2][0] != '-')
								{
									add_lookup(&g_macro_lookup,argv[start],strlen(argv[start]),argv[start+2],strlen(argv[start+2]));
								}
								else if ((argv[start+1][0] == '=') && (argv[start+1][1] != '\0'))
								{
									add_lookup(&g_macro_lookup,argv[start],strlen(argv[start]),&argv[start+1][1],strlen(&argv[start+1][1]));
								}
								else
								{
									failed = 1;
									error_string = "bad format for -D";
								}
							}
						}
						else if (argv[start+1][0] != '-')
						{
							if ((argv[start+2][0] == '=') && (argv[start+2][1] == '\0') && argv[start+3][0] != '-')
							{
								add_lookup(&g_macro_lookup,argv[start+1],strlen(argv[start+1]),argv[start+3],strlen(argv[start+3]));
							}
							else if ((argv[start+2][0] == '=') && (argv[start+2][1] != '\0'))
							{
								add_lookup(&g_macro_lookup,argv[start+1],strlen(argv[start+1]),&argv[start+2][1],strlen(&argv[start+2][1]));
							}
							else
							{
								failed = 1;
								error_string = "bad format for -D";
							}
						}
						else
						{
							error_string = "-D requires a macro name\n";
							failed = 1;
						}
						break;


					case '?':	/* help  - just fail! */
						failed = 1;
						break;

					default:
						failed = 1;
						error_string = "Unknown parameter\n";
				}		

			}else{
				/* the input file is just a position file */
				if (have_filename)
				{
					error_string = "too many input file names.\n";
					failed = 1;
				}else{
					infile_name = argv[start];
					g_input_filename = infile_name;
					have_filename = 1;
				}
			}

			start++;
		}
		while(start < argc);
	}

	if (infile_name == NULL)
	{
		error_string = "must have a input file";
		failed = 1;
	}

	if (verbose)
	{
		printf("version header should be printed here\n");
	}

	if (failed)
	{
		printf(error_string,error_param);
		exit_code = 1;
	}
	else
	{
		if ((input_file = fopen(infile_name,"r")) == NULL)
		{
			printf("Failed to open file: %s \n",infile_name);
			exit_code = 1;
		}
		else
		{
			exit_code = EXIT_FAILURE;
			
			line_buffer = malloc(buffer_size);

			init_atoms_index(&raw_atoms);

			/* add a holder for the default group */
			find_add_lookup(&g_group_lookup,(unsigned char*)"",0);

			while((linesize = getline((char**)&line_buffer,&buffer_size,input_file)) != -1)
			{
				raw_atoms.line_number++;
	
				if (linesize > 1)
				{
					result &= parse_line(line_buffer,linesize-1,&raw_atoms);
				}
			}
			free(line_buffer);

			if (result == 0)
			{
				if (generate_output(&raw_atoms,output_filename,infile_name))
				{
					exit_code = EXIT_SUCCESS;
				}
			}
			fclose(input_file);
		}
	}

	exit(exit_code);
}

