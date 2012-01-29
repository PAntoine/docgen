/*--------------------------------------------------------------------------------*
 *               Document Generator Project
 *			        by Peter Antoine
 *
 * Name  : test_statemachine
 * Desc  : This file is a test file for the State Machine.
 *
 * Author: pantoine
 * Date  : 10/11/2011 21:08:10
 *--------------------------------------------------------------------------------*
 *                     Copyright (c) 2011 Peter Antoine
 *                            All rights Reserved.
 *                    Released Under the Artistic Licence
 *--------------------------------------------------------------------------------*/

/**
 * @state: start
 * @next: state_1
 */

/**
 * @state: start
 * @next: state_4
 */

/**
 * @state: state_1
 * @next: state_2
 * @triggers: trigger5
 */

/**
 * @state: state_1
 * @next: state_4
 * @trigger: trigger5;
 * @triggers: $STATE_ONE.trigger2
 * @triggers: $STATE_ONE.trigger4
 */

/**
 * @state: state_1
 * @next: state_3
 */

/**
 * @state: state_2
 * @next: state_3
 */

/**
 * @state:  state_3
 * @next: 	finish
 */

/**
 * @state:	state_3
 * @next:   state_4
 */

/**
 * @state:	state_4
 * @next:	state_2
 */


int main(int argc, char* argv[])
{
	/* this is just a comment to test the false positives: call, send, condition */

	return 0;
}

int other_state_machine(void)
{
	/**
	 * @state: other.start
	 * @next: state_1
	 */

	/**
	 * @state: other.start
	 * @next: state_4
	 */
	
	/**
	 * @state: other.state_1
	 * @next: state_2
	 * @condition: if the world is spinning to the right.
	 */

	/**
	 * @state: other.state_1
	 * @next: state_4
	 * @condition: if the world is spinning to the left.
	 */

	/**
	 * @state: other.state_2
	 * @next: state_3
	 */

	/**
	 * @state:  other.state_3
	 * @triggers: trigger1
	 * @next: 	finish
	 */

	/**
	 * @state:	other.state_3
	 * @trigger: $STATE_ONE.trigger3
	 * @next:   state_4
	 */

	/**
	 * @state:	other.state_4
	 * @next:	state_2
	 */
}

/**---- FUNCTION -----------------------------------------------------------------*
 * Name : state ome
 * Desc : This is a state for a state machine state one.
 *
 * @state: $STATE_ONE.start
 * @next: state_2
 *--------------------------------------------------------------------------------*/
void state_start ( unsigned int hello, unsigned char* param_2 )
{
}

/**---- FUNCTION -----------------------------------------------------------------*
 * Name : state ome
 * Desc : This is a state for a state machine state one.
 *
 * @state: $STATE_ONE.start
 * @condition: if the world is spinning to the right.
 * @next: state_3
 *--------------------------------------------------------------------------------*/
void state_one ( unsigned int hello, unsigned char* param_2 )
{
}

/**---- FUNCTION -----------------------------------------------------------------*
 * Name : state two
 * Desc : This is a state for a state machine state one.
 *
 * @state: $STATE_ONE.state_2
 * @trigger: other.trigger1
 * @next: state_3
 *--------------------------------------------------------------------------------*/
void state_two ( unsigned int hello, unsigned char* param_2 )
{
}

/**---- FUNCTION -----------------------------------------------------------------*
 * Name : state three
 * Desc : This is a state for a state machine state one.
 *
 * @state: $STATE_ONE.state_2
 * @next: state_4
 *--------------------------------------------------------------------------------*/
void state_two_part_2 ( unsigned int hello, unsigned char* param_2 )
{
}

/**---- FUNCTION -----------------------------------------------------------------*
 * Name : state three
 * Desc : This is a state for a state machine state one.
 *
 * @state: $STATE_ONE.state_3
 * @trigger: trigger2
 * @next: state_4
 *--------------------------------------------------------------------------------*/
void state_three ( unsigned int hello, unsigned char* param_2 )
{
}

/**---- FUNCTION -----------------------------------------------------------------*
 * Name : state four
 * Desc : This is a state for a state machine state one.
 *
 * @state: $STATE_ONE.state_4
 * @triggers: trigger3
 * @trigger: trigger4
 * @next: state_4
 *--------------------------------------------------------------------------------*/
void state_four ( unsigned int hello, unsigned char* param_2 )
{
}

/**---- FUNCTION -----------------------------------------------------------------*
 * Name : state four
 * Desc : This is a state for a state machine state one.
 *
 * @state: $STATE_ONE.state_4
 * @next: finish
 *--------------------------------------------------------------------------------*/
void state_three ( unsigned int hello, unsigned char* param_2 )
{
}

/**---- FUNCTION -----------------------------------------------------------------*
 * Name : finish
 * Desc : This is a state for a state machine state one.
 *
 * @state: $STATE_ONE.finish
 *--------------------------------------------------------------------------------*/
void finish ( unsigned int hello, unsigned char* param_2 )
{
}


