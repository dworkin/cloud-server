# include <kernel/user.h>
# include <String.h>
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include "HttpConnection.h"

inherit HttpConnection;


object server;		/* associated server object */
string requestPath;	/* HttpRequest object path */
string responsePath;	/* HttpResponse object path */
string headersPath;	/* HttpFields object path */
HttpRequest request;	/* HTTP request */
string headers;		/* HTTP 1.x headers */
StringBuffer inbuf;	/* entity included in request */
int length;		/* length of entity to receive */
StringBuffer outbuf;	/* output buffer */


/*
 * initialize connection object
 */
static void create(object server, varargs string requestPath,
		   string responsePath, string headersPath)
{
    ::server = server;
    ::requestPath = (requestPath) ?
		     requestPath : OBJECT_PATH(RemoteHttpRequest);
    ::responsePath = (responsePath) ?
		      responsePath : OBJECT_PATH(HttpResponse);
    ::headersPath = (headersPath) ?
		     headersPath : OBJECT_PATH(RemoteHttpFields);
    call_out("disconnect", 300);
}

/*
 * prepare to receive a request
 */
void expectRequest()
{
    if (previous_object() == server) {
	set_mode(MODE_LINE);
	request = nil;
    }
}

/*
 * parse and verify a HTTP request
 */
static int receiveRequestLine(string str)
{
    request = new_object(requestPath, str);
    if (request->version() >= 2.0) {
	return HTTP_NOT_IMPLEMENTED;
    }
    return ::receiveRequestLine(request);
}

/*
 * parse and verify HTTP headers
 */
static int receiveRequestHeaders(string str)
{
    return ::receiveRequestHeaders(request, new_object(headersPath, str));
}

/*
 * receive a request
 */
static int receiveRequest(int code, HttpRequest request)
{
    int mode;
    string host;

    set_mode(MODE_BLOCK);
    try {
	code = server->receiveRequest(code, request);
    } catch (...) {
	code = HTTP_INTERNAL_ERROR;
    }

    if (request) {
	host = request->host();
	if (!host) {
	    host = "";
	}
	::login("HTTP from " + address() + ", " + code + " " +
		request->method() + " " + host + request->path() + "\n");
    } else {
	::login("HTTP from " + address() + ", " + code + "\n");
    }

    return code;
}

/*
 * prepare to receive an entity
 */
void expectEntity(int length)
{
    if (previous_object() == server) {
	set_mode(MODE_RAW);
	inbuf = new StringBuffer;
	set_message_length(::length = length);
    }
}

/*
 * receive an entity
 */
static void receiveEntity(StringBuffer entity)
{
    set_mode(MODE_BLOCK);
    server->receiveEntity(request, entity);
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
	    code = call_limited("receiveRequestLine", str);
	} catch (...) {
	    code = HTTP_BAD_REQUEST;
	}

	if (code != 0 || request->version() < 1.0) {
	    /*
	     * call receiveRequest() early, on error output a raw HTML message
	     * and disconnect immediately
	     */
	    switch (call_limited("receiveRequest", code, request)) {
	    case HTTP_OK:
		return MODE_NOCHANGE;

	    case HTTP_INTERNAL_ERROR:
		::message("<HTML>\n<HEAD><TITLE>" + HTTP_INTERNAL_ERROR +
			  " Internal Error</TITLE></HEAD>\n<BODY><H1>" +
			  HTTP_INTERNAL_ERROR +
			  " Internal Error</H1></BODY>\n</HTML>\n");
		return MODE_DISCONNECT;

	    default:
		::message("<HTML>\n<HEAD><TITLE>" + HTTP_BAD_REQUEST +
			  " Bad Request</TITLE></HEAD>\n<BODY><H1>" +
			  HTTP_BAD_REQUEST +
			  " Bad Request</H1></BODY>\n</HTML>\n");
		return MODE_DISCONNECT;
	    }
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
	server->logout();
	destruct_object(this_object());
    }
}

/*
 * receive (part of) HTTP message
 */
int receive_message(string str)
{
    int code, len;
    StringBuffer entity;

    if (previous_program() == LIB_CONN) {
	if (headers) {
	    /*
	     * headers
	     */
	    if (strlen(str) != 0) {
		headers += str + "\n";
		return MODE_LINE;
	    }
	    str = headers;
	    headers = nil;
	    try {
		code = call_limited("receiveRequestHeaders", str);
	    } catch (...) {
		code = HTTP_BAD_REQUEST;
	    }

	    switch (call_limited("receiveRequest", code, request)) {
	    case HTTP_OK:
		return MODE_NOCHANGE;

	    case HTTP_INTERNAL_ERROR:
		::message(httpInternalError(responsePath)->transport());
		return MODE_DISCONNECT;

	    default:
		::message(httpBadRequest(responsePath)->transport());
		return MODE_DISCONNECT;
	    }
	} else if (inbuf) {
	    /*
	     * entity
	     */
	    inbuf->append(str);
	    length -= strlen(str);
	    if (length == 0) {
		entity = inbuf;
		inbuf = nil;
		call_limited("receiveEntity", entity);
	    }
	} else {
	    /*
	     * request
	     */
	    if (strlen(str) == 0) {
		return MODE_NOCHANGE;
	    }

	    try {
		code = call_limited("receiveRequestLine", str);
		if (request->version() < 1.0) {
		    code = HTTP_BAD_REQUEST;
		}
	    } catch (...) {
		code = HTTP_BAD_REQUEST;
	    }

	    if (code != 0) {
		switch (call_limited("receiveRequest", code, request)) {
		case HTTP_OK:
		    return MODE_NOCHANGE;

		case HTTP_INTERNAL_ERROR:
		    ::message(httpInternalError(responsePath)->transport());
		    return MODE_DISCONNECT;

		default:
		    ::message(httpBadRequest(responsePath)->transport());
		    return MODE_DISCONNECT;
		}
	    }
	    headers = "";
	    return MODE_LINE;
	}
    }

    return MODE_RAW;
}

/*
 * send (part of) a message
 */
static int message(StringBuffer buffer)
{
    string str;

    str = buffer->chunk();
    if (!str || strlen(str) == 0) {
	return FALSE;
    } else {
	::message(str);
	return TRUE;
    }
}

/*
 * output remainer of message
 */
int message_done()
{
    if (previous_program() == LIB_CONN && !call_limited("message", outbuf)) {
	server->chunkDone();
    }
    return MODE_NOCHANGE;
}

/*
 * send a response
 */
void sendResponse(HttpResponse response)
{
    if (previous_object() == server) {
	::sendResponse(response);
	message(outbuf = new StringBuffer(response->transport()));
    }
}

/*
 * send a chunk of data
 */
void sendChunk(StringBuffer chunk)
{
    if (previous_object() == server) {
	message(outbuf = chunk);
    }
}

/*
 * break the connection
 */
void disconnect()
{
    if (previous_object() == this_object() || previous_object() == server) {
	::disconnect();
    }
}
