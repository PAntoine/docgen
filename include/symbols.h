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

#ifndef	__SYMBOLS_H__
#define __SYMBOLS_H__

typedef	struct
{
	char*			name;
	unsigned int	length;
} SYMBOLS_STRING_TABLE;

typedef enum
{
	SYMBOLS_CHAR,
	SYMBOLS_CONST,
	SYMBOLS_DOUBLE,
	SYMBOLS_ENUM,
	SYMBOLS_EXTERN,
	SYMBOLS_FLOAT,
	SYMBOLS_INT,
	SYMBOLS_LONG,
	SYMBOLS_SHORT,
	SYMBOLS_SIGNED,
	SYMBOLS_STATIC,
	SYMBOLS_STRUCT,
	SYMBOLS_TYPEDEF,
	SYMBOLS_UNION,
	SYMBOLS_UNSIGNED,
	SYMBOLS_VOID,
	SYMBOLS_VOLATILE,
	SYMBOLS_ZZZZ,
	SYMBOLS_NUM_STRINGS
} SYMBOLS_SYMBOLS;

unsigned int symbols_get_length(int word);
int	symbols_check_word(unsigned char* word);

#endif
