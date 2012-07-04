/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : supported_formats
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

#ifndef __SUPPORTED_FORMATS_H__
#define __SUPPORTED_FORMATS_H__

#include <stdio.h>
#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * Output formats
 *--------------------------------------------------------------------------------*/
typedef enum
{
	OUTPUT_TEXT,
	OUTPUT_FORMATS_MAX

} OUTPUT_FORMAT;

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

#define OUTPUT_DEFAULT	(OUTPUT_TEXT)

/*--------------------------------------------------------------------------------*
 * Structures for output flags
 *--------------------------------------------------------------------------------*/
#define	OUTPUT_FLAG_TYPE_INVALID	((unsigned int) 0x00000000)		/* invalid type */
#define	OUTPUT_FLAG_TYPE_BOOLEAN	((unsigned int) 0x00000001)		/* the flag expects a true/false value (and converts to 1/0) */
#define OUTPUT_FLAG_TYPE_STRING		((unsigned int) 0x00000002)		/* the flag expects a string value */
#define OUTPUT_FLAG_TYPE_NUMBER		((unsigned int) 0x00000004)		/* the flag expects a numeric value to be passed in */
#define OUTPUT_FLAG_TYPE_REAL		((unsigned int) 0x00000008)		/* the flag expects a floating point numeric value to be passed in */

typedef struct
{
	unsigned char*	name;
	unsigned int	id;
	unsigned int	hash;
	unsigned int	name_length;
	unsigned int	type;

} OUTPUT_FLAG;

/* the flag list for a type */
typedef struct
{
	unsigned int	num_flags;
	OUTPUT_FLAG*	flag_list;

} OUTPUT_FLAG_LIST;

typedef struct
{
	unsigned int	type;
	unsigned int	id;

	union
	{
		unsigned char	boolean;
		int				number;
		float			real;
		NAME			string;
	} value;

} OUTPUT_FLAG_VALUE;

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
 * Function Pointer Types for the output functions.
 *--------------------------------------------------------------------------------*/
typedef struct tag_draw_state DRAW_STATE;

typedef unsigned int	(*OUTPUT_DECODE_FLAGS_FUNCTION)		(INPUT_STATE* input_state, unsigned hash,NAME* value);

typedef unsigned int	(*OUTPUT_OPEN_FUNCTION)				(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
typedef void			(*OUTPUT_CLOSE_FUNCTION)			(DRAW_STATE* draw_state);
typedef void			(*OUTPUT_HEADER_FUNCTION)			(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
typedef void			(*OUTPUT_FOOTER_FUNCTION)			(DRAW_STATE* draw_state);
typedef void			(*OUTPUT_RAW_FUNCTION)				(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size);
typedef void			(*OUTPUT_SAMPLE_FUNCTION)			(DRAW_STATE* draw_state, SAMPLE* sample);

typedef void			(*OUTPUT_TIMELINES_FUNCTION)		(DRAW_STATE* draw_state, TIMELINE* timeline);
typedef void			(*OUTPUT_MESSAGE_FUNCTION)			(DRAW_STATE* draw_state, MESSAGE* message);

typedef void			(*OUTPUT_STATES_FUNCTION)			(DRAW_STATE* draw_state,STATE* list);
typedef void			(*OUTPUT_END_STATE_FUNCTION)		(DRAW_STATE* draw_state,STATE* state);
typedef void			(*OUTPUT_TRANSITION_FUNCTION)		(DRAW_STATE* draw_state,STATE* state, STATE_TRANSITION* transition);
typedef void			(*OUTPUT_START_STATE_FUNCITON)		(DRAW_STATE* draw_state,STATE* state);

typedef void			(*OUTPUT_API_NAME_FUNCTION)			(DRAW_STATE* draw_state, API_FUNCTION* function);
typedef void			(*OUTPUT_API_DESCRIPTION_FUNCTION)	(DRAW_STATE* draw_state, API_FUNCTION* function);
typedef void			(*OUTPUT_API_PROTOTYPE_FUNCTION)	(DRAW_STATE* draw_state, API_FUNCTION* function);
typedef void			(*OUTPUT_API_PARAMETERS_FUNCTION)	(DRAW_STATE* draw_state, API_FUNCTION* function);
typedef void			(*OUTPUT_API_ACTION_FUNCTION)		(DRAW_STATE* draw_state, API_FUNCTION* function);
typedef void			(*OUTPUT_API_RETURNS_FUNCTION)		(DRAW_STATE* draw_state, API_FUNCTION* function);

typedef void			(*OUTPUT_TYPE_NAME_FUNCTION)		(DRAW_STATE* draw_state, API_TYPE* type);
typedef void			(*OUTPUT_TYPE_DESCRIPTION_FUNCTION)	(DRAW_STATE* draw_state, API_TYPE* type);
typedef void			(*OUTPUT_TYPE_RECORDS_FUNCTION)		(DRAW_STATE* draw_state, API_TYPE* type);

typedef void			(*OUTPUT_CONSTANTS_RECORDS_FUNCTION)	(DRAW_STATE* draw_state, API_CONSTANTS* constants);
typedef void			(*OUTPUT_CONSTANTS_DESCRIPTION_FUNCTION)(DRAW_STATE* draw_state, API_CONSTANTS* constants);
typedef void			(*OUTPUT_CONSTANT_NAME_FUNCTION)		(DRAW_STATE* draw_state, API_CONSTANTS* constants);

typedef void			(*OUTPUT_APPLICATION_NAME_FUNCTION)		(DRAW_STATE* draw_state,APPLICATION* application,NAME* name);
typedef void			(*OUTPUT_APPLICATION_SYNOPSIS_FUNCTION)	(DRAW_STATE* draw_state,APPLICATION* application,unsigned int parts,NAME* name);
typedef void			(*OUTPUT_APPLICATION_OPTION_FUNCTION)	(DRAW_STATE* draw_state,APPLICATION* application,unsigned int parts,NAME* name);
typedef void			(*OUTPUT_APPLICATION_COMMAND_FUNCTION)	(DRAW_STATE* draw_state,APPLICATION* application,unsigned int parts,NAME* name);
typedef void			(*OUTPUT_APPLICATION_SECTION_FUNCTION)	(DRAW_STATE* draw_state,APPLICATION* application,unsigned int parts,NAME* name);
typedef void			(*OUTPUT_APPLICATION_SUBSECTION_FUNCTION)(DRAW_STATE* draw_state,APPLICATION* application,unsigned int parts,NAME* name);

typedef struct
{
	unsigned char*				format_name;
	unsigned int				format_name_length;

	/* function for decoding the type specific flags */
	OUTPUT_DECODE_FLAGS_FUNCTION	decode_flags;
	
	/* generic functions called for all diagrams */
	OUTPUT_OPEN_FUNCTION		output_open;
	OUTPUT_CLOSE_FUNCTION		output_close;
	OUTPUT_HEADER_FUNCTION		output_header;	
	OUTPUT_FOOTER_FUNCTION		output_footer;
	OUTPUT_RAW_FUNCTION			output_raw;
	OUTPUT_SAMPLE_FUNCTION		output_sample;

	/* sequence diagram functions */
	OUTPUT_TIMELINES_FUNCTION	output_timelines;
	OUTPUT_MESSAGE_FUNCTION		output_message;

	/* state machine functions */
	OUTPUT_STATES_FUNCTION		output_states;
	OUTPUT_START_STATE_FUNCITON	output_start_state;
	OUTPUT_TRANSITION_FUNCTION	output_transition;
	OUTPUT_END_STATE_FUNCTION 	output_end_state;

	/* API function functions */
	OUTPUT_API_NAME_FUNCTION			output_function_name;
	OUTPUT_API_DESCRIPTION_FUNCTION		output_function_description;
	OUTPUT_API_PROTOTYPE_FUNCTION		output_function_prototype;
	OUTPUT_API_PARAMETERS_FUNCTION		output_function_parameters;
	OUTPUT_API_ACTION_FUNCTION			output_function_action;
	OUTPUT_API_RETURNS_FUNCTION			output_function_returns;

	/* API type functions */
	OUTPUT_TYPE_NAME_FUNCTION			output_type_name;
	OUTPUT_TYPE_DESCRIPTION_FUNCTION	output_type_description;
	OUTPUT_TYPE_RECORDS_FUNCTION		output_type_records;

	/* API constants functions */
	OUTPUT_CONSTANTS_RECORDS_FUNCTION		output_constant_records;
	OUTPUT_CONSTANTS_DESCRIPTION_FUNCTION	output_constant_description;
	OUTPUT_CONSTANT_NAME_FUNCTION			output_constant_name;

	/* Application functions */
	OUTPUT_APPLICATION_NAME_FUNCTION		output_application_name;
	OUTPUT_APPLICATION_SYNOPSIS_FUNCTION    output_application_synopsis;
	OUTPUT_APPLICATION_OPTION_FUNCTION      output_application_option;
	OUTPUT_APPLICATION_COMMAND_FUNCTION     output_application_command;
	OUTPUT_APPLICATION_SECTION_FUNCTION     output_application_section;

} OUTPUT_FORMATS;

/*--------------------------------------------------------------------------------*
 * Output function defines.
 *--------------------------------------------------------------------------------*/
#define	OUTPUT_FORMAT_FLAGS_PAGED	((unsigned int)	0x00000001)
#define	OUTPUT_FORMAT_FLAGS_INLINE	((unsigned int)	0x00000002)

typedef struct
{
	unsigned short	num_columns;
	unsigned short	column[MAX_OUTPUT_COLUMNS];

} SEQUENCE_DRAW_STATE;

struct tag_draw_state
{
	int				output_file;
	unsigned int	offset;
	unsigned int	buffer_size;
	unsigned int	format;
	unsigned int	path_length;
	unsigned int	page_width;				/* this is a type dependant page width */
	unsigned int	margin_width;
	unsigned int	format_flags;
	unsigned int	global_margin_width;
	unsigned int	global_format_flags;
	unsigned char	path[FILENAME_MAX];
	unsigned char*	buffer;
	unsigned char*	output_buffer;			/* random buffer controlled by the type code */

	union
	{
		SEQUENCE_DRAW_STATE	sequence;
	}
	data;
};

/*--------------------------------------------------------------------------------*
 * Function prototypes for the global functions
 *--------------------------------------------------------------------------------*/
void			output_initialise(void);
unsigned int	output_open(DRAW_STATE* draw_state, char* file_name, unsigned char* path, unsigned int path_length);
void			output_close(DRAW_STATE* draw_state);

/*--------------------------------------------------------------------------------*
 * TEXT format function types
 *--------------------------------------------------------------------------------*/
unsigned int	text_decode_flags_function(INPUT_STATE* input_state, unsigned int hash, NAME* value);
unsigned int	text_open(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
void			text_close(DRAW_STATE* draw_state);
void			text_output_header(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
void			text_output_footer(DRAW_STATE* draw_state);
void			text_output_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size);
void			text_output_sample(DRAW_STATE* draw_state, SAMPLE* sample);
void			text_output_message(DRAW_STATE* draw_state, MESSAGE* message);
void			text_output_timelines(DRAW_STATE* draw_state, TIMELINE* timeline);
void			text_output_states(DRAW_STATE* draw_state, STATE* list);
void			text_output_start_state(DRAW_STATE* draw_state,STATE* state);
void			text_output_transition(DRAW_STATE* draw_state,STATE* state, STATE_TRANSITION* transition);
void			text_output_end_state(DRAW_STATE* draw_state,STATE* state);
void			text_output_api_name_function(DRAW_STATE* draw_state, API_FUNCTION* function);
void			text_output_api_description_function(DRAW_STATE* draw_state, API_FUNCTION* function);
void			text_output_api_prototype_function(DRAW_STATE* draw_state, API_FUNCTION* function);
void			text_output_api_parameters_function(DRAW_STATE* draw_state, API_FUNCTION* function);
void			text_output_api_action_function(DRAW_STATE* draw_state, API_FUNCTION* function);
void			text_output_api_returns_function(DRAW_STATE* draw_state, API_FUNCTION* function);
void			text_output_type_name_function(DRAW_STATE* draw_state, API_TYPE* type);
void			text_output_type_description_function(DRAW_STATE* draw_state, API_TYPE* type);
void			text_output_type_records_function(DRAW_STATE* draw_state, API_TYPE* type);
void			text_output_constants_records_function(DRAW_STATE* draw_state, API_CONSTANTS* constants);
void			text_output_constants_description_function(DRAW_STATE* draw_state, API_CONSTANTS* constants);
void			text_output_constant_name_function(DRAW_STATE* draw_state, API_CONSTANTS* constants);
void			text_output_application_name_function(DRAW_STATE* draw_state,APPLICATION* application,NAME* name);
void			text_output_application_synopsis_function(DRAW_STATE* draw_state,APPLICATION* application,unsigned int parts,NAME* name);
void			text_output_application_option_function(DRAW_STATE* draw_state,APPLICATION* application,unsigned int parts,NAME* name);
void			text_output_application_command_function(DRAW_STATE* draw_state,APPLICATION* application,unsigned int parts,NAME* name);
void			text_output_application_section_function(DRAW_STATE* draw_state,APPLICATION* application,unsigned int parts,NAME* name);

/*--------------------------------------------------------------------------------*
 * DOT format function types
 *--------------------------------------------------------------------------------*/
void	dot_output_header(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
void	dot_output_footer(DRAW_STATE* draw_state);
void	dot_output_message(DRAW_STATE* draw_state, MESSAGE* message);
void	dot_output_timelines(DRAW_STATE* draw_state, TIMELINE* timeline);
void	dot_output_states(DRAW_STATE* draw_state, STATE* list);
void	dot_output_state(DRAW_STATE* draw_state, STATE* state);

/*--------------------------------------------------------------------------------*
 * output flag functions.
 *--------------------------------------------------------------------------------*/
void			output_init_flag_list(OUTPUT_FLAG_LIST* list);
unsigned int	output_find_flag(OUTPUT_FLAG_LIST* list, unsigned int option_hash, NAME* value, OUTPUT_FLAG_VALUE* flag_value);
void			output_parse_flags(INPUT_STATE* input_state, DRAW_STATE* draw_state);



#endif

