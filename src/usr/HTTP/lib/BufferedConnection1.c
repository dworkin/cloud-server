# include <kernel/user.h>
# include <String.h>

inherit "Connection1";


private StringBuffer buffer;	/* input buffer */
private string chunk;		/* chunk from input buffer */
private int mode;		/* input mode */
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
 * process input buffer
 */
private void receiveBuffer()
{
    int len;
    string str, head, pre;

    while (!blocked && mode != MODE_DISCONNECT) {
	if (strlen(chunk) < 32767 && buffer->length() != 0) {
	    chunk += buffer->chunk();
	}
	if (mode != MODE_RAW) {
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

		set_mode(::receive_message(str));
	    } else {
		noline = TRUE;
		break;
	    }
	} else {
	    if (strlen(chunk) != 0) {
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
		set_mode(::receive_message(str));
	    }
	    break;
	}
    }
}

/*
 * receive a message
 */
static void receive_message(StringBuffer str)
{
    buffer->append(str);
    noline = FALSE;
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
    return (mode == MODE_LINE || mode == MODE_EDIT) ? !noline : TRUE;
}

/*
 * reprocess pending input
 */
void restart_input()
{
    if (previous_program() == BINARY_CONN) {
	receiveBuffer();
    }
}
