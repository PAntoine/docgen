/**-------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *   @file	html
 *   @group	html
 *   @desc	This file holds the entry point to the html output plugin.
 *
 * 	@author	P.Antoine 
 *  date 	2012-07-19
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <malloc.h>
#include <memory.h>
#include "html.h"
#include "plugin.h"
#include "utilities.h"
#include "error_codes.h"
#include "output_format.h"
#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * HTML5 page structure.
 *--------------------------------------------------------------------------------*/

/* strings */
static unsigned char	page_start_part1[]	= "<!DOCTYPE HTML><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>"	/* cont */
											  "<script type='text/javascript' src='canvas.js'></script><title>";
static unsigned char	page_start_part2[]	= "</title></head><body>";

static unsigned char	footer_part1[]		= "<footer>";
static unsigned char	footer_part2[]		= "</footer>";

static unsigned char	canvas_start[]		= "<canvas>";
static unsigned char	canvas_end[]		= "</canvas>";

static unsigned char	header_start[]		= "<header>";
static unsigned char	header_end[]		= "</header>";

static unsigned char	header_level_start[]= "<hx>";		/* these are special and get amended by the code */
static unsigned char	header_level_end[]	= "</hx>";

static unsigned char	sequence_start[]	= "<canvas id='seq_";	/* the sequence requires an id for the javascript to be able to find it */
static unsigned char	sequence_end[]		= "'><pre>";
static unsigned char	sequence_finish[]	= "</pre></canvas><p>";

static unsigned char	pre_start[]			= "<pre>";
static unsigned char	pre_end[]			= "</pre>";

static unsigned char	space[]				= "&nbsp;";

static unsigned char	line_break[]		= "<br>";
static unsigned char	paragraph[]			= "<p>";

static unsigned char	bold_start[]		= "<b>";
static unsigned char	bold_end[]			= "</b>";
static unsigned char	italic_start[]		= "<i>";
static unsigned char	italic_end[]		= "</i>";
static unsigned char	code_start[]		= "<code>";
static unsigned char	code_end[]			= "</code>";

static unsigned char	table_start[]		= "<table >";
static unsigned char	table_start_full[]	= "<table width=90%>";
static unsigned char	table_start_box[]	= "<table border=1>";
static unsigned char	table_start_fb[]	= "<table border=1 width=90%>";
static unsigned char	table_row_start[]	= "<tr>";
static unsigned char	table_row_end[]		= "</tr>";
static unsigned char	table_col_start[]	= "<td>";
static unsigned char	table_col_bold[]	= "<td class=\"tr_bold\">";
static unsigned char	table_col_italic[]	= "<td class=\"tr_italic\">";
static unsigned char	table_col_end[]		= "</td>";
static unsigned char	table_head_start[]	= "<th>";
static unsigned char	table_head_bold[]	= "<th class=\"tr_bold\">";
static unsigned char	table_head_italic[]	= "<th class=\"tr_italic\">";
static unsigned char	table_head_end[]	= "</th>";
static unsigned char	table_end[]			= "</table>";

static unsigned char	section_start[]		= "<section>";
static unsigned char	section_end[]		= "</section>";

static unsigned char	page_end[]			= "</body></html>";

/* lengths */
static unsigned int	page_start_part1_size	= sizeof(page_start_part1) - 1;
static unsigned int	page_start_part2_size	= sizeof(page_start_part2) - 1;

static unsigned int	footer_part1_size		= sizeof(footer_part1) - 1;
static unsigned int	footer_part2_size		= sizeof(footer_part2) - 1;

static unsigned int	pre_start_size			= sizeof(pre_start) - 1;
static unsigned int	pre_end_size			= sizeof(pre_end) - 1;

static unsigned int	space_size				= sizeof(space) - 1;

static unsigned int line_break_size			= sizeof(line_break) - 1;
static unsigned int paragraph_size			= sizeof(paragraph) - 1;

static unsigned int	bold_start_size			= sizeof(bold_start) - 1;
static unsigned int	bold_end_size			= sizeof(bold_end) - 1;
static unsigned int	italic_start_size		= sizeof(italic_start) - 1;
static unsigned int	italic_end_size			= sizeof(italic_end) - 1;
static unsigned int	code_start_size			= sizeof(code_start) - 1;
static unsigned int	code_end_size			= sizeof(code_end) - 1;



static unsigned int	canvas_start_size		= sizeof(canvas_start) - 1;
static unsigned int	canvas_end_size			= sizeof(canvas_end) - 1;

static unsigned int	header_start_size		= sizeof(header_start) - 1;
static unsigned int	header_end_size			= sizeof(header_end) - 1;

static unsigned int header_level_start_size	= sizeof(header_level_start) - 1;
static unsigned int header_level_end_size	= sizeof(header_level_end) - 1;

static unsigned int sequence_start_size		= sizeof(sequence_start)-1;
static unsigned int sequence_end_size		= sizeof(sequence_end)-1;
static unsigned int sequence_finish_size	= sizeof(sequence_finish)-1;

static unsigned int	table_start_size		= sizeof(table_start) - 1;
static unsigned int	table_start_fb_size		= sizeof(table_start_fb) - 1;
static unsigned int	table_start_box_size	= sizeof(table_start_box) - 1;
static unsigned int	table_start_full_size	= sizeof(table_start_full) - 1;
static unsigned int	table_row_start_size	= sizeof(table_row_start) - 1;
static unsigned int	table_row_end_size		= sizeof(table_row_end) - 1;
static unsigned int	table_col_start_size	= sizeof(table_col_start) - 1;
static unsigned int	table_col_bold_size		= sizeof(table_col_bold) - 1;
static unsigned int	table_col_italic_size	= sizeof(table_col_italic) - 1;
static unsigned int	table_col_end_size		= sizeof(table_col_end) - 1;
static unsigned int	table_head_start_size	= sizeof(table_head_start) - 1;
static unsigned int	table_head_bold_size	= sizeof(table_head_bold) - 1;
static unsigned int	table_head_italic_size	= sizeof(table_head_italic) - 1;
static unsigned int	table_head_end_size		= sizeof(table_head_end) - 1;
static unsigned int	table_end_size			= sizeof(table_end) - 1;

static unsigned int	section_start_size		= sizeof(section_start) - 1;
static unsigned int	section_end_size		= sizeof(section_end) - 1;

static unsigned int	page_end_size			= sizeof(page_end) - 1;

/*--------------------------------------------------------------------------------*
 * Other structural components.
 *--------------------------------------------------------------------------------*/
static unsigned char	start_script_str[] 			= "<script type=\"text/javascript\">\nvar sequence_diagrams = [";
static unsigned char	start_sequence_dia_str[]	= "{name:'";
static unsigned char	canvas_id_start_str[]		= "',canvas_id:'seq_";
static unsigned char	start_timelines_str[]		= "',timelines:[";
static unsigned char	start_timeline_str[]		= "'";
static unsigned char	end_timeline_str[]			= "',";
static unsigned char	end_all_timelines_str[]		= "'";
static unsigned char	start_messages_str[]		= "],messages:[";
static unsigned char	start_message_str[]			= "{from:";
static unsigned char	message_name_str[]			= ",name:'";
static unsigned char	message_to_str[]			= "',to:";
static unsigned char	end_message_str[]			= "},";
static unsigned char	end_all_message_str[]		= "}";
static unsigned char	end_sequence_dia_str[]		= "]},";
static unsigned char	end_sequence_table_str[]	= "]}];";
static unsigned char	end_script_str[]			= "update_all(sequence_diagrams);</script>";

unsigned int	start_script_size			= sizeof(start_script_str)-1;
unsigned int	start_sequence_dia_size		= sizeof(start_sequence_dia_str)-1;
unsigned int	canvas_id_start_size		= sizeof(canvas_id_start_str)-1;
unsigned int	start_timelines_size		= sizeof(start_timelines_str)-1;
unsigned int	start_timeline_size			= sizeof(start_timeline_str)-1;
unsigned int	end_timeline_size			= sizeof(end_timeline_str)-1;
unsigned int	end_all_timelines_size		= sizeof(end_all_timelines_str)-1;
unsigned int	start_messages_size			= sizeof(start_messages_str)-1;
unsigned int	start_message_size			= sizeof(start_message_str)-1;
unsigned int	message_name_size			= sizeof(message_name_str)-1;
unsigned int	message_to_size				= sizeof(message_to_str)-1;
unsigned int	end_message_size			= sizeof(end_message_str)-1;
unsigned int	end_all_message_size		= sizeof(end_all_message_str)-1;
unsigned int	end_sequence_dia_size		= sizeof(end_sequence_dia_str)-1;
unsigned int	end_sequence_table_size		= sizeof(end_sequence_table_str)-1;
unsigned int	end_script_size				= sizeof(end_script_str)-1;

/*--------------------------------------------------------------------------------*
 * HTML character escaping.
 *--------------------------------------------------------------------------------*/

static unsigned char	html_lookup[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x07,0x08,0x04,0x00,0x0a,0x0b,0x01,0x05,0x0c,0x0d,0x00,0x0f,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x0e,0x03,0x00,
	0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x00,0x13,0x00,0x00,
	0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x11,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

typedef struct
{
	unsigned char*	html_char;
	unsigned int	size;
} HTML_CHARS;

static HTML_CHARS	html_replace[] = 
{
	{(unsigned char*) "&#255;",	sizeof("&#255;")-1},
	{(unsigned char*) "&#38;",	sizeof("&#38;")-1},
	{(unsigned char*) "&#60;",	sizeof("&#60;")-1},
	{(unsigned char*) "&#62;",	sizeof("&#62;")-1},
	{(unsigned char*) "&#34;",	sizeof("&#34;")-1},
	{(unsigned char*) "&#39;",	sizeof("&#39;")-1},
	{(unsigned char*) "&#96;",	sizeof("&#96;")-1},
	{(unsigned char*) "&#32;",	sizeof("&#32;")-1},
	{(unsigned char*) "&#33;",	sizeof("&#33;")-1},
	{(unsigned char*) "&#64;",	sizeof("&#64;")-1},
	{(unsigned char*) "&#36;",	sizeof("&#36;")-1},
	{(unsigned char*) "&#37;",	sizeof("&#37;")-1},
	{(unsigned char*) "&#40;",	sizeof("&#40;")-1},
	{(unsigned char*) "&#41;",	sizeof("&#41;")-1},
	{(unsigned char*) "&#61;",	sizeof("&#61;")-1},
	{(unsigned char*) "&#43;",	sizeof("&#43;")-1},
	{(unsigned char*) "&#123;",	sizeof("&#123;")-1},
	{(unsigned char*) "&#125;",	sizeof("&#125;")-1},
	{(unsigned char*) "&#91;",	sizeof("&#91;")-1},
	{(unsigned char*) "&#93;",	sizeof("&#93;")-1}
};

/*---------------------------------------------------------------------------------*
 * @constant	DEFAULT_PAGE_WIDTH
 * @description	default page width - used only for laying out sequence diagrams.
 *              if we don't support the canvas.
 *--------------------------------------------------------------------------------*/
#define DEFAULT_PAGE_WIDTH	(1024)

/*--------------------------------------------------------------------------------*
 * Plugin Globals
 *--------------------------------------------------------------------------------*/
static unsigned char	format_name[] = "html";

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
	MAKE_VERSION(VERSION_MAJOR,HTML_VERSION),
	DGOF_PLUGIN_OUTPUT,
	DGOF_plugin_release,
	0L,
	&format
};


/*--------------------------------------------------------------------------------*
 * utility functions.
 *--------------------------------------------------------------------------------*/

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: write_escaped_html
 * @desc: This function will write escaped html to the file, it will walk through
 *        all the text in the given buffer and write the escaped chars out instead
 *        of the ascii chars that were passed in.
 *
 *        This function will also search for emp chars and handle those.
 *--------------------------------------------------------------------------------*/
static void	write_escaped_html(int file_no, unsigned char* buffer, unsigned int buffer_length)
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
			write(file_no,bold_start,bold_start_size);
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
				write(file_no,bold_end,bold_end_size);
				write(file_no," ",1);
				last_write = count+1;

				emp = 0;
			}
			else
			{
				end_emp = 0;
			}
		}
		else if (html_lookup[buffer[count]])
		{
			/* ok, the char needs escaping */
			write(file_no,&buffer[last_write],count-last_write);
			
			/* skip the char */
			last_write = count+1;

			write(file_no,html_replace[html_lookup[buffer[count]]].html_char,html_replace[html_lookup[buffer[count]]].size);
		}

		space = (buffer[count] == 0x20);
	}

	write(file_no,&buffer[last_write],count-last_write);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: write_escaped_fixed_html
 * @desc: This function will write escaped html to the file, it will walk through
 *        all the text in the given buffer and write the escaped chars out instead
 *        of the ascii chars that were passed in.
 *--------------------------------------------------------------------------------*/
static void	write_escaped_fixed_html(int file_no, unsigned char* buffer, unsigned int buffer_length)
{
	unsigned int count;
	unsigned int last_write = 0;

	for (count=0; count < buffer_length; count++)
	{
		if (html_lookup[buffer[count]])
		{
			/* ok, the char needs escaping */
			write(file_no,&buffer[last_write],count-last_write);
			
			/* skip the char */
			last_write = count+1;

			write(file_no,html_replace[html_lookup[buffer[count]]].html_char,html_replace[html_lookup[buffer[count]]].size);
		}
	}

	write(file_no,&buffer[last_write],count-last_write);
}



/*----- FUNCTION -----------------------------------------------------------------*
 * @name: html_add_sequence
 * @desc: This function will add a sequence diagram to the list.
 *--------------------------------------------------------------------------------*/
static HTML_SEQUENCE_DIAGRAM*	html_add_sequence(HTML_SEQUENCE_DIAGRAM* diagram, NAME* name)
{
	HTML_SEQUENCE_DIAGRAM*	result = calloc(1,sizeof(HTML_SEQUENCE_DIAGRAM));

	/* create and connect the object */
	result->name.name = name->name;
	result->name.name_length = name->name_length;
	result->last_message = &result->message;
	result->last_timeline = &result->timeline;
	result->sequence_no = diagram->sequence_no;

	/* add to the list */
	result->next = diagram->next;
	diagram->next = result;

	diagram->sequence_no++;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: html_add_timeline
 * @desc: This function will add a timeline to the sequence tree.
 *--------------------------------------------------------------------------------*/
static	void	html_add_timeline(HTML_SEQUENCE_DIAGRAM* diagram, TIMELINE* timeline)
{
	HTML_TIMELINE*	new_timeline;

	if (diagram->timelines == 0)
	{
 		new_timeline = calloc(1,sizeof(HTML_TIMELINE));
		new_timeline->timeline = timeline;

		diagram->last_timeline->next 	= new_timeline;
		diagram->last_timeline			= new_timeline;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: html_add_message
 * @desc: This function will add a message to the sequence diagram.
 *--------------------------------------------------------------------------------*/
static	void	html_add_message(HTML_SEQUENCE_DIAGRAM* diagram, MESSAGE* message)
{
	HTML_MESSAGE*	new_message = calloc(1,sizeof(HTML_MESSAGE));

	if (message->sending_timeline != NULL)
	{
		new_message->from = message->sending_timeline->column;
	}

	if (message->target_timeline != NULL)
	{
		new_message->to = message->target_timeline->column;
	}

	new_message->message = message;
	diagram->last_message->next = new_message;
	diagram->last_message 		= new_message;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: html_generate_sequence_diagram
 * @desc: This function will output the javascript array for the sequence
 *        diagrams.
 *--------------------------------------------------------------------------------*/
static	void	html_generate_sequence_diagram(DRAW_STATE* draw_state, HTML_SEQUENCE_DIAGRAM* diagram)
{
	unsigned int			num_length;
	unsigned char			number[10];
	HTML_MESSAGE*			current_message;
	HTML_TIMELINE*			current_timeline;
	HTML_SEQUENCE_DIAGRAM*	next;
	HTML_SEQUENCE_DIAGRAM*	current = diagram;

	write(draw_state->output_file,start_script_str,start_script_size);

	while (current != NULL)
	{
		write(draw_state->output_file,start_sequence_dia_str,start_sequence_dia_size);
		write(draw_state->output_file,diagram->name.name,diagram->name.name_length);
		write(draw_state->output_file,canvas_id_start_str,canvas_id_start_size);
		num_length = SizeToString(number,current->sequence_no,10);
		write(draw_state->output_file,&number[10-num_length],num_length);
		write(draw_state->output_file,start_timelines_str,start_timelines_size);

		current_timeline = current->timeline.next;

		/* output timelines first */
		while (current_timeline != NULL)
		{
			write(draw_state->output_file,start_timeline_str,start_timeline_size);
			write(draw_state->output_file,current_timeline->timeline->name,current_timeline->timeline->name_length);

			/* is it the last timeline? */
			if (current_timeline->next != NULL)
			{
				write(draw_state->output_file,end_timeline_str,end_timeline_size);
			}
			else
			{
				write(draw_state->output_file,end_all_timelines_str,end_all_timelines_size);
			}

			current_timeline = current_timeline->next;
		}

		write(draw_state->output_file,start_messages_str,start_messages_size);

		current_timeline = current->timeline.next;

		current_message = current->message.next;

		while (current_message != NULL)
		{
			/* output the message */
			num_length = SizeToString(number,current_message->from,10);

			write(draw_state->output_file,start_message_str,start_message_size);
			write(draw_state->output_file,&number[10-num_length],num_length);
			write(draw_state->output_file,message_name_str,message_name_size);
			write(draw_state->output_file,current_message->message->name,current_message->message->name_length);
			write(draw_state->output_file,message_to_str,message_to_size);
			num_length = SizeToString(number,current_message->to,10);
			write(draw_state->output_file,&number[10-num_length],num_length);

			/* is it the last message? */
			if (current_message->next != NULL)
			{
				write(draw_state->output_file,end_message_str,end_message_size);
			}
			else
			{
				write(draw_state->output_file,end_all_message_str,end_all_message_size);
			}

			current_message = current_message->next;
		}

		current_timeline = current_timeline->next;

		/* is it the last sequence? */
		if (current->next != NULL)
		{
			write(draw_state->output_file,end_sequence_dia_str,end_sequence_dia_size);
		}
		else
		{
			write(draw_state->output_file,end_sequence_table_str,end_sequence_table_size);
		}

		current = current->next;
	}
	write(draw_state->output_file,end_script_str,end_script_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: html_free_table
 * @desc: This function will release all the memory that has been allocated to
 *        the given sequence table tree.
 *--------------------------------------------------------------------------------*/
static	void	html_free_table(HTML_SEQUENCE_DIAGRAM* table)
{
	HTML_SEQUENCE_DIAGRAM*	next;
	HTML_SEQUENCE_DIAGRAM*	current = table;

	while (current != NULL)
	{
		next = current->next;

		/* free current */
		free_name(&current->name);
		free(current);

		current = next;
	}
}

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

	*version		= MAKE_VERSION(VERSION_MAJOR,HTML_VERSION);
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
	HTML_SEQUENCE_DIAGRAM*	diagram;

	if ((length + 4) < MAX_FILENAME)
	{
		memcpy(&draw_state->path[draw_state->path_length],name,name_length);
		draw_state->path[length++] = '.';
		draw_state->path[length++] = 'h';
		draw_state->path[length++] = 't';
		draw_state->path[length++] = 'm';
		draw_state->path[length++] = 'l';
		draw_state->path[length] = '\0';
	
		draw_state->page_width = DEFAULT_PAGE_WIDTH;
		draw_state->global_margin_width = 1;
		draw_state->global_format_flags = 0;

		draw_state->output_buffer = malloc(draw_state->page_width + 1);
		draw_state->output_buffer[draw_state->page_width] = '\n';

		if ((draw_state->output_file = open((char*)draw_state->path,O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) != -1)
		{
			/* file successfully opened - write header*/
			write(draw_state->output_file,page_start_part1,page_start_part1_size);
			write_escaped_html(draw_state->output_file,name,name_length);
			write(draw_state->output_file,page_start_part2,page_start_part2_size);

			/* set up the sequence diagram table */
			diagram = calloc(1,sizeof(HTML_SEQUENCE_DIAGRAM));
			draw_state->data.sequence.data = diagram;
			diagram->last_message = &diagram->message;
			diagram->last_timeline = &diagram->timeline;

			result = EC_OK;
		}
	}
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_close
 * @desc: This function will close the format files.
 *--------------------------------------------------------------------------------*/
void	format_close(DRAW_STATE* draw_state)
{
	if (draw_state->output_file != -1)
	{
		/* need to output the html table */
		html_generate_sequence_diagram(draw_state,((HTML_SEQUENCE_DIAGRAM*)draw_state->data.sequence.data)->next);

		/* TODO: need to write the footer here */

		write(draw_state->output_file,page_end,page_end_size);
		close(draw_state->output_file);
		free(draw_state->output_buffer);
	}

	/* close has been called  - lose the captured data */
	html_free_table((HTML_SEQUENCE_DIAGRAM*)draw_state->data.sequence.data);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_header
 * @desc: This function will add a section header and separate the objects from
 *        each other.
 *--------------------------------------------------------------------------------*/
void	format_header(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	NAME			name;
	NAME			group;
	NAME			item;
	unsigned int	num_length;
	unsigned int	name_length;
	unsigned char	number[10];

	if (input_state->state == TYPE_SEQUENCE_DIAGRAM)
	{
		/* write the start of the sequence header to the file */
		write(draw_state->output_file,sequence_start,sequence_start_size);
		num_length = SizeToString(number,((HTML_SEQUENCE_DIAGRAM*)draw_state->data.sequence.data)->sequence_no,10);
		write(draw_state->output_file,&number[10-num_length],num_length);
		write(draw_state->output_file,sequence_end,sequence_end_size);

		if (input_state->item_length > 0)
		{
			name_length = input_state->item_length + 1 + input_state->group_length;

			allocate_name(&name,name_length);
			append_name(&name,input_state->group_name,input_state->group_length);
			append_name(&name,(unsigned char*)":",1);
			append_name(&name,input_state->item_name,input_state->item_length);
		}
		else
		{
			allocate_name(&name,input_state->group_length);
			append_name(&name,input_state->group_name,input_state->group_length);
		}

		html_add_sequence((HTML_SEQUENCE_DIAGRAM*)draw_state->data.sequence.data,&name);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_footer
 * @desc: This is the opposite of the header and exists an item.
 *--------------------------------------------------------------------------------*/
void	format_footer(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	if (input_state->state == TYPE_SEQUENCE_DIAGRAM)
	{
		write(draw_state->output_file,sequence_finish,sequence_finish_size);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_raw
 * @desc: This function will output raw text. It will find the \n and replace 
 *        them with a ' '.
 *--------------------------------------------------------------------------------*/
void	format_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size)
{
	unsigned int offset = 0;

	while (offset < buffer_size)
	{
		if (buffer[offset] == 0x0a || buffer[offset] == 0x0d)
		{
			buffer[offset] = ' ';
		}
		offset++;
	}

	write(draw_state->output_file,buffer,buffer_size);
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
	write(draw_state->output_file,section_start,section_start_size);
	write(draw_state->output_file,pre_start,pre_start_size);
	
	write_escaped_fixed_html(draw_state->output_file,sample->sample.name,sample->sample.name_length);

	write(draw_state->output_file,pre_end,pre_end_size);
	write(draw_state->output_file,section_end,section_end_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_timelines
 * @desc: This function writes the timelines, it expects to be within a <pre>
 *        section so that the text ascii art representation can be used.
 *--------------------------------------------------------------------------------*/
void	format_timelines(DRAW_STATE* draw_state)
{
	unsigned int	offset = 0;
	unsigned int	write_size = 0;
	TIMELINE* 		current = draw_state->data.sequence.start_timeline;
	
	/* unformatted man page line */
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
		
			/* add it to the structure for the javascript */
			html_add_timeline(((HTML_SEQUENCE_DIAGRAM*)draw_state->data.sequence.data)->next,current);
		}

		current = current->next;
	}
		
	write(draw_state->output_file,draw_state->output_buffer,write_size);
	
	/* stop duplication of timelines when the bottom is called */
	((HTML_SEQUENCE_DIAGRAM*)draw_state->data.sequence.data)->next->timelines = 1;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_message
 * @desc: The outputs the message.
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
	

	buffer = &draw_state->output_buffer[draw_state->margin_width];

	if (message->receiver == NULL)
	{
		special = '?';
	}

	/* clear the line */
	memset(draw_state->output_buffer,' ',draw_state->page_width+1);

	if (message->sending_timeline != NULL)
	{
		/* add the message to the structure to generate the javascript */
		html_add_message(((HTML_SEQUENCE_DIAGRAM*)draw_state->data.sequence.data)->next,message);

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
			write(draw_state->output_file,"\n",1);
			write(draw_state->output_file,draw_state->output_buffer,draw_state->data.sequence.column[draw_state->data.sequence.num_columns]);
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
		write_escaped_html(draw_state->output_file,state->name,state->name_length);
		write(draw_state->output_file,":\n",2);
	}
	else
	{
		write(draw_state->output_file,"    <unknown>:\n",sizeof("    <unknown>:\n")-1);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_transition
 * @desc: This function outputs the transition.
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
		write_escaped_html(draw_state->output_file,state->name,state->name_length);
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
		case OUTPUT_MARKER_BLOCK_END:
			/* all the above do nothing */
			break;

		case OUTPUT_MARKER_EMP_START:
			write(draw_state->output_file,italic_start,italic_start_size);
			break;
		case OUTPUT_MARKER_EMP_END:
			write(draw_state->output_file,italic_end,italic_end_size);
			break;

		case OUTPUT_MARKER_HEADER_START:
			level &= MAX_HEADER_LEVEL_MASK;
			header_level_start[2] = ('1' + level);
			write(draw_state->output_file,header_level_start,header_level_start_size);
			break;

		case OUTPUT_MARKER_HEADER_END:
			level &= MAX_HEADER_LEVEL_MASK;
			header_level_end[3] = ('1' + level);
			write(draw_state->output_file,header_level_end,header_level_end_size);
			break;

		case OUTPUT_MARKER_LINE_BREAK:
			write(draw_state->output_file,paragraph,paragraph_size);
			draw_state->offset = 0;
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
	if (text_style & OUTPUT_TEXT_STYLE_L_NEWLINE)
	{
		write(draw_state->output_file,line_break,line_break_size);
	}

	if (text_style & (OUTPUT_TEXT_STYLE_L_SPACE | OUTPUT_TEXT_STYLE_SPACED))
	{
		write(draw_state->output_file," ",1);
	}

	if (text_style & OUTPUT_TEXT_STYLE_CODE)
	{
		write(draw_state->output_file,code_start,code_start_size);
	}

	if (text_style & OUTPUT_TEXT_STYLE_BOLD)
	{
		write(draw_state->output_file,bold_start,bold_start_size);
	}

	if (text_style & OUTPUT_TEXT_STYLE_ITALIC)
	{
		write(draw_state->output_file,italic_start,italic_start_size);
	}

	if (text != NULL)
	{
		write_escaped_html(draw_state->output_file,text->name,text->name_length);
	}

	if (text_style & (OUTPUT_TEXT_STYLE_ITALIC))
	{
		write(draw_state->output_file,italic_end,italic_end_size);
	}

	if (text_style & OUTPUT_TEXT_STYLE_BOLD)
	{
		write(draw_state->output_file,bold_end,bold_end_size);
	}

	if (text_style & OUTPUT_TEXT_STYLE_CODE)
	{
		write(draw_state->output_file,code_end,code_end_size);
	}

	if (text_style & (OUTPUT_TEXT_STYLE_T_SPACE | OUTPUT_TEXT_STYLE_SPACED))
	{
		write(draw_state->output_file," ",1);
	}

	if (text_style & OUTPUT_TEXT_STYLE_T_NEWLINE)
	{
		write(draw_state->output_file,line_break,line_break_size);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_section
 * @desc: This function will write a section to the file.
 *--------------------------------------------------------------------------------*/
void	format_section(DRAW_STATE* draw_state, unsigned int header_level,NAME* name, unsigned int format, NAME* section_data)
{
	header_level &= MAX_HEADER_LEVEL_MASK;
	header_level_start[2] = ('1' + header_level);
	write(draw_state->output_file,header_level_start,header_level_start_size);
	write_escaped_html(draw_state->output_file,name->name,name->name_length);

	header_level_end[3] = ('1' + header_level);
	write(draw_state->output_file,header_level_end,header_level_end_size);
	
	write(draw_state->output_file,paragraph,paragraph_size);
	
	if (section_data->fixed)
	{
		write(draw_state->output_file,pre_start,pre_start_size);
		write_escaped_fixed_html(draw_state->output_file,section_data->name,section_data->name_length);
		write(draw_state->output_file,pre_end,pre_end_size);
	}
	else
	{
		write_escaped_html(draw_state->output_file,section_data->name,section_data->name_length);
	}
	write(draw_state->output_file,paragraph,paragraph_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_title
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_title(DRAW_STATE* draw_state, unsigned int header_level, NAME* title, NAME* name)
{

	header_level &= MAX_HEADER_LEVEL_MASK;
	header_level_start[2] = ('1' + header_level);
	write(draw_state->output_file,header_level_start,header_level_start_size);

	write_escaped_html(draw_state->output_file,title->name,title->name_length);

	if (name != NULL)
	{
		write(draw_state->output_file,space,space_size);
		write_escaped_html(draw_state->output_file,name->name,name->name_length);
	}

	header_level_end[3] = ('1' + header_level);
	write(draw_state->output_file,header_level_end,header_level_end_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_block
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_block(DRAW_STATE* draw_state, unsigned int format, NAME* block)
{
	if (block->fixed)
	{
		write(draw_state->output_file,pre_start,pre_start_size);
	}

	write_block_text(draw_state,block,format);

	if (block->fixed)
	{
		write(draw_state->output_file,pre_end,pre_end_size);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_table_start
 * @desc: This will layout a table.
 *--------------------------------------------------------------------------------*/
void	format_table_start(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout)
{

	if ((table_layout->table_flags & OUTPUT_TABLE_FORMAT_BOXED) && (table_layout->table_flags & OUTPUT_TABLE_FORMAT_FULL_WIDTH))
	{
		write(draw_state->output_file,table_start_fb,table_start_fb_size);
	}
	else if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_BOXED)
	{
		write(draw_state->output_file,table_start_box,table_start_box_size);
	}
	else if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_BOXED)
	{
		write(draw_state->output_file,table_start_full,table_start_full_size);
	}
	else
	{
		write(draw_state->output_file,table_start,table_start_size);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_table_header
 * @desc: This function will output the header.
 *--------------------------------------------------------------------------------*/
void	format_table_header(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row)
{
	unsigned int count = 0;

	write(draw_state->output_file,table_row_start,table_row_start_size);
	
	for (count = 0; count < table_layout->num_columns; count++)
	{
		write(draw_state->output_file,table_head_start,table_head_start_size);

		if (table_row->row[count] != NULL && table_row->row[count]->name)
		{
			write_escaped_fixed_html(draw_state->output_file,table_row->row[count]->name,table_row->row[count]->name_length);
		}
		else
		{
			write(draw_state->output_file,space,space_size);
		}

		write(draw_state->output_file,table_head_end,table_head_end_size);
	}

	write(draw_state->output_file,table_row_end,table_row_end_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_table_row
 * @desc: This function will output the rows.
 *--------------------------------------------------------------------------------*/
void	format_table_row(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row)
{
	unsigned int count = 0;

	write(draw_state->output_file,table_row_start,table_row_start_size);
	
	for (count = 0; count < table_layout->num_columns; count++)
	{
		write(draw_state->output_file,table_col_start,table_col_start_size);

		if (table_row->row[count] != NULL && table_row->row[count]->name)
		{
			write_escaped_fixed_html(draw_state->output_file,table_row->row[count]->name,table_row->row[count]->name_length);
		}
		else
		{
			write(draw_state->output_file,space,space_size);
		}

		write(draw_state->output_file,table_col_end,table_col_end_size);
	}

	write(draw_state->output_file,table_row_end,table_row_end_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_table_end
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_table_end(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout)
{
	write(draw_state->output_file,table_end,table_end_size);
}

