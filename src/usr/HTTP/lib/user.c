# include <kernel/kernel.h>
# include <kernel/user.h>
# include <String.h>
# include "http.h"

inherit	user "~System/lib/user";


private mixed *request;	/* HTTP request */
private string headers;	/* HTTP 1.x headers */
private object mesg;	/* entity included in request, or output message */
private int length;	/* length of entity to receive */

/*
 * NAME:	internal_error()
 * DESCRIPTION:	output an internal error message
 */
private int internal_error(void)
{
    ::message("<HTML>\n<HEAD><TITLE>" + HTTP_INTERNAL_ERROR +
	      " Internal Error</TITLE></HEAD>\n<BODY><H1>" +
	      HTTP_INTERNAL_ERROR +
	      " Internal Error</H1></BODY>\n</HTML>\n");
    return MODE_DISCONNECT;
}

/*
 * NAME:	message()
 * DESCRIPTION:	send a message, return bit remaining to be sent, or nil
 */
static object message(object mesg)
{
    string str;

    str = mesg->chunk();
    if (!str || strlen(str) == 0) {
	return nil;
    }

    ::message(str);
    return mesg;
}

/*
 * NAME:	message_done()
 * DESCRIPTION:	output remainer of message, close connection if done
 */
nomask int message_done(void)
{
    if (previous_program() == LIB_CONN) {
	mesg = call_limited("message", mesg);
	return (mesg) ? MODE_RAW : MODE_DISCONNECT;
    }
}

/*
 * NAME:	respond()
 * DESCRIPTION:	generate a response to the HTTP request, and send it
 */
private int respond(int code)
{
    string host;

    set_mode(MODE_RAW);
    catch {
	mesg = call_limited("http_message", code, mesg);
    } : {
	return internal_error();
    }

    code = this_object()->query_code();
    host = this_object()->query_host();
    if (!host) {
	host = "";
    }
    ::login("Connection: HTTP from " + address() + ", " + code + " " +
	    this_object()->query_method() + " " + host +
	    this_object()->query_path() + "\n");

    mesg = call_limited("message", mesg);
    return MODE_RAW;
}

/*
 * NAME:	request()
 * DESCRIPTION:	called by binary connection manager to forward HTTP request
 *		info
 */
nomask void request(mixed *req)
{
    if (previous_program() == "/usr/WWW/sys/server") {
	request = req;
    }
}

/*
 * NAME:	login()
 * DESCRIPTION:	accept HTTP connection
 */
nomask int login(string str)
{
    int code;

    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	catch {
	    code = call_limited("http_request", request);
	} : {
	    return internal_error();
	}

	if (code != 0 || request[HTTPREQ_VERSION] < 1.0) {
	    return respond(code);
	}
	headers = "";
    }

    return MODE_LINE;
}

/*
 * NAME:	logout()
 * DESCRIPTION:	break HTTP connection
 */
nomask void logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	::destruct_object(this_object());
    }
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	receive (part of) HTTP message
 */
nomask int receive_message(string str)
{
    int code, len;

    if (previous_program() == LIB_CONN) {
	if (headers) {
	    if (strlen(str) != 0) {
		headers += str + "\n";
		return MODE_LINE;
	    }
	    str = headers;
	    headers = nil;
	    catch {
		code = call_limited("http_headers", str);
	    } : {
		return internal_error();
	    }

	    if (code == 0) {
		length = call_limited("query_content_length");
		if (length >= 0) {
		    mesg = new StringBuffer;
		    if (length > 0) {
			return MODE_RAW;
		    }
		}
	    }

	    return respond(code);
	} else if (mesg && (len=mesg->length()) < length) {
	    if (len + strlen(str) > length) {
		str = str[.. length - len - 1];
	    }
	    mesg->append(str);
	    if (len + strlen(str) == length) {
		return respond(0);
	    }
	}
    }

    return MODE_RAW;
}


/*
 * disabled functions
 */
void connection(void) { }
void redirect(void) { }
void datagram_challenge(string str) { }
int datagram(string str) { return 0; }
int receive_datagram(string str) { return 0; }
