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

/**-------------------------------------------------------------------------------*
 * @constants: 		Base64 Lookup Table
 * @description:	These constants are the lookup tables for the base64 encoding.
 *--------------------------------------------------------------------------------*/
static const char encoding_string[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char decoded_byte[256] = {	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x3f,
										0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
										0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x00,0x00,0x00,0x00,0x00,
										0x00,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
										0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

/**--------------------------------------------------------------------------------*
 * The following are lookup tables for the date functions.
 *--------------------------------------------------------------------------------*/
/** @brief: normal */
unsigned char	wday_month_starts[2][12] = {
											{0,3,3,6,1,4,6,2,5,0,3,5},					
											{6,2,3,6,1,4,6,2,5,0,3,5}					/* leap year */
										   };

unsigned char	month_length[2][12] = {
										{31,28,31,30,31,30,31,31,30,31,30,31},			/* normal */
										{31,29,31,30,31,30,31,31,30,31,30,31}			/* leap year */
									  };

unsigned short	day_month_starts[2][12]= {
											{0,31,59,90,120,151,181,212,243,273,304,334},	/* normal */
											{0,31,60,91,121,152,182,213,244,274,305,335}	/* leap year */
										 };
unsigned int offset_month[2][12]	=	{
											{31,31,28,31,30,31,30,31,31,30,31,30},
			 								{31,31,29,31,30,31,30,31,31,30,31,30}
										};
/** @brief does this happen always? */


char*	month_text[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};
char*	day_text[] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday"};



static	char*	severity_text[] = { "NONE   :",
									"EVENT  :",
									"WARNING:",
									"COMMENT:"};

static	char*	event_text[]	= { "NONE",
									"START",
									"STOP",
									"LOGIN",
									"LOGOUT",
									"MESSAGE"};

static unsigned int data_bit[] = 
{
	0x00000080, 0x00000040, 0x00000020, 0x00000010, 0x00000008, 0x00000004, 0x00000002, 0x00000001, 
	0x00008000, 0x00004000, 0x00002000, 0x00001000, 0x00000800, 0x00000400, 0x00000200, 0x00000100,
	0x00800000, 0x00400000, 0x00200000, 0x00100000, 0x00080000, 0x00040000, 0x00020000, 0x00010000,
	0x80000000, 0x40000000, 0x20000000, 0x10000000, 0x08000000, 0x04000000, 0x02000000, 0x01000000
};


const HNMT_MIME_TYPE_MAP	hnmt_mime_types[] = {
		{HNMT_MTM_TEXT_HTML			,(unsigned char*)"text/html",							sizeof("text/html")-1},
		{HNMT_MTM_TEXT_CSS			,(unsigned char*)"text/css",							sizeof("text/css")-1},
		{HNMT_MTM_TEXT_CAL			,(unsigned char*)"text/calendar",						sizeof("text/calendar")-1},
		{HNMT_MTM_TEXT_JAVASCRIPT	,(unsigned char*)"text/javascript",						sizeof("text/javascript")-1},
		{HNMT_MTM_APPL_JAVASCRIPT	,(unsigned char*)"application/javascript",				sizeof("application/javascript")-1},
		{HNMT_MTM_APPL_FORM			,(unsigned char*)"application/x-www-form-urlencoded",	sizeof("application/x-www-form-urlencoded")-1},
		{HNMT_MTM_APPL_JSON			,(unsigned char*)"application/json",					sizeof("application/json")-1},
		{HNMT_MTM_IMAGE_PNG			,(unsigned char*)"image/png",							sizeof("image/png")-1},
		{HNMT_MTM_AUDIO_BASIC		,(unsigned char*)"audio/basic",							sizeof("audio/basic")-1},
		{HNMT_MTM_AUDIO_MID			,(unsigned char*)"audio/mid",							sizeof("audio/mid")-1},
		{HNMT_MTM_AUDIO_MPEG		,(unsigned char*)"audio/mpeg",							sizeof("audio/mpeg")-1},
		{HNMT_MTM_AUDIO_X_AIFF		,(unsigned char*)"audio/x-aiff",						sizeof("audio/x-aiff")-1},
		{HNMT_MTM_AUDIO_MPEGURL		,(unsigned char*)"audio/x-mpegurl",						sizeof("audio/x-mpegurl")-1},
		{HNMT_MTM_AUDIO_REALAUDIO	,(unsigned char*)"audio/x-pn-realaudio",				sizeof("audio/x-pn-realaudio")-1},
		{HNMT_MTM_AUDIO_X_WAV		,(unsigned char*)"audio/x-wav",							sizeof("audio/x-wav")-1},
		{HNMT_MTM_VIDEO_MPEG2TS		,(unsigned char*)"video/MP2T",							sizeof("video/MP2T")-1},

		{HNMT_MTM_UNDEFINED			,0L,													0}			/* invalid mime type - stop bad code blowing up! */
};

static unsigned char	fred_bloggs = 0x0000;
static unsigned char	fred_bloggs1 = "DDDDDDDD";
static unsigned char 	fred_blocks3 = FFFFFFFFFFF;

/** @end_constants: */

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
 * @api			 DGTF_IF_AddInt
 * @desc	 	 This function adds an integer to the buffer.
 *
 * @action :DDTF_AddInt adds an integer to the output packet. 
 *         :
 *         :It adds the *integer* in the correct format for the packet that is
 *         :being sent.
 *         :this.
 *         :
 *         :    +----------------------------------------+
 *         :    |   dfdsfsdfsdfds   |  fdfsdfsd          |
 *         :    +----------------------------------------+
 *         :
 * @parameter	buffer			This is the buffer to add the integer to.
 * @parameter	buffer_offset	The offset of the buffer to write to.
 * @parameter	name			The name of the int to add, this description needs to be really long so that we can test the clipping of the thing so that it works correctly.
 * @parameter	name_length		The length of the name to add.
 * @parameter	value			The integer to write.
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
 * @action	This thing doers some *stuff* on its _own_ so there.
 *          But we really need a multiline one here so I can test that it is 
 *          banging out all the text and it looks really nice. It is a thing that
 *          is nice to do so that it can all be seen. We are there so the thing 
 *          is fine and full of noise. This is enough I am getting bored of type
 *          stuff in now.
 *
 *          But, I forgot that I allow for paras in this stuff so lets see if it
 *          comes out, if you are reading this is paras it probably means that
 *          I had to fix it.
 *--------------------------------------------------------------------------------*/
unsigned int	DGTF_IF_IsInExtension(unsigned int offset, unsigned int id)
{
	unsigned int result;

	return result;
}


#endif

