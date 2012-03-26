/*--------------------------------------------------------------------------------*
 * Name  : input_formats
 * Desc  : This file holds the supported input formats for the document processor.
 *
 * Author: 
 * Date  : 05/03/2012 19:44:03
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __INPUT_FORMATS_H__
#define __INPUT_FORMATS_H__

#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * Function states.
 *--------------------------------------------------------------------------------*/
#define	FUNCTION_STATE_FAILED		(1)
#define	FUNCTION_STATE_SEARCHING	(2)
#define	FUNCTION_STATE_PARAMETERS	(3)
#define	FUNCTION_STATE_BODY			(4)
	
/*--------------------------------------------------------------------------------*
 * Input formats
 *--------------------------------------------------------------------------------*/
typedef enum
{
	INPUT_FORMAT_C,
	INPUT_FORMAT_H,
	INPUT_FORMAT_CPP,
	INPUT_FORMATS_MAX,
	INPUT_FORMAT_INVALID = INPUT_FORMATS_MAX

} INPUT_FORMATS;

/*--------------------------------------------------------------------------------*
 * Function Pointer Types for the output functions.
 *--------------------------------------------------------------------------------*/
typedef unsigned int	(*INPUT_FIND_COMMENT_FUNCTION)		(unsigned char* line,unsigned int line_length,unsigned int* pos);
typedef unsigned int	(*INPUT_END_COMMENT_FUNCTION)		(unsigned char* line,unsigned int line_length,unsigned int* pos);
typedef unsigned int	(*INPUT_START_FUNCTION)				(unsigned char* line,unsigned int line_length,unsigned int* pos, NAME* return_type, NAME* name);
typedef unsigned int	(*INPUT_FIND_PARAMETER_FUNCTION)	(unsigned char* line,unsigned int line_length,unsigned int* pos, NAME* return_type, NAME* name, unsigned int* found);
typedef unsigned int	(*INPUT_START_LEVEL_FUNCTION)		(unsigned char* line,unsigned int line_length,unsigned int* pos);
typedef unsigned int	(*INPUT_END_LEVEL_FUNCTION)			(unsigned char* line,unsigned int line_length,unsigned int* pos);
typedef unsigned int	(*INPUT_END_FUNCTION)				(unsigned char* line,unsigned int line_length,unsigned int* pos);

typedef struct
{
	unsigned char*					format_name;
	unsigned int					format_name_length;
	INPUT_FIND_COMMENT_FUNCTION		find_comment;
	INPUT_END_COMMENT_FUNCTION		end_comment;
	INPUT_START_FUNCTION			function_start;
	INPUT_FIND_PARAMETER_FUNCTION	find_parameter;
	INPUT_START_LEVEL_FUNCTION		start_level;
	INPUT_END_LEVEL_FUNCTION		end_level;
	INPUT_END_FUNCTION				function_end;

} SOURCE_FORMAT;

/*--------------------------------------------------------------------------------*
 * C/C++ source handler functions.
 *--------------------------------------------------------------------------------*/
unsigned int	c_find_comment(unsigned char* line,unsigned int line_length,unsigned int* pos);
unsigned int	c_end_comment(unsigned char* line,unsigned int line_length,unsigned int* pos);
unsigned int	c_function_start(unsigned char* line,unsigned int line_length,unsigned int* pos, NAME* return_type, NAME* name);
unsigned int	c_find_parameter(unsigned char* line,unsigned int line_length,unsigned int* pos, NAME* return_type, NAME* name, unsigned int* found);
unsigned int	c_function_start_level(unsigned char* line,unsigned int line_length,unsigned int* pos);
unsigned int	c_function_end_level(unsigned char* line,unsigned int line_length,unsigned int* pos);
unsigned int	c_function_end(unsigned char* line,unsigned int line_length,unsigned int* pos);

#endif
