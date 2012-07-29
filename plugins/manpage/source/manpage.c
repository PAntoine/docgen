/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: manpage
 *   @desc: This file holds the entry point to the manpage output plugin.
 *
 * @author: 
 *   @date: 06/07/2012 23:53:45
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <malloc.h>
#include <memory.h>
#include "manpage.h"
#include "plugin.h"
#include "utilities.h"
#include "error_codes.h"
#include "output_format.h"
#include "document_generator.h"


/*--------------------------------------------------------------------------------*
 * default page width - used only for laying out sequence diagrams.
 *--------------------------------------------------------------------------------*/
#define DEFAULT_PAGE_WIDTH	(120)

/*--------------------------------------------------------------------------------*
 * Plugin Globals
 *--------------------------------------------------------------------------------*/
static unsigned char	format_name[] = "manpage";

static	OUTPUT_FORMAT	format = 
{
	format_name,
	sizeof(format_name)-1,

	format_decode_flags,
	
	format_open,
	format_close,
	format_header,	
	format_footer,
	format_raw,
	format_sample,

	format_timelines,
	format_message,

	format_states,
	format_start_state,
	format_transition,
	format_end_state,

	format_marker,

	format_text,

	format_section,
	format_title,
	format_block,

	format_table_start,
	format_table_header,
	format_table_row,
	format_table_end,

	0L
};

/*--------------------------------------------------------------------------------*
 * Plugin static variables
 *--------------------------------------------------------------------------------*/
static	PLUGIN_LIBRARY plugin_library = 
{ 
	MAKE_VERSION(VERSION_MAJOR,MANPAGE_VERSION),
	DGOF_PLUGIN_OUTPUT,
	DGOF_plugin_release,
	0L,
	&format
};

/*--------------------------------------------------------------------------------*
 * manpage macros
 *--------------------------------------------------------------------------------*/
static unsigned char troff_title		[] = "\n.TH ";		/*title section [extra1] [extra2] [extra3] */
static unsigned char troff_section		[] = "\n.SH ";		/*[text for a heading] */
static unsigned char troff_subsection	[] = "\n.SS ";		/*[text for a heading] */
static unsigned char troff_title_para	[] = "\n.TP ";		/*[nnn] */
static unsigned char troff_interspace	[] = "\n.PD";
static unsigned char troff_no_interspace[] = "\n.PD 0";
static unsigned char troff_header_1     [] = "\n.PD 0 \n";
static unsigned char troff_header_2     [] = "\n.PD 4 \n";
static unsigned char troff_header_3     [] = "\n.PD 8 \n";
static unsigned char troff_header_4     [] = "\n.PD 12\n";
static unsigned char troff_new_para		[] = "\n.LP";
static unsigned char troff_indent_para	[] = "\n.IP ";		/*[designator]	[nnn] */
static unsigned char troff_hang_param	[] = "\n.HP ";		/*[nnn] */
static unsigned char troff_margin_right	[] = "\n.RS\n";		/*[nnn] */
static unsigned char troff_margin_left	[] = "\n.RE\n";		/*[nnn] */
static unsigned char troff_small_font	[] = "\n.SM ";		/*[text] */
static unsigned char troff_small_bold	[] = "\n.SB ";		/*[text] */
static unsigned char troff_bold			[] = "\n.B ";		/*[text] */
static unsigned char troff_italic		[] = "\n.I ";		/*[text] */
static unsigned char troff_ignore		[] = "\n\\&";		/* ignore and use test verbatim */
static unsigned char troff_comment		[] = "\n.\\\"";		/* word */

static const unsigned int	troff_title_size 			= sizeof(troff_title)-1;
static const unsigned int	troff_section_size 			= sizeof(troff_section)-1;
static const unsigned int	troff_subsection_size 		= sizeof(troff_subsection)-1;
static const unsigned int	troff_header_level_size		= sizeof(troff_header_4)-1;
static const unsigned int	troff_interspace_size		= sizeof(troff_interspace)-1;
static const unsigned int	troff_no_interspace_size	= sizeof(troff_no_interspace)-1;
static const unsigned int	troff_title_para_size 		= sizeof(troff_title_para)-1;
static const unsigned int	troff_new_para_size 		= sizeof(troff_new_para)-1;
static const unsigned int	troff_indent_para_size 		= sizeof(troff_indent_para)-1;
static const unsigned int	troff_hang_param_size 		= sizeof(troff_hang_param)-1;
static const unsigned int	troff_margin_right_size 	= sizeof(troff_margin_right)-1;
static const unsigned int	troff_margin_left_size 		= sizeof(troff_margin_left)-1;
static const unsigned int	troff_small_font_size 		= sizeof(troff_small_font)-1;
static const unsigned int	troff_small_bold_size 		= sizeof(troff_small_bold)-1;
static const unsigned int	troff_bold_size 			= sizeof(troff_bold)-1;
static const unsigned int	troff_italic_size 			= sizeof(troff_italic)-1;
static const unsigned int	troff_ignore_size 			= sizeof(troff_ignore)-1;
static const unsigned int	troff_comment_size 			= sizeof(troff_comment)-1;

static unsigned char tbl_start	[] = "\n.TS\n";
static unsigned char tbl_end	[] = "\n.TE\n";

static unsigned int		tbl_start_size = sizeof(tbl_start) - 1;
static unsigned int		tbl_end_size = sizeof(tbl_end) - 1;

/*--------------------------------------------------------------------------------*
 * Required plugin functions
 *--------------------------------------------------------------------------------*/

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: DGOF_plugin_get_details
 * @desc: This function will return the version and the format type from the
 *        plugin.
 *--------------------------------------------------------------------------------*/
unsigned int	DGOF_plugin_get_details(unsigned int* version, unsigned int* format_size, PLUGIN_LIBRARY** formats)
{
	unsigned int result = 1;

	*version		= MAKE_VERSION(VERSION_MAJOR,MANPAGE_VERSION);
	*format_size	= sizeof(plugin_library);
	*formats		= &plugin_library;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: DGOF_plugin_release
 * @desc: 
 *--------------------------------------------------------------------------------*/
unsigned int	DGOF_plugin_release(void)
{
	unsigned int result = 1;

	return result;
}

/*--------------------------------------------------------------------------------*
 * Output functions
 *--------------------------------------------------------------------------------*/

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: write_escaped_manpage
 * @desc: This function will search for emp chars and handle those.
 *
 *  TODO: It should also escape any manpage commands it finds in the text.
 *--------------------------------------------------------------------------------*/
static void	write_escaped_manpage(int file_no, unsigned char* buffer, unsigned int buffer_length)
{
	unsigned int emp = 0;
	unsigned int end_emp = 0;
	unsigned int space = 0;
	unsigned int count;
	unsigned int last_write = 0;

	for (count=0; count < buffer_length; count++)
	{
		if (space && (buffer[count] == '*' || buffer[count] == '_'))
		{
			/* emp start */
			write(file_no,&buffer[last_write],count-last_write);
			write(file_no,troff_bold,troff_bold_size);
			last_write = count+1;

			emp = 1;
		}
		else if (emp && (buffer[count] == '*' || buffer[count] == '_'))
		{
			end_emp = 1;
		}
		else if (end_emp)
		{
			if (buffer[count] == 0x20)
			{
				/* end the emphasis */
				write(file_no,&buffer[last_write],count-last_write-1);
				write(file_no,"\n.\n",3);
				last_write = count+1;

				emp = 0;
			}
			else
			{
				end_emp = 0;
			}
		}

		space = (buffer[count] == 0x20);
	}

	write(file_no,&buffer[last_write],count-last_write);
}


/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_decode_flags
 * @desc: 
 *--------------------------------------------------------------------------------*/
unsigned int	format_decode_flags(INPUT_STATE* input_state, unsigned hash, NAME* value)
{
	unsigned int result;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_open
 * @desc: This function will open the manpage and add the file header to the 
 *        page.
 *--------------------------------------------------------------------------------*/
unsigned int	format_open(DRAW_STATE* draw_state, unsigned char* name, unsigned int name_length)
{
	unsigned int result = EC_FAILED;
	unsigned int length = name_length + draw_state->path_length;

	if ((length + 4) < MAX_FILENAME)
	{
		memcpy(&draw_state->path[draw_state->path_length],name,name_length);
		draw_state->path[length++] = '.';
		draw_state->path[length++] = 'm';
		draw_state->path[length++] = 'a';
		draw_state->path[length++] = 'n';
		draw_state->path[length] = '\0';
	
		draw_state->page_width = DEFAULT_PAGE_WIDTH;
		draw_state->global_margin_width = 1;
		draw_state->global_format_flags = 0;

		draw_state->output_buffer = malloc(draw_state->page_width + 1);
		draw_state->output_buffer[draw_state->page_width] = '\n';

		if ((draw_state->output_file = open((char*)draw_state->path,O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) != -1)
		{
			/* file successfully opened */
			write(draw_state->output_file,troff_title,troff_title_size);
			write(draw_state->output_file,name,name_length);
			write(draw_state->output_file," 1",2);
			write(draw_state->output_file,troff_new_para,troff_new_para_size);
			write(draw_state->output_file,"\n",1);

			result = EC_OK;
		}
	}
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_close
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_close(DRAW_STATE* draw_state)
{
	if (draw_state->output_file != -1)
	{
		close(draw_state->output_file);
		free(draw_state->output_buffer);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_header
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_header(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	if ((draw_state->format_flags & OUTPUT_FORMAT_INLINE) == 0)
	{
		write(draw_state->output_file,troff_new_para,troff_new_para_size);
		write(draw_state->output_file,"\n",1);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_footer
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_footer(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	if ((draw_state->format_flags & OUTPUT_FORMAT_INLINE) == 0)
	{
		write(draw_state->output_file,troff_interspace,troff_interspace_size);
		write(draw_state->output_file,troff_new_para,troff_new_para_size);
		write(draw_state->output_file,"\n",1);

		draw_state->offset = 0;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_raw
 * @desc: This function will output raw text. It will find the \n and replace 
 *        them with '\n ' as this is what the manpage will use a line start.
 *--------------------------------------------------------------------------------*/
void	format_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size)
{
	unsigned int offset = 0;
	unsigned int start_point = 0;

	while (offset < buffer_size)
	{
		if (buffer[offset] == 0x0a || buffer[offset] == 0x0d)
		{
			if (offset - start_point > 1)
			{
				write(draw_state->output_file,&buffer[start_point],offset-start_point);
			}
			start_point = offset + 1;
		}
		offset++;
	}

	if (start_point < offset)
	{
		write(draw_state->output_file,&buffer[start_point],offset-start_point);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_sample
 * @desc: This function will output a given sample.
 *        It will just add a header and the data. It does not do any clever 
 *        formatting. It just makes all the lines start with a space so that
 *        the troff processor leaves them alone.
 *--------------------------------------------------------------------------------*/
void	format_sample(DRAW_STATE* draw_state, SAMPLE* sample)
{
	unsigned int	offset = 0;
	unsigned int	start_point = 0;
	unsigned int	buffer_size;
	unsigned char*	buffer;

	format_title(draw_state,OHL_LEVEL_2,&sample->name,NULL);

	buffer = sample->sample.name;
	buffer_size = sample->sample.name_length;

	while (offset < buffer_size)
	{
		if (buffer[offset] == 0x0a || buffer[offset] == 0x0d)
		{
			if (offset - start_point > 1)
			{
				write(draw_state->output_file,"\n ",2);
				write(draw_state->output_file,&buffer[start_point],offset-start_point);
			}
			start_point = offset + 1;
		}
		offset++;
	}

	if (start_point < offset)
	{
		write(draw_state->output_file,"\n ",2);
		write(draw_state->output_file,&buffer[start_point],offset-start_point);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_timelines
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_timelines(DRAW_STATE* draw_state)
{
	unsigned int	offset = 0;
	unsigned int	write_size = 0;
	TIMELINE* 		current = draw_state->data.sequence.start_timeline;
	
	/* unformatted man page line */
	write(draw_state->output_file,troff_no_interspace,troff_no_interspace_size);
	write(draw_state->output_file,troff_new_para,troff_new_para_size);
	write(draw_state->output_file,"\n",1);
		
	offset = draw_state->data.sequence.column[current->column] - draw_state->data.sequence.window_start;
	write_size = write_size;

	memset(draw_state->output_buffer,' ',offset);

	while (current != draw_state->data.sequence.end_timeline)
	{
		if ((current->flags & FLAG_TAGGED) == FLAG_TAGGED)
		{
			offset = draw_state->data.sequence.column[current->column] - draw_state->data.sequence.window_start;
			draw_state->output_buffer[offset++] = '[';
			memcpy(&draw_state->output_buffer[offset],current->name,current->name_length);
			offset += current->name_length;
			draw_state->output_buffer[offset++] = ']';

			write_size = offset;

			memset(&draw_state->output_buffer[offset],' ',current->group->max_message_length+2);
			offset += current->group->max_message_length;
		}

		current = current->next;
	}
			
	write(draw_state->output_file,draw_state->output_buffer,write_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_message
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_message(DRAW_STATE* draw_state, MESSAGE* message)
{
	unsigned int	end;
	unsigned int	pos;
	unsigned int	count;
	unsigned int	start;
	unsigned int	offset = draw_state->margin_width;
	unsigned char	special = '<';
	unsigned char*	buffer;
	
	write(draw_state->output_file,troff_new_para,troff_new_para_size);
	write(draw_state->output_file,"\n",1);

	buffer = &draw_state->output_buffer[draw_state->margin_width];

	if (message->receiver == NULL)
	{
		special = '?';
	}

	/* clear the line */
	memset(draw_state->output_buffer,' ',draw_state->page_width+1);

	if (message->sending_timeline != NULL)
	{
		/* put the start within the window, we know it must be inside the window */
		start = draw_state->data.sequence.column[message->sending_timeline->column];

		if (start < draw_state->data.sequence.window_start)
		{
			start = 0;
		}
		else if (start > draw_state->data.sequence.window_end)
		{
			start = draw_state->page_width;
		}
		else
		{
			start -= draw_state->data.sequence.window_start;
		}

		end = start;

		if (message->receiver != NULL)
		{
			/* fit end to the window */
			end = draw_state->data.sequence.column[message->receiver->timeline->column];

			if (end < draw_state->data.sequence.window_start)
			{
				end = 0;
			}
			else if (end > draw_state->data.sequence.window_end)
			{
				end = draw_state->page_width;
			}
			else
			{
				end -= draw_state->data.sequence.window_start;
			}
		}

		/* if it is out of the window at either side skip */
		if (!(end == start && (end == 0 || end >= draw_state->data.sequence.window_end)))
		{
			if (end < start)
			{
				memset(&buffer[end+1],'-',start-end-1);
				buffer[end+1] = '<';
				offset = end + (start - end - message->name_length)/2;
			}
			else if (end > start)
			{
				memset(&buffer[start+1],'-',end-start-1);
				buffer[end-1] = '>';
				offset = start + (end - start - message->name_length)/2;
			}
			else
			{
				buffer[end]   = special;
				offset = end - (message->name_length);
			}

			for (count=draw_state->data.sequence.first_column;count<draw_state->data.sequence.last_column;count++)
			{
				pos = draw_state->data.sequence.column[count] - draw_state->data.sequence.window_start;

				if (buffer[pos] == '-')
					buffer[pos] = '+';
				else if (buffer[pos] == ' ')
					buffer[pos] = '|';
			}

			/* set the start of the window */
			if (draw_state->data.sequence.first_column != 0)
			{
				if (buffer[0] == '-')
					buffer[0] = '+';
				else if (buffer[0] == ' ')
					buffer[0] = '|';
			}

			if (draw_state->data.sequence.last_column != draw_state->data.sequence.num_columns)
			{
				if (buffer[draw_state->data.sequence.window_end] == '-')
					buffer[draw_state->data.sequence.window_end] = '+';
				else if (buffer[draw_state->data.sequence.window_end] == ' ')
					buffer[draw_state->data.sequence.window_end] = '|';
			}

			memcpy(&buffer[offset],message->name,message->name_length);
			offset += message->name_length;
			pos = draw_state->data.sequence.column[count] - draw_state->data.sequence.window_start;

			/* now write the line to the file */
			write(draw_state->output_file,draw_state->output_buffer,draw_state->page_width+1);
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_states
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_states(DRAW_STATE* draw_state,STATE* list)
{

}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_end_state
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_end_state(DRAW_STATE* draw_state,STATE* state)
{
	if (state->name_length > 0)
	{
		write(draw_state->output_file,"    ",4);
		write(draw_state->output_file,state->name,state->name_length);
		write(draw_state->output_file,":\n",2);
	}
	else
	{
		write(draw_state->output_file,"    <unknown>:\n",sizeof("    <unknown>:\n")-1);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_transition
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_transition(DRAW_STATE* draw_state,STATE* state, STATE_TRANSITION* transition)
{
	TRIGGERS*			current_triggers;

	/* TODO: this needs to be clipped and margined */

	write(draw_state->output_file,"        -> ",sizeof("        -> ")-1);
	write(draw_state->output_file,transition->next_state->name,transition->next_state->name_length);

	if (transition->trigger != NULL)
	{
		write(draw_state->output_file," when ",sizeof(" when ")-1);

		if (state->group != transition->trigger->group)
		{
			write(draw_state->output_file,transition->trigger->group->name,transition->trigger->group->name_length);
			write(draw_state->output_file,":",1);
		}

		write(draw_state->output_file,transition->trigger->name,transition->trigger->name_length);
	}
	else if (transition->condition != NULL)
	{
		write(draw_state->output_file," when '",sizeof(" when '")-1);
		write(draw_state->output_file,transition->condition,transition->condition_length);
	}

	if (transition->triggers != NULL)
	{
		write(draw_state->output_file," triggering ",sizeof(" triggering ")-1);

		current_triggers = transition->triggers;

		do
		{
			if (state->group != current_triggers->trigger->group)
			{
				write(draw_state->output_file,current_triggers->trigger->group->name,current_triggers->trigger->group->name_length);
				write(draw_state->output_file,":",1);
			}

			write(draw_state->output_file,current_triggers->trigger->name,current_triggers->trigger->name_length);

			if (current_triggers->next != NULL)
			{
				write(draw_state->output_file,",",1);
			}
			current_triggers = current_triggers->next;
		} 
		while (current_triggers != NULL);
	}

	write(draw_state->output_file,"\n",1);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_start_state
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_start_state(DRAW_STATE* draw_state,STATE* state)
{
	if (state->name_length > 0)
	{
		write(draw_state->output_file,"    ",4);
		write(draw_state->output_file,state->name,state->name_length);
		write(draw_state->output_file,":\n",2);
	}
	else
	{
		write(draw_state->output_file,"    <unknown>:\n",sizeof("    <unknown>:\n")-1);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_marker
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_marker(DRAW_STATE* draw_state, unsigned int marker)
{
	unsigned char array[1];
	unsigned char level = marker & 0x0000ffff;

	switch ((marker & 0xffff0000))
	{
		case OUTPUT_MARKER_CODE_START:
		case OUTPUT_MARKER_CODE_END:
		case OUTPUT_MARKER_BLOCK_START:
			/* all the above do nothing */
			break;
		
		case OUTPUT_MARKER_EMP_START:
			write(draw_state->output_file,troff_italic,troff_italic_size);
			break;
		case OUTPUT_MARKER_EMP_END:
			write(draw_state->output_file,"\n",1);
			break;

		case OUTPUT_MARKER_HEADER_START:
			if (level == 0)
			{
				write(draw_state->output_file,troff_section,troff_section_size);
			}
			else
			{
				write(draw_state->output_file,troff_subsection,troff_subsection_size);
			}
			break;
		case OUTPUT_MARKER_HEADER_END:
			write(draw_state->output_file,"\n",1);
			break;

		case OUTPUT_MARKER_LINE_BREAK:
			draw_state->offset = 0;
			write(draw_state->output_file,troff_new_para,troff_new_para_size);
			write(draw_state->output_file,"\n",1);
			break;

		case OUTPUT_MARKER_ASCII_CHAR:
			array[0] = (unsigned char) marker & 0x7f;
			draw_state->offset++;
			write(draw_state->output_file,array,1);
			break;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_text
 * @desc: This function will output the given text in the following style.
 *--------------------------------------------------------------------------------*/
void	format_text(DRAW_STATE* draw_state, unsigned int text_style, NAME* text)
{
	if ((text_style & ~(OUTPUT_TEXT_STYLE_L_SPACE|OUTPUT_TEXT_STYLE_T_SPACE|OUTPUT_TEXT_STYLE_SPACED)) == OUTPUT_TEXT_STYLE_NORMAL)
	{
		write(draw_state->output_file,"\n",1);
	}

	if (text_style & OUTPUT_TEXT_STYLE_L_NEWLINE)
	{
		write(draw_state->output_file,troff_new_para,troff_new_para_size);
	}

#if 0	
	if (text_style & (OUTPUT_TEXT_STYLE_L_SPACE | OUTPUT_TEXT_STYLE_SPACED))
	{
		write(draw_state->output_file," ",1);
	}
#endif

	if (text_style & (OUTPUT_TEXT_STYLE_BOLD | OUTPUT_TEXT_STYLE_ITALIC))
	{
		write(draw_state->output_file,troff_bold,troff_bold_size);
	}
	if (text_style & (OUTPUT_TEXT_STYLE_ITALIC))
	{
		write(draw_state->output_file,troff_italic,troff_italic_size);
	}

	if (text != NULL)
	{
		if (text->name[0] == '.' || text->name[0] == ' ')
		{
			write(draw_state->output_file,"\\&",2);
		}

		write(draw_state->output_file,text->name,text->name_length);
	}

	if (text_style & (OUTPUT_TEXT_STYLE_BOLD | OUTPUT_TEXT_STYLE_ITALIC))
	{
		write(draw_state->output_file,"\n.",2);
	}
	else if (text_style & (OUTPUT_TEXT_STYLE_T_SPACE | OUTPUT_TEXT_STYLE_SPACED))
	{
		/*don't write the space after a newline */
		write(draw_state->output_file," ",1);
	}

	if (text_style & OUTPUT_TEXT_STYLE_T_NEWLINE)
	{
		write(draw_state->output_file,troff_new_para,troff_new_para_size);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_section
 * @desc: This function will write a section to the file.
 *--------------------------------------------------------------------------------*/
void	format_section(DRAW_STATE* draw_state, unsigned int header_level,NAME* name, unsigned int format, NAME* section_data)
{
	unsigned char* level_str = troff_header_4;

	switch (header_level)
	{
		case OHL_LEVEL_1:	level_str = troff_header_1; break;
		case OHL_LEVEL_2:	level_str = troff_header_2; break;
		case OHL_LEVEL_3:	level_str = troff_header_3; break;
		case OHL_LEVEL_4:	level_str = troff_header_4; break;
	}

	write(draw_state->output_file,level_str,troff_header_level_size);
	write(draw_state->output_file,troff_bold,troff_bold_size);
	write(draw_state->output_file,name->name,name->name_length);
		
	write(draw_state->output_file,troff_margin_right,troff_margin_right_size);
	write_escaped_manpage(draw_state->output_file,section_data->name,section_data->name_length);	
	write(draw_state->output_file,troff_margin_left,troff_margin_left_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_title
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_title(DRAW_STATE* draw_state, unsigned int header_level, NAME* title, NAME* name)
{
	unsigned char* level_str = troff_header_4;

	switch (header_level)
	{
		case OHL_LEVEL_1:	level_str = troff_header_1; break;
		case OHL_LEVEL_2:	level_str = troff_header_2; break;
		case OHL_LEVEL_3:	level_str = troff_header_3; break;
		case OHL_LEVEL_4:	level_str = troff_header_4; break;
	}

	write(draw_state->output_file,troff_bold,troff_bold_size);
	write(draw_state->output_file,title->name,title->name_length);

	if (name != NULL)
	{
		write(draw_state->output_file,troff_italic,troff_italic_size);
		write(draw_state->output_file,name->name,name->name_length);
	}
	write(draw_state->output_file,troff_new_para,troff_new_para_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_block
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_block(DRAW_STATE* draw_state, unsigned int format, NAME* block)
{
	write_block_text(draw_state,block,format);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_table_start
 * @desc: This will layout a table.
 *--------------------------------------------------------------------------------*/
void	format_table_start(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout)
{
	unsigned int count;
	unsigned int opt_count = 0;
	unsigned char divider = ' ';
	unsigned char options[10];

	if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_BOXED)
	{
		divider = '|';
	}

	write(draw_state->output_file,tbl_start,tbl_start_size);
	write(draw_state->output_file,"left,tab(#);\n",sizeof("left,tab(#);\n")-1);

	/* for the header row */
	if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_HEADER)
	{
		for (count=0;count<table_layout->num_columns;count++)
		{
			opt_count = 0;

			options[opt_count++] = 'l';

			if (count+1 < table_layout->num_columns)
			{
				options[opt_count++] = ' ';
				options[opt_count++] = divider;
			}

			write(draw_state->output_file,options,opt_count);
		}
		write(draw_state->output_file,"\n",1);
	}

	/* handle the column layout */
	for (count=0;count<table_layout->num_columns;count++)
	{
		opt_count = 0;

		options[opt_count++] = 'l';

		if (table_layout->column[count].style & OUTPUT_TEXT_STYLE_BOLD)
		{
			options[opt_count++] = 'b';
		}

		if (table_layout->column[count].style & OUTPUT_TEXT_STYLE_ITALIC)
		{
			options[opt_count++] = 'i';
		}

		if (count+1 < table_layout->num_columns)
		{
			options[opt_count++] = ' ';
			options[opt_count++] = divider;
		}
		else
		{
			options[opt_count++] = '.';
		}

		write(draw_state->output_file,options,opt_count);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_table_header
 * @desc: This function will output the header.
 *--------------------------------------------------------------------------------*/
void	format_table_header(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row)
{
	unsigned int count;
	
	write(draw_state->output_file,"\n",1);

	if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_BOXED)
	{
		write(draw_state->output_file,"=\n",2);
	}

	for (count=0;count<table_layout->num_columns;count++)
	{
		if (table_row->row[count] != NULL && table_row->row[count]->name)
		{
			write(draw_state->output_file,table_row->row[count]->name,table_row->row[count]->name_length);
		}

		if (count+1 < table_layout->num_columns)
		{
			write(draw_state->output_file,"#",1);
		}
	}
	
	write(draw_state->output_file,"",1);

	if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_BOXED)
	{
		write(draw_state->output_file,"\n=",2);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_table_row
 * @desc: This function will outout the rows.
 *--------------------------------------------------------------------------------*/
void	format_table_row(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row)
{
	unsigned int count;
	
	write(draw_state->output_file,"\n",1);

	for (count=0;count<table_layout->num_columns;count++)
	{
		if (table_row->row[count] != NULL && table_row->row[count]->name)
		{
			write(draw_state->output_file,"T{\n",3);
			write(draw_state->output_file,table_row->row[count]->name,table_row->row[count]->name_length);
			write(draw_state->output_file,"\nT}",3);
		}

		if (count+1 < table_layout->num_columns)
		{
			write(draw_state->output_file,"#",1);
		}
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_table_end
 * @desc: This function will end the table.
 *--------------------------------------------------------------------------------*/
void	format_table_end(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout)
{
	if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_BOXED)
	{
		write(draw_state->output_file,"\n=",2);
	}

	write(draw_state->output_file,tbl_end,tbl_end_size);
	write(draw_state->output_file,troff_new_para,troff_new_para_size);
}

