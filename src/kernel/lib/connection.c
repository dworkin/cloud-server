# include <kernel/kernel.h>
# include <kernel/user.h>


private object userd;		/* user daemon */
private int port;		/* port # */
private object user;		/* user object */
private string conntype;	/* connection type */
private int mode;		/* connection mode */
private int blocked;		/* connection blocked? */
private string buffer;		/* buffered output string */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize
 */
static void create(string type, int mode)
{
    userd = find_object(USERD);
    port = -1;
    conntype = type;
    ::mode = mode;
}


/*
 * NAME:	set_mode()
 * DESCRIPTION:	set the current connection mode
 */
static void set_mode(int newmode)
{
    if (newmode != mode && newmode != MODE_NOCHANGE) {
	if (newmode == MODE_DISCONNECT) {
	    destruct_object(this_object());
	} else {
	    rlimits (-1; -1) {
		if (newmode >= MODE_UNBLOCK) {
		    if (newmode - MODE_UNBLOCK != blocked) {
			block_input(blocked = newmode - MODE_UNBLOCK);
		    }
		} else {
		    if (blocked) {
			block_input(blocked = FALSE);
		    }
		    mode = newmode;
		}
	    }
	}
    }
}

/*
 * NAME:	query_mode()
 * DESCRIPTION:	return the current connection mode
 */
int query_mode()
{
    return (blocked) ? MODE_BLOCK : mode;
}


/*
 * NAME:	open()
 * DESCRIPTION:	open the connection
 */
static void open(mapping tls)
{
    int mode, timeout;
    string banner;

    mode = call_other(userd, "query_" + conntype + "_mode", port,
		      this_object());
    if (mode != MODE_NOCHANGE) {
	set_mode(mode);
	if (mode == MODE_DISCONNECT) {
	    return;	/* disconnect immediately */
	}
    }

    if (conntype != "datagram") {
	banner = call_other(userd, "query_" + conntype + "_banner", port,
			    this_object());
	if (banner) {
	    send_message(banner);
	}
    }

    timeout = call_other(userd, "query_" + conntype + "_timeout", port,
			 this_object());
    if (!user) {
	if (timeout > 0) {
	    call_out("timeout", timeout);
	} else if (timeout < 0) {
	    /* disconnect immediately */
	    destruct_object(this_object());
	}
    }
}

/*
 * NAME:	unconnected()
 * DESCRIPTION:	an outbound connection could not be established
 */
static void unconnected(mapping tls, int errcode)
{
    if (user) {
	user->connect_failed(errcode);
    }
    destruct_object(this_object());
}

/*
 * NAME:	close()
 * DESCRIPTION:	close the connection
 */
static void close(mapping tls, int dest)
{
    rlimits (-1; -1) {
	if (user) {
	    catch {
		user->logout(dest);
	    }
	}
	if (!dest) {
	    destruct_object(this_object());
	}
    }
}

/*
 * NAME:	disconnect()
 * DESCRIPTION:	break connection
 */
void disconnect()
{
    if (previous_program() == LIB_USER) {
	destruct_object(this_object());
    }
}

/*
 * NAME:	reboot()
 * DESCRIPTION:	destruct connection object after a reboot
 */
void reboot()
{
    if (previous_object() == userd || SYSTEM()) {
	if (user) {
	    catch {
		user->logout(FALSE);
	    }
	}
	destruct_object(this_object());
    }
}

/*
 * NAME:	set_port()
 * DESCRIPTION:	set the port number
 */
void set_port(int num)
{
    if (previous_object() == userd) {
	port = num;
    }
}

/*
 * NAME:	query_port()
 * DESCRIPTION:	return the port number
 */
int query_port()
{
    return port;
}

/*
 * NAME:	set_user()
 * DESCRIPTION:	set or change the user object directly
 */
void set_user(object obj, string str)
{
    if (KERNEL()) {
	user = obj;
	set_mode(obj->login(str));
    }
}

/*
 * NAME:	query_user()
 * DESCRIPTION:	return the associated user object
 */
nomask object query_user()
{
    return user;
}

/*
 * NAME:	timeout()
 * DESCRIPTION:	if the connection timed out, disconnect
 */
static void timeout(mapping tls)
{
    if (!user || user->query_conn() != this_object()) {
	destruct_object(this_object());
    }
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	forward a message to user object
 */
static int receive_message(mapping tls, string str)
{
    int mode;

    if (!user) {
	user = call_other(userd, conntype + "_user", port, str);
	set_mode(mode = user->login(str));
    } else {
	set_mode(mode = user->receive_message(str));
    }
    return mode;
}

/*
 * NAME:	message()
 * DESCRIPTION:	send a message across the connection
 */
int message(string str)
{
    if (previous_object() == user) {
	rlimits (-1; -1) {
	    int len;

	    len = send_message(str);
	    if (len != strlen(str)) {
		/*
		 * string couldn't be sent completely; buffer the remainder
		 */
		buffer = str[len ..];
		return FALSE;
	    } else {
		if (buffer) {
		    buffer = nil;
		}
		return TRUE;
	    }
	}
    }
}

/*
 * NAME:	message_done()
 * DESCRIPTION:	called when output is completed
 */
static void message_done(mapping tls)
{
    if (buffer) {
	send_message(buffer);
	buffer = nil;
    } else if (user) {
	set_mode(user->message_done());
    }
}

/*
 * NAME:	datagram_challenge()
 * DESCRIPTION:	set the challenge for the datagram channel
 */
void datagram_challenge(string str)
{
    if (previous_object() == user) {
	::datagram_challenge(str);
    }
}

/*
 * NAME:	datagram_attach()
 * DESCRIPTION:	attach a datagram channel to this connection
 */
static void datagram_attach(mapping tls)
{
    if (user) {
	user->datagram_attach();
    }
}

/*
 * NAME:	receive_datagram()
 * DESCRIPTION:	forward a datagram to the user
 */
static void receive_datagram(mapping tls, string str)
{
    if (user) {
	user->receive_datagram(str);
    }
}

/*
 * NAME:	datagram()
 * DESCRIPTION:	send a datagram across the connection
 */
int datagram(string str)
{
    if (previous_object() == user) {
	return (send_datagram(str) == strlen(str));
    }
}
