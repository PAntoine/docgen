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
 * @ignore
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include "utilities.h"
#include "output_format.h"
#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * states for the markdown decoding.
 *--------------------------------------------------------------------------------*/
enum
{
	MARKDOWN_LOOKING,
	MARKDOWN_POS_CODE_START,
	MARKDOWN_CODE_START,
	MARKDOWN_POS_CODE_END,
	MARKDOWN_FOUND_SPACE,
	MARKDOWN_FOUND_SPACES,
	MARKDOWN_POS_EMP_START,
	MARKDOWN_IN_EMP,
	MARKDOWN_SPACE_IN_EMP,
	MARKDOWN_POS_END_EMP,
	MARKDOWN_HEADER_COUNT,
	MARKDOWN_HEADER,
	MARKDOWN_HEADER_END,
	MARKDOWN_CODE_SECTION,
	MARKDOWN_CODE_POS_END
};

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: output_section
 * @desc: This function will output a section of the input buffer as requested.
 *        It will dump from the end of the last section to the current position
 *        of the current input buffer, excluding the current character. It will
 *        set the section offset to after the current position.
 *--------------------------------------------------------------------------------*/
static void	output_section(INPUT_STATE* input_state, DRAW_STATE* draw_state)
{
	if (input_state->buffer_pos > input_state->section)
	{
		draw_state->format->output_raw(draw_state,&input_state->buffer[input_state->section],(input_state->buffer_pos - input_state->section));
	}

	input_state->section = input_state->buffer_pos+1;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: parse_markdown
 * @desc: This function will parse the given text for the markdown symbols. It
 *        is using a restricted version of markdown as the full html derived
 *        version is not required for the doc processor. 
 *
 *        TODO: handle DOS and MAC line endings.
 *--------------------------------------------------------------------------------*/
void	parse_markdown(INPUT_STATE* input_state, DRAW_STATE* draw_state)
{
	unsigned char		emp_char = '*';
	unsigned int		reset_line_start = 0;
	static unsigned int	state = MARKDOWN_LOOKING;

	/* reset the section start */
	input_state->section = input_state->buffer_pos;

	/* white space remove the white space on entry */
	while (	input_state->buffer_pos < input_state->bytes_read &&
			((input_state->buffer[input_state->buffer_pos] == 0x20 || input_state->buffer[input_state->buffer_pos] < 0x0f)))
	{
		if (input_state->buffer[input_state->buffer_pos] == 0x0a || input_state->buffer[input_state->buffer_pos] == 0x0d)
		{
			input_state->line_number++;
			reset_line_start = 1;
		}

		input_state->section++;
		input_state->buffer_pos++;
	}

	if (reset_line_start)
	{
		input_state->line_start = input_state->buffer_pos;
	}

	while(input_state->buffer_pos < input_state->bytes_read && input_state->internal_state == INPUT_STATE_INTERNAL_SEARCHING)
	{
		switch (state)
		{
			case MARKDOWN_LOOKING:
				switch(input_state->buffer[input_state->buffer_pos])
				{
					case ' ':	state = MARKDOWN_FOUND_SPACE; 		break;
					case 0x09:	state = MARKDOWN_FOUND_SPACE;		break;
					case '\'':	state = MARKDOWN_POS_CODE_START;	break;
					case 0x0a:	
								output_section(input_state,draw_state);
								draw_state->format->output_marker(draw_state,OUTPUT_MARKER_ASCII_CHAR|(unsigned int)0x00000020);
								input_state->line_start = input_state->buffer_pos + 1;
								input_state->line_number++;
								break;

					case '>':	output_section(input_state,draw_state);
								if ((input_state->buffer_pos - input_state->line_start) < 2)
								{
									draw_state->format->output_marker(draw_state,OUTPUT_MARKER_BLOCK_START);
									state = MARKDOWN_CODE_SECTION;
								}
								break;

					case '#':	output_section(input_state,draw_state);
								if ((input_state->buffer_pos - input_state->line_start) < 2)
								{
									input_state->header_level = 0;
									state = MARKDOWN_HEADER_COUNT;
								}
								break;

					case '[':
								output_section(input_state,draw_state);
								input_state->model_pos = 0;
								input_state->internal_state = INPUT_STATE_INTERNAL_SCHEME;
								break;
				}
				break;

			case MARKDOWN_POS_CODE_START:
				if (input_state->buffer[input_state->buffer_pos] == '\'')
				{
					draw_state->format->output_marker(draw_state,OUTPUT_MARKER_CODE_START);
					state = MARKDOWN_POS_CODE_START;
				}
				else
				{
					state = MARKDOWN_LOOKING;
				}
				break;

			case MARKDOWN_CODE_START:
				if (input_state->buffer[input_state->buffer_pos] == '\'')
				{
					state = MARKDOWN_POS_CODE_END;
				}
				break;

			case MARKDOWN_POS_CODE_END:
				if (input_state->buffer[input_state->buffer_pos] == '\'')
				{
					output_section(input_state,draw_state);
					draw_state->format->output_marker(draw_state,OUTPUT_MARKER_CODE_END);
					state = MARKDOWN_POS_CODE_END;
				}
				else
				{
					state = MARKDOWN_POS_CODE_START;
				}
				break;

			case MARKDOWN_FOUND_SPACE:
				if (input_state->buffer[input_state->buffer_pos] == 0x0a)
				{
					input_state->line_number++;
					output_section(input_state,draw_state);
					input_state->line_start = input_state->buffer_pos + 1;
					state = MARKDOWN_LOOKING;
				}
				else if (input_state->buffer[input_state->buffer_pos] == ' ')
				{
					output_section(input_state,draw_state);
					state = MARKDOWN_FOUND_SPACES;
				}
				else if ((input_state->buffer[input_state->buffer_pos] == '*') ||
						 (input_state->buffer[input_state->buffer_pos] == '_'))
				{
					emp_char = input_state->buffer[input_state->buffer_pos];
					input_state->emp_level = 1;
					output_section(input_state,draw_state);
					state = MARKDOWN_POS_EMP_START;
				}
				else if (input_state->buffer[input_state->buffer_pos] == '[')
				{
					output_section(input_state,draw_state);
					input_state->model_pos = 0;
					state = MARKDOWN_LOOKING;
					input_state->internal_state = INPUT_STATE_INTERNAL_SCHEME;
				}
				else
				{
					state = MARKDOWN_LOOKING;
				}
				break;

			case MARKDOWN_FOUND_SPACES:
				if (input_state->buffer[input_state->buffer_pos] == 0x0a)
				{
					input_state->line_number++;
					input_state->line_start = input_state->buffer_pos + 1;
					input_state->section = input_state->line_start;
					draw_state->format->output_marker(draw_state,OUTPUT_MARKER_LINE_BREAK);
					state = MARKDOWN_LOOKING;
				}
				else if (input_state->buffer[input_state->buffer_pos] != ' ')
				{
					input_state->section = input_state->buffer_pos;
					state = MARKDOWN_LOOKING;
				}
				break;

			case MARKDOWN_POS_EMP_START:
				if (input_state->buffer[input_state->buffer_pos] == emp_char)
				{
					input_state->emp_level++;
				}
				else if ((input_state->buffer[input_state->buffer_pos] == 0x09) ||
						 (input_state->buffer[input_state->buffer_pos] == 0x20))
				{
					draw_state->format->output_marker(draw_state,OUTPUT_MARKER_ASCII_CHAR|(unsigned int)emp_char);
					state = MARKDOWN_LOOKING;
				}
				else
				{
					draw_state->format->output_marker(draw_state,(OUTPUT_MARKER_EMP_START|(0x000f & input_state->emp_level)));
					state = MARKDOWN_IN_EMP;
				}
				break;

			case MARKDOWN_IN_EMP:
				if ((input_state->buffer[input_state->buffer_pos] == 0x09) &&
					(input_state->buffer[input_state->buffer_pos] == 0x20))
				{
					state = MARKDOWN_SPACE_IN_EMP;
				}
				else if (input_state->buffer[input_state->buffer_pos] == emp_char)
				{
					output_section(input_state,draw_state);
					state = MARKDOWN_POS_END_EMP;
				}
				else if (input_state->buffer[input_state->buffer_pos] == 0x0a)
				{
					/* Ok. bad format can't have a line ending in emp - just end it here */
					draw_state->format->output_marker(draw_state,OUTPUT_MARKER_EMP_END);
					state = MARKDOWN_LOOKING;
				}
				break;

				case MARKDOWN_SPACE_IN_EMP:
				if ((input_state->buffer[input_state->buffer_pos] != 0x09) &&
					(input_state->buffer[input_state->buffer_pos] != 0x20))
				{
					state = MARKDOWN_IN_EMP;
				}
				break;

			case MARKDOWN_POS_END_EMP:
				if ((input_state->buffer[input_state->buffer_pos] == 0x09) ||
					(input_state->buffer[input_state->buffer_pos] == 0x20))
				{
					draw_state->format->output_marker(draw_state,OUTPUT_MARKER_EMP_END);
					state = MARKDOWN_LOOKING;
				}
				else
				{
					draw_state->format->output_marker(draw_state,OUTPUT_MARKER_ASCII_CHAR|(unsigned int)emp_char);
					state = MARKDOWN_IN_EMP;
				}
				break;

			case MARKDOWN_HEADER_COUNT:
				if (input_state->buffer[input_state->buffer_pos] == '#')
				{
					input_state->header_level++;
					input_state->section = input_state->buffer_pos+1;
				}
				else
				{
					draw_state->format->output_marker(draw_state,(OUTPUT_MARKER_HEADER_START | (MAX_HEADER_LEVEL_MASK & input_state->header_level)));
					state = MARKDOWN_HEADER;
				}
				break;

			case MARKDOWN_HEADER:
				if (input_state->buffer[input_state->buffer_pos] == '#')
				{
					output_section(input_state,draw_state);
					draw_state->format->output_marker(draw_state,(OUTPUT_MARKER_HEADER_END | (MAX_HEADER_LEVEL_MASK & input_state->header_level)));
					state = MARKDOWN_HEADER_END;
				}
				else if (input_state->buffer[input_state->buffer_pos] == 0x0a)
				{
					input_state->line_number++;
					input_state->line_start = input_state->buffer_pos + 1;
					output_section(input_state,draw_state);
					draw_state->format->output_marker(draw_state,(OUTPUT_MARKER_HEADER_END | (MAX_HEADER_LEVEL_MASK & input_state->header_level)));
					state = MARKDOWN_LOOKING;
				}
				break;

			case MARKDOWN_HEADER_END:
				if (input_state->buffer[input_state->buffer_pos] != '#')
				{
					state = MARKDOWN_LOOKING;
				}
				else
				{
					input_state->section = input_state->buffer_pos+1;
				}
				break;

			case MARKDOWN_CODE_SECTION:
				if (input_state->buffer[input_state->buffer_pos] == 0x0a)
				{
					input_state->line_number++;
					input_state->line_start = input_state->buffer_pos + 1;
					draw_state->format->output_marker(draw_state,OUTPUT_MARKER_LINE_BREAK);
					state = MARKDOWN_CODE_POS_END;
				}
				break;

			case MARKDOWN_CODE_POS_END:
				if (input_state->buffer[input_state->buffer_pos] == '>')
				{
					state = MARKDOWN_CODE_SECTION;
				}
				else
				{
					draw_state->format->output_marker(draw_state,OUTPUT_MARKER_BLOCK_END);
					state = MARKDOWN_LOOKING;
				}
				break;
		}
		
		input_state->buffer_pos++;
	}

	/* dump anything that is left */
	output_section(input_state,draw_state);
}

