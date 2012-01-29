/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : utilities
 * Desc  : This file holds the implementation of the utility functions.
 *
 * Author: pantoine
 * Date  : 10/11/2011 19:43:01
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

/*---  FUNCTION  ----------------------------------------------------------------------*
 *         Name:  fnv_32_hash
 *  Description:  This function will return a hash of the string passed in.
 *                using FNV hash.
 *-------------------------------------------------------------------------------------*/
unsigned int	fnv_32_hash ( unsigned char *string, unsigned int length )
{
	unsigned int	h = 0x811c9dc5;
	unsigned int	i;

	for ( i = 0; i < length; i++ )
	{
		h = ( h * 0x01000193 ) ^ string[i];
	}

	return h;
}

/*---  FUNCTION  ----------------------------------------------------------------------*
 *         Name:  SizeToString
 *  Description:  This function will return int as a string max of max-length  bytes.
 *  			  it will return an int of the size of the string that has been made.
 *-------------------------------------------------------------------------------------*/
unsigned int	SizeToString(unsigned char* string,unsigned int size, unsigned int max_length)
{
	unsigned int digits = 0;

	if (size == 0)
	{
		string[max_length-1] = '0';
		digits = 1;
	}
	else
	{
		while(size != 0 && digits < max_length)
		{
			string[(max_length-digits)-1] = '0' + (size % 10);
			digits++;
			size = size / 10;
		}
	}

	return digits;
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : hex_dump
 * Desc : This function will dump the given buffer to the standard output.
 *--------------------------------------------------------------------------------*/
void	hex_dump(unsigned char* memory, unsigned int memory_size)
{
	int		count,count2,count3,count4;
	char	line[89];
	char	HexBit[] = "0123456789ABCDEF";

	/* write the straplines */
	printf("                     Hex Dump                                          Characters      \n");
	printf("                     ------------------------------------------------  ----------------\n");

	/* clear the space under the timestamp */
	memset(line,' ',86);
	
	/* now dump the rest */
	for (count=0;count < memory_size;count+=16)
	{
		SizeToString((unsigned char*)line,count,9);

		for(count2=count,count3=21,count4=71;count2 < count+16 && count2 < memory_size;count2++,count3+=3,count4++)
		{
			/* do the main hex conversion */
			line[count3] = HexBit[((memory[count2]&0xf0)>>4)];
			line[count3+1] = HexBit[(memory[count2]&0x0f)];
			line[count3+2] = ' ';

			/* do the end stuff */
			if (!isprint(memory[count2]))
				line[count4] = '.';
			else
				line[count4] = (char) memory[count2];

		}

		if (count2 < count+16)
		{
			for(;count2<count+16;count2++,count3+=3,count4++)
			{
				line[count3] = ' ';
				line[count3+1] = ' ';
				line[count3+2] = ' ';

				line[count4] = ' ';
			}
		}

		line[87] = '\n';
		line[88] = '\0';
		
		/*just to stop GCC whining about things it should not whine about */
		printf("%s",line);
	}
}

/*----- FUNCTION -----------------------------------------------------------------*
 * Name : strcpycnt
 * Desc : String copy and count.
 *        This function will return the number of bytes copied upto a max number
 *        of bytes.
 *--------------------------------------------------------------------------------*/
unsigned int	strcpycnt( unsigned char* dest, unsigned char* src, unsigned int max)
{
	unsigned int result = 0;

	for (; result < max && src[result] != 0; result++)
	{
		dest[result] = src[result];
	}

	if (result < max)
		dest[result++] = '\0';

	return result;
}

