/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : error_codes
 * Desc  : The file holds the functions and the error code messages for the 
 *         errors in the system.
 *
 * Author: pantoine
 * Date  : 12/11/2011 16:52:17
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __ERROR_CODES_H__
#define __ERROR_CODES_H__

#include "document_generator.h"

typedef enum
{
	EC_OK,
	EC_FAILED,
	EC_PARSE_FAILED,
	EC_UNKNOWN_MACRO,
	EC_MACRO_NOT_GROUP,
	EC_INVALID_ITEM_NAME,
	EC_INVALID_NAME_OR_NAME_NOT_FOUND,
	EC_UNKNOWN_COMMAND,
	EC_WEIRD_STUFF_AFTER_COMMAND,
	EC_FAILED_TO_OPEN_OUTPUT_FILE,
	EC_INTERNAL_ERROR_UNKNOWN_RECORD_TYPE,
	EC_INPUT_FILE_BUILT_WITH_LATER_MAJOR_VERSION,
	EC_PROBLEM_WITH_INPUT_FILE,
	EC_UNKNOWN_ATOM,
	EC_MULTIPLE_STATES_IN_ONE_BLOCK,
	EC_MIXED_DIAGRAM_TYPES_IN_BLOCK,
	EC_WRONG_ATOM_IN_BLOCK,
	EC_ATOM_NOT_ALLOWED_TO_HAVE_GROUP,
	EC_GROUP_NAME_TOO_LONG,
	EC_ATOM_NAME_TOO_LONG,
	EC_DUPLICATE_TRANSITION,
	EC_BLOCK_MUST_HAVE_DEFINER,
	EC_STATE_MUST_HAVE_TRANSITION,
	EC_DUPLICATE_TRIGGER,
	EC_UNDEFINDED_REFERENCED_STATE_FOUND,
	EC_STATE_MACHINE_DOES_NOT_HAVE_START,
	EC_DUPLICATE_CONDITION_IN_TRANSITION,
	EC_CANNOT_HAVE_TRIGGER_AND_CONDITION_IN_TRANSITION,	
	EC_MULTIPLE_TRIGGER,
	EC_UNDEFINED_TRIGGER,
	EC_MULTIPLE_TIMELINES_IN_ONE_BLOCK,	
	EC_DUPLICATE_MESSAGE,
	EC_DUPLICATE_AFTER,
	EC_DUPLICATE_SEQUENCE,
	EC_MULTIPLE_TO_TIMELINES_IN_ONE_BLOCK,
	EC_SEQUENCE_NODE_MISSING_TIMELINE,
	EC_ONLY_MESSAGES_CAN_DEPEND_ON_MESSAGES,
	EC_MULTIPLE_FUNCTION_ATOMS_WITHOUT_FUNCTION,
	EC_NESTED_FUNCTION_DEFINITIONS_NOT_ALLOWED,
	EC_MULTIPLE_DEFINITIONS_OF_FUNCTION,
	EC_CANNOT_CALL_HAVE_THESE_ATOMS_IN_A_FUNCTION,	
	EC_MAX_FUNCTION_DEPTH_REACHED,	
	EC_AFTER_NODE_DOES_NOT_EXIST,
	EC_UNDEFINED_FUNCTION,
	EC_SEND_AND_NO_WAIT,
	EC_RESPOND_AND_NO_SEND,
	EC_MESSAGE_SENT_TO_AN_ANCESTOR,
	EC_MAX_ERROR_CODE

} ERROR_CODES;

/*--------------------------------------------------------------------------------*
 * Raise Warning
 *--------------------------------------------------------------------------------*/
void raise_warning ( unsigned int line_number, unsigned int error_code, const unsigned char* parameter, SOURCE_REFERENCE* reference );

#endif

