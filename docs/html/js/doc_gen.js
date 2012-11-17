/*--------------------------------------------------------------------------------*
 *   @file: canvas
 *   @desc: This is a javascript file that handles drawing to a canvas.
 *
 * @author: pantoine
 *   @date: 22/07/2012 10:54:49
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

var sequence_diagram = {name:'test_page',
						timelines:['one','test two','test three'],
						messages:[	{from:0,name:'message one',to:2},
									{from:1,name:'message two',to:2},
									{from:2,name:'message three',to:1},
									{from:1,name:'message four',to:0},
									{from:2,name:'message five',to:0}]};

var sequence_diagrams = [sequence_diagram];

/* 1 pixel border and 5 pixel margin */
var box_size 		= 2 + 6;
var box_offset 		= 1 + 3;
var font_size_px	= 10;
var box_height		= box_size + font_size_px;
var row_spacing		= font_size_px + (font_size_px / 2);

function drawFlatArrow(context,x,y,length,pointers)
{
	context.beginPath();
	context.moveTo(x,y);
	context.lineTo(x+length,y);
	context.stroke();

	if (pointers)
	{
		point_dir = 0 - Math.abs(length)/length;

		context.fillStyle   = '#000'; // black
		
		var start = x+length;

		// draw pointer
		context.beginPath();
		context.moveTo(start,y);
		context.lineTo(start + (10 * point_dir),y - 4);
		context.lineTo(start + (10 * point_dir),y + 4);
		context.fill();
	}
}

function drawTextArrow(context,x,y,length,text)
{

	var point_dir;
	var text_size = context.measureText(text).width;
	var	line_size = (Math.abs(length) - text_size) / 2;
	
	if (length != 0)
	{
 		point_dir = 0 - Math.abs(length)/length;
	}
	else
	{
		point_dir = 1;
	}

	var seg_1_end 	= x - ((line_size - 1) * point_dir);
	var seg_2_start	= x - ((line_size + text_size + 1) * point_dir);
	var seg_2_end	= x - (((line_size * 2) + text_size) * point_dir);

	// draw first line segment
	context.beginPath();
	context.moveTo(x,y);
	try
	{
	context.lineTo(seg_1_end,y);
	}
	catch(e)
	{
		alert("seg_1: " + seg_1_end + " line_size: " + line_size + "point_dir:" + point_dir + "length:" + length);
	}
	context.stroke();

	// add the arrow
	context.beginPath();
	context.moveTo(seg_2_start,y);
	context.lineTo(seg_2_end,y);
	context.stroke();
	
	context.beginPath();
	context.moveTo(seg_2_end,y);
	context.lineTo(seg_2_end + (10 * point_dir),y - 4);
	context.lineTo(seg_2_end + (10 * point_dir),y + 4);
	context.fill();

	if (point_dir < 0)
	{
		context.fillText(text,seg_1_end+1,y-5);
	}
	else
	{
		context.fillText(text,seg_2_start+1,y-5);
	}
}
// This box is x-centered, and y is the top
function drawTextBox(context,x,y,text)
{
	var text_length = context.measureText(text).width;
			
	context.textBaseline = 'top';

	// set the colours
	context.fillStyle   = '#ddd'; // something
	context.strokeStyle = '#000'; // black
	context.lineWidth   = 1;

	// Draw some rectangles.
	x_pos = x - (box_size + text_length) / 2;
	context.fillRect  (x_pos,   y, box_size + text_length, box_size + 10);
	context.strokeRect(x_pos,   y, box_size + text_length, box_size + 10);

	context.fillStyle   = '#000'; // black
	context.fillText(text,x_pos+box_offset,y+box_offset);
}

function calculateColumns(context,sequence_diagram)
{
	var start			= 0;
	var max_message		= 0;
	var column_start	= [];

	/* find the longest name
	 * if the name is longer than the longest message, then it should define
	 * the width between columns.
	 */
	for (column=0; column < sequence_diagram.timelines.length; column++)
	{
		var text_length = context.measureText(sequence_diagram.timelines[column]).width + box_size;

		if (text_length > max_message)
		{
			max_message = text_length;
		}
	}

	/* now try and find the longest column */
	for (message=0; message < sequence_diagram.messages.length; message++)
	{
		var text_length = context.measureText(sequence_diagram.messages[message].name).width + 10 + 4 + 20;
		if (text_length > max_message)
		{
			max_message = text_length;
		}
	}

	return max_message;
}

function drawTimelines(context,column_spacing,y,sequence_diagram)
{
	total_timeline_height = row_spacing * (sequence_diagram.messages.length + 1);
	sequence_offset = box_size + (context.measureText(sequence_diagram.timelines[0]).width / 2);

	var column_offsets = [];

	for (column=0; column < sequence_diagram.timelines.length; column++)
	{
		var x = sequence_offset + (column * column_spacing);
		column_offsets[column] = x;
			
		drawTextBox(context,x,y,sequence_diagram.timelines[column]);
		
		context.beginPath();
		context.moveTo(x,y+box_height);
		context.lineTo(x,y+box_height+total_timeline_height);
		context.stroke();

		drawTextBox(context,x,y+box_height+total_timeline_height,sequence_diagram.timelines[column]);
	}
	return column_offsets;
}

function drawMessages(context,y,column_spacing,column_offsets,sequence_diagram)
{
	var y_axis = y + box_height;

	/* now try and find the longest column */
	for (message=0; message < sequence_diagram.messages.length; message++)
	{
		y_axis += 15;
		var start = column_offsets[sequence_diagram.messages[message].from];
		var end   = column_offsets[sequence_diagram.messages[message].to];

		drawTextArrow(context, start, y_axis, end - start, sequence_diagram.messages[message].name);
	}
}

function drawSequenceDiagram(sequence_diagram)
{
	var canvas  = document.getElementById(sequence_diagram.canvas_id);
	var context;
	var x_size;
	var y_size;

	if (canvas)
	{
		context = canvas.getContext('2d');

		/* if not a canvas then leave it alone, should fall back to the text version */
		if (context)
		{
			column_spacing = calculateColumns(context,sequence_diagram);

			/* calculate the width that the canvas has to be */
			x_size = box_size + (context.measureText(sequence_diagram.timelines[0]).width / 2);
			x_size += (column_spacing * sequence_diagram.timelines);
			x_size += box_size + (context.measureText(sequence_diagram.timelines[sequence_diagram.timelines.length-1]).width / 2);
			x_size += box_size;

			/* calculate the height that it has to be */
			y_size = box_height + ((sequence_diagram.messages.length + 1) * 15) + box_height + box_size;

			canvas.width	= column_spacing * (sequence_diagram.timelines.length + 1);
			canvas.height	= y_size;

			column_offsets = drawTimelines(context,column_spacing,box_size/2,sequence_diagram);
			drawMessages(context,box_size/2,column_spacing,column_offsets,sequence_diagram);
		}
	}
}

function update_sequence_diagrams(sequence_diagrams)
{
	for (count=0; count < sequence_diagrams.length; count++)
	{
		drawSequenceDiagram(sequence_diagrams[count]);
	}
}

function update_state_machines(state_machines)
{
	for (count=0; count < state_machines.length; count++)
	{
	}
}

