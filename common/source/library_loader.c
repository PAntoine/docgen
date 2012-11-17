/*--------------------------------------------------------------------------------*
 *				 Document Generator Project
 *					by Peter Antoine
 *
 *	 @file: library_loader
 *	 @desc: This file holds a set of functions that are used to load a plugin
 *			library.
 *
 *			It will look for the plugin libraries in certain locations then try
 *			and find if a particular function is within the library, if so then
 *			it will return the library handle. If the function is not there then
 *			the function will unload the library.
 *
 * @author: 
 *	 @date: 05/07/2012 19:07:47
 *--------------------------------------------------------------------------------*
 *					   Copyright (c) 2012 Peter Antoine
 *							  All rights Reserved.
 *					  Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <dirent.h>
#include "plugin.h"
#include "document_generator.h"

static	PLUGIN_LIBRARY	g_plugin_list = {0,0,NULL,NULL,NULL,NULL};

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: load_plugins
 * @desc: This function will load any libraries that have been found. It will 
 *        keep track of any plugins that were found.
 *--------------------------------------------------------------------------------*/
void  load_plugins(char* directory, unsigned int type)
{
	DIR*					dir;
	char					library_path[MAX_FILENAME];
	void					*lib_handle;
	unsigned int			length;
	unsigned int  			version;
	unsigned int  			format_size;
	struct dirent			*dp;
	DGOF_PLUGIN_get_details	plugin_get_details;
	PLUGIN_LIBRARY*			plugin;

	strncpy(library_path,directory,MAX_FILENAME);
	library_path[MAX_FILENAME-1] = '\0';
	length = strlen(library_path);
	
	library_path[length++] = '/';
			
	if ((dir = opendir(directory)) != NULL)
	{
		/* Loop through directory entries. */
		while ((dp = readdir(dir)) != NULL) 
		{
			/* Get entry's information. */
			strncpy(&library_path[length],dp->d_name,MAX_FILENAME - length);
			lib_handle = dlopen(library_path, RTLD_LAZY);

			if (lib_handle != NULL) 
			{
				/* don't you just hate ridiculous type checking? f**king committees */
				*(void**)(&plugin_get_details) = dlsym(lib_handle, "DGOF_plugin_get_details");

				if (plugin_get_details != NULL)
				{
					format_size = 0;

					if (plugin_get_details(&version,&format_size,&plugin))
					{
						if (IS_VERSION_VALID(version) && plugin->type == type)
						{
							/* Ok, plugin successfully loaded and initialised */
							plugin->library_handle = lib_handle;

							/* add plugin to list of loaded plugins */
							plugin->next = g_plugin_list.next;
							g_plugin_list.next = plugin;

							/* register the plugin with the caller */
							register_format(plugin->format);
						}
						else
						{
							/* signature did not match - unload */
							dlclose(lib_handle);
						}
					}
					else
					{
						dlclose(lib_handle);
					}
				}
				else
				{
					dlclose(lib_handle);
				}
			}
		}

		closedir(dir);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * @name: unload_plugins
 * @desc: This function will unload the loaded plugins.
 *        It will call the release function that will tell the plugin to do any
 *        tidy-ups it needs to do then remove the libraries.
 *--------------------------------------------------------------------------------*/
void	unload_plugins(void)
{
	PLUGIN_LIBRARY*	hold;
	PLUGIN_LIBRARY*	current_library;

	current_library = g_plugin_list.next;
	g_plugin_list.next = NULL;

	while (current_library != NULL)
	{
		if (current_library->release != NULL)
		{
			current_library->release();
		}

		hold = current_library;
		current_library = current_library->next;
		
		hold->next = NULL;
		dlclose(hold->library_handle);
	}
}

