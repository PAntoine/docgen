/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : media_sink
 * Desc  : This file mimics the media sink.
 *
 * Author: pantoine
 * Date  : 28/12/2011 13:07:21
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

/**
 * @function: play_media
 */
void	play_media()
{
	/**
	 * @send: REQUEST_TRACK_DATA
	 * @to: media_source
	 */

	/**
	 * @waitfor: SEND_TRACK_DATA
	 */

	/* handle playing the track data */

	/* @responds: ACK
	 *
	 * we have finished the playing of the data, acknowledge to the requester that it has
	 * been done.
	 */
}

void main_loop()
{
	/**
	 * @waitfor: SERVER_ANNOUNCE
	 * @call: initialise_function
	 * @parameter: MEDIA_SINK
	 * @timeline: media_sink
	 */

	while((message = wait_for_message()) != CLOSE_SYSTEM)
	{
		case PLAY_REQUEST:
			/**
			 * @waitfor: PLAY_REQUEST
			 * @call: play_media
			 * @timeline: media_sink
			 */
			play_media();
			break;

		default:
			break;
	}
}


