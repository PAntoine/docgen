/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : supported_formats
 * Desc  : This file is simply a table of the functions for the supported formats.
 *
 * Author: pantoine
 * Date  : 27/01/2012 12:06:20
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <string.h>
#include "error_codes.h"
#include "supported_formats.h"

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
unsigned char	margin_string[]	= "margin";
unsigned char	inline_string[]	= "inline";

#define	PAGED_SIZE 		(sizeof(paged_string) - 1)
#define MARGIN_SIZE		(sizeof(margin_string) - 1)
#define INLINE_SIZE		(sizeof(inline_string) - 1)

#define GENERIC_OUTPUT_FLAG_PAGED	((unsigned int) 1)
#define GENERIC_OUTPUT_FLAG_MARGIN	((unsigned int) 2)
#define GENERIC_OUTPUT_FLAG_INLINE	((unsigned int) 3)

static	OUTPUT_FLAG	generic_out_flags[] =
{
	{paged_string,	GENERIC_OUTPUT_FLAG_PAGED 	,0,	PAGED_SIZE,		OUTPUT_FLAG_TYPE_BOOLEAN},
	{margin_string,	GENERIC_OUTPUT_FLAG_MARGIN	,0,	MARGIN_SIZE,	OUTPUT_FLAG_TYPE_NUMBER},
	{inline_string,	GENERIC_OUTPUT_FLAG_INLINE	,0,	INLINE_SIZE,	OUTPUT_FLAG_TYPE_BOOLEAN}
};

#define	OUTPUT_FLAG_SIZE	((sizeof(generic_out_flags)/sizeof(generic_out_flags[0])))

static	OUTPUT_FLAG_LIST	g_flag_list = {OUTPUT_FLAG_SIZE,generic_out_flags};

/*--------------------------------------------------------------------------------*
 * static structures.
 *--------------------------------------------------------------------------------*/

OUTPUT_FORMATS	output_formats[] = 
{
	{text_fmt,	TEXT_LEN,	text_decode_flags_function,
							text_open,text_close,text_output_header,text_output_footer,text_output_raw,text_output_sample,
							text_output_timelines,text_output_message,
							text_output_states,	text_output_start_state,text_output_transition,text_output_end_state,
							text_output_api_name_function, text_output_api_description_function, text_output_api_prototype_function,
							text_output_api_parameters_function, text_output_api_action_function, text_output_api_returns_function,
							text_output_type_name_function, text_output_type_description_function, text_output_type_records_function,
							text_output_constants_records_function, text_output_constants_description_function, 
							text_output_constant_name_function,
							text_output_application_name_function, text_output_application_synopsis_function,
							text_output_application_option_function, text_output_application_command_function,
							text_output_application_section_function
	}

#if 0
	{ "dot",dot_output_header,dot_output_footer,dot_output_timelines,dot_output_message,dot_output_states,dot_output_transition}
#endif
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

	if (stat((char*)path_name,&stat_buf) == -1)
	{
		/* directory does not exist need to create it.
		 * NOTE: intentional use of octal and uses the users umask to set the permissions.
		 */
		if (mkdir((char*)path_name,0777) != -1)
		{
			result = 1;
			path_name[name_length] = PATH_SEPARATOR;
			*path_length = name_length+1;
		}
	}
	else if (S_ISDIR(stat_buf.st_mode))
	{
		/* it already exists and it a directory */
		result = 1;
		path_name[name_length] = PATH_SEPARATOR;
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
		if (path[pos] == PATH_SEPARATOR)
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
 * name: output_initialise
 * desc: This function will initialise the output system. It will load any of
 *       the output modules.
 *
 * TODO: Actually load the output modules, using some form of shared object
 *       loader.
 *--------------------------------------------------------------------------------*/
void	output_initialise(void)
{
	output_init_flag_list(&g_flag_list);
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
	unsigned int	failed = 0;
	unsigned int	result = EC_FAILED;
	unsigned int	name_length;
	unsigned int	file_name_length;
	unsigned char*	file_name;

	draw_state->format = OUTPUT_TEXT;

	/* set the per section defaults */
	draw_state->global_margin_width = 4;
	draw_state->global_format_flags = 0;

	get_filename((unsigned char*)input_file_name,&file_name,&file_name_length);

	if (draw_state->format < OUTPUT_FORMATS_MAX)
	{
		draw_state->path_length = 0;

		/* add the route path */
		if (extend_path(draw_state->path,&draw_state->path_length,path,path_length))
		{
			if (extend_path(draw_state->path,
							&draw_state->path_length,
							output_formats[draw_state->format].format_name,
							output_formats[draw_state->format].format_name_length))
			{
				/* ok, let the file draw_state->format open the file/dir that it requires */
				result = output_formats[draw_state->format].output_open(draw_state,file_name,file_name_length);
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
		close(draw_state->output_file);
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
			flag_value->id	 = count;
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
			case GENERIC_OUTPUT_FLAG_PAGED:		draw_state->format_flags |= OUTPUT_FORMAT_FLAGS_PAGED;	break;
			case GENERIC_OUTPUT_FLAG_INLINE:	draw_state->format_flags |= OUTPUT_FORMAT_FLAGS_INLINE;	break;
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
				if (output_formats[draw_state->format].decode_flags(input_state,hash,&value) == OUTPUT_FLAG_TYPE_INVALID)
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
			if (output_formats[draw_state->format].decode_flags(input_state,hash,&value) == OUTPUT_FLAG_TYPE_INVALID)
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


