/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: plugin
 *   @desc: This file holds the structures for controlling the plugins.
 *
 * @author: pantoine
 *   @date: 06/07/2012 08:20:45
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

typedef struct tag_plugin_library	PLUGIN_LIBRARY;

/*--------------------------------------------------------------------------------*
 * definitions for the plugins
 *--------------------------------------------------------------------------------*/
#define PLUGIN_SIGNATURE		((unsigned int)(0x47F60000))    /* Doc Gen Object Format (D=4,G=7,O=F,F=6) */
#define SIGNATURE_MASK			((unsigned int)(0xFFFF0000))
#define MAJOR_MASK				((unsigned int)(0x0000FF00))
#define MINOR_MASK				((unsigned int)(0x000000FF))
	
#define	MAKE_VERSION(major,minor)	((PLUGIN_SIGNATURE | (((unsigned char)major) << 8) | ((unsigned char)minor)))
#define IS_VERSION_VALID(version)	((((unsigned int)version) & SIGNATURE_MASK) == PLUGIN_SIGNATURE)
#define GET_MAJOR(version)			(((unsigned int)version) & MAJOR_MASK)
#define GET_MINOR(version)			(((unsigned int)version) & MINOR_MASK)

/* plugin types */
#define	DGOF_PLUGIN_OUTPUT		((unsigned int)(0x00000001))	/* it is an output format plugin */

/*--------------------------------------------------------------------------------*
 * Functions that are used by the plugins.
 *--------------------------------------------------------------------------------*/
void			load_plugins(char* directory, unsigned int type);
void			unload_plugins(void);
unsigned int	register_format(void* format);			/* this function is defined in the calling app - returns an 0 if it did not register */

/*--------------------------------------------------------------------------------*
 * Plugin Functions.
 *
 * All plugins must define the following function, be able to return the plugin
 * structure.
 *--------------------------------------------------------------------------------*/
typedef	unsigned int	(*DGOF_PLUGIN_get_details)	(unsigned int* version, unsigned int* format_size, PLUGIN_LIBRARY** formats);
typedef	unsigned int	(*DGOF_PLUGIN_RELEASE)		(void);

unsigned int	DGOF_plugin_get_details(unsigned int* version, unsigned int* format_size, PLUGIN_LIBRARY** formats);
unsigned int	DGOF_plugin_release(void);

/*--------------------------------------------------------------------------------*
 * Plugin Structures
 *--------------------------------------------------------------------------------*/
struct tag_plugin_library
{
	unsigned int			version;
	unsigned int			type;
	DGOF_PLUGIN_RELEASE		release;
	void*					library_handle;			
	void*					format;

	struct tag_plugin_library*	next;
};

#endif

