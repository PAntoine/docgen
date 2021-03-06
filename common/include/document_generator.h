/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : document_generator
 * Desc  : This file holds all the definitions for document generator.
 *
 * Author: pantoine
 * Date  : 07/11/2011 08:40:14
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __DOCUMENT_GENERATOR_H__
#define __DOCUMENT_GENERATOR_H__

#include "atoms.h"
#include "document_generator.h"
#include <limits.h>

#define VERSION_MAJOR		(0x01)
#define	VERSION_MINOR		(0x00)

#define	INVALID_ITEM		(UINT_MAX)
#define DEFAULT_GROUP		(0xFFFF)
#define MAX_NAME_LENGTH		(255)
#define MAX_OUTPUT_COLUMNS	(255)

#define MAX_RECORD_SIZE		(USHRT_MAX)
#define MAX_GROUPS_PER_FILE	(0x7fff)

#define MAX_FILENAME		((unsigned int)1024)

#define FILE_BLOCK_SIZE		(16 * 1024)

#define MAX_NUMBER_LENGTH	(30)

/* note: if you make this bigger the 255 then the output_synopsis function will have to change */
#define MAX_NUM_OPTIONS		(255)

/*--------------------------------------------------------------------------------*
 * Forward definitions.
 *--------------------------------------------------------------------------------*/
typedef struct tag_timeline TIMELINE;
typedef struct tag_state STATE;
typedef struct tag_group GROUP;
typedef struct tag_node NODE;
typedef struct tag_function FUNCTION;

/*--------------------------------------------------------------------------------*
 * File header format
 *--------------------------------------------------------------------------------*/
#define COMPILED_SOURCE_MAGIC	{0x47,0x44,0x53,0x43}
#define LINKED_SOURCE_MAGIC		{0x47,0x44,0x53,0x4c}

#define FILE_MAGIC			(0)
#define FILE_VERSION_MAJOR	(4)
#define FILE_VERSION_MINOR	(5)
#define FILE_DATE			(6)
#define FILE_DAY_OFF		(6)
#define FILE_MONTH_OFF		(7)
#define FILE_YEAR_OFF		(8)
#define FILE_HOUR_OFF		(9)
#define FILE_MINUTE_OFF		(10)
#define FILE_SECONDS_OFF	(11)
#define FILE_NUMBER_RECORDS	(12)
#define FILE_NAME_LENGTH	(14)
#define FILE_NAME_START		(16)
#define FILE_HEADER_SIZE	(18)

/*--------------------------------------------------------------------------------*
 * general file format
 *--------------------------------------------------------------------------------*/
/* record types */
#define INTERMEDIATE_RECORD_EOF			( 0)
#define INTERMEDIATE_RECORD_GROUP		( 1)
#define INTERMEDIATE_RECORD_API			( 2)
#define INTERMEDIATE_RECORD_FUNCTION	( 3)
#define INTERMEDIATE_RECORD_NAME		( 4)
#define INTERMEDIATE_RECORD_STRING		( 5)
#define INTERMEDIATE_RECORD_NUMBERIC	( 6)
#define INTERMEDIATE_RECORD_EMPTY		( 7)
#define INTERMEDIATE_RECORD_MULTILINE	( 8)
#define INTERMEDIATE_RECORD_TYPE		( 9)
#define INTERMEDIATE_RECORD_PAIR		(10)
#define INTERMEDIATE_RECORD_START		(11)
#define INTERMEDIATE_RECORD_END			(12)
#define INTERMEDIATE_RECORD_SAMPLE		(13)
#define INTERMEDIATE_RECORD_BOOLEAN		(14)
#define INTERMEDIATE_RECORD_APPLICATION	(15)

/* record format */
#define RECORD_TYPE			(0)
#define RECORD_ATOM			(1)
#define RECORD_GROUP		(2)		/* group and api and function and application share the same field */ 
#define RECORD_FUNCTION		(2) 	/* group and api and function and application share the same field */ 
#define RECORD_API			(2) 	/* group and api and function and application share the same field */
#define RECORD_APPLICATION	(2) 	/* group and api and function and application share the same field */
#define RECORD_BLOCK_NUM	(4) 
#define RECORD_LINE_NUM		(6)
#define RECORD_DATA_SIZE	(8) 
#define RECORD_DATA_START	(10) 

#define RECORD_FUNC_API_MASK	(0xC000)
#define RECORD_FUNCTION_FLAG	(0xC000)
#define RECORD_API_FLAG			(0x8000)

#define RECORD_GROUP_TYPE		(0x01)	/* The record group contains a single type */
#define RECORD_GROUP_RECORD		(0x02)	/* The record group contains a record structure */
#define RECORD_GROUP_CONSTANT	(0x03)	/* The record group contains a constant */

/*--------------------------------------------------------------------------------*
 * linker file format
 *--------------------------------------------------------------------------------*/
#define LINKER_VERSION_MAJOR_OFF	(4)
#define LINKER_VERSION_MINOR_OFF	(5)
#define LINKER_DAY_OFF				(6)
#define LINKER_MONTH_OFF			(7)
#define LINKER_YEAR_OFF				(8)
#define LINKER_HOUR_OFF				(9)
#define LINKER_MINUTE_OFF			(10)
#define LINKER_SECONDS_OFF			(11)
#define LINKER_NUMBER_RECORDS_OFF	(12)
#define LINKER_NAME_START_OFF		(16)
#define LINKER_HEADER_SIZE			(18)

/* record layout */
#define LINKER_RECORD_TYPE			(0)
#define LINKER_RECORD_SIZE			(1)
#define LINKER_RECORD_DATA_START	(3)

#define LINKER_MAX_RECORD_SIZE		(512)

/* record definitions:
 *  all data items are size,bytes for strings. BYTE = 1 byte, SHORT = 2 bytes, and
 *  LONG = 4 bytes.
 *
 *	linker_end					= {}
 *  linker_trigger				= {string:name}
 *  linker_triggers				= {string:name}
 *	linker_condition			= {string:name}
 *	linker_function				= {string:name}
 *	linker_source_file			= {string:name}
 *	linker_source_ref			= {short:file_number,long:line_number}
 *	linker_group				= {string:name}
 *	linker_state_machine_start	= {string:name}
 *	linker_state				= {string:name}
 *	linker_transition			= {short:group_id,short:to_state,string:name,short:trigger,<trigger_id>,short:num_triggers,<trigger_id>}
 *	linker_sequence_start		= {}
 *	linker_timeline				= {string:name}
 *	linker_message				= {string:message,short:to_node,byte:num_parameters}
 *	linker_parameter			= {string:name,byte:type,string:value}
 *	linker_node_start			= {}
 *	linker_sent_message			= {short:message_id,short:to_node}
 *	linker_received_message		= {short:message_id,short:from_node}
 *	linker_api_start			= {string:name}
 *	linker_api_function			= {string:return_type,string:name,string:brief}
 *	linker_api_description		= {string:description}
 *	linker_api_parameter		= {string:type,string:name,string:brief}
 *	linker_api_function_end		= {}
 *	linker_api_end				= {}
 *	linker_sample				= {string:name,string:sample}
 *	linker_application_start	= {string:name}
 *	linker_application_section	= {string:name,long_string:details}
 *	linker_application_option	= {short:flags,string:name,string:value,string:description}
 *	linker_application_synopsis	= {string:name,byte_list:(option_index)}
 *	linker_application_end		= {}
 */

#define LINKER_TRIGGER					( 0)	/* defines a trigger */
#define LINKER_TRIGGERS					( 1)	/* define a triggers */
#define LINKER_CONDITION				( 2)	/* defines a condition */
#define LINKER_SOURCE_FILE				( 3)	/* defines a source file reference */
#define LINKER_SOURCE_REFERENCE			( 4)	/* defines a source line reference */
#define	LINKER_END						( 5)	/* the end of a list of items */
#define LINKER_BLOCK_END				( 6)	/* the end of a block - NULL record */

#define LINKER_GROUP					( 6)	/* defines A group name */
#define LINKER_STATE_MACHINE_START		( 8)	/* defines the start of a state machine */
#define LINKER_STATE					( 9)	/* the start of a state list */
#define LINKER_TRANSITON				(10)	/* defines a transition between two states */
#define LINKER_STATE_MACHINE_END		(11)	/* denotes the end of a state machine */
#define LINKER_SEQUENCE_START			(12)	/* denotes the start of a sequence diagram */
#define	LINKER_TIMELINE					(13)	/* defines a timeline */
#define LINKER_FUNCTION					(14)	/* a function name definition */
#define LINKER_MESSAGE					(15)	/* defines a message */
#define LINKER_PARAMETER				(16)	/* denotes the start of a sequence diagram */
#define LINKER_NODE_START				(17)	/* node start */
#define LINKER_SENT_MESSAGE				(18)	/* references a message that is being send from this node */
#define LINKER_NODE_END					(19)	/* node start */
#define LINKER_SEQUENCE_END				(20)	/* denotes the end of a sequence diagram */
#define	LINKER_API_START				(21)	/* the start of the api */
#define	LINKER_API_FUNCTION				(22)	/* the function prototype */
#define	LINKER_API_ACTION				(23)	/* the actions for the function */
#define	LINKER_API_DESCRIPTION			(24)	/* the description of the function */
#define	LINKER_API_PARAMETER			(25)	/* a parameter to the api */
#define	LINKER_API_RETURNS				(26)	/* a return value for the api */
#define	LINKER_API_FUNCTION_END			(27)	/* the end of the function */
#define LINKER_API_TYPE_START			(28)	/* the type start */
#define LINKER_API_TYPE_FIELD			(29)	/* a field of the type */
#define	LINKER_API_TYPE_END				(30)	/* the end of the type */
#define LINKER_API_CONSTANTS_START		(31)	/* start of a constant group */
#define LINKER_API_CONSTANT				(32)	/* a constant */
#define LINKER_API_DATA_CONSTANT		(33)	/* a data constant */
#define	LINKER_API_CONSTANTS_END		(34)	/* the end of a constant group */
#define	LINKER_API_END					(35)	/* the end of the api */
#define	LINKER_SAMPLE					(36)	/* a sample */
#define	LINKER_APPLICATION_START		(37)	/* the start of an application group */
#define	LINKER_APPLICATION_SECTION		(38)	/* a section for describing the application */
#define	LINKER_APPLICATION_SUB_SECTION	(39)	/* a section for describing the application */
#define	LINKER_APPLICATION_OPTION		(40)	/* application option */
#define	LINKER_APPLICATION_COMMAND		(41)	/* application command */
#define	LINKER_APPLICATION_SYNOPSIS		(42)	/* application synopsis */
#define	LINKER_APPLICATION_END			(43)	/* application end */

/*--------------------------------------------------------------------------------*
 * general useful structures.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned char*	name;
	unsigned int	name_length;
	unsigned int	allocated_size;
	unsigned char	fixed;
} NAME;

/*--------------------------------------------------------------------------------*
 * ATOM list structures.
 *--------------------------------------------------------------------------------*/
#define	ATOM_BLOCK_SIZE		(100)

#define ATOM_BLOCK_UNKNOWN	(0)		/* the type of the block is unknown */
#define ATOM_BLOCK_FILE		(1)		/* the block holds a file type block */
#define ATOM_BLOCK_STATE	(2)		/* the block holds a state */
#define ATOM_BLOCK_TIMELINE	(3)		/* the block holds a timeline definition */
#define ATOM_BLOCK_FUNCTION	(4)		/* the block holds a function definition */

typedef struct
{
	unsigned int	type;
	unsigned int	line;
	unsigned int	atom;
	unsigned int	block;
	unsigned int	func_api;

} ATOM_TYPE_ANY;

typedef struct
{
	unsigned int	type;
	unsigned int	line;
	unsigned int	atom;
	unsigned int	block;
	unsigned int	func_api;
	NAME			name;
	NAME			type_type;
	NAME			description;

} ATOM_TYPE_TYPE;

typedef struct
{
	unsigned int	type;
	unsigned int	line;
	unsigned int	atom;
	unsigned int	block;
	unsigned int	func_api;
	unsigned int	group;
	unsigned int	name_length;
	unsigned char*	name;

} ATOM_TYPE_NAME;

typedef struct
{
	unsigned int	type;
	unsigned int	line;
	unsigned int	atom;
	unsigned int	block;
	unsigned int	func_api;
	unsigned int	fixed;
	unsigned int	string_length;
	unsigned char*	string;

} ATOM_TYPE_STRING;

typedef struct
{
	unsigned int	type;
	unsigned int	line;
	unsigned int	atom;
	unsigned int	block;
	unsigned int	func_api;
	unsigned int	name_length;
	unsigned int	string_length;
	unsigned char*	name;
	unsigned char*	string;

} ATOM_TYPE_PAIR;

typedef struct
{
	unsigned int	type;
	unsigned int	line;
	unsigned int	atom;
	unsigned int	block;
	unsigned int	func_api;
	unsigned int	api;
	unsigned char	number[4];

} ATOM_TYPE_NUMBER;

typedef struct
{
	unsigned int	type;
	unsigned int	line;
	unsigned int	atom;
	unsigned int	block;
	unsigned int	func_api;
	unsigned int	api;
	unsigned int	true_false;

} ATOM_TYPE_BOOLEAN;

typedef union
{
	ATOM_TYPE_ANY		any;
	ATOM_TYPE_NAME		name;
	ATOM_TYPE_PAIR		pair;
	ATOM_TYPE_TYPE		type;
	ATOM_TYPE_NUMBER	number;
	ATOM_TYPE_STRING	string;
	ATOM_TYPE_BOOLEAN	boolean;

} ATOM_ITEM;

typedef struct tag_atom_index
{
	unsigned int	num_items;
	ATOM_ITEM		index[ATOM_BLOCK_SIZE];

	struct tag_atom_index* next;
} ATOM_BLOCK;

typedef struct tag_atom_list
{
	unsigned int	block_count;
	unsigned int	line_number;
	unsigned int	number_atoms;
	unsigned int	multiline:1;
	ATOM_BLOCK		index;
	ATOM_BLOCK*		last;

} ATOM_INDEX;

/*--------------------------------------------------------------------------------*
 * structures for describing the document.
 *--------------------------------------------------------------------------------*/
#define LEVEL_BLOCK_SIZE	(10)

typedef struct tag_doc_section
{
	NAME			section_title;		/* the section title */
	unsigned int	level;				/* the level that this section is at */
	unsigned int	index;				/* the index number */
	GROUP*			api_index_group;	/* the group that the api belongs too  (used for id) */
	unsigned int	api_index_flags;	/* api flags that was used to generate the index (used for id) */

	struct tag_doc_section*	child;		/* the items below this node */
	struct tag_doc_section*	parent;		/* the item that this node is a child of */
	struct tag_doc_section*	next;		/* the next in the list */
	struct tag_doc_section*	last;		/* the last item in this section of the list */
} DOC_SECTION;

typedef struct tag_chapter
{
	NAME			file_name;			/* the file name that the chapter was loaded from */
	unsigned int	index;				/* the chapter number */
	
	DOC_SECTION		root;				/* the last section in the document */

	struct tag_chapter*	next;			/* next in list */
} CHAPTER;

typedef struct
{
	NAME			document_name;		/* the name of the document being rendered */
	CHAPTER			chapter_list;		/* the list of document chapters */
	CHAPTER*		last_chapter;		/* the end of the chapter list */
	unsigned int	version;			/* the version */
	unsigned int	date;				/* the date */
	unsigned int	chapter_count;		/* the number of chapters found */
	unsigned short*	level_index;		/* an array of level indices */
		
} DOCUMENT;

/*--------------------------------------------------------------------------------*
 * the compiler structures.
 *--------------------------------------------------------------------------------*/
#define	FLAG_GHOST				(0x8000)	/* the item is a ghost item */
#define	FLAG_MESSAGE			(0x4000)	/* the item is a message */
#define	FLAG_DEPENDENCY			(0x2000)	/* the item has a node that is depend on it */
#define	FLAG_WAIT				(0x1000)	/* a wait has been claimed on this item */
#define	FLAG_IN_FUNCTION		(0x0800)	/* the item is in a function */
#define	FLAG_TAGGED				(0x0400)	/* this is a toggle bit for marking items as interesting */
#define	FLAG_ACTIVE				(0x0200)	/* this is the active item of interest */
#define	FLAG_FUNCTION			(0x0100)	/* this message is a function */
#define	FLAG_FUNCTION_END		(0x0080)	/* this message is the end of a function */
#define	FLAG_BROADCAST			(0x0040)	/* this message is a broadcast */

#define FLAG_MESSAGE_MASK		(0x0003)	/* mask the message types */
#define	FLAG_MESSAGE_CALL		(0x0003)	/* the message is a call */
#define	FLAG_MESSAGE_SEND		(0x0002)	/* the message is a message */
#define	FLAG_SEQUENCE_RESPONDS	(0x0001)	/* the message responds to a message */

#define	FLAG_SEQUENCE_WAITFOR	(0x0004)	/* the message waits for another message */

#define	ATOM_TYPE_RECORD		(0x01)		/* records */
#define ATOM_TYPE_FIELD			(0x02)		/* field */

typedef struct
{
	char*			filename;
	unsigned int	line_number;

} SOURCE_REFERENCE;

typedef struct tag_block_name
{
	unsigned int		name_length;			/* length of the trigger name */
	unsigned char		name[MAX_NAME_LENGTH];	/* the trigger name */
	union
	{
		GROUP*			group;					/* the group the thing belongs to */
		FUNCTION*		function;				/* the function it belongs to */
	} owner;

	struct tag_block_name*	next;
} BLOCK_NAME;

typedef struct tag_sample
{
	NAME	name;								/* the name of the sample */	
	NAME	sample;								/* the contents of the sample */

	struct tag_sample* next;
} SAMPLE;

/* state machine types */

typedef struct
{
	unsigned int	max_state_length;	/* max name length for the states */
	STATE*			init_state;			/* state machine init node */
	STATE*			state_list;			/* the nodes in a simple list */
	GROUP*			group;				/* the owner group */	
} STATE_MACHINE;

typedef struct tag_trigger
{
	unsigned int		flags;					/* the status flags for the item */
	unsigned int		name_length;
	unsigned char		name[MAX_NAME_LENGTH];	/* the name of the trigger */
	GROUP*				group;					/* the group that the trigger belongs to */

	struct tag_trigger*	next;

} TRIGGER;

typedef struct tag_triggers
{
	TRIGGER*			trigger;			/* the trigger that this references */

	struct tag_triggers*	next;
} TRIGGERS;

typedef struct tag_state_transition
{
	STATE*				next_state;			/* the state that the transition goes to */
	unsigned char*		condition;			/* the condition that causes the transition */
	unsigned int		condition_length;	/* the length of the condition string */
	TRIGGER*			trigger;			/* waits for this trigger to transition */
	TRIGGERS*			triggers;			/* the list of triggers created by this transition */
	SOURCE_REFERENCE	reference;			/* the source reference that the transition is defined on */

	struct tag_state_transition*	next;	/* the list of transitions that belong to the state */

} STATE_TRANSITION;

/* sequence diagram types */

typedef struct
{
	TIMELINE*		timeline_list;		/* the sequence diagram is made up of a list of timelines */
	GROUP*			group;				/* the owner group */	
	unsigned int	max_name_length;	/* the largest timeline name in the sequence diagram */

} SEQUENCE_DIAGRAM;

typedef struct tag_parameter
{
	unsigned char	name[MAX_NAME_LENGTH];
	unsigned int	name_length;
	unsigned int	parameter_type;	/* the type of the parameter */

	struct tag_parameter* next;
} PARAMETER;

typedef struct tag_node_list
{
	NODE*		node;			/* the node that is referenced */
	NODE*		last;			/* the end of the node list */
	TIMELINE*	timeline;		/* the timeline that the node should belong to */

	struct tag_node_list*	next;

} NODE_LIST;

typedef struct tag_message
{
	unsigned char		name[MAX_NAME_LENGTH];	/* the name of the message */
	unsigned int		name_length;
	unsigned int		flags;					/* flags - what type of message */
	TIMELINE*			sending_timeline;		/* where the message is being sent from */
	TIMELINE*			target_timeline;		/* where the message is being sent to */
	PARAMETER*			parameter_list;			/* the list of parameters for the message */
	NODE*				receiver;				/* the node that receives this message */
	NODE*				sender;					/* the node that sent the message */
} MESSAGE;

struct tag_timeline
{
	unsigned char*		name;				/* the name of the state */
	unsigned int		flags;				/* the status flags for the item */
	unsigned int		name_length;
	unsigned int		line_number;		/* the line number that first atom in the list was defined in */
	unsigned int		tag_id;				/* the tag id of the state */
	unsigned int		column;				/* the column number for drawing */
	GROUP*				group;				/* the group that the state belongs to */
	NODE*				node;				/* list of nodes for this timeline */
	NODE*				last_node;			/* the last node in the timeline */
	TIMELINE*			next;				/* the list states that belong to the group */
};

struct tag_state
{
	unsigned char*		name;				/* the name of the state */
	unsigned int		flags;				/* the status flags for the item */
	unsigned int		name_length;
	unsigned int		line_number;		/* the line number that first atom in the list was defined in */
	unsigned int		tag_id;				/* the tag id of the state */
	void*				graph_node;			/* this is the graph node for the state */
	GROUP*				group;				/* the group that the state belongs to */
	STATE_TRANSITION*	transition_list;	/* the list of transitions */
	STATE*				next;				/* the list states that belong to the group */
};

struct tag_node
{
	unsigned int		flags;				/* the status flags for the item */
	unsigned int		line_number;		/* the line number that first atom in the list was defined in */
	unsigned int		tag_id;				/* the tag id of the state */
	unsigned int		level;				/* the level the the NODE is at */
	unsigned int		condition_length;	/* the length of the condition string */
	unsigned char*		condition;			/* the condition that causes the transition */
	BLOCK_NAME			wait_message;		/* the message that this is waiting for */
	BLOCK_NAME			after;				/* the message or function that this is following */
	GROUP*				group;				/* the group that the state belongs to */
	TRIGGER*			trigger;			/* if the message is triggered on an event */
	TRIGGERS*			triggers;			/* the message node also, triggers events */
	MESSAGE*			sent_message;		/* the message that this node references */
	MESSAGE*			received_message;	/* if this has a wait, the fixup of the received message */
	FUNCTION*			function;			/* this function calls a timeline */
	TIMELINE*			timeline;			/* the timeline this belongs to */
	NODE*				function_end;		/* the end of the function */
	NODE*				next;				/* next node in the list */
	NODE*				return_node;		/* used for walking the tree */
};

typedef struct tag_function_node
{
	unsigned int	flags;						/* used for the messages to define the types */
	TIMELINE*		to_timeline;				/* the timeline that the message is specifically being sent to (optional) */
	FUNCTION*		call;						/* the function that is called by the block */
	BLOCK_NAME		to_timeline_name;			/* the message to be send from here */
	BLOCK_NAME		message;					/* the message to be send from here */
	BLOCK_NAME		after;						/* the message follows the sending of this message */
	BLOCK_NAME		sequence;					/* the message waits for the message to arrive, or responds to it */
	BLOCK_NAME		condition;					/* if this is a state then the condition for the transition */
	BLOCK_NAME*		trigger;					/* this is a reference to the trigger */
	BLOCK_NAME*		triggers_list;				/* if this item causes a trigger */

	struct tag_function_node* next;
} FUNCTION_NODE;

struct tag_function
{
	unsigned int	flags;						/* the flags for the function */
	unsigned int	name_length;				/* the length of the name */
	unsigned char	name[MAX_NAME_LENGTH];		/* name of the function */
	FUNCTION_NODE*	node;						/* the nodes of the function */

	struct tag_function*	next;
};

/* api structures */
typedef struct tag_api_type_record
{
	unsigned int	record_type;
	unsigned int	depth;						/* is this a sub-type and how deep is it */
	NAME			type_item;
	NAME			name_value;
	NAME			brief;

	struct tag_api_type_record*	next;

} API_TYPE_RECORD;

typedef struct tag_api_type
{
	unsigned short		max_type_item_length;	/* largest size of type_item that we have added */
	unsigned short		max_name_value_length;	/* the largest name_value that has been added */
	unsigned int		depth;					/* how many sub-types does this API TYPE have */
	NAME				name;
	NAME				description;
	DOC_SECTION*		index_item;				/* the index item that is referenced by this item */
	API_TYPE_RECORD*	record_list;
	API_TYPE_RECORD*	last_record;			/* the last record added to the record list */

	struct tag_api_type*	next;

} API_TYPE;

typedef struct tag_api_constant
{
	unsigned int	constant;
	NAME			name;
	NAME			type;
	NAME			brief;
	NAME			value;
	DOC_SECTION*	index_item;					/* the index item that is referenced by this item */

	struct tag_api_constant*	next;
} API_CONSTANT;

typedef struct tag_api_constants
{
	unsigned short	max_name_length;
	unsigned short	max_value_length;
	NAME			name;
	NAME			description;
	API_CONSTANT*	last_constant;
	API_CONSTANT*	constant_list;
	DOC_SECTION*	index_item;					/* the index item that is referenced by this item */

	struct tag_api_constants*	next;
} API_CONSTANTS;

typedef struct tag_api_parameter
{
	NAME			name;
	NAME			type;
	NAME			brief;

	struct tag_api_parameter*	next;
} API_PARAMETER;

typedef struct tag_api_returns
{
	NAME			value;
	NAME			brief;

	struct tag_api_returns*	next;
} API_RETURNS;

typedef struct tag_api_function
{
	unsigned short	max_param_name_length;
	unsigned short	max_param_type_length;
	unsigned short	max_return_value_length;
	unsigned short	api_id;						/* ID number associated with the API */
	NAME			name;
	NAME			return_type;
	NAME			action;
	NAME			description;
	API_PARAMETER*	parameter_list;
	API_RETURNS*	returns_list;
	DOC_SECTION*	index_item;					/* the index item that is referenced by this item */
	
	struct tag_api_function*	next;
} API_FUNCTION;

typedef struct
{
	API_TYPE*		type_list;			/* the list of types in this API*/
	API_FUNCTION*	function_list;		/* the list of functions in this API */
	API_CONSTANTS*	constants_list;		/* the list of constant groups in this API */
	GROUP*			group;				/* the owner group */	
} API;

/* application group of atoms */
#define	OPTION_FLAG_MULTIPLE	((unsigned int) 0x00000001)
#define OPTION_FLAG_REQUIRED	((unsigned int) 0x00000002)

typedef struct tag_option
{
	unsigned int		flags;
	unsigned int		option_id;
	NAME				name;
	NAME				value;
	NAME				description;

	struct tag_option*	last;
	struct tag_option*	next;
} OPTION;

typedef struct tag_command
{
	NAME				name;
	NAME				parameters;
	NAME				description;

	struct tag_command*	last;
	struct tag_command*	next;
} COMMAND;

typedef struct tag_section
{
	NAME				name;
	NAME				section_data;
	struct tag_section*	sub_section_list;
	
	struct tag_section*	last;
	struct tag_section*	next;

} SECTION;

typedef struct tag_synopsis
{
	unsigned int			list_length;
	NAME					name;
	OPTION**				list;
	
	struct tag_synopsis*	last;
	struct tag_synopsis*	next;
} SYNOPSIS;

typedef struct tag_synopsis_list
{
	NAME		name;
	NAME		items;

	struct tag_synopsis_list*	last;
	struct tag_synopsis_list*	next;

} SYNOPSIS_LIST;

typedef struct tag_application
{
	NAME			name;				/* the name of the application */
	OPTION*			option_list;		/* the list of options that belong to this application */
	COMMAND*		command_list;		/* the list of commands that belong to this application */
	SECTION*		section_list;		/* the list of sections that belong to this application */
	SYNOPSIS*		synopsis;			/* the connected synopsis - used in the processor */
	SYNOPSIS_LIST	synopsis_list;		/* the list of synopsis items - that are read from the input */
	unsigned int	max_option_length;	/* the length of the largest option */
	unsigned int	max_command_length;	/* the length of the largest command name */

	struct tag_application*	next;

} APPLICATION;


/*--------------------------------------------------------------------------------*
 * global pull it all together structure
 *--------------------------------------------------------------------------------*/

struct tag_group
{
	unsigned char		name[MAX_NAME_LENGTH];
	unsigned int		name_length;
	unsigned int		max_message_length;	/* the max message name length */
	DOCUMENT*			document;			/* This only belongs in the root group, and is the document */
	SEQUENCE_DIAGRAM*	sequence_diagram;	/* if not NULL the sequence diagram that belongs to this group */
	STATE_MACHINE*		state_machine;		/* if not NULL the state machine that belongs to this group */
	API*				api;				/* if not NULL the api definitions that belong to this group */
	APPLICATION*		application;		/* if not NULL the application details that belong to this group */
	TRIGGER*			trigger_list;		/* the list of triggers that belong to this list */
	SAMPLE*				sample_list;		/* the list of samples that belong to the group */
	struct tag_group*	next;				/* the next group in the list */
};

/*--------------------------------------------------------------------------------*
 * Decoder defines.
 *--------------------------------------------------------------------------------*/
typedef struct tag_deferred_list
{
	NODE*			node;
	MESSAGE*		message;
	FUNCTION_NODE*	function;

	struct tag_deferred_list*	next;
} DEFFERED_LIST;

typedef struct	tag_name_pairs_list
{
	NAME	name;
	NAME	string;

	struct tag_name_pairs_list*	next;

} NAME_PAIRS_LIST;

typedef struct
{
	unsigned int		flags;					/* used for the messages to define the types */
	unsigned int		line_number;			/* the line number that the block started in */
	unsigned int		block_number;			/* the current block number */
	unsigned int		type;					/* the type of the block */
	unsigned int		tag;					/* the tag for this block */
	unsigned int		activation;				/* the activation that this belongs to */
	unsigned int		num_returns;			/* the number of return values */
	unsigned int		num_parameters;			/* the number of parameters */
	unsigned int		application_id;			/* the current application */
	unsigned int		application_flag;		/* the flags set for the application */
	STATE*				state;					/* state that this block belongs to */
	GROUP*				group;					/* the group that the node belongs to */
	FUNCTION*			function;				/* the function that the node belongs to */
	TIMELINE*			timeline;				/* the timeline that this block belongs to */
	TIMELINE*			to_timeline;			/* the timeline that the message is specifically being sent to (optional) */
	BLOCK_NAME*			trigger;				/* this is a reference to the trigger */
	BLOCK_NAME*			triggers_list;			/* if this item causes a trigger */
	API_TYPE*			api_type;				/* the block has a reference to a type */
	API_FUNCTION*		api_function;			/* the block has a reference to an API */
	API_CONSTANTS*		api_constants;			/* the block has a set of constants attached */
	SOURCE_REFERENCE*	reference;				/* source reference of the block */
	BLOCK_NAME			after;					/* the message follows the sending of this message */
	BLOCK_NAME			author;					/* the author field of an api, file or function */
	BLOCK_NAME			message;				/* the message for this block */
	BLOCK_NAME			sequence;				/* the message waits for the message to arrive, or responds to it */
	BLOCK_NAME			condition;				/* if this is a state then the condition for the transition */
	BLOCK_NAME			transition;				/* if this is a state then transition */
	NAME				value;					/* the value of the option */
	NAME				option;					/* the option */
	NAME				action;					/* this block contains an action */
	NAME				description;			/* this block contains a description */
	NAME_PAIRS_LIST		returns;				/* list of return pairs from the header block */
	NAME_PAIRS_LIST		parameters;				/* list of parameters from the header block */
	BLOCK_NAME			function_to_timeline;	/* differed lookup for function timeline */
} BLOCK_NODE;

typedef struct
{
	unsigned short	size;
	unsigned char*	buffer;

} RECORD_BITS;

typedef struct
{
	int				outfile;
	unsigned int	parts;
	unsigned int	offset;
	unsigned int	record_size;
	unsigned char*	buffer;
	RECORD_BITS		buffer_list[20];

} OUTPUT_FILE;

/*--------------------------------------------------------------------------------*
 * Document processor defines and structures.
 *--------------------------------------------------------------------------------*/
#define	MODEL_LOAD_UNKNOWN					(0)
#define	MODEL_LOAD_STATE					(1)
#define	MODEL_LOAD_SEQUENCE					(2)
#define	MODEL_LOAD_API						(3)
#define	MODEL_LOAD_APPLICATION				(4)

#define TYPE_TEXT							(0)
#define TYPE_STATE_MACHINE					(1)
#define TYPE_SEQUENCE_DIAGRAM				(2)
#define TYPE_API							(3)
#define TYPE_SAMPLE							(4)
#define TYPE_APPLICATION					(5)
#define TYPE_LIST							(7)
#define TYPE_TABLE							(8)
#define TYPE_CODE_BLOCK						(9)
#define TYPE_QUOTE_BLOCK					(10)
#define TYPE_INDEX							(11)

#define	INPUT_STATE_INTERNAL_SEARCHING		(0)
#define	INPUT_STATE_INTERNAL_SCHEME			(1)
#define	INPUT_STATE_INTERNAL_GROUP_COLLECT	(2)
#define	INPUT_STATE_INTERNAL_TYPE_COLLECT	(3)
#define	INPUT_STATE_INTERNAL_ITEM_COLLECT	(4)
#define	INPUT_STATE_INTERNAL_FIND_FLAGS		(5)
#define	INPUT_STATE_INTERNAL_DUMP_TILL_END	(6)

#define TYPE_STATE_MACHINE_STR				"state_machine"
#define TYPE_SEQUENCE_DIAGRAM_STR			"sequence_diagram"
#define TYPE_API_STR						"api"
#define TYPE_SAMPLE_STR						"sample"
#define TYPE_APPLICATION_STR				"application"

/* loading list */
typedef struct tag_name_list
{
	unsigned char*	name;
	unsigned int	name_length;

	struct tag_name_list* next;
} NAME_LIST;

typedef struct
{
	unsigned int	to_tag;
	NAME			name;
	NAME			condition;
	NAME			trigger;
	NAME_LIST		triggers;
} LOAD_TRANSITION;

typedef struct
{
	unsigned int	sender;
	unsigned int	receiver;
	NAME			timeline;
	NAME			message_name;
} LOAD_MESSAGE;

typedef struct
{
	int				input_file;
	unsigned int	state;
	unsigned int	temp_type;
	unsigned int	count;
	unsigned int	section;
	unsigned int	internal_state;
	unsigned int	buffer_pos;
	unsigned int	bytes_read;
	unsigned int	output_start;
	unsigned int	output_end;
	unsigned int	model_pos;
	unsigned int	line_number;
	unsigned int 	emp_level;
	unsigned int 	line_start;
	unsigned int 	header_level;
	unsigned int 	buffer_size;
	unsigned char*	input_name;
	unsigned char*	buffer;
	NAME			flags;			/* the following are the parts of the URL for the model */
	NAME			sub_part;		/* using the following format: */
	NAME			type_name;		/* doc_gen:/<group_name>/<type_name>/<part_name>/<item_name>/<sub_part>?<flags> */
	NAME			item_name;
	NAME			part_name;
	NAME			group_name;
	NAME			title;
	DOC_SECTION*	index_item;		/* the index item that is referenced by the API item that is going to referenced */

} INPUT_STATE;

/*--------------------------------------------------------------------------------*
 * shared functions
 *--------------------------------------------------------------------------------*/
void add_api_start_atom (ATOM_INDEX* list, unsigned char record_group_type, unsigned int group_id);
void add_string_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, unsigned char* string, unsigned int string_length, unsigned int func_api );
void add_api_type_atom ( ATOM_INDEX* list, ATOM_ATOMS atom, NAME* type, NAME* name, NAME* description);

/*--------------------------------------------------------------------------------*
 * Cross-Platform defines.
 *--------------------------------------------------------------------------------*/

#ifdef __unix__
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	define WRITE_FILE_STATUS	(O_TRUNC|O_CREAT|O_WRONLY)
#	define WRITE_FILE_PERM		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#	define READ_FILE_STATUS		(O_RDONLY)
#	define DIR_DELIMETER		'/'
#elif defined(_WIN32)
#	include <io.h>
#	define WRITE_FILE_STATUS	(O_BINARY|O_TRUNC|O_CREAT|O_WRONLY)
#	define WRITE_FILE_PERM		(S_IWRITE|S_IREAD)
#	define READ_FILE_STATUS		(O_BINARY|O_RDONLY)
#	define DIR_DELIMETER		'\\'
#else
	#error("Your OS is not supported. Sorry!");

#endif

#endif

