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

#define FILE_BLOCK_SIZE		(8 * 1024)

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
#define INTERMEDIATE_RECORD_GROUP		(0)
#define INTERMEDIATE_RECORD_FUNCTION	(1)
#define INTERMEDIATE_RECORD_NAME		(2)
#define INTERMEDIATE_RECORD_STRING		(3)

/* record format */
#define RECORD_TYPE			(0)
#define RECORD_ATOM			(1)
#define RECORD_GROUP		(2)		/* group and functions share the same field */ 
#define RECORD_FUNCTION		(2) 	/* group and functions share the same field */ 
#define RECORD_BLOCK_NUM	(4) 
#define RECORD_LINE_NUM		(6)
#define RECORD_DATA_SIZE	(8) 
#define RECORD_DATA_START	(10) 

#define RECORD_FUNCTION_MASK	(0x8000)

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

/* record defintions:
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
 *	linker_transistion			= {short:group_id,short:to_state,string:name,short:trigger,<trigger_id>,short:num_triggers,<trigger_id>}
 *	linker_sequence_start		= {}
 *	linker_timeline				= {string:name}
 *	linker_message				= {string:message,short:to_node,byte:num_parameters}
 *	linker_parameter			= {string:name,byte:type,string:value}
 *	linker_node_start			= {}
 *	linker_sent_message			= {short:message_id,short:to_node}
 *	linker_recieved_message		= {short:message_id,short:from_node}
 */

#define LINKER_TRIGGER				( 0)	/* defines a trigger */
#define LINKER_TRIGGERS				( 1)	/* define a triggers */
#define LINKER_CONDITION			( 2)	/* defines a condition */
#define LINKER_SOURCE_FILE			( 3)	/* defines a source file reference */
#define LINKER_SOURCE_REFERENCE		( 4)	/* defines a source line reference */
#define	LINKER_END					( 5)	/* the end of a list of items */
#define LINKER_BLOCK_END			( 6)	/* the end of a block - NULL record */

#define LINKER_GROUP				( 6)	/* defines A group name */
#define LINKER_STATE_MACHINE_START	( 8)	/* defines the start of a state machine */
#define LINKER_STATE				( 9)	/* the start of a state list */
#define LINKER_TRANSITON			(10)	/* defines a transition between two states */
#define LINKER_STATE_MACHINE_END	(11)	/* denotes the end of a state machine */
#define LINKER_SEQUENCE_START		(12)	/* denotes the start of a sequence diagram */
#define	LINKER_TIMELINE				(13)	/* defines a timeline */
#define LINKER_FUNCTION				(14)	/* a function name definition */
#define LINKER_MESSAGE				(15)	/* defines a message */
#define LINKER_PARAMETER			(16)	/* denotes the start of a sequence diagram */
#define LINKER_NODE_START			(17)	/* node start */
#define LINKER_SENT_MESSAGE			(18)	/* references a message that is being send from this node */
#define LINKER_NODE_END				(19)	/* node start */
#define LINKER_SEQUENCE_END			(20)	/* denotes the end of a sequence diagram */

/*--------------------------------------------------------------------------------*
 * The Group lookup list structures.
 *--------------------------------------------------------------------------------*/
#define	LOOKUP_INDEX_SIZE	(100)

typedef struct
{
	unsigned int	hash;
	unsigned int	name_length;
	unsigned int	payload_length;
	unsigned char*	name;
	unsigned char*	payload;

} LOOKUP_ITEM;

typedef struct tag_lookup_list
{
	unsigned int	num_items;
	LOOKUP_ITEM		lookup[LOOKUP_INDEX_SIZE];

	struct tag_lookup_list*	next;

} LOOKUP_LIST;

/*--------------------------------------------------------------------------------*
 * ATOM list structures.
 *--------------------------------------------------------------------------------*/
#define	ATOM_BLOCK_SIZE		(100)

typedef struct
{
	unsigned int	type;
	unsigned int	line;
	unsigned int	atom;
	unsigned int	block;
	unsigned int	function;

} ATOM_TYPE_ANY;

typedef struct
{
	unsigned int	type;
	unsigned int	line;
	unsigned int	atom;
	unsigned int	block;
	unsigned int	function;
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
	unsigned int	function;
	unsigned int	string_length;
	unsigned char*	string;

} ATOM_TYPE_STRING;

typedef union
{
	ATOM_TYPE_ANY		any;
	ATOM_TYPE_NAME		name;
	ATOM_TYPE_STRING	string;

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

typedef struct tag_timeline TIMELINE;
typedef struct tag_state STATE;
typedef struct tag_group GROUP;
typedef struct tag_node NODE;
typedef struct tag_function FUNCTION;

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

typedef struct
{
	STATE*	init_state;			/* state machine init node */
	STATE*	state_list;			/* the nodes in a simple list */
	GROUP*	group;				/* the owner group */	
} STATE_MACHINE;

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

typedef struct
{
	TIMELINE*		timeline_list;	/* the sequence diagram is made up of a list of timelines */
	GROUP*			group;			/* the owner group */	

} SEQUENCE_DIAGRAM;

struct tag_group
{
	unsigned char		name[MAX_NAME_LENGTH];
	unsigned int		name_length;
	unsigned int		max_message_length;	/* the max message name length */
	SEQUENCE_DIAGRAM*	sequence_diagram;	/* if not NULL the sequence diagram that belongs to this group */
	STATE_MACHINE*		state_machine;		/* if not NULL the state machine that belongs to this group */
	TRIGGER*			trigger_list;		/* the list of triggers that belong to this list */
	struct tag_group*	next;				/* the next group in the list */
};

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

typedef struct tag_deferred_list
{
	NODE*			node;
	MESSAGE*		message;
	FUNCTION_NODE*	function;

	struct tag_deferred_list*	next;
} DEFFERED_LIST;

typedef struct
{
	unsigned int		flags;					/* used for the messages to define the types */
	unsigned int		line_number;			/* the line number that the block started in */
	unsigned int		block_number;			/* the current block number */
	unsigned int		type;					/* the type of the block */
	unsigned int		tag;					/* the tag for this block */
	unsigned int		activation;				/* the activation that this belongs to */
	STATE*				state;					/* state that this block belongs to */
	GROUP*				group;					/* the group that the node belongs to */
	FUNCTION*			function;				/* the function that the node belongs to */
	TIMELINE*			timeline;				/* the timeline that this block belongs to */
	TIMELINE*			to_timeline;			/* the timeline that the message is specifically being sent to (optional) */
	BLOCK_NAME*			trigger;				/* this is a reference to the trigger */
	BLOCK_NAME*			triggers_list;			/* if this item causes a trigger */
	SOURCE_REFERENCE*	reference;				/* source reference of the block */
	BLOCK_NAME			function_to_timeline;	/* differed lookup for function timeline */
	BLOCK_NAME			message;				/* the message for this block */
	BLOCK_NAME			after;					/* the message follows the sending of this message */
	BLOCK_NAME			sequence;				/* the message waits for the message to arrive, or responds to it */
	BLOCK_NAME			condition;				/* if this is a state then the condition for the transition */
	BLOCK_NAME			transition;				/* if this is a state then transition */
} BLOCK_NODE;

typedef struct
{
	unsigned char	size;
	unsigned char*	buffer;

} RECORD_BITS;

typedef struct
{
	int				outfile;
	unsigned int	parts;
	unsigned int	offset;
	unsigned int	record_size;
	unsigned char*	buffer;
	RECORD_BITS		buffer_list[4];

} OUTPUT_FILE;

/*--------------------------------------------------------------------------------*
 * Document processor defines and structures.
 *--------------------------------------------------------------------------------*/
#define	MODEL_LOAD_UNKNOWN					(0)
#define	MODEL_LOAD_STATE					(1)
#define	MODEL_LOAD_SEQUENCE					(2)

#define TYPE_TEXT							(0)
#define TYPE_STATE_MACHINE					(1)
#define TYPE_SEQUENCE_DIAGRAM				(2)

#define	INPUT_STATE_INTERNAL_SEARCHING		(0)
#define	INPUT_STATE_INTERNAL_SCHEME			(1)
#define	INPUT_STATE_INTERNAL_GROUP_COLLECT	(2)
#define	INPUT_STATE_INTERNAL_TYPE_COLLECT	(3)
#define	INPUT_STATE_INTERNAL_ITEM_COLLECT	(4)
#define	INPUT_STATE_INTERNAL_DUMP_TILL_END	(5)


/* loading list */
typedef struct
{
	unsigned char*	name;
	unsigned int	name_length;
} NAME;

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
	unsigned int	internal_state;
	unsigned int	buffer_pos;
	unsigned int	bytes_read;
	unsigned int	output_start;
	unsigned int	output_end;
	unsigned int	model_pos;
	unsigned int	item_length;
	unsigned int	group_length;
	unsigned char*	input_name;
	unsigned char	buffer[FILE_BLOCK_SIZE];
	unsigned char	item_name[MAX_NAME_LENGTH];
	unsigned char	group_name[MAX_NAME_LENGTH];

} INPUT_STATE;

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

