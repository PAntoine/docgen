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
 * API Parts Decode Definitions.
 *--------------------------------------------------------------------------------*/

typedef struct
{
	unsigned int	flags;
	NAME			name;
} API_PARTS;

/* the major parts */
#define	OUTPUT_API_MULTIPLE				((unsigned int) (0x80000000))

/* what to dump */
#define	OUTPUT_API_FUNCTIONS			((unsigned int) (0x08000000))
#define	OUTPUT_API_TYPES				((unsigned int) (0x04000000))
#define	OUTPUT_API_DEFINES				((unsigned int) (0x02000000))
#define	OUTPUT_API_ALL					((unsigned int) (0x0E000000))

/* the parts of a function description */
#define	OUTPUT_API_FUNCTION_NAME		((unsigned int) (0x00000001))
#define	OUTPUT_API_FUNCTION_DESCRIPTION	((unsigned int) (0x00000002))
#define	OUTPUT_API_FUNCTION_PROTOTYPE	((unsigned int) (0x00000004))
#define	OUTPUT_API_FUNCTION_PARAMETERS	((unsigned int) (0x00000008))
#define	OUTPUT_API_FUNCTION_ACTION		((unsigned int) (0x00000010))
#define	OUTPUT_API_FUNCTION_RETURNS		((unsigned int) (0x00000020))
#define OUTPUT_API_FUNCTION_ALL_PARTS	((unsigned int) (0x0000003F))

/*--------------------------------------------------------------------------------*
 * Function Pointer Types for the output functions.
 *--------------------------------------------------------------------------------*/
typedef struct tag_draw_state DRAW_STATE;

typedef unsigned int	(*OUTPUT_OPEN_FUNCTION)				(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
typedef void			(*OUTPUT_CLOSE_FUNCTION)			(DRAW_STATE* draw_state);
typedef void			(*OUTPUT_HEADER_FUNCTION)			(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
typedef void			(*OUTPUT_FOOTER_FUNCTION)			(DRAW_STATE* draw_state);
typedef void			(*OUTPUT_RAW_FUNCTION)				(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size);

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

typedef struct
{
	unsigned char*				format_name;
	unsigned int				format_name_length;
	
	/* generic functions called for all diagrams */
	OUTPUT_OPEN_FUNCTION		output_open;
	OUTPUT_CLOSE_FUNCTION		output_close;
	OUTPUT_HEADER_FUNCTION		output_header;	
	OUTPUT_FOOTER_FUNCTION		output_footer;
	OUTPUT_RAW_FUNCTION			output_raw;

	/* sequence diagram functions */
	OUTPUT_TIMELINES_FUNCTION	output_timelines;
	OUTPUT_MESSAGE_FUNCTION		output_message;

	/* state machine functions */
	OUTPUT_STATES_FUNCTION		output_states;
	OUTPUT_START_STATE_FUNCITON	output_start_state;
	OUTPUT_TRANSITION_FUNCTION	output_transition;
	OUTPUT_END_STATE_FUNCTION 	output_end_state;

	/* API function functions */
	OUTPUT_API_NAME_FUNCTION		output_function_name;
	OUTPUT_API_DESCRIPTION_FUNCTION	output_function_description;
	OUTPUT_API_PROTOTYPE_FUNCTION	output_function_prototype;
	OUTPUT_API_PARAMETERS_FUNCTION	output_function_parameters;
	OUTPUT_API_ACTION_FUNCTION		output_function_action;
	OUTPUT_API_RETURNS_FUNCTION		output_function_returns;

} OUTPUT_FORMATS;

/*--------------------------------------------------------------------------------*
 * Graph output function defines.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned short	num_columns;
	unsigned short	column[MAX_OUTPUT_COLUMNS];

} SEQUENCE_DRAW_STATE;

struct tag_draw_state
{
	int				output_file;
	unsigned int	type;
	unsigned int	offset;
	unsigned int	buffer_size;
	unsigned int	format;
	unsigned int	path_length;
	unsigned char	path[FILENAME_MAX];
	unsigned char*	buffer;
	
	union
	{
		SEQUENCE_DRAW_STATE	sequence;
	}
	data;
};


/*--------------------------------------------------------------------------------*
 * Function prototypes for the global functions
 *--------------------------------------------------------------------------------*/
unsigned int	output_open(DRAW_STATE* draw_state, char* file_name, unsigned char* path, unsigned int path_length);
void			output_close(DRAW_STATE* draw_state);

/*--------------------------------------------------------------------------------*
 * TEXT format function types
 *--------------------------------------------------------------------------------*/
unsigned int	text_open(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
void			text_close(DRAW_STATE* draw_state);
void			text_output_header(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
void			text_output_footer(DRAW_STATE* draw_state);
void			text_output_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size);
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

/*--------------------------------------------------------------------------------*
 * DOT format function types
 *--------------------------------------------------------------------------------*/
void	dot_output_header(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
void	dot_output_footer(DRAW_STATE* draw_state);
void	dot_output_message(DRAW_STATE* draw_state, MESSAGE* message);
void	dot_output_timelines(DRAW_STATE* draw_state, TIMELINE* timeline);
void	dot_output_states(DRAW_STATE* draw_state, STATE* list);
void	dot_output_state(DRAW_STATE* draw_state, STATE* state);



#endif

