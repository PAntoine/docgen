/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : file_functions
 * Desc  : This file holds the generic file functions.
 *
 * Author: peterantoine
 * Date  : 07/09/2012 07:38:08
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

#include "error_codes.h"
#include "file_functions.h"

/*----- FUNCTION -----------------------------------------------------------------*
 * @name	copy_files_to_directory
 * @desc	This function will copy the file to the directory.
 *
 * @param	filename		The file name(s) to copy.
 * @param	number_files	The number of files to copy.
 * @param	directory_name	The directory name to copy the file to.
 *
 * @returns	0	if failed to copy the file
 * @returns	1	if the file was successfully copied.
 *--------------------------------------------------------------------------------*/
unsigned int	copy_files_to_directory(unsigned char** source_path, unsigned int number_files, unsigned char* directory_name, unsigned int directory_name_length)
{
	unsigned int		count;
	unsigned int		result = EC_OK;
	unsigned char		new_file_name[MAX_FILENAME];
	unsigned char*		filename;
	unsigned int		filename_length;
	
	for (count=0;count<number_files;count++)
	{
		/* build directory target */
		get_filename_with_ext(source_path[count],&filename,&filename_length);

		if (filename_length > 0)
		{
			/* make the target name */
			copy_extend_path_name(new_file_name,directory_name,directory_name_length,filename,filename_length);

			result = copy_file(source_path[count],new_file_name);
		}
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : extend_path
 * Desc : This function will extend the path by adding a directory to the path
 *        it will check to see if the directory exists and creates the directory
 *        if it can.
 *--------------------------------------------------------------------------------*/
unsigned int	extend_path(unsigned char* path_name, unsigned int* path_length, unsigned char* extend_name, unsigned int extend_length)
{
	unsigned int	result = 0;
	unsigned int	name_length = *path_length;
	struct stat		stat_buf;

	/* add the format directory */
	memcpy(&path_name[name_length],extend_name,extend_length);
	name_length += extend_length;
	path_name[name_length] = 0;

	/* TODO: the path separator needs to be part of the configuration, maybe a build flag */
	if (stat((char*)path_name,&stat_buf) == -1)
	{
		/* directory does not exist need to create it.
		 * NOTE: intentional use of octal and uses the users umask to set the permissions.
		 */
		if (mkdir((char*)path_name,0777) != -1)
		{
			result = 1;
			path_name[name_length] = '/';
			*path_length = name_length+1;
		}
	}
	else if (S_ISDIR(stat_buf.st_mode))
	{
		/* TODO: here - the path separator is being used */
		/* it already exists and it a directory */
		result = 1;
		path_name[name_length] = '/';
		*path_length = name_length+1;
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : copy_extend_path_name
 * Desc : This function will copy and extend the path name into the new buffer.
 *        The new buffer should be MAX_FILENAME as all other file names.
 *--------------------------------------------------------------------------------*/
unsigned int	copy_extend_path_name(unsigned char* new_file_buffer, unsigned char* path, unsigned int path_length, unsigned char* extend_name, unsigned int extend_length)
{
	unsigned int result = path_length;

	if (path_length + extend_length + 1 < MAX_FILENAME)
	{
		memcpy(new_file_buffer,path,path_length);

		if (result > 0 && new_file_buffer[result-1] != '/')
		{
			new_file_buffer[result++] = '/';
		}

		memcpy(&new_file_buffer[result],extend_name,extend_length);
		result += extend_length;
		new_file_buffer[result++] = '\0';
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : copy_file
 * Desc : Boring function to copy a file. 
 *        Here mostly because the posix C copyfile function is not supported as
 *        standard across platforms. So here is an inefficient and boring version.
 *
 *        As these will be using the system functions, it expects C null-terminated
 *        strings as names.
 *--------------------------------------------------------------------------------*/
unsigned int	copy_file(unsigned char* source_file_name, unsigned char* new_file_name)
{
	int				source_file;
	int				dest_file;
	unsigned int	result = EC_OK;
	unsigned int	bytes_read;
	unsigned char	copy_buffer[8 * 1024];

	if ((source_file = open((char*)source_file_name,READ_FILE_STATUS)) == -1)
	{
		result = EC_FAILED_TO_OPEN_INPUT_FILE;
		raise_warning(0,result,(unsigned char*)source_file_name,NULL);
	}
	else if ((dest_file = open((char*)new_file_name,WRITE_FILE_STATUS,WRITE_FILE_PERM)) == -1)
	{
		close(source_file);

		result = EC_FAILED_TO_OPEN_OUTPUT_FILE;
		raise_warning(0,result,(unsigned char*)new_file_name,NULL);
	}
	else
	{
		/* both files are open - we are good to copy */
		while ((bytes_read = read(source_file,copy_buffer,sizeof(copy_buffer))) > 0)
		{
			if (write(dest_file,copy_buffer,bytes_read) != bytes_read)
			{
				result = EC_FAILED_TO_WRITE_TO_OUTPUT_FILE;
				raise_warning(0,result,(unsigned char*)new_file_name,NULL);
				break;
			}
		}

		/* tidy up */
		close(dest_file);
		close(source_file);
	}

	return result;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : get_filename
 * Desc : This function will return the filename part of the path. It will remove
 *        the extension from the filename and only return the filename and the
 *        length of the filename.
 *--------------------------------------------------------------------------------*/
void	get_filename(unsigned char* path, unsigned char** file_name, unsigned int* file_name_length)
{
	unsigned int pos = 0;
	unsigned int start = 0;
	unsigned int length = 0;
	unsigned int last_dot = UINT_MAX;

	while (path[pos] != '\0')
	{
		/* TODO: fix this as above - should be a configuration thing (path separator) */
		if (path[pos] == '/')
		{
 			if (path[pos+1] != '\0')
				start = pos+1;
		}
		else if (path[pos] == '.')
		{
			last_dot = pos;
		}
		pos++;
	}

	if (last_dot == UINT_MAX)
	{
		length = pos - start;
	}
	else
	{
		length = last_dot - start;
	}

	*file_name = &path[start];
	*file_name_length = length;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : get_filename_with_ext
 * Desc : This function will return the filename part of the path.
 *--------------------------------------------------------------------------------*/
void	get_filename_with_ext(unsigned char* path, unsigned char** file_name, unsigned int* file_name_length)
{
	unsigned int pos = 0;
	unsigned int start = 0;

	while (path[pos] != '\0')
	{
		/* TODO: fix this as above - should be a configuration thing (path separator) */
		if (path[pos] == '/')
		{
 			if (path[pos+1] != '\0')
				start = pos+1;
		}

		pos++;
	}

	*file_name = &path[start];
	*file_name_length = pos - start;
}

