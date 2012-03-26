/*--------------------------------------------------------------------------------*
 * Name  : c_cpp_input_functions
 * Desc  : This file holds the definitions of the functions for handling the 
 *         C & CPP files. This file may also be used for JAVA and other C like
 *         structured source files.
 *
 * Author: 
 * Date  : 05/03/2012 19:25:35
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <malloc.h>
#include <memory.h>
#include "symbols.h"
#include "input_formats.h"

/*--------------------------------------------------------------------------------*
 * Global Settings (with defaults)
 *--------------------------------------------------------------------------------*/
static unsigned char	g_comment_start		= '/';
static unsigned char	g_comment_start_end	= '*';
static unsigned char	g_comment_marker	= '*';
static unsigned char	g_comment_end_start	= '*';
static unsigned char	g_comment_end_end	= '/';
static unsigned char	g_function_start	= '{';
static unsigned char	g_function_end		= '}';

static unsigned int		g_found_start = 0;
static unsigned int		g_level = 0;

extern unsigned char is_valid_char[];

/*--------------------------------------------------------------------------------*
 * find function state machine.
 *--------------------------------------------------------------------------------*/
#define	C_START					(0)
#define	C_LOOKING				(1)
#define	C_GOT_EXTERN_STATIC		(2)
#define	C_GOT_UNSIGNED			(3)
#define	C_GOT_SIGNED			(4)
#define	C_GET_NAME				(5)
#define	C_GOT_CONST_VOLATILE	(6)
#define	C_FIND_TYPE				(7)
#define	C_GOT_LONG				(8)
#define	C_GOT_PRE_TYPE			(9)
#define	C_POINTER				(10)

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : c_get_type_name
 * Desc : This function will decode a C/C++ type. It will return the length of
 *        the type when the type is fully decoded. It decodes the following BNF
 *        as a valid type.
 *
 *        static_extern		:= 'static' | 'extern'
 *        const_volatile	:= 'const' | 'volatile'
 *        struct_union_enum	:= 'struct' | 'union' | 'enum'
 *        pre_type			:= 'unsigned' | 'signed' | 'long'
 *        type				:= 'signed' | 'long' | 'int' | 'char' | 'float' | 'double' | 'void'
 *        valid				:= extern_static type_spec
 *        type_spec			:= {static_extern} {const_volatile} name {pointer} name     |
 *                             {static_extern} {const_volatile} type_ref {pointer} name
 *        pointer			:= * {const_volatile} |
 *                             * {const_volatile} pointer
 *        type_ref			:= {struct_union_enum} name {pointer}
 *        name				:= NAME
 *
 * This is not the full C spec a varies from the awful BNF in the C books, but this
 * is all the types that are going to be supported.
 *--------------------------------------------------------------------------------*/
unsigned int	c_get_type_name(unsigned char* line,unsigned int line_length,unsigned int *pos,NAME* return_type, NAME* name)
{
	int						word;
	unsigned int			token_start;
	unsigned int			result = 0;
	unsigned int			current_pos = *pos;
	static unsigned int		current_state = C_START;
	static unsigned int		name_length = 0;
	static unsigned int		return_type_length = 0;
	static unsigned char	name_string[MAX_NAME_LENGTH];
	static unsigned char	return_type_name[MAX_NAME_LENGTH * 3];

	if (current_state == C_START)
	{
		name_length = 0;
		return_type_length = 0;
		current_state = C_LOOKING;
	}

	if (current_state == C_GET_NAME)
	{
		if (line[current_pos] == '*')
		{
			*pos = *pos + 1;
			return_type_name[return_type_length++] = line[current_pos];
			return_type_name[return_type_length++] = ' ';
			current_state = C_POINTER;
		}
		else
		{
			token_start = current_pos;

			while (current_pos < line_length && is_valid_char[line[current_pos]])
			{
				name_string[name_length++] = line[current_pos++];
			}

			if (line[current_pos] == '(' || line[current_pos] == '\t' || line[current_pos] == ' ' || line[current_pos] == 0x0a || line[current_pos] == 0x0d || line[current_pos] == ',' || line[current_pos] == ')')
			{
				*pos = current_pos;
				g_level = 0;
				current_state = C_START;
				result = 1;

				/* set the names */
				name->name_length = name_length;
				name->name = malloc(name_length);
				memcpy(name->name,name_string,name_length);

				return_type->name_length = return_type_length;
				return_type->name = malloc(return_type_length);
				memcpy(return_type->name,return_type_name,return_type_length);
			}
			else
			{
				printf("failed --- we have %c\n",line[current_pos]);
				current_state = C_START;
				result = 2;
			}
		}
	}
	else if ((word = symbols_check_word(&line[*pos])) > -1)
	{
		switch(word)
		{
			case SYMBOLS_EXTERN:
			case SYMBOLS_STATIC:
				if (current_state == C_LOOKING)
				{
					/* add the name to the return type */
					memcpy(return_type_name,&line[*pos],symbols_get_length(word));
					return_type_length = symbols_get_length(word);
					return_type_name[return_type_length++] = ' ';
						
					current_state = C_GOT_EXTERN_STATIC;
				}
				else
				{
					printf("bad function format\n");
					current_state = C_START;
					result = 2;
				}
				break;
			
			case SYMBOLS_CONST:
			case SYMBOLS_VOLATILE:
				if (current_state == C_POINTER)
				{
					/* add the name to the return type */
					memcpy(&return_type_name[return_type_length],&line[*pos],symbols_get_length(word));
					return_type_length += symbols_get_length(word);
					return_type_name[return_type_length++] = ' ';
						
					current_state = C_POINTER;
				}
				else if (current_state == C_LOOKING || current_state == C_GOT_EXTERN_STATIC)
				{
					/* add the name to the return type */
					memcpy(&return_type_name[return_type_length],&line[*pos],symbols_get_length(word));
					return_type_length += symbols_get_length(word);
					return_type_name[return_type_length++] = ' ';
						
					current_state = C_GOT_CONST_VOLATILE;
				}
				else
				{
					printf("bad function format\n");
					current_state = C_START;
					result = 2;
				}
				break;

			case SYMBOLS_LONG:
				if (current_state == C_GOT_LONG)
				{
					memcpy(&return_type_name[return_type_length],&line[*pos],symbols_get_length(word));
					return_type_length += symbols_get_length(word);
					return_type_name[return_type_length++] = ' ';
					
					current_state = C_GET_NAME;
				}
				else if (current_state == C_LOOKING || current_state == C_GOT_EXTERN_STATIC || current_state == C_GOT_PRE_TYPE || current_state == C_GOT_CONST_VOLATILE)
				{
					/* add to current type */
					memcpy(&return_type_name[return_type_length],&line[*pos],symbols_get_length(word));
					return_type_length += symbols_get_length(word);
					return_type_name[return_type_length++] = ' ';
				
					current_state = C_GOT_LONG;
				}
				else
				{
					printf("bad function format\n");
					current_state = C_START;
					result = 2;
				}
				break;

			case SYMBOLS_SIGNED:
			case SYMBOLS_UNSIGNED:
				if (current_state == C_LOOKING || current_state == C_GOT_EXTERN_STATIC || current_state == C_GOT_CONST_VOLATILE)
				{
					/* add to current type */
					memcpy(&return_type_name[return_type_length],&line[*pos],symbols_get_length(word));
					return_type_length += symbols_get_length(word);
					return_type_name[return_type_length++] = ' ';
				
					current_state = C_GOT_PRE_TYPE;
				}
				else
				{
					printf("bad function format\n");
					current_state = C_START;
					result = 2;
				}
				break;

			case SYMBOLS_INT:
			case SYMBOLS_CHAR:
			case SYMBOLS_VOID:
			case SYMBOLS_FLOAT:
			case SYMBOLS_SHORT:
			case SYMBOLS_DOUBLE:
				if (current_state == C_GOT_PRE_TYPE			|| 
					current_state == C_LOOKING				|| 
					current_state == C_GOT_EXTERN_STATIC	||  
					current_state == C_GOT_CONST_VOLATILE)
				{
					/* add to current type */
					memcpy(&return_type_name[return_type_length],&line[*pos],symbols_get_length(word));
					return_type_length += symbols_get_length(word);
					return_type_name[return_type_length++] = ' ';
	
					current_state = C_GET_NAME;
				}
				else
				{
					printf("bad function format\n");
					current_state = C_START;
					result = 2;
				}
				break;

			default:
				printf("unexpected reserved symbol in function header: %d\n",word);
				current_state = C_START;
				result = 2;
		}

		/* move the search pointer */
		*pos = *pos + symbols_get_length(word);
	}
	else
	{
		if (current_state == C_POINTER && line[current_pos] != '*')
		{
			current_state = C_GET_NAME;
		}
		else if (line[current_pos] == '*' || current_state == C_POINTER || current_state == C_GOT_PRE_TYPE || current_state == C_GOT_LONG)
		{
			return_type_name[return_type_length++] = line[current_pos];
			return_type_name[return_type_length++] = ' ';
			*pos = current_pos + 1;

			current_state = C_POINTER;
		}
		else
		{
			/* read the token from the buffer */
			while (current_pos < line_length && is_valid_char[line[current_pos]])
			{
				return_type_name[return_type_length++] = line[current_pos++];
			}

			if (current_state == C_LOOKING || current_state == C_GOT_EXTERN_STATIC ||  current_state == C_GOT_CONST_VOLATILE)
			{
				/* OK, assume it is a non-standard type */
				if (line[current_pos] == '\t' || line[current_pos] == ' ' || line[current_pos] == 0x0a || line[current_pos] == 0x0d)
				{
					/* name length: current_pos - token_start */
					*pos = current_pos;
					current_state = C_GET_NAME;
				}
				else if (line[current_pos] == '*')
				{
					return_type_name[return_type_length++] = line[current_pos++];
					return_type_name[return_type_length++] = ' ';
					*pos = current_pos;
					current_state = C_POINTER;
				}
				else
				{
					current_state = C_START;
					result = 2;
				}
			}
			else
			{
				current_state = C_START;
				result = 2;
			}
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_comment
 * Desc : This function will check to see if the comment starts at the current 
 *        file position, having first removed leading whitespace.
 *--------------------------------------------------------------------------------*/
unsigned int	c_find_comment(unsigned char* line,unsigned int line_length,unsigned int* pos)
{
	unsigned int result = 0;

	if (*pos + 2 < line_length)
	{
		if (line[*pos] == g_comment_start && line[(*pos)+1] == g_comment_start_end && line[(*pos)+2] == g_comment_marker)
		{
			result = 1;
			*pos = *pos + 2;
		}
	}
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : end_comment
 * Desc : This function will find the end of the comment.
 *--------------------------------------------------------------------------------*/
unsigned int	c_end_comment(unsigned char* line,unsigned int line_length,unsigned int* pos)
{
	unsigned int result = 0;

	if (*pos + 1 < line_length)
	{
		if (line[*pos] == g_comment_end_start && line[(*pos)+1] == g_comment_end_end)
		{
			result = 1;
			*pos = *pos + 2;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : function_start
 * Desc : This function will find the start of a c function. It will decode the
 *        fields for the function and return the return type and the name as 
 *        NAME items.
 *
 *        The function MUST follow the comment block that defines that it is a
 *        function, so that it is simpler to find.
 *
 *        c function pro-type structure:
 *         {extern|static} {{unsigned|signed} <type>}|<name>} <function_name> (
 *
 *        also signed is a type in it's own right.
 *
 *        The following is the simple state machine for the c_style functions.
 * 
 *        if extern or static and state == looking, goto got_ext_sta
 *        if unsigned and state == looking or got_ext_sta then goto got_unsigned
 *        if signed and state == looking or got_ext_sta then goto got_signed
 *        if valid_type and state == looking or got ext_sta or got_unsigned or got_signed then goto get_name
 *        if unknown_string and state == looking or ext_sta then goto get_name
 *        if unknown_string and state == get_name next goto exit
 *
 *--------------------------------------------------------------------------------*/
unsigned int	c_function_start(unsigned char* line,unsigned int line_length,unsigned int* pos, NAME* return_type, NAME* name)
{
	unsigned int		result = FUNCTION_STATE_SEARCHING;
	static unsigned int	got_type = 0;
			
	if (line[*pos] == '(')
	{
		if (got_type == 1)
		{
			g_found_start = 1;
			result = FUNCTION_STATE_PARAMETERS;
		}
		else
		{
			result = FUNCTION_STATE_FAILED;
		}
	}
	else
	{
		got_type = c_get_type_name(line,line_length,pos,return_type,name);

		if (got_type == 1 && line[*pos] == '(')
		{
			g_found_start = 1;
			result = FUNCTION_STATE_PARAMETERS;
		}
		else if (got_type == 2)
		{
			result = FUNCTION_STATE_FAILED;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : find_parameter
 * Desc : This function will find the parameters and when the parameter list
 *        comes to and end will set the state to the next correct state.
 *--------------------------------------------------------------------------------*/
unsigned int	c_find_parameter(unsigned char* line,unsigned int line_length,unsigned int* pos, NAME* return_type, NAME* name, unsigned int* found)
{
	unsigned int		result = FUNCTION_STATE_PARAMETERS;
	static unsigned int	got_type = 0;
	static unsigned int	state = C_FIND_TYPE;

	*found = 0;

	if (line[*pos] == '(')
	{
		/* TODO: this is the reset point, we know we are at the start of a parameter */
		*pos = *pos + 1;
	}
	else if (line[*pos] == ')')
	{
		*pos = *pos + 1;
		result = FUNCTION_STATE_BODY;
	}
	else if (line[*pos] == ',')
	{
		/* just ignore commas */
		*pos = *pos + 1;
	}
	else
	{
		got_type = c_get_type_name(line,line_length,pos,return_type,name);
		*found = got_type;
		
		if (got_type == 2)
		{
			result = FUNCTION_STATE_FAILED;
			*found = 0;
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : function_start_level
 * Desc : This function is looking for the next level.
 *--------------------------------------------------------------------------------*/
unsigned int	c_function_start_level(unsigned char* line,unsigned int line_length,unsigned int* pos)
{
	unsigned int result = 0;
	
	if (line[*pos] == '{')
	{
		g_level++;
		*pos = *pos + 1;

		result = 1;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : function_end_level
 * Desc : This function will find the end of function marker.
 *--------------------------------------------------------------------------------*/
unsigned int	c_function_end_level(unsigned char* line,unsigned int line_length,unsigned int* pos)
{
	unsigned int result = 0;
		
	if (line[*pos] == '}')
	{
		g_level--;
		result = 1;
	}

	*pos = *pos + 1;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : function_end
 * Desc : This function will always return 1 as c does not have a specific
 *        function end.
 *--------------------------------------------------------------------------------*/
unsigned int	c_function_end(unsigned char* line,unsigned int line_length,unsigned int* pos)
{
	unsigned int result = 0;

	if (g_level == 0 && g_found_start)
	{
		g_found_start = 0;
		result = 1;
	}

	return result;
}

