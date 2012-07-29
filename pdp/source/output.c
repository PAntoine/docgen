/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : output
 * Desc  : This file is simply a table of the functions for the supported formats.
 *
 * Author: pantoine
 * Date  : 27/01/2012 12:06:20
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "plugin.h"
#include "output.h"
#include "error_codes.h"
#include "output_format.h"
#include "text_output_format.h"

/*--------------------------------------------------------------------------------*
 * static strings
 *--------------------------------------------------------------------------------*/
static unsigned char	text_fmt[] = "text";
static unsigned char	sequence_diagram_name[] = "sequence";
static unsigned char	state_machine_diagram_name[] = "state_machine";

static unsigned int		text_len = sizeof(text_fmt)-1;
static unsigned int		sequence_diagram_len = sizeof(sequence_diagram_name)-1;
static unsigned int		state_machine_diagram_len = sizeof(state_machine_diagram_name)-1;

#define TEXT_LEN					(sizeof(text_fmt)-1)
#define	SEQUENCE_DIAGRAM_LEN		(sizeof(sequence_diagram_name)-1)
#define	STATE_MACHINE_DIAGRAM_LEN	(sizeof(state_machine_diagram_name)-1)

/*--------------------------------------------------------------------------------*
 * Generic output type flags.
 *--------------------------------------------------------------------------------*/
unsigned char	paged_string[] 	= "paged";
unsigned char	table_string[]	= "table";
unsigned char	margin_string[]	= "margin";
unsigned char	inline_string[]	= "inline";
unsigned char	format_string[]	= "format";

#define	PAGED_SIZE 		(sizeof(paged_string) - 1)
#define	TABLE_SIZE 		(sizeof(table_string) - 1)
#define MARGIN_SIZE		(sizeof(margin_string) - 1)
#define INLINE_SIZE		(sizeof(inline_string) - 1)
#define FORMAT_SIZE		(sizeof(format_string) - 1)

#define GENERIC_OUTPUT_FLAG_PAGED	((unsigned int) 1)
#define GENERIC_OUTPUT_FLAG_MARGIN	((unsigned int) 2)
#define GENERIC_OUTPUT_FLAG_INLINE	((unsigned int) 3)
#define GENERIC_OUTPUT_FLAG_FORMAT	((unsigned int) 4)
#define GENERIC_OUTPUT_FLAG_TABLE	((unsigned int) 5)

static	OUTPUT_FLAG	generic_out_flags[] =
{
	{paged_string,	GENERIC_OUTPUT_FLAG_PAGED 	,0,	PAGED_SIZE,		OUTPUT_FLAG_TYPE_BOOLEAN},
	{margin_string,	GENERIC_OUTPUT_FLAG_MARGIN	,0,	MARGIN_SIZE,	OUTPUT_FLAG_TYPE_NUMBER},
	{inline_string,	GENERIC_OUTPUT_FLAG_INLINE	,0,	INLINE_SIZE,	OUTPUT_FLAG_TYPE_BOOLEAN},
	{format_string,	GENERIC_OUTPUT_FLAG_FORMAT	,0,	FORMAT_SIZE,	OUTPUT_FLAG_TYPE_STRING},
	{table_string,	GENERIC_OUTPUT_FLAG_TABLE	,0,	TABLE_SIZE,		OUTPUT_FLAG_TYPE_BOOLEAN}
};

#define	OUTPUT_FLAG_SIZE	((sizeof(generic_out_flags)/sizeof(generic_out_flags[0])))

static	OUTPUT_FLAG_LIST	g_flag_list = {OUTPUT_FLAG_SIZE,generic_out_flags};

/*--------------------------------------------------------------------------------*
 * static structures.
 *--------------------------------------------------------------------------------*/

OUTPUT_FORMAT	output_formats = 
{
	text_fmt,	TEXT_LEN,
	text_decode_flags,
	text_open,
	text_close,
	text_output_header,
	text_output_footer,
	text_output_raw,text_output_sample,
	text_output_timelines,
	text_output_message,
	text_output_states,
	text_output_start_state,
	text_output_transition,
	text_output_end_state,
	text_output_marker,
	text_output_text,
	text_output_section,
	text_output_title,
	text_output_block,
	text_output_table_start,
	text_output_table_header,
	text_output_table_row,
	text_output_table_end,
	
	NULL
};

DIAGRAM_TYPES	diagram_type[] =
{
	{sequence_diagram_name, SEQUENCE_DIAGRAM_LEN},
	{state_machine_diagram_name, STATE_MACHINE_DIAGRAM_LEN}
};

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : extend_path
 * Desc : This function will extend the path by adding a directory to the path
 *        it will check to see if the directory exists and creates the directory
 *        if it can.
 *--------------------------------------------------------------------------------*/
unsigned int	extend_path(unsigned char* path_name, unsigned int* path_length, unsigned char* extend_name, unsigned int extend_length)
{
	unsigned int	result = 0;
	unsigned int	name_length = *path_length;
	struct stat		stat_buf;

	/* add the format directory */
	memcpy(&path_name[name_length],extend_name,extend_length);
	name_length += extend_length;
	path_name[name_length] = 0;

	/* TODO: the path separator needs to be part of the configuration, maybe a build flag */
	if (stat((char*)path_name,&stat_buf) == -1)
	{
		/* directory does not exist need to create it.
		 * NOTE: intentional use of octal and uses the users umask to set the permissions.
		 */
		if (mkdir((char*)path_name,0777) != -1)
		{
			result = 1;
			path_name[name_length] = '/';
			*path_length = name_length+1;
		}
	}
	else if (S_ISDIR(stat_buf.st_mode))
	{
		/* TODO: here - the path separator is being used */
		/* it already exists and it a directory */
		result = 1;
		path_name[name_length] = '/';
		*path_length = name_length+1;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : get_filename
 * Desc : This function will return the filename part of the path. It will remove
 *        the extension from the filename and only return the filename and the
 *        length of the filename.
 *--------------------------------------------------------------------------------*/
void	get_filename(unsigned char* path, unsigned char** file_name, unsigned int* file_name_length)
{
	unsigned int pos = 0;
	unsigned int start = 0;
	unsigned int length = 0;
	unsigned int last_dot = UINT_MAX;

	while (path[pos] != '\0')
	{
		/* TODO: fix this as above - should be a configuration thing (path separator) */
		if (path[pos] == '/')
		{
 			if (path[pos+1] != '\0')
				start = pos+1;
		}
		else if (path[pos] == '.')
		{
			last_dot = pos;
		}
		pos++;
	}

	if (last_dot == UINT_MAX)
	{
		length = pos - start;
	}
	else
	{
		length = last_dot - start;
	}

	*file_name = &path[start];
	*file_name_length = length;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: register_format
 * @desc: This function will manage the registration of the format to the format
 *        list that will be used to produce the output.
 *--------------------------------------------------------------------------------*/
unsigned int	register_format(void* format)
{
	unsigned int	result = 0;
	OUTPUT_FORMAT*	new_format	= (OUTPUT_FORMAT*) format;
	OUTPUT_FORMAT*	current 	= &output_formats;

	while (current != NULL)
	{
		if (current->name_length == new_format->name_length && 
			memcmp(current->name,new_format->name,new_format->name_length) == 0)
		{
			raise_warning(0,EC_DUPLICATE_PLUGIN_FOR_OUTPUT_FORMAT,new_format->name,NULL);
			break;
		}
		if (current->next == NULL)
		{
			current->next = new_format;
			break;
		}
		current = current->next;
	}
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: output_initialise
 * desc: This function will initialise the output system. It will load any of
 *       the output modules.
 *--------------------------------------------------------------------------------*/
void	output_initialise(char* path_root)
{
	char 			search_path[MAX_FILENAME];
	unsigned int	length;

	/* TODO: this should be passed in a a parameter -d */
	if (path_root == NULL)
	{
		path_root = getenv("BUILD_ROOT");
	}
		
	length = strlen(path_root);
	
	if (length < MAX_FILENAME)
	{
		memcpy(search_path,path_root,length);
		memcpy(&search_path[length],"/output",sizeof("/output"));
		output_init_flag_list(&g_flag_list);

		load_plugins(search_path,DGOF_PLUGIN_OUTPUT);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: output_release
 * desc: This function will release the resources used by the output system.
 *--------------------------------------------------------------------------------*/
void	output_release(void)
{
	char search_path[MAX_FILENAME];

	unload_plugins();
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_find_format
 * Desc : This function will find the output format that has been specified.
 *--------------------------------------------------------------------------------*/
OUTPUT_FORMAT*	output_find_format(unsigned char* name, unsigned int name_length)
{
	OUTPUT_FORMAT* result = NULL;
	OUTPUT_FORMAT* current = &output_formats;

	if (name_length == 0)
	{
		/* if name empty default to text */
		result = &output_formats;
	}
	else
	{
		while (current != NULL)
		{
			if (current->name_length == name_length && memcmp(current->name,name,name_length) == 0)
			{
				result = current;
				break;
			}

			current = current->next;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_open
 * Desc : This function will open the output and set up any handles that are
 *        required for the output format.
 *
 *        All files will be created in a directory structure in path. The structure
 *        below that it up to the file format to structure the format.
 *
 *        The code relies on the fact that the file type will not change between
 *        the open and the close as it will build the directory path for the 
 *        target files to be placed in. If the type is changed mid run then the
 *        system will behave inconsistently.
 *--------------------------------------------------------------------------------*/
unsigned int	output_open(DRAW_STATE* draw_state, char* input_file_name, unsigned char* path, unsigned int path_length)
{
	unsigned int	pos = 0;
	unsigned int	failed = 0;
	unsigned int	result = EC_FAILED;
	unsigned int	name_length;
	unsigned int	file_name_length;
	unsigned char*	file_name;

	if (draw_state->format != NULL)
	{
		/* set the per section defaults */
		get_filename((unsigned char*)&input_file_name[pos],&file_name,&file_name_length);

		draw_state->path_length = 0;

		/* add the route path */
		if (extend_path(draw_state->path,&draw_state->path_length,path,path_length))
		{
			if (extend_path(draw_state->path,
							&draw_state->path_length,
							draw_state->format->name,
							draw_state->format->name_length))
			{
				/* ok, let the file draw_state->format open the file/dir that it requires */
				result = draw_state->format->output_open(draw_state,file_name,file_name_length);
			}
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_close
 * Desc : This function close the output and tidy-up anything that needs tidying
 *        up for the format.
 *--------------------------------------------------------------------------------*/
void	output_close(DRAW_STATE* draw_state)
{
	if (draw_state->output_file != -1)
	{
		draw_state->format->output_close(draw_state);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: output_init_flag_list
 * desc: Set up the flag list so it can be searched. This function mostly will
 *       hash the strings so that searching is quicker.
 *--------------------------------------------------------------------------------*/
void	output_init_flag_list(OUTPUT_FLAG_LIST* list)
{
	unsigned int	count;

	for (count=0; count < list->num_flags; count++)
	{
		list->flag_list[count].hash = fnv_32_hash(list->flag_list[count].name,list->flag_list[count].name_length);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: boolean_check
 * @desc: This function will check to see if the value matches true or false. 
 *        If the value is empty, then the answer will default to true, if the
 *        value does not match true, then it will be false.
 *--------------------------------------------------------------------------------*/
unsigned char	boolean_check(NAME* value)
{
	unsigned char result = 1;

	if (value->name_length > 0)
	{
		if (value->name_length != 4 || memcmp(value->name,"true",4) != 0)
		{
			result = 0;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: number_check
 * @desc: This function will decode a number into a signed int.
 *--------------------------------------------------------------------------------*/
int	number_check(NAME* value)
{
	unsigned int pos = 0;
	unsigned int result = 0;

	if (value->name_length == 0)
	{
		result = 0;
	}
	else
	{
		if (value->name[0] == '-')
		{
			result = -1;
			pos = 1;
		}

		while (pos < value->name_length)
		{
			if (value->name[pos] >= '0' && value->name[pos] <= '9')
			{
				result = (result * 10) + (value->name[pos] - '0');
			}
			else
			{
				/* failed return 0 */
				result = 0;
				break;
			}

			pos++;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: real_check
 * @desc: This function will decode a float from a string.
 *--------------------------------------------------------------------------------*/
float	real_check(NAME* value)
{
	float			result = 0;
	float			number;
	unsigned int	pos = 0;
	unsigned int	no_point = 1;

	if (value->name[0] == '-')
		result = -1.0;

	while (pos < value->name_length)
	{
		if (value->name[pos] >= '0' && value->name[pos] <= '9')
		{
			number = (number * 10.0) + (value->name[pos] - '0');
		}
		else if (value->name[pos] == '.' && no_point)
		{
			result += number;
			number = 0;
			no_point = 0;
		}
		else
		{
			result = 0;
			break;
		}

		pos++;
	}

	if (!no_point && number > 0)
	{
		result += 1/number;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: output_find_flag
 * desc: This function will decode a option flag and decode the value in the 
 *       value field.
 *--------------------------------------------------------------------------------*/
unsigned int	output_find_flag(OUTPUT_FLAG_LIST* list, unsigned int option_hash, NAME* value, OUTPUT_FLAG_VALUE* flag_value)
{
	unsigned int	count;

	flag_value->type = OUTPUT_FLAG_TYPE_INVALID;

	for (count=0; count < list->num_flags; count++)
	{
		if (list->flag_list[count].hash == option_hash)
		{
			flag_value->id	 = list->flag_list[count].id;
			flag_value->type = list->flag_list[count].type;
			break;
		}
	}

	switch (flag_value->type)
	{
		case OUTPUT_FLAG_TYPE_BOOLEAN: 	flag_value->value.boolean	= boolean_check(value);	break;
		case OUTPUT_FLAG_TYPE_NUMBER:	flag_value->value.number	= number_check(value);	break;
		case OUTPUT_FLAG_TYPE_REAL:		flag_value->value.real		= real_check(value);	break;
		case OUTPUT_FLAG_TYPE_STRING:
			flag_value->value.string.name			= value->name;
			flag_value->value.string.name_length	= value->name_length;
			break;
	}

	return flag_value->type;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_set_flag
 * @desc: This function will set global draw state flags.
 *--------------------------------------------------------------------------------*/
static void	output_set_flag(DRAW_STATE* draw_state, OUTPUT_FLAG_VALUE* flag_value)
{
	if (flag_value->type == OUTPUT_FLAG_TYPE_BOOLEAN && flag_value->value.boolean)
	{
		switch(flag_value->id)
		{
			case GENERIC_OUTPUT_FLAG_PAGED:		draw_state->format_flags |= OUTPUT_FORMAT_PAGED;	break;
			case GENERIC_OUTPUT_FLAG_INLINE:	draw_state->format_flags |= OUTPUT_FORMAT_INLINE;	break;
			case GENERIC_OUTPUT_FLAG_TABLE:		draw_state->format_flags |= OUTPUT_FORMAT_TABLE;	break;
		}
	}
	else
	{
		switch(flag_value->id)
		{
			case GENERIC_OUTPUT_FLAG_MARGIN:	draw_state->margin_width = flag_value->value.number;	break;
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name  output_parse_flags
 * desc  This function will parse the input flags. If the flags are not one of
 *       the standard flags then the input will be passed onto the specified 
 *       formats to see if it is known.
 *--------------------------------------------------------------------------------*/
void	output_parse_flags(INPUT_STATE* input_state, DRAW_STATE* draw_state)
{
	unsigned int		pos = 0;
	unsigned int		hash;
	unsigned int		last_flag = 0;
	unsigned int		last_equals = 0;
	NAME				value;
	OUTPUT_FLAG_VALUE	flag_value;

	/* reset the format flags */
	draw_state->margin_width = draw_state->global_margin_width;
	draw_state->format_flags = draw_state->global_format_flags;

	while (pos < input_state->flags_length)
	{
		if (input_state->flags[pos] == '=')
		{
			last_equals = pos;
		}
		else if (input_state->flags[pos] == ',')
		{
			value.name			= &input_state->flags[last_equals+1];
			value.name_length	= pos - last_equals - 2;

			hash = fnv_32_hash(&input_state->flags[last_flag],(last_equals - last_flag));

			/* look in the global list first, then in the format specific one after */
			if (output_find_flag(&g_flag_list,hash,&value,&flag_value) == OUTPUT_FLAG_TYPE_INVALID)
			{
				if (draw_state->format->decode_flags(input_state,hash,&value) == OUTPUT_FLAG_TYPE_INVALID)
				{
					raise_warning(input_state->line_number,EC_UNKNOWN_OUTPUT_FLAG,value.name,NULL);
				}
			}
			else
			{
				/* set the output flags */
				output_set_flag(draw_state,&flag_value);
			}

			last_flag = pos + 1;
			last_equals = pos + 1;
		}

		pos++;
	}

	/* catch the last option */
	if (pos > last_flag)
	{
		value.name			= &input_state->flags[last_equals+1];
		value.name_length	= pos - last_equals - 1;

		hash = fnv_32_hash(&input_state->flags[last_flag],(last_equals - last_flag));

		if (output_find_flag(&g_flag_list,hash,&value,&flag_value) == OUTPUT_FLAG_TYPE_INVALID)
		{
			/* look in the global list first, then in the format specific one after */
			if (draw_state->format->decode_flags(input_state,hash,&value) == OUTPUT_FLAG_TYPE_INVALID)
			{
				raise_warning(input_state->line_number,EC_UNKNOWN_OUTPUT_FLAG,value.name,NULL);
			}
		}
		else
		{
			/* set the output flags */
			output_set_flag(draw_state,&flag_value);
		}
	}
}


