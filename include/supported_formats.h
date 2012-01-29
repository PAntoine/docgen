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
 * Function Pointer Types for the output functions.
 *--------------------------------------------------------------------------------*/
typedef struct tag_draw_state DRAW_STATE;

typedef unsigned int	(*OUTPUT_OPEN_FUNCTION)			(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
typedef void			(*OUTPUT_CLOSE_FUNCTION)		(DRAW_STATE* draw_state);
typedef void			(*OUTPUT_HEADER_FUNCTION)		(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
typedef void			(*OUTPUT_FOOTER_FUNCTION)		(DRAW_STATE* draw_state);

typedef void			(*OUTPUT_TIMELINES_FUNCTION)	(DRAW_STATE* draw_state, TIMELINE* timeline);
typedef void			(*OUTPUT_MESSAGE_FUNCTION)		(DRAW_STATE* draw_state, MESSAGE* message);

typedef void			(*OUTPUT_STATES_FUNCTION)		(DRAW_STATE* draw_state, STATE* list);
typedef void			(*OUTPUT_STATE_FUNCTION)		(DRAW_STATE* draw_state, STATE* state);

typedef struct
{
	unsigned char*				format_name;
	unsigned int				format_name_length;
	
	/* generic functions called for all diagrams */
	OUTPUT_OPEN_FUNCTION		output_open;
	OUTPUT_CLOSE_FUNCTION		output_close;
	OUTPUT_HEADER_FUNCTION		output_header;	
	OUTPUT_FOOTER_FUNCTION		output_footer;

	/* sequence diagram functions */
	OUTPUT_TIMELINES_FUNCTION	output_timelines;
	OUTPUT_MESSAGE_FUNCTION		output_message;

	/* state machine functions */
	OUTPUT_STATES_FUNCTION		output_states;
	OUTPUT_STATE_FUNCTION		output_state;

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
unsigned int	output_open(DRAW_STATE* draw_state, GROUP* group, unsigned char* path, unsigned int path_length);
void			output_close(DRAW_STATE* draw_state);

/*--------------------------------------------------------------------------------*
 * TEXT format function types
 *--------------------------------------------------------------------------------*/
unsigned int	text_open(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
void			text_close(DRAW_STATE* draw_state);
void			text_output_header(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length);
void			text_output_footer(DRAW_STATE* draw_state);
void			text_output_message(DRAW_STATE* draw_state, MESSAGE* message);
void			text_output_timelines(DRAW_STATE* draw_state, TIMELINE* timeline);
void			text_output_states(DRAW_STATE* draw_state, STATE* list);
void			text_output_state(DRAW_STATE* draw_state, STATE* state);

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

