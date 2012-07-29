/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 *   @file: html
 *   @desc: The definitions for the HTML plugin.
 *
 * @author: pantoine
 *   @date: 19/07/2012 07:35:05
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

#ifndef __HTML_H__
#define __HTML_H__

#include "document_generator.h"

#define	HTML_VERSION	((unsigned char) 0x01)

/*--------------------------------------------------------------------------------*
 * structures required for drawing the HTML sequences.
 *--------------------------------------------------------------------------------*/
typedef struct tag_html_message
{
	unsigned int	from;
	unsigned int	to;
	MESSAGE*		message;

	struct tag_html_message*	next;
} HTML_MESSAGE;

typedef struct tag_html_timeline
{
	TIMELINE*	timeline;

	struct tag_html_timeline*	next;

} HTML_TIMELINE;

typedef struct tag_html_sequence_diagram
{
	NAME			name;
	HTML_MESSAGE	message;
	HTML_MESSAGE*	last_message;
	HTML_TIMELINE	timeline;
	HTML_TIMELINE*	last_timeline;
	unsigned int	timelines;
	unsigned int	sequence_no;			/* the root diagram uses this as a count of all sequence diagrams */

	struct tag_html_sequence_diagram*	next;
} HTML_SEQUENCE_DIAGRAM; 

#endif

