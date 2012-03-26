/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : media_source
 * Desc  : Test file for documenting a media source.
 *
 * Author: pantoine
 * Date  : 28/12/2011 12:54:22
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

/**
 * @function: request_directory_listing
 *
 * This function would handle the requesting of the directory listing.
 */
void	request_directory_listing()
{
	/**
	 * @responds: DIRECTORY_LISTING
	 */
	HNCP_SendDirectoryListing();
}

/**
 * @function: request_track_data
 */
void	request_track_data()
{
	/**
	 * @responds: SEND_TRACK_DATA
	 */
}

void main_loop()
{
	/**
	 * @waitfor: SERVER_ANNOUNCE
	 * @call: initialise_function
	 * @parameter: MEDIA_SOURCE	this is a funny thing
	 * @timeline: media_source
	 */

	while((message = wait_for_message()) != CLOSE_SYSTEM)
	{
		case REQUEST_DIRECTORY_LISTING:
			/**
			 * @waitfor: REQUEST_DIRECTORY_LISTING
			 * @call: request_directory_listing
			 * @timeline: media_source
			 */
			send_directory_listing();
			break;

		case REQUEST_TRACK_DATA:
			/**
			 * @waitfor: REQUEST_TRACK_DATA
			 * @call: request_track_data
			 * @timeline: media_source
			 */
			send_track_data();
			break;

		default:
			break;
	}
}


