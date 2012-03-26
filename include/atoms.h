/*--------------------------------------------------------------------------------*
 * Name: Parser Look-up tree
 * Desc: function and data structures to speed up a specific string search
 *
 *         **** DO NOT AMEND THIS CODE - IT IS AUTO_GENERATED ****
 *
 * Code and table produced by:
 *            build_graph 
 *            version 0.8
 *
 *  written by Peter Antoine. 
 *
 *   **** This code can be used however you like, no claims whatsoever are ****
 *   **** placed on the auto-generated code or data, or on the code that   ****
 *   **** uses the that code or data.                                      ****
 *   **** Or, to paraphrase "Fill your boots".                             ****
 *--------------------------------------------------------------------------------*/

#ifndef	__ATOMS_H__
#define __ATOMS_H__

typedef	struct
{
	char*			name;
	unsigned int	length;
} ATOMS_STRING_TABLE;

typedef enum
{
	ATOM_ACTIVATION,
	ATOM_AFTER,
	ATOM_API,
	ATOM_AUTHOR,
	ATOM_BRIEF,
	ATOM_CALL,
	ATOM_CONDITION,
	ATOM_COPYRIGHT,
	ATOM_DEFINES,
	ATOM_END_DEFINE,
	ATOM_EXAMPLES,
	ATOM_FILE,
	ATOM_FUNCTION,
	ATOM_GROUP,
	ATOM_IGNORE,
	ATOM_LICENCE,
	ATOM_MESSAGE,
	ATOM_NAME,
	ATOM_NEXT,
	ATOM_OPTION,
	ATOM_PARAMETER,
	ATOM_RECORD,
	ATOM_REPEATS,
	ATOM_RESPONDS,
	ATOM_RETURNS,
	ATOM_SEE_ALSO,
	ATOM_SEND,
	ATOM_STATE,
	ATOM_SYNOPSIS,
	ATOM_TIMELINE,
	ATOM_TO,
	ATOM_TRIGGER,
	ATOM_TRIGGERS,
	ATOM_TYPE,
	ATOM_WAITFOR,
	ATOM_ENTRY,
	ATOM_ACTION,
	ATOM_DESC,
	ATOM_DESCRIPTION,
	ATOM_PROTOTYPE,
	ATOM_PARAMETERS,
	ATOM_ZZZZ,
	ATOM_NUM_STRINGS
} ATOM_ATOMS;

unsigned int atoms_get_length(int word);
int	atoms_check_word(unsigned char* word);

#endif
