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

/*
 * initialize HTTP1 connection with buffered input
 */
static void create(object relay, string trailersPath)
{
    ::create(relay, trailersPath);
    buffer = new StringBuffer(nil, 32767);
    chunk = "";
    mode = MODE_RAW;
}

/*
 * set the local input mode
 */
static void set_mode(int mode)
{
    switch (mode) {
    case MODE_BLOCK:
	blocked = TRUE;
	break;

    case MODE_UNBLOCK:
	blocked = FALSE;
	break;

    case MODE_LINE:
    case MODE_EDIT:
    case MODE_RAW:
	blocked = FALSE;
	::mode = mode;
	mode = MODE_RAW;
	break;

    case MODE_NOCHANGE:
	return;
    }

    ::set_mode(mode);
}

/*
 * set the local message length
 */
static void set_message_length(int length)
{
    ::length = length;
}

/*
 * process first message
 */
static int receiveFirstMessage(string str)
{
    return ::receiveBytes(str);
}

/*
 * process input buffer
 */
private void receiveBuffer()
{
    int len;
    string str, head, pre;

    while (!blocked && mode != MODE_DISCONNECT) {
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
		    set_mode(receiveFirstMessage(str));
		} else {
		    set_mode(::receiveBytes(str));
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
		set_mode(receiveFirstMessage(str));
	    } else {
		set_mode(::receiveBytes(str));
	    }
	}
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
 * is there buffered input?
 */
int buffered_input()
{
    if (strlen(chunk) == 0 && buffer->length() == 0) {
	return FALSE;
    }
    return (mode == MODE_RAW) ? TRUE : !noline;
}

/*
 * reprocess pending input
 */
static void restartInput()
{
    receiveBuffer();
}
