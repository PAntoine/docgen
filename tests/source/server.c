/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : server
 * Desc  : This is a test file for the sequence diagram. 
 *         It is used to define a server that will handle messages from teo
 *         different devices.
 *
 * Author: pantoine
 * Date  : 08/12/2011 17:42:34
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

main()
{
	/**
	 * @send: SERVER_ANNOUNCE
	 * @to: broadcast
	 * @timeline: server
	 */
	intialise();

	while(message_id = handle_message())
	{
		switch(message_id)
		{
			/**
			 * @waitfor: DEVICE_ANNOUNCE
			 * @timeline: server
			 * @call: send_server_response
			 */
			case DEVICE_ANNOUNCE:
				send_server_response();
				break;			

			/**
			 * @waitfor: DEVICE_REGISTER
			 * @timeline: server
			 */
			case DEVICE_REGISTER:
				handle_device_register();
				break;
	
			/**
			 * @send: DEVICE_CHANGED
			 * @to: broadcast
			 * @timeline: server
			 */
			case DEVICE_REGISTER:
				handle_device_register();
				break;

			default:
		}

		/** 
		 * @send: MESSAGE_TO_SELF
		 * @to: server
		 * @timeline: server
		 */

		/**
		 * @waitfor: MESSAGE_TO_SELF
		 * @timeline: server
		 */
	}
}

/**
 * @function: send_server_response
 *
 * This function responds to the devices annouce massages and handles the
 * the response.
 */
void	send_server_response(void)
{
	/**
	 * @responds: SERVER_RESPONSE
	 */
}

void ignore_this_function()
{
	while()
	{
		case ffff:
			break;

		case ggg:
			break;
	}

	if ()
	{
		
	}
	else
	{
	}
}


