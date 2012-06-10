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
static unsigned char	g_comment_start			= '/';
static unsigned char	g_comment_start_end		= '*';
static unsigned char	g_comment_marker		= '*';
static unsigned char	g_comment_pre_marker	= ':';
static unsigned char	g_comment_cont_marker	= '*';
static unsigned char	g_comment_end_start		= '*';
static unsigned char	g_comment_end_end		= '/';
static unsigned char	g_function_start		= '{';
static unsigned char	g_function_end			= '}';

static unsigned int		g_found_start = 0;
static unsigned int		g_level = 0;
static unsigned int		g_comment_start_length = 2;

extern unsigned char is_valid_char[];

extern SYMBOLS_STRING_TABLE	symbols_table[];

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
#define	C_GOT_UNION_STRUCT_ENUM	(11)
#define C_LOOK_FOR_ARRAY		(12)
#define C_GET_INDEX				(13)

/*--------------------------------------------------------------------------------*
 * find type state machine.
 *--------------------------------------------------------------------------------*/
#define	C_TYPE_STATE_LOOKING			(0)
#define	C_TYPE_STATE_GOT_TYPEDEF		(1)
#define	C_TYPE_STATE_GET_TYPE			(2)
#define	C_TYPE_STATE_GET_NAME			(3)
#define C_TYPE_STATE_GET_TAG			(4)
#define C_TYPE_STATE_GET_STRUCT_FIELD	(5)
#define C_TYPE_STATE_GET_ENUM_FIELD		(6)
#define C_TYPE_STATE_GET_SUB_NAME		(7)

/*--------------------------------------------------------------------------------*
 * find constant state machine.
 *--------------------------------------------------------------------------------*/
#define C_CONSTANT_STATE_LOOKING			(0)
#define C_CONSTANT_STATE_DEFINE				(1)
#define C_CONSTANT_STATE_TYPE_DATA			(2)
#define C_CONSTANT_STATE_CONTINUATION		(3)
#define C_CONSTANT_STATE_FAILED				(4)
#define C_CONSTANT_GET_STRING				(5)
#define C_CONSTANT_GET_NUMBERS				(6)
#define C_CONSTANT_GET_NAME					(7)
#define C_CONSTANT_GET_ARRAY_DATA			(8)
#define C_CONSTANT_FOUND_EQUALS				(9)

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
 * This is not the full C spec and varies from the awful BNF in the C books, but
 * this is all the types that are going to be supported.
 *
 * Note: if the search fails it might be because it's a partial type, and that 
 *       symbol is returned in the 'symbol' parameter. This will help in some 
 *       of the searches so it does not have to search again to find the symbol.
 *--------------------------------------------------------------------------------*/
unsigned int	c_get_type_name(unsigned char* line,unsigned int line_length,unsigned int *pos,NAME* return_type, NAME* name,unsigned int* symbol)
{
	int						word;
	unsigned int			token_start;
	unsigned int			result = 0;
	unsigned int			current_pos = *pos;
	static unsigned int		hold_symbol;
	static unsigned int		current_state = C_START;
	static unsigned int		name_length = 0;
	static unsigned int		return_type_length = 0;
	static unsigned char	name_string[MAX_NAME_LENGTH];
	static unsigned char	return_type_name[MAX_NAME_LENGTH * 3];

	if (current_state == C_START)
	{
		hold_symbol = SYMBOLS_NUM_STRINGS;
		name_length = 0;
		return_type_length = 0;
		current_state = C_LOOKING;
	}

	if (current_state == C_GET_INDEX)
	{
		while (current_pos < line_length && is_valid_char[line[current_pos]])
		{
			return_type_name[return_type_length++] = line[current_pos++];
		}

		if (line[current_pos] == ']')
		{
			return_type_name[return_type_length++] = ']';
			*pos = current_pos + 1;
			current_state = C_LOOK_FOR_ARRAY;
		}
		else
		{
			current_state = C_START;
			result = 2;
		}
	}
	else if (current_state == C_LOOK_FOR_ARRAY)
	{
		if (line[current_pos] == '[')
		{
			return_type_name[return_type_length++] = '[';
			*pos = *pos + 1;
			current_state = C_GET_INDEX;
		}
		else
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
	}
	else if (current_state == C_GET_NAME)
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

			if (line[current_pos] == '(' 	|| line[current_pos] == '\t' || line[current_pos] == ' ' ||
				line[current_pos] == 0x0a	|| line[current_pos] == 0x0d || line[current_pos] == ',' ||
				line[current_pos] == ')' 	|| line[current_pos] == ';'  || line[current_pos] == '[' )
			{
				/* remove white space */
				while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20))
				{
					current_pos++;
				}

				if (line[current_pos] == '[')
				{
					*pos = current_pos;
					current_state = C_LOOK_FOR_ARRAY;
				}
				else
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
			}
			else
			{
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
					current_state = C_START;
					result = 2;
				}
				break;

			case SYMBOLS_ENUM:
			case SYMBOLS_UNION:
			case SYMBOLS_STRUCT:
				if (current_state == C_LOOKING || current_state == C_GOT_EXTERN_STATIC ||  current_state == C_GOT_CONST_VOLATILE)
				{
					/* add to current type */
					memcpy(&return_type_name[return_type_length],&line[*pos],symbols_get_length(word));
					return_type_length += symbols_get_length(word);
					return_type_name[return_type_length++] = ' ';
				
					hold_symbol = word;
					current_state = C_GOT_UNION_STRUCT_ENUM;
				}
				else
				{
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
					current_state = C_START;
					result = 2;
				}
				break;

			default:
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

			if (current_state == C_LOOKING 				|| 
				current_state == C_GOT_EXTERN_STATIC	||
				current_state == C_GOT_CONST_VOLATILE	||
				current_state == C_GOT_UNION_STRUCT_ENUM)
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
					/* if we had the struct/union/enum that might be useful */
					*symbol = hold_symbol;
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
		/* have we found a comment */
		if (line[*pos] == g_comment_start && line[(*pos)+1] == g_comment_start_end)
		{
			*pos = *pos + 1;
			result = 1;

			/* is it a special comment */
			if (line[(*pos)+1] == g_comment_marker)
			{
				*pos = *pos + 1;
				result = 2;
			}
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
	unsigned int		unsued;
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
		got_type = c_get_type_name(line,line_length,pos,return_type,name,&unsued);

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
	unsigned int		unused;
	static unsigned int	got_type = 0;
	static unsigned int	state = C_FIND_TYPE;

	*found = 0;

	if (line[*pos] == '(')
	{
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
		got_type = c_get_type_name(line,line_length,pos,return_type,name,&unused);
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

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : c_decode_type
 * Desc : This function will find the start of a type. As we are not going to
 *        bother understanding the type like was needed for the parameters we
 *        are looking for the start of the type. So the structure we are looking
 *        for is:
 *        		{typedef} type name
 *        		{typedef} {struct|union|enum} {name} '{' list '}' name;
 *
 *        If we cannot find this then we fail.
 *--------------------------------------------------------------------------------*/
unsigned int	c_decode_type(unsigned char* line,unsigned int line_length,unsigned int* pos, unsigned int current_group, ATOM_INDEX* atom_list)
{
	NAME					name;
	NAME					value;
	NAME					return_type;
	unsigned int			token_start;
	unsigned int			word = SYMBOLS_NUM_STRINGS;
	unsigned int			result = 0;
	unsigned int			unused;
	unsigned int			got_type;
	unsigned int			name_length = 0;
	unsigned int			value_length = 0;
	unsigned int			current_pos = *pos;
	unsigned int			sub_type_type;
	static unsigned int		level = 0;
	static unsigned int		state = C_TYPE_STATE_LOOKING;
	static unsigned int		internal_type = SYMBOLS_NUM_STRINGS;
	static unsigned int		type_def_length;
	static unsigned char	value_string[MAX_NAME_LENGTH];
	static unsigned char	name_string[MAX_NAME_LENGTH];
	static unsigned char	type_definition[MAX_NAME_LENGTH * 3];

	if (line[*pos] == '#')
	{
		/* pre-processor line - skip it */
		*pos = line_length;
	}
	else if (state == C_TYPE_STATE_LOOKING || state == C_TYPE_STATE_GOT_TYPEDEF)
	{
		/* find something */
		if (state != C_TYPE_STATE_GOT_TYPEDEF && (word = symbols_check_word(&line[*pos])) == SYMBOLS_TYPEDEF)
		{
			state = C_TYPE_STATE_GOT_TYPEDEF;
			*pos = *pos + symbols_get_length(word);
			memcpy(type_definition,symbols_table[word].name,symbols_get_length(word));
			type_def_length  = symbols_get_length(word);
		}
		else if (state == C_TYPE_STATE_GOT_TYPEDEF)
		{
			word = symbols_check_word(&line[*pos]);
			*pos = *pos + symbols_get_length(word);

			switch(word)
			{
				case SYMBOLS_STRUCT:
				case SYMBOLS_UNION:
					internal_type = SYMBOLS_STRUCT;
					state = C_TYPE_STATE_GET_TAG;
				
					type_definition[type_def_length++] = ' ';
					memcpy(&type_definition[type_def_length],symbols_table[word].name,symbols_get_length(word));
					type_def_length += symbols_get_length(word);
					break;

				case SYMBOLS_ENUM:						/* enum is wrong here - but we are not a c-compiler */
					internal_type = SYMBOLS_ENUM;
					state = C_TYPE_STATE_GET_TAG;
				
					type_definition[type_def_length++] = ' ';
					memcpy(&type_definition[type_def_length],symbols_table[word].name,symbols_get_length(word));
					type_def_length += symbols_get_length(word);
					break;

				default:
					/* found a typedef now looking for a plain type */
					got_type = c_get_type_name(line,line_length,pos,&return_type,&name,&unused);
					state = C_TYPE_STATE_GET_TYPE;

					if (got_type == 2)
					{
						result = TYPE_STATE_FAILED;
					}
					break;
			}
		}
		else
		{
			/* start decoding a plain type */
			if (got_type = c_get_type_name(line,line_length,pos,&return_type,&name,&unused))
			{
				if (got_type == 2)
				{
					result = TYPE_STATE_FAILED;
				}
				else
				{
					printf("ERROR: got the type: successfully\n");
					
					add_api_start_atom(atom_list,RECORD_GROUP_TYPE,current_group);
					
					result = TYPE_STATE_FAILED;
				}
			}
		}
	}
	else if (state == C_TYPE_STATE_GET_TAG)
	{
		/* get the tag name after the struct/union/enum definition */
		if(line[current_pos] == '{')
		{
			current_pos++;
			
			add_api_start_atom(atom_list,RECORD_GROUP_RECORD,current_group);
			add_string_atom(atom_list,ATOM_TYPE,type_definition,type_def_length,0);

			if (internal_type == SYMBOLS_STRUCT)
			{
				level = 0;
				state = C_TYPE_STATE_GET_STRUCT_FIELD;
			}
			else
			{
				level = 0;
				state = C_TYPE_STATE_GET_ENUM_FIELD;
			}
		}
		else
		{
			token_start = current_pos;
			type_definition[type_def_length++] = ' ';
			
			/* get the type */
			while (current_pos < line_length && is_valid_char[line[current_pos]])
			{
				type_definition[type_def_length++] = line[current_pos++];
			}

			/*remove whitespace */
			while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20))
			{
				current_pos++;
			}

			/* are we at a valid place after the tag has been found? */
			if (line[current_pos] < 0x0f)
			{
				current_pos == line_length;
			}
			else if (line[current_pos] != '{')
			{
				result = TYPE_STATE_FAILED;
			}
		}

		*pos = current_pos;
	}
	else if (state == C_TYPE_STATE_GET_TYPE)
	{
		/* expect a type - do get it */
		if (got_type = c_get_type_name(line,line_length,pos,&return_type,&name,&unused))
		{
			if (got_type == 2)
			{
				result = TYPE_STATE_FAILED;
			}
			else
			{
				printf("ERROR: got the type: successfully\n");
				result = TYPE_STATE_FAILED;
			}
		}
	}
	else if (state == C_TYPE_STATE_GET_NAME || state == C_TYPE_STATE_GET_SUB_NAME)
	{
		/* get a name string */
		token_start = current_pos;

		while (current_pos < line_length && is_valid_char[line[current_pos]])
		{
			name_string[name_length++] = line[current_pos++];
		}

		if (line[current_pos] == ';')
		{
			*pos = current_pos + 1;
			/* found the end of the type - start looking again */
			if (state == C_TYPE_STATE_GET_SUB_NAME)
			{
				name_string[name_length] = 0;
				state = C_TYPE_STATE_GET_STRUCT_FIELD;
			}
			else
			{
				name_string[name_length] = 0;
				state = C_TYPE_STATE_LOOKING;
				result = TYPE_STATE_FINISHED;
			}

			add_string_atom(atom_list,ATOM_NAME,name_string,name_length,0);
		}
		else
		{
			result = TYPE_STATE_FAILED;
		}
	}
	else if (state == C_TYPE_STATE_GET_STRUCT_FIELD)
	{
		/* handle the struct list item - note the does allow for nested structs/unions */
		if (line[current_pos] == '}')
		{
			if (level == 0)
			{
				state = C_TYPE_STATE_GET_NAME;
			}
			else
			{
				level--;
				state = C_TYPE_STATE_GET_SUB_NAME;
			}
			*pos = *pos + 1;
		}
		else if (got_type = c_get_type_name(line,line_length,pos,&return_type,&name,&sub_type_type))
		{
			if (got_type == 2)
			{
				if (line[*pos] == '{' && (sub_type_type == SYMBOLS_STRUCT || sub_type_type == SYMBOLS_UNION))
				{
					add_string_atom(atom_list,ATOM_RECORD,(unsigned char*)symbols_table[sub_type_type].name,symbols_get_length(sub_type_type),0);
					
					level++;
					*pos = *pos + 1;
				}
				else
				{
					result = TYPE_STATE_FAILED;
				}
			}
			else
			{
				add_api_type_atom (atom_list,ATOM_TYPE,&return_type,&name,NULL);
				*pos = *pos + 1;
			}
		}
	}
	else if (state == C_TYPE_STATE_GET_ENUM_FIELD) 
	{
		/* all enum fields must follow the 'NAME {= VALUE}{,} {trailing comment}' */
		token_start = current_pos;
		name.name			= &line[current_pos];
		name.name_length	= current_pos; 
		value.name_length	= 0;

		while (current_pos < line_length && is_valid_char[line[current_pos]])
		{
			current_pos++;
		}
		
		name.name_length = current_pos - name.name_length; 

		/* remove whitespace */
		while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20))
		{
			current_pos++;
		}

		if (line[current_pos] == '=')
		{
			current_pos++;

			/* remove whitespace */
			while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20))
			{
				current_pos++;
			}

			/* get the value */
			value.name			= &line[current_pos];
			value.name_length	= current_pos;

			while (current_pos < line_length && is_valid_char[line[current_pos]])
			{
				current_pos++;
			}

			value.name_length = current_pos - value.name_length;
		
			/* remove whitespace */
			while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20))
			{
				current_pos++;
			}
		}
	
		/* only add if we have found a type or a name */
		if (name.name_length > 0)
		{
			add_pair_atom(atom_list,ATOM_TYPE,name.name,name.name_length,value.name,value.name_length);
		}

		/* have we found the end of line */
		if (line[current_pos] < 0x0f || line[current_pos] == ',')
		{
			current_pos++;
		}
		else if (line[current_pos] == '}')
		{
			/* ok, reached the end of the enum definition */
			current_pos++;
			state = C_TYPE_STATE_GET_NAME;
		}
		else if (line[current_pos] != g_comment_start)
		{
			state = C_TYPE_STATE_LOOKING;
			result = TYPE_STATE_FAILED;
		}

		*pos = current_pos;
	}
	else
	{
		result = TYPE_STATE_FAILED;
	}
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : c_decode_constant
 * Desc : This function will decode a c constant.
 *        In this case c constants are just the #defines. variable constants can
 *        be caught by the @gobals as this is designed for type captures.
 *
 *        This is looking for defines of the type:
 *        #define <NAME> = <VALUE>
 *
 *        It does handle multiple comments. If you need a specific code snippit
 *        i.e. a complex MACRO then the @snippit - @end_snippit pair should be
 *        used instead.
 *
 *        Also, unlike other functions that will fail/produce warnings this code
 *        does not because the #define's can define constants and MACROS.
 *--------------------------------------------------------------------------------*/
unsigned int c_decode_constant(unsigned char* line,unsigned int line_length,unsigned int* pos, unsigned int current_group, ATOM_INDEX* atom_list)
{
	unsigned int		got_type;
	unsigned int 		result = 0;
	unsigned int		line_end = line_length;
	unsigned int		current_pos = *pos;
	unsigned int		name_start;
	unsigned int		value_start;
	unsigned int		name_length = 0;
	unsigned int		type_type;
	unsigned char		define_name[MAX_NAME_LENGTH];
	NAME				type_name;
	NAME				return_type;
	static unsigned int	state = C_CONSTANT_STATE_LOOKING;
	static unsigned int brack_count = 0;

	switch(state)
	{
		case C_CONSTANT_STATE_CONTINUATION:
		{
			/* remove leading whitespace */
			while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20))
			{
				current_pos++;
			}

			/* test the end of the line for the continuation mark */
			while(line_end > current_pos && (line[line_end] == 0x0a || line[line_end] == 0x0d))
			{
				line_end--;
			}

			if (line[line_end] == '\\')
			{
				state = C_CONSTANT_STATE_CONTINUATION;
				current_pos = line_end;
			}
			else
			{
				state = C_CONSTANT_STATE_LOOKING;
				line_end = current_pos;
			}
		}
		break;

		case C_CONSTANT_STATE_LOOKING:
		{
			if (line[current_pos] == '#')
			{
				*pos = current_pos + 1;
				state = C_CONSTANT_STATE_DEFINE;
			}
			else if (got_type = c_get_type_name(line,line_length,pos,&return_type,&type_name,&type_type))
			{
				if (got_type == 2)
				{
					/* failed to get a type - so fail the whole thing */
					state = C_CONSTANT_STATE_FAILED;
					result = 2;
				}
				else
				{
					add_api_start_atom(atom_list,RECORD_GROUP_CONSTANT,current_group);
					state = C_CONSTANT_STATE_TYPE_DATA;
				}
			}
		}
		break;

		case C_CONSTANT_FOUND_EQUALS:
		{
			/* remove leading whitespace */
			while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20))
			{
				current_pos++;
			}
			
			if (current_pos < line_length)
			{
				/* look for type starters */
				if (line[current_pos] == '"')
				{
					state = C_CONSTANT_GET_STRING;
				}
				else if (line[current_pos] >= 0 && line[current_pos] <= 9)
				{
					state = C_CONSTANT_GET_NUMBERS;
				}
				else if (is_valid_char[line[current_pos]])
				{
					state = C_CONSTANT_GET_NAME;
				}
				else if (line[current_pos] == '{')
				{
					state = C_CONSTANT_GET_ARRAY_DATA;
				}
				else
				{
					/* fail as given data invalid */
					state = C_CONSTANT_STATE_FAILED;
					result = 2;
				}
			}
		}
		break;

		case C_CONSTANT_STATE_TYPE_DATA:
		{
			/* add the type */
			add_api_type_atom (atom_list,ATOM_CONSTANTS,&return_type,&type_name,NULL);

			if (line[current_pos] != '=')
			{
				state = C_CONSTANT_STATE_FAILED;
				result = 2;
			}
			else
			{
				*pos = current_pos + 1;
				state = C_CONSTANT_FOUND_EQUALS;
			}
		}
		break;

		case C_CONSTANT_GET_NAME:
		case C_CONSTANT_GET_NUMBERS:
		{
			unsigned int	start_pos = current_pos;
	
			while (is_valid_char[line[current_pos]])
			{
				current_pos++;
			}

			add_string_atom(atom_list,ATOM_CONSTANTS,&line[start_pos],(current_pos - start_pos),0);
			
			if (line[current_pos] == ';')
			{
				result = 1;
				state = C_CONSTANT_STATE_LOOKING;
				current_pos++;
			}
			else
			{
				result = 2;
				state = C_CONSTANT_STATE_LOOKING;
			}
			
			/* always end the atom - work or fail */
			add_api_end_atom (atom_list,current_group);


			*pos = current_pos;
		}
		break;
		
		case C_CONSTANT_GET_ARRAY_DATA:
		{
			unsigned int	start_pos = current_pos;
			
			do
			{
				if (line[current_pos] == '/' && line[current_pos+1] == '*')
				{
					/* split the string around the comment */
					current_pos--;
					break;
				}

				if (line[current_pos] == '{')
				{
					brack_count++;
				}
				else if (line[current_pos] == '}')
				{
					brack_count--;
				}

				current_pos++;
			}
			while (current_pos < line_length && brack_count > 0);
		
			if ((current_pos - start_pos) > 0)
			{
				add_string_atom(atom_list,ATOM_CONSTANTS,&line[start_pos],(current_pos - start_pos),0);
			}

			if (line[current_pos] == ';')
			{
				add_api_end_atom (atom_list,current_group);
				result = 1;
				state = C_CONSTANT_STATE_LOOKING;
				current_pos++;
			}


			*pos = current_pos;
		}
		break;

		case C_CONSTANT_GET_STRING:
		{	
			
			if (line[current_pos] == '"')
			{
				unsigned int	start_pos = current_pos;
				current_pos++;

				while (current_pos < line_length && (line[current_pos] != '"'))
				{
					current_pos++;
				}

				if (line[current_pos] == '"')
				{
					add_string_atom(atom_list,ATOM_CONSTANTS,&line[start_pos],(current_pos - start_pos),0);
				}
			}

			if (line[current_pos] == ';')
			{
				add_api_end_atom (atom_list,current_group);
				result = 1;
				state = C_CONSTANT_STATE_LOOKING;
			}

			*pos = current_pos + 1;
		}
		break;

		case C_CONSTANT_STATE_DEFINE:
		{
			/* found the start of a constant definition */
			if ((line_length - current_pos) > 6 && memcmp("define",&line[current_pos],6) == 0)
			{
				current_pos += 6;

				/* remove white space */
				while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20))
				{
					current_pos++;
				}

				/* Ok, we have found a define - lets find the equals */
				name_start = current_pos;

				while (current_pos < line_length && is_valid_char[line[current_pos]])
				{
					define_name[name_length++] = line[current_pos++];
				}

				/* Ok, it is what we are looking for */
				if ((line[current_pos] == 0x09 || line[current_pos] == 0x20))
				{
					/* remove white space */
					do
					{
						current_pos++;
					}
					while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20));


					/* TODO: make the following a function and call it at the end of the mutliple line stuff above */


					/* test the end of the line for the continuation mark */
					while(line_end > current_pos && (line[line_end] == 0x0a || line[line_end] == 0x0d))
					{
						line_end--;
					}

					if (line[line_end] == '\\')
					{
						state = C_CONSTANT_STATE_CONTINUATION;
						line_end = line_length;
					}
					else
					{
						/* Ok, we will have find the end of constant as it does not have a continuation
						 * So the only thing that we are worried about on this line, is EOF or start of
						 * a comment. 
						 */
						value_start = current_pos;
						while (current_pos < (line_length - 1) && (line[line_end] != 0x0a && line[line_end] != 0x0d))
						{
							if (c_find_comment(line,line_length,&current_pos))
							{
								/* Ok, found a comment - skip back over it. */
								current_pos -= g_comment_start_length + 1;
								break;
							}
							current_pos++;
						}

						/* no need to return the whitespace */
						line_end = current_pos;

						/* now trim the white space */
						while (line[current_pos] < 0x0f || line[current_pos] == 0x20)
						{
							current_pos--;
						}

						add_pair_atom(atom_list,ATOM_RECORD,&line[name_start],name_length,&line[value_start],current_pos-value_start+1);
						state = C_CONSTANT_STATE_LOOKING;
					}
				}
			}
			*pos = line_end;
		}
		break;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: c_trim_multiline
 * @desc: This function will trim the comment block of whitespace for the
 *        multiline comments.
 *--------------------------------------------------------------------------------*/
unsigned int	c_trim_multiline(unsigned char* line,unsigned int line_length,unsigned int* pos)
{
	unsigned int 	result = 0;
	unsigned int 	current_pos = *pos;
	unsigned char	h_rule;

	if (current_pos < line_length && line[current_pos] == g_comment_cont_marker)
	{
		/* ok, it's the marker */
		current_pos++;

		/* if a char is butted to the cont_marker and is not the pre_marker - delete all of them */
		if (current_pos < line_length && (line[current_pos] != 0x09 && line[current_pos] != 0x20) && line[current_pos] != g_comment_pre_marker)
		{
			h_rule = line[current_pos];
			current_pos++;
			
			/* ok, no space between cont marker and the char - assume hrule */
			while (current_pos < line_length && line[current_pos] == h_rule)
			{
				current_pos++;
			}
		}
		else
		{
			/* remove white space */
			while (current_pos < line_length && (line[current_pos] == 0x09 || line[current_pos] == 0x20))
			{
				current_pos++;
			}
		}
	}

	if (line[current_pos] == g_comment_pre_marker)
	{
		/* skip the pre-marker */
		current_pos++;
		result = 1;
	}

	*pos = current_pos;

	return result;
}

