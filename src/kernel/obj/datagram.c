# include <kernel/kernel.h>
# include <kernel/user.h>
# include <status.h>

inherit LIB_CONN;	/* basic connection object */


object driver;		/* driver object */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize
 */
static void create()
{
    ::create("datagram", MODE_RAW);
    driver = find_object(DRIVER);
}

/*
 * NAME:	connect_datagram()
 * DESCRIPTION:	initiate an outbound datagram connection
 */
void connect_datagram(object user, int dgram, string address, int port)
{
    if (previous_program() == LIB_USER) {
	::connect_datagram(dgram, address, port);
	set_user(user);
	call_out("connecting", 0);
    }
}

/*
 * NAME:	connecting()
 * DESCRIPTION:	pending connection?
 */
static void connecting()
{
    if (!status(this_object(), O_SPECIAL)) {
	::unconnected(([ ]), -1);
    }
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
 * NAME:	unconnected()
 * DESCRIPTION:	an outbound connection could not be established
 */
static void unconnected(int refused)
{
    ::unconnected(([ ]), refused);
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
 * NAME:	timeout()
 * DESCRIPTION:	connection timed out
 */
static void timeout()
{
    ::timeout(([ ]));
}

/*
 * NAME:	receive_datagram()
 * DESCRIPTION:	forward a message to listeners
 */
static void receive_datagram(string str)
{
    if (this_object()) {
	set_mode(::receive_message(([ ]), str));
    }
}

/*
 * NAME:	set_mode()
 * DESCRIPTION:	set the connection mode
 */
void set_mode(int mode)
{
    if (KERNEL()) {
	::set_mode(mode);
    }
}

/*
 * NAME:	message()
 * DESCRIPTION:	send a message to the other side
 */
int message(string str)
{
    return datagram(str);
}
