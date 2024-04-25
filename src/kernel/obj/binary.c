# include <kernel/kernel.h>
# include <kernel/user.h>
# include <status.h>

inherit LIB_CONN;	/* basic connection object */


string buffer;		/* buffered input */
int length;		/* length of message to receive */
int noline;		/* no full line in input buffer */
int restart;		/* restart_input handle */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize
 */
static void create()
{
    ::create("binary", MODE_RAW);
    buffer = "";
}

/*
 * NAME:	connect()
 * DESCRIPTION:	initiate an outbound connection
 */
void connect(object user, string address, int port)
{
    if (previous_program() == LIB_USER) {
	::connect(address, port);
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
 * NAME:	set_message_length()
 * DESCRIPTION:	set the size of the receive buffer
 */
void set_message_length(int len)
{
    if (previous_program() == LIB_USER) {
	length = len;
    }
}

/*
 * NAME:	add_to_buffer()
 * DESCRIPTION:	do this where an error is allowed to happen
 */
private void add_to_buffer(mapping tls, string str)
{
    try {
	buffer += str;
    } catch (...) {
	error("Binary connection buffer overflow");
    }
    noline = FALSE;
}

/*
 * NAME:	receive_buffer()
 * DESCRIPTION:	process input buffer
 */
private void receive_buffer(mapping tls)
{
    int mode, len;
    string str, head, pre;

    while ((mode=query_mode()) != MODE_BLOCK && mode != MODE_DISCONNECT) {
	if (mode != MODE_RAW) {
	    if (sscanf(buffer, "%s\r\n%s", str, buffer) != 0 ||
		sscanf(buffer, "%s\n%s", str, buffer) != 0) {
		if (mode == MODE_EDIT) {
		    while (sscanf(str, "%s\b%s", head, str) != 0) {
			while (sscanf(head, "%s\x7f%s", pre, head) != 0) {
			    len = strlen(pre);
			    if (len != 0) {
				head = pre[0 .. len - 2] + head;
			    }
			}
			len = strlen(head);
			if (len != 0) {
			    str = head[0 .. len - 2] + str;
			}
		    }
		    while (sscanf(str, "%s\x7f%s", head, str) != 0) {
			len = strlen(head);
			if (len != 0) {
			    str = head[0 .. len - 2] + str;
			}
		    }
		}

		set_mode(::receive_message(tls, str));
	    } else {
		noline = TRUE;
		break;
	    }
	} else {
	    if (strlen(buffer) != 0) {
		if (length > 0) {
		    if (length < strlen(buffer)) {
			str = buffer[.. length - 1];
			buffer = buffer[length ..];
			length = 0;
		    } else {
			length -= strlen(buffer);
			str = buffer;
			buffer = "";
		    }
		} else {
		    str = buffer;
		    buffer = "";
		}
		set_mode(::receive_message(tls, str));
	    }
	    break;
	}
    }
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	forward a message to listeners
 */
static void receive_message(string str)
{
    mapping tls;

    if (restart != 0) {
	remove_call_out(restart);
	restart = 0;
    }
    add_to_buffer(tls = ([ ]), str);
    receive_buffer(tls);
}

/*
 * NAME:	set_mode()
 * DESCRIPTION:	set the connection mode
 */
void set_mode(int mode)
{
    int oldmode;
    object user;

    if (KERNEL()) {
	oldmode = query_mode();
	::set_mode(mode);
	if (oldmode == MODE_BLOCK && mode != MODE_DISCONNECT &&
	    mode != MODE_BLOCK && restart == 0) {
	    if (strlen(buffer) != 0) {
		switch (mode) {
		case MODE_LINE:
		case MODE_EDIT:
		    if (!noline) {
			break;
		    }
		    /* fall through */
		default:
		    if (!(user=query_user()) || !user->buffered_input()) {
			return;
		    }
		    /* fall through */
		case MODE_RAW:
		    break;
		}
	    } else if (!(user=query_user()) || !user->buffered_input()) {
		return;
	    }

	    restart = call_out("restart_input", 0);
	}
    }
}

/*
 * NAME:	user_restart_input()
 * DESCRIPTION:	reprocess user input buffer
 */
private void user_restart_input(mapping tls, object user)
{
    user->restart_input();
}

/*
 * NAME:	restart_input()
 * DESCRIPTION:	reprocess the input buffer
 */
static void restart_input()
{
    int mode;
    mapping tls;
    object user;

    restart = 0;
    if ((mode=query_mode()) != MODE_BLOCK && mode != MODE_DISCONNECT) {
	tls = ([ ]);
	TLSVAR(tls, TLS_USER) = this_object();
	user = query_user();
	if (user) {
	    user_restart_input(tls, user);
	}
	receive_buffer(tls);
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

/*
 * NAME:	datagram_attach()
 * DESCRIPTION:	attach a datagram channel to this connection
 */
static void datagram_attach()
{
    ::datagram_attach(([ ]));
}

/*
 * NAME:	receive_datagram()
 * DESCRIPTION:	receive a datagram
 */
static void receive_datagram(string str)
{
    ::receive_datagram(([ ]), str);
}
