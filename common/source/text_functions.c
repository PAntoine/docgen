/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : text_functions
 * Desc  : These are some useful text handling functions that can be shared.
 *
 * Author: peterantoine
 * Date  : 18/07/2012 16:20:19
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <memory.h>
#include "utilities.h"
#include "output_format.h"
#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * local defines.
 *--------------------------------------------------------------------------------*/
#define	COMPLEX_LOOKING			(0)
#define	COMPLEX_OTHER			(1)
#define	COMPLEX_ARRAY			(2)

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: word_wrap
 * @desc: This function will word wrap the given buffer. it will do some basic
 *        wrapping depending on the characters found. It does a greedy wrapping
 *        as it is only for test files.
 *--------------------------------------------------------------------------------*/
unsigned int	word_wrap(unsigned char* buffer, unsigned int buffer_length, unsigned int* wrapped)
{
	unsigned int result = buffer_length;

	*wrapped = 1;

	while (result > 0)
	{
		if (buffer[result] == 0x20 || buffer[result] == 0x09)
			break;

		result--;
	}

	/* if cant find a break, then hard-clip the line */
	if (result == 0)
	{
		*wrapped = 0;
		result = buffer_length;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : text_punctuation
 * Desc : This function returns 1 when the character is a text punctuation char.
 *        This only works for ASCII english now, but will have to be extended to
 *        handle UNICODE punctuation.
 *--------------------------------------------------------------------------------*/
unsigned int	text_punctuation(unsigned char* buffer)
{
	unsigned int result = 0;

	if (buffer[0] == '.' || buffer[0] == '!' || buffer[0] == '?' || buffer[0] == '\'' || buffer[0] == '`' || buffer[0] == '_')
	{
		result = 1;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: write_complex
 * @desc: This function will handle the complex wrapping of the text. It will
 *        try and work out what the text type is and then wrap it accordingly.
 *--------------------------------------------------------------------------------*/
static unsigned int	write_complex (	DRAW_STATE* draw_state, NAME* text_block )
{
	unsigned int	size = 0;
	unsigned int	state = COMPLEX_LOOKING;
	unsigned int	level = 0;
	unsigned int	group = 0;
	unsigned int	result = 0;
	unsigned int	instring = 0;
	unsigned int	last_comma = 0;
	unsigned int	out_pos  = draw_state->margin_width;
	unsigned int	position = 0;
	
	/*clear the margin */
	memset(draw_state->output_buffer,' ',draw_state->margin_width);

	/* white space remove */
	while (text_block->name[position] == 0x20 || text_block->name[position] < 0x0f)
	{
		position++;
	}

	while (position < text_block->name_length)
	{
		switch(state)
		{
			case COMPLEX_LOOKING:
				if (text_block->name[position] == '{')
				{
					do
					{
						draw_state->output_buffer[out_pos++] = text_block->name[position++];

						/* white space remove */
						while (text_block->name[position] == 0x20 || text_block->name[position] < 0x0f)
						{
							position++;
						}

						level++;
					}
					while (text_block->name[position] == '{');

					group = level - 1;
					state = COMPLEX_ARRAY;
				}
				else
				{
					state = COMPLEX_OTHER;
				}
				break;

			case COMPLEX_OTHER:
				/* need to clip the data into the line width */
				position = text_block->name_length;
				break;

			case COMPLEX_ARRAY:

				while (position < text_block->name_length)
				{
					if (text_block->name[position] == 0)
					{
						position++;
					}
					else if (instring)
					{
						/* skip the escaped end " */
						if ((text_block->name[position] == '\\') && (text_block->name[position+1] == '"'))
						{
							draw_state->output_buffer[out_pos++] = text_block->name[position++];
						}
						else if (text_block->name[position] == '"')
						{
							instring = 0;
						}

						draw_state->output_buffer[out_pos++] = text_block->name[position++];
					}
					else
					{
						switch(text_block->name[position])
						{
							case '{':
								/* white space remove */
								while (text_block->name[position] == 0x20 || text_block->name[position] < 0x0f)
								{
									position++;
								}

								level++;
								size = 0;
								draw_state->output_buffer[out_pos++] = text_block->name[position++];
								break;

							case '"':
								instring = 1;
								draw_state->output_buffer[out_pos++] = text_block->name[position++];
								break;

							case ',':
								/* need to reverse remove whitespace from the output */
								if (out_pos > 0)
								{
									if ((draw_state->output_buffer[out_pos-1] == 0x20 || draw_state->output_buffer[out_pos-1] < 0x0f))
									{
										while (	out_pos > 0 && 
												(draw_state->output_buffer[out_pos-1] == 0x20 || draw_state->output_buffer[out_pos-1] < 0x0f))
										{
											out_pos--;
										}
									}
								}
								draw_state->output_buffer[out_pos++] = text_block->name[position++];
								last_comma = out_pos;

								if (level == group)
								{
									/* write it to the output file */
									draw_state->output_buffer[out_pos++] = '\n';
									write(draw_state->output_file,draw_state->output_buffer,out_pos);
									memset(draw_state->output_buffer,' ',draw_state->margin_width+level);
									out_pos = draw_state->margin_width+level;
									last_comma = 0;
								}

								/* now check for post comma whitespace */
								while (	position < text_block->name_length && 
										(text_block->name[position] == 0x20 || text_block->name[position] < 0x0f))
								{
									position++;
								}

								break;

							case '}':
								level--;
								if (level < group)
								{
									draw_state->output_buffer[out_pos++] = '\n';
									write(draw_state->output_file,draw_state->output_buffer,out_pos);
									memset(draw_state->output_buffer,' ',draw_state->margin_width+level);
									out_pos = draw_state->margin_width+level;
									last_comma = 0;
								}

								draw_state->output_buffer[out_pos++] = text_block->name[position++];
								while (	position < text_block->name_length && 
										(text_block->name[position] == 0x20 || text_block->name[position] < 0x0f))
								{
									position++;
								}
								break;

							default:
								size++;
								draw_state->output_buffer[out_pos++] = text_block->name[position++];
						}
					}

					if (out_pos == draw_state->page_width)
					{
						if (last_comma > 0)
						{
							unsigned char temp = draw_state->output_buffer[last_comma];
							draw_state->output_buffer[last_comma] = '\n';
							write(draw_state->output_file,draw_state->output_buffer,last_comma+1);
							
							draw_state->output_buffer[last_comma] = temp;
							memcpy(&draw_state->output_buffer[draw_state->margin_width + level],&draw_state->output_buffer[last_comma],(out_pos - last_comma));
							out_pos = draw_state->margin_width + level + (out_pos - last_comma);
						}
						else
						{
							draw_state->output_buffer[out_pos] = '\n';
							write(draw_state->output_file,draw_state->output_buffer,out_pos+1);

							out_pos = draw_state->margin_width+level;
						}
						memset(draw_state->output_buffer,' ',draw_state->margin_width+level);
						last_comma = 0;
					}
				}

				draw_state->output_buffer[out_pos] = '\n';
				write(draw_state->output_file,draw_state->output_buffer,out_pos+1);
				break;
		}
	}

	return result;
} 

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: write_line_clipped
 * @desc: This function will write a block of text, clipping it at line width,
 *        and then discarding anything that is on the same line until the line
 *        ending chars are found.
 *--------------------------------------------------------------------------------*/
static void	write_line_clipped(	DRAW_STATE* draw_state, NAME* text_block )
{
	unsigned int start = 0;
	unsigned int length = 0;
	unsigned int position = 0;

	while (position < text_block->name_length)
	{
		if (text_block->name[position] == 0x0a || text_block->name[position] == 0x0d)
		{
			if (length > 0)
			{
				write(draw_state->output_file,&text_block->name[start],length);
				write(draw_state->output_file,"\n",1);
			}

			length = 0;
			start = position + 1;
		}
		else if (length < draw_state->page_width)
		{
			length++;
		}

		position++;
	}

	if (length > 0)
	{
		write(draw_state->output_file,&text_block->name[start],length);
		write(draw_state->output_file,"\n",1);
	}

}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_text
 * Desc : This function will simply write the given text to the file. It will
 *        remember what it's place on the line is and handle the line wrapping.
 *--------------------------------------------------------------------------------*/
void	write_text(DRAW_STATE* draw_state, NAME* text_block)
{
	unsigned int is_wrapped;
	unsigned int text_length = text_block->name_length;
	unsigned int text_left = 0;
	unsigned int text_used = 0;
	unsigned int line_left = draw_state->page_width - draw_state->offset;
	unsigned int full_page = draw_state->page_width - draw_state->margin_width;
	
	memset(draw_state->output_buffer,' ',draw_state->margin_width);

	/* remove whitespace from the start */
	while (text_used < text_length && (text_block->name[text_used] == 0x20 || text_block->name[text_used] == 0x09))
	{
		text_used++;
	}

	/* remove whitespace from the end */
	if (text_length > 0 && (text_block->name[text_length-1] == 0x20 || text_block->name[text_length-1] == 0x09))
	{
		while (text_length > 0 && (text_block->name[text_length-1] == 0x20 || text_block->name[text_length-1] == 0x09))
		{
			text_length--;
		}
	}

	while (text_used < text_length)
	{
		text_left = text_length - text_used;

		if (draw_state->offset == 0 || line_left == 0)
		{
			/* Ok, write the margin */
			write(draw_state->output_file,draw_state->output_buffer,draw_state->margin_width);
			draw_state->offset = draw_state->margin_width;
			draw_state->no_space = 0;
		}
		else if (!text_punctuation(&text_block->name[text_used]) && !draw_state->no_space)
		{
			/* it's a continuation - so need to add a space */
			write(draw_state->output_file," ",1);
			draw_state->offset++;
		}

		line_left = (draw_state->page_width - draw_state->offset);

		if (text_left < line_left)
		{
			/* ok, partial so just write it */
			write(draw_state->output_file,&text_block->name[text_used],text_left);
			text_used = text_length;
			draw_state->offset += text_left;
		}
		else if (line_left > 0)
		{
			/* ok, look backwards for the first whitespace */
			line_left = word_wrap(&text_block->name[text_used],line_left,&is_wrapped);

			if (!is_wrapped && (draw_state->page_width - draw_state->offset) < full_page)
			{
				/* less than a word, end the line and go around again */
				write(draw_state->output_file,"\n",1);
				draw_state->offset = 0;
			}
			else
			{
				/* write the text - increment if it is wrapped */
				write(draw_state->output_file,&text_block->name[text_used],line_left);
				text_used += line_left + is_wrapped;
				draw_state->offset += line_left;
				write(draw_state->output_file,"\n",1);
			}

			line_left = 0;
		}
		else
		{
			/* at end of line */
			write(draw_state->output_file,"\n",1);
			draw_state->offset = 0;
		}
	}

	/* only useful for a simple call */
	draw_state->no_space = 0;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: write_block_text
 * @desc: This function will output a block of text.
 *        it will use the flags passed in to format the text.
 *--------------------------------------------------------------------------------*/
void	write_block_text(DRAW_STATE* draw_state, NAME* text_block, unsigned int flags)
{
	unsigned int start;
	unsigned int offset = 0;
	unsigned int clipped;
	unsigned int wrapped;
	unsigned int text_used = 0;
	unsigned int text_block_offset = draw_state->margin_width;
	unsigned int text_block_remaining = text_block->name_length;
	
	if ((flags & OUTPUT_COLUMN_FORMAT_WRAP_COMPLEX) != 0)
	{
		write_complex(draw_state,text_block);
	}
	else if ((flags & OUTPUT_COLUMN_FORMAT_LINE_CLIP) != 0)
	{
		write_line_clipped(draw_state,text_block);
	}
	else if (text_block->fixed)
	{
		memset(draw_state->output_buffer,' ',draw_state->margin_width);
		
		/* fixed block, now need to look for the 0x0a to know when to wrap */
		for (offset=0;offset < text_block->name_length;offset++)
		{
			if (text_block->name[offset] == 0x0a)
			{
				write(draw_state->output_file,draw_state->output_buffer,draw_state->margin_width);
				write(draw_state->output_file,&text_block->name[start],(offset - start) + 1);
				start = offset + 1;
			}
			else if ((offset - start) >= draw_state->page_width)
			{
				write(draw_state->output_file,draw_state->output_buffer,draw_state->margin_width);
				write(draw_state->output_file,&text_block->name[start],offset - start);
				start = offset + 1;
			}
		}

		if ((start - offset) > 0)
		{
			write(draw_state->output_file,&text_block->name[start],offset - start);
		}
	}
	else
	{
		draw_state->output_buffer[draw_state->page_width] = '\n';
		memset(draw_state->output_buffer,' ',draw_state->margin_width);

		do
		{
			clipped = 0;
			wrapped = 0;

			write(draw_state->output_file,draw_state->output_buffer,draw_state->margin_width);

			if ((text_block_remaining + draw_state->margin_width) > draw_state->page_width)
			{
				if ((flags & OUTPUT_COLUMN_FORMAT_WORD_WRAP) != 0)
				{
					text_used = word_wrap(&text_block->name[offset],draw_state->page_width,&wrapped);
				}
				else
				{
					/* all others are hard wrapped */
					text_used = draw_state->page_width;
				}

				/* write the output */
				write(draw_state->output_file,&text_block->name[offset],text_used);

				text_block_remaining	-= text_used + wrapped;
				offset					+= text_used + wrapped;

				clipped = 1;
			}
			else
			{
				write(draw_state->output_file,&text_block->name[offset],text_block_remaining);
				text_block_remaining = 0;
			}
			write(draw_state->output_file,"\n",1);
		}
		while (clipped);
	}
	
	/* add a line after the section */
	write(draw_state->output_file,"\n",1);
}
