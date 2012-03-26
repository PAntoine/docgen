/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : input_formats
 * Desc  : This file defines the input formats that are supported.
 *
 * Author: 
 * Date  : 05/03/2012 19:58:52
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include "input_formats.h"

/*--------------------------------------------------------------------------------*
 * static values
 *--------------------------------------------------------------------------------*/
static unsigned char	h_fmt[] = "h";
static unsigned char	c_fmt[] = "c";
static unsigned char	cpp_fmt[] = "cpp";

#define C_FMT_LENGTH 	(sizeof(c_fmt) - 1)
#define H_FMT_LENGTH	(sizeof(h_fmt) - 1)
#define CPP_FMT_LENGTH	(sizeof(cpp_fmt) - 1)

/*--------------------------------------------------------------------------------*
 * Input format structures.
 *--------------------------------------------------------------------------------*/
#define c_style_functions	c_find_comment, c_end_comment, c_function_start, c_find_parameter, c_function_start_level, c_function_end_level, c_function_end

SOURCE_FORMAT	input_formats[] = 
{
	{c_fmt,		C_FMT_LENGTH,	c_style_functions},
	{h_fmt,		H_FMT_LENGTH,	c_style_functions},
	{cpp_fmt,	CPP_FMT_LENGTH,	c_style_functions}
};

/*--------------------------------------------------------------------------------*
 * Keep the namespace clean.
 *--------------------------------------------------------------------------------*/
#undef	c_style_functions
#undef	C_FMT_LENGTH  
#undef	H_FMT_LENGTH  
#undef	CPP_FMT_LENGTH

/*--------------------------------------------------------------------------------*
 * Generic input functions.
 *--------------------------------------------------------------------------------*/
unsigned int input_decode_format(unsigned char* file_name)
{
	unsigned int	pos = 0;
	unsigned int	index;
	unsigned int	count;
	unsigned int	found_last_dot = UINT_MAX;
	unsigned int	result = INPUT_FORMAT_INVALID;

	while (file_name[pos] != 0)
	{
		if (file_name[pos] == '.')
		{
			found_last_dot = pos+1;
		}

		pos++;
	}

	if (found_last_dot != UINT_MAX)
	{
		for (count=0; count < INPUT_FORMATS_MAX; count++)
		{
			if ((pos-found_last_dot) == input_formats[count].format_name_length)
			{
				for (index=0;index < input_formats[count].format_name_length; index++)
				{
					if (file_name[found_last_dot+index] != input_formats[count].format_name[index])
					{
						break;
					}
				}

				if (index == input_formats[count].format_name_length)
				{
					result = count;
					break;
				}
			}
		}
	}
}


