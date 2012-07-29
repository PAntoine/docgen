/**-------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : document_source_compiler
 * Desc  : This file holds the main loop for the document compiler.
 *
 * Author: pantoine
 * Date  : 07/11/2011 08:13:08
 *
 * @application	pdsc
 *
 * @section		Description
 * Document Source Compiler
 *
 * This function will search the given source files to locate the special markup
 * symbols that are specified within comment blocks. The symbols are specified 
 * by special tokens that start with the *at* symbol.
 *
 * This file will generate a file that has all the data that has been collected
 * from the source file.
 *
 * This object file generated should be linked together with the *pdsl* application
 * and then documents can be generated from the system using the *pdp* application.
 *
 * The purpose of this system it to allow for generic documentation to be create
 * from the build, and tailored to a specific build configuration.
 * 
 * @synopsis	all		-v, -q, -o, -D, input_file
 *
 * @section		Error Reporting
 * 				Please report any problems to https://github.com/PAntoine/docgen
 *
 * @section		Author 
 * 				Peter Antoine
 *
 * @ignore
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
#include "input_formats.h"
#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * Global Settings (with defaults)
 *--------------------------------------------------------------------------------*/
static unsigned char	g_marker = '@';

/*--------------------------------------------------------------------------------*
 * Global Structures.
 *--------------------------------------------------------------------------------*/
static LOOKUP_LIST	g_group_lookup;
static LOOKUP_LIST	g_macro_lookup;
static LOOKUP_LIST	g_functions;
static LOOKUP_LIST	g_apis;
static LOOKUP_LIST	g_samples;
static LOOKUP_LIST	g_applications;

extern unsigned char is_valid_char[];

extern char* g_input_filename;

extern SOURCE_FORMAT	input_formats[];

/*--------------------------------------------------------------------------------*
 * Externals
 *--------------------------------------------------------------------------------*/
extern unsigned char	g_file_header[];
extern unsigned int		g_file_header_size;
extern unsigned int		g_date_offset;
extern unsigned int		g_number_records_offset;
extern unsigned int		g_date_offset;

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_string
 * Desc : This function will write a string to the output file.
 *--------------------------------------------------------------------------------*/
void	write_string(int output_file, NAME* string)
{
	unsigned char	length[2];

	if (string != NULL)
	{
		length[0] = (string->name_length & 0xff00) >> 8;
		length[1] = (string->name_length & 0x00ff);
	}
	else
	{
		length[0] = 0;
		length[1] = 0;
	}

	write(output_file,length,2);
	
	if (string != NULL && string->name_length > 0)
	{
		write(output_file,string->name,string->name_length);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_atoms
 * Desc : This function will output a lookup table to the output file.
 *--------------------------------------------------------------------------------*/
void output_atoms (int output_file, ATOM_INDEX* index)
{
	unsigned int	size;
	unsigned int	count;
	unsigned int	data_size;
	unsigned char	length[2];
	unsigned char	record[RECORD_DATA_START+1];
	ATOM_BLOCK*		current = &index->index;
	
	while (current != NULL)
	{
		for (count=0;count<current->num_items;count++)
		{
			/* output the common fields */
			record[RECORD_TYPE] = current->index[count].any.type;
			record[RECORD_ATOM] = current->index[count].any.atom;
			record[RECORD_GROUP  ] = (current->index[count].any.func_api & 0xff00) >> 8;
			record[RECORD_GROUP+1] = current->index[count].any.func_api & 0xff;
			record[RECORD_LINE_NUM  ] = (current->index[count].any.line & 0xff00) >> 8;
			record[RECORD_LINE_NUM+1] = current->index[count].any.line & 0xff;
			record[RECORD_BLOCK_NUM  ] = (current->index[count].any.block & 0xff00) >> 8;
			record[RECORD_BLOCK_NUM+1] = current->index[count].any.block & 0xff;

			/* now write the records to the file */
			switch(current->index[count].any.type)
			{
				case INTERMEDIATE_RECORD_NAME:

					if (current->index[count].any.func_api == INVALID_ITEM)
					{
						record[RECORD_GROUP  ] = (current->index[count].name.group & 0xff00) >> 8;
						record[RECORD_GROUP+1] = current->index[count].name.group & 0xff;
					}
					record[RECORD_DATA_SIZE  ] = ((current->index[count].name.name_length+1) & 0xff00) >> 8;
					record[RECORD_DATA_SIZE+1] = (current->index[count].name.name_length+1) & 0xff;

					record[RECORD_DATA_START ] = 0;					/* fixed flag that is not used for strings */
					write(output_file,record,RECORD_DATA_START+1);
					write(output_file,current->index[count].name.name,current->index[count].name.name_length);
				break;

				case INTERMEDIATE_RECORD_PAIR:
				/* set data length and write record header */
					data_size = 2 + current->index[count].pair.name_length + 2 + current->index[count].pair.string_length;
					record[RECORD_DATA_SIZE  ] = (data_size & 0xff00) >> 8;
					record[RECORD_DATA_SIZE+1] = data_size & 0xff;
					write(output_file,record,RECORD_DATA_START);
					
					/* write name */
					length[0] = (current->index[count].pair.name_length & 0x0ff00) >> 8;
					length[1] = (current->index[count].pair.name_length & 0x000ff);
					write(output_file,length,2);
					write(output_file,current->index[count].pair.name,current->index[count].pair.name_length);
	
					/* write string */
					length[0] = (current->index[count].pair.string_length & 0x0ff00) >> 8;
					length[1] = (current->index[count].pair.string_length & 0x000ff);
					write(output_file,length,2);
					write(output_file,current->index[count].pair.string,current->index[count].pair.string_length);
					break;
				
				case INTERMEDIATE_RECORD_STRING:
				case INTERMEDIATE_RECORD_MULTILINE:

					record[RECORD_DATA_SIZE  ] = ((current->index[count].string.string_length+1) & 0xff00) >> 8;
					record[RECORD_DATA_SIZE+1] = (current->index[count].string.string_length+1) & 0xff;
					record[RECORD_DATA_START ] = current->index[count].string.fixed;

					write(output_file,record,RECORD_DATA_START+1);
					write(output_file,current->index[count].string.string,current->index[count].string.string_length);
					break;

				case INTERMEDIATE_RECORD_NUMBERIC:
					record[RECORD_DATA_SIZE  ] = 0;
					record[RECORD_DATA_SIZE+1] = 4;

					write(output_file,record,RECORD_DATA_START);
					write(output_file,current->index[count].number.number,4);
				break;

				case INTERMEDIATE_RECORD_END:
				case INTERMEDIATE_RECORD_START:
				case INTERMEDIATE_RECORD_EMPTY:
					record[RECORD_DATA_SIZE  ] = 0;
					record[RECORD_DATA_SIZE+1] = 0;
					write(output_file,record,RECORD_DATA_START);
					break;

				case INTERMEDIATE_RECORD_TYPE:
					size =	current->index[count].type.name.name_length +
							current->index[count].type.type_type.name_length +
							current->index[count].type.description.name_length + 6;

					record[RECORD_DATA_SIZE  ] = (size & 0xff00) >> 8;
					record[RECORD_DATA_SIZE+1] = (size & 0x00ff);
					write(output_file,record,RECORD_DATA_START);
	
					write_string(output_file,&current->index[count].type.type_type);
					write_string(output_file,&current->index[count].type.name);
					write_string(output_file,&current->index[count].type.description);
					break;

				case INTERMEDIATE_RECORD_BOOLEAN:
					record[RECORD_DATA_SIZE  ] = 0;
					record[RECORD_DATA_SIZE+1] = 1;
					record[RECORD_DATA_START]  = current->index[count].boolean.true_false;
					write(output_file,record,RECORD_DATA_START+1);
					break;

				default:
					raise_warning(0,EC_INTERNAL_ERROR_UNKNOWN_RECORD_TYPE,NULL,NULL);
			}
		}

		current = current->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_type
 * Desc : This function will output a type to the file.
 *--------------------------------------------------------------------------------*/
void	output_type(int output_file, ATOM_ATOMS atom_type, unsigned short api_id, unsigned short line_num, NAME* return_type, NAME* name, NAME* brief)
{
	unsigned char	empty[] = {0x00,0x00};
	unsigned char	length[2];
	unsigned char	record[RECORD_DATA_START];
	unsigned short	data_size = 0;
	unsigned short	name_size = 0;
	unsigned short	brief_size = 0;
	unsigned short	return_size = 0;

	if (return_type != NULL)
	{
		return_size = return_type->name_length;
	}

	if (name != NULL)
	{
		name_size = name->name_length;
	}

	if (brief != NULL)
	{
		brief_size = brief->name_length;
	}
	
	data_size = 2 + return_size + 2 + name_size + 2 + brief_size;

	/* generic header for the function output */
	record[RECORD_TYPE] = INTERMEDIATE_RECORD_TYPE;
	record[RECORD_ATOM] = atom_type;
	record[RECORD_GROUP  ] = (api_id & 0xff00) >> 8;
	record[RECORD_GROUP+1] = api_id & 0xff;
	record[RECORD_LINE_NUM  ] = (line_num & 0xff00) >> 8;
	record[RECORD_LINE_NUM+1] = line_num & 0xff;
	record[RECORD_BLOCK_NUM  ] = 0;
	record[RECORD_BLOCK_NUM+1] = 0;
	record[RECORD_DATA_SIZE  ] = (data_size & 0xff00) >> 8;
	record[RECORD_DATA_SIZE+1] = data_size & 0xff;

	write(output_file,record,RECORD_DATA_START);

	write_string(output_file,return_type);
	write_string(output_file,name);
	write_string(output_file,brief);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_empty
 * Desc : This function will output the empty record.
 *--------------------------------------------------------------------------------*/
void	output_empty(int output_file, ATOM_ATOMS atom)
{
	unsigned char	record[RECORD_DATA_START];

	/* generic header for the function output */
	record[RECORD_TYPE] = INTERMEDIATE_RECORD_EMPTY;
	record[RECORD_ATOM] = atom;
	record[RECORD_GROUP  ] = 0;
	record[RECORD_GROUP+1] = 0;
	record[RECORD_LINE_NUM  ] = 0;
	record[RECORD_LINE_NUM+1] = 0;
	record[RECORD_BLOCK_NUM  ] = 0;
	record[RECORD_BLOCK_NUM+1] = 0;
	record[RECORD_DATA_SIZE  ] = 0;
	record[RECORD_DATA_SIZE+1] = 0;

	write(output_file,record,RECORD_DATA_START);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_api_function
 * Desc : This function outputs the contents of the api function to the file.
 *--------------------------------------------------------------------------------*/
void	output_api_function(int output_file, API_FUNCTION* function, unsigned short group_id, unsigned short line_num)
{
	API_PARAMETER*	current_parameter = function->parameter_list;

	output_type(output_file,ATOM_API,function->api_id,line_num,&function->return_type,&function->name,NULL);

	while (current_parameter != NULL)
	{
		/* output the parameter */
		output_type(output_file,ATOM_PARAMETER,function->api_id,line_num,&current_parameter->type,&current_parameter->name,&current_parameter->brief);

		current_parameter = current_parameter->next;
	}

	output_empty(output_file,ATOM_API);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_lookup
 * Desc : This function will output a lookup table to the output file.
 *--------------------------------------------------------------------------------*/
void output_lookup (int output_file, LOOKUP_LIST* lookup_list, unsigned int type)
{
	unsigned int	count;
	unsigned int	data_size;
	unsigned char	record[RECORD_DATA_START];
	LOOKUP_LIST*	current = lookup_list;
	
	while (current != NULL)
	{
		for (count=0;count<current->num_items;count++)
		{
			record[RECORD_TYPE] = type;
			record[RECORD_ATOM] = 0;
			record[RECORD_GROUP      ] = (current->lookup[count].group_id & 0xff00) >> 8;
			record[RECORD_GROUP+1    ] = current->lookup[count].group_id & 0xff;
			record[RECORD_BLOCK_NUM  ] = 0;
			record[RECORD_BLOCK_NUM+1] = 0;
			record[RECORD_LINE_NUM   ] = (current->lookup[count].line_num & 0xff00) >> 8;
			record[RECORD_LINE_NUM+1 ] = current->lookup[count].line_num & 0xff;
		
			if (type == INTERMEDIATE_RECORD_SAMPLE)
			{
				/* write the header */
				data_size = current->lookup[count].name_length + current->lookup[count].payload_length + 4;
	
				record[RECORD_DATA_SIZE  ] = (data_size & 0xff00) >> 8;
				record[RECORD_DATA_SIZE+1] = data_size & 0xff;

				write(output_file,record,RECORD_DATA_START);

				/* write the data sizes */
				record[0] = (current->lookup[count].name_length & 0xff00) >> 8;
				record[1] = current->lookup[count].name_length & 0xff;
				record[2] = (current->lookup[count].payload_length & 0xff00) >> 8;
				record[3] = current->lookup[count].payload_length & 0xff;
				write(output_file,record,4);

				/* write the payload */	
				write(output_file,current->lookup[count].name,current->lookup[count].name_length);
				write(output_file,current->lookup[count].payload,current->lookup[count].payload_length);
			}
			else
			{
				/* write all other records */
				data_size = current->lookup[count].name_length;
				
				record[RECORD_DATA_SIZE  ] = (data_size & 0xff00) >> 8;
				record[RECORD_DATA_SIZE+1] = data_size & 0xff;

				write(output_file,record,RECORD_DATA_START);
				write(output_file,current->lookup[count].name,current->lookup[count].name_length);

				if (type == INTERMEDIATE_RECORD_API && current->lookup[count].payload != NULL)
				{
					output_api_function(output_file,(API_FUNCTION*)current->lookup[count].payload,current->lookup[count].group_id,current->lookup[count].line_num);
				}
			}
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

		/* dump the apis next */
		output_lookup(outfile,&g_apis,INTERMEDIATE_RECORD_API);

		/* dump the apis next */
		output_lookup(outfile,&g_samples,INTERMEDIATE_RECORD_SAMPLE);
		
		/* dump the apis next */
		output_lookup(outfile,&g_applications,INTERMEDIATE_RECORD_APPLICATION);

		/* dump the atoms */
		output_atoms (outfile,atom_index);

		result = 1;

		close(outfile);
	}
	else
	{
		raise_warning(0,EC_FAILED_TO_OPEN_OUTPUT_FILE,(unsigned char*)filename,NULL);
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
unsigned int add_lookup ( LOOKUP_LIST* lookup_list, char* name, unsigned int name_length, char* payload, unsigned int payload_length, unsigned short line_num )
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

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_parameter_brief
 * Desc : This function will add a parameter to the API. It will add the parameter
 *        name if the parameter does not exist. It will return true if it had to
 *        add the parameter. It will add the description to the parameter if it 
 *        had to add it or not.
 *--------------------------------------------------------------------------------*/
unsigned int	add_parameter_brief(API_FUNCTION* function, NAME* parameter_name, NAME* brief)
{
	unsigned int 	result = EC_INVALID_PARAMETER;
	API_PARAMETER*	new_parameter;
	API_PARAMETER*	current_parameter;
	API_PARAMETER*	previous_parameter;
	
	if (function != NULL)
	{
		current_parameter = function->parameter_list;
		previous_parameter = current_parameter;

		while(current_parameter != NULL)
		{
			if(	parameter_name->name_length == current_parameter->name.name_length && 
				memcmp(current_parameter->name.name,parameter_name->name,parameter_name->name_length) == 0)
			{
				break;
			}

			previous_parameter = current_parameter;
			current_parameter = current_parameter->next;
		}

		if (current_parameter == NULL)
		{
			new_parameter = calloc(1,sizeof(API_PARAMETER));
			
			new_parameter->name.name = malloc(parameter_name->name_length);
			memcpy(new_parameter->name.name,parameter_name->name,parameter_name->name_length);
			new_parameter->name.name_length = parameter_name->name_length;
	
			if (previous_parameter == NULL)
			{
				function->parameter_list = new_parameter;
				current_parameter = new_parameter;
			}
			else
			{
				previous_parameter->next = new_parameter;
				current_parameter = new_parameter;
			}

			/* Ok, did not find -- need to add */
			result = EC_PARAMETER_ADDED;
		}

		/* add the description to the found/created parameter */
		if (current_parameter->brief.name_length > 0)
		{
			result = EC_DUPLICATE_FIELD_IN_DEFINITION;
			raise_warning(0,result,function->name.name,NULL);
		}
		else
		{
			memcpy(current_parameter->brief.name,brief->name,brief->name_length);
			current_parameter->brief.name_length = brief->name_length;
			result = EC_OK;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_parameter_type
 * Desc : This function will add a parameter to the API. It will add the parameter
 *        name if the parameter does not exist. It will return true if it had to
 *        add the parameter.
 *--------------------------------------------------------------------------------*/
unsigned int	add_parameter_type(API_FUNCTION* function, NAME* parameter_name, NAME* type)
{
	unsigned int 	result = EC_INVALID_PARAMETER;
	API_PARAMETER*	new_parameter;
	API_PARAMETER*	current_parameter;
	API_PARAMETER*	previous_parameter;
	
	if (function != NULL)
	{
		current_parameter = function->parameter_list;
		previous_parameter = current_parameter;

		while(current_parameter != NULL)
		{
			if(	parameter_name->name_length == current_parameter->name.name_length && 
				memcmp(current_parameter->name.name,parameter_name->name,parameter_name->name_length) == 0)
			{
				break;
			}

			previous_parameter = current_parameter;
			current_parameter = current_parameter->next;
		}

		if (current_parameter == NULL)
		{
			new_parameter = calloc(1,sizeof(API_PARAMETER));
			
			new_parameter->name.name = malloc(parameter_name->name_length);
			memcpy(new_parameter->name.name,parameter_name->name,parameter_name->name_length);
			new_parameter->name.name_length = parameter_name->name_length;
	
			if (previous_parameter == NULL)
			{
				function->parameter_list = new_parameter;
				current_parameter = new_parameter;
			}
			else
			{
				previous_parameter->next = new_parameter;
				current_parameter = new_parameter;
			}

			/* Ok, did not find -- need to add */
			result = EC_PARAMETER_ADDED;
		}

		if (current_parameter->type.name_length > 0)
		{
			result = EC_DUPLICATE_FIELD_IN_DEFINITION;
			raise_warning(0,result,function->name.name,NULL);
		}
		else
		{
			current_parameter->type.name = malloc(type->name_length);
			memcpy(current_parameter->type.name,type->name,type->name_length);
			current_parameter->type.name_length = type->name_length;
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
		list->last->next = calloc(1,sizeof(ATOM_BLOCK));
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
 * Desc : This function will add an atom_name item to the atom index.
 *--------------------------------------------------------------------------------*/
static unsigned int add_name_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned char* atom_group, unsigned int atom_group_length, unsigned char* atom_name, unsigned int atom_name_length, unsigned int func_api )
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
		item->name.func_api		= func_api;
		item->name.name 		= malloc(atom_name_length);
		item->name.name_length	= atom_name_length;
		memcpy(item->name.name,atom_name,atom_name_length);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_boolean_atom
 * Desc : This function will add the string literal item to the atom index.
 *--------------------------------------------------------------------------------*/
void add_boolean_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned int true_false)
{
	ATOM_ITEM*	item = add_atom(list,INTERMEDIATE_RECORD_BOOLEAN,atom);
	
	item->boolean.true_false	= true_false;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_string_atom
 * Desc : This function will add the string literal item to the atom index.
 *--------------------------------------------------------------------------------*/
void add_string_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned char* string, unsigned int string_length, unsigned int func_api )
{
	ATOM_ITEM*	item = add_atom(list,INTERMEDIATE_RECORD_STRING,atom);
	
	item->string.func_api		= func_api;
	item->string.string			= malloc(string_length);
	item->string.string_length	= string_length;
	item->string.fixed			= 0;
	memcpy(item->string.string,string,string_length);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_start_atom
 * desc : This function adds the api start atom to the atom_list.
 *--------------------------------------------------------------------------------*/
void add_api_start_atom (ATOM_INDEX* list, unsigned char record_group_type, unsigned int group_id)
{
	ATOM_ITEM*	item = add_atom(list,INTERMEDIATE_RECORD_START,record_group_type);
	item->any.func_api	= group_id;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_type_atom
 * Desc : This function will add the api type atom. This will add the type and
 *        the name of the type to the atom list. 
 *
 *        The structure of a type record is:
 *        string	type
 *        string	name of the type
 *        string	description of the type.
 *--------------------------------------------------------------------------------*/
void add_api_type_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, NAME* type, NAME* name, NAME* description)
{
	ATOM_ITEM*	item = add_atom(list,INTERMEDIATE_RECORD_TYPE,atom);

	copy_name(name,&item->type.name);
	copy_name(type,&item->type.type_type);
	copy_name(description,&item->type.description);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_api_end_atom
 * desc : This function adds the api end atom to the atom_list.
 *--------------------------------------------------------------------------------*/
void add_api_end_atom (ATOM_INDEX* list, unsigned short group_id)
{
	ATOM_ITEM*	item = add_atom(list,INTERMEDIATE_RECORD_END,0);
	item->any.func_api = group_id;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_multiline_atom
 * Desc : This function will add the multiline atom, it will trim the white space
 *        from the front and the end of the string.
 *--------------------------------------------------------------------------------*/
unsigned int	add_multiline_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned char* string, unsigned int string_length, unsigned int input_type, unsigned int* end_comment, unsigned int dont_trim)
{
	unsigned int	size = 0;
	unsigned int	count;
	unsigned int	found = dont_trim;
	unsigned int	end = string_length;
	unsigned int	start = 0;
	ATOM_ITEM*		item = add_atom(list,INTERMEDIATE_RECORD_MULTILINE,atom);
	
	item->string.fixed = dont_trim;

	for (count=0;count<string_length;count++)
	{
		if (found == 0 && (string[count] < 0x0f || string[count] == ' ' || string[count] == '\t'))
		{
			start++;
		}
		else if (input_formats[input_type].end_comment(string,string_length,&count))
		{
			*end_comment = 1;
			size = count - start - input_formats[input_type].end_of_line_comment_size;
			end = count;
			break;
		}
		else if (string[count] == '@' && string[count+1] != '@')
		{
			/* we found and atom signature or the end of comment, end the search and assume this is the end of the string */
			size = count - start;
			end = count;
			break;
		}
		else
		{
			size++;
			found = 1;
		}
	}

	size += dont_trim;

	item->string.string			= malloc(size);
	item->string.string_length	= size;

	memcpy(item->string.string,&string[start],size);

	if (dont_trim)
	{
		/* need to add a linefeed to the end as this is fixed format */
		item->string.string[size-1] = 0x0a;
	}

	return end;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_number_atom
 * Desc : This function will add the numeric value to the atom list.
 *--------------------------------------------------------------------------------*/
void add_number_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned int number, unsigned int func_api )
{
	ATOM_ITEM*	item = add_atom(list,INTERMEDIATE_RECORD_NUMBERIC,atom);
	
	item->number.func_api		= func_api;
	
	item->number.number[0] = (number >> 24) & 0xff;
	item->number.number[1] = (number >> 16) & 0xff;
	item->number.number[2] = (number >>  8) & 0xff;
	item->number.number[3] = (number      ) & 0xff;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : add_pair_atom
 * Desc : This function will add the atom pair.
 *--------------------------------------------------------------------------------*/
void add_pair_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned char* name, unsigned int name_length, unsigned char* string, unsigned int string_length, unsigned int func_api )
{
	ATOM_ITEM*	item = add_atom(list,INTERMEDIATE_RECORD_PAIR,atom);

	if (name_length > 0)
	{
		item->pair.name				= malloc(name_length);
		item->pair.name_length		= name_length;
		memcpy(item->pair.name,name,name_length);
	}

	if (string_length > 0)
	{
		item->pair.string			= malloc(string_length);
		item->pair.string_length	= string_length;
		memcpy(item->pair.string,string,string_length);
	}
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
 * Name : decode_name_string
 * Desc : This function will decode a NAME from the input.
 *--------------------------------------------------------------------------------*/
unsigned int decode_name_string (unsigned char* buffer, unsigned int buffer_length, NAME* string)
{
	unsigned int count = 0;

	if (buffer_length > 0)
	{
		/* trim any unsightly characters */
		for (count=buffer_length-1;count>0 && buffer[count] < 0x0f ;count--)
		{
		}

		buffer_length = count+1;
	}

	string->name = buffer;
	string->name_length = count;

	return buffer_length;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_boolean
 * Desc : This function will decode a binary atom. The atom string should be either
 *        yes or no.
 *--------------------------------------------------------------------------------*/
unsigned int decode_boolean (unsigned char* buffer, unsigned int buffer_length, ATOM_INDEX* atom_list, ATOM_ATOMS atom, unsigned int function )
{
	unsigned int 	count = 0;
	unsigned int	true_false = 0;

	if (buffer_length > 0)
	{
		/* trim any unsightly characters */
		for (count=buffer_length-1;count>0 && buffer[count] < 0x0f ;count--)
		{
		}

		buffer_length = count+1;
	}

	/* if it matches yes then set it, else it is false which is the default */
	if (buffer_length == 3 && memcmp("yes",buffer,3) == 0)
	{
		true_false = 1;
	}
	else if (buffer_length != 2 || memcmp("no",buffer,2) != 0)
	{
		raise_warning(atom_list->line_number,EC_EXPECTED_YES_NO,NULL,NULL);
	}

	add_boolean_atom(atom_list,atom,true_false);

	return buffer_length;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_string
 * Desc : This function will decode a string atom and add it to the atom index.
 *        Actually there is not much to do, as the string will be all data on the
 *        line to the end, so just add it to the atom list.
 *--------------------------------------------------------------------------------*/
unsigned int decode_string (unsigned char* buffer, unsigned int buffer_length, ATOM_INDEX* atom_list, ATOM_ATOMS atom, unsigned int function )
{
	unsigned int count = 0;

	if (buffer_length > 0)
	{
		/* trim any unsightly characters */
		for (count=buffer_length-1;count>0 && buffer[count] < 0x0f ;count--)
		{
		}

		buffer_length = count+1;
	}

	add_string_atom(atom_list,atom,buffer,buffer_length,function);

	return buffer_length;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_pair
 * Desc : This function will decode a pair of strings from the input. It expects
 *        the strings to be a name and a string. The will be one or more white
 *        space chars separating the pair.
 *--------------------------------------------------------------------------------*/
void decode_pair (unsigned char* buffer, unsigned int buffer_length, ATOM_INDEX* atom_list, ATOM_ATOMS atom, unsigned int function )
{
	unsigned int count;
	unsigned int name_length = 0;

	/* trim any unsightly characters from the end */
	for (count=buffer_length-1;count>0 && buffer[count] < 0x0f ;count--)
	{
	}

	buffer_length = count+1;
	
	/* look for the first space that delimits the name */
	for (count=0;count<buffer_length;count++)
	{
		if (buffer[count] == ' ' || buffer[count] == '\t')
		{
			/* found the white space */
			break;
		}
	}
			
	name_length = count;

	/* remove the white space */
	while ((buffer[count] == ' ' || buffer[count] == '\t') && count < buffer_length)
	{
		count++;
	}

	add_pair_atom(atom_list,atom,buffer,name_length,&buffer[count],buffer_length - count,function);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_group
 * Desc : This function will decode a group atom and add it to the atom index.
 *        Actually there is not much to do, as the group will be all data on the
 *        line to the end, so just add it to the atom list.
 *--------------------------------------------------------------------------------*/
unsigned short decode_group (unsigned char* buffer, unsigned int buffer_length, ATOM_INDEX* atom_list, ATOM_ATOMS atom)
{
	unsigned int count;
	unsigned short result;

	/* trim any unsightly characters */
	for (count=buffer_length-1;count>0 && buffer[count] < 0x0f ;count--)
	{
	}

	buffer_length = count+1;
								
	result = find_add_lookup(&g_group_lookup,buffer,buffer_length);

	add_number_atom(atom_list,atom,result,INVALID_ITEM);

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : decode_application
 * Desc : This function will decode a application atom and add it to the atom index.
 *        Actually there is not much to do, as the application will be all data on the
 *        line to the end, so just add it to the atom list.
 *--------------------------------------------------------------------------------*/
unsigned short decode_application (unsigned char* buffer, unsigned int buffer_length)
{
	unsigned int count;
	unsigned short result;

	/* trim any unsightly characters */
	for (count=buffer_length-1;count>0 && buffer[count] < 0x0f ;count--)
	{
	}

	buffer_length = count+1;
					
	result = find_add_lookup(&g_applications,buffer,buffer_length);

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : collect_sample
 * Desc : This function will collect the sample and add it to the sample payload.
 *        Note: there is NO formatting done on the data repeated as it is in the
 *        original file.
 *--------------------------------------------------------------------------------*/
void	collect_sample(LOOKUP_ITEM *item, unsigned char* data, unsigned int data_length)
{
	void*			temp;
	unsigned int	new_size = item->payload_length + data_length + 1;

	if ((temp = realloc(item->payload,new_size)) != NULL)
	{	
		item->payload = temp;
		item->payload[item->payload_length] = 0x0a;
		memcpy(&item->payload[item->payload_length+1],data,data_length);
		item->payload_length = new_size;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : parse_line
 * Desc : This function will parse the line and extract any atoms from it and add
 *        them to the atom list.
 *--------------------------------------------------------------------------------*/
static unsigned int parse_line ( unsigned char* line, unsigned int line_length, ATOM_INDEX* atom_list, unsigned int input_type )
{
	unsigned int			err = EC_OK;
	unsigned int			pos = 0;
	unsigned int			mask;
	unsigned int			atom = -1;
	unsigned int			found = 0;
	unsigned int			result = EC_OK;
	unsigned int			fixed;
	unsigned int			not_used;
	unsigned int			real_eol = line_length;
	unsigned int			comment_type;
	unsigned int			sample_start = 0;
	unsigned int			comment_end = 0;
	unsigned int			collect_start = 0;
	NAME					name;
	API_FUNCTION*			temp;
	static NAME				return_type;
	static NAME				function_name;
	static unsigned int		g_ignore = 0;
	static unsigned int		g_multiline = 0;
	static unsigned char	g_type_state = TYPE_STATE_SEARCHING;
	static unsigned int		g_in_function = 0;
	static unsigned int		g_current_api = INVALID_ITEM;
	static unsigned int		g_current_group = DEFAULT_GROUP;
	static unsigned int		g_in_file_block = 0;
	static unsigned short	g_default_group = DEFAULT_GROUP;
	static unsigned int		g_current_sample = INVALID_ITEM;
	static unsigned int		g_current_function = INVALID_ITEM;
	static unsigned int		g_current_application = INVALID_ITEM;
	static unsigned char	g_looking_for_type = 0;
	static unsigned int		g_waiting_to_collect = 0;
	static unsigned char	g_collecting_sample = 0;
	static unsigned char	g_looking_for_comment = 1;
	static unsigned char	g_looking_for_globals = 0;
	static unsigned char	g_looking_for_function = 0;
	static unsigned char	g_looking_for_constants = 0;
	static unsigned char	g_function_state = FUNCTION_STATE_SEARCHING;

	/* must be more then one char before the end of the line */
	while(pos < line_length)
	{
		pos = remove_white_space(pos,line);
	
		if (g_looking_for_comment)
		{
			/* find the comment start */
			if ((comment_type = input_formats[input_type].find_comment(line,line_length,&pos)))
			{
				if (comment_type == 1)
				{
					/* we need to skip the contents of this comment */
					g_looking_for_comment = 0;
					g_ignore = 1;
				}
				else if (comment_type == 2)
				{
					if (g_looking_for_function && g_function_state != FUNCTION_STATE_BODY)
					{
						result = EC_FOUND_COMMENT_EXPECTING_FUNCTION;
						raise_warning(atom_list->line_number,result,NULL,NULL);
					}

					/* we have found the comment we are looking for */
					g_current_group = g_default_group;
					g_looking_for_comment = 0;
					atom_list->block_count++;
				}
			}
			else if (g_looking_for_constants)
			{
				/* decode the constants */
				if (input_formats[input_type].decode_constant(line,line_length,&pos,g_current_group,atom_list) == 2)
				{
					result = EC_FAILED_TO_FIND_CONSTANT;
					raise_warning(atom_list->line_number,result,NULL,NULL);
				}
			}
			else if (g_looking_for_type)
			{
				switch(g_type_state)
				{
					case TYPE_STATE_SEARCHING:
						if ((g_type_state = input_formats[input_type].decode_type(line,line_length,&pos,g_current_group,atom_list)) == TYPE_STATE_FINISHED)
						{
							/* we have finished getting the type - end it.
							 * This also gives us time to collect the last of the comments that follow the type.
							 */
							add_api_end_atom(atom_list,g_current_group);
							g_type_state = TYPE_STATE_SEARCHING;
							g_looking_for_type = 0;
						}
						break;

					case TYPE_STATE_FAILED:
						g_function_state = TYPE_STATE_SEARCHING;
						g_looking_for_type = 0;
						
						result = EC_FAILED_TO_FIND_TYPE;
						raise_warning(atom_list->line_number,result,NULL,NULL);
						break;

					default:
						result = EC_INTERNAL_STATE_ERROR;
						raise_warning(atom_list->line_number,result,NULL,NULL);
						
						g_function_state = TYPE_STATE_SEARCHING;
				}
			}
			else if (g_looking_for_function)
			{
				switch(g_function_state)
				{
					case FUNCTION_STATE_SEARCHING:
						if ((g_function_state = input_formats[input_type].function_start(line,line_length,&pos,&return_type,&function_name)) == FUNCTION_STATE_PARAMETERS)
						{
							if (g_current_function != -1)
							{
								if (g_functions.lookup[g_current_function].name_length > 0)
								{
									result = EC_DUPLICATE_NAME;
									raise_warning(atom_list->line_number,result,g_functions.lookup[g_current_function].name,NULL);
								}
								else if (find_lookup(&g_functions,function_name.name,function_name.name_length) != NULL)
								{
									result = EC_DUPLICATE_FUNCTION;
									raise_warning(atom_list->line_number,result,g_functions.lookup[g_current_function].name,NULL);
								}
								else
								{
									/* found the function specification now add the function return type and the name */
									set_lookup_name(&g_functions.lookup[g_current_function],&function_name,atom_list->line_number);
								}
							}

							if (g_current_api != -1)
							{
								if (g_apis.lookup[g_current_api].name_length > 0)
								{
									result = EC_DUPLICATE_NAME;
									raise_warning(atom_list->line_number,result,g_functions.lookup[g_current_function].name,NULL);
								}
								else if (find_lookup(&g_apis,function_name.name,function_name.name_length) != NULL)
								{
									result = EC_DUPLICATE_FUNCTION;
									raise_warning(atom_list->line_number,result,g_functions.lookup[g_current_function].name,NULL);
								}
								else
								{
									/* found the function specification now add the function return type and the name */
									set_lookup_name(&g_apis.lookup[g_current_api],&function_name,atom_list->line_number);
									set_lookup_group(&g_apis.lookup[g_current_api],g_current_group);

									/* set the function headers */
									temp = (API_FUNCTION*)g_apis.lookup[g_current_api].payload;
									temp->name.name = g_apis.lookup[g_current_api].name;
									temp->name.name_length = g_apis.lookup[g_current_api].name_length;
									memcpy(&((API_FUNCTION*)g_apis.lookup[g_current_api].payload)->return_type,&return_type,sizeof(NAME));
								}
							}
						}
						break;

					case FUNCTION_STATE_PARAMETERS:
						g_function_state = input_formats[input_type].find_parameter(line,line_length,&pos,&return_type,&function_name,&found);
						if (found && g_current_api != -1)
						{
							/* found a parameter, time to add the atom for it */
							if (add_parameter_type(	(API_FUNCTION*)g_apis.lookup[g_current_api].payload,
													&function_name,
													&return_type) != EC_PARAMETER_ADDED)
							{
								result = EC_DUPLICATE_PARAMETER;
								raise_warning(atom_list->line_number,result,g_functions.lookup[g_current_function].name,NULL);
							}
						}
						break;
					
					case FUNCTION_STATE_BODY:
						g_in_function == 1;

						if (input_formats[input_type].start_level(line,line_length,&pos))
						{
						}
						else if (input_formats[input_type].end_level(line,line_length,&pos))
						{
						}
						
						if (input_formats[input_type].function_end(line,line_length,&pos))
						{
							/* found the function end - end the function */
							g_in_function = 0;
							g_looking_for_function = 0;
							g_function_state = FUNCTION_STATE_SEARCHING;

							g_current_function = INVALID_ITEM;
							g_current_api = INVALID_ITEM;
						}
						break;

					default:
						result = EC_FAILED_TO_FIND_FUNCTION;
						raise_warning(atom_list->line_number,result,g_functions.lookup[g_current_function].name,NULL);
						
						g_function_state = FUNCTION_STATE_SEARCHING;
						g_looking_for_function = 0;
						break;
				}
			}
			else
			{
				pos++;
			}
		}
		else if (g_ignore == 0 && line[pos] == g_marker)
		{
			if (line[pos+1] == g_marker)
			{
				/* g_marker escapes itself */
				pos++;
			}
			else if ((atom = atoms_check_word(&line[pos+1])) != -1)
			{
				/* found an atom */
				g_multiline = 0;
				pos += atoms_get_length(atom) + 1;

				if (pos < line_length && line[pos] != ':' && line[pos] != 0x09 && line[pos] != 0x20 && line[pos] != 0x0a && line[pos] != 0x0d)
				{
					result = EC_WEIRD_STUFF_AFTER_COMMAND;
					raise_warning(atom_list->line_number,EC_WEIRD_STUFF_AFTER_COMMAND,NULL,NULL);
					break;
				}

				if (pos < line_length)
				{
					pos++;

					pos = remove_white_space(pos,line);

					/* detach end of line comments. */
					{
						unsigned int temp = line_length-input_formats[input_type].end_of_line_comment_size;
						if (input_formats[input_type].end_comment(line,line_length,&temp))
						{
							line_length -= input_formats[input_type].end_of_line_comment_size;
							comment_end = 1;
						}
					}
				}
	
				/* ok, we have a good start char */
				switch(atom)
				{
					/* handle ingore atom - this is special as it ends the comment block */
					case ATOM_IGNORE:
						if (g_multiline)
						{
							g_multiline = 0;
							add_atom(atom_list,INTERMEDIATE_RECORD_EMPTY,g_multiline);
						}
						
						g_ignore = 1;
						break;

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
					
					/* file atom - is just a name atom */
					case ATOM_FILE:
						if (atom_list->block_count != 1)
						{
							result = EC_FILE_BLOCK_NOT_FIRST_BLOCK;
							raise_warning(atom_list->line_number,result,NULL,NULL);
						}
						pos += decode_string(&line[pos],line_length-pos,atom_list,atom,(RECORD_FUNCTION_FLAG| g_current_function));

						g_in_file_block = 1;
						break;

					/* name atoms */
					case ATOM_TO:
					case ATOM_CALL:
					case ATOM_SEND:
					case ATOM_AFTER:
					case ATOM_RECORD:
					case ATOM_WAITFOR:
					case ATOM_TRIGGER:
					case ATOM_TRIGGERS:
					case ATOM_RESPONDS:
						if ((err = decode_name(&line[pos],atom_list,atom,(RECORD_FUNCTION_FLAG| g_current_function))) != EC_OK)
						{
							/*TODO: decode name needs to change POS so that we dont search the same chars more than once */
							result = err;
						}
						break;

					case ATOM_GROUP:
						g_current_group = decode_group(&line[pos],line_length-pos,atom_list,atom);

						if (g_in_file_block)
						{
							g_default_group = g_current_group;
						}
						break;
					
					case ATOM_SAMPLE:
						if (g_collecting_sample == 1)
						{
							result = EC_MULTIPLE_SAMPLES_WITHOUT_END;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else
						{
							decode_name_string(&line[pos],line_length-pos,&name);
							g_current_sample = new_lookup(&g_samples);
							set_lookup_name(&g_samples.lookup[g_current_sample],&name,atom_list->line_number);
							
							g_waiting_to_collect = 1;
						}
						break;

					case ATOM_END_SAMPLE:
						if (g_collecting_sample == 0)
						{
							result = EC_END_SAMPLE_WITHOUT_START;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else
						{
							g_collecting_sample = 0;
							g_waiting_to_collect = 0;
						}
						break;

					/* go find the language dependant constants */
					case ATOM_CONSTANTS:
						if (g_looking_for_constants == 1)
						{
							result = EC_MULTIPLE_CONSTANTS_REQUEST_WITHOUT_END;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else if (g_in_function)
						{
							result = EC_CANNOT_CALL_HAVE_THESE_ATOMS_IN_A_FUNCTION;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else
						{
							add_atom(atom_list,INTERMEDIATE_RECORD_EMPTY,atom);
							g_looking_for_constants = 1;
						}
						break;

					case ATOM_END_CONSTANTS:
						if (g_looking_for_constants == 0)
						{
							result = EC_END_CONSTANTS_WITHOUT_START;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else
						{
							add_atom(atom_list,INTERMEDIATE_RECORD_EMPTY,atom);
							g_looking_for_constants = 0;
						}
						break;

					/* need to collect the type from the file */
					case ATOM_TYPE:
						if (g_looking_for_type != 0)
						{
							result = EC_MULTIPLE_TYPE_REQUESTS_WITHOUT_TYPE;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else if (g_in_function)
						{
							result = EC_CANNOT_CALL_HAVE_THESE_ATOMS_IN_A_FUNCTION;
							raise_warning(atom_list->line_number,result,&line[pos],NULL);
						}
						else
						{
							g_looking_for_type = 1;
						}
						break;

					/* add an application lookup */
					case ATOM_APPLICATION:
						g_current_application = decode_application(&line[pos],line_length-pos);
						break;

					/* function or API atom, starts a function lookup and a creates a function/api */
					case ATOM_API:
					case ATOM_FUNCTION:
						if (atom == ATOM_API)
							mask = 0x01;
						else
							mask = 0x10;

						if ((g_looking_for_function & mask) != 0)
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
							/* create the new api/function but don't set the name until the function header is found */
							if (atom == ATOM_API)
							{
								g_current_api = new_lookup(&g_apis);
								g_apis.lookup[g_current_api].payload = calloc(1,sizeof(API_FUNCTION));
								((API_FUNCTION*)g_apis.lookup[g_current_api].payload)->api_id = g_current_api;
								add_number_atom(atom_list,atom,g_current_api,INVALID_ITEM);
							}
							else
							{
								g_current_function = new_lookup(&g_functions);
							}
							g_looking_for_function |= mask;
						}
						break;
					
					case ATOM_SYNOPSIS:
							/* TODO: pos += */ decode_pair(&line[pos],line_length-pos,atom_list,atom,g_current_application);
						break;

					case ATOM_RETURNS:
					case ATOM_PARAMETER:
						if (g_in_function)
						{
							result = EC_CANNOT_CALL_HAVE_THESE_ATOMS_IN_A_FUNCTION;
							raise_warning(atom_list->line_number,result,NULL,NULL);
						}
						else 
						{
							/* TODO: pos += */ decode_pair(&line[pos],line_length-pos,atom_list,atom,g_current_api);
						}
						break;
					
					/* now add the multi-line atoms */
					case ATOM_DESC:
					case ATOM_ACTION:
					case ATOM_EXAMPLES:
					case ATOM_DESCRIPTION:

						if (atom == ATOM_DESC)
						{
							/* desc is an alias of description */
							atom = ATOM_DESCRIPTION;
						}

						g_multiline = atom + 1;
						comment_end = 0;

						fixed = input_formats[input_type].trim_multiline(line,line_length,&pos);
						pos += add_multiline_atom(atom_list,g_multiline-1,&line[pos],line_length-pos,input_type,&comment_end,fixed);

						if (comment_end)
						{
							g_multiline = 0;
							g_looking_for_comment = 1;
						}
						break;

					/* string atom, that have a multiline follower */
					case ATOM_SECTION:
					case ATOM_SUBSECTION:
						g_multiline = atom + 1;
						comment_end = 0;

						pos += decode_string(&line[pos],line_length-pos,atom_list,atom,g_current_application);
						break;
					
					/* string atoms for applications */	
					case ATOM_VALUE:
					case ATOM_OPTION:
					case ATOM_COMMAND:
						pos += decode_string(&line[pos],line_length-pos,atom_list,atom,g_current_application);
						break;

					/* string atoms */
					case ATOM_BRIEF:
					case ATOM_AUTHOR:
					case ATOM_REPEATS:
					case ATOM_CONDITION:
						pos += decode_string(&line[pos],line_length-pos,atom_list,atom,(RECORD_FUNCTION_FLAG| g_current_function));
						break;

					case ATOM_MULTIPLE:
					case ATOM_REQUIRED:
						pos += decode_boolean(&line[pos],line_length-pos,atom_list,atom,(RECORD_FUNCTION_FLAG| g_current_application));
						break;

					default:	
						result = EC_UNKNOWN_COMMAND;
						raise_warning(atom_list->line_number,EC_UNKNOWN_COMMAND,NULL,NULL);
						pos++;
						break;
				}

				if (comment_end)
				{
					/* ok, the comment block has come to an end */
					g_ignore = 0;
					g_multiline = 0;
					g_in_file_block = 0;
					g_looking_for_comment = 1;
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
		else if (input_formats[input_type].end_comment(line,line_length,&pos))
		{
			g_ignore = 0;
			g_multiline = 0;
			g_in_file_block = 0;
			g_looking_for_comment = 1;
		
			comment_end = 1;
		}
		else if (g_ignore == 0 && g_multiline)
		{
			/* trim the multiline comments down */
			fixed = input_formats[input_type].trim_multiline(line,line_length,&pos);
			pos += add_multiline_atom(atom_list,g_multiline-1,&line[pos],line_length-pos,input_type,&comment_end,fixed);
			
			if (comment_end)
			{
				g_multiline = 0;
				g_in_file_block = 0;
				g_looking_for_comment = 1;
			}
		}
		else
		{
			/* did not match anything - search next */
			pos++;
		}
	}
	
	/* see if a sample jar is open and collect the sample */
	if (g_waiting_to_collect && comment_end)
	{
		g_collecting_sample = 1;
		g_waiting_to_collect = 0;
	}
	else if (g_collecting_sample)
	{
		collect_sample(&g_samples.lookup[g_current_sample],line,real_eol);
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
	unsigned int	err;
	unsigned int	start = 1;
	unsigned int	result = 0;
	unsigned int	linesize = 0;
	unsigned int	input_file_type;
	unsigned char*	line_buffer;
	FILE*			input_file;
	ATOM_INDEX		raw_atoms;

	memset(&g_group_lookup,0, sizeof(g_group_lookup));
	memset(&g_macro_lookup,0, sizeof(g_macro_lookup));
	memset(&g_functions,0, sizeof(g_functions));
	memset(&g_apis,0, sizeof(g_apis));
	memset(&g_samples,0, sizeof(g_samples));
	memset(&g_applications,0, sizeof(g_applications));

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
					/**-----------------------------------------------------------*
					 * @option	-v
					 *
					 * @description 
					 * This produces extra output information during the generation
					 * of the output.
					 *------------------------------------------------------------*/
					case 'v':
						verbose = 1;
						break;

					/**-----------------------------------------------------------*
					 * @option	-q
					 *
					 * @description
					 * quiet. This flag suppresses any non-error output from the
					 * application.
					 *------------------------------------------------------------*/
					case 'q':
						quiet = 1;
						break;
					
					/**-----------------------------------------------------------*
					 * @option		-o
					 * @value		<output file name>
					 * @required	no
					 *
					 * @description
					 *
					 * This function defines the name of the output file produced
					 * by this application. If the name is not given then it will
					 * default to "doc.pdso".
					 *------------------------------------------------------------*/
					case 'o':
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

					/**-----------------------------------------------------------*
					 * @option		-D
					 * @value		MACRO_NAME=MACRO_VALUE
					 * @required	no
					 * @multiple	yes
					 *
					 * @description
					 * The macro value is used to allow for strings passed into
					 * the build to be substituted within some of the symbol names.
					 * This allows for the same code and markup to be used within
					 * the same document without the name clashing.
					 *------------------------------------------------------------*/
					case 'D':
						if (argv[start][2] != '\0')
						{
							if ((equals = index(&argv[start][2],'=')) != NULL)
							{
								if (equals[1] != '\0')
								{
									add_lookup(&g_macro_lookup,&argv[start][2],equals-&argv[start][2],&equals[1],strlen(&equals[1]),0);
								}
								else if (argv[start+1][0] != '-')
								{
									add_lookup(&g_macro_lookup,&argv[start][2],equals-&argv[start][2],&argv[start+1][0],strlen(&argv[start+1][0]),0);
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
									add_lookup(&g_macro_lookup,argv[start],strlen(argv[start]),argv[start+2],strlen(argv[start+2]),0);
								}
								else if ((argv[start+1][0] == '=') && (argv[start+1][1] != '\0'))
								{
									add_lookup(&g_macro_lookup,argv[start],strlen(argv[start]),&argv[start+1][1],strlen(&argv[start+1][1]),0);
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
								add_lookup(&g_macro_lookup,argv[start+1],strlen(argv[start+1]),argv[start+3],strlen(argv[start+3]),0);
							}
							else if ((argv[start+2][0] == '=') && (argv[start+2][1] != '\0'))
							{
								add_lookup(&g_macro_lookup,argv[start+1],strlen(argv[start+1]),&argv[start+2][1],strlen(&argv[start+2][1]),0);
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

					/**-----------------------------------------------------------*
					 * @option		-?
					 *
					 * @description
					 * This outputs the help message.
					 *------------------------------------------------------------*/
					case '?':
						failed = 1;
						break;

					default:
						failed = 1;
						error_string = "Unknown parameter\n";
				}		

			}else{
				/**-----------------------------------------------------------*
				 * @option 		input_file
				 * @required	yes
				 *
				 * @description
				 *
				 * The source code file to compile.
				 *------------------------------------------------------------*/
		
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
		if ((input_file_type = input_decode_format(infile_name)) == INPUT_FORMAT_INVALID)
		{
			/* A file type that we don't support */
			raise_warning(0,EC_UNSUPPORTED_INPUT_FILE,(unsigned char*)infile_name,NULL);
			result = 1;
		}
		else
		{
			if ((input_file = fopen(infile_name,"r")) == NULL)
			{
				raise_warning(0,EC_FAILED_TO_OPEN_INPUT_FILE,(unsigned char*)infile_name,NULL);
				exit_code = 1;
			}
			else
			{
				exit_code = EXIT_FAILURE;

				line_buffer = malloc(buffer_size);

				init_atoms_index(&raw_atoms);

				/* add a holder for the default group */
				find_add_lookup(&g_group_lookup,(unsigned char*)"",0);
				
				/* application 0 - means not defined so add a default */
				find_add_lookup(&g_applications,(unsigned char*)"",0);

				while((linesize = getline((char**)&line_buffer,&buffer_size,input_file)) != -1)
				{
					raw_atoms.line_number++;

					if (linesize > 1)
					{
						if ((err = parse_line(line_buffer,linesize-1,&raw_atoms,input_file_type)) != EC_OK)
						{
							raise_warning(raw_atoms.line_number,EC_PROBLEM_WITH_INPUT_FILE,(unsigned char*)infile_name,NULL);
							result = 1;
						}
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
	}

	exit(exit_code);
}

