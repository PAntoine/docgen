/*--------------------------------------------------------------------------------*
 * Name: Parser Look-up tree
 * Desc: function and data structures to speed up a specific string search
 *
 *         **** DO NOT AMEND THIS CODE - IT IS AUTO_GENERATED ****
 *
 * Code and table produced by:
 *            build_graph 
 *            version 0.9
 *
 *  written by Peter Antoine. 
 *
 *   **** This code can be used however you like, no claims whatsoever are ****
 *   **** placed on the auto-generated code or data, or on the code that   ****
 *   **** uses the that code or data.                                      ****
 *   **** Or, to paraphrase "Fill your boots".                             ****
 *--------------------------------------------------------------------------------*/

#include "symbols.h"

SYMBOLS_STRING_TABLE	symbols_table[18] = {
		{"char",4},
		{"const",5},
		{"double",6},
		{"enum",4},
		{"extern",6},
		{"float",5},
		{"int",3},
		{"long",4},
		{"short",5},
		{"signed",6},
		{"static",6},
		{"struct",6},
		{"typedef",7},
		{"union",5},
		{"unsigned",8},
		{"void",4},
		{"volatile",8},
		{"zzzz",4}
};

static signed char	symbol_table[256] = 
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x03,0x0B,0x01,0x09,0x0D,0x10,0x12,0x02,0x11,0x00,0x00,0x0C,0x0E,0x06,0x05,
	 0x14,0x00,0x04,0x07,0x08,0x0A,0x15,0x00,0x0F,0x13,0x16,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

typedef struct
{	unsigned char	mask;
	unsigned char	table;
} LOOKUP_TABLE;

static LOOKUP_TABLE	state_table[9] = {
	{ 0, 0},{ 1, 0},{ 2, 0},{ 3, 1},{ 4, 0},{ 5, 1},{ 6, 2},{ 7, 1},{ 8, 3}};

static unsigned int mask_table[9] = {
	0x00233782,0x00000025,0x00008040,0x00020104,0x00000018,0x00000040,0x00020080,0x00000020,
	0x00021000};

static unsigned char table[4][22] = {
	{0xee,0x01,0xff,0xf5,0xf4,0xfe,0xfc,0x03,0xf3,0xfd,0x05,0x00,0xf8,0x02,0x00,0xfb,0xfa,0xf9,0x00,0x00,0x00,0x07},
	{0x00,0x00,0xf7,0x00,0x00,0x08,0x06,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf6,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf2,0x00,0x00,0x00,0x00},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xef,0x00,0x00,0x00,0x00,0xf0,0x00,0x00,0x00,0x00}};

unsigned int symbols_get_length(int word)
{
	return symbols_table[word].length;
}

int	symbols_check_word(unsigned char* word)
{
	signed char		line = 0;
	unsigned int	symbol = 0;
	unsigned int	count = 0;
	unsigned int	letter = 0;
	unsigned int	symbol_bit;

	do
	{
		symbol = symbol_table[word[letter]];
		symbol_bit = (0x1 << (symbol & 0x1F));

		if ((mask_table[state_table[line].mask] & symbol_bit) == 0)
		{
			/* bad symbol for line */
			line = 0;
		}
		else
		{
			line = (signed char) table[state_table[line].table][symbol];
		}
		letter++;

	}
	while (line > 0);

	symbol_bit = (0x1 << (symbol_table[word[letter]] & 0x1F));

	if (line < 0)
		line = (0-line) - 1;
	else if (((mask_table[state_table[line].mask] & symbol_bit) != 0) && ((line = (signed char) table[state_table[line].table][symbol_table[0]]) < 0))
		line = (0-line) - 1;
	else
		line = -1;
	if (line >= 0)
	{
		for (count=letter;count < symbols_table[line].length;count++)
		{
			if (word[count] != symbols_table[line].name[count])
			{
				line = -1;
				break;
			}
		}
	}
	return line;
}
