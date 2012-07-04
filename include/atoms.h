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
	ATOM_ACTION,
	ATOM_ACTIVATION,
	ATOM_AFTER,
	ATOM_API,
	ATOM_APPLICATION,
	ATOM_AUTHOR,
	ATOM_BRIEF,
	ATOM_CALL,
	ATOM_COMMAND,
	ATOM_CONDITION,
	ATOM_CONSTANTS,
	ATOM_COPYRIGHT,
	ATOM_DESCRIPTION,
	ATOM_DESC,
	ATOM_DATE,
	ATOM_END_CONSTANTS,
	ATOM_END_SAMPLE,
	ATOM_ENTRY,
	ATOM_EXAMPLES,
	ATOM_FILE,
	ATOM_FUNCTION,
	ATOM_GROUP,
	ATOM_IGNORE,
	ATOM_LICENCE,
	ATOM_MESSAGE,
	ATOM_MULTIPLE,
	ATOM_NAME,
	ATOM_NEXT,
	ATOM_OPTION,
	ATOM_PARAMETER,
	ATOM_PARAMETERS,
	ATOM_PROTOTYPE,
	ATOM_RECORD,
	ATOM_REPEATS,
	ATOM_REQUIRED,
	ATOM_RESPONDS,
	ATOM_RETURNS,
	ATOM_SAMPLE,
	ATOM_SECTION,
	ATOM_SEE_ALSO,
	ATOM_SEND,
	ATOM_STATE,
	ATOM_SUBSECTION,
	ATOM_SYNOPSIS,
	ATOM_TAG,
	ATOM_TIMELINE,
	ATOM_TO,
	ATOM_TRIGGER,
	ATOM_TRIGGERS,
	ATOM_TYPE,
	ATOM_VALUE,
	ATOM_WAITFOR,
	ATOM_ZZZZ,
	ATOM_NUM_STRINGS
} ATOM_ATOMS;

unsigned int atoms_get_length(int word);
int	atoms_check_word(unsigned char* word);

#endif
