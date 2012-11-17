/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: output_format
 *   @desc: This file holds the definitions for the output formats.
 *
 * @author: pantoine
 *   @date: 06/07/2012 06:39:11
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __OUTPUT_FORMAT_H__
#define __OUTPUT_FORMAT_H__

#include "level_index.h"
#include "document_generator.h"
#include "lookup_tables.h"

typedef struct tag_output_format	OUTPUT_FORMAT;

OUTPUT_FORMAT*	output_find_format(unsigned char* name, unsigned int name_length);

/*--------------------------------------------------------------------------------*
 * Output format specifiers
 *--------------------------------------------------------------------------------*/
typedef enum
{
	OHL_NO_LEVEL,
	OHL_LEVEL_1,
	OHL_LEVEL_2,
	OHL_LEVEL_3,
	OHL_LEVEL_4,
	OHL_MAX_LEVELS

} OUTPUT_HEADER_LEVELS;

#define MAX_HEADER_LEVEL_MASK	0x00000003

/* output text styles - bit fields */
#define	OUTPUT_TEXT_STYLE_NORMAL				((unsigned int) 0x00000000)
#define	OUTPUT_TEXT_STYLE_BOLD					((unsigned int) 0x00000001)
#define	OUTPUT_TEXT_STYLE_ITALIC				((unsigned int) 0x00000002)
#define	OUTPUT_TEXT_STYLE_CODE					((unsigned int) 0x00000004)
#define OUTPUT_TEXT_STYLE_ASCII_ART				((unsigned int) 0x00000008)		/* the following may contain weird chars */
#define	OUTPUT_TEXT_STYLE_SPACED				((unsigned int) 0x00000010)		/* add space before and after text */
#define	OUTPUT_TEXT_STYLE_L_SPACE				((unsigned int) 0x00000020)		/* add leading space */
#define	OUTPUT_TEXT_STYLE_T_SPACE				((unsigned int) 0x00000040)		/* add trailing space */
#define	OUTPUT_TEXT_STYLE_L_NEWLINE				((unsigned int) 0x00000100)		/* add leading new line */
#define	OUTPUT_TEXT_STYLE_T_NEWLINE				((unsigned int) 0x00000200)		/* add trailing new line */

/* output format flags */
#define OUTPUT_FORMAT_TABLE						((unsigned int) 0x00000001)		/* the item will be output as a table */
#define OUTPUT_FORMAT_TEXT						((unsigned int)	0x00000002)		/* the item will be output using text */
#define OUTPUT_FORMAT_INLINE					((unsigned int)	0x00000004)		/* the item will be output inline */
#define OUTPUT_FORMAT_PAGED						((unsigned int)	0x00000008)		/* the item will be output with a page break after */
#define OUTPUT_FORMAT_REFERENCE					((unsigned int)	0x00000010)		/* the item will be output a reference to the item (if supported) */
#define OUTPUT_FORMAT_NUMBERED					((unsigned int)	0x00000020)		/* the item will be output numbered */
#define OUTPUT_FORMAT_LINED						((unsigned int)	0x00000040)		/* the item will be output lined (normally for index's) */
#define OUTPUT_FORMAT_INDEX						((unsigned int)	0x00000080)		/* the item will be output is in index form */
#define OUTPUT_FORMAT_LEVEL						((unsigned int)	0x00000100)		/* the item will be output index start at this level */
#define OUTPUT_FORMAT_FLAT						((unsigned int)	0x00000200)		/* the item will be output without level steps */
#define OUTPUT_FORMAT_CBREAK					((unsigned int)	0x00000400)		/* the item will be output will have chapter breaks */

#define OUTPUT_FORMAT_INDEX_ONLY				((unsigned int)	0x80000000)		/* special flag, that lets the object know it is part of an index run */

/* output markers - note low bits are used for counts */
#define OUTPUT_MARKER_MASK						((unsigned int) 0xffff0000)		/* the mask for the significant bits of the marker */

#define OUTPUT_MARKER_CODE_START				((unsigned int) 0x00001000)		/* code text style start */
#define OUTPUT_MARKER_CODE_END					((unsigned int) 0x00002000)		/* code text style end */
#define OUTPUT_MARKER_BLOCK_START				((unsigned int) 0x00004000)		/* code block start */
#define OUTPUT_MARKER_BLOCK_END					((unsigned int) 0x00008000)		/* code block end */
#define OUTPUT_MARKER_QUOTE_START				((unsigned int) 0x00010000)		/* quote block start */
#define OUTPUT_MARKER_QUOTE_END					((unsigned int) 0x00020000)		/* quote block end */
#define OUTPUT_MARKER_LINE_BREAK				((unsigned int) 0x00040000)		/* line break */
#define OUTPUT_MARKER_PARAGRAPH_BREAK			((unsigned int) 0x00080000)		/* paragraph break */
#define OUTPUT_MARKER_EMP_START					((unsigned int) 0x00100000)		/* emphasis start: level in the low byte */
#define OUTPUT_MARKER_EMP_END					((unsigned int) 0x00200000)		/* emphasis end */
#define OUTPUT_MARKER_HEADER_START				((unsigned int) 0x00400000)		/* header start: level in the low byte */
#define OUTPUT_MARKER_HEADER_END				((unsigned int) 0x00800000)		/* header end: level in low byte  */
#define OUTPUT_MARKER_ASCII_CHAR				((unsigned int) 0x01000000)		/* writes an ascii char to the output - low 7 bits */
#define OUTPUT_MARKER_NO_INDEX					((unsigned int) 0x02000000)		/* do not index this marker */

/* table format flags */
#define OUTPUT_TABLE_FORMAT_HEADER				((unsigned int) 0x00000001)		/* This table has a header */
#define OUTPUT_TABLE_FORMAT_BOXED				((unsigned int) 0x00000002)		/* The table should be boxed */
#define OUTPUT_TABLE_FORMAT_FULL_WIDTH			((unsigned int) 0x00000004)		/* The table should be space the whole page */
#define OUTPUT_TABLE_FORMAT_PROTOTYPE			((unsigned int) 0x00000008)		/* The table should aligned to the left without borders */

/* table column format flags */
#define OUTPUT_COLUMN_FORMAT_HARD_WRAP			((unsigned int) 0x00000001)		/* the column will wrap but at the column boundary */
#define	OUTPUT_COLUMN_FORMAT_WORD_WRAP			((unsigned int) 0x00000002)		/* the column will clip to the last whole word */
#define	OUTPUT_COLUMN_FORMAT_WRAP_COMPLEX		((unsigned int) 0x00000004)		/* the column uses the data to select complex wrapping */
#define	OUTPUT_COLUMN_FORMAT_NO_TRUNCATE		((unsigned int) 0x00000008)		/* the column should no be made smaller (unless any columns can't fit) */
#define	OUTPUT_COLUMN_FORMAT_LINE_CLIP			((unsigned int) 0x00000010)		/* clip per line (using 0x0a or 0x0d as line endings) */
#define	OUTPUT_COLUMN_FORMAT_NO_PARAGRAPH		((unsigned int) 0x00000020)		/* don't add newlines at the start and end */
#define	OUTPUT_COLUMN_FORMAT_LEFT_JUSTIFIED		((unsigned int) 0x00000040)		/* the column is filled to the left */
#define	OUTPUT_COLUMN_FORMAT_RIGHT_JUSTIFIED	((unsigned int) 0x00000080)		/* the column is filled to the right */

/*--------------------------------------------------------------------------------*
 * Structures for output flags
 *--------------------------------------------------------------------------------*/
#define	OUTPUT_FLAG_TYPE_INVALID	((unsigned int) 0x00000000)		/* invalid type */
#define	OUTPUT_FLAG_TYPE_BOOLEAN	((unsigned int) 0x00000001)		/* the flag expects a true/false value (and converts to 1/0) */
#define OUTPUT_FLAG_TYPE_STRING		((unsigned int) 0x00000002)		/* the flag expects a string value */
#define OUTPUT_FLAG_TYPE_NUMBER		((unsigned int) 0x00000004)		/* the flag expects a numeric value to be passed in */
#define OUTPUT_FLAG_TYPE_REAL		((unsigned int) 0x00000008)		/* the flag expects a floating point numeric value to be passed in */

typedef struct
{
	unsigned char*	name;
	unsigned int	id;
	unsigned int	hash;
	unsigned int	name_length;
	unsigned int	type;

} OUTPUT_FLAG;

/* the flag list for a type */
typedef struct
{
	unsigned int	num_flags;
	OUTPUT_FLAG*	flag_list;

} OUTPUT_FLAG_LIST;

typedef struct
{
	unsigned int	type;
	unsigned int	id;

	union
	{
		unsigned char	boolean;
		int				number;
		float			real;
		NAME			string;
	} value;

} OUTPUT_FLAG_VALUE;

/*--------------------------------------------------------------------------------*
 * Structure for the tables.
 *--------------------------------------------------------------------------------*/
#define MAX_TABLE_SIZE	(10)

typedef struct
{
	unsigned int	flags;
	unsigned int	offset;
	unsigned int	width;
	unsigned int	style;
} COLUMN;

typedef struct
{
	unsigned int	valid;
	unsigned int	table_flags;
	unsigned int	num_columns;
	unsigned int	column_spacing;
	COLUMN			column[MAX_TABLE_SIZE];
} TABLE_LAYOUT;

typedef struct
{
	NAME*			row[MAX_TABLE_SIZE];
	unsigned int	copied[MAX_TABLE_SIZE];
	unsigned int	remaining[MAX_TABLE_SIZE];
} TABLE_ROW;

/*--------------------------------------------------------------------------------*
 * Structures to holds the drawing state.
 *--------------------------------------------------------------------------------*/

typedef struct
{
	unsigned short	num_columns;
	unsigned short	column[MAX_OUTPUT_COLUMNS];
	unsigned short	window_start;
	unsigned short	window_end;
	unsigned short	first_column;
	unsigned short	last_column;
	TIMELINE*		start_timeline;
	TIMELINE*		end_timeline;
	void*			data;

} SEQUENCE_DRAW_STATE;

typedef struct
{
	unsigned int	num_nodes;
	unsigned int	num_vertices;
	void*			data;

} STATE_MACHINE_DRAW_STATE;

typedef struct tag_draw_state
{
	int				output_file;
	unsigned int	offset;
	unsigned int	buffer_size;
	unsigned int	path_length;
	unsigned int	page_width;				/* this is a type dependant page width */
	unsigned int	margin_width;
	unsigned int	max_constant_size;		/* this is the maximum size of a constant for a table view */
	unsigned int	level_start;			/* the level to start the index at */
	unsigned int	format_flags;
	unsigned int	no_space;				/* this is a bit of a hack - continuations that must not have a leading space */
	unsigned int	number_level;			/* what level to end numbering at */
	unsigned int	chapter;				/* the number of the chapter(file) that the output is on */
	unsigned int	global_margin_width;	/* this used to reset the page width after every item */
	unsigned int	global_format_flags;	/* this is used to reset the flags after every item */
	unsigned int	global_number_level;	/* this is used to reset number level after every item */
	unsigned int	global_max_constant;	/* this is used to reset the max_constant width */
	unsigned char	path[MAX_FILENAME];
	NAME			number_style;			/* the style that the number will be laid out in */
	NAME			resources_path;			/* the path to the resources */
	unsigned char*	buffer;
	unsigned char*	output_buffer;			/* random buffer controlled by the type code */
	unsigned char*	resource_path;			/* the path to the resources */
	OUTPUT_FORMAT*	format;
	void*			format_state;			/* data used by the formats */
	LEVEL_INDEX		index;					/* structure for handling the levels of the document index */
	LEVEL_INDEX		list_index;				/* index used for *text* lists */
	GROUP*			model;					/* the model that is being rendered */
	LOOKUP_LIST		macro_lookup;			/* the macros that are added to the system */

	union
	{
		SEQUENCE_DRAW_STATE			sequence;
		STATE_MACHINE_DRAW_STATE	state_machine;
	}
	data;
} DRAW_STATE;

/*--------------------------------------------------------------------------------*
 * Function Pointer Types for the output functions.
 *--------------------------------------------------------------------------------*/
typedef unsigned int	(*OUTPUT_DECODE_FLAGS_FUNCTION)			(DRAW_STATE* draw_state,INPUT_STATE* input_state, unsigned hash,NAME* value);

typedef unsigned int	(*OUTPUT_OPEN_FUNCTION)					(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned char* name, unsigned int name_length);
typedef void			(*OUTPUT_CLOSE_FUNCTION)				(DRAW_STATE* draw_state, INPUT_STATE* input_state);
typedef void			(*OUTPUT_HEADER_FUNCTION)				(DRAW_STATE* draw_state, INPUT_STATE* input_state);
typedef void			(*OUTPUT_FOOTER_FUNCTION)				(DRAW_STATE* draw_state, INPUT_STATE* input_state);
typedef void			(*OUTPUT_RAW_FUNCTION)					(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size);
typedef void			(*OUTPUT_SAMPLE_FUNCTION)				(DRAW_STATE* draw_state, SAMPLE* sample);

typedef void			(*OUTPUT_TIMELINES_FUNCTION)			(DRAW_STATE* draw_state);
typedef void			(*OUTPUT_MESSAGE_FUNCTION)				(DRAW_STATE* draw_state, MESSAGE* message);

typedef void			(*OUTPUT_STATE_SET_SIZE_FUNCTION)		(DRAW_STATE* draw_state, unsigned int nodes, unsigned int vertices);
typedef void			(*OUTPUT_STATE_FUNCTION)				(DRAW_STATE* draw_state,STATE* state, unsigned int x, unsigned int y);
typedef void			(*OUTPUT_TRANSITION_FUNCTION)			(	DRAW_STATE* draw_state,
																	STATE* state, 
																	STATE_TRANSITION* transition,
																	unsigned int from_id,
																	unsigned int to_id);

typedef void			(*OUTPUT_MARKER)						(DRAW_STATE* draw_state, unsigned int marker);
typedef void			(*OUTPUT_TEXT_FUNCTION)					(DRAW_STATE* draw_state, unsigned int text_style, NAME* text);
typedef void			(*OUTPUT_SECTION_FUNCTION)				(	DRAW_STATE*		draw_state, 
																	unsigned int	header_level, 
																	NAME*			name,
																	unsigned int	format,
																	NAME*			section_data);
typedef void			(*OUTPUT_TITLE_FUNCTION)				(DRAW_STATE* draw_state, unsigned int header_level, NAME* title, NAME* name);
typedef void			(*OUTPUT_BLOCK_FUNCTION)				(DRAW_STATE* draw_state, unsigned int format, NAME* block);

typedef void			(*OUTPUT_TABLE_START_FUNCTION)			(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout);
typedef void			(*OUTPUT_TABLE_HEADER_FUNCTION)			(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row);
typedef void			(*OUTPUT_TABLE_ROW_FUNCTION)			(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row);
typedef void			(*OUTPUT_TABLE_END_FUNCTION)			(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout);

typedef void			(*OUTPUT_INDEX_CHAPTER_FUNCTION)		(DRAW_STATE* draw_state, DOC_SECTION* index_item);
typedef void			(*OUTPUT_INDEX_START_SUBLEVEL_FUNCTION)	(DRAW_STATE* draw_state, DOC_SECTION* index_item);
typedef void			(*OUTPUT_INDEX_ENTRY_FUNCTION)			(DRAW_STATE* draw_state, DOC_SECTION* index_item);
typedef void			(*OUTPUT_INDEX_END_SUBLEVEL_FUNCTION)	(DRAW_STATE* draw_state, DOC_SECTION* index_item);

typedef void			(*OUTPUT_LIST_ITEM_START_FUNCTION)		(DRAW_STATE* draw_state, unsigned int level, unsigned char marker);
typedef void			(*OUTPUT_LIST_NUMERIC_START_FUNCTION)	(DRAW_STATE* draw_state, unsigned int level);
typedef void			(*OUTPUT_LIST_END_FUNCTION)				(DRAW_STATE* draw_state);

struct tag_output_format
{
	unsigned char*				name;
	unsigned int				name_length;

	/* function for decoding the type specific flags */
	OUTPUT_DECODE_FLAGS_FUNCTION	decode_flags;
	
	/* generic functions called for all diagrams */
	OUTPUT_OPEN_FUNCTION		output_open;
	OUTPUT_CLOSE_FUNCTION		output_close;
	OUTPUT_HEADER_FUNCTION		output_header;	
	OUTPUT_FOOTER_FUNCTION		output_footer;
	OUTPUT_RAW_FUNCTION			output_raw;
	OUTPUT_SAMPLE_FUNCTION		output_sample;

	/* sequence diagram functions */
	OUTPUT_TIMELINES_FUNCTION	output_timelines;
	OUTPUT_MESSAGE_FUNCTION		output_message;

	/* state machine functions */
	OUTPUT_STATE_SET_SIZE_FUNCTION	output_state_set_size;
	OUTPUT_STATE_FUNCTION			output_state;
	OUTPUT_TRANSITION_FUNCTION		output_transition;

	/* generic output functions */
	OUTPUT_MARKER							output_marker;
	OUTPUT_TEXT_FUNCTION					output_text;

	OUTPUT_SECTION_FUNCTION					output_section;
	OUTPUT_TITLE_FUNCTION					output_title;
	OUTPUT_BLOCK_FUNCTION					output_block;

	OUTPUT_TABLE_START_FUNCTION				output_table_start;
	OUTPUT_TABLE_HEADER_FUNCTION			output_table_header;
	OUTPUT_TABLE_ROW_FUNCTION				output_table_row;
	OUTPUT_TABLE_END_FUNCTION				output_table_end;

	/* index writing functions */
	OUTPUT_INDEX_CHAPTER_FUNCTION			output_index_chapter;
	OUTPUT_INDEX_START_SUBLEVEL_FUNCTION	output_index_start_sublevel;
	OUTPUT_INDEX_ENTRY_FUNCTION				output_index_entry;
	OUTPUT_INDEX_END_SUBLEVEL_FUNCTION		output_index_end_sublevel;

	/* list writing functions */
	OUTPUT_LIST_ITEM_START_FUNCTION			output_list_item_start;
	OUTPUT_LIST_NUMERIC_START_FUNCTION		output_list_numeric_start;
	OUTPUT_LIST_END_FUNCTION				output_list_end;

	struct tag_output_format*	next;
};

/*--------------------------------------------------------------------------------*
 * static definitions of function for plugins.
 *--------------------------------------------------------------------------------*/
static unsigned int	format_decode_flags			(DRAW_STATE* draw_state,INPUT_STATE* input_state, unsigned hash, NAME* value);

static unsigned int	format_open					(DRAW_STATE* draw_state, INPUT_STATE* input_state, unsigned char* name, unsigned int name_length);
static void			format_close				(DRAW_STATE* draw_state, INPUT_STATE* input_state);
static void			format_header				(DRAW_STATE* draw_state, INPUT_STATE* input_state);
static void			format_footer				(DRAW_STATE* draw_state, INPUT_STATE* input_state);
static void			format_raw					(DRAW_STATE* draw_state, unsigned char* buffer, unsigned int buffer_size);
static void			format_sample				(DRAW_STATE* draw_state, SAMPLE* sample);

static void			format_timelines			(DRAW_STATE* draw_state);
static void			format_message				(DRAW_STATE* draw_state, MESSAGE* message);

static void			format_state_set_size		(DRAW_STATE* draw_state, unsigned int nodes, unsigned int vertices);
static void			format_state				(DRAW_STATE* draw_state,STATE* state, unsigned int x, unsigned int y);
static void			format_transition			(	DRAW_STATE* draw_state,
													STATE* state,
													STATE_TRANSITION* transition,
													unsigned int from_id,
													unsigned int to_id);

static	void		format_api_prototype		(DRAW_STATE* draw_state, API_FUNCTION* function);

static	void		format_marker				(DRAW_STATE* draw_state, unsigned int marker);

static	void		format_text					(DRAW_STATE* draw_state, unsigned int text_style, NAME* text);

static	void		format_section				(	DRAW_STATE*		draw_state, 
													unsigned int	header_level,	
													NAME*			name,
													unsigned int	format,
													NAME*			section_data);
static	void		format_title				(DRAW_STATE* draw_state, unsigned int header_level, NAME* title, NAME* name);
static	void		format_block				(DRAW_STATE* draw_state, unsigned int format, NAME* block);

static	void		format_table_start			(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout);
static	void		format_table_header			(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row);
static	void		format_table_row			(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout, TABLE_ROW* table_row);
static	void		format_table_end			(DRAW_STATE* draw_state, TABLE_LAYOUT* table_layout);

static void			format_index_chapter		(DRAW_STATE* draw_state, DOC_SECTION* index_item);
static void			format_index_start_sublevel	(DRAW_STATE* draw_state, DOC_SECTION* index_item);
static void			format_index_entry			(DRAW_STATE* draw_state, DOC_SECTION* index_item);
static void			format_index_end_sublevel	(DRAW_STATE* draw_state, DOC_SECTION* index_item);

static void			format_list_item_start		(DRAW_STATE* draw_state, unsigned int level, unsigned char marker);
static void			format_list_numeric_start	(DRAW_STATE* draw_state, unsigned int level);
static void			format_list_end				(DRAW_STATE* draw_state);


#endif
