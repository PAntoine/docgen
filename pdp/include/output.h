/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : output
 * Desc  : This function holds the header functions for the supported output
 *         formats. Each of the formats function should be defined in this
 *         header file and the resulting structure should be placed in the
 *         supported_formats C file the builds the output structure.
 *
 * Author: pantoine
 * Date  : 27/01/2012 11:58:57
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <stdio.h>
#include "output_format.h"
#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * Output formats
 *--------------------------------------------------------------------------------*/
typedef enum
{
	DIAGRAM_TYPE_STATE_MACHINE,
	DIAGRAM_TYPE_SEQUENCE_DIAGRAM

} DIAGRAM_TYPE;

typedef struct
{
	unsigned char*	name;
	unsigned int	name_length;

} DIAGRAM_TYPES;

/*--------------------------------------------------------------------------------*
 * API Parts Decode Definitions.
 *--------------------------------------------------------------------------------*/

typedef struct
{
	unsigned int	flags;
	NAME			name;
} API_PARTS;

/* the major parts */
#define	OUTPUT_API_MULTIPLE					((unsigned int) (0x80000000))

/* what to dump */
#define	OUTPUT_API_APPLICATON				((unsigned int) (0x10000000))
#define	OUTPUT_API_FUNCTIONS				((unsigned int) (0x08000000))
#define	OUTPUT_API_TYPES					((unsigned int) (0x04000000))
#define	OUTPUT_API_CONSTANTS				((unsigned int) (0x02000000))
#define	OUTPUT_API_GLOBALS					((unsigned int) (0x01000000))
#define	OUTPUT_API_ALL						((unsigned int) (0x1F000000))

/* the parts of a function description */
#define	OUTPUT_API_FUNCTION_NAME			((unsigned int) (0x00000001))
#define	OUTPUT_API_FUNCTION_DESCRIPTION		((unsigned int) (0x00000002))
#define	OUTPUT_API_FUNCTION_PROTOTYPE		((unsigned int) (0x00000004))
#define	OUTPUT_API_FUNCTION_PARAMETERS		((unsigned int) (0x00000008))
#define	OUTPUT_API_FUNCTION_ACTION			((unsigned int) (0x00000010))
#define	OUTPUT_API_FUNCTION_RETURNS			((unsigned int) (0x00000020))
#define OUTPUT_API_FUNCTION_ALL_PARTS		((unsigned int) (0x0000003F))

/* the parts of the types */
#define OUTPUT_API_TYPE_NAME				((unsigned int)	(0x00000100))
#define OUTPUT_API_TYPE_RECORDS				((unsigned int)	(0x00000200))
#define OUTPUT_API_TYPE_DESCRIPTION			((unsigned int)	(0x00000400))
#define OUTPUT_API_TYPE_ALL_PARTS			((unsigned int) (0x00000F00))

/* the parts of the constants */
#define OUTPUT_API_CONSTANTS_NAME			((unsigned int)	(0x00001000))
#define OUTPUT_API_CONSTANTS_CONSTANT		((unsigned int)	(0x00002000))
#define OUTPUT_API_CONSTANTS_DESCRIPTION	((unsigned int)	(0x00004000))
#define OUTPUT_API_CONSTANTS_ALL_PARTS		((unsigned int) (0x0000F000))

/*--------------------------------------------------------------------------------*
 * Application parts 
 *--------------------------------------------------------------------------------*/
#define OUTPUT_APPLICATION_NAME				((unsigned int)	(0x00000001))
#define OUTPUT_APPLICATION_SECTION			((unsigned int)	(0x00000002))
#define OUTPUT_APPLICATION_OPTION			((unsigned int)	(0x00000004))
#define OUTPUT_APPLICATION_COMMAND			((unsigned int)	(0x00000008))
#define OUTPUT_APPLICATION_SYNOPSIS			((unsigned int)	(0x00000010))
#define OUTPUT_APPLICATION_ALL_PARTS		((unsigned int) (0x0000001F))

#define OUTPUT_APPLICATION_MULTIPLE			((unsigned int) (0x80000000))

/*--------------------------------------------------------------------------------*
 * Function prototypes for the global functions
 *--------------------------------------------------------------------------------*/
void			output_initialise(char* search_path);
unsigned int	output_open(DRAW_STATE* draw_state, char* file_name, unsigned char* path, unsigned int path_length);
void			output_close(DRAW_STATE* draw_state);
void			output_release(void);

/*--------------------------------------------------------------------------------*
 * output flag functions.
 *--------------------------------------------------------------------------------*/

void			output_init_flag_list(OUTPUT_FLAG_LIST* list);
unsigned int	output_find_flag(OUTPUT_FLAG_LIST* list, unsigned int option_hash, NAME* value, OUTPUT_FLAG_VALUE* flag_value);
void			output_parse_flags(INPUT_STATE* input_state, DRAW_STATE* draw_state);

/*--------------------------------------------------------------------------------*
 * output functions.
 *--------------------------------------------------------------------------------*/

void	output_function_prototype(DRAW_STATE* draw_state, API_FUNCTION* function);
void	output_function_parameters(DRAW_STATE* draw_state, API_FUNCTION* function);
void	output_function_returns(DRAW_STATE* draw_state, API_FUNCTION* function);
void	output_type_records(DRAW_STATE* draw_state, API_TYPE* function);
void	output_constant_records(DRAW_STATE* draw_state, API_CONSTANTS* constant);
void	output_application_option(DRAW_STATE* draw_state, APPLICATION* application,unsigned int parts, NAME* part_name);
void	output_application_command(DRAW_STATE* draw_state, APPLICATION* application,unsigned int parts, NAME* part_name);
void	output_application_section(DRAW_STATE* draw_state, APPLICATION* application,unsigned int parts, NAME* part_name);
void	output_application_synopsis(DRAW_STATE* draw_state, APPLICATION* application,unsigned int parts, NAME* part_name);

#endif

