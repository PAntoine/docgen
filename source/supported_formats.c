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
 * static structures.
 *--------------------------------------------------------------------------------*/

OUTPUT_FORMATS	output_formats[] = 
{
	{text_fmt,TEXT_LEN,text_open,text_close,text_output_header,text_output_footer,text_output_timelines,text_output_message,text_output_states,text_output_state},
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
 * Name : output_open
 * Desc : This function will open the output and set up any handles that are
 *        required for the output format.
 *
 *        All output will be created in a directory of named hierarchy. 
 *        	format/<group_name>/<item_type>/<object_name>
 *
 *        The directories up to <item_type> will be created if they do not exist.
 *        The final name is passed to the specific format type to create the final
 *        object, in whatever format that needs.
 *
 *        The code relies on the fact that the file type will not change between
 *        the open and the close as it will build the directory path for the 
 *        target files to be placed in. If the type is changed mid run then the
 *        system will behave inconsistently.
 *--------------------------------------------------------------------------------*/
unsigned int	output_open(DRAW_STATE* draw_state, GROUP* group, unsigned char* path, unsigned int path_length)
{
	unsigned int	failed = 0;
	unsigned int	result = 0;
	unsigned int	name_length;
	unsigned int	group_name_length;
	unsigned char*	group_name;

	if (group->name_length == 0)
	{
		group_name = (unsigned char*)"default";
		group_name_length = sizeof("default")-1;
	}
	else
	{
		group_name = group->name;
		group_name_length = group->name_length;
	}

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
				if (extend_path(draw_state->path,&draw_state->path_length,group_name,group_name_length))
				{
					/* ok, let the file draw_state->format open the file/dir that it requires */
					result = output_formats[draw_state->format].output_open(draw_state,
																			diagram_type[draw_state->type].name,
																			diagram_type[draw_state->type].name_length);
				}
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

