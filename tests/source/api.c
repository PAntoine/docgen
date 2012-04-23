/**-------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * @file: 	api
 * @group:	config_file_api
 * @desc:	This file holds the test for the api structures.
 *          It handles the bit that are used to make the API functions and
 *          all the new atoms work and can be used.
 *
 * @author: pantoine
 * Date  : 26/02/2012 12:01:24
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------*
 * Silly test functions for some corner cases.
 *--------------------------------------------------------------------------------*/

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: 	silly_1
 *--------------------------------------------------------------------------------*/
extern const unsigned int	silly_1 (unsigned char*const*const payload,  unsigned int payload_size, unsigned int offset, unsigned int* value)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: 	silly_2
 *--------------------------------------------------------------------------------*/
extern const unsigned int*	silly_2 (unsigned char*const*const payload, const unsigned int *payload_size, unsigned int offset, unsigned int*const value)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: 	silly_3
 * @group: test_2_group
 *--------------------------------------------------------------------------------*/
extern unsigned int	silly_3 (unsigned char* payload,  unsigned int payload_size, unsigned int offset, unsigned int* value)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: 	silly_4
 * @group: test_2_group
 *--------------------------------------------------------------------------------*/
extern unsigned int	silly_4 (unsigned char*const payload)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: 	silly_5
 *--------------------------------------------------------------------------------*/
static unsigned int	silly_5 (const unsigned char*const payload)
{
	unsigned int result;

	return result;
}


/*--------------------------------------------------------------------------------*
 * The functions used to read the .ini files.
 *--------------------------------------------------------------------------------*/

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: 	DGTF_IF_GetListInt
 * @desc: 	This function gets an it from the list.
 * 			line two of the multiline comment is here.
 * 			might as well have a third-line or a guess it would not be fair.
 * @ignore:
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_GetListInt (unsigned char* payload,  unsigned int payload_size, unsigned int offset, unsigned int* value)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api:DGTF_IF_GetListString
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_GetListString (unsigned char* payload, unsigned int payload_size, unsigned int offset, unsigned char** string, unsigned int* string_length)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_GetListHexItem
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_GetListHexItem	(unsigned char* payload, unsigned int payload_size, unsigned int index)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api:	DGTF_CloseConfig
 *--------------------------------------------------------------------------------*/
void 			DGTF_CloseConfig ( void )
{
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_ReadLine
 *--------------------------------------------------------------------------------*/
unsigned int 	DGTF_IF_ReadLine (unsigned int* section,unsigned int* id,DGTF_IF_CONFIG_TYPE* config_type, DGTF_IF_PAYLOAD_TYPE* type,unsigned char** payload, unsigned int* payload_length, unsigned int* object)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_ExtendConfig
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_ExtendConfig(DGTF_IF_CONFIGURATION* base, DGTF_IF_CONFIGURATION* extension)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api:	DGTF_OpenConfig
 *--------------------------------------------------------------------------------*/
int DGTF_OpenConfig( unsigned char* file_name, DGTF_IF_CONFIGURATION* configuration)
{
	int result;

	return result;
}

/*--------------------------------------------------------------------------------*
 * Helper functions for writing the ini file.
 *--------------------------------------------------------------------------------*/

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_StartList
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_StartList(unsigned char* buffer, unsigned int buffer_offset, unsigned char* name, unsigned int name_length, unsigned int general)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddListString
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddListString(unsigned char* buffer, unsigned int buffer_offset, unsigned char* string, unsigned int string_length)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddListHexPair
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddListHexPair(unsigned char* buffer, unsigned int buffer_offset, unsigned int one, unsigned int two)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddListHex
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddListHex(unsigned char* buffer, unsigned int buffer_offset, unsigned int one)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddListInt
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddListInt(unsigned char* buffer, unsigned int buffer_offset, unsigned int one)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_EndList
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_EndList(unsigned char* buffer, unsigned int buffer_offset, unsigned int general)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddObjectList
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddObjectList(unsigned char* buffer, unsigned int buffer_offset, unsigned int object, unsigned char* data, unsigned int data_length)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddObjectHex
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddObjectHex(unsigned char* buffer, unsigned int buffer_offset, unsigned int object, unsigned int value)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddSection
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddSection(unsigned char* buffer, unsigned int buffer_offset, unsigned char* section, unsigned int section_length)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddString
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddString(unsigned char* buffer, unsigned int buffer_offset, unsigned char* name, unsigned int name_length, unsigned char* string, unsigned int string_length)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddHex
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddHex(unsigned char* buffer, unsigned int buffer_offset, unsigned int formatted, unsigned char* name, unsigned int name_length, unsigned char* data, unsigned int data_length)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddBoolean
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddBoolean(unsigned char* buffer, unsigned int buffer_offset, unsigned char* name, unsigned int name_length, int value)
{
	unsigned int result;

	return result;
}

/**---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_AddInt
 * @desc: 	This function adds an integer to the buffer.
 * @action: DDTF_AddInt adds an integer to the output packet. 
 *   
 *          It adds the integer in the correct format for the packet that is
 *          being sent.
 * @parameter:	buffer			This is the buffer to add the integer to.
 * @parameter:	buffer_offset	The offset of the buffer to write to.
 * @parameter:	name			The name of the int to add.
 * @parameter:	name_length		The length of the name to add.
 * @parameter:	value			The integer to write.
 *
 * @returns:	0 	If it worked.
 * @returns:	1	If it failed.
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_AddInt(unsigned char* buffer, unsigned int buffer_offset, unsigned char* name, unsigned int name_length, int value)
{
	unsigned int result;

	return result;
}

/**i---- FUNCTION -----------------------------------------------------------------*
 * @api: DGTF_IF_IsInExtension
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_IsInExtension(unsigned int offset, unsigned int id)
{
	unsigned int result;

	return result;
}


#endif

