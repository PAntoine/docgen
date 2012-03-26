/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : error_codes
 * Desc  : This file holds the error codes and the error codes tables for the
 *         document generator functions.
 *
 * Author: pantoine
 * Date  : 12/11/2011 16:58:28
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/
#include "error_codes.h"
#include <stdio.h>

char* g_input_filename = NULL;
char* g_source_filename = NULL;
unsigned int g_source_filename_length = 0;

static char* error_string[EC_MAX_ERROR_CODE] =
{
	"Ok.",
	"Failed.",
	"Parse Failed",
	"Undefined macro",
	"macro must only be used for atom group names",
	"Invalid item name",
	"Invalid name or name not found",
	"Unknown command",
	"Unexpected charters after the command.",
	"Failed to open output file.",
	"INTERNAL ERROR: unknown record type.",
	"Input file built with later major version.",
	"Input file corrupt.",
	"Unknown atom.",
	"Multiple definitions in one block.",
	"Mixed diagram types in one block",
	"Wrong atom in the block.",
	"Atom not allowed to specify a group name",
	"Group name more than 255 characters",
	"Atom name more than 255 characters",
	"Duplicate transition within same block/state",
	"Block must have a defining atom @state, @message or @command",
	"State must have a transition to the next state",
	"Duplicate trigger defined for state",
	"State referenced that has not been defined.",
	"State machine does not have a start node defined.",
	"Duplicate condition for the state transition.",
	"Cannot have condition and trigger for the state transition.",
	"Multiple trigger definitions for state.",
	"Trigger referenced that is not defined.",
	"Block can only be assigned to one timeline.",
	"Multiple messages/calls defined in one block.",
	"Multiple @after atoms for the same message.",
	"Multiple sequence atoms for the same message.",
	"Multiple @to atoms defined for the same message.",
	"Sequence nodes must have @timeline specified.",
	"@after must be in a block that includes a @send or a @call.",
	"Multiple @functions found without a function defined in between.",
	"Nested @function found.",
	"Multiple definition of the same function",
	"Not allowed to have state and/or local atoms in the function",
	"Max nested call depth reached.",
	"@after waiting for a message/function/wait that does not exist",
	"Undefined function reference: ",
	"@send without a receiving @waitfor.",
	"@respond without a @send",
	"message being sent to an ancestor in the message chain. Loops are not allowed.",
	"input file signature incorrect.",
	"INTERNAL ERROR: input found unknown state",
	"Unknown item requested",
	"Multiple authors in a single block",
	"Undefined group referenced",
	"@file MUST be in the first block of the file",
	"Unsupported input file type",
	"INTERNAL ERROR: invalid parameter",
	"INTERNAL ERROR: Parameter Added - you should never read this.",
	"Duplicate field in definition",
	"Undefined API referenced"
};
	
/*----- FUNCTION -----------------------------------------------------------------*
 * Name : raise_warning
 * Desc : This function will raise a warning.
 *--------------------------------------------------------------------------------*/
void raise_warning ( unsigned int line_number, unsigned int error_code, const unsigned char* parameter, SOURCE_REFERENCE* reference )
{
	char* input_filename = g_input_filename;

	if (g_input_filename == NULL)
	{
		if (parameter == NULL)
			printf("warning: %s\n",error_string[error_code]);
		else
			printf("warning: %s %s\n",error_string[error_code],parameter);
	}
	else
	{
		if (g_source_filename != NULL)
			input_filename = g_source_filename;

		if (reference != NULL)
		{
			if (error_code < EC_MAX_ERROR_CODE)
				if (parameter == NULL)
					printf("%s:%d: warning: %s\n",reference->filename,reference->line_number,error_string[error_code]);
				else
					printf("%s:%d: warning: %s %s\n",reference->filename,reference->line_number,error_string[error_code],parameter);
			else
				printf("%s:%d: warning: unknown error/warning.\n",reference->filename,reference->line_number);
		}
		else if (line_number == 0)
		{
			if (error_code < EC_MAX_ERROR_CODE)
				if (parameter == NULL)
					printf("%s: warning: %s\n",input_filename,error_string[error_code]);
				else
					printf("%s: warning: %s %s\n",input_filename,error_string[error_code],parameter);
			else
				printf("%s: warning: unknown error/warning.\n",input_filename);
		}
		else
		{
			if (error_code < EC_MAX_ERROR_CODE)
				if (parameter == NULL)
					printf("%s:%d: warning: %s\n",input_filename,line_number,error_string[error_code]);
				else
					printf("%s:%d: warning: %s %s\n",input_filename,line_number,error_string[error_code],parameter);
			else
				printf("%s:%d: warning: unknown error/warning.\n",input_filename,line_number);
		}
	}
}

