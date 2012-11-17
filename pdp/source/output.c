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
#include "graph.h"
#include "plugin.h"
#include "output.h"
#include "utilities.h"
#include "error_codes.h"
#include "output_format.h"
#include "file_functions.h"
#include "indexing_format.h"
#include "text_output_format.h"

/*--------------------------------------------------------------------------------*
 * static strings
 *--------------------------------------------------------------------------------*/
static unsigned char	sequence_diagram_name[] = "sequence";
static unsigned char	state_machine_diagram_name[] = "state_machine";
static unsigned char	plugins_dir[] = "/plugins";

static unsigned int		sequence_diagram_len = sizeof(sequence_diagram_name)-1;
static unsigned int		state_machine_diagram_len = sizeof(state_machine_diagram_name)-1;
static unsigned int		plugins_dir_len = sizeof(plugins_dir) - 1;

#define	SEQUENCE_DIAGRAM_LEN		(sizeof(sequence_diagram_name)-1)
#define	STATE_MACHINE_DIAGRAM_LEN	(sizeof(state_machine_diagram_name)-1)

/*--------------------------------------------------------------------------------*
 * static constant strings
 *--------------------------------------------------------------------------------*/
static unsigned char	string_none[] = "";
static unsigned char	string_api[] = "api";
static unsigned char	string_application[] = "application";
static unsigned char	string_state_machine[] = "state_machine";
static unsigned char	string_sequence_diagram[] = "sequence_diagram";
static unsigned char	string_sample[] = "sample";
static unsigned char*	type_string[] = {string_none,string_state_machine,string_sequence_diagram,string_api,string_sample,string_application};
static unsigned int		type_length[] = {	0,
											sizeof(string_state_machine)-1,
											sizeof(string_sequence_diagram)-1,
											sizeof(string_api)-1,
											sizeof(string_sample)-1,
											sizeof(string_application)-1};

static unsigned char	model_name[] = "model:";
static unsigned int		model_length = sizeof(model_name) - 1;

static unsigned char	string_to[] = "To";
static unsigned char	string_from[] = "From";
static unsigned char	string_name[] = "name";
static unsigned char	string_type[] = "type";
static unsigned char	string_comma[] = ",";
static unsigned char	string_colon[] = ":";
static unsigned char	string_cname[] = "Name";
static unsigned char	string_state[] = "State";
static unsigned char	string_input[] = "Input";
static unsigned char	string_options[] = "Options";
static unsigned char	string_returns[] = "Returns";
static unsigned char	string_message[] = "Message";
static unsigned char	string_ellipsis[] = "...";
static unsigned char	string_commands[] = "Commands";
static unsigned char	string_constant[] = "constant";
static unsigned char	string_synopsis[] = "Synopsis";
static unsigned char	string_triggers[] = "Triggers";
static unsigned char	string_open_round[] = "(";
static unsigned char	string_next_state[] = "Next State";
static unsigned char	string_description[] = "description";
static unsigned char	string_open_square[] = "[";
static unsigned char	string_close_round[] = ")";
static unsigned char	string_close_square[] = "]";
static unsigned char	string_ctype[] = "API Types";
static unsigned char	string_function[] = "API Functions";
static unsigned char	string_constants[] = "API Constants";

static NAME				name_to = {string_to,sizeof(string_to)-1,0,0};
static NAME				name_from = {string_from,sizeof(string_from)-1,0,0};
static NAME				name_name = {string_name,sizeof(string_name)-1,0,0};
static NAME				name_type = {string_type,sizeof(string_type)-1,0,0};
static NAME				name_comma = {string_comma,sizeof(string_comma)-1,0,0};
static NAME				name_colon = {string_colon,sizeof(string_colon)-1,0,0};
static NAME				name_cname = {string_cname,sizeof(string_cname)-1,0,0};
static NAME				name_state = {string_state,sizeof(string_state)-1,0,0};
static NAME				name_input = {string_input,sizeof(string_input)-1,0,0};
static NAME				name_options = {string_options,sizeof(string_options)-1,0,0};
static NAME				name_returns = {string_returns,sizeof(string_returns)-1,0,0};
static NAME				name_message = {string_message,sizeof(string_message)-1,0,0};
static NAME				name_ellipsis = {string_ellipsis,sizeof(string_ellipsis)-1,0,0};
static NAME				name_commands = {string_commands,sizeof(string_commands)-1,0,0};
static NAME				name_constant = {string_constant,sizeof(string_constant)-1,0,0};
static NAME				name_synopsis = {string_synopsis,sizeof(string_synopsis)-1,0,0};
static NAME				name_triggers = {string_triggers,sizeof(string_triggers)-1,0,0};
static NAME				name_next_state = {string_next_state,sizeof(string_next_state)-1,0,0};
static NAME				name_description = {string_description,sizeof(string_description)-1,0,0};
static NAME				name_open_round = {string_open_round,sizeof(string_open_round)-1,0,0};
static NAME				name_open_square = {string_open_square,sizeof(string_open_square)-1,0,0};
static NAME				name_close_round = {string_close_round,sizeof(string_close_round)-1,0,0};
static NAME				name_close_square = {string_close_square,sizeof(string_close_square)-1,0,0};
static NAME				name_ctype = {string_ctype,sizeof(string_ctype)-1,0,0};
static NAME				name_function = {string_function,sizeof(string_function)-1,0,0};
static NAME				name_constants = {string_constants,sizeof(string_constants)-1,0,0};



/*--------------------------------------------------------------------------------*
 * Generic output type flags.
 *--------------------------------------------------------------------------------*/
unsigned char	line_string[]		= "line";
unsigned char	flat_string[]		= "flat";
unsigned char	paged_string[]		= "paged";
unsigned char	table_string[]		= "table";
unsigned char	margin_string[]		= "margin";
unsigned char	inline_string[]		= "inline";
unsigned char	format_string[]		= "format";
unsigned char	index_string[]		= "index";
unsigned char	level_string[]		= "level";
unsigned char	levels_string[]		= "levels";
unsigned char	cbreak_string[]		= "cbreak";
unsigned char	reference_string[]	= "reference";
unsigned char	number_string[]		= "number";
unsigned char	number_style_string[] = "number_style";


#define	LINE_SIZE			(sizeof(line_string) - 1)
#define	FLAT_SIZE			(sizeof(flat_string) - 1)
#define	PAGED_SIZE			(sizeof(paged_string) - 1)
#define	TABLE_SIZE			(sizeof(table_string) - 1)
#define MARGIN_SIZE			(sizeof(margin_string) - 1)
#define INLINE_SIZE			(sizeof(inline_string) - 1)
#define FORMAT_SIZE			(sizeof(format_string) - 1)
#define LEVEL_SIZE			(sizeof(level_string) - 1)
#define LEVELS_SIZE			(sizeof(levels_string) - 1)
#define CBREAK_SIZE			(sizeof(cbreak_string) - 1)
#define INDEX_SIZE			(sizeof(index_string) - 1)
#define REFERENCE_SIZE		(sizeof(reference_string) - 1)
#define NUMBER_SIZE			(sizeof(number_string) - 1)
#define NUMBER_STYLE_SIZE	(sizeof(number_style_string) - 1)

#define GENERIC_OUTPUT_FLAG_PAGED			((unsigned int)  1)
#define GENERIC_OUTPUT_FLAG_MARGIN			((unsigned int)  2)
#define GENERIC_OUTPUT_FLAG_INLINE			((unsigned int)  3)
#define GENERIC_OUTPUT_FLAG_FORMAT			((unsigned int)  4)
#define GENERIC_OUTPUT_FLAG_TABLE			((unsigned int)  5)
#define GENERIC_OUTPUT_FLAG_INDEX			((unsigned int)  7)
#define GENERIC_OUTPUT_FLAG_NUMBER		    ((unsigned int)  8)
#define GENERIC_OUTPUT_FLAG_REFERENCE	    ((unsigned int)  9)
#define GENERIC_OUTPUT_FLAG_NUMBER_STYLE    ((unsigned int) 10)
#define GENERIC_OUTPUT_FLAG_LEVEL			((unsigned int) 11)
#define GENERIC_OUTPUT_FLAG_LINE			((unsigned int) 12)
#define GENERIC_OUTPUT_FLAG_FLAT			((unsigned int) 13)
#define GENERIC_OUTPUT_FLAG_CBREAK			((unsigned int) 14)
#define GENERIC_OUTPUT_FLAG_LEVELS			((unsigned int) 15)

static	OUTPUT_FLAG	generic_out_flags[] =
{
	{paged_string,			GENERIC_OUTPUT_FLAG_PAGED		,0,	PAGED_SIZE,			OUTPUT_FLAG_TYPE_BOOLEAN},
	{inline_string,			GENERIC_OUTPUT_FLAG_INLINE		,0,	INLINE_SIZE,		OUTPUT_FLAG_TYPE_BOOLEAN},
	{reference_string,		GENERIC_OUTPUT_FLAG_REFERENCE	,0,	REFERENCE_SIZE,		OUTPUT_FLAG_TYPE_BOOLEAN},
	{table_string,			GENERIC_OUTPUT_FLAG_TABLE		,0,	TABLE_SIZE,			OUTPUT_FLAG_TYPE_BOOLEAN},
	{level_string,			GENERIC_OUTPUT_FLAG_LEVEL		,0,	LEVEL_SIZE,			OUTPUT_FLAG_TYPE_BOOLEAN},
	{index_string,			GENERIC_OUTPUT_FLAG_INDEX		,0,	INDEX_SIZE,			OUTPUT_FLAG_TYPE_BOOLEAN},
	{line_string,			GENERIC_OUTPUT_FLAG_LINE		,0,	LINE_SIZE,			OUTPUT_FLAG_TYPE_BOOLEAN},
	{flat_string,			GENERIC_OUTPUT_FLAG_FLAT		,0,	FLAT_SIZE,			OUTPUT_FLAG_TYPE_BOOLEAN},
	{number_string,			GENERIC_OUTPUT_FLAG_NUMBER		,0,	NUMBER_SIZE,		OUTPUT_FLAG_TYPE_BOOLEAN},
	{cbreak_string,			GENERIC_OUTPUT_FLAG_CBREAK		,0,	CBREAK_SIZE,		OUTPUT_FLAG_TYPE_BOOLEAN},
	{levels_string,			GENERIC_OUTPUT_FLAG_LEVELS		,0,	LEVELS_SIZE,		OUTPUT_FLAG_TYPE_NUMBER},
	{margin_string,			GENERIC_OUTPUT_FLAG_MARGIN		,0,	MARGIN_SIZE,		OUTPUT_FLAG_TYPE_NUMBER},
	{format_string,			GENERIC_OUTPUT_FLAG_FORMAT		,0,	FORMAT_SIZE,		OUTPUT_FLAG_TYPE_STRING},
	{number_style_string,	GENERIC_OUTPUT_FLAG_NUMBER_STYLE,0,	NUMBER_STYLE_SIZE,	OUTPUT_FLAG_TYPE_STRING}
};

#define	OUTPUT_FLAG_SIZE	((sizeof(generic_out_flags)/sizeof(generic_out_flags[0])))

static	OUTPUT_FLAG_LIST	g_flag_list = {OUTPUT_FLAG_SIZE,generic_out_flags};

static unsigned char	string_action[] = "Action";
static unsigned char	string_cdescription[] = "Description";

static NAME				name_action = {string_action,sizeof(string_action)-1,0,0};
static NAME				name_cdescription = {string_cdescription,sizeof(string_cdescription)-1,0,0};

DIAGRAM_TYPES	diagram_type[] =
{
	{sequence_diagram_name, SEQUENCE_DIAGRAM_LEN},
	{state_machine_diagram_name, STATE_MACHINE_DIAGRAM_LEN}
};

static void			format_list_start			(DRAW_STATE* draw_state, unsigned int level, unsigned char marker);
static void			format_list_numeric_start	(DRAW_STATE* draw_state, unsigned int level);

extern OUTPUT_FORMAT	output_formats;

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
 * Name : decode_api_item
 * Desc : This function will decode the api item name set to flags to the items
 *        that are required.
 *--------------------------------------------------------------------------------*/
static unsigned int	decode_api_item(INPUT_STATE* input_state, unsigned int *api_flags)
{
	unsigned int pos = 0;
	unsigned int name_end = 0;
	unsigned int name_start = 0;
	unsigned int result = 1;
	unsigned int offset = 0;
		
	if (input_state->part_name.name_length == 0)
	{
		*api_flags = (	OUTPUT_API_ALL					| 
						OUTPUT_API_MULTIPLE				| 
						OUTPUT_API_FUNCTION_ALL_PARTS	| 
						OUTPUT_API_TYPE_ALL_PARTS		|
						OUTPUT_API_CONSTANTS_ALL_PARTS);
	}
	else
	{
		switch(atoms_check_word(input_state->part_name.name))
		{
			case ATOM_FUNCTION:
				*api_flags = OUTPUT_API_FUNCTIONS;
				*api_flags |= OUTPUT_API_FUNCTION_ALL_PARTS;
				pos += atoms_get_length(ATOM_FUNCTION);
				break;

			case ATOM_TYPE:
				*api_flags = OUTPUT_API_TYPES;
				*api_flags |= OUTPUT_API_TYPE_ALL_PARTS;
				pos += atoms_get_length(ATOM_TYPE);
				break;

			case ATOM_CONSTANTS:
				*api_flags = OUTPUT_API_CONSTANTS;
				pos += atoms_get_length(ATOM_CONSTANTS);
				break;

			default:
				raise_warning(input_state->line_number,EC_BAD_NAME_FORMAT_IN_REQUEST,input_state->part_name.name,NULL);
				result = 0;
		}

		if (input_state->item_name.name_length == 0)
		{
			/* if no name is specified then output all the things */
			*api_flags |= OUTPUT_API_MULTIPLE;
		}
		else
		{
			/* read the sub-parts */
			if (input_state->sub_part.name_length != 0)
			{
				*api_flags &= ~(OUTPUT_API_FUNCTION_ALL_PARTS | OUTPUT_API_TYPE_ALL_PARTS);

				if ((*api_flags & OUTPUT_API_FUNCTIONS) != 0 && input_state->sub_part.name_length)
				{
					switch(atoms_check_word(input_state->sub_part.name))
					{
						case 	ATOM_NAME:
							pos += atoms_get_length(ATOM_NAME);
							*api_flags |= OUTPUT_API_FUNCTION_NAME;
							break;

						case 	ATOM_ACTION:
							pos += atoms_get_length(ATOM_ACTION);
							*api_flags |= OUTPUT_API_FUNCTION_ACTION;
							break;

						case 	ATOM_RETURNS:
							pos += atoms_get_length(ATOM_RETURNS);
							*api_flags |= OUTPUT_API_FUNCTION_RETURNS;
							break;

						case 	ATOM_PROTOTYPE:
							pos += atoms_get_length(ATOM_PROTOTYPE);
							*api_flags |= OUTPUT_API_FUNCTION_PROTOTYPE;
							break;

						case 	ATOM_PARAMETERS:
							pos += atoms_get_length(ATOM_PARAMETERS);
							*api_flags |= OUTPUT_API_FUNCTION_PARAMETERS;
							break;

						case 	ATOM_DESCRIPTION:
							pos += atoms_get_length(ATOM_DESCRIPTION);
							*api_flags |= OUTPUT_API_FUNCTION_DESCRIPTION;
							break;  	

						default:
							raise_warning(input_state->line_number,EC_BAD_NAME_FORMAT_IN_REQUEST,input_state->sub_part.name,NULL);
							result = 0;
							break;
					}
				}
				else if ((*api_flags & OUTPUT_API_TYPES) != 0 && input_state->sub_part.name_length)
				{
					switch(atoms_check_word(input_state->sub_part.name))
					{
						case	ATOM_NAME:
							pos += atoms_get_length(ATOM_NAME);
							*api_flags |= OUTPUT_API_TYPE_NAME;
							break;

						case 	ATOM_DESCRIPTION:
							pos += atoms_get_length(ATOM_DESCRIPTION);
							*api_flags |= OUTPUT_API_TYPE_DESCRIPTION;
							break;

						default:
							raise_warning(input_state->line_number,EC_BAD_NAME_FORMAT_IN_REQUEST,input_state->sub_part.name,NULL);
							result = 0;
							break;
					}
				}
				else if ((*api_flags & OUTPUT_API_CONSTANTS) != 0 && input_state->sub_part.name_length)
				{
					switch(atoms_check_word(input_state->sub_part.name))
					{
						case	ATOM_NAME:
							pos += atoms_get_length(ATOM_NAME);
							*api_flags |= OUTPUT_API_CONSTANTS_NAME;
							break;

						case 	ATOM_DESCRIPTION:
							pos += atoms_get_length(ATOM_DESCRIPTION);
							*api_flags |= OUTPUT_API_CONSTANTS_DESCRIPTION;
							break;

						case	ATOM_CONSTANTS:
							pos += atoms_get_length(ATOM_CONSTANTS);
							*api_flags |= OUTPUT_API_CONSTANTS_CONSTANT;
							break;

						default:
							raise_warning(input_state->line_number,EC_BAD_NAME_FORMAT_IN_REQUEST,input_state->sub_part.name,NULL);
							result = 0;
							break;
					}
				}
			}
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: output_init_flag_list
 * desc: Set up the flag list so it can be searched. This function mostly will
 *       hash the strings so that searching is quicker.
 *--------------------------------------------------------------------------------*/
static void	output_init_flag_list(OUTPUT_FLAG_LIST* list)
{
	unsigned int	count;

	for (count=0; count < list->num_flags; count++)
	{
		list->flag_list[count].hash = fnv_32_hash(list->flag_list[count].name,list->flag_list[count].name_length);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: output_initialise
 * desc: This function will initialise the output system. It will load any of
 *       the output modules.
 *--------------------------------------------------------------------------------*/
void	output_initialise(char* resource_path, int resource_path_length)
{

	if (resource_path_length + plugins_dir_len < MAX_FILENAME)
	{
		/* build the path */
		memcpy(&resource_path[resource_path_length],plugins_dir,plugins_dir_len);
		resource_path[resource_path_length + plugins_dir_len] = '\0';
		
		output_init_flag_list(&g_flag_list);

		load_plugins(resource_path,DGOF_PLUGIN_OUTPUT);
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
unsigned int	output_open(DRAW_STATE* draw_state, INPUT_STATE* input_state, char* input_file_name, unsigned char* path, unsigned int path_length)
{
	unsigned int	failed = 0;
	unsigned int	result = EC_FAILED;
	unsigned int	name_length;
	unsigned int	file_name_length;
	unsigned char*	file_name;

	if (draw_state->format != NULL)
	{
		/* set the per section defaults */
		get_filename((unsigned char*)input_file_name,&file_name,&file_name_length);


		/* set the default name to the file name */
		draw_state->model->document->document_name.name = file_name;
		draw_state->model->document->document_name.name_length = file_name_length;

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
				result = draw_state->format->output_open(draw_state,input_state, file_name,file_name_length);
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
void	output_close(DRAW_STATE* draw_state,INPUT_STATE* input_state)
{
	if (draw_state->output_file != -1)
	{
		draw_state->format->output_close(draw_state,input_state);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: boolean_check
 * @desc: This function will check to see if the value matches true or false. 
 *        If the value is empty, then the answer will default to true, if the
 *        value does not match true, then it will be false.
 *--------------------------------------------------------------------------------*/
static unsigned char	boolean_check(NAME* value)
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
static int	number_check(NAME* value)
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
static float	real_check(NAME* value)
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
static unsigned int	output_find_flag(OUTPUT_FLAG_LIST* list, unsigned int option_hash, NAME* value, OUTPUT_FLAG_VALUE* flag_value)
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
			case GENERIC_OUTPUT_FLAG_FLAT:		draw_state->format_flags |= OUTPUT_FORMAT_FLAT;			break;
			case GENERIC_OUTPUT_FLAG_LINE:		draw_state->format_flags |= OUTPUT_FORMAT_LINED;		break;
			case GENERIC_OUTPUT_FLAG_PAGED:		draw_state->format_flags |= OUTPUT_FORMAT_PAGED;		break;
			case GENERIC_OUTPUT_FLAG_INDEX:		draw_state->format_flags |= OUTPUT_FORMAT_INDEX;		break;
			case GENERIC_OUTPUT_FLAG_LEVEL:		draw_state->format_flags |= OUTPUT_FORMAT_LEVEL;		break;
			case GENERIC_OUTPUT_FLAG_TABLE:		draw_state->format_flags |= OUTPUT_FORMAT_TABLE;		break;
			case GENERIC_OUTPUT_FLAG_INLINE:	draw_state->format_flags |= OUTPUT_FORMAT_INLINE;		break;
			case GENERIC_OUTPUT_FLAG_CBREAK:	draw_state->format_flags |= OUTPUT_FORMAT_CBREAK;		break;
			case GENERIC_OUTPUT_FLAG_NUMBER:	draw_state->format_flags |= OUTPUT_FORMAT_NUMBERED;		break;
			case GENERIC_OUTPUT_FLAG_REFERENCE:	draw_state->format_flags |= OUTPUT_FORMAT_REFERENCE;	break;
		}
	}
	else
	{
		switch(flag_value->id)
		{
			case GENERIC_OUTPUT_FLAG_MARGIN:		draw_state->margin_width = flag_value->value.number;	break;
			case GENERIC_OUTPUT_FLAG_LEVELS:		draw_state->number_level = flag_value->value.number;	break;
			case GENERIC_OUTPUT_FLAG_FORMAT:
			case GENERIC_OUTPUT_FLAG_NUMBER_STYLE:
				flag_value->value.string.name			= flag_value->value.string.name;
				flag_value->value.string.name_length	= flag_value->value.string.name_length;
			break;
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name  output_parse_flags
 * desc  This function will parse the input flags. If the flags are not one of
 *       the standard flags then the input will be passed onto the specified 
 *       formats to see if it is known.
 *--------------------------------------------------------------------------------*/
static void	output_parse_flags(INPUT_STATE* input_state, DRAW_STATE* draw_state)
{
	unsigned int		pos = 0;
	unsigned int		hash;
	unsigned int		res_1;
	unsigned int		res_2;
	unsigned int		last_flag = 0;
	unsigned int		last_equals = 0;
	unsigned int		flag_length;
	NAME				value;
	OUTPUT_FLAG_VALUE	flag_value;

	/* reset the format flags */
	draw_state->margin_width = draw_state->global_margin_width;
	draw_state->format_flags = draw_state->global_format_flags;
	draw_state->number_level = draw_state->global_number_level;
	draw_state->max_constant_size = draw_state->global_max_constant;

	if (input_state->flags.name_length > 0)
	{
		while (pos < input_state->flags.name_length)
		{
			if (input_state->flags.name[pos] == '=')
			{
				last_equals = pos;
			}
			else if (input_state->flags.name[pos] == ',')
			{
				if (last_equals == last_flag)
				{
					value.name			= &input_state->flags.name[last_equals+1];
					value.name_length	= 0;
					flag_length			= pos - last_equals;
				}
				else
				{
					value.name			= &input_state->flags.name[last_equals+1];
					value.name_length	= pos - last_equals - 1;
					flag_length			= last_equals - last_flag;
				}

				hash = fnv_32_hash(&input_state->flags.name[last_flag],flag_length);

				/* look in the global list first, then in the format specific one after */
				res_1 = output_find_flag(&g_flag_list,hash,&value,&flag_value);
				res_2 =	draw_state->format->decode_flags(draw_state,input_state,hash,&value);
				
				if (res_1 == OUTPUT_FLAG_TYPE_INVALID && res_2 == OUTPUT_FLAG_TYPE_INVALID)
				{
					raise_warning(input_state->line_number,EC_UNKNOWN_OUTPUT_FLAG,value.name,NULL);
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
		if (pos > last_flag && pos > 0)
		{
			if (last_equals == last_flag)
			{
				value.name			= &input_state->flags.name[last_equals+1];
				value.name_length	= 0;
				flag_length			= pos - last_flag;
			}
			else
			{
				value.name			= &input_state->flags.name[last_equals+1];
				value.name_length	= pos - last_equals - 1;
				flag_length			= last_equals - last_flag;
			}

			hash = fnv_32_hash(&input_state->flags.name[last_flag],flag_length);

			/* look in the global list first, then in the format specific one after */
			res_1 = output_find_flag(&g_flag_list,hash,&value,&flag_value);
			res_2 =	draw_state->format->decode_flags(draw_state,input_state,hash,&value);

			if (res_1 == OUTPUT_FLAG_TYPE_INVALID && res_2 == OUTPUT_FLAG_TYPE_INVALID)
			{
				raise_warning(input_state->line_number,EC_UNKNOWN_OUTPUT_FLAG,value.name,NULL);
			}
			else
			{
				/* set the output flags */
				output_set_flag(draw_state,&flag_value);
			}
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_function_prototype
 * @desc: This function will output the functions prototype.
 *--------------------------------------------------------------------------------*/
static void	output_function_prototype(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	TABLE_ROW			table_row;
	TABLE_LAYOUT		table_layout;
	API_PARAMETER*		current_parameter;

	/* set the table row up */
	table_layout.num_columns = 6;
	table_layout.table_flags = OUTPUT_TABLE_FORMAT_PROTOTYPE;
	table_layout.column_spacing = 1;
	table_layout.column[0].flags = 0;
	table_layout.column[1].flags = 0;
	table_layout.column[2].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;
	table_layout.column[3].flags = 0;
	table_layout.column[4].flags = 0;
	table_layout.column[5].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;

	table_layout.column[0].style = OUTPUT_TEXT_STYLE_NORMAL;
	table_layout.column[1].style = OUTPUT_TEXT_STYLE_BOLD;
	table_layout.column[2].style = OUTPUT_TEXT_STYLE_NORMAL;
	table_layout.column[3].style = OUTPUT_TEXT_STYLE_NORMAL;
	table_layout.column[4].style = OUTPUT_TEXT_STYLE_NORMAL;
	table_layout.column[5].style = OUTPUT_TEXT_STYLE_NORMAL;
	
	/* ok. where do the columns start */
	table_layout.column[0].width  = function->return_type.name_length;
	table_layout.column[1].width  = function->name.name_length;
	table_layout.column[2].width  = 1;
	table_layout.column[3].width  = function->max_param_type_length;
	table_layout.column[4].width  = function->max_param_name_length;
	table_layout.column[5].width  = 1;
	
	draw_state->format->output_table_start(draw_state,&table_layout);

	table_row.row[0] = &function->return_type;
	table_row.row[1] = &function->name;
	
	table_row.row[2] = &name_open_round;
	table_row.row[5] = &name_comma;

	if (function->parameter_list == NULL)
	{
		table_row.row[3] = NULL;
		table_row.row[4] = NULL;
		table_row.row[5] = &name_close_round;
		draw_state->format->output_table_row(draw_state,&table_layout,&table_row);
	}
	else
	{
		current_parameter = function->parameter_list;

		do
		{
			if (current_parameter->next == NULL)
			{
				table_row.row[5] = &name_close_round;
			}

			table_row.row[3] = &current_parameter->type;
			table_row.row[4] = &current_parameter->name;
			draw_state->format->output_table_row(draw_state,&table_layout,&table_row);

			table_row.row[0] = NULL;
			table_row.row[1] = NULL;
			table_row.row[2] = NULL;

			current_parameter = current_parameter->next;
		}
		while (current_parameter != NULL);
	}

	draw_state->format->output_table_end(draw_state,&table_layout);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_function_parameters
 * @desc: This function will output the functions parameter list.
 *--------------------------------------------------------------------------------*/
static void	output_function_parameters(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	unsigned int	max_length;
	TABLE_ROW		table_row;
	TABLE_LAYOUT	table_layout;
	API_PARAMETER*	current_parameter;

	/* ok. where do the columns start */
	table_layout.num_columns = 3;
	table_layout.table_flags = (OUTPUT_TABLE_FORMAT_HEADER | OUTPUT_TABLE_FORMAT_BOXED | OUTPUT_TABLE_FORMAT_FULL_WIDTH);
	table_layout.column[0].flags = 0;
	table_layout.column[1].flags = 0;
	table_layout.column[2].flags = OUTPUT_COLUMN_FORMAT_WORD_WRAP;

	/* how wide are they */
	table_layout.column[0].width = function->max_param_name_length;
	table_layout.column[1].width = function->max_param_type_length;
	table_layout.column[2].width = 0;
	
	/* set the style */
	table_layout.column[0].style	= OUTPUT_TEXT_STYLE_BOLD;
	table_layout.column[1].style	= 0;
	table_layout.column[2].style	= 0;

	/* start the table */
	draw_state->format->output_table_start(draw_state,&table_layout);
	
	/* output the header */
	table_row.row[0] = &name_name;
	table_row.row[1] = &name_type;
	table_row.row[2] = &name_description;

	draw_state->format->output_table_header(draw_state,&table_layout,&table_row);

	/* now output the rest of the table */
	current_parameter = function->parameter_list;

	while (current_parameter != NULL)
	{
		table_row.row[0] = &current_parameter->name;
		table_row.row[1] = &current_parameter->type;
		table_row.row[2] = &current_parameter->brief;

		draw_state->format->output_table_row(draw_state,&table_layout,&table_row);
	
		current_parameter = current_parameter->next;
	}
	
	draw_state->format->output_table_end(draw_state,&table_layout);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name : output_function_returns
 * Desc : This function will output the records that make up the type.
 *--------------------------------------------------------------------------------*/
static void	output_function_returns(DRAW_STATE* draw_state, API_FUNCTION* function)
{
	API_RETURNS*	current_returns = function->returns_list;
	TABLE_ROW		table_row;
	TABLE_LAYOUT	table_layout;
	
	draw_state->format->output_marker(draw_state,OUTPUT_MARKER_NO_INDEX|OUTPUT_MARKER_HEADER_START|OHL_LEVEL_3);
	draw_state->format->output_text(draw_state,OUTPUT_TEXT_STYLE_NORMAL,&name_returns);
	draw_state->format->output_marker(draw_state,OUTPUT_MARKER_HEADER_END|OHL_LEVEL_3);

	/* ok. where do the columns start */
	table_layout.num_columns = 2;
	table_layout.table_flags = (OUTPUT_TABLE_FORMAT_FULL_WIDTH);
	table_layout.column[0].flags = 0;
	table_layout.column[1].flags = OUTPUT_COLUMN_FORMAT_WORD_WRAP;

	/* set the widths */
	table_layout.column[0].width  = function->max_return_value_length;
	table_layout.column[1].width  = 0;
	
	/* set the style */
	table_layout.column[0].style	= 0;
	table_layout.column[1].style	= 0;

	/* start the table */
	draw_state->format->output_table_start(draw_state,&table_layout);

	/* output table */
	while (current_returns != NULL)
	{
		table_row.row[0] = &current_returns->value;
		table_row.row[1] = &current_returns->brief;
		draw_state->format->output_table_row(draw_state,&table_layout,&table_row);

		current_returns = current_returns->next;
	}

	draw_state->format->output_table_end(draw_state,&table_layout);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_type_records
 * @desc: This function will output the type records.
 *--------------------------------------------------------------------------------*/
static void	output_type_records(DRAW_STATE* draw_state, API_TYPE* type)
{
	TABLE_ROW			table_row;
	TABLE_LAYOUT		table_layout;
	API_TYPE_RECORD*	current_record;

	/* ok. where do the columns start */
	table_layout.num_columns = 3;
	table_layout.table_flags = (OUTPUT_TABLE_FORMAT_HEADER | OUTPUT_TABLE_FORMAT_BOXED | OUTPUT_TABLE_FORMAT_FULL_WIDTH);
	table_layout.column_spacing = 2;
	table_layout.column[0].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;
	table_layout.column[1].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;
	table_layout.column[2].flags = OUTPUT_COLUMN_FORMAT_WORD_WRAP;

	/* set the widths */
	table_layout.column[0].width  = type->max_name_value_length;
	table_layout.column[1].width  = type->max_type_item_length;
	table_layout.column[2].width  = 0;

	/* draw the header */
	draw_state->format->output_table_start(draw_state,&table_layout);
	
	/* output the header */
	table_row.row[0] = &name_name;
	table_row.row[1] = &name_type;
	table_row.row[2] = &name_description;

	draw_state->format->output_table_header(draw_state,&table_layout,&table_row);

	current_record = type->record_list;

	while (current_record != NULL)
	{
		if (current_record->name_value.name_length > 0 && current_record->type_item.name_length > 0)
		{
			table_row.row[0] = &current_record->name_value;
			table_row.row[1] = &current_record->type_item;
			table_row.row[2] = &current_record->brief;

			draw_state->format->output_table_row(draw_state,&table_layout,&table_row);
		}
		current_record = current_record->next;
	}
	draw_state->format->output_table_end(draw_state,&table_layout);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_constant_records
 * @desc: This function will output the constants. 
 *        it will output them in two formats. The constants the will fit in a table
 *        without wrapping will be output in tabular form. Then the larger
 *        constants will be output in a section block format that will make the
 *        constant easier to read.
 *--------------------------------------------------------------------------------*/
static void	output_constant_records(DRAW_STATE* draw_state, API_CONSTANTS* constant)
{
	TABLE_ROW		table_row;
	TABLE_LAYOUT	table_layout;
	API_CONSTANT*	current_constant;
	unsigned int	max_value_size = draw_state->max_constant_size;

	if (max_value_size > constant->max_value_length)
	{
		max_value_size = constant->max_value_length;
	}

	/* ok. where do the columns start */
	table_layout.num_columns = 3;
	table_layout.table_flags = (OUTPUT_TABLE_FORMAT_HEADER | OUTPUT_TABLE_FORMAT_BOXED | OUTPUT_TABLE_FORMAT_FULL_WIDTH);
	table_layout.column[0].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;
	table_layout.column[1].flags = 0;
	table_layout.column[2].flags = OUTPUT_COLUMN_FORMAT_WORD_WRAP;
	
	/* ok, lets start with the min name length */
	current_constant = constant->constant_list;

	table_layout.column[0].width = constant->max_name_length;
	table_layout.column[1].width = max_value_size;
	table_layout.column[2].width = 0;

	draw_state->format->output_table_start(draw_state,&table_layout);
	
	/* output the header */
	table_row.row[0] = &name_name;
	table_row.row[1] = &name_constant;
	table_row.row[2] = &name_description;

	draw_state->format->output_table_header(draw_state,&table_layout,&table_row);
	
	/* do the items that fit in the table */
	current_constant = constant->constant_list;

	while (current_constant != NULL)
	{
		/* large constants need to handled differently */
		if (current_constant->value.name_length < draw_state->max_constant_size )
		{
			/* don't print the special fields in the table - only the actual records */
			if (current_constant->name.name_length > 0)
			{
				table_row.row[0] = &current_constant->name;
				table_row.row[1] = &current_constant->value;
				table_row.row[2] = &current_constant->brief;

				draw_state->format->output_table_row(draw_state,&table_layout,&table_row);
			}
		}

		current_constant = current_constant->next;
	}
	
	draw_state->format->output_table_end(draw_state,&table_layout);

	current_constant = constant->constant_list;

	while (current_constant != NULL)
	{
		/* only interested in the long items */
		if (current_constant->value.name_length >= draw_state->max_constant_size)
		{
			/* don't print the special fields in the table - only the actual records */
			if (current_constant->name.name_length > 0 && current_constant->value.name_length > 0)
			{
				draw_state->format->output_section(	draw_state,
													OHL_LEVEL_3,
													&current_constant->name,
													OUTPUT_COLUMN_FORMAT_WORD_WRAP,
													&current_constant->brief);

				draw_state->format->output_block(draw_state,OUTPUT_COLUMN_FORMAT_WRAP_COMPLEX,&current_constant->value);
			}
		}

		current_constant = current_constant->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_application_option
 * @desc: This output application option.
 *--------------------------------------------------------------------------------*/
static void	output_application_option(DRAW_STATE* draw_state, APPLICATION* application,unsigned int parts, NAME* part_name)
{
	OPTION*		option;
	TABLE_ROW		table_row;
	TABLE_LAYOUT	table_layout;
	
	if (application->option_list != NULL)
	{
		/* write the title */
		draw_state->format->output_title(draw_state,OHL_LEVEL_2,&name_options,NULL);

		/* now write the table */
		table_layout.num_columns = 2;
		table_layout.table_flags = 0;
		table_layout.column[0].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;
		table_layout.column[1].flags = OUTPUT_COLUMN_FORMAT_WORD_WRAP;
		
		table_layout.column[0].style = OUTPUT_TEXT_STYLE_BOLD;

		table_layout.column[0].width = application->max_option_length;
		table_layout.column[1].width = draw_state->page_width - 18;

		/* start the table */
		draw_state->format->output_table_start(draw_state,&table_layout);

		/* now write the options to the file */
		option = application->option_list;

		while (option != NULL)
		{
			if ((parts & OUTPUT_APPLICATION_MULTIPLE) || compare_name(&option->name,part_name) == 0)
			{
				table_row.row[0] = &option->name;
				table_row.row[1] = &option->value;
				draw_state->format->output_table_row(draw_state,&table_layout,&table_row);

				table_row.row[0] = NULL;
				table_row.row[1] = &option->description;
				draw_state->format->output_table_row(draw_state,&table_layout,&table_row);
			}

			option = option->next;
		}
	
		draw_state->format->output_table_end(draw_state,&table_layout);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_application_command
 * @desc: This function will list the list of commands that belong to the 
 *        application.
 *--------------------------------------------------------------------------------*/
static void	output_application_command(DRAW_STATE* draw_state, APPLICATION* application,unsigned int parts, NAME* part_name)
{
	COMMAND*		command;
	TABLE_ROW		table_row;
	TABLE_LAYOUT	table_layout;
	
	if (application->command_list != NULL)
	{
		/* write the title */
		draw_state->format->output_title(draw_state,OHL_LEVEL_2,&name_commands,NULL);

		/* now write the table */
		table_layout.num_columns = 2;
		table_layout.table_flags = (OUTPUT_TABLE_FORMAT_HEADER | OUTPUT_TABLE_FORMAT_BOXED | OUTPUT_TABLE_FORMAT_FULL_WIDTH);
		table_layout.column[0].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;
		table_layout.column[1].flags = OUTPUT_COLUMN_FORMAT_WORD_WRAP;
		
		table_layout.column[0].style = OUTPUT_TEXT_STYLE_BOLD;

		table_layout.column[0].width = application->max_command_length;
		table_layout.column[1].width = draw_state->page_width - 18;

		/* start the table */
		draw_state->format->output_table_start(draw_state,&table_layout);

		/* now write the commands to the file */
		command = application->command_list;

		while (command != NULL)
		{
			if ((parts & OUTPUT_APPLICATION_MULTIPLE) || compare_name(&command->name,part_name) == 0)
			{
				table_row.row[0] = &command->name;
				table_row.row[1] = &command->parameters;
				draw_state->format->output_table_row(draw_state,&table_layout,&table_row);

				table_row.row[0] = NULL;
				table_row.row[1] = &command->description;
				draw_state->format->output_table_row(draw_state,&table_layout,&table_row);
			}

			command = command->next;
		}
	
		draw_state->format->output_table_end(draw_state,&table_layout);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_application_section
 * @desc: This function will output the all or the named section of the  given
 *        application.
 *--------------------------------------------------------------------------------*/
static void	output_application_section(DRAW_STATE* draw_state, APPLICATION* application,unsigned int parts, NAME* part_name)
{
	SECTION*	section;
	SECTION*	sub_section;

	section = application->section_list;
	
	while (section != NULL)
	{
		if ((parts & OUTPUT_APPLICATION_MULTIPLE) || compare_name(&section->name,part_name) == 0)
		{
			draw_state->format->output_section(draw_state,OHL_LEVEL_2,&section->name,OUTPUT_COLUMN_FORMAT_WORD_WRAP,&section->section_data);

			if (section->sub_section_list != NULL)
			{
				sub_section = section->sub_section_list;

				while (sub_section != NULL)
				{
					draw_state->format->output_section(	draw_state,
														OHL_LEVEL_3,
														&sub_section->name,
														OUTPUT_COLUMN_FORMAT_WORD_WRAP,
														&sub_section->section_data);

					sub_section = sub_section->next;
				}
			}
		}

		section = section->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_application_synopsis
 * @desc: This function will output the synopsis for the application.
 *--------------------------------------------------------------------------------*/
static void	output_application_synopsis(DRAW_STATE* draw_state, APPLICATION* application,unsigned int parts, NAME* part_name)
{
	SYNOPSIS*		synopsis;
	unsigned int	count;

	if (application->synopsis != NULL)
	{
		/* title */
		draw_state->format->output_title(draw_state,OHL_LEVEL_2,&name_synopsis,NULL);
		
		synopsis = application->synopsis;

		while (synopsis != NULL)
		{
			draw_state->format->output_text(draw_state,OUTPUT_TEXT_STYLE_BOLD|OUTPUT_TEXT_STYLE_T_SPACE,&application->name);

			if ((parts & OUTPUT_APPLICATION_MULTIPLE) || compare_name(&synopsis->name,part_name) == 0)
			{
				if (synopsis->list != NULL)
				{
					for (count=0; count < synopsis->list_length; count++)
					{
						if ((synopsis->list[count]->flags & OPTION_FLAG_REQUIRED) == 0)
						{
							draw_state->format->output_text(draw_state,OUTPUT_TEXT_STYLE_NORMAL,&name_open_square);
						}
			
						draw_state->format->output_text(draw_state,OUTPUT_TEXT_STYLE_BOLD,&synopsis->list[count]->name);

						if (synopsis->list[count]->value.name_length > 0)
						{
							draw_state->format->output_text(draw_state,
															(OUTPUT_TEXT_STYLE_NORMAL|OUTPUT_TEXT_STYLE_T_SPACE),
															&synopsis->list[count]->value);
						}
						
						if ((synopsis->list[count]->flags & OPTION_FLAG_MULTIPLE) != 0)
						{
							draw_state->format->output_text(draw_state,OUTPUT_TEXT_STYLE_NORMAL|OUTPUT_TEXT_STYLE_T_SPACE,&name_ellipsis);
						}

						if ((synopsis->list[count]->flags & OPTION_FLAG_REQUIRED) == 0)
						{
							draw_state->format->output_text(draw_state,OUTPUT_TEXT_STYLE_NORMAL|OUTPUT_TEXT_STYLE_T_SPACE,&name_close_square);
						}
					}
				}
			}
						
			draw_state->format->output_text(draw_state,OUTPUT_TEXT_STYLE_T_NEWLINE,NULL);
			
			synopsis = synopsis->next;
		}
	
		draw_state->format->output_text(draw_state,OUTPUT_TEXT_STYLE_T_NEWLINE,NULL);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_name
 * @desc: This function is a used to output names, this is wrapped so it can
 *        handle inline names without breaking the title api.
 *--------------------------------------------------------------------------------*/
static void	output_name(DRAW_STATE* draw_state, NAME* name)
{
	if ((draw_state->format_flags & OUTPUT_FORMAT_INLINE) != 0)
	{
		/* ok, the name is to be output in line */
		draw_state->format->output_text(draw_state,(OUTPUT_TEXT_STYLE_CODE|OUTPUT_TEXT_STYLE_T_SPACE),name);
	}
	else
	{
		draw_state->format->output_title(draw_state,OHL_LEVEL_2,&name_cname,name);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_index_section
 * Desc : This function will walk the index tree and output the index from the
 *        section specified until the specified level. It will only display the
 *        children of the given level.
 *--------------------------------------------------------------------------------*/
void	output_index_section(DRAW_STATE* draw_state, DOC_SECTION* start_section)
{
	DOC_SECTION*	current_section = start_section->child;
	unsigned char	spaces[200];
	unsigned int	count = 0;
	unsigned int	level = start_section->level;
	unsigned int	start;

	unsigned char	label[200];
	memset(spaces,' ',200);

	while (current_section != NULL && current_section->level > level)
	{
		start = index_generate_label(current_section,0,INDEX_STYLE_NUMBERS,label,200,'_');

		if (current_section->child != NULL)
		{
			draw_state->format->output_index_start_sublevel(draw_state,current_section);
			current_section = current_section->child;
			count++;
		}
		else if (current_section->next != NULL)
		{
			draw_state->format->output_index_entry(draw_state,current_section);
			current_section = current_section->next;
		}
		else
		{
			draw_state->format->output_index_entry(draw_state,current_section);

			while(current_section != NULL && current_section->next == NULL)
			{
				current_section = current_section->parent;
	
				if (current_section->level <= level)
				{
					break;
				}
				else if (current_section != NULL && current_section->level != 0)
				{
					draw_state->format->output_index_end_sublevel(draw_state,current_section);
					count--;
				}

			}

			if (current_section != NULL)
			{
				current_section = current_section->next;
			}
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : output_api_index
 * Desc : This function will output the specific api index.
 *--------------------------------------------------------------------------------*/
static void	output_api_index(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned int api_flags, GROUP* group)
{
	CHAPTER* 		current_chapter = draw_state->model->document->chapter_list.next;
	DOC_SECTION*	current_section;
	LEVEL_INDEX		level_index;
	unsigned short	current_level;
	unsigned short	min_level;

	level_index.level_index = NULL;
	level_index_init(&level_index);

	while (current_chapter != NULL)
	{
		level_index_set_level(&level_index,0);

		/* is it the right chapter? */
		if (draw_state->chapter == current_chapter->index)
		{
			current_section = current_chapter->root.child;
			current_level   = 1;

			while (current_section != NULL)
			{
				level_index_set_level(&level_index,current_section->level);
				level_index_set(&level_index,current_section->index);

				if (current_section->api_index_group == group && current_section->api_index_flags == api_flags)
				{
					/* Ok, found the one we are looking for - it must be inline */
					draw_state->format_flags |= OUTPUT_FORMAT_LEVEL;

					input_state->state = TYPE_INDEX;
					draw_state->format->output_header(draw_state,input_state);
					output_index_section(draw_state,current_section);
					draw_state->format->output_footer(draw_state,input_state);
					break;
				}

				/* walk the tree */
				if (current_section->child != NULL)
				{
					current_section = current_section->child;
				}
				else if (current_section->next == NULL)
				{
					current_section = current_section->parent;

					if (current_section != NULL)
					{
						current_section = current_section->next;
					}
				}
				else
				{
					current_section = current_section->next;
				}
			}

			/* we only search one chapter */
			break;
		}
		current_chapter = current_chapter->next;
	}

	level_index_release(&level_index);
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: output_document_index
 *  desc: This function will output the document index for the document.
 *        It will output the index based on the level specified, if the level is
 *        0, then the index is based on the current index level of the document.
 *--------------------------------------------------------------------------------*/
void	output_document_index(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	CHAPTER* 		current_chapter = draw_state->model->document->chapter_list.next;
	DOC_SECTION*	current_section;
	LEVEL_INDEX		level_index;
	unsigned short	current_level;
	unsigned short	min_level;

	memset(&level_index,0,sizeof(LEVEL_INDEX));
	level_index_init(&level_index);

	if ((draw_state->format_flags & OUTPUT_FORMAT_LEVEL) == 0)
	{	
		input_state->state = TYPE_INDEX;
		draw_state->format->output_header(draw_state,input_state);

		/* Ok, dump the whole index */
		while (current_chapter != NULL)
		{
			level_index_set_level(&level_index,0);
			output_index_section(draw_state,&current_chapter->root);
			current_chapter = current_chapter->next;
		}
			
		draw_state->format->output_footer(draw_state,input_state);
	}
	else
	{
		min_level = level_index_get_level(&draw_state->index);

		while (current_chapter != NULL)
		{
			level_index_set_level(&level_index,0);

			/* is it the right chapter? */
			if (draw_state->chapter == current_chapter->index)
			{
				current_section = current_chapter->root.child;
				current_level   = 1;

				while (current_section != NULL)
				{
					level_index_set_level(&level_index,current_section->level);
					level_index_set(&level_index,current_section->index);

					/* Ok, we have found the level item we are inserted in */
					if (current_level == min_level && current_section->index == level_index_get_index(&draw_state->index,current_level))
					{
						/* Ok, we are here, dump it */
						input_state->state = TYPE_INDEX;
						draw_state->format->output_header(draw_state,input_state);
						output_index_section(draw_state,current_section);
						draw_state->format->output_footer(draw_state,input_state);
						break;
					}
					else if (current_section->child != NULL)
					{
						/* Ok, do down a level in the index */
						current_level++;
						current_section = current_section->child;
					}
					else
					{
						current_section = current_section->next;
					}
				}

				/* we only search one chapter */
				break;
			}
			current_chapter = current_chapter->next;
		}
	}

	level_index_release(&level_index);
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: output_decode_model
 *  desc: This function will decode the model.
 *        The model format is: 
 *        [docgen:/<group>/<type>/<part>/<item>/<sub_part>?<flags>]
 *--------------------------------------------------------------------------------*/
static unsigned int	output_decode_model(DRAW_STATE* draw_state, INPUT_STATE* input_state, NAME* model)
{
	unsigned int state = 0;
	unsigned int start = 0;
	unsigned int count;
	unsigned int result = EC_OK;
	LOOKUP_ITEM* macro_item;

	/* clear the fields */
	input_state->group_name.name_length = 0;
	input_state->type_name.name_length = 0;
	input_state->part_name.name_length = 0;
	input_state->item_name.name_length = 0;
	input_state->sub_part.name_length = 0;
	input_state->flags.name_length = 0;

	input_state->group_name.name = &model->name[1];

	for (count=1; count < model->name_length; count++)
	{
		if (model->name[count] == '/' || model->name[count] == '?')
		{
			/* handle the path dividers */
			switch(state)
			{
				case 0:	/* group */
						input_state->group_name.name_length = count - 1;
						input_state->type_name.name = &model->name[count+1];
						start = count+1;
						state = 1;
						break;

				case 1: /* type */
						input_state->type_name.name_length = count - start;
						input_state->part_name.name = &model->name[count+1];
						start = count+1;
						state = 2;
						break;

				case 2:	/* part */
						input_state->part_name.name_length = count - start;
						input_state->item_name.name = &model->name[count+1];
						start = count+1;
						state = 3;
						break;

				case 3: /* item_name */
						input_state->item_name.name_length = count - start;
						input_state->sub_part.name = &model->name[count+1];
						start = count+1;
						state = 4;
						break;

				case 4: /* sub_part */
						input_state->sub_part.name_length = count - start;
						start = count+1;
						state = 5;
						break;

				default:
						result = EC_MODEL_BADLY_FORMATTED;
						raise_warning(input_state->line_number,result,input_state->input_name,0);
			}
		}

		if (model->name[count] == '?')
		{
			if (input_state->group_name.name_length == 0)
			{
				/* must have at least a name */
				result = EC_MODEL_BADLY_FORMATTED;
				raise_warning(input_state->line_number,result,input_state->input_name,0);
			}
			else
			{
				/* handle flags */
				input_state->flags.name = &model->name[count+1];
				input_state->flags.name_length = model->name_length - count - 1;
			}
			break;
		}
	}


	/* tidy up the lose ends */
	if (count > start)
	{
		switch(state)
		{
			case 0:	/* group */
				input_state->group_name.name_length = count;
				break;

			case 1:	/* type */
				input_state->type_name.name_length = count - start;
				break;

			case 2:	/* part */
				input_state->part_name.name_length = count - start;
				break;

			case 3: /* item_name */
				input_state->item_name.name_length = count - start;
				break;

			case 4: /* sub_part */
				input_state->sub_part.name_length = count - start;
				break;
		}
	}

	/* now check for macros */
	if (input_state->item_name.name_length > 0 && input_state->item_name.name[0] == '$')
	{
		/* it's a macro */
		macro_item = find_lookup(	&draw_state->macro_lookup,
				&input_state->item_name.name[1],
				input_state->item_name.name_length-1);

		if (macro_item == NULL)
		{
			result = EC_UNKNOWN_MACRO;
			raise_warning(input_state->line_number,result,(unsigned char*)input_state->input_name,NULL);
		}
		else
		{
			/* insert the macro */
			input_state->item_name.name = macro_item->payload;
			input_state->item_name.name_length = macro_item->payload_length;
		}
	}

	if (input_state->group_name.name_length > 0 && input_state->group_name.name[0] == '$')
	{
		/* it's a macro */
		macro_item = find_lookup(	&draw_state->macro_lookup,
				&input_state->group_name.name[1],
				input_state->group_name.name_length-1);

		if (macro_item == NULL)
		{
			result = EC_UNKNOWN_MACRO;
			raise_warning(input_state->line_number,result,(unsigned char*)input_state->input_name,NULL);
		}
		else
		{
			/* insert the macro */
			input_state->group_name.name = macro_item->payload;
			input_state->group_name.name_length = macro_item->payload_length;
		}
	}
	
	/* now decode the part */
	if (input_state->type_name.name_length > 0)
	{
		/* ok, we have a part - so lets see what it is */
		if (input_state->type_name.name_length == sizeof(TYPE_STATE_MACHINE_STR)-1 &&
			memcmp(input_state->type_name.name,TYPE_STATE_MACHINE_STR,input_state->type_name.name_length) == 0)
		{
			input_state->state = TYPE_STATE_MACHINE;
		}
		else if (input_state->type_name.name_length == sizeof(TYPE_SEQUENCE_DIAGRAM_STR)-1 &&
			memcmp(input_state->type_name.name,TYPE_SEQUENCE_DIAGRAM_STR,input_state->type_name.name_length) == 0)
		{
			input_state->state = TYPE_SEQUENCE_DIAGRAM;
		}
		else if (input_state->type_name.name_length == sizeof(TYPE_API_STR)-1 &&
			memcmp(input_state->type_name.name,TYPE_API_STR,input_state->type_name.name_length) == 0)
		{
			input_state->state = TYPE_API;
		}
		else if (input_state->type_name.name_length == sizeof(TYPE_SAMPLE_STR)-1 &&
			memcmp(input_state->type_name.name,TYPE_SAMPLE_STR,input_state->type_name.name_length) == 0)
		{
			input_state->state = TYPE_SAMPLE;
		}
		else if (input_state->type_name.name_length == sizeof(TYPE_APPLICATION_STR)-1 &&
			memcmp(input_state->type_name.name,TYPE_APPLICATION_STR,input_state->type_name.name_length) == 0)
		{
			input_state->state = TYPE_APPLICATION;
		}
		else
		{
			result = EC_MODEL_BADLY_FORMATTED;
			raise_warning(input_state->line_number,result,input_state->input_name,0);
		}
	}
	

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: output_document_specials
 *  desc: This function will handle the references to the document group.
 *--------------------------------------------------------------------------------*/
static unsigned int	output_document_specials(DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group)
{
	unsigned int result = EC_OK;

	if (input_state->item_name.name_length == 0 && draw_state->model->document != NULL)
	{
		/* no item then we are looking at the document level */
		if ((draw_state->format_flags & OUTPUT_FORMAT_INDEX) != 0)
		{
			output_document_index(draw_state,input_state);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_api
 * Desc : This function output an API.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_api( DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group, unsigned int special_group)
{
	GROUP*				current_group = group;
	API_TYPE*			current_type;
	API_FUNCTION*		current_function;
	API_CONSTANTS*		current_constants;
	unsigned int		flags = 0;
	unsigned int		update_indexs = 0;

	if (!decode_api_item(input_state,&flags))
	{
		raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (group->api == NULL && special_group != ALL_GROUP)
	{
		raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if ((draw_state->format_flags & OUTPUT_FORMAT_INDEX) != 0)
	{
		output_api_index(draw_state,input_state,flags,group);
	}
	else
	{
		do
		{
			if (current_group->api != NULL)
			{
				/* output the function parts */
				if ((flags & OUTPUT_API_FUNCTIONS) != 0)
				{
					if (input_state->item_name.name_length == 0)
					{
						current_function = current_group->api->function_list;
					}
					else
					{
						current_function = current_group->api->function_list;
					
						while (current_function != NULL)
						{
							if (compare_name(&input_state->item_name,&current_function->name) == 0)
							{
								/* found it */
								break;
							}
							current_function = current_function->next;
						}
					}

					if (current_function == NULL)
					{
						/* only warn if we are not dumping all */
						if (special_group != ALL_GROUP)
						{
							raise_warning(input_state->line_number,EC_UNDEFINED_FUNCTION,input_state->item_name.name,NULL);
						}
					}
					else
					{
						do
						{
							/* first generate the header for the API */
							input_state->title.name = current_function->name.name;
							input_state->title.name_length = current_function->name.name_length;
							input_state->index_item = current_function->index_item;

							draw_state->format->output_header(draw_state,input_state);

							if ((flags & OUTPUT_API_FUNCTION_NAME) != 0)
							{
								output_name(draw_state,&current_function->name);
							}

							if ((flags & OUTPUT_API_FUNCTION_DESCRIPTION) != 0)
							{
								draw_state->format->output_section(	draw_state,
																	OHL_LEVEL_3,
																	&name_cdescription,
																	OUTPUT_COLUMN_FORMAT_WORD_WRAP,
																	&current_function->description);
							}

							if ((flags & OUTPUT_API_FUNCTION_PROTOTYPE) != 0)
							{
								output_function_prototype(draw_state,current_function);
							}

							if ((flags & OUTPUT_API_FUNCTION_PARAMETERS) != 0)
							{
								output_function_parameters(draw_state,current_function);
							}

							if ((flags & OUTPUT_API_FUNCTION_ACTION) != 0)
							{
								draw_state->format->output_section(	draw_state,
																	OHL_LEVEL_3,
																	&name_action,
																	OUTPUT_COLUMN_FORMAT_WORD_WRAP,
																	&current_function->action);
							}

							if ((flags & OUTPUT_API_FUNCTION_RETURNS) != 0)
							{
								output_function_returns(draw_state,current_function);
							}

							/* add the footer */
							draw_state->format->output_footer(draw_state,input_state);

							current_function = current_function->next;
						}
						while (current_function != NULL && (flags & OUTPUT_API_MULTIPLE));
					}
				}

				/* output the types */
				if ((flags & OUTPUT_API_TYPES) != 0)
				{
					if (input_state->item_name.name_length == 0)
					{
						current_type = current_group->api->type_list;
					}
					else
					{
						current_type = current_group->api->type_list;

						while (current_type != NULL)
						{
							if (compare_name(&input_state->item_name,&current_type->name) == 0)
							{
								/* found it */
								break;
							}
							current_type = current_type->next;
						}
					}

					if (current_type == NULL)
					{
						/* check to see if the user specifically asked for the type or not */
						if ((flags & OUTPUT_API_ALL) != OUTPUT_API_ALL)
						{
							if (special_group != ALL_GROUP)
							{
								raise_warning(input_state->line_number,EC_UNDEFINED_TYPE,input_state->item_name.name,NULL);
							}
						}
					}
					else
					{
						do
						{
							/* first generate the header for the API */
							input_state->title.name = current_type->name.name;
							input_state->title.name_length = current_type->name.name_length;
							input_state->index_item = current_type->index_item;

							draw_state->format->output_header(draw_state,input_state);

							if ((flags & OUTPUT_API_TYPE_NAME) != 0)
							{
								output_name(draw_state,&current_type->name);
							}

							if ((flags & OUTPUT_API_TYPE_RECORDS) != 0)
							{
								output_type_records(draw_state,current_type);
							}

							if ((flags & OUTPUT_API_TYPE_DESCRIPTION) != 0)
							{
								draw_state->format->output_section(	draw_state,
																	OHL_LEVEL_3,
																	&name_cdescription,
																	OUTPUT_COLUMN_FORMAT_WORD_WRAP,
																	&current_type->description);
							}

							/* first generate the header for the API */
							draw_state->format->output_footer(draw_state,input_state);

							current_type = current_type->next;
						}
						while(current_type != NULL && (flags & OUTPUT_API_MULTIPLE));
					}
				}

				/* output the constants */
				if ((flags & OUTPUT_API_CONSTANTS) != 0)
				{
					if (input_state->item_name.name_length == 0)
					{
						current_constants = current_group->api->constants_list;
					}
					else
					{
						current_constants = current_group->api->constants_list;

						while (current_constants != NULL)
						{
							if (compare_name(&input_state->item_name,&current_constants->name) == 0)
							{
								/* found it */
								break;
							}
							current_constants = current_constants->next;
						}
					}

					if (current_constants == NULL)
					{
						/* check to see if the user specifically asked for the constants or not */
						if ((flags & OUTPUT_API_ALL) != OUTPUT_API_ALL)
						{
							if (special_group != ALL_GROUP)
							{
								raise_warning(input_state->line_number,EC_UNDEFINED_CONSTANTS_GROUP,input_state->item_name.name,NULL);
							}
						}
					}
					else
					{
						do
						{
							/* first generate the header for the API */
							input_state->title.name = current_constants->name.name;
							input_state->title.name_length = current_constants->name.name_length;
							input_state->index_item = current_constants->index_item;

							draw_state->format->output_header(draw_state,input_state);

							if ((flags & OUTPUT_API_CONSTANTS_NAME) != 0)
							{
								output_name(draw_state,&current_constants->name);
							}

							if ((flags & OUTPUT_API_CONSTANTS_DESCRIPTION) != 0)
							{
								draw_state->format->output_section(	draw_state,
																	OHL_LEVEL_3,
																	&name_cdescription,
																	OUTPUT_COLUMN_FORMAT_WORD_WRAP,
																	&current_constants->description);
							}

							if ((flags & OUTPUT_API_CONSTANTS_CONSTANT) != 0)
							{
								output_constant_records(draw_state,current_constants);
							}

							/* first generate the footer for the API */
							draw_state->format->output_footer(draw_state,input_state);

							current_constants = current_constants->next;
						}
						while(current_constants != NULL && (flags & OUTPUT_API_MULTIPLE));
					}
				}
			}
			current_group = current_group->next;
		}
		while (current_group != NULL && special_group == ALL_GROUP);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_sample
 * Desc : This function output an sample.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_sample( DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group, unsigned int special_group)
{
	SAMPLE*	current_sample = NULL;

	/* only, support non-grouped samples are the moment */
	if (group->name_length != sizeof("default")-1 && memcmp(group->name,"default",sizeof("default")-1) && special_group != ALL_GROUP)
	{
		raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if ((draw_state->format_flags & OUTPUT_FORMAT_INDEX) != 0)
	{
		raise_warning(input_state->line_number,EC_INDEX_NOT_SUPPORTED_OF_THIS_ITEM,input_state->input_name,NULL);
	}
	else
	{
		current_sample = group->sample_list->next;

		while (current_sample != NULL)
		{
			if (compare_name(&input_state->part_name,&current_sample->name) == 0)
			{
				/* first generate the footer for the API */
				input_state->item_name.name = NULL;
				input_state->item_name.name_length = 0;

				draw_state->format->output_header(draw_state,input_state);
				
				draw_state->format->output_sample(draw_state,current_sample);
					
				/* first generate the footer for the API */
				draw_state->format->output_footer(draw_state,input_state);

				break;
			}

			current_sample = current_sample->next;
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: decode_application
 * @desc: This function will decide the application name.
 *--------------------------------------------------------------------------------*/
static unsigned int	decode_application(INPUT_STATE* input_state)
{
	unsigned int pos = 0;
	unsigned int result = OUTPUT_APPLICATION_ALL_PARTS | OUTPUT_APPLICATION_MULTIPLE;
	
	if (input_state->sub_part.name_length > 0)
	{
		switch (atoms_check_word(input_state->sub_part.name))
		{
			case ATOM_NAME:			result = (OUTPUT_APPLICATION_MULTIPLE | OUTPUT_APPLICATION_NAME);		break;
			case ATOM_SECTION:		result = (OUTPUT_APPLICATION_MULTIPLE | OUTPUT_APPLICATION_SECTION);	break;
			case ATOM_OPTION:		result = (OUTPUT_APPLICATION_MULTIPLE | OUTPUT_APPLICATION_OPTION);		break;
			case ATOM_COMMAND:		result = (OUTPUT_APPLICATION_MULTIPLE | OUTPUT_APPLICATION_COMMAND);	break;
			case ATOM_SYNOPSIS:		result = (OUTPUT_APPLICATION_MULTIPLE | OUTPUT_APPLICATION_SYNOPSIS);	break;
			default:
				raise_warning(input_state->line_number,EC_BAD_NAME_FORMAT_IN_REQUEST,input_state->sub_part.name,NULL);
				result = 0;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_application
 * Desc : This function output an application request.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_application( DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group, unsigned int special_group)
{
	NAME			title;
	SAMPLE*			current_sample = NULL;
	APPLICATION*	application;
	unsigned int	parts;

	/* only, support non-grouped applications */
	if (group->name_length != sizeof("default")-1 && memcmp(group->name,"default",sizeof("default")-1) && special_group != ALL_GROUP)
	{
		raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (!(parts = decode_application(input_state)))
	{
		raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if ((draw_state->format_flags & OUTPUT_FORMAT_INDEX) != 0)
	{
		raise_warning(input_state->line_number,EC_INDEX_NOT_SUPPORTED_OF_THIS_ITEM,input_state->input_name,NULL);
	}
	else
	{
		application = group->application;
		
		while (application != NULL)
		{
			if (compare_name(&input_state->part_name,&application->name) == 0)
			{
				break;
			}
			application = application->next;
		}

		if (application == NULL)
		{
			raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->part_name.name,NULL);
		}
		else
		{
			/* first generate the footer for the API */
			draw_state->format->output_header(draw_state,input_state);

			if (parts & OUTPUT_APPLICATION_NAME)
			{
				output_name(draw_state,&application->name);
			}

			if (parts & OUTPUT_APPLICATION_SYNOPSIS)
			{
				output_application_synopsis(draw_state,application,parts,&input_state->sub_part);
			}
	
			if (parts & OUTPUT_APPLICATION_OPTION)
			{
				output_application_option(draw_state,application,parts,&input_state->sub_part);
			}

			if (parts & OUTPUT_APPLICATION_COMMAND)
			{
				output_application_command(draw_state,application,parts,&input_state->sub_part);
			}

			if (parts & OUTPUT_APPLICATION_SECTION)
			{
				output_application_section(draw_state,application,parts,&input_state->sub_part);
			}

			/* first generate the footer for the API */
			draw_state->format->output_footer(draw_state,input_state);
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : tag_all_states
 * Desc : This function will untag all states in the state_machine.
 *--------------------------------------------------------------------------------*/
static void	tag_all_states(STATE_MACHINE* state_machine)
{
	STATE*	current_state = state_machine->state_list;

	while (current_state != NULL)
	{
		current_state->flags |= (FLAG_TAGGED | FLAG_ACTIVE);

		current_state = current_state->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : untag_all_states
 * Desc : This function will untag all states in the state_machine.
 *--------------------------------------------------------------------------------*/
static void	untag_all_states(STATE_MACHINE* state_machine)
{
	STATE*	current_state = state_machine->state_list;

	while (current_state != NULL)
	{
		current_state->flags &= ~(FLAG_TAGGED | FLAG_ACTIVE);

		current_state = current_state->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : tag_state
 * Desc : This function will tag the state and all the states that communicate
 *        with it. The main state is tagged as active and the other states are
 *        tagged as tagged so the generator knows which states it has to output.
 *--------------------------------------------------------------------------------*/
static void	tag_state(STATE* state)
{
	STATE_TRANSITION*	current_transition = state->transition_list;

	state->flags |= (FLAG_ACTIVE | FLAG_TAGGED);

	while(current_transition != NULL)
	{
		if (current_transition->next_state != NULL)
		{
			current_transition->next_state->flags |= FLAG_TAGGED;
		}

		current_transition = current_transition->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: handle_state_machine_table
 * @desc: This function will handle outputting the state machine as a table.
 *--------------------------------------------------------------------------------*/
static void	handle_state_machine_table(DRAW_STATE* draw_state, STATE_MACHINE* state_machine)
{
	NAME				temp = {NULL,0,0,0};
	NAME				group;
	NAME				state = {NULL,0,0,0};
	NAME				input = {NULL,0,0,0};
	NAME				triggers = {NULL,0,0,0};
	NAME				condition;
	NAME				next_state;
	STATE*				current_state = NULL;
	TRIGGERS*			current_triggers;
	TABLE_ROW			table_row;
	TABLE_LAYOUT		table_layout;
	STATE_TRANSITION*	current_trans;
	unsigned int		size = 0;

	/* create the table layout */
	/* ok. where do the columns start */
	table_layout.num_columns = 4;
	table_layout.table_flags = (OUTPUT_TABLE_FORMAT_HEADER | OUTPUT_TABLE_FORMAT_BOXED | OUTPUT_TABLE_FORMAT_FULL_WIDTH);
	table_layout.column_spacing = 2;
	table_layout.column[0].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;
	table_layout.column[1].flags = OUTPUT_COLUMN_FORMAT_WORD_WRAP;
	table_layout.column[2].flags = OUTPUT_COLUMN_FORMAT_WORD_WRAP;
	table_layout.column[3].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;

	/* how wide are they */
	table_layout.column[0].width = state_machine->max_state_length;
	table_layout.column[1].width = 0;
	table_layout.column[2].width = 0;
	table_layout.column[3].width = state_machine->max_state_length;
	
	/* style */
	table_layout.column[0].style = OUTPUT_TEXT_STYLE_BOLD;
	table_layout.column[3].style = OUTPUT_TEXT_STYLE_BOLD;

	/* set up the output table */
	draw_state->format->output_table_start(draw_state,&table_layout);

	/* draw column headings */
	table_row.row[0] = &name_state;
	table_row.row[1] = &name_input;
	table_row.row[2] = &name_triggers;
	table_row.row[3] = &name_next_state;

	draw_state->format->output_table_header(draw_state,&table_layout,&table_row);
	
	/* now generate the output */
	current_state = state_machine->state_list;

	while (current_state != NULL)
	{
		if ((current_state->flags & FLAG_TAGGED) == FLAG_TAGGED)
		{
			/* we have a state that we want to output */
			state.name			= current_state->name;
			state.name_length	= current_state->name_length;

			current_trans = current_state->transition_list;

			while(current_trans != NULL)
			{
				/* zero the items */
				input.name_length = 0;
				triggers.name_length = 0;

				/* is this transition one that we are interested in? */
				if (((current_state->flags & FLAG_ACTIVE) == FLAG_ACTIVE) || ((current_trans->next_state->flags & FLAG_ACTIVE) == FLAG_ACTIVE))
				{
					/* handle the input column */
					if (current_trans->trigger != NULL)
					{
						condition.name_length = 0;

						if (current_state->group != current_trans->trigger->group)
						{
							group.name				= current_trans->trigger->group->name;
							group.name_length		= current_trans->trigger->group->name_length;

							condition.name			= current_trans->trigger->name;
							condition.name_length	= current_trans->trigger->name_length;

							concat_names(&temp,&group,&name_colon,&condition);
					
							input.name			= temp.name;
							input.name_length	= temp.name_length;
						}
						else
						{
							input.name			= current_trans->trigger->name;
							input.name_length	= current_trans->trigger->name_length;
						}
					}
					else if (current_trans->condition != NULL)
					{
						input.name			= current_trans->condition;
						input.name_length	= current_trans->condition_length;
					}
				}

				/* now build the output for the triggers column */
				if (current_trans->triggers != NULL)
				{
					/* first pass - calculate the size */
					current_triggers = current_trans->triggers;
					size = 0;

					do
					{
						if (current_state->group != current_triggers->trigger->group)
						{
							size += current_triggers->trigger->group->name_length + 1;
						}

						size += current_triggers->trigger->name_length;

						if (current_triggers->next != NULL)
						{
							size += 2;
						}
						current_triggers = current_triggers->next;
					} 
					while (current_triggers != NULL);

					/* second pass build that bad boy */
					allocate_name(&triggers,size);

					current_triggers = current_trans->triggers;

					do
					{
						if (current_state->group != current_triggers->trigger->group)
						{
							append_string(&triggers,current_triggers->trigger->group->name,current_triggers->trigger->group->name_length);
							append_string(&triggers,(unsigned char*)":",1);
						}

						append_string(&triggers,current_triggers->trigger->name,current_triggers->trigger->name_length);

						if (current_triggers->next != NULL)
						{
							append_string(&triggers,(unsigned char*)", ",2);
						}
						current_triggers = current_triggers->next;
					} 
					while (current_triggers != NULL);
				}

				/* next state */
				next_state.name			= current_trans->next_state->name;
				next_state.name_length	= current_trans->next_state->name_length;

				/* output the row of the table */
				table_row.row[0] = &state;
				table_row.row[1] = &input;
				table_row.row[2] = &triggers;
				table_row.row[3] = &next_state;
					
				draw_state->format->output_table_row(draw_state,&table_layout,&table_row);
			
				/* release any allocated memory */
				free_name(&triggers);
				free_name(&temp);

				current_trans = current_trans->next;
			}
		}
		current_state = current_state->next;
	}
	
	draw_state->format->output_table_end(draw_state,&table_layout);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: handle_state_machine_graph
 * @desc: This function will generate the graphed version of the state machine.
 *--------------------------------------------------------------------------------*/
static void	handle_state_machine_graph(DRAW_STATE* draw_state, INPUT_STATE* input_state, STATE_MACHINE* state_machine)
{
	GRAPH				graph;
	STATE*				current_state = NULL;
	GRAPH_NODE*			node;
	GRAPH_VERTEX*		vertex;
	STATE_TRANSITION*	current_trans;
	unsigned int		x;
	unsigned int		y;
	unsigned int		id = 0;

	/* Ok, lets build the graph */
	graph_initialise(&graph);

	/* add the nodes first */
	current_state = state_machine->state_list;
	while (current_state != NULL)
	{
		if ((current_state->flags & FLAG_TAGGED) == FLAG_TAGGED)
		{
			/* add the state to the graph */
			current_state->graph_node = graph_create_node(&graph,current_state);
		}

		current_state = current_state->next;
	}

	/* now add the transitions */
	current_state = state_machine->state_list;
	while (current_state != NULL)
	{
		if ((current_state->flags & FLAG_TAGGED) == FLAG_TAGGED)
		{
			current_trans = current_state->transition_list;

			while(current_trans != NULL)
			{
				/* is this transition one that we are interested in? */
				if (((current_state->flags & FLAG_ACTIVE) == FLAG_ACTIVE) || ((current_trans->next_state->flags & FLAG_ACTIVE) == FLAG_ACTIVE))
				{
					graph_add_vertex(&graph,current_state->graph_node,current_trans->next_state->graph_node,current_trans);
				}

				current_trans = current_trans->next;
			}
		}

		current_state = current_state->next;
	}
	
	/* layout the graph */
	graph_force_directed(&graph);

	/* now dump the ordered graph 
	 * Start by outputting the nodes and tagging them
	 */
	draw_state->format->output_state_set_size(draw_state,graph.number_nodes,graph.number_vertex);

	node = graph_get_start_node(&graph,&x,&y,(void**)&current_state);

	while (node != NULL)
	{
		draw_state->format->output_state(draw_state,current_state,x,y);
		current_state->tag_id = id++;

		node = graph_get_next_node(node,&x,&y,(void**)&current_state);
	}
	
	/* now output the vertices */
	node = graph_get_start_node(&graph,&x,&y,(void**)&current_state);

	while (node != NULL)
	{
		vertex = graph_get_start_vertex(node,(void**)&current_trans);

		while (vertex != NULL)
		{
			draw_state->format->output_transition(	draw_state,current_state,
													current_trans,
													((STATE*)vertex->from_node->graph_item)->tag_id,
													((STATE*)vertex->to_node->graph_item)->tag_id);
			
			vertex = graph_get_next_vertex(vertex,(void**)&current_trans);
		}
	
		node = graph_get_next_node(node,&x,&y,(void**)&current_state);
	}
	
	/* release the graph */
	graph_release(&graph);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_state_machine
 * Desc : This function will produce a dot file for the given state machine.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_state_machine(DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group, unsigned int special_group)
{
	unsigned int		result = EC_OK;
	GROUP*				current_group = group;
	STATE*				current_state = NULL;
	STATE_TRANSITION*	current_trans;

	if (input_state->part_name.name_length > 0 && (current_state = find_state(group,input_state->part_name.name,input_state->part_name.name_length)) == NULL)
	{
		raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (group->state_machine == NULL)
	{
		/* do not generate a warning if the 'all' group is specified */
		if (special_group != ALL_GROUP)
		{
			raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
		}
	}
	else if ((draw_state->format_flags & OUTPUT_FORMAT_INDEX) != 0)
	{
		raise_warning(input_state->line_number,EC_INDEX_NOT_SUPPORTED_OF_THIS_ITEM,input_state->input_name,NULL);
	}
	else
	{
		do
		{
			if (current_state == NULL)
			{
				tag_all_states(group->state_machine);
			}
			else
			{
				untag_all_states(group->state_machine);
				tag_state(current_state);
			}

			draw_state->format->output_header(draw_state,input_state);

			if ((draw_state->format_flags & OUTPUT_FORMAT_TABLE) != 0)
			{
				/* defaults to the table style */
				handle_state_machine_table(draw_state,group->state_machine);
			}
			else
			{
				handle_state_machine_graph(draw_state,input_state,group->state_machine);
			}
							
			draw_state->format->output_footer(draw_state,input_state);

			current_group = current_group->next;

		} while (current_group != NULL && special_group == ALL_GROUP);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : tag_all_timelines
 * Desc : This function simply untags all the timelines.
 *--------------------------------------------------------------------------------*/
static void	tag_all_timelines(SEQUENCE_DIAGRAM* sequence_diagram)
{
	TIMELINE*	current_timeline = sequence_diagram->timeline_list;

	while (current_timeline != NULL)
	{
		current_timeline->flags |= (FLAG_TAGGED | FLAG_ACTIVE);

		current_timeline = current_timeline->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : untag_timelines
 * Desc : This function simply untags all the timelines.
 *--------------------------------------------------------------------------------*/
static void	untag_timelines(SEQUENCE_DIAGRAM* sequence_diagram)
{
	TIMELINE*	current_timeline = sequence_diagram->timeline_list;

	while (current_timeline != NULL)
	{
		current_timeline->flags &= ~(FLAG_TAGGED|FLAG_ACTIVE);

		current_timeline = current_timeline->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : tag_timelines
 * Desc : This function will walk down the given timeline and tag the timelines
 *        that it sends or receives messages to/from. It does not clear down the
 *        timelines that it does not talk to.
 *--------------------------------------------------------------------------------*/
static void	tag_timelines(TIMELINE* source)
{
	NODE*	current_node = source->node;

	source->flags |= (FLAG_TAGGED | FLAG_ACTIVE);

	while (current_node != NULL)
	{
		if (current_node->sent_message != NULL && current_node->sent_message->target_timeline != NULL)
		{
			current_node->sent_message->target_timeline->flags |= FLAG_TAGGED;
		}
		else if (current_node->received_message != NULL && current_node->received_message->sending_timeline != NULL)
		{
			current_node->received_message->sending_timeline->flags |= FLAG_TAGGED;
		}

		current_node = current_node->next;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_columns
 * Desc : This function will count the number of columns in the sequence diagram
 *        and will calculate the text column offset for each of the timelines.
 *
 *        TODO: fix this. This is not allowing for margins so all the rest of
 *        the sequence drawing functions are off, and all have crap code for
 *        correcting the errors that this has generated.
 *--------------------------------------------------------------------------------*/
static void	generate_columns(SEQUENCE_DIAGRAM* sequence_diagram, DRAW_STATE* draw_state)
{
	unsigned short	pos = 0;
	unsigned short	half_length = 0;
	TIMELINE*		current_timeline = sequence_diagram->timeline_list;

	draw_state->data.sequence.num_columns = 0;

	while (current_timeline != NULL)
	{
		if ((current_timeline->flags & FLAG_TAGGED) == FLAG_TAGGED)
		{
			current_timeline->column = draw_state->data.sequence.num_columns;
			pos += half_length + (current_timeline->name_length / 2) + 1;
			draw_state->data.sequence.column[draw_state->data.sequence.num_columns] = pos;

			half_length = (current_timeline->name_length / 2) + 1;

			draw_state->data.sequence.num_columns++;
			
			pos += current_timeline->group->max_message_length;
		}
		current_timeline = current_timeline->next;
	}

	draw_state->data.sequence.column[draw_state->data.sequence.num_columns] = pos;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : sequence_window
 * Desc : This function will work out how much of the sequence diagram it can 
 *        fit on the page. It will use the page width the generate the window. 
 *        If all the items fits on the page it will set the window at the last
 *        item, or if the page_width == 0. If no all the items fit on the page
 *        it will set the window at page_width - max_message_length. If any
 *        item can fit on the page, or it's line point fits in the window then
 *        the window will be set there. The next call of the this function will
 *        start at the item that was outside of the window.
 *--------------------------------------------------------------------------------*/
static unsigned int sequence_window(SEQUENCE_DIAGRAM* sequence_diagram, DRAW_STATE* draw_state)
{
	unsigned int	result = 0;
	unsigned short	pos = draw_state->margin_width;
	TIMELINE*		current_timeline = draw_state->data.sequence.end_timeline;
	static int stop = 0;
					
	if (draw_state->data.sequence.start_timeline != draw_state->data.sequence.end_timeline && current_timeline != NULL)
	{
		result = 1;
		draw_state->data.sequence.start_timeline = NULL;

		draw_state->data.sequence.first_column = current_timeline->column;

		/* where does the virtual window start */
		draw_state->data.sequence.window_start = draw_state->data.sequence.window_end;

		if (draw_state->data.sequence.window_start > 0)
		{
			draw_state->data.sequence.window_start -= current_timeline->group->max_message_length;
		}

		while (current_timeline != NULL)
		{
			if ((current_timeline->flags & FLAG_TAGGED) == FLAG_TAGGED)
			{
				if (draw_state->page_width > (pos + current_timeline->name_length + current_timeline->group->max_message_length + 2))
				{
					/* Ok, it fits within the window - and not the last one */
					pos += current_timeline->name_length + current_timeline->group->max_message_length + 2;
				}
				else
				{
					/* Ok, we have found the windows limit */
					draw_state->data.sequence.start_timeline = draw_state->data.sequence.end_timeline;
					draw_state->data.sequence.end_timeline = current_timeline;
					draw_state->data.sequence.window_end = draw_state->data.sequence.window_start + pos;
					draw_state->data.sequence.last_column = current_timeline->column;
					break;
				}
			}
			current_timeline = current_timeline->next;
		}

		/* ok, all what's left fits on the screen */
		if (current_timeline == NULL)
		{
			draw_state->data.sequence.start_timeline = draw_state->data.sequence.end_timeline;
			draw_state->data.sequence.end_timeline = NULL;
			draw_state->data.sequence.window_end = draw_state->data.sequence.window_start + pos;
			draw_state->data.sequence.last_column = draw_state->data.sequence.num_columns;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: generate_sequence_table
 * @desc: This function will draw the table version of the sequence diagram.
 *--------------------------------------------------------------------------------*/
static void	generate_sequence_table(DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group)
{
	NAME			to;
	NAME			from;
	NAME			message;
	NODE			walk_start;
	NODE*			search_node;
	NODE*			active_node;
	TIMELINE*		current_timeline = NULL;
	TABLE_ROW		table_row;
	TABLE_LAYOUT	table_layout;

	/* create the table layout */
	draw_state->format->output_header(draw_state,input_state);


	/* ok. where do the columns start */
	table_layout.num_columns = 3;
	table_layout.table_flags = (OUTPUT_TABLE_FORMAT_HEADER | OUTPUT_TABLE_FORMAT_BOXED | OUTPUT_TABLE_FORMAT_FULL_WIDTH);
	table_layout.column_spacing = 2;
	table_layout.column[0].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;
	table_layout.column[1].flags = OUTPUT_COLUMN_FORMAT_WORD_WRAP;
	table_layout.column[2].flags = OUTPUT_COLUMN_FORMAT_NO_TRUNCATE;

	/* how wide are they */
	table_layout.column[0].width = group->sequence_diagram->max_name_length;
	table_layout.column[1].width = 0;
	table_layout.column[2].width = group->sequence_diagram->max_name_length;;

	/* style */
	table_layout.column[0].style = OUTPUT_TEXT_STYLE_BOLD;
	table_layout.column[2].style = OUTPUT_TEXT_STYLE_BOLD;

	/* set up the output table */
	draw_state->format->output_table_start(draw_state,&table_layout);

	/* draw column headings */
	table_row.row[0] = &name_from;
	table_row.row[1] = &name_message;
	table_row.row[2] = &name_to;

	draw_state->format->output_table_header(draw_state,&table_layout,&table_row);

	/* we build the NAMEs in the loop */
	table_row.row[0] = &from;
	table_row.row[1] = &message;
	table_row.row[2] = &to;

	memset(&walk_start,0,sizeof(NODE));

	current_timeline = group->sequence_diagram->timeline_list;

	while (current_timeline != NULL)
	{
		if (current_timeline->node != NULL && ((current_timeline->flags & FLAG_TAGGED) == FLAG_TAGGED))
		{
			walk_start.next = current_timeline->node;

			search_node = &walk_start;

			do
			{
				search_node = search_next_node(search_node,&active_node);

				if (active_node != NULL)
				{
					do
					{
						if (active_node->sent_message != NULL)
						{
							if (active_node->sent_message->receiver != NULL &&
									(((active_node->sent_message->sender->timeline->flags & FLAG_ACTIVE) == FLAG_ACTIVE) ||
									 ((active_node->sent_message->receiver->timeline->flags & FLAG_ACTIVE) == FLAG_ACTIVE))) 
							{
								/* output a table row here */
								to.name				= active_node->sent_message->receiver->timeline->name;
								to.name_length		= active_node->sent_message->receiver->timeline->name_length;

								from.name			= active_node->sent_message->sender->timeline->name;
								from.name_length	= active_node->sent_message->sender->timeline->name_length;

								message.name		= active_node->sent_message->name;
								message.name_length	= active_node->sent_message->name_length;

								draw_state->format->output_table_row(draw_state,&table_layout,&table_row);
							}
						}
						active_node = next_active_node(active_node);
					}
					while (active_node != NULL && search_node != active_node);
				}
			}
			while (search_node != NULL);
		}
		current_timeline = current_timeline->next;
	}

	draw_state->format->output_table_end(draw_state,&table_layout);

	draw_state->format->output_footer(draw_state,input_state);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: generate_sequence_graph
 * @desc: This function will draw the graph version of the sequence diagram.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_sequence_graph(DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group)
{
	NODE			walk_start;
	NODE*			search_node;
	NODE*			active_node;
	TIMELINE*		current_timeline = NULL;
	unsigned int 	result;

	memset(&walk_start,0,sizeof(NODE));

	/* first generate the column offsets for the sequence diagram */
	generate_columns(group->sequence_diagram,draw_state);

	/* set the start window to the first column*/
	draw_state->data.sequence.start_timeline = NULL;
	draw_state->data.sequence.end_timeline = group->sequence_diagram->timeline_list;
	draw_state->data.sequence.window_start = 0;
	draw_state->data.sequence.window_end   = 0;

	while (sequence_window(group->sequence_diagram,draw_state))
	{
		current_timeline = group->sequence_diagram->timeline_list;

		draw_state->format->output_header(draw_state,input_state);
		draw_state->format->output_timelines(draw_state);

		if (draw_state->data.sequence.num_columns > 0)
		{
			while (current_timeline != NULL)
			{
				if (current_timeline->node != NULL && ((current_timeline->flags & FLAG_TAGGED) == FLAG_TAGGED))
				{
					walk_start.next = current_timeline->node;

					search_node = &walk_start;

					do
					{
						search_node = search_next_node(search_node,&active_node);

						if (active_node != NULL)
						{
							do
							{
								if (active_node->sent_message != NULL)
								{
									if (active_node->sent_message->receiver != NULL &&
											(((active_node->sent_message->sender->timeline->flags & FLAG_ACTIVE) == FLAG_ACTIVE) ||
											 ((active_node->sent_message->receiver->timeline->flags & FLAG_ACTIVE) == FLAG_ACTIVE))) 
									{
										draw_state->format->output_message(draw_state,active_node->sent_message);
									}
								}
								active_node = next_active_node(active_node);
							}
							while (active_node != NULL && search_node != active_node);
						}
					}
					while (search_node != NULL);
				}
				current_timeline = current_timeline->next;
			}
		}

		draw_state->format->output_timelines(draw_state);
		draw_state->format->output_footer(draw_state,input_state);
	}
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : generate_sequence_diagram
 * Desc : This function output a sequence diagram.
 *        It requires a output function so that the same code can be used to output
 *        any format. The status for the output function should be passed in the
 *        status parameter.
 *--------------------------------------------------------------------------------*/
static unsigned int	generate_sequence_diagram( DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group, unsigned int special_group)
{
	GROUP*		current_group = group;
	TIMELINE*	current_timeline = NULL;

	memset(&draw_state->data.sequence,0,sizeof(SEQUENCE_DRAW_STATE));

	if (input_state->part_name.name_length > 0 && (current_timeline = find_timeline(group,input_state->part_name.name,input_state->part_name.name_length)) == NULL)
	{
		raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
	}
	else if (group->sequence_diagram == NULL)
	{
		/* do not generate a warning if the all group is specified */
		if (special_group != ALL_GROUP)
		{
			raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
		}
	}	
	else if ((draw_state->format_flags & OUTPUT_FORMAT_INDEX) != 0)
	{
		raise_warning(input_state->line_number,EC_INDEX_NOT_SUPPORTED_OF_THIS_ITEM,input_state->input_name,NULL);
	}
	else
	{
		do
		{
			if (current_timeline == NULL)
			{
				tag_all_timelines(group->sequence_diagram);
			}
			else
			{
				untag_timelines(group->sequence_diagram);
				tag_timelines(current_timeline);
			}

			if ((draw_state->format_flags & OUTPUT_FORMAT_TABLE) != 0)
			{
				generate_sequence_table(draw_state,input_state,group);
			}
			else
			{
				generate_sequence_graph(draw_state,input_state,group);
			}

			current_group = current_group->next;

		} while (current_group != NULL && special_group == ALL_GROUP);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : index_api
 * Desc : This function will index a reference to an api.
 *--------------------------------------------------------------------------------*/
static unsigned int	index_api( DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group, unsigned int special_group)
{
	NAME				group_name;
	GROUP*				current_group = group;
	API_TYPE*			current_type;
	API_FUNCTION*		current_function;
	API_CONSTANT*		current_constant;
	API_CONSTANTS*		current_constants;
	DOC_SECTION*		new_section;
	unsigned int		flags = 0;

	if (decode_api_item(input_state,&flags) && (draw_state->format_flags & OUTPUT_FORMAT_INDEX) == 0)
	{
		do
		{
			if (current_group->api != NULL)
			{
				/* only whole API's groups are going to be indexed */
				if (input_state->part_name.name_length == 0)
				{
					draw_state->index.api_index_id++;

					level_index_set_level(&draw_state->index,3);
					new_section = index_add_section(draw_state);
					new_section->api_index_group = current_group;
					new_section->api_index_flags = flags;

					group_name.name = current_group->name;
					group_name.name_length = current_group->name_length;
					copy_name(&group_name,&new_section->section_title);

					/* output the function parts */
					if ((flags & OUTPUT_API_FUNCTIONS) != 0)
					{
						current_function = current_group->api->function_list;

						if (current_function != NULL && (flags & OUTPUT_API_MULTIPLE))
						{
							/* add function index */
							level_index_set_level(&draw_state->index,4);
							new_section = index_add_section(draw_state);
							copy_name(&name_function,&new_section->section_title);

							do
							{
								/* add function to the index */
								level_index_set_level(&draw_state->index,5);
								if (current_function->index_item == NULL)
								{
									current_function->index_item = index_add_section(draw_state);
								}
								copy_name(&current_function->name,&((DOC_SECTION*)draw_state->format_state)->section_title);

								current_function = current_function->next;
							}
							while (current_function != NULL);
						}
					}

					/* output the types */
					if ((flags & OUTPUT_API_TYPES) != 0)
					{
						current_type = current_group->api->type_list;

						if (current_type != NULL)
						{
							level_index_set_level(&draw_state->index,4);
							new_section = index_add_section(draw_state);
							copy_name(&name_ctype,&new_section->section_title);

							do
							{
								/* add type to the index */
								level_index_set_level(&draw_state->index,5);
								if (current_type->index_item == NULL)
								{
									current_type->index_item = index_add_section(draw_state);
								}
								copy_name(&current_type->name,&((DOC_SECTION*)draw_state->format_state)->section_title);

								current_type = current_type->next;
							}
							while(current_type != NULL);
						}
					}

					/* output the constants */
					if ((flags & OUTPUT_API_CONSTANTS) != 0)
					{
						current_constants = current_group->api->constants_list;

						if (current_constants != NULL)
						{
							level_index_set_level(&draw_state->index,4);
							new_section = index_add_section(draw_state);
							copy_name(&name_constants,&new_section->section_title);

							do
							{
								/* add constant to the index */	
								current_constant = current_constants->constant_list;

								while (current_constant != NULL)
								{
									level_index_set_level(&draw_state->index,5);
									if (current_constant->index_item == NULL)
									{
										current_constant->index_item = index_add_section(draw_state);
									}

									copy_name(&current_constant->name,&((DOC_SECTION*)draw_state->format_state)->section_title);

									current_constant = current_constant->next;
								}

								current_constants = current_constants->next;
							}
							while(current_constants != NULL);
						}
					}
				}
			}
			current_group = current_group->next;
		}
		while (current_group != NULL && special_group == ALL_GROUP);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: output_parse_model
 *  desc: This function will parse the model string given. If it points to a
 *        valid model item output the requested item. If the model has a name
 *        part (i.e. defined as [name][docgen:<....>] then the reference to that
 *        item will be output.
 *--------------------------------------------------------------------------------*/
unsigned int	output_parse_model(DRAW_STATE* draw_state, INPUT_STATE* input_state, GROUP* group_tree, NAME* model)
{
	GROUP*			group;
	unsigned int	special_group = NORMAL_GROUP;
	unsigned int	result = EC_OK;

	if ((result = output_decode_model(draw_state,input_state,model)) == EC_OK)
	{
		/* let the output format parse the flags */
		output_parse_flags(input_state,draw_state);

		/* check for selected group - or walk the tree if required */
		if ((group = find_group(group_tree,input_state->group_name.name,input_state->group_name.name_length,&special_group)) == NULL)
		{
			raise_warning(input_state->line_number,EC_UNKNOWN_ITEM,input_state->input_name,NULL);
		}
		else
		{
			if (special_group == DOCUMENT_GROUP)
			{
				output_document_specials(draw_state,input_state,group);
			}
			else if ((draw_state->format_flags & OUTPUT_FORMAT_INDEX_ONLY) != 0)
			{
				if (input_state->state == TYPE_API)
				{
					index_api(draw_state,input_state,group,special_group);
				}
			}
			else
			{
				/* action the found markers */
				switch (input_state->state)
				{
					case TYPE_STATE_MACHINE:
						generate_state_machine(draw_state,input_state,group,special_group);
						break;

					case TYPE_SEQUENCE_DIAGRAM:
						generate_sequence_diagram(draw_state,input_state,group,special_group);
						break;

					case TYPE_API:
						generate_api(draw_state,input_state,group,special_group);
						break;

					case TYPE_SAMPLE:
						generate_sample(draw_state,input_state,group,special_group);
						break;

					case TYPE_APPLICATION:
						generate_application(draw_state,input_state,group,special_group);
						break;

					default:
						result = EC_MODEL_BADLY_FORMATTED;
						raise_warning(input_state->line_number,result,input_state->input_name,0);
				}
			}
		}
	}

	return result;
}

