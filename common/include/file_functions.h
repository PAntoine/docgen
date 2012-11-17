/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : file_functions
 * Desc  : This header file describes the file functions.
 *
 * Author: peterantoine
 * Date  : 07/09/2012 07:41:31
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __FILE_FUNCTIONS_H__
#define __FILE_FUNCTIONS_H__

unsigned int	extend_path(unsigned char* path_name, unsigned int* path_length, unsigned char* extend_name, unsigned int extend_length);
void			get_filename(unsigned char* path, unsigned char** file_name, unsigned int* file_name_length);
unsigned int	copy_extend_path_name(unsigned char* new_file_buffer, unsigned char* path, unsigned int path_length, unsigned char* extend_name, unsigned int extend_length);
unsigned int	copy_file(unsigned char* source_file_name, unsigned char* new_file_name);
void			get_filename_with_ext(unsigned char* path, unsigned char** file_name, unsigned int* file_name_length);
unsigned int	copy_files_to_directory(unsigned char** source_path, unsigned int number_files, unsigned char* directory_name, unsigned int directory_name_length);

#endif

