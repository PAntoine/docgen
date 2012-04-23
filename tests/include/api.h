/**-------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * @file: api.h
 * @desc: This is a test file for the API configuration.
 *        It just defines some types that will be referenced within the api.c
 *        test file.
 *
 * @author: 
 * Date  : 12/03/2012 10:41:45
 *--------------------------------------------------------------------------------*
 *                     @copyright: (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __API_H__
#define __API_H__

/*--------------------------------------------------------------------------------*
 * macros
 *--------------------------------------------------------------------------------*/

/**--------------------------------------------------------------------------------*
 * @constants: test constants
 * @description: A basic single set of test constants just to produce some output
 *               for the test to work. They don't make sense and they dont fully
 *               stretch the generator, but they are a start.
 *--------------------------------------------------------------------------------*/

#define DGTF_IF_EXTENTION_OFFSET	(64 * 1024)				/** @brief: when extending the ini file, this number offsets the extension sets */
#define DGTF_AF_EXTENTION_OFFSET	(14 * 1024)				/** @brief: more stuff for testing */
#define DGTF_AF_ONE					14						/** @brief: more stuff for testing */
#define DGTF						"gffgfgfgfg gfgfgfgg"	/** @brief: more stuff for testing */

#define DGTF_IF_IdIsSystem(x)	(((x)<DGTF_IF_EXTENTION_OFFSET?1:0))

/**--------------------------------------------------------------------------------*
 * @end_constants:
 *--------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------*
 * The standard defines.
 *--------------------------------------------------------------------------------*/

/**--------------------------------------------------------------------------------*
 * @type: DGTF_IF_PAYLOAD_TYPE
 * @group: test_1
 * @desc:	This enum defines the different types that can be added as payload.
 *--------------------------------------------------------------------------------*/
typedef enum
{
	DGTF_IFPT_NONE,					/** @brief: This is an invalid item */
	DGTF_IFPT_BOOLEAN,				/** @brief: This is a boolean item 	*/
	DGTF_IFPT_INTEGER = 5,			/** @brief: This is an integer item */
	DGTF_IFPT_STRING=0x30,			/** @brief: This is a string item	*/
	DGTF_IFPT_HEX_STRING	=1,		/** @brief:	This is a string item 	*/
	DGTF_IFPT_LIST,					/** @brief:	This is a list item		*/
	DGTF_IFPT_GENERAL_LIST			/** @brief:	This is a general item 	*/

} DGTF_IF_PAYLOAD_TYPE;

/**--------------------------------------------------------------------------------*
 * @type:	DGTF_IF_CONFIG_TYPE
 * @group:	test_1
 * @desc:	This enum defines the config sub-item type.
 *--------------------------------------------------------------------------------*/
typedef enum
{
	DGTF_IFCT_INVALID,
	DGTF_IFCT_STRING,
	DGTF_IFCT_NUMERIC

} DGTF_IF_CONFIG_TYPE;

/* this must be the first item of the item list */
#define DGTF_IF_ITEM_INVALID	(0)

/**--------------------------------------------------------------------------------*
 * @type: DGTF_IF_SECTION_NAME
 * @group: test_1
 * @desc:
 *
 * The following are the .ini handlers.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned int		id;
	unsigned char*		name;
	unsigned int		length;

} DGTF_IF_SECTION_NAME;

/**--------------------------------------------------------------------------------*
 * @type: DGTF_IF_CONFIG_ITEM
 * @group: test_1
 * @desc:
 * The description of the config item.
 *--------------------------------------------------------------------------------*/
typedef struct
{
	unsigned int		id;				/** @brief: just testing this  - id*/
	unsigned int		section_id;		/** @brief: just testing this  - section_id */
	DGTF_IF_CONFIG_TYPE	type;			/** @brief: just testing this  - type */
	unsigned char*		name;			/** @brief: just testing this  - name */
	unsigned int		length;			/** @brief: just testing this  - length */

} DGTF_IF_CONFIG_ITEM;

/**--------------------------------------------------------------------------------*
 * @type: DGTF_IF_SECTION_NAME
 * @desc:
 *
 * The following are the .ini handlers.
 *--------------------------------------------------------------------------------*/
typedef struct tag_hnut_if_configuration
{
	unsigned int			num_sections;
	unsigned int			num_config_items;
	DGTF_IF_SECTION_NAME*	section;
	DGTF_IF_CONFIG_ITEM*	config_item;		/** @brief: this really needs a description */

	struct tag_hnut_if_configuration*	next;

} DGTF_IF_CONFIGURATION;

/**--------------------------------------------------------------------------------*
 * @type: DGTF_IF_TEST_CONFIGURATION_2
 * @desc:
 *
 * ### The following are the .ini handlers - just another description.
 *--------------------------------------------------------------------------------*/
typedef union tag_hnut_if_configuration_2
{
	unsigned int			num_sections;
	unsigned int			num_config_items;
	DGTF_IF_SECTION_NAME*	section;
	DGTF_IF_CONFIG_ITEM*	config_item;

	struct
	{
		unsigned int			num_sections;
		unsigned int			num_config_items;
	} hell;
	struct tag_hnut_if_configuration*	next;

} DGTF_IF_TEST_CONFIGURATION_2;

/**--------------------------------------------------------------------------------*
 * @type: DGTF_IF_TEST_CONFIGURATION_3
 * @desc:
 *
 * --- The following are the .ini handlers.
 *--------------------------------------------------------------------------------*/
typedef struct tag_hnut_if_configuration_3{
	unsigned int			num_sections;
	unsigned int			num_config_items;
	DGTF_IF_SECTION_NAME*	section;
	DGTF_IF_CONFIG_ITEM*	config_item;

	union
	{
		unsigned int			num_sections;
		unsigned int			num_config_items;
	} hell;
	struct tag_hnut_if_configuration*	next;

} DGTF_IF_TEST_CONFIGURATION_3;

/**--------------------------------------------------------------------------------*
 * @type: DGTF_IF_TEST_CONFIGURATION_4
 * @desc:
 *
 * --- The following are the .ini handlers.
 *--------------------------------------------------------------------------------*/
typedef struct tag_hnut_if_configuration_4{
	unsigned int			num_sections;
	unsigned int			num_config_items;
	DGTF_IF_SECTION_NAME*	section;
	DGTF_IF_CONFIG_ITEM*	config_item;

	union
	{
		unsigned int			num_sections;
		unsigned int			num_config_items;
	} hell;
	struct tag_hnut_if_configuration*	next;

} DGTF_IF_TEST_CONFIGURATION_4;


#endif

