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

/* 1 pixel border and 5 pixel margin */
var box_size				= 2 + 6;
var box_offset				= 1 + 3;
var font_size_px			= 10;
var box_height				= box_size + font_size_px;
var row_spacing				= font_size_px + (font_size_px / 2);
var arrow_tip_length		= 10
var arrow_tip_half_width	= 3

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

// The draws an arrow the lies directly on the x/y axis.
function drawStraightArrow(context,x,y,x_length,y_length)
{
	var x_offset = 0;
	var y_offset = 0;
	var y_line_offset = 0;
	var x_line_offset = 0;

	if (x_length == 0)
	{
		x_offset = 4;
		point_dir = Math.abs(y_length)/y_length;
		y_line_offset = 4 * point_dir;
	}
	else
	{
		y_offset = 4;
		point_dir = Math.abs(x_length)/x_length;
		x_line_offset = 4 * point_dir;
	}

	context.beginPath();
	context.moveTo(x,y);
	context.lineTo(x+x_length,y+y_length);
	context.stroke();

	context.fillStyle   = '#000'; // black

	// draw pointer
	context.beginPath();
	context.moveTo(x,y);
	context.lineTo(x + x_offset + x_line_offset, y + y_offset + y_line_offset);
	context.lineTo(x - x_offset + x_line_offset, y - y_offset + y_line_offset);
	context.fill();
}

function drawArrowHead(context,x1,y1,angle,x_dir,y_dir)
{
	context.fillStyle   = '#000'; // black
	
	// now work out the arrow head
	var y2 = y1 - ((Math.sin(angle) * 4) * y_dir) - ((Math.cos(angle) * 3) * y_dir);
	var y3 = y1 - ((Math.sin(angle) * 4) * y_dir) + ((Math.cos(angle) * 3) * y_dir);
	
	var x2 = x1 + ((Math.cos(angle) * 4) * x_dir) - ((Math.sin(angle) * 3) * x_dir);
	var x3 = x1 + ((Math.cos(angle) * 4) * x_dir) + ((Math.sin(angle) * 3) * x_dir);

	context.beginPath();
	context.moveTo(x1,y1);
	context.lineTo(x2,y2);
	context.lineTo(x3,y3);
	context.fill();
}

// The draws an arrow the lies directly on the x/y axis.
function drawAngledArrow(context,from_x,from_y,to_x,to_y,radius,text)
{
	context.strokeStyle = '#000'; // black
	context.fillStyle   = '#000'; // black

	// Normallise the arrow in x
	if (from_x > to_x)
	{
		var x1 = to_x;
		var y1 = to_y;
		var x2 = from_x;
		var y2 = from_y;
		var arrow_dir = 1;
	}
	else
	{
		var x1 = from_x;
		var y1 = from_y;
		var x2 = to_x;
		var y2 = to_y;
		var arrow_dir = -1;
	}

	// calculate the needed offsets
	var x_length = x2 - x1;
	var y_length = y2 - y1;
	var x_offset = Math.cos(Math.atan2(y_length,x_length)) * radius;
	var y_offset = Math.sin(Math.atan2(y_length,x_length)) * radius;
	var y_dir = Math.abs(to_y - from_y) / (to_y - from_y);

	x1 += x_offset;
	y1 += y_offset;
	x2 -= x_offset;
	y2 -= y_offset;

	// Draw the line
	context.beginPath();
	context.moveTo(x1,y1);
	context.lineTo(x2,y2);
	context.stroke();

	var angle = Math.abs(Math.atan2(y_length,x_length));

	drawArrowHead(context,to_x + (x_offset * arrow_dir),to_y + (y_offset * arrow_dir),angle,arrow_dir,y_dir);
}

function drawCurvedArrow(context,from_x,from_y,to_x,to_y,radius,text)
{
	context.strokeStyle = '#000'; // black
	context.fillStyle   = '#000'; // black

	// Normallise the arrow in x
	if (from_x > to_x)
	{
		var x1 = to_x;
		var y1 = to_y;
		var x2 = from_x;
		var y2 = from_y;
	}
	else
	{
		var x1 = from_x;
		var y1 = from_y;
		var x2 = to_x;
		var y2 = to_y;
	}

	// calculate the needed offsets
	var x_length = x2 - x1;
	var y_length = y2 - y1;
	var x_offset = (Math.cos(Math.atan2(y_length,x_length))) * radius;
	var y_offset = (Math.sin(Math.atan2(y_length,x_length))) * radius;

	x1 += x_offset;
	y1 += y_offset;
	x2 -= x_offset;
	y2 -= y_offset;

	/* ok, now calculate the two control points. */
	var angle = Math.abs(Math.atan2(y_length,x_length));
	var x3 = x1 - Math.sin(angle) * (radius * 2);
	var x4 = x2 - Math.sin(angle) * (radius * 2);
	var y3 = y1 + Math.cos(angle) * (radius * 2);
	var y4 = y2 + Math.cos(angle) * (radius * 2);

	/* draw the curve */
	context.beginPath();
	context.moveTo(x1,y1);
	context.bezierCurveTo(x3,y3,x4,y4,x2,y2);
	context.stroke();

	/* draw arrow head */
	var y_dir = Math.abs(to_y - from_y) / (to_y - from_y);
	drawArrowHead(context,to_x - (x_offset * y_dir),to_y - (y_offset * y_dir),angle,-1,-1);
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

// The circle is x/y centred
function drawTextCircle(context,x,y,radius,text)
{
	context.textBaseline = 'middle';

	// set the colours
	context.fillStyle   = '#ddd'; // something
	context.strokeStyle = '#000'; // black
	context.lineWidth   = 1;

	// Ok draw and fill the circle
	context.beginPath();
	context.arc(x,y,radius, 0, 2 * Math.PI, false);	

	context.fill();
	context.stroke();
	
	// Ok, write the text in the middle of the box
	context.fillStyle   = '#000'; // black
	context.fillText(text,x-(context.measureText(text).width/2),y);

}

// draw an arrow with text on it from to leaving the radius.
function drawCircleTextArrow(context,from_x,from_y,to_x,to_y,radius,clash,text)
{
	context.textBaseline = 'middle';

	// set the colours
	context.fillStyle   = '#ddd'; // something
	context.strokeStyle = '#000'; // black
	context.lineWidth   = 1;

	// calculate the difference in the circle.
	var x_diff = from_x - to_x;
	var y_diff = from_y - to_y;

	var x_offset = Math.cos(Math.atan2(y_diff,x_diff)) * radius;
	var y_offset = Math.sin(Math.atan2(y_diff,x_diff)) * radius;

	if (from_x == to_x && from_y == to_y)
	{
		/* arrow to self */
		// calculate the offset from the x and y points to the edge of the
		// circle.
		var point_x = from_x + x_offset + Math.sin(Math.PI/4) * (radius * 2);
		var point_y1 = from_y + y_offset - Math.cos(Math.PI/4) * (radius * 2);
		var point_y2 = from_y + y_offset + Math.cos(Math.PI/4) * (radius * 2);

		context.beginPath();
		context.moveTo(from_x + x_offset + 4,from_y + y_offset);
		context.bezierCurveTo(point_x,point_y1,point_x,point_y2,from_x + x_offset + 4,from_y + y_offset);
		context.stroke();

		drawArrowHead(context,from_x + x_offset,from_y + y_offset,0,1,-1);
	}
	else if (x_diff == 0 || y_diff == 0)
	{
		drawStraightArrow(context,to_x+x_offset,to_y+y_offset,x_diff-x_offset*2,y_diff-y_offset*2);
	}
	else if (!clash)
	{
		drawAngledArrow(context,from_x,from_y,to_x,to_y,radius,text);
	}
	else
	{
		drawCurvedArrow(context,from_x,from_y,to_x,to_y,radius,text);
	}
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

function calculateNodeSize(context,state_machine)
{
	var max_width = 0;
	var test_string;

	for (index=0; index < state_machine.nodes.length; index++)
	{
		var text_length = context.measureText(state_machine.nodes[index].name).width + box_size;

		if (text_length > max_width)
		{
			max_width = text_length;
		}
	}

	return max_width;
}

function drawStateMachine(state_machine)
{
	var canvas  = document.getElementById(state_machine.canvas_id);
	var context;
	var x_size;
	var y_size;

	if (canvas)
	{
		context = canvas.getContext('2d');

		/* if not a canvas then leave it alone, should fall back to the text version */
		if (context)
		{
			var node_size  = calculateNodeSize(context,state_machine);
			var node_space = node_size + 10;
			var radius     = node_size / 2;

			canvas.width	= state_machine.x_size * node_space;
			canvas.height	= state_machine.y_size * node_space;
	
			for (var index=0; index < state_machine.nodes.length; index++)
			{
				drawTextCircle(	context,
								node_space * state_machine.nodes[index].x,
								node_space * state_machine.nodes[index].y,
								radius,
								state_machine.nodes[index].name);
			}

			for (var index=0; index < state_machine.vertices.length; index++)
			{
				var clash = false;

				// check to see if the line intersects
				for (var count=0; count < state_machine.nodes.length; count++)
				{
					if (count != state_machine.vertices[index].to_node && count != state_machine.vertices[index].from_node)
					{
						var x_length = state_machine.nodes[state_machine.vertices[index].from_node].x - state_machine.nodes[state_machine.vertices[index].to_node].x;
						var y_length = state_machine.nodes[state_machine.vertices[index].from_node].y - state_machine.nodes[state_machine.vertices[index].to_node].y;
						var x_point_length = state_machine.nodes[state_machine.vertices[index].from_node].x - state_machine.nodes[count].x;
						var y_point_length = state_machine.nodes[state_machine.vertices[index].from_node].y - state_machine.nodes[count].y;

						if (Math.abs(x_length) > Math.abs(x_point_length) && (x_length/y_length) == (x_point_length/y_point_length))
						{
							clash = true;
							break;
						}
					}
				}

				// draw the lines 
				drawCircleTextArrow(context,
									node_space * state_machine.nodes[state_machine.vertices[index].from_node].x,
									node_space * state_machine.nodes[state_machine.vertices[index].from_node].y,
									node_space * state_machine.nodes[state_machine.vertices[index].to_node].x,
									node_space * state_machine.nodes[state_machine.vertices[index].to_node].y,
									radius,
									clash,
									"test_text");
			}
		}
	}
}

function update_sequence_diagrams(sequence_diagrams)
{
	for (var count=0; count < sequence_diagrams.length; count++)
	{
		drawSequenceDiagram(sequence_diagrams[count]);
	}
}

function update_state_machines(state_machines)
{
	for (var count=0; count < state_machines.length; count++)
	{
		drawStateMachine(state_machines[count]);
	}
}

function ToggleHideDiv(div_id)
{
	var element = document.getElementById(div_id);

	if (element.style.display == "none")
	{
		element.style.display = "block";
	}
	else
	{
		element.style.display = "none";
	}
}

function close_all_divs()
{
	var all_divs = document.getElementsByClassName('hh');

	for (var count=0; count < all_divs.length; count++)
	{
		all_divs[count].style.display = "none";
	}
}

