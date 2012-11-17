/**-------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: parse_markdown
 *   @desc: This file holds the functions that parse the markdown files that are
 *          used as the structure to produce the output docs.
 * @author: pantoine
 *    date: 12/07/2012 07:09:30
 *
 * @application	pdp
 * @section		MarkDown Supported Format
 * 			
 * 			The format of markdown that is supported is a (at the moment) subset
 * 			of that described at: http://daringfireball.net/projects/markdown/
 * 			It does not currently support underling but that will change. Also
 * 			it is intended to extend the markup by adding support for tables and
 * 			a couple of other features. See the specification for details.
 *
 * 			Also, a couple of features from Extended Markdown and Python Markdown
 * 			have been added and of course extensions to support including the 
 * 			model and to make the Markdown a bit more useful for generating 
 * 			actual documents.
 *
 * @ignore
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include "utf8.h"
#include "output.h"
#include "utilities.h"
#include "error_codes.h"
#include "level_index.h"
#include "output_format.h"
#include "parse_markdown.h"
#include "document_generator.h"

#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>

/*--------------------------------------------------------------------------------*
 * constants required for the decode.
 *--------------------------------------------------------------------------------*/
static unsigned char	doc_gen_string[]	= "doc_gen";
static unsigned int		doc_gen_size		= sizeof(doc_gen_string) - 1;

enum
{
	MARKDOWN_NORMAL_BLOCK,
	MARKDOWN_TABLE,
	MARKDOWN_SIMPLE_TABLE,
	MARKDOWN_CODE_BLOCK,
	MARKDOWN_HEADER
};

/*--------------------------------------------------------------------------------*
 * structure for keeping track of the state of the parse.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned char*	buffer;			/* the data buffer that holds the file that was read in */
	unsigned int	in_block;		/* the decode is in a block, so some of the markups are turned off */
	unsigned int	in_index;		/* turns off the markdowns index tracking - as the indexer handles this */
	unsigned int	buffer_size;	/* the size of the buffer (file) */
	unsigned int	buffer_pos;		/* the position of the decode within the buffer */
	unsigned int	line_number;	/* the number of lines of the file that have been read */
	unsigned int	line_start;		/* the position within the buffer of the first char to be decoded for the line */
	unsigned int	space_count;	/* the number of spaces at the start of the line */
	DRAW_STATE*		draw_state;		/* the draw state for the decode */
	INPUT_STATE*	input_state;	/* the state of the input */

} MARKDOWN_STATE;

/*--------------------------------------------------------------------------------*
 * forward definition of the parser functions
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_hash(MARKDOWN_STATE* state);
static unsigned int	markdown_parse_list_block(MARKDOWN_STATE* state);
static unsigned int	markdown_parse_numbered_header(MARKDOWN_STATE* state);
static unsigned int markdown_parse(MARKDOWN_STATE* state);
static unsigned int markdown_parse_fenced_code_block(MARKDOWN_STATE* state);
static unsigned int	markdown_parse_block(MARKDOWN_STATE* state);
static unsigned int	markdown_parse_quote_block(MARKDOWN_STATE* state);

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_loadfile
 * Desc : This function will load the file into memory for the markdown.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_loadfile(INPUT_STATE* input_state)
{
	off_t         offset;
	unsigned int count;
	unsigned int  result = EC_FAILED;
	unsigned int  file_pos = 0;
	unsigned int  file_size;

	/* check for end of file */
	if ((offset = lseek(input_state->input_file,0,SEEK_END)) > 0)
	{
		lseek(input_state->input_file,0,SEEK_SET);

		/* minus numbers are evil :) - also might clip the file size */
		file_size = (unsigned int) offset;

		/* 6 byte extension is to allow for utf-8 decoding - guard against evil people */
		if (((file_size + 6) > file_size) && (input_state->buffer = malloc(file_size + 10)) == NULL)
		{
			result = EC_OUT_OF_MEMORY_OR_FILE_TO_BIG;
			raise_warning(0,result,(unsigned char*)input_state->input_name,NULL);
		}
		else
		{
			/* this will save a lot of effort when reading the file - on a dword boundary incase this
			 * causes the disk read code to have problems with unaligned writes. Means no special case
			 * if statements to handle the start of file cases.
			 */
			input_state->buffer[0] = 0x0a;
			input_state->buffer[1] = 0x0a;
			input_state->buffer[2] = 0x0a;
			input_state->buffer[3] = 0x0a;

			/* zero the extra bytes used for buffer overflow utf-8 decoding protection */
			input_state->buffer[file_size - 9] = 0x00;
			input_state->buffer[file_size - 8] = 0x00;
			input_state->buffer[file_size - 7] = 0x00;
			input_state->buffer[file_size - 6] = 0x00;
			input_state->buffer[file_size - 5] = 0x00;
			input_state->buffer[file_size - 4] = 0x00;

			file_pos += 4;

			/* load the file into the input_state->buffer */
			do
			{
				if ((offset = read(input_state->input_file,&input_state->buffer[file_pos],file_size - file_pos)) < 0)
				{
					printf("load\n");
					result = EC_PROBLEM_WITH_INPUT_FILE;
					raise_warning(0,result,(unsigned char*)input_state->input_name,NULL);
					break;
				}

				file_pos += offset;
			}
			while (file_pos < (file_size+4) && offset > 0);

			if (file_pos == file_size)
			{
				/* we have read the whole file */
				input_state->buffer_size = file_size + 4;
				input_state->buffer_pos  = 4;
				result = EC_OK;
			}
			else
			{
				free(input_state->buffer);
				input_state->buffer = NULL;
			}
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_whitespace_level
 *  desc: This function will remove white space and return the level of the 
 *        whitespace. This level is either the number of tabs, or the number
 *        of spaces / 4.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_whitespace_level(MARKDOWN_STATE* state, unsigned int start_pos, unsigned int* level )
{
	unsigned int pos = start_pos;
	unsigned int space_count = 0;

	*level = 0;

	while (state->buffer[pos] == 0x20 || state->buffer[pos] == 0x09)
	{
		if (state->buffer[pos] == 0x20)
			space_count++;
		else
			space_count += 4;

		pos++;
	}

	/* relative level to the start of the list/block item */
	if (space_count < state->space_count)
	{
		*level = 0;
	}
	else
	{
		*level = ((space_count - state->space_count)/4);
	}

	return pos;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_unload_file
 *  desc: This function will unload the file. This function does not close the 
 *        file as it is not its responsibility, but will seek to start to leave
 *        the file in a nice state.
 *--------------------------------------------------------------------------------*/
static void	markdown_unload_file(INPUT_STATE* input_state)
{
	if (input_state->buffer != NULL)
	{
		free(input_state->buffer);
		input_state->buffer = NULL;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_skip_newline
 * Desc : This function will handle skipping the new lines.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_skip_newline(MARKDOWN_STATE* state, unsigned int buffer_pos)
{
	unsigned int result = buffer_pos;
	
	if (state->buffer[buffer_pos] == 0x0a && state->buffer[buffer_pos+1] == 0x0d)
	{
		/* Is DOS */
		result = buffer_pos + 2;
	}
	else if (state->buffer[buffer_pos] == 0x0d && state->buffer[buffer_pos + 1] == 0x0a)
	{
		/* Is f**king MAC */
		result = buffer_pos + 2;
	}
	else
	{
		/* unix or VAX (unix - 0x0a, vax - 0x0d) */
		result = buffer_pos + 1;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: handle_newline
 *  desc: This function will have the different newline chars.
 *--------------------------------------------------------------------------------*/
static unsigned int	handle_newline(MARKDOWN_STATE* state, unsigned int buffer_pos)
{
	unsigned int result;

	result = markdown_skip_newline(state,buffer_pos);

	/* update the states */
	state->line_number++;
	state->line_start = buffer_pos;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api		markdown_parse_buffer
 * @group	markdown_api
 * @desc	This function will parse a buffer for markdown markup. It uses the 
 *        	special blend of markdown markers that this function supports.
 *
 * @parameter	buffer		The buffer to parse for markdown.
 * @parameter	buffer_size	The size of the buffer.
 * @parameter	in_block	Is the buffer in a block, so is it allowed to include
 *           	            top level markdown items.
 *--------------------------------------------------------------------------------*/
unsigned int	markdown_parse_buffer(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned char* buffer, unsigned int buffer_size, unsigned int in_block)
{
	unsigned int	index;
	unsigned int	result = EC_OK;
	unsigned int	curr_pos = 0;
	unsigned int	block_state = in_block;
	unsigned char	char_size;
	MARKDOWN_STATE	state;

	memset(&state,0,sizeof(MARKDOWN_STATE));
	state.in_block		= in_block;
	state.draw_state	= draw_state;
	state.input_state	= input_state;
	state.buffer		= buffer;
	state.buffer_size	= buffer_size;
	state.line_number	= 0;

	if ((char_size = utf8_char_size(&state.buffer[state.buffer_pos])) == 1)
	{
		/* first call to handle the start of the buffer - assumes new line */
		result = markdown_parse(&state);
	}

	while (state.buffer_pos < state.buffer_size)
	{
		if ((char_size = utf8_char_size(&state.buffer[state.buffer_pos])) == 0)
		{
			/* invalid utf8 char - replace with a safe char */
			state.buffer[state.buffer_pos++] = '.';
		}
		else if (char_size == 1 && (state.buffer[state.buffer_pos] == 0x0a || state.buffer[state.buffer_pos] == 0x0d))
		{
			/* only interested in UNICODE page 0 - get the index into the jump table and call that function */
			result = markdown_parse(&state);
		}
		else
		{
			state.buffer_pos += char_size;
		}
	}

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api		markdown_parse_input
 * @group	markdown_api
 * @desc	This function will parse the input in the input state. It will cause
 *			the buffer to be loaded in to the file space and then handle the 
 *			parsing of the file.
 *			
 *			The sub-functions do not take the INPUT_STATE, so that they can be used
 *			on other buffer data within some of the model items, this will make
 *			there usage more flexible.
 *
 * @parameter	draw_state	The state of the output.
 * @parameter	input_state	The state of the input.
 *--------------------------------------------------------------------------------*/
unsigned int	markdown_parse_input(DRAW_STATE* draw_state,INPUT_STATE* input_state)
{
	unsigned int result = EC_FAILED;

	if (draw_state->format == NULL)
	{
		result = EC_UNSUPPORTED_OUTPUT_FORMAT;
		raise_warning(0,result,(unsigned char*)input_state->input_name,NULL);
	}
	else if ((input_state->input_file = open((char*)input_state->input_name,READ_FILE_STATUS)) == -1)
	{
		result = EC_FAILED_TO_OPEN_INPUT_FILE;
		raise_warning(0,result,(unsigned char*)input_state->input_name,NULL);
	}
	else if ((result = markdown_loadfile(input_state)) == EC_OK)
	{
		result = markdown_parse_buffer(draw_state,input_state,&input_state->buffer[3],input_state->buffer_size-4,0);

		markdown_unload_file(input_state);
	
		close(input_state->input_file);
	}

	return result;
}

/*--------------------------------------------------------------------------------*
 * Markdown parser functions
 *--------------------------------------------------------------------------------*/

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_parse
 * Desc : This function will handle parsing the markdown. It assumes that it is
 *        aligned to a newline.
 *--------------------------------------------------------------------------------*/
unsigned int	markdown_parse(MARKDOWN_STATE* state)
{
	unsigned int result = EC_OK;
	
	/* handle the initial new line */
	state->buffer_pos = handle_newline(state,state->buffer_pos);

	/* handle multiple newlines */
	while (state->buffer[state->buffer_pos] == 0x0a || state->buffer[state->buffer_pos] == 0x0d)
	{
		state->buffer_pos = handle_newline(state,state->buffer_pos);
	}

	/* count the spaces on entry */
	state->space_count = 0;

	while (state->buffer[state->buffer_pos] == 0x20 || state->buffer[state->buffer_pos] == 0x09)
	{
		if (state->buffer[state->buffer_pos] == 0x20)
			state->space_count++;
		else
			state->space_count += 4;

		state->buffer_pos++;
	}

	switch (state->buffer[state->buffer_pos])
	{
		case '#':	result = markdown_parse_hash(state);				break;
		case '`':	result = markdown_parse_fenced_code_block(state);	break;
		case '*':
		case '+':
		case '-':	result = markdown_parse_list_block(state);			break;
		case '>':	result = markdown_parse_quote_block(state);			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
				if ((result = markdown_parse_numbered_header(state)) != EC_OK)
				{
					result = markdown_parse_list_block(state);
				}
				break;

		default:
				result = markdown_parse_block(state);
	}

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_parse_square_bracket
 *  desc: This function will capture the data within the square bracket pair.
 *
 * @application	pdp
 * @section		Supported References
 *
 *	The Markdown supports the following styles of references.
 *		
 *		\[reference\]
 *	    \[name\]\[reference\]
 *
 *	Where as reference is the URI that the reference points to. If the reference
 *	is a URI that starts with "doc_gen:" then this is decoded as a reference that
 *	points to the internal model and the model tree is searched for that reference.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_square_bracket(MARKDOWN_STATE* state, unsigned int start_pos)
{
	NAME			title = {NULL,0,0,0};
	NAME			reference = {NULL,0,0,0};
	unsigned int	pos = start_pos + 1;
	unsigned int	start = pos;
	unsigned int	colon = start;
	unsigned int	result = pos;
	unsigned int	end_url = 0;
	unsigned int	start_url = pos;
	unsigned int	have_title = 0;

	/* search for the rest of the boxes */
 	while (pos < state->buffer_size)
	{
		if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
		{
			result = pos;
			raise_warning(state->line_number,EC_UNEXPECTED_END_OF_LINE,(unsigned char*)state->input_state->input_name,NULL);
			break;
		}
		else if (state->buffer[pos] == ']' && have_title == 0)
		{
			if (state->buffer[pos+1] == '[' || ((state->buffer[pos+1] == 0x20 || state->buffer[pos+1] == 0x09) && state->buffer[pos+2] == '['))
			{
				have_title	= 1;
				title.name  = &state->buffer[start];
				title.name_length = pos - start;

				if (state->buffer[pos+1] == '[')
					start_url = pos + 2;
				else
					start_url = pos + 3;
			}
			else
			{
				/* Ok, we have the end of the box */
				result = pos + 1;

				end_url = pos;
				break;
			}
		}
		else if (state->buffer[pos] == ']')
		{
			result = pos + 1;

			end_url = pos;
			break;
		}

		pos++;
	}
	
	/* decode the url here */
	if (end_url > (start_url + 8))
	{
		if (state->buffer[start_url + 7] == ':' && memcmp(&state->buffer[start_url],doc_gen_string,doc_gen_size) == 0)
		{
			/* copy the title */
			state->input_state->title.name = title.name;
			state->input_state->title.name_length = title.name_length;

			/* ok it's a doc_gen url and we need to handle it */
			reference.name = &state->buffer[start_url + 8];
			reference.name_length = end_url - start_url - 8;

			/* handle the model objects */
			state->input_state->line_number = state->line_number;
			output_parse_model(state->draw_state,state->input_state,state->draw_state->model,&reference);
		}
		else
		{
			/* TODO: output the plain URL --- if it is not a ^footer */
		}
	}

	state->buffer_pos = result;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_inline_code
 *  desc: This function will decode the inline text.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_inline_code(MARKDOWN_STATE* state, unsigned int start_pos)
{
	NAME			code = {NULL,0,0,0};
	unsigned int	result = start_pos;
	
	result += 2;

	/* find the end of the selection */
	while(result < state->buffer_size && state->buffer[result] != 0x0a && state->buffer[result] != 0x0d &&
			(state->buffer[result] != '`' && state->buffer[result+1] != '`')) 
	{
		result++;
	}

	/* write the code to the file */
	code.name			= &state->buffer[start_pos+2];
	code.name_length	= result-(start_pos+1);
	state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_CODE,&code);
	
	result += 3;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_macro
 *
 * @application	pdp
 * @section		Markdown Extensions
 *
 * As the documentation and models are designed to be placed within a build system
 * it makes sense for the markdown files to support macros. Macros allow for text 
 * to be specified in the doc_gen model files and in the plain text. This means
 * that the macros are fairly limited in there use, but will allow for generic 
 * document files to be used to generate documentation for different products.
 *
 * The format for macro's are as follows:
 *
 *     \$<macro_name>
 *
 * These are only allows in the main text and in the group and name parts of the 
 * model.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_macro(MARKDOWN_STATE* state, unsigned int start_pos)
{
	unsigned int	result = start_pos;
	NAME			macro;
	LOOKUP_ITEM*	macro_item;

	/* skip the "$" */
	result++;

	/* find the end of the macro */
	while(result < state->buffer_size && state->buffer[result] != 0x0a && state->buffer[result] != 0x0d && state->buffer[result] != 0x20 && state->buffer[result] != 0x09)
	{
		result++;
	}

	/* lookup the macro */
	if ((macro_item = find_lookup(&state->draw_state->macro_lookup,&state->buffer[start_pos+1],result-start_pos-1)) != NULL)
	{
		macro.name = macro_item->payload;
		macro.name_length = macro_item->payload_length;
		state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&macro);
	}
	else
	{
		raise_warning(state->line_number,EC_UNKNOWN_MACRO,(unsigned char*)state->input_state->input_name,NULL);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_emphasis
 *  desc: This function will handle the inline emphasis of text.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_emphasis(MARKDOWN_STATE* state, unsigned int start_pos)
{
	NAME			text = {NULL,0,0,0};
	unsigned int	pos = start_pos + 1;
	unsigned int	level = 1;
	unsigned int	result = pos;
	unsigned char	emp_char = state->buffer[start_pos];
	
	while (state->buffer[pos] == emp_char && pos < state->buffer_size)
	{
		level++;
		pos++;
	}

	if (state->buffer[pos] != 0x20 && state->buffer[pos] != 0x09)
	{
		/* OK, valid emaphsis characters */
		text.name = &state->buffer[pos];

		/* lets find the end */
		while (state->buffer[pos] != emp_char && state->buffer[pos] != 0x0a && state->buffer[pos] != 0x0d && pos < state->buffer_size)
		{
			pos++;
		}

		if (state->buffer[pos] == emp_char)
		{
			/* Ok, we have the end of the emphasis */
			text.name_length = &state->buffer[pos] - text.name;

			if (level == 1)
			{
				state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_ITALIC,&text);
			}
			else
			{
				state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_BOLD,&text);
			}

			/* remove the trailing markers */
			while (state->buffer[pos] == emp_char && pos < state->buffer_size)
			{
				pos++;
			}
		}
		else
		{
			/* not valid just output it */
			text.name = &state->buffer[start_pos];
			text.name_length = pos - start_pos;
			state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);
		}

		result = pos;
	}
	else
	{
		/* not valid just output it */
		text.name = &state->buffer[start_pos];
		text.name_length = pos - start_pos + 1;
		state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);
		result = pos;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_consume_line
 * Desc : This function will consume a single line of text within a block. It can
 *        only handle the basic formatting markup, and all other markers will be
 *        ignored.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_consume_line(MARKDOWN_STATE* state, unsigned int start_pos)
{
	NAME			text = {NULL,0,0,0};
	unsigned int	start = start_pos;
	unsigned int	result = start_pos;

	/* remove white space */
	while (result < state->buffer_size && (state->buffer[result] == 0x20 || state->buffer[result] == 0x09))
	{
		result++;
	}
	
	start = result;
	text.name = &state->buffer[result];
		
	/* now handle the text */
	while (result < state->buffer_size && state->buffer[result] != 0x0a && state->buffer[result] != 0x0d)
	{
		if (state->buffer[result] == '\\')
		{
			text.name_length = result-start;
			state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);
			result++;

			if (state->buffer[result] == '\\')
			{
				if (state->buffer[result-2] == ' ')
				{
					state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_ASCII_CHAR|' ');
				}

				state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_ASCII_CHAR|'\\');
				result++;
			}
			else
			{
				/* skip the following char */
				state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_ASCII_CHAR|state->buffer[result]);
				result++;
			}
				
			if (state->buffer[result] != ' ')
			{
				state->draw_state->no_space = 1;
			}


			text.name = &state->buffer[result];
			start = result;
		}
		else if (state->buffer[result] == '`' && state->buffer[result+1] == '`')
		{
			/* output the left over */
			text.name_length = result-start;
			state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);

			/* output the inline code */
			result = markdown_inline_code(state,result);
			
			/* set up for the next write */
			text.name = &state->buffer[result];
			start = result;
		}
		else if (state->buffer[result] == '$')
		{
			/* output the left over */
			text.name_length = result-start;
			state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);

			result = markdown_macro(state,result);
			/* set up for the next write */
			text.name = &state->buffer[result];
			start = result;
		}
		else if (state->buffer[result] == '*' || state->buffer[result] == '_')
		{
			text.name_length = result-start;
			state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);
			
			result = markdown_emphasis(state,result);
			
			text.name = &state->buffer[result];
			start = result;
		}
		else if (state->buffer[result] == '[')
		{
			text.name_length = result-start;
			state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);

			result = markdown_parse_square_bracket(state,result);
			
			/* set up for the next write */
			text.name = &state->buffer[result];
			start = result;
		}
		else
		{
			result++;
		}
	}

	if (start != result)
	{
		text.name_length = result-start;

		if (state->buffer[result-1] == ' ' && state->buffer[result-2] == ' ')
		{
			state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL|OUTPUT_TEXT_STYLE_T_NEWLINE,&text);
		}
		else
		{
			/* output the left overs */
			state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_get_block_type
 * Desc : This function will decode the block and workout what it's type is.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_get_block_type(MARKDOWN_STATE* state)
{
	unsigned int	pos = state->buffer_pos;
	unsigned int	result = MARKDOWN_NORMAL_BLOCK;
	unsigned char	line_char;

	if (state->space_count < 4)
	{
		while (pos < state->buffer_size)
		{
			if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
			{
				/* newline - search failed */
				pos = markdown_skip_newline(state,pos);
				break;
			}
			else if (state->buffer[pos] == '\\')
			{
				/* skip the next character */
				pos++;
			}
			else if (state->buffer[pos] == '|')
			{
				/* ok, we have a pipe, assume this is a table */
				result = MARKDOWN_TABLE;
				break;
			}

			pos++;
		}

		if (result == MARKDOWN_NORMAL_BLOCK)
		{
			if ((state->buffer[pos] == '-' && state->buffer[pos+1] == '-' && state->buffer[pos+2] == '-' && state->buffer[pos+3] == '-') ||
					(state->buffer[pos] == '=' && state->buffer[pos+1] == '=' && state->buffer[pos+2] == '=' && state->buffer[pos+3] == '='))
			{
				/* ok, we have a underline or a simple table */
				line_char = state->buffer[pos];
				pos += 4;

				result = MARKDOWN_HEADER;

				while (pos < state->buffer_size)
				{
					if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
					{
						break;
					}
					else if (state->buffer[pos] != line_char)
					{
						/* found a break */
						if (state->buffer[pos] == 0x09)
						{
							/* golden - a tab we have simple table -- if it's not users fault */
							result = MARKDOWN_SIMPLE_TABLE;
							break;
						}
						else if (state->buffer[pos] == 0x20 && state->buffer[pos+1] == 0x20 && state->buffer[pos+2] == 0x20 && state->buffer[pos+3] == 0x20)
						{
							/* as above */
							result = MARKDOWN_SIMPLE_TABLE;
							break;
						}
						else
						{
							raise_warning(state->line_number,EC_AMBIGUOUS_MARKUP,state->input_state->input_name,0);
							break;
						}
					}

					pos++;
				}
			}
		}
	}
	else if (state->space_count >= 4)
	{
		/* OK, 4 spaces - then this is a code block */
		result = MARKDOWN_CODE_BLOCK;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_parse_normal_block
 * Desc : This function will read the normal markdown block.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_normal_block(MARKDOWN_STATE* state)
{
	unsigned int result = EC_OK;
	unsigned int pos = state->buffer_pos;

	while (pos < state->buffer_size)
	{
		/* decode the line */
		pos = markdown_consume_line(state,pos);

		if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
		{
			/* ok, we have an end of line */
			pos = handle_newline(state,pos);

			if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
			{
				/* double return - end of block */
				state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_PARAGRAPH_BREAK);
				break;
			}
		}
	}

	state->buffer_pos = pos;

	return result;
}

#define MAX_COLUMNS	(10)

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_parse_table_block
 * Desc : This function will parse the pipe delimited table format.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_table_block(MARKDOWN_STATE* state)
{
	unsigned int	pos = state->buffer_pos;
	unsigned int	row = 0;
	unsigned int	count = 0;
	unsigned int	start = 0;
	unsigned int	boxed = 0;
	unsigned int	column = 0;
	unsigned int	whitespace;
	unsigned int	result = EC_OK;
	unsigned int	continuation = 0;
	NAME			column_data[MAX_TABLE_SIZE];
	NAME			column_header[MAX_TABLE_SIZE];
	TABLE_ROW		table_row;
	TABLE_LAYOUT	table_layout;
	
	/* Ok, start the table block */
	state->input_state->state = TYPE_TABLE;
	state->draw_state->format->output_header(state->draw_state,state->input_state);

	/* initialise the table */
	for (column=0; column < MAX_TABLE_SIZE; column++)
	{
		column_data[column].fixed = 0;
		table_row.row[column] = &column_header[column];
	}
		
	while (pos < state->buffer_size)
	{
		column = 0;

		if (state->buffer[pos] == '|')
		{
			/* first item is a bar, we are in a boxed table */
			boxed = 1;
			pos++;
		}
		
		if (row == 1 && state->buffer[pos] == ':')
		{
			/* left justified */
			table_layout.column[0].flags = OUTPUT_COLUMN_FORMAT_RIGHT_JUSTIFIED;
		}
		
		/* start of row */
		column_data[0].name = &state->buffer[pos];
		
		start = pos;

		while (pos < state->buffer_size && state->buffer[pos] != 0x0a && state->buffer[pos] != 0x0d)
		{
			if (state->buffer[pos] == '\\')
			{
				/* skip this and the next char */
				pos++;
			}
			else if (state->buffer[pos] == '|')
			{
				if (row == 0)
				{
					table_layout.column[column].width = pos - start;
					column_header[column].name = column_data[column].name;
					column_header[column].name_length = pos - start;
					column++;
				}
				else if (row == 1)
				{
					if (state->buffer[pos-1] == ':')
					{
						/* right justified */
						table_layout.column[column].flags |= OUTPUT_COLUMN_FORMAT_RIGHT_JUSTIFIED;
					}
					
					column_data[column].name_length = pos - start;

					column++;

					if (state->buffer[pos+1] == ':')
					{
						/* left justified */
						table_layout.column[column].flags = OUTPUT_COLUMN_FORMAT_LEFT_JUSTIFIED;
					}
				}
				else
				{
					whitespace = pos - 1;

					while (state->buffer[whitespace] == 0x20 || state->buffer[whitespace] == 0x09)
					{
						whitespace--;
					}

					column_data[column].name_length = whitespace - start + 1;
					column++;
				}

				start = pos + 1;
				column_data[column].name = &state->buffer[pos+1];
				continuation = 0;
			}

			pos++;
		}

		if (row == 0)
		{
			if (boxed && state->buffer[pos-1] == '|')
			{
				column--;
			}
			else
			{
				column_header[column].name = column_data[column].name;
				column_header[column].name_length = pos - start;
			}
				
			table_layout.column[column].width = column_header[column].name_length;
		
			if (boxed)
			{
				table_layout.table_flags |= OUTPUT_TABLE_FORMAT_BOXED;
			}

			table_layout.num_columns = column + 1;
			table_layout.column_spacing = 2;
		}
		else if (row == 1)
		{
			if (column == 0)
			{
				/* it is not a correctly formed table - so dump the text and run */
				column_data[0].name = &state->buffer[state->buffer_pos];
				column_data[0].name_length = pos - state->buffer_pos;

				state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&column_data[0]);
				raise_warning(state->line_number-1,EC_AMBIGUOUS_MARKUP,state->input_state->input_name,0);
				break;
			}

			if (!(boxed && state->buffer[pos-1] == '|'))
			{
				column_data[column].name_length = pos - start;
			}

			for (count=0;count<table_layout.num_columns;count++)
			{
				if (table_layout.column[count].width < column_data[count].name_length)
				{
					table_layout.column[count].width = column_data[count].name_length;
				}
			}

			/* start the table */
			state->draw_state->format->output_table_start(state->draw_state,&table_layout);

			/* Ok, output the header */
			state->draw_state->format->output_table_header(state->draw_state,&table_layout,&table_row);

			/* now back to column data */
			for (column=0; column < MAX_TABLE_SIZE; column++)
			{
				column_data[column].fixed = 0;
				table_row.row[column] = &column_data[column];
			}
		}
		else if (row > 1)
		{
			if (!(boxed && state->buffer[pos-1] == '|'))
			{
				column_data[column].name_length =  pos - start;
			}

			state->draw_state->format->output_table_row(state->draw_state,&table_layout,&table_row);
		}

		if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
		{
			row++;

			/* ok, we have an end of line */
			pos = handle_newline(state,pos);

			if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
			{
				/* double return - end of block */
				state->draw_state->format->output_table_end(state->draw_state,&table_layout);
				break;
			}
		}
	}
	
	state->draw_state->format->output_footer(state->draw_state,state->input_state);
	state->buffer_pos = pos;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_parse_simple_table_block
 * Desc : This function will decode the simple table.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_simple_table_block(MARKDOWN_STATE* state)
{
	unsigned int row = 0;
	unsigned int pos = state->buffer_pos - state->space_count;
	unsigned int column = 0;
	unsigned int result = EC_OK;
	unsigned int row_start = pos;
	unsigned int row_1_start = pos;
	unsigned int column_start = pos;
	NAME		 column_data[MAX_TABLE_SIZE];
	TABLE_ROW	 table_row;
	TABLE_LAYOUT table_layout;
	
	/* Ok, start the table block */
	state->input_state->state = TYPE_TABLE;
	state->draw_state->format->output_header(state->draw_state,state->input_state);

	/* initialise the table */
	for (column=0; column < MAX_TABLE_SIZE; column++)
	{
		column_data[column].fixed = 0;
		table_row.row[column] = &column_data[column];
	}

	while (pos < state->buffer_size)
	{
		column = 0;
		row_start = pos;
		column_start = pos;

		if (row == 1 && state->buffer[pos] == ' ')
		{
			table_layout.column[0].flags = OUTPUT_COLUMN_FORMAT_RIGHT_JUSTIFIED;
		}

		/* start of row */
		column_data[0].name = &state->buffer[pos];

		/* decode the line */
		while (pos < state->buffer_size && (state->buffer[pos] != 0x0a && state->buffer[pos] != 0x0d))
		{
			if (row == 1)
			{
				if (state->buffer[pos] != '-')
				{
					if (state->buffer[pos+1] == 0x09 || 
						(state->buffer[pos+1] == 0x20 && state->buffer[pos+2] == 0x20 && state->buffer[pos+3] && state->buffer[pos+4]))
					{
						/* add 4 if it's spaces - else add 1 for the tab */
						if (state->buffer[pos+1] == 0x20)
						{
							pos += 4;
						}
						else
						{
							pos++;
						}

						if (state->buffer[row_1_start+(pos-row_start)] == 0x20)
						{
							table_layout.column[column].flags |= OUTPUT_COLUMN_FORMAT_LEFT_JUSTIFIED;
						}
					}
					else
					{
						raise_warning(state->line_number,EC_AMBIGUOUS_MARKUP,state->input_state->input_name,0);
					}
					
					table_layout.column[column].width = pos-column_start;

					column++;

					if (column > MAX_COLUMNS)
					{
						raise_warning(state->line_number,EC_TOO_MANY_COLUMNS_IN_TABLE,state->input_state->input_name,0);
						break;
					}
					
					table_layout.column[column].flags = OUTPUT_COLUMN_FORMAT_LINE_CLIP;

					do
					{
						pos++;
					}
					while (pos < state->buffer_size && state->buffer[pos] != '-' && state->buffer[pos] != 0x0a && state->buffer[pos] != 0x0d);
	
					/* allow for tabs at the end of tables */
					if (state->buffer[row_1_start+(pos-row_start)] == 0x20 || state->buffer[row_1_start+(pos-row_start)] == 0x09)
					{
						table_layout.column[column].flags = OUTPUT_COLUMN_FORMAT_RIGHT_JUSTIFIED;
					}

					/* capture the start of the column header */
					column_start = pos;
				}
			}
			else if (row != 1 && 
					 (state->buffer[pos] == 0x09 || 
					 (state->buffer[pos] == 0x20 && state->buffer[pos+1] == 0x20 && state->buffer[pos+2] && state->buffer[pos+3])))
			{

				/* now capture the columns data */
				column_data[column].name_length = pos-column_start;
				column++;
					
				while (state->buffer[pos] == 0x09 || state->buffer[pos] == 0x20)
				{
					pos++;
				}
				
				column_data[column].name = &state->buffer[pos];
				column_start = pos;

				/* so that the end of loop increment does not throw away a good character */
				pos--;
			}

			pos++;
		}
	
		/* get the size of the last row */
		if (row != 1)
		{
			column_data[column].name_length = pos - column_start;
		}

		if (row == 1)
		{
			if (state->buffer[row_1_start+(pos-row_start)-2] == 0x20)
			{
				table_layout.column[column].flags = OUTPUT_COLUMN_FORMAT_LEFT_JUSTIFIED;
			}

			table_layout.num_columns = column + 1;
			table_layout.table_flags = (OUTPUT_TABLE_FORMAT_HEADER);
			table_layout.column_spacing = 2;
			table_layout.column[column].width = pos-column_start;

			/* need to point to column one */
			column_data[0].name = &state->buffer[row_1_start];

			/* start the table */
			state->draw_state->format->output_table_start(state->draw_state,&table_layout);

			/* Ok, output the header */
			state->draw_state->format->output_table_header(state->draw_state,&table_layout,&table_row);
		}
		else if (row > 1)
		{
			/* output the row */
			state->draw_state->format->output_table_row(state->draw_state,&table_layout,&table_row);
		}

		if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
		{
			row++;

			/* ok, we have an end of line */
			pos = handle_newline(state,pos);

			if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
			{
				/* double return - end of block */
				break;
			}
		}
	}
	
	state->draw_state->format->output_table_end(state->draw_state,&table_layout);
	state->draw_state->format->output_footer(state->draw_state,state->input_state);
	
	state->buffer_pos = pos;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_parse_header_block
 * Desc : This function will parse 
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_header_block(MARKDOWN_STATE* state)
{
	unsigned int pos = state->buffer_pos;
	unsigned int result = EC_OK;
	
	/* find the end of the header */
	while(pos < state->buffer_size && (state->buffer[pos] != 0x0a && state->buffer[pos] != 0x0d))
	{
		pos++;
	}
	
	pos = handle_newline(state,pos);

	/* now output the header */
	if (state->buffer[pos] == '-')
	{
		state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_HEADER_START|OHL_LEVEL_2);
		markdown_consume_line(state,state->buffer_pos);
		state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_HEADER_END|OHL_LEVEL_2);
	}
	else
	{
		state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_HEADER_START|OHL_LEVEL_1);
		markdown_consume_line(state,state->buffer_pos);
		state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_HEADER_END|OHL_LEVEL_1);
	}

	/* lose the underline */
	while(pos < state->buffer_size && (state->buffer[pos] != 0x0a && state->buffer[pos] != 0x0d))
	{
		pos++;
	}

	/* now throw in a paragraph break */
	pos = handle_newline(state,pos);
				
	state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_PARAGRAPH_BREAK);
	
	state->buffer_pos = pos;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_parse_code_block
 * Desc : This function decode the code blocks.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_code_block(MARKDOWN_STATE* state)
{
	NAME			text = {NULL,0,0,0};
	unsigned int	pos = state->buffer_pos;
	unsigned int	start;
	unsigned int	result = EC_OK;

	/* Ok, start the code block */
	state->input_state->state = TYPE_CODE_BLOCK;
	state->draw_state->format->output_header(state->draw_state,state->input_state);

	while (pos < state->buffer_size)
	{
		/* grab the raw text */
		start = pos;
		text.name = &state->buffer[pos];

		while (pos < state->buffer_size && (state->buffer[pos] != 0x0a && state->buffer[pos] != 0x0d))
		{
			pos++;
		}

		text.name_length = pos-start;
		state->draw_state->format->output_raw(state->draw_state,text.name,text.name_length);

		if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
		{
			/* ok, we have an end of line */
			pos = handle_newline(state,pos);

			if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
			{
				/* double return - end of block */
				break;
			}
		}

		if (state->buffer[pos] == 0x09)
		{
			pos++;
		}
		else if (state->buffer[pos] == ' ' && state->buffer[pos+1] == ' ' && state->buffer[pos+2] == ' ' && state->buffer[pos+3] == ' ')
		{
			pos += 4;
		}
		else
		{
			raise_warning(state->line_number,EC_AMBIGUOUS_MARKUP,state->input_state->input_name,0);
			break;
		}
	}

	/* exit and output markers */
	state->draw_state->format->output_footer(state->draw_state,state->input_state);

	state->buffer_pos = pos;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_parse_block
 * Desc : This function will parse the block of text that does not have a defined
 *        start marker.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_block(MARKDOWN_STATE* state)
{
	unsigned int result = EC_OK;
	unsigned int level;
	unsigned int search;
	unsigned int num_lines = 0;

	switch(markdown_get_block_type(state))
	{
		case MARKDOWN_TABLE:		result = markdown_parse_table_block(state);			break;
		case MARKDOWN_SIMPLE_TABLE:	result = markdown_parse_simple_table_block(state);	break;
		case MARKDOWN_CODE_BLOCK:	result = markdown_parse_code_block(state);			break;
		case MARKDOWN_HEADER:		result = markdown_parse_header_block(state);		break;
		default:
			result = markdown_parse_normal_block(state);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_parse_fenced_code_block
 *  desc: This function will parse for the fenced code block.
 *        The triple tick ``` which will be can only be found
 *        at the start of a line and denotes the start of a code block. This 
 *        block must be terminated by another triple-tick. As this is a code
 *        block, no other markup is decoded within this block.
 *--------------------------------------------------------------------------------*/
unsigned int markdown_parse_fenced_code_block(MARKDOWN_STATE* state)
{
	NAME			text = {NULL,0,0,0};
	unsigned int	pos = state->buffer_pos;
	unsigned int	start;
	unsigned int	num_lines = 0;
	unsigned int	result = EC_OK;

	if (state->buffer[pos+1] == '`' && state->buffer[pos+2] == '`')
	{
		pos += 3;

		/* start the code block */
		state->input_state->state = TYPE_CODE_BLOCK;
		state->input_state->title.name = NULL;
		state->input_state->title.name_length = 0;
		state->draw_state->format->output_header(state->draw_state,state->input_state);

		state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_CODE_START);
		start = pos;
		text.name = &state->buffer[pos];

		/* find the end of the code block */
		do
		{
			if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
			{
				/* output the text */
				text.name_length = pos-start;
				state->draw_state->format->output_raw(state->draw_state,text.name,text.name_length);

				pos = handle_newline(state,pos);
				start = pos;

				if (state->buffer[pos] == '`' && state->buffer[pos+1] == '`' && state->buffer[pos+2] == '`')
				{
					/* ok, we have found the end marker */
					pos += 3;

					/* clear the rest of the line */
					while (pos < state->buffer_size)
					{
						if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
						{
							pos = handle_newline(state,pos);
							break;
						}
						pos++;
					}
					break;
				}

				state->line_start = pos;

				/* look for the next line */
				start = pos;
				text.name = &state->buffer[pos];
			}
			else
			{
				pos++;
			}
		}
		while (pos < state->buffer_size);

		if (pos == state->buffer_size)
		{
			result = EC_UNTERMINATED_MARKUP_BLOCK;
			raise_warning(state->line_number,result,(unsigned char*)state->input_state->input_name,NULL);
		}

		state->buffer_pos = pos;

		state->draw_state->format->output_footer(state->draw_state,state->input_state);
	}
	else
	{
		/* This is not a fenced block, treat it as a normal block */
		markdown_parse_block(state);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_parse_hash
 *  desc: This function will count the number of hashes to see the header level
 *        required. It will also find the end of the header and remove the leading
 *        and trailing spaces.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_hash(MARKDOWN_STATE* state)
{
	NAME			text = {NULL,0,0,0};
	unsigned int	level = 1;
	unsigned int	header_start = 0;
	unsigned int	header_end = 0;
	unsigned int	result = EC_OK;
	unsigned int	pos = state->buffer_pos + 1;

	/* count the opening '#' as this sets the level */
	while (pos < state->buffer_size && state->buffer[pos] == '#')
	{
		if (state->buffer[pos] == '#')
		{
			pos++;
			level++;
		}
	}

	/* remove the whitespace */
	while(state->buffer[pos] == 0x020 || state->buffer[pos] == 0x09)
	{
		pos++;
	}
	
	/* output the start of the header */
	header_start = pos;
	text.name = &state->buffer[pos];

	/* now read the header name */
	while (pos < state->buffer_size)
	{
		if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d || state->buffer[pos] == '#')
		{
			/* don't handle the new-line here */
			break;
		}
			
		pos++;
	}
	
	/* write the header */
	text.name_length = pos - header_start;
	state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_HEADER_START|level);
	state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);

	state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_HEADER_END|level);
	state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_PARAGRAPH_BREAK);

	header_end = pos;

	if (state->buffer[pos] == '#')
	{
		header_end = pos-1;

		while(state->buffer[header_end] == 0x20 || state->buffer[header_end] == 0x09)
		{
			header_end--;
		}

		/* get rid of the end of the headers */
		while(state->buffer[pos] == '#')
		{
			pos++;
		}
	}

	state->buffer_pos = pos;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_parse_numbered_header
 *  desc: The headers are a sequence of numbers and dots that does not end in a
 *        dot. So the following are valid headers "1","1.1.1" or "2.3.5.6". As
 *        with the list these should be the first non-whitespace character on the
 *        line. The numbers on the header is not used, but the number of numbers
 *        sets the level of the header.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_numbered_header(MARKDOWN_STATE* state)
{
	NAME			numbers = {NULL,0,0,0};
	unsigned int	pos = state->buffer_pos;
	unsigned int	bpos = pos - 1;
	unsigned int	level = 0;
	unsigned int	result = EC_OK;
	unsigned char	buffer[MAX_NUMBER_LENGTH];
	
	/* Ok, get to the end of the first number */
	while (state->buffer[pos] >= '0' && state->buffer[pos] <= '9')
	{
		pos++;
	}

	if (state->buffer[pos] == ' ' || (state->buffer[pos] == '.' && (state->buffer[pos+1] >= '0' && state->buffer[pos+1] <= '9')))
	{
		level = 1;

		/* ok, just search now for the next white space - counting all the dots */
		while (pos < state->buffer_size)
		{
			if (state->buffer[pos] == '.')
			{
				level++;
			}
			else if (state->buffer[pos] == 0x20 || state->buffer[pos] == 0x09)
			{
				/* end of header */
				if (state->buffer[pos-1] == '.')
				{
					/* should not terminate with a dot - ignore the dot */
					level--;
					result = EC_AMBIGUOUS_MARKUP_IGNORED;
					raise_warning(state->line_number,result,(unsigned char*)state->input_state->input_name,NULL);
				}
				break;
			}
			else if (state->buffer[pos] < '0' && state->buffer[pos] > '9')
			{
				/* invalid char - exit and ignore all */
				pos = state->buffer_pos;

				result = EC_AMBIGUOUS_MARKUP;
				raise_warning(state->line_number,result,(unsigned char*)state->input_state->input_name,NULL);
				break;
			}

			pos++;
		}
			
		/* Ok, let's output the header */
		state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_HEADER_START|level);

		/* output the level numbers */
		numbers.name = buffer;
		numbers.name_length = level_index_gen_label(&state->draw_state->index,1,INDEX_STYLE_NUMBERS,buffer,MAX_NUMBER_LENGTH,'.');

		state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_T_SPACE,&numbers);
		state->draw_state->no_space = 1;

		state->buffer_pos = markdown_consume_line(state,pos);

		state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_HEADER_END|level);
		state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_PARAGRAPH_BREAK);
	}
	else
	{
		/* not a numbered header */
		result = EC_FAILED;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_handle_quote_block
 *  desc: This code will handle the quote block parsing.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_handle_quote_block(MARKDOWN_STATE* state, unsigned int start_pos, unsigned int nested)
{
	NAME			text;
	unsigned int pos = start_pos;
	unsigned int level = 0;
	unsigned int not_used;
	unsigned int old_level = 0;
	unsigned int leading = OUTPUT_TEXT_STYLE_NORMAL;

	while (pos < state->buffer_size)
	{
		/* decode the line */
		level = 0;

		while (state->buffer[pos] == '>' && state->buffer[pos+1] == ' ')
		{
			level++;
			pos += 2;
		}

		if (level == 0)
		{
			raise_warning(state->line_number,EC_AMBIGUOUS_MARKUP,state->input_state->input_name,0);
			break;
		}
		else
		{
			if (old_level != level)
			{
				if (old_level < level)
				{
					/* going up a level */
					text.name = (unsigned char*) "''";
					text.name_length = 2;
					state->draw_state->format->output_text(state->draw_state,leading,&text);
					leading |= OUTPUT_TEXT_STYLE_L_NEWLINE;
					state->draw_state->margin_width += 2;
				}
				else
				{
					/* going down a level */
					state->draw_state->margin_width -= 2;
					text.name = (unsigned char*) "''";
					text.name_length = 2;
					state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL|OUTPUT_TEXT_STYLE_T_NEWLINE,&text);
				}
				old_level = level;
			}

			pos = markdown_consume_line(state,pos);

			if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
			{
				/* ok, we have an end of line */
				pos = handle_newline(state,pos);
				
				if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
				{
					/* double return - end of block */
					break;
				}
			}
		}

		if (nested)
		{
			pos = markdown_whitespace_level(state,pos,&not_used);
	
			if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
			{
				/* double return - end of block */
				break;
			}
		}
	}

	/* need to end the quote block */
	state->draw_state->margin_width -= 2;
	text.name = (unsigned char*) "''";
	text.name_length = 2;
	state->draw_state->format->output_text(state->draw_state,OUTPUT_TEXT_STYLE_NORMAL,&text);
	

	return pos;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_parse_inner_block
 *  desc: This function will parse the inner block.
 *        
 *        It is looking for blocks that can only exist within other blocks.
 *        It is looking for paragraphs, quote blocks and fenced code blocks. All
 *        the others are ambiguous so cannot be used.
 *
 *        All items are bound to the last 'start' item. If it is not one of the
 *        expected items then it will end the search. The levels of the inner
 *        blocks are ignored. There are one "spanned" list items as these are
 *        just stupid. :)
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_inner_block(MARKDOWN_STATE* state, unsigned int start_pos)
{
	unsigned int pos = start_pos;
	unsigned int level;
	unsigned int search;
	unsigned int had_continuation = 0;

	while (pos < state->buffer_size)
	{
		if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
		{
			/* ok, we have an end of line */
			pos = handle_newline(state,pos);

			if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
			{
				search = pos+1;
				
				/* ok, now double line break - don't consume the newline yet */
				if (state->buffer[search] == 0x0a || state->buffer[search] == 0x0d)
				{
					/* have a two char line break */
					search++;
				}
	
				/* Ok, possible that we have a continuation */
				search = markdown_whitespace_level(state,search,&level);
				
				if (state->buffer[search] == '-' || state->buffer[search] == '+' || state->buffer[search] == '*' ||
					(state->buffer[search] >= '0' && state->buffer[search] <= '9'))
				{
					/* ok, it's a new list element - exit but consume the newline */
					pos = handle_newline(state,pos);
					break;
				}
				else if (level > 0)
				{
					/* Ok, we have a continuation */
					had_continuation = 1;
					state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_PARAGRAPH_BREAK);
					handle_newline(state,pos);

					if (state->buffer[search] == '>' && state->buffer[search+1] == ' ')
					{
						pos = markdown_handle_quote_block(state,search,1);
					}
					else
					{
						pos = markdown_consume_line(state,search);
					}
				}
				else
				{
					/* not continuation - exit but don't consume the newline */
					break;
				}
			}
			else
			{
				search = pos;

				while (state->buffer[search] == 0x20 || state->buffer[search] == 0x09)
				{
					search++;
				}

				if (state->buffer[search] == '-' || state->buffer[search] == '+' || state->buffer[search] == '*' ||
					(state->buffer[search] >= '0' && state->buffer[search] <= '9'))
				{
					/* ok, it's a new list element - exit */
					break;
				}
				else
				{
					pos = search;
				}
			}
		}
		else
		{
			/* consume the line of text */
			pos = markdown_consume_line(state,pos);
		}
	}

	if (had_continuation)
	{
		state->draw_state->format->output_marker(state->draw_state,OUTPUT_MARKER_LINE_BREAK);
	}

	state->buffer_pos = pos;

	return pos;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : markdown_parse_quote_block
 * Desc : This function will handle decoding the quote blocks.
 *--------------------------------------------------------------------------------*/
static	unsigned int	markdown_parse_quote_block(MARKDOWN_STATE* state)
{
	unsigned int result = EC_OK;
	
	state->input_state->state = TYPE_QUOTE_BLOCK;
	state->draw_state->format->output_header(state->draw_state,state->input_state);

	state->buffer_pos = markdown_handle_quote_block(state,state->buffer_pos,0);
	
	state->draw_state->format->output_footer(state->draw_state,state->input_state);

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 *  name: markdown_parse_list_block
 *  desc: This function will parse the input for the numbers.
 *
 *        list values are the first non-whitespace character on the line. The list
 *        items will also end in a '.'. So the "1." will be seen as a list item.
 *        As with all markups the actual values in the list items are ignored.
 *
 *        As lists are blocks, this code will keep searching until the whole list
 *        is consumed.
 *--------------------------------------------------------------------------------*/
static unsigned int	markdown_parse_list_block(MARKDOWN_STATE* state)
{
	unsigned int pos = state->buffer_pos;
	unsigned int end = 0;
	unsigned int bpos = pos - 1;
	unsigned int start = 0;
	unsigned int level = 0;
	unsigned int old_level = 0;
	unsigned int result = EC_OK;
	unsigned int marker;

	state->input_state->state = TYPE_LIST;
	state->draw_state->format->output_header(state->draw_state,state->input_state);

	do
	{
		if (state->buffer[pos] == 0x0a || state->buffer[pos] == 0x0d)
		{
			/* have an double end of line. end of search */
			break;
		}
		else if (state->buffer[pos] == '-' || state->buffer[pos] == '+' || state->buffer[pos] == '*')
		{
			if (state->buffer[pos+1] == 0x20 || state->buffer[pos+1] == 0x09)
			{
				state->draw_state->format->output_list_item_start(state->draw_state,level,state->buffer[pos]);
				pos += 2;
				pos = markdown_parse_inner_block(state,pos);
				state->draw_state->format->output_list_end(state->draw_state);
			}
			else
			{
				/* this is not a list item -- leave */
				break;
			}
		}
		else if (state->buffer[pos] >= '0' && state->buffer[pos] <= '9')
		{
			/* we have a start of list line - remove multiple numbers */
			while (state->buffer[pos] >= '0' && state->buffer[pos] <= '9')
			{
				pos++;
			}

			if (state->buffer[pos] == '.' && state->buffer[pos+1] == ' ')
			{
				/* Ok, valid list entry start */
				state->draw_state->format->output_list_numeric_start(state->draw_state,level);
				pos += 2;
				pos = markdown_parse_inner_block(state,pos);
				state->draw_state->format->output_list_end(state->draw_state);
			}
			else
			{
				/* this is not a list item -- leave */
				break;
			}
		}

		pos = markdown_whitespace_level(state,pos,&level);
	}
	while(pos < state->buffer_size);

	state->buffer_pos = pos;

	state->input_state->state = TYPE_LIST;
	state->draw_state->format->output_footer(state->draw_state,state->input_state);

	return result;
}


