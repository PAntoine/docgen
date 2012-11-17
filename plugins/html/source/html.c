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
#include "level_index.h"
#include "output_format.h"
#include "state_machine.h"
#include "document_generator.h"

/*--------------------------------------------------------------------------------*
 * HTML5 page structure.
 *--------------------------------------------------------------------------------*/

/* strings */
static unsigned char	page_start_part1[]		= "<!DOCTYPE HTML><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>"	/* cont */
												  "<link rel=\"stylesheet\" href=\"css/doc_gen.css\">" 													/* cont */
												  "<script type='text/javascript' src='js/doc_gen.js'></script><title>";
static unsigned char	page_start_part2[]		= "</title></head><body onload=\"close_all_divs();\"><div class=\"title_bar\" ><span class=\"project_title\">";
static unsigned char	page_start_part3[]		= "</span><span class=\"project_version\">&nbsp;version&nbsp;";
static unsigned char	page_start_part4[]		= "</span><div class=\"clear\"></div></div><div class=\"side_bar\">";
static unsigned char	page_start_part5[]		= "</div><div class=\"main_body\">";

static unsigned char	footer_part1[]			= "<footer>";
static unsigned char	footer_part2[]			= "</footer>";

static unsigned char	canvas_start[]			= "<canvas>";
static unsigned char	canvas_end[]			= "</canvas>";

static unsigned char	header_start[]			= "<header>";
static unsigned char	header_end[]			= "</header>";

static unsigned char	header_level_a_start[]	= "<hx id=\"a";		/* these are special and get amended by the code */
static unsigned char	header_level_a_end[]	= "\">";		
static unsigned char	header_level_start[]	= "<hx>";			/* these are special and get amended by the code */
static unsigned char	header_level_end[]		= "</hx>";

static unsigned char	sequence_start[]		= "<div class=centered><canvas id='seq_";	/* the sequence requires an id for the javascript to be able to find it */
static unsigned char	state_start[]			= "<div class=centered><canvas id='sta_";	/* the state_machine requires an id for the javascript to be able to find it */
static unsigned char	sequence_end[]			= "'><pre>";
static unsigned char	sequence_finish[]		= "</pre></canvas></div>";

static unsigned char	pre_start[]				= "<pre>";
static unsigned char	pre_end[]				= "</pre>";

static unsigned char	space[]					= "&nbsp;";

static unsigned char	line_break[]			= "<br>";
static unsigned char	paragraph[]				= "<p>";

static unsigned char	bold_start[]			= "<b>";
static unsigned char	bold_end[]				= "</b>";
static unsigned char	italic_start[]			= "<i>";
static unsigned char	italic_end[]			= "</i>";
static unsigned char	code_start[]			= "<code>";
static unsigned char	code_end[]				= "</code>";
static unsigned char	div_start[]				= "<div>";
static unsigned char	div_end[]				= "</div>";

static unsigned char	blockquote_start[]		= "<blockquote>";
static unsigned char	blockquote_end[]		= "</blockquote>";

static unsigned char	table_start[]			= "<table>";
static unsigned char	table_start_proto[]		= "<table class=\"code_table\">";
static unsigned char	table_start_full[]		= "<table width=90%>";
static unsigned char	table_start_box[]		= "<table class=\"parameters shadow\">";
static unsigned char	table_start_fb[]		= "<table class=\"parameters shadow\">";
static unsigned char	table_row_start[]		= "<tr>";
static unsigned char	table_row_end[]			= "</tr>";
static unsigned char	table_col_start[]		= "<td>";
static unsigned char	table_col_bold[]		= "<td class=\"tr_bold\">";
static unsigned char	table_col_italic[]		= "<td class=\"tr_italic\">";
static unsigned char	table_col_end[]			= "</td>";
static unsigned char	table_head_start[]		= "<th>";
static unsigned char	table_head_bold[]		= "<th class=\"tr_bold\">";
static unsigned char	table_head_italic[]		= "<th class=\"tr_italic\">";
static unsigned char	table_head_end[]		= "</th>";
static unsigned char	table_end[]				= "</table>";

static unsigned char	section_start[]			= "<section>";
static unsigned char	section_end[]			= "</section>";

static unsigned char	item_no_header[]		= "<div class=\"item\"><div class=\"item_body rounded_bottom rounded_top shadow class=centered\">";
static unsigned char	item_start[]			= "<div class=\"item\"><div class=\"item_title rounded_top shadow class=centered\"><h2>";
static unsigned char	item_header_end[]		= "</h2></div><div class=\"item_body rounded_bottom shadow\">";
static unsigned char	item_end[]				= "</div></div>";

static unsigned char	index_item_part1[]		= "<div class=\"index_item\"><a href=\"#a";
static unsigned char	index_item_part2[]		= "\">";
static unsigned char	index_item_part3[]		= "</a></div>";
static unsigned char	index_down_part1[]		= "<div class=\"index_sub_level\" onclick=\"ToggleHideDiv('si";
static unsigned char	index_down_part2[]		= "'); return true;\"><a href=\"#a";
static unsigned char	index_down_part3[]		= "\">";
static unsigned char	index_down_part4[]		= "</a></div>";
static unsigned char	index_toggle_part1[]	= "<div class='hh' id=\"si";
static unsigned char	index_toggle_part2[]	= "\">";

static unsigned char	index_anchor_start[]	= "<a id=\"a";
static unsigned char	index_anchor_end[]		= "\">";

static unsigned char	numb_list_start[]		= "<ol>";
static unsigned char	numb_list_end[]			= "</ol>";
static unsigned char	dash_list_start[]		= "<ul type=\"square\">";
static unsigned char	asti_list_start[]		= "<ul type=\"disc\">";
static unsigned char	plus_list_start[]		= "<ul type=\"circle\">";
static unsigned char	list_end[]				= "</ul>";
static unsigned char	list_item_start[]		= "<li>";
static unsigned char	list_item_end[]			= "</li>";

static unsigned char	page_end[]				= "</div></body></html>";

/* lengths */
static unsigned int	page_start_part1_size	= sizeof(page_start_part1) - 1;
static unsigned int	page_start_part2_size	= sizeof(page_start_part2) - 1;
static unsigned int	page_start_part3_size	= sizeof(page_start_part3) - 1;
static unsigned int	page_start_part4_size	= sizeof(page_start_part4) - 1;
static unsigned int	page_start_part5_size	= sizeof(page_start_part5) - 1;

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
static unsigned int div_start_size			= sizeof(div_start) - 1;
static unsigned int div_end_size			= sizeof(div_end) - 1;

static unsigned int blockquote_start_size	= sizeof(blockquote_start) - 1;
static unsigned int blockquote_end_size		= sizeof(blockquote_end) - 1;

static unsigned int	canvas_start_size		= sizeof(canvas_start) - 1;
static unsigned int	canvas_end_size			= sizeof(canvas_end) - 1;

static unsigned int	header_start_size		= sizeof(header_start) - 1;
static unsigned int	header_end_size			= sizeof(header_end) - 1;

static unsigned int header_level_a_start_size	= sizeof(header_level_a_start) - 1;
static unsigned int header_level_a_end_size		= sizeof(header_level_a_end) - 1;
static unsigned int header_level_start_size	= sizeof(header_level_start) - 1;
static unsigned int header_level_end_size	= sizeof(header_level_end) - 1;

static unsigned int state_start_size		= sizeof(state_start)-1;
static unsigned int sequence_start_size		= sizeof(sequence_start)-1;
static unsigned int sequence_end_size		= sizeof(sequence_end)-1;
static unsigned int sequence_finish_size	= sizeof(sequence_finish)-1;

static unsigned int	table_start_size		= sizeof(table_start) - 1;
static unsigned int	table_start_fb_size		= sizeof(table_start_fb) - 1;
static unsigned int	table_start_box_size	= sizeof(table_start_box) - 1;
static unsigned int	table_start_proto_size	= sizeof(table_start_proto) - 1;
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

static unsigned int item_no_header_size		= sizeof(item_no_header) - 1;
static unsigned int item_start_size			= sizeof(item_start) - 1;
static unsigned int item_header_end_size	= sizeof(item_header_end) - 1;
static unsigned int item_end_size			= sizeof(item_end) - 1;

static unsigned int index_item_part1_size	= sizeof(index_item_part1) - 1;
static unsigned int index_item_part2_size	= sizeof(index_item_part2) - 1;
static unsigned int index_item_part3_size	= sizeof(index_item_part3) - 1;
static unsigned int index_down_part1_size	= sizeof(index_down_part1) - 1;
static unsigned int index_down_part2_size	= sizeof(index_down_part2) - 1;
static unsigned int index_down_part3_size	= sizeof(index_down_part3) - 1;
static unsigned int index_down_part4_size	= sizeof(index_down_part4) - 1;
static unsigned int index_toggle_part1_size	= sizeof(index_toggle_part1) - 1;
static unsigned int index_toggle_part2_size	= sizeof(index_toggle_part2) - 1;

static unsigned int index_anchor_start_size = sizeof(index_anchor_start) - 1;
static unsigned int index_anchor_end_size	= sizeof(index_anchor_end) - 1;

static unsigned int numb_list_start_size	= sizeof(numb_list_start) - 1;
static unsigned int numb_list_end_size		= sizeof(numb_list_end) - 1;
static unsigned int dash_list_start_size	= sizeof(dash_list_start) - 1;
static unsigned int asti_list_start_size	= sizeof(asti_list_start) - 1;
static unsigned int plus_list_start_size	= sizeof(plus_list_start) - 1;
static unsigned int list_end_size			= sizeof(list_end) - 1;

static unsigned int	list_item_start_size	= sizeof(list_item_start) - 1;
static unsigned int	list_item_end_size		= sizeof(list_item_end) - 1;

static unsigned int	page_end_size			= sizeof(page_end) - 1;

/*--------------------------------------------------------------------------------*
 * Strings for the Javascript arrays for the sequence_diagrams.
 *--------------------------------------------------------------------------------*/
static unsigned char	seq_start_script_str[] 			= "<script type=\"text/javascript\">\nvar sequence_diagrams = [";
static unsigned char	seq_start_sequence_dia_str[]	= "{name:'";
static unsigned char	seq_canvas_id_start_str[]		= "',canvas_id:'seq_";
static unsigned char	seq_start_timelines_str[]		= "',timelines:[";
static unsigned char	seq_start_timeline_str[]		= "'";
static unsigned char	seq_end_timeline_str[]			= "',";
static unsigned char	seq_end_all_timelines_str[]		= "'";
static unsigned char	seq_start_messages_str[]		= "],messages:[";
static unsigned char	seq_start_message_str[]			= "{from:";
static unsigned char	seq_message_name_str[]			= ",name:'";
static unsigned char	seq_message_to_str[]			= "',to:";
static unsigned char	seq_end_message_str[]			= "},";
static unsigned char	seq_end_all_message_str[]		= "}";
static unsigned char	seq_end_sequence_dia_str[]		= "]},";
static unsigned char	seq_end_sequence_table_str[]	= "]}];";
static unsigned char	seq_end_script_str[]			= "update_sequence_diagrams(sequence_diagrams);</script>";

unsigned int	seq_start_script_size			= sizeof(seq_start_script_str)-1;
unsigned int	seq_start_sequence_dia_size		= sizeof(seq_start_sequence_dia_str)-1;
unsigned int	seq_canvas_id_start_size		= sizeof(seq_canvas_id_start_str)-1;
unsigned int	seq_start_timelines_size		= sizeof(seq_start_timelines_str)-1;
unsigned int	seq_start_timeline_size			= sizeof(seq_start_timeline_str)-1;
unsigned int	seq_end_timeline_size			= sizeof(seq_end_timeline_str)-1;
unsigned int	seq_end_all_timelines_size		= sizeof(seq_end_all_timelines_str)-1;
unsigned int	seq_start_messages_size			= sizeof(seq_start_messages_str)-1;
unsigned int	seq_start_message_size			= sizeof(seq_start_message_str)-1;
unsigned int	seq_message_name_size			= sizeof(seq_message_name_str)-1;
unsigned int	seq_message_to_size				= sizeof(seq_message_to_str)-1;
unsigned int	seq_end_message_size			= sizeof(seq_end_message_str)-1;
unsigned int	seq_end_all_message_size		= sizeof(seq_end_all_message_str)-1;
unsigned int	seq_end_sequence_dia_size		= sizeof(seq_end_sequence_dia_str)-1;
unsigned int	seq_end_sequence_table_size		= sizeof(seq_end_sequence_table_str)-1;
unsigned int	seq_end_script_size				= sizeof(seq_end_script_str)-1;

/*--------------------------------------------------------------------------------*
 * Strings for the Javascript arrays for the state machine.
 * 
 * state_machines = [
 *  {name:'name_of',canvas_id:'sta_0',x_size:nn,y_size:nn,
 *  nodes:[{name:'node_name',x:nn,y:nn},...],
 *  vertices:[{from_node:nn,to_node:nn,trigger:'<some_text>',triggers:'<some_text>'},..],
 *  },];
 *--------------------------------------------------------------------------------*/
static unsigned char	sta_start_script_str[] 			= "<script type=\"text/javascript\">\nvar state_machines = [";
static unsigned char	sta_start_sequence_dia_str[]	= "{name:'";
static unsigned char	sta_canvas_id_start_str[]		= "',canvas_id:'sta_";
static unsigned char	sta_x_size_str[]				= "',x_size:";
static unsigned char	sta_y_size_str[]				= ",y_size:";
static unsigned char	sta_start_nodes_str[]			= ",nodes:[";
static unsigned char	sta_start_node_name_str[]		= "{name:'";
static unsigned char	sta_start_node_x_str[]			= "',x:";
static unsigned char	sta_start_node_y_str[]			= ",y:";
static unsigned char	sta_end_node_str[]			 	= "},";
static unsigned char	sta_end_all_nodes_str[]			= "}";
static unsigned char	sta_start_vertex_str[]			= "],vertices:[";
static unsigned char	sta_start_from_str[]			= "{from_node:";
static unsigned char	sta_start_to_str[]				= ",to_node:";
static unsigned char	sta_trigger_str[]				= ",trigger:'";
static unsigned char	sta_triggers_str[]				= "',triggers:'";
static unsigned char	sta_triggers_gap_str[]			= ",";
static unsigned char	sta_end_vertex_str[]			= "'},";
static unsigned char	sta_end_all_vertex_str[]		= "'}";
static unsigned char	sta_end_vertices_str[]			= "]},";
static unsigned char	sta_end_state_table_str[]		= "]}];";
static unsigned char	sta_end_script_str[]			= "update_state_machines(state_machines)</script>";

static unsigned int	sta_start_script_size				= sizeof(sta_start_script_str) - 1;
static unsigned int	sta_start_sequence_dia_size			= sizeof(sta_start_sequence_dia_str) - 1;
static unsigned int	sta_canvas_id_start_size			= sizeof(sta_canvas_id_start_str) - 1;
static unsigned int sta_x_size_size						= sizeof(sta_x_size_str) - 1;
static unsigned int sta_y_size_size						= sizeof(sta_y_size_str) - 1;
static unsigned int	sta_start_nodes_size				= sizeof(sta_start_nodes_str) - 1;
static unsigned int	sta_start_node_name_size			= sizeof(sta_start_node_name_str) - 1;
static unsigned int	sta_start_node_x_size				= sizeof(sta_start_node_x_str) - 1;
static unsigned int	sta_start_node_y_size				= sizeof(sta_start_node_y_str) - 1;
static unsigned int	sta_end_node_size					= sizeof(sta_end_node_str) - 1;
static unsigned int	sta_end_all_nodes_size				= sizeof(sta_end_all_nodes_str) - 1;
static unsigned int	sta_start_vertex_size				= sizeof(sta_start_vertex_str) - 1;
static unsigned int	sta_start_from_size					= sizeof(sta_start_from_str) - 1;
static unsigned int	sta_start_to_size					= sizeof(sta_start_to_str) - 1;
static unsigned int	sta_trigger_size					= sizeof(sta_trigger_str) - 1;
static unsigned int	sta_triggers_size					= sizeof(sta_triggers_str) - 1;
static unsigned int	sta_triggers_gap_size				= sizeof(sta_triggers_gap_str) - 1;
static unsigned int	sta_end_vertex_size					= sizeof(sta_end_vertex_str) - 1;
static unsigned int	sta_end_all_vertex_size				= sizeof(sta_end_all_vertex_str) - 1;
static unsigned int	sta_end_vertices_size				= sizeof(sta_end_vertices_str) - 1;
static unsigned int	sta_end_state_table_size			= sizeof(sta_end_state_table_str) - 1;
static unsigned int	sta_end_script_size					= sizeof(sta_end_script_str) - 1;


/*--------------------------------------------------------------------------------*
 * static names.
 *--------------------------------------------------------------------------------*/
static NAME				name_name = {(unsigned char*)"Name:&nbsp;",sizeof("Name:&nbsp;")-1,0,0};
static NAME				index_name = {(unsigned char*)"INDEX",sizeof("INDEX")-1,0,0};

/*--------------------------------------------------------------------------------*
 * static files that need copying to the output.
 *--------------------------------------------------------------------------------*/
static NAME		copy_file_from[]	= { {(unsigned char*)"/html/doc_gen.js",sizeof("/html/doc_gen.js"),0,0},
										{(unsigned char*)"/html/doc_gen.css",sizeof("/html/doc_gen.css"),0,0}};
static NAME		copy_file_to[]		= { {(unsigned char*)"js/doc_gen.js",sizeof("js/doc_gen.js"),0,0},
										{(unsigned char*)"css/doc_gen.css",sizeof("css/doc_gen.css"),0,0}};

#define NUMBER_OF_COPY_FILES	((sizeof(copy_file_from)/sizeof(copy_file_from[0])))

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

static unsigned char	format_name[] = "html";

/*--------------------------------------------------------------------------------*
 * Plugin Globals
 *--------------------------------------------------------------------------------*/

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

	format_state_set_size,
	format_state,
	format_transition,

	format_marker,

	format_text,

	format_section,
	format_title,
	format_block,

	format_table_start,
	format_table_header,
	format_table_row,
	format_table_end,

	format_index_chapter,
	format_index_start_sublevel,
	format_index_entry,
	format_index_end_sublevel,

	format_list_item_start,
	format_list_numeric_start,
	format_list_end,

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
	&format,
	0L
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
static void	write_escaped_html(int file_no, unsigned char* buffer, unsigned int buffer_length, unsigned int no_space)
{
	unsigned int count;
	unsigned int last_write = 0;

	if (!text_punctuation(buffer) && !no_space)
	{
		/* it's a continuation - so need to add a space */
		write(file_no," ",1);
	}

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
 * @name: html_add_state_machine
 * @desc: This function will add a state_machine diagram to the list.
 *--------------------------------------------------------------------------------*/
static HTML_STATE_MACHINE*	html_add_state_machine(HTML_STATE_MACHINE* diagram, NAME* name)
{
	HTML_STATE_MACHINE*	result = calloc(1,sizeof(HTML_STATE_MACHINE));

	/* create and connect the object */
	result->name.name = name->name;
	result->name.name_length = name->name_length;
	result->state_machine_no = diagram->state_machine_no;

	/* create the state machine */
	state_machine_allocate(&result->state_machine);

	/* add to the list */
	result->next = diagram->next;
	diagram->next = result;

	diagram->state_machine_no++;

	return result;
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
 * @name: html_generate_sequence_diagrams
 * @desc: This function will output the javascript array for the sequence
 *        diagrams.
 *--------------------------------------------------------------------------------*/
static	void	html_generate_sequence_diagrams(DRAW_STATE* draw_state, HTML_SEQUENCE_DIAGRAM* diagram)
{
	unsigned int			num_length;
	unsigned char			number[10];
	HTML_MESSAGE*			current_message;
	HTML_TIMELINE*			current_timeline;
	HTML_SEQUENCE_DIAGRAM*	next;
	HTML_SEQUENCE_DIAGRAM*	current = diagram;

	if (current != NULL)
	{
		write(draw_state->output_file,seq_start_script_str,seq_start_script_size);

		while (current != NULL)
		{
			write(draw_state->output_file,seq_start_sequence_dia_str,seq_start_sequence_dia_size);
			write(draw_state->output_file,diagram->name.name,diagram->name.name_length);
			write(draw_state->output_file,seq_canvas_id_start_str,seq_canvas_id_start_size);
			num_length = SizeToString(number,current->sequence_no,10);
			write(draw_state->output_file,&number[10-num_length],num_length);
			write(draw_state->output_file,seq_start_timelines_str,seq_start_timelines_size);

			current_timeline = current->timeline.next;

			/* output timelines first */
			while (current_timeline != NULL)
			{
				write(draw_state->output_file,seq_start_timeline_str,seq_start_timeline_size);
				write(draw_state->output_file,current_timeline->timeline->name,current_timeline->timeline->name_length);

				/* is it the last timeline? */
				if (current_timeline->next != NULL)
				{
					write(draw_state->output_file,seq_end_timeline_str,seq_end_timeline_size);
				}
				else
				{
					write(draw_state->output_file,seq_end_all_timelines_str,seq_end_all_timelines_size);
				}

				current_timeline = current_timeline->next;
			}

			write(draw_state->output_file,seq_start_messages_str,seq_start_messages_size);

			current_message = current->message.next;

			while (current_message != NULL)
			{
				/* output the message */
				num_length = SizeToString(number,current_message->from,10);

				write(draw_state->output_file,seq_start_message_str,seq_start_message_size);
				write(draw_state->output_file,&number[10-num_length],num_length);
				write(draw_state->output_file,seq_message_name_str,seq_message_name_size);
				write(draw_state->output_file,current_message->message->name,current_message->message->name_length);
				write(draw_state->output_file,seq_message_to_str,seq_message_to_size);
				num_length = SizeToString(number,current_message->to,10);
				write(draw_state->output_file,&number[10-num_length],num_length);

				/* is it the last message? */
				if (current_message->next != NULL)
				{
					write(draw_state->output_file,seq_end_message_str,seq_end_message_size);
				}
				else
				{
					write(draw_state->output_file,seq_end_all_message_str,seq_end_all_message_size);
				}

				current_message = current_message->next;
			}

			/* is it the last sequence? */
			if (current->next != NULL)
			{
				write(draw_state->output_file,seq_end_sequence_dia_str,seq_end_sequence_dia_size);
			}
			else
			{
				write(draw_state->output_file,seq_end_sequence_table_str,seq_end_sequence_table_size);
			}

			current = current->next;
		}
		write(draw_state->output_file,seq_end_script_str,seq_end_script_size);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: html_generate_state_machines
 * @desc: This function will output the javascript array for the sequence
 *        diagrams.
 *--------------------------------------------------------------------------------*/
static	void	html_generate_state_machines(DRAW_STATE* draw_state, HTML_STATE_MACHINE* diagram)
{
	unsigned int			count;
	unsigned int			num_length;
	unsigned int			max_x = 0;
	unsigned int			max_y = 0;
	unsigned char			number[10];
	TRIGGERS*				current_triggers;
	HTML_STATE_MACHINE*		next;
	HTML_STATE_MACHINE*		current = diagram;

	if (current != NULL)
	{
		write(draw_state->output_file,sta_start_script_str,sta_start_script_size);

		while (current != NULL)
		{
			write(draw_state->output_file,sta_start_sequence_dia_str,sta_start_sequence_dia_size);
			write(draw_state->output_file,diagram->name.name,diagram->name.name_length);
			
			write(draw_state->output_file,sta_canvas_id_start_str,sta_canvas_id_start_size);
			num_length = SizeToString(number,current->state_machine_no,10);
			write(draw_state->output_file,&number[10-num_length],num_length);

			/* need to write the size of the grid */
			max_y = 0;
			max_x = 0;
			for (count=0;count<current->state_machine->num_nodes; count++)
			{
				if (current->state_machine->node_list[count].x + 1 > max_x)
				{
					max_x = current->state_machine->node_list[count].x + 1;
				}

				if (current->state_machine->node_list[count].y + 1 > max_y)
				{
					max_y = current->state_machine->node_list[count].y + 1;
				}
			}

			write(draw_state->output_file,sta_x_size_str,sta_x_size_size);
			num_length = SizeToString(number,max_x,10);
			write(draw_state->output_file,&number[10-num_length],num_length);

			write(draw_state->output_file,sta_y_size_str,sta_y_size_size);
			num_length = SizeToString(number,max_y,10);
			write(draw_state->output_file,&number[10-num_length],num_length);

			/* ok, now dump the nodes of the state_machine */
			write(draw_state->output_file,sta_start_nodes_str,sta_start_nodes_size);

			for (count=0;count<current->state_machine->num_nodes; count++)
			{
				write(draw_state->output_file,sta_start_node_name_str,sta_start_node_name_size);
				write(draw_state->output_file,current->state_machine->node_list[count].state->name,current->state_machine->node_list[count].state->name_length);

				write(draw_state->output_file,sta_start_node_x_str,sta_start_node_x_size);
				num_length = SizeToString(number,current->state_machine->node_list[count].x,10);
				write(draw_state->output_file,&number[10-num_length],num_length);

				write(draw_state->output_file,sta_start_node_y_str,sta_start_node_y_size);
				num_length = SizeToString(number,current->state_machine->node_list[count].y,10);
				write(draw_state->output_file,&number[10-num_length],num_length);

				if (count != current->state_machine->num_nodes-1)
				{
					write(draw_state->output_file,sta_end_node_str,sta_end_node_size);
				}
				else
				{
					write(draw_state->output_file,sta_end_all_nodes_str,sta_end_all_nodes_size);
				}
			}

			/* output the vertices to the javascript */
			write(draw_state->output_file,sta_start_vertex_str,sta_start_vertex_size);

			for (count=0; count < current->state_machine->num_vertices; count++)
			{
				write(draw_state->output_file,sta_start_from_str,sta_start_from_size);
				num_length = SizeToString(number,current->state_machine->vertex_list[count].from,10);
				write(draw_state->output_file,&number[10-num_length],num_length);

				write(draw_state->output_file,sta_start_to_str,sta_start_to_size);
				num_length = SizeToString(number,current->state_machine->vertex_list[count].to,10);
				write(draw_state->output_file,&number[10-num_length],num_length);

				write(draw_state->output_file,sta_trigger_str,sta_trigger_size);

				if (current->state_machine->vertex_list[count].transition->trigger != NULL && current->state_machine->vertex_list[count].transition->trigger->name_length > 0)
				{
					write(draw_state->output_file,current->state_machine->vertex_list[count].transition->trigger->name,current->state_machine->vertex_list[count].transition->trigger->name_length);
				}

				write(draw_state->output_file,sta_triggers_str,sta_triggers_size);
				current_triggers = current->state_machine->vertex_list[count].transition->triggers;

				while (current_triggers != NULL)
				{
					if (current_triggers->trigger != NULL)
					{
						if (current_triggers->trigger->group != NULL)
						{
							write(draw_state->output_file,current_triggers->trigger->group->name,current_triggers->trigger->group->name_length);
							write(draw_state->output_file,":",1);
						}

						write(draw_state->output_file,current_triggers->trigger->name,current_triggers->trigger->name_length);
					}

					if (current_triggers->next != NULL)
					{
						write(draw_state->output_file,sta_triggers_gap_str,sta_triggers_gap_size);
					}

					current_triggers = current_triggers->next;
				}

				if (count != current->state_machine->num_vertices - 1)
				{
					write(draw_state->output_file,sta_end_vertex_str,sta_end_vertex_size);
				}
				else
				{
					write(draw_state->output_file,sta_end_all_vertex_str,sta_end_all_vertex_size);
				}
			}

			if (current->next != NULL)
			{
				write(draw_state->output_file,sta_end_vertices_str,sta_end_vertices_size);
			}
			else
			{
				write(draw_state->output_file,sta_end_state_table_str,sta_end_state_table_size);
			}

			current = current->next;
		}

		write(draw_state->output_file,sta_end_script_str,sta_end_script_size);
	}
}


/*----- FUNCTION -----------------------------------------------------------------*
 * Name : html_init_table
 * Desc : This function will initialise the sequence table.
 *--------------------------------------------------------------------------------*/
static void	html_init_table(HTML_SEQUENCE_DIAGRAM* table)
{
	table->next = NULL;
	table->last_message = &table->message;
	table->last_timeline = &table->timeline;
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
unsigned int	format_decode_flags(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned hash, NAME* value)
{
	unsigned int result = OUTPUT_FLAG_TYPE_BOOLEAN;

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_open
 * @desc: This function will open the manpage and add the file header to the 
 *        page.
 *--------------------------------------------------------------------------------*/
unsigned int	format_open(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned char* name, unsigned int name_length)
{
	unsigned int			count;
	unsigned int			result = EC_FAILED;
	unsigned int			length = name_length + draw_state->path_length;
	unsigned int			temp_length = draw_state->path_length;
	unsigned char			version_buffer[VERSION_STRING_LENGTH];
	NAME					path;
	NAME					source;
	NAME					destination;
	HTML_DATA*				data;

	path.name = draw_state->path;
	path.name_length = draw_state->path_length;

	/* need to create the directories to be copied into */
	extend_path(&draw_state->path,&temp_length,"js",2);
	
	temp_length = draw_state->path_length;

	extend_path(&draw_state->path,&temp_length,"css",3);

	for (count=0; count < NUMBER_OF_COPY_FILES; count++)
	{
		concat_names(&source,&draw_state->resources_path,&copy_file_from[count],NULL);
		concat_names(&destination,&path,&copy_file_to[count],NULL);
		
		copy_file(source.name,destination.name);

		free_name(&source);
		free_name(&destination);
	}

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
		draw_state->global_max_constant = 50;

		draw_state->output_buffer = malloc(draw_state->page_width + 1);
		draw_state->output_buffer[draw_state->page_width] = '\n';

		if ((draw_state->output_file = open((char*)draw_state->path,O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) != -1)
		{
			/* set up the data for the types */
			data = calloc(1,sizeof(HTML_DATA));
			draw_state->format_state = data;

			/* set up the sequence diagram table */
			html_init_table(&((HTML_DATA*)draw_state->format_state)->sequence_diagram);

			/* file successfully opened - write header*/
			write(draw_state->output_file,page_start_part1,page_start_part1_size);
			write_escaped_html(draw_state->output_file,name,name_length,draw_state->no_space);
			draw_state->no_space = 0;
			write(draw_state->output_file,page_start_part2,page_start_part2_size);
			write(draw_state->output_file,draw_state->model->document->document_name.name,draw_state->model->document->document_name.name_length);
			write(draw_state->output_file,page_start_part3,page_start_part3_size);
			
			temp_length = make_version_string(draw_state->model->document->version,version_buffer);
			write(draw_state->output_file,&version_buffer[temp_length],VERSION_STRING_LENGTH - temp_length);

			write(draw_state->output_file,page_start_part4,page_start_part4_size);

			input_state->title.name = index_name.name;
			input_state->title.name_length = index_name.name_length;
			output_document_index(draw_state,input_state);
			input_state->title.name = NULL;
			input_state->title.name_length = 0;

			write(draw_state->output_file,page_start_part5,page_start_part5_size);

			result = EC_OK;
		}
	}
	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_close
 * @desc: This function will close the format files.
 *--------------------------------------------------------------------------------*/
void	format_close(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	if (draw_state->output_file != -1)
	{
		/* need to output the javascript arrays */
		html_generate_sequence_diagrams(draw_state,((HTML_DATA*)draw_state->format_state)->sequence_diagram.next);
		html_generate_state_machines(draw_state,((HTML_DATA*)draw_state->format_state)->state_machine.next);

		/* TODO: need to write the footer here */

		write(draw_state->output_file,page_end,page_end_size);
		close(draw_state->output_file);
		free(draw_state->output_buffer);
	}

	/* close has been called  - lose the captured data */
	html_free_table(((HTML_DATA*)draw_state->format_state)->sequence_diagram.next);
	((HTML_DATA*)draw_state->format_state)->sequence_diagram.next = NULL;

	/* free the format data */
	free(draw_state->format_state);
	draw_state->format_state = NULL;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : html_start_box
 * Desc : This function will start a box for an item.
 *        It will either start a titled box, or a plain box.
 *--------------------------------------------------------------------------------*/
static void	html_start_box(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	NAME			name = {NULL,0,0,0};
	NAME			group = {NULL,0,0,0};
	NAME			item = {NULL,0,0,0};
	unsigned int	name_length;
	unsigned int	buffer_start;
	unsigned char	buffer[256];

	/* build the name for the item */
	if (input_state->state == TYPE_API)
	{
		if (input_state->index_item != NULL)
		{
			buffer_start = index_generate_label(input_state->index_item,0, INDEX_STYLE_NUMBERS,buffer,256,'_');
			
			write(draw_state->output_file,index_anchor_start,index_anchor_start_size);
			write(draw_state->output_file,&buffer[buffer_start],256-buffer_start);
			write(draw_state->output_file,index_anchor_end,index_anchor_end_size);
		}

		name_length = name_name.name_length + 1 + input_state->title.name_length;
		allocate_name(&name,name_length);
		append_name(&name,&name_name);
		append_name(&name,&input_state->title);
		((HTML_DATA*)draw_state->format_state)->suppress_title = 1;
	}
	else if (input_state->title.name_length > 0)
	{
		copy_name(&input_state->title,&name);
	}
	else if (input_state->item_name.name_length > 0)
	{
		name_length = input_state->item_name.name_length + 1 + input_state->group_name.name_length;

		allocate_name(&name,name_length);
		append_name(&name,&input_state->group_name);
		append_string(&name,(unsigned char*)":",1);
		append_name(&name,&input_state->item_name);
	}
		
	/* start the box */
	if (name.name_length > 0)
	{
		write(draw_state->output_file,item_start,item_start_size);
		write(draw_state->output_file,name.name,name.name_length);
		write(draw_state->output_file,item_header_end,item_header_end_size);
	}
	else
	{
		write(draw_state->output_file,item_no_header,item_no_header_size);
	}

	/* release the name */
	free_name(&name);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : html_end_box
 * Desc : This function will end a box.
 *--------------------------------------------------------------------------------*/
static void	html_end_box(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	write(draw_state->output_file,item_end,item_end_size);
	((HTML_DATA*)draw_state->format_state)->suppress_title = 0;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_header
 * @desc: This function will add a section header and separate the objects from
 *        each other.
 *--------------------------------------------------------------------------------*/
void	format_header(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	unsigned char			number[10];
	unsigned int			num_length;
	NAME					name = {NULL,0,0,0};
	HTML_SEQUENCE_DIAGRAM*	diagram;

	switch(input_state->state)
	{
		case TYPE_STATE_MACHINE:
			/* this is a boxed item - start the box */
			html_start_box(draw_state,input_state);

			/* only required for graphs */
			if ((draw_state->format_flags & OUTPUT_FORMAT_TABLE) == 0)
			{
				/* write the start of the state machine header to the file */
				write(draw_state->output_file,state_start,state_start_size);
				num_length = SizeToString(number,((HTML_DATA*)draw_state->format_state)->state_machine.state_machine_no,10);
				write(draw_state->output_file,&number[10-num_length],num_length);
				write(draw_state->output_file,sequence_end,sequence_end_size);

				/* now allocate the structure */
				html_add_state_machine(&((HTML_DATA*)draw_state->format_state)->state_machine,&name);
			}
			break;

		case TYPE_SEQUENCE_DIAGRAM:
			/* this is a boxed item - start the box */
			html_start_box(draw_state,input_state);

			/* only required for graphs */
			if ((draw_state->format_flags & OUTPUT_FORMAT_TABLE) == 0)
			{
				/* write the start of the sequence header to the file */
				write(draw_state->output_file,sequence_start,sequence_start_size);
				num_length = SizeToString(number,((HTML_DATA*)draw_state->format_state)->sequence_diagram.sequence_no,10);	
				write(draw_state->output_file,&number[10-num_length],num_length);
				write(draw_state->output_file,sequence_end,sequence_end_size);

				html_add_sequence(&((HTML_DATA*)draw_state->format_state)->sequence_diagram,&name);
			}
			break;

		case TYPE_CODE_BLOCK:
				/* this is a boxed item - start the box */
				html_start_box(draw_state,input_state);

				write(draw_state->output_file,pre_start,pre_start_size);
			break;

		case TYPE_QUOTE_BLOCK: 
				write(draw_state->output_file,blockquote_start,blockquote_start_size);
			break;
		
		case TYPE_TABLE:
			break;

		case TYPE_INDEX:
			/* increment the indexes */
			((HTML_DATA*)draw_state->format_state)->index_state.index_number++;
			((HTML_DATA*)draw_state->format_state)->index_state.index_number_size = IntToAlphaSafe(
									((HTML_DATA*)draw_state->format_state)->index_state.index_number,
									((HTML_DATA*)draw_state->format_state)->index_state.index_number_str,
									4);
			/* this is a boxed item - start the box */
			html_start_box(draw_state,input_state);
			break;

		case TYPE_API:
			/* this is a boxed item - start the box */
			html_start_box(draw_state,input_state);
			break;

		case TYPE_SAMPLE:
			html_start_box(draw_state,input_state);
			break;

		case TYPE_LIST:
			((HTML_DATA*)draw_state->format_state)->list_state.list_level = UINT_MAX;
			level_index_init(&draw_state->list_index);
			break;
	}
}


/*----- FUNCTION -----------------------------------------------------------------*
 * Name : html_list_level_up
 * Desc : This function will end the lists on the way up.
 *--------------------------------------------------------------------------------*/
static void	html_list_level_up(DRAW_STATE* draw_state, unsigned int new_level)
{
	unsigned int count;

	if (((HTML_DATA*)draw_state->format_state)->list_state.list_level != UINT_MAX)
	{
		for (count = ((HTML_DATA*)draw_state->format_state)->list_state.list_level; count > new_level; count--)
		{
			if (((HTML_DATA*)draw_state->format_state)->list_state.level_char[count] == '1')
			{
				write(draw_state->output_file,numb_list_end,numb_list_end_size);
			}
			else
			{
				write(draw_state->output_file,list_end,list_end_size);
			}
		}
	}

	((HTML_DATA*)draw_state->format_state)->list_state.list_level = new_level;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_footer
 * @desc: This is the opposite of the header and exists an item.
 *--------------------------------------------------------------------------------*/
void	format_footer(DRAW_STATE* draw_state, INPUT_STATE* input_state)
{
	NAME			empty = {NULL,0,0,0};
	unsigned int	count;

	switch(input_state->state)
	{
		case TYPE_STATE_MACHINE:
		{
			if ((draw_state->format_flags & OUTPUT_FORMAT_TABLE) == 0)
			{
				/* write the pre-formatted state machine (text) */
				state_machine_draw(((HTML_DATA*)draw_state->format_state)->state_machine.next->state_machine,draw_state);
	
				/* end the html */
				write(draw_state->output_file,sequence_finish,sequence_finish_size);
			}

			html_end_box(draw_state,input_state);
		}
		break;

		case TYPE_SEQUENCE_DIAGRAM:
		{
			if ((draw_state->format_flags & OUTPUT_FORMAT_TABLE) == 0)
			{
				write(draw_state->output_file,sequence_finish,sequence_finish_size);
			}

			html_end_box(draw_state,input_state);
		}
		break;

		case TYPE_CODE_BLOCK:
			write(draw_state->output_file,pre_end,pre_end_size);
			html_end_box(draw_state,input_state);
		break;

		case TYPE_QUOTE_BLOCK: 
			write(draw_state->output_file,blockquote_end,blockquote_end_size);
		break;

		case TYPE_TABLE:
			break;

		case TYPE_INDEX:
			html_end_box(draw_state,input_state);
			break;

		case TYPE_API:
			html_end_box(draw_state,input_state);
			break;

		case TYPE_SAMPLE:
			html_end_box(draw_state,input_state);
			break;

		case TYPE_LIST:
			if (((HTML_DATA*)draw_state->format_state)->list_state.list_level == UINT_MAX)
			{
				((HTML_DATA*)draw_state->format_state)->list_state.list_level = 1;
			}
			else
			{
				((HTML_DATA*)draw_state->format_state)->list_state.list_level++;
			}

			html_list_level_up(draw_state,0);
			level_index_release(&draw_state->list_index);
		break;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_raw
 * @desc: This function will output raw text. It will find the \n and replace 
 *        them with a ' '.
 *--------------------------------------------------------------------------------*/
void	format_raw(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size)
{
	write(draw_state->output_file,line_break,line_break_size);
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
			html_add_timeline(((HTML_DATA*)draw_state->format_state)->sequence_diagram.next,current);
		}

		current = current->next;
	}
		
	write(draw_state->output_file,draw_state->output_buffer,write_size);
	
	/* stop duplication of timelines when the bottom is called */
	((HTML_DATA*)draw_state->format_state)->sequence_diagram.next->timelines = 1;
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
		html_add_message(((HTML_DATA*)draw_state->format_state)->sequence_diagram.next,message);

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
 *  name: format_state_set_size
 *  desc: This function sets the size of the state machine graph.
 *--------------------------------------------------------------------------------*/
void	format_state_set_size(DRAW_STATE* draw_state, unsigned int nodes, unsigned int vertices)
{
	state_machine_set_size(((HTML_DATA*)draw_state->format_state)->state_machine.next->state_machine,nodes,vertices);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_state
 * @desc: This function handles the state creation from the main creation code.
 *--------------------------------------------------------------------------------*/
void	format_state(DRAW_STATE* draw_state,STATE* state, unsigned int x, unsigned int y)
{
	state_machine_add_node(((HTML_DATA*)draw_state->format_state)->state_machine.next->state_machine,state,x,y);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_transition
 * @desc: This function outputs the transition.
 *--------------------------------------------------------------------------------*/
void	format_transition(DRAW_STATE* draw_state,STATE* state, STATE_TRANSITION* transition, unsigned int from_id, unsigned int to_id)
{
	state_machine_add_vertex(((HTML_DATA*)draw_state->format_state)->state_machine.next->state_machine,transition,from_id,to_id);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_marker
 * @desc: 
 *--------------------------------------------------------------------------------*/
void	format_marker(DRAW_STATE* draw_state, unsigned int marker)
{
	unsigned int  size;
	unsigned char array[200];
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

		case (OUTPUT_MARKER_HEADER_START|OUTPUT_MARKER_NO_INDEX):
		case OUTPUT_MARKER_HEADER_START:
			if ((marker & OUTPUT_MARKER_NO_INDEX) == 0)
			{
				level_index_set_level(&draw_state->index,level);
			}
				
			size = level_index_gen_label(&draw_state->index,1,INDEX_STYLE_NUMBERS,array,200,'_');

			header_level_a_start[2] = ('0' + level);
			write(draw_state->output_file,header_level_a_start,header_level_a_start_size);
			write(draw_state->output_file,array,size);
			write(draw_state->output_file,header_level_a_end,header_level_a_end_size);
			break;

		case OUTPUT_MARKER_HEADER_END:
			header_level_end[3] = ('0' + level);
			write(draw_state->output_file,header_level_end,header_level_end_size);
			break;
		
		case OUTPUT_MARKER_LINE_BREAK:
			write(draw_state->output_file,line_break,line_break_size);
			draw_state->offset = 0;
			break;

		case OUTPUT_MARKER_PARAGRAPH_BREAK:
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
		write_escaped_html(draw_state->output_file,text->name,text->name_length,draw_state->no_space);
		draw_state->no_space = 0;
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
	header_level_start[2] = ('0' + header_level);
	write(draw_state->output_file,header_level_start,header_level_start_size);
	write_escaped_html(draw_state->output_file,name->name,name->name_length,draw_state->no_space);
	draw_state->no_space = 0;

	header_level_end[3] = ('0' + header_level);
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
		write_escaped_html(draw_state->output_file,section_data->name,section_data->name_length,draw_state->no_space);
		draw_state->no_space = 0;
	}
	write(draw_state->output_file,paragraph,paragraph_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: format_title
 * @desc: This function will display a title.
 *--------------------------------------------------------------------------------*/
void	format_title(DRAW_STATE* draw_state, unsigned int header_level, NAME* title, NAME* name)
{
	if (!((HTML_DATA*)draw_state->format_state)->suppress_title)
	{
		header_level &= MAX_HEADER_LEVEL_MASK;
		header_level_start[2] = ('0' + header_level);
		write(draw_state->output_file,header_level_start,header_level_start_size);

		write_escaped_html(draw_state->output_file,title->name,title->name_length,draw_state->no_space);
		draw_state->no_space = 0;

		if (name != NULL)
		{
			write(draw_state->output_file,space,space_size);
			write_escaped_html(draw_state->output_file,name->name,name->name_length,draw_state->no_space);
			draw_state->no_space = 0;
		}

		header_level_end[3] = ('0' + header_level);
		write(draw_state->output_file,header_level_end,header_level_end_size);
	}
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
	if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_PROTOTYPE)
	{
		write(draw_state->output_file,table_start_proto,table_start_proto_size);
	}
	else if ((table_layout->table_flags & OUTPUT_TABLE_FORMAT_BOXED) && (table_layout->table_flags & OUTPUT_TABLE_FORMAT_FULL_WIDTH))
	{
		write(draw_state->output_file,table_start_fb,table_start_fb_size);
	}
	else if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_BOXED)
	{
		write(draw_state->output_file,table_start_box,table_start_box_size);
	}
	else if (table_layout->table_flags & OUTPUT_TABLE_FORMAT_FULL_WIDTH)
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

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : format_index_chapter
 * Desc : This index chapter.
 *--------------------------------------------------------------------------------*/
void	format_index_chapter(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : format_index_start_sublevel
 * Desc : The start sub-level
 *--------------------------------------------------------------------------------*/
void	format_index_start_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
	unsigned char	temp[200];
	unsigned int	start;

	start = index_generate_label(index_item,0,INDEX_STYLE_NUMBERS,temp,200,'_');

	write(draw_state->output_file,index_down_part1,index_down_part1_size);
	write(draw_state->output_file,
			((HTML_DATA*)draw_state->format_state)->index_state.index_number_str,
			((HTML_DATA*)draw_state->format_state)->index_state.index_number_size);
	write(draw_state->output_file,&temp[start],200-start);
	write(draw_state->output_file,index_down_part2,index_down_part2_size);
	write(draw_state->output_file,&temp[start],200-start);
	write(draw_state->output_file,index_down_part3,index_down_part3_size);
	write(draw_state->output_file,index_item->section_title.name,index_item->section_title.name_length);
	write(draw_state->output_file,index_down_part4,index_down_part4_size);

	write(draw_state->output_file,index_toggle_part1,index_toggle_part1_size);
	write(draw_state->output_file,
			((HTML_DATA*)draw_state->format_state)->index_state.index_number_str,
			((HTML_DATA*)draw_state->format_state)->index_state.index_number_size);
	write(draw_state->output_file,&temp[start],200-start);
	write(draw_state->output_file,index_toggle_part2,index_toggle_part2_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : format_index_entry
 * Desc : The same level.
 *--------------------------------------------------------------------------------*/
void	format_index_entry(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
	unsigned char	temp[200];
	unsigned int	start;

	start = index_generate_label(index_item,0,INDEX_STYLE_NUMBERS,temp,200,'_');

	write(draw_state->output_file,index_item_part1,index_item_part1_size);
	write(draw_state->output_file,&temp[start],200-start);
	write(draw_state->output_file,index_item_part2,index_item_part2_size);
	write(draw_state->output_file,index_item->section_title.name,index_item->section_title.name_length);
	write(draw_state->output_file,index_item_part3,index_item_part3_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : format_index_end_sublevel
 * Desc : end the sub level.
 *--------------------------------------------------------------------------------*/
void	format_index_end_sublevel(DRAW_STATE* draw_state, DOC_SECTION* index_item)
{
	write(draw_state->output_file,"</div>",6);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : write_list_start
 * Desc : This function will simply write the list start.
 *--------------------------------------------------------------------------------*/
static void	write_list_start(DRAW_STATE* draw_state, unsigned char marker)
{
	switch(marker)
	{
		case '1':	write(draw_state->output_file,numb_list_start,numb_list_start_size);	break;
		case '-':	write(draw_state->output_file,dash_list_start,dash_list_start_size);	break;
		case '+':	write(draw_state->output_file,plus_list_start,plus_list_start_size);	break;
		case '*':	write(draw_state->output_file,asti_list_start,asti_list_start_size);	break;
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: format_list_start
 * desc: The format output essentially does nothing.
 *--------------------------------------------------------------------------------*/
void	format_list_item_start(DRAW_STATE* draw_state, unsigned int level, unsigned char marker)
{
	if (((HTML_DATA*)draw_state->format_state)->list_state.list_level > level)
	{
		/* reducing level(s) so end the lists on the way */
		if (((HTML_DATA*)draw_state->format_state)->list_state.list_level == UINT_MAX)
		{
			write_list_start(draw_state,marker);
			((HTML_DATA*)draw_state->format_state)->list_state.level_char[level] = marker;
		}
		else
		{
			html_list_level_up(draw_state,level);
		}
	}
	else if (((HTML_DATA*)draw_state->format_state)->list_state.list_level < level)
	{
		/* going up a level */
		write_list_start(draw_state,marker);
		((HTML_DATA*)draw_state->format_state)->list_state.level_char[level] = marker;
	}
	else if (level == 0)
	{
		((HTML_DATA*)draw_state->format_state)->list_state.level_char[1] = marker;
	}
		
	/* write the new list start and set the level */
	((HTML_DATA*)draw_state->format_state)->list_state.list_level = level;
	level_index_set_level(&draw_state->list_index,level);
	write(draw_state->output_file,list_item_start,list_item_start_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: format_list_numeric_start
 * desc: The format output essentially does nothing.
 *--------------------------------------------------------------------------------*/
void	format_list_numeric_start(DRAW_STATE* draw_state, unsigned int level)
{
	if (((HTML_DATA*)draw_state->format_state)->list_state.list_level > level)
	{
		/* reducing level(s) so end the lists on the way */
		if (((HTML_DATA*)draw_state->format_state)->list_state.list_level == UINT_MAX)
		{
			write_list_start(draw_state,'1');
			((HTML_DATA*)draw_state->format_state)->list_state.level_char[level] = '1';
		}
		else
		{
			html_list_level_up(draw_state,level);
		}
	}
	else if (((HTML_DATA*)draw_state->format_state)->list_state.list_level < level)
	{
		/* going up a level */
		write_list_start(draw_state,'1');
		((HTML_DATA*)draw_state->format_state)->list_state.level_char[level] = '1';
	}
	else if (level == 0)
	{
		((HTML_DATA*)draw_state->format_state)->list_state.level_char[1] = '1';
	}
	
	/* write the new list start and set the level */
	((HTML_DATA*)draw_state->format_state)->list_state.list_level = level;
	level_index_set_level(&draw_state->list_index,level);
	write(draw_state->output_file,list_item_start,list_item_start_size);
}

/*----- FUNCTION -----------------------------------------------------------------*
 * name: format_list_end
 * desc: The format output essentially does nothing.
 *--------------------------------------------------------------------------------*/
void	format_list_end(DRAW_STATE* draw_state)
{
	write(draw_state->output_file,list_item_end,list_item_end_size);
}

