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
	ATOM_CALL,
	ATOM_CONDITION,
	ATOM_MESSAGE,
	ATOM_NEXT,
	ATOM_PARAMETER,
	ATOM_REPEATS,
	ATOM_RESPONDS,
	ATOM_SEND,
	ATOM_STATE,
	ATOM_TIMELINE,
	ATOM_TO,
	ATOM_TRIGGER,
	ATOM_TRIGGERS,
	ATOM_WAITFOR,
	ATOM_FUNCTION,
	ATOM_NUM_STRINGS
} ATOM_ATOMS;

unsigned int atoms_get_length(int word);
int	atoms_check_word(unsigned char* word);

#endif
