/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : file_header
 * Desc  : This file holds the intermediate file header. 
 *         It is in its own file as it needs to be shared between the different
 *         components.
 *
 * Author: pantoine
 * Date  : 13/11/2011 11:02:46
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include "document_generator.h"

unsigned char	g_file_header[] = {	0x70,0x64,0x73,0x63,				/* magic number */
									VERSION_MAJOR, VERSION_MINOR,
									0xff,0x0ff,0x0ff,0xff,0xff,0xff,	/* dd:mm:yy:hh:mm:ss */
									0xff,0xff,							/* number of records */
									0xff,0xff};							/* name size */

unsigned int	g_file_header_size = sizeof(g_file_header);
unsigned int	g_date_offset = 6;
unsigned int	g_number_records_offset = 12;

