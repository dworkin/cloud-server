# include <kernel/user.h>
# include <String.h>
# include "SimUser.h"


static int receive_message(string str);
static void logout(int quit);
static int message_done();
static int buffered_input();
static void restart_input();

private SimUser remote;		/* remote user */
private StringBuffer outbuf;	/* output buffer */
private int awaitingDone;	/* awaiting output (mostly) done */
private StringBuffer inbuf;	/* input buffer */
private string chunk;		/* chunk from input buffer */
private int pendingDone;	/* input is being processed */
private int mode;		/* connection mode */
private int blocked;		/* connection blocked? */
private int length;		/* expected length */
private int noline;		/* no full line in input buffer */
private int receive, restart;	/* input callout handles */

static void connect(string host, int port) { }	/* simulation only */

/*
 * initialize simulated user
 */
static void create(int mode)
{
    inbuf = new StringBuffer(nil, 32767);
    chunk = "";
    ::mode = mode;
}

/*
 * establish connection
 */
static void connection(SimUser remote)
{
    ::remote = remote;
}

/*
 * add message to output buffer
 */
static int message(string str)
{
    if (outbuf) {
	if (outbuf->length() > 65535) {
	    return FALSE;
	}
	outbuf->append(str);
    } else {
	outbuf = new StringBuffer(str);
	if (!awaitingDone) {
	    call_out("sim_send", 0);
	}
    }

    return (outbuf->length() <= 65535);
}

/*
 * send buffered message
 */
static void sim_send()
{
    if (!awaitingDone) {
	string chunk;

	chunk = outbuf->chunk();
	if (outbuf->length() == 0) {
	    outbuf = nil;
	}
	if (remote) {
	    awaitingDone = TRUE;
	    remote->remote_message(chunk);
	}
    }
}

/*
 * remote is done
 */
void remote_done()
{
    call_out("sim_done", 0, previous_object());
}

/*
 * simulated message_done()
 */
static void sim_done(object caller)
{
    if (caller == remote) {
	awaitingDone = FALSE;
	if (outbuf) {
	    sim_send();
	} else {
	    message_done();
	}
    }
}

/*
 * break connection
 */
static void disconnect()
{
    if (remote) {
	remote->remote_disconnect();
    }
    logout(TRUE);
}

/*
 * remote broke the connection
 */
void remote_disconnect()
{
    call_out("sim_disconnect", 0, previous_object());
}

/*
 * connection is broken
 */
static void sim_disconnect(object caller)
{
    if (caller == remote) {
	logout(FALSE);
    }
}

/*
 * set connection mode
 */
static void set_mode(int mode)
{
    if (::mode != MODE_DISCONNECT) {
	switch (mode) {
	case MODE_BLOCK:
	    blocked = TRUE;
	    return;

	case MODE_UNBLOCK:
	    if (blocked) {
		blocked = FALSE;
		break;
	    }
	    return;

	case MODE_NOCHANGE:
	    return;

	case MODE_DISCONNECT:
	    ::mode = mode;
	    disconnect();
	    return;

	default:
	    ::mode = mode;
	    if (blocked) {
		blocked = FALSE;
		break;
	    }
	    return;
	}

	if (restart == 0) {
	    if (inbuf->length() != 0 || strlen(chunk) != 0) {
		switch (mode) {
		case MODE_LINE:
		case MODE_EDIT:
		    if (!noline) {
			break;
		    }
		    /* fall through */
		default:
		    if (!buffered_input()) {
			return;
		    }
		    /* fall through */
		case MODE_RAW:
		    break;
		}
	    } else if (!buffered_input()) {
		return;
	    }

	    if (receive != 0) {
		remove_call_out(receive);
		receive = 0;
	    }
	    restart = call_out("sim_restart_input", 0);
	}
    }
}

/*
 * set expected message length
 */
static void set_message_length(int length)
{
    ::length = length;
}

/*
 * process input
 */
static void sim_receive_buffer()
{
    int len;
    string str, head, pre;

    receive = 0;
    if (!blocked && mode != MODE_DISCONNECT) {
	if (strlen(chunk) <= 32768 && inbuf->length() != 0) {
	    chunk += inbuf->chunk();
	    noline = FALSE;
	} else if (strlen(chunk) == 0) {
	    if (pendingDone) {
		pendingDone = FALSE;
		if (remote) {
		    remote->remote_done();
		}
	    }
	    return;
	}

	if (mode != MODE_RAW) {
	    if (!noline) {
		if (sscanf(chunk, "%s\r\n%s", str, chunk) != 0 ||
		    sscanf(chunk, "%s\n%s", str, chunk) != 0) {
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

		    set_mode(receive_message(str));
		} else {
		    noline = TRUE;
		}
	    }
	} else if (strlen(chunk) != 0) {
	    if (length > 0) {
		if (length < strlen(chunk)) {
		    str = chunk[.. length - 1];
		    chunk = chunk[length ..];
		    length = 0;
		} else {
		    length -= strlen(chunk);
		    str = chunk;
		    chunk = "";
		}
	    } else {
		str = chunk;
		chunk = "";
	    }
	    set_mode(receive_message(str));
	}
    }

    if (mode != MODE_DISCONNECT) {
	if (inbuf->length() == 0) {
	    if (pendingDone) {
		pendingDone = FALSE;
		if (remote) {
		    remote->remote_done();
		}
	    }

	    if (strlen(chunk) == 0 || (mode != MODE_RAW && noline)) {
		return;
	    }
	}

	if (!blocked && !restart) {
	    receive = call_out("sim_receive_buffer", 0);
	}
    }
}

/*
 * receive message from remote
 */
void remote_message(string input)
{
    call_out("sim_receive_message", 0, previous_object(), input);
}

/*
 * message received
 */
static void sim_receive_message(object caller, string input)
{
    if (caller == remote) {
	if (receive != 0) {
	    remove_call_out(receive);
	    receive = 0;
	}
	if (restart != 0) {
	    remove_call_out(restart);
	    restart = 0;
	}

	inbuf->append(input);
	pendingDone = TRUE;
	sim_receive_buffer();
    }
}

/*
 * input restarted
 */
static void sim_restart_input()
{
    restart = 0;
    if (!blocked && mode != MODE_DISCONNECT) {
	restart_input();
	sim_receive_buffer();
    }
}
