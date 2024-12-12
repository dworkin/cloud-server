# include <kernel/kernel.h>
# include <kernel/user.h>


private object userd;		/* user daemon */
private int port;		/* port # */
private object user;		/* user object */
private string conntype;	/* connection type */
private int mode;		/* connection mode */
private int blocked;		/* connection blocked? */
private string buffer;		/* buffered output string */
private int timeout;		/* callout handle */
private int flow;		/* flow state */

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
static void set_mode(int mode)
{
    if (mode != MODE_NOCHANGE && ::mode != MODE_DISCONNECT &&
	(mode != ::mode || blocked)) {
	rlimits (-1; -1) {
	    if (mode == MODE_DISCONNECT) {
		if (conntype != "datagram") {
		    send_close();
		}
		if (timeout != 0) {
		    remove_call_out(timeout);
		}
		timeout = call_out("timeout", DISCONNECT_TIMEOUT);
	    } else if (mode >= MODE_UNBLOCK) {
		if (mode - MODE_UNBLOCK != blocked) {
		    block_input(blocked = mode - MODE_UNBLOCK);
		}
		return;
	    }

	    if (blocked) {
		block_input(blocked = FALSE);
	    }
	    ::mode = mode;
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
 * NAME:	flow()
 * DESCRIPTION:	enable flow state
 */
void flow()
{
    if (previous_object() == user) {
	flow = TRUE;
    }
}

/*
 * NAME:	flow_mode()
 * DESCRIPTION:	flow: set the mode
 */
static void flow_mode(mapping tls, int mode)
{
    set_mode(mode);
}


/*
 * NAME:	login()
 * DESCRIPTION:	call login in user
 */
static int login(string str)
{
    return user->login(str);
}

/*
 * NAME:	open()
 * DESCRIPTION:	open the connection
 */
static void open(mapping tls)
{
    if (user) {
	set_mode(login(nil));
    } else {
	int mode, delay;
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

	delay = call_other(userd, "query_" + conntype + "_timeout", port,
			   this_object());
	if (delay > 0) {
	    timeout = call_out("timeout", delay);
	} else if (delay < 0) {
	    /* disconnect immediately */
	    mode = MODE_DISCONNECT;
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
    mode = MODE_DISCONNECT;
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
	    if (flow) {
		user->flow_logout(dest);
	    } else {
		catch {
		    user->logout(dest);
		}
	    }
	}
	if (!dest) {
	    mode = MODE_DISCONNECT;
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
    if (previous_program() == LIB_USER && mode != MODE_DISCONNECT) {
	mode = MODE_DISCONNECT;
	destruct_object(this_object());
    }
}

/*
 * NAME:	_disconnect()
 * DESCRIPTION:	break connection
 */
static void _disconnect(mapping tls)
{
    if (mode != MODE_DISCONNECT) {
	mode = MODE_DISCONNECT;
	destruct_object(this_object());
    }
}

/*
 * NAME:	flow_disconnect()
 * DESCRIPTION:	flow: break connection
 */
void flow_disconnect()
{
    if (previous_program() == LIB_USER) {
	call_out("_disconnect", 0);
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
	    if (flow) {
		user->flow_logout(FALSE);
	    } else {
		catch {
		    user->logout(FALSE);
		}
	    }
	}
	mode = MODE_DISCONNECT;
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
void set_user(object LIB_USER obj, varargs string str)
{
    if (KERNEL() || SYSTEM()) {
	flow = FALSE;
	user = obj;
	if (str) {
	    set_mode(login(str));
	}
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
    if (!user || user->query_conn() != this_object() || user->timeout()) {
	mode = MODE_DISCONNECT;
	destruct_object(this_object());
    }
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	forward a message to user object
 */
static int receive_message(mapping tls, string str)
{
    if (mode != MODE_DISCONNECT) {
	if (!user) {
	    user = call_other(userd, conntype + "_user", port, str);
	    return login(str);
	} else if (flow) {
	    if (user->flow_receive_message(str, mode)) {
		return MODE_BLOCK;
	    }
	} else {
	    return user->receive_message(str);
	}
    }
    return MODE_NOCHANGE;
}

/*
 * NAME:	_message()
 * DESCRIPTION:	send a message across the connection
 */
static int _message(string str, object prev)
{
    if (prev == user && !buffer) {
	rlimits (-1; -1) {
	    int len;

	    len = send_message(str);
	    if (len == strlen(str)) {
		return TRUE;
	    }

	    /*
	     * string couldn't be sent completely; buffer the remainder
	     */
	    buffer = str[len ..];
	}
    }
    return FALSE;
}

/*
 * NAME:	message()
 * DESCRIPTION:	send a message across the connection
 */
int message(string str)
{
    return _message(str, previous_object());
}

/*
 * NAME:	flow_message()
 * DESCRIPTION:	flow: send a message across the connection
 */
void flow_message(string str)
{
    call_out("_message", 0, str, previous_object());
}

/*
 * NAME:	message_done()
 * DESCRIPTION:	called when output is completed
 */
static int message_done(mapping tls)
{
    if (mode != MODE_DISCONNECT) {
	if (buffer) {
	    int len;

	    len = send_message(buffer);
	    buffer = (len != strlen(buffer)) ? buffer[len ..] : nil;
	} else if (user) {
	    if (flow) {
		user->flow_message_done();
	    } else {
		return user->message_done();
	    }
	}
    }
    return MODE_NOCHANGE;
}

/*
 * NAME:	_datagram_challenge()
 * DESCRIPTION:	set the challenge for the datagram channel
 */
static void _datagram_challenge(string str, object prev)
{
    if (prev == user) {
	::datagram_challenge(str);
    }
}

/*
 * NAME:	datagram_challenge()
 * DESCRIPTION:	set the challenge for the datagram channel
 */
void datagram_challenge(string str)
{
    _datagram_challenge(str, previous_object());
}

/*
 * NAME:	flow_datagram_challenge()
 * DESCRIPTION:	flow: set the challenge for the datagram channel
 */
void flow_datagram_challenge(string str)
{
    call_out("_datagram_challenge", 0, str, previous_object());
}

/*
 * NAME:	datagram_attach()
 * DESCRIPTION:	attach a datagram channel to this connection
 */
static void datagram_attach(mapping tls)
{
    if (mode != MODE_DISCONNECT && user) {
	if (flow) {
	    user->flow_datagram_attach();
	} else {
	    user->datagram_attach();
	}
    }
}

/*
 * NAME:	receive_datagram()
 * DESCRIPTION:	forward a datagram to the user
 */
static int receive_datagram(mapping tls, string str)
{
    if (mode != MODE_DISCONNECT && user) {
	if (flow) {
	    if (user->flow_receive_datagram(str)) {
		return MODE_BLOCK;
	    }
	} else {
	    user->receive_datagram(str);
	}
    }
    return MODE_NOCHANGE;
}

/*
 * NAME:	_datagram()
 * DESCRIPTION:	send a datagram across the connection
 */
static int _datagram(string str, object prev)
{
    if (prev == user) {
	return (send_datagram(str) == strlen(str));
    }
}

/*
 * NAME:	datagram()
 * DESCRIPTION:	send a datagram across the connection
 */
int datagram(string str)
{
    return _datagram(str, previous_object());
}

/*
 * NAME:	flow_datagram()
 * DESCRIPTION:	flow: send a datagram across the connection
 */
void flow_datagram(string str)
{
    call_out("_datagram", 0, str, previous_object());
}
