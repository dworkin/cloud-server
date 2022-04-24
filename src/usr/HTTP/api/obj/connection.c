# include <kernel/kernel.h>
# include <kernel/user.h>
# include <String.h>
# include "HttpRequest.h"
# include "HttpHeader.h"
# include "HttpResponse.h"
# include "HttpConnection.h"

inherit "~System/lib/user";
inherit HttpConnection;


object user;		/* associated user */
HttpRequest request;	/* HTTP request */
string headers;		/* HTTP 1.x headers */
object mesg;		/* entity included in request, or output message */
int length;		/* length of entity to receive */

/*
 * initialize connection object
 */
static void create(object user)
{
    ::user = user;
    call_out("disconnect", 300);
}

/*
 * output an internal error message
 */
private int internal_error()
{
    ::message("<HTML>\n<HEAD><TITLE>" + HTTP_INTERNAL_ERROR +
	      " Internal Error</TITLE></HEAD>\n<BODY><H1>" +
	      HTTP_INTERNAL_ERROR +
	      " Internal Error</H1></BODY>\n</HTML>\n");
    return MODE_DISCONNECT;
}

/*
 * output a bad request message
 */
private int bad_request()
{
    ::message("<HTML>\n<HEAD><TITLE>" + HTTP_BAD_REQUEST +
	      " Bad Request</TITLE></HEAD>\n<BODY><H1>" +
	      HTTP_BAD_REQUEST +
	      " Bad Request</H1></BODY>\n</HTML>\n");
    return MODE_DISCONNECT;
}

/*
 * send a message, return bit remaining to be sent, or nil
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
 * output remainer of message, close connection if done
 */
int message_done()
{
    if (previous_program() == LIB_CONN) {
	mesg = call_limited("message", mesg);
	return (mesg) ? MODE_RAW : MODE_DISCONNECT;
    }
}

/*
 * generate a response to the HTTP request, and send it
 */
static int respond(int code)
{
    int code;
    string host;

    set_mode(MODE_RAW);
    try {
	({ code, mesg }) = user->http_message(code, request, mesg);
    } catch (...) {
	return internal_error();
    }

    host = request->host();
    if (!host) {
	host = "";
    }
    ::login("Connection: HTTP from " + address() + ", " + code + " " +
	    request->method() + " " + host + request->path() + "\n");

    mesg = message(mesg);
    return MODE_RAW;
}

/*
 * parse and verify a HTTP request
 */
static int httpRequest(string str)
{
    request = new RemoteHttpRequest(str);
    if (request->version() >= 2.0) {
	return HTTP_NOT_IMPLEMENTED;
    }
    length = -1;
    return ::httpRequest(request);
}

/*
 * accept HTTP connection
 */
int login(string str)
{
    int code;

    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	try {
	    code = call_limited("httpRequest", str);
	} catch (...) {
	    return bad_request();
	}

	if (code != 0 || request->version() < 1.0) {
	    return call_limited("respond", code);
	}
	headers = "";
    }

    return MODE_LINE;
}

/*
 * break HTTP connection
 */
void logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	user->logout();
	::destruct_object(this_object());
    }
}

/*
 * parse and verify HTTP headers
 */
static int httpHeaders(string str)
{
    HttpHeaders headers;
    HttpHeader header;

    headers = new RemoteHttpHeaders(str);
    header = headers->get("content-length");
    if (header) {
	length = header->value();
    }
    return ::httpHeaders(headers);
}

/*
 * receive (part of) HTTP message
 */
int receive_message(string str)
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
	    try {
		code = call_limited("httpHeaders", str);
	    } catch (...) {
		return bad_request();
	    }

	    if (code == 0) {
		if (length >= 0) {
		    mesg = new StringBuffer;
		    if (length > 0) {
			return MODE_RAW;
		    }
		}
	    }

	    return call_limited("respond", code);
	} else if (mesg && (len=mesg->length()) < length) {
	    if (len + strlen(str) > length) {
		str = str[.. length - len - 1];
	    }
	    mesg->append(str);
	    if (len + strlen(str) == length) {
		return call_limited("respond", 0);
	    }
	}
    }

    return MODE_RAW;
}
