# include <kernel/kernel.h>
# include <kernel/user.h>

inherit LIB_CONN;	/* basic connection object */


object driver;		/* driver object */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize
 */
static void create()
{
    ::create("telnet", MODE_ECHO);
    driver = find_object(DRIVER);
}

/*
 * NAME:	open()
 * DESCRIPTION:	open the connection
 */
static void open()
{
    ::open(([ ]));
}

/*
 * NAME:	close()
 * DESCRIPTION:	close the connection
 */
static void close(int dest)
{
    ::close(([ ]), dest);
}

/*
 * NAME:	_flow_mode()
 * DESCRIPTION:	set the flow mode
 */
static void _flow_mode(int mode)
{
    ::flow_mode(([ ]), mode);
}

/*
 * NAME:	flow_mode()
 * DESCRIPTION:	flow: set the mode
 */
void flow_mode(int mode, int length)
{
    if (previous_program() == LIB_USER) {
	call_out("_flow_mode", 0, mode);
    }
}

/*
 * NAME:	_disconnect()
 * DESCRIPTION:	break the connection
 */
static void _disconnect()
{
    ::_disconnect(([ ]));
}

/*
 * NAME:	timeout()
 * DESCRIPTION:	connection timed out
 */
static void timeout()
{
    ::timeout(([ ]));
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	forward a message to listeners
 */
static void receive_message(string str)
{
    set_mode(::receive_message(([ ]), str));
}

/*
 * NAME:	set_mode()
 * DESCRIPTION:	set the connection mode
 */
void set_mode(int newmode)
{
    int mode;

    if (KERNEL()) {
	mode = query_mode();
	::set_mode(newmode);
	if (newmode != mode && (newmode == MODE_NOECHO || newmode == MODE_ECHO))
	{
	    send_message(newmode - MODE_NOECHO);
	}
    }
}

/*
 * NAME:	message_done()
 * DESCRIPTION:	called when output is completed
 */
static void message_done()
{
    set_mode(::message_done(([ ])));
}
