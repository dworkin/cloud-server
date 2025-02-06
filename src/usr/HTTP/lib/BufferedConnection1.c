# include <kernel/user.h>
# include <String.h>

inherit "Connection1";


private StringBuffer buffer;	/* input buffer */
private string chunk;		/* chunk from input buffer */
private int mode;		/* input mode */
private int established;	/* first message received? */
private int blocked;		/* input blocked? */
private int length;		/* length of message to receive */
private int noline;		/* no full line in input? */
private int start;		/* starting buffered input? */

/*
 * initialize HTTP1 connection with buffered input
 */
static void create(int mode)
{
    buffer = new StringBuffer(nil, 32767);
    chunk = "";
    ::mode = mode;
}

/*
 * start processing input
 */
static void startInput()
{
    if ((strlen(chunk) != 0 || buffer->length() != 0) &&
	(mode == MODE_RAW || !noline)) {
	if (start == 0) {
	    start = call_out("startBufferedInput", 0);
	}
    } else {
	::setMode(MODE_UNBLOCK);
    }
}

/*
 * set the local input mode
 */
static void setMode(int mode, varargs int length)
{
    switch (mode) {
    case MODE_BLOCK:
	blocked = TRUE;
	break;

    case MODE_LINE:
    case MODE_EDIT:
    case MODE_RAW:
	::mode = mode;
	::length = length;
	/* fall through */
    case MODE_UNBLOCK:
	if (blocked) {
	    blocked = FALSE;
	    startInput();
	}
	break;
    }
}

/*
 * process first message
 */
static void receiveFirstMessage(string str)
{
    ::receiveBytes(str);
}

/*
 * process input buffer
 */
private void receiveBuffer()
{
    int len;
    string str, head, pre;

    while (!blocked && mode != MODE_DISCONNECT && this_object()) {
	if (strlen(chunk) <= 32768 && buffer->length() != 0) {
	    chunk += buffer->chunk();
	    noline = FALSE;
	} else if (strlen(chunk) == 0) {
	    break;
	}

	if (mode != MODE_RAW) {
	    if (noline) {
		break;
	    }
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

		if (!established) {
		    established = TRUE;
		    receiveFirstMessage(str);
		} else {
		    blocked = TRUE;
		    ::receiveBytes(str);
		}
	    } else {
		noline = TRUE;
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
	    if (!established) {
		established = TRUE;
		receiveFirstMessage(str);
	    } else {
		blocked = TRUE;
		::receiveBytes(str);
	    }
	}
    }

    if (start != 0) {
	remove_call_out(start);
	start = 0;
    }
}

/*
 * receive a message
 */
static void receiveBytes(StringBuffer str)
{
    buffer->append(str);
    receiveBuffer();
}

/*
 * process buffered input
 */
static void startBufferedInput()
{
    start = 0;
    receiveBuffer();
}
