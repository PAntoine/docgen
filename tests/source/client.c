/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : client
 * Desc  : This file will initiate the playback of the media on the target device.
 *
 * Author: pantoine
 * Date  : 12/01/2012 18:29:38
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2012 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

void main()
{
	/**
	 * @send: REQUEST_DIRECTORY_LISTING
	 * @to: media_source
	 * @timeline: client
	 */

	/**
	 * @waitfor: DIRECTORY_LISTING
	 * @timeline: client
	 */

	/**
	 * @send: PLAY_REQUEST
	 * @to: media_sink
	 * @timeline: client
	 */
}

