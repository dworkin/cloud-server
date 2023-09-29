# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"

inherit "~/lib/BufferedConnection1";


object server;			/* associated server object */
string requestPath;		/* HttpRequest object path */
string headersPath;		/* HttpFields object path */
TlsServerSession session;	/* TLS session */
int connected;			/* TLS connection established */
HttpRequest request;		/* HTTP request */


/*
 * initialize connection object
 */
static void create(object server, string certificate, string key,
		   varargs string requestPath, string fieldsPath,
		   string tlsServerSessionPath)
{
    ::server = server;
    ::requestPath = (requestPath) ?
		     requestPath : OBJECT_PATH(RemoteHttpRequest);
    headersPath = (fieldsPath) ?
		   fieldsPath : OBJECT_PATH(RemoteHttpFields);
    if (!tlsServerSessionPath) {
	tlsServerSessionPath = OBJECT_PATH(TlsServerSession);
    }
    session = new_object(tlsServerSessionPath, certificate, key);
    ::create(server, headersPath);
    call_out("disconnect", 300);
}

/*
 * create internal error response
 */
private HttpResponse httpInternalError()
{
    HttpResponse response;
    HttpFields headers;

    response = new HttpResponse(1.1, HTTP_INTERNAL_ERROR, "Internal Error");
    headers = new HttpFields();
    headers->add(new HttpField("Date", new HttpTime));
    headers->add(new HttpField("Connection", ({ "close" })));
    response->setHeaders(headers);

    return response;
}

/*
 * parse and verify a HTTP request
 */
static int receiveRequestLine(string str)
{
    request = new_object(requestPath, str);
    return ::receiveRequestLine(request);
}

/*
 * receive a request
 */
static int receiveRequest(int code, HttpRequest request)
{
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
	::login("HTTPS from " + address() + ", " + code + " " +
		request->method() + " " + host + request->path() + "\n");
    } else {
	::login("HTTPS from " + address() + ", " + code + "\n");
    }

    return code;
}

/*
 * receive request headers
 */
static int receiveHeaders(string str)
{
    int code;

    try {
	code = receiveRequestHeaders(request, new_object(headersPath, str));
    } catch (...) {
	code = HTTP_BAD_REQUEST;
    }

    switch (receiveRequest(code, request)) {
    case HTTP_INTERNAL_ERROR:
	message(httpInternalError()->transport());
	return MODE_DISCONNECT;

    case HTTP_BAD_REQUEST:
	return MODE_DISCONNECT;

    default:
	return MODE_NOCHANGE;
    }
}

/*
 * finished handling a request
 */
void doneRequest()
{
    if (previous_object() == server) {
	set_mode((persistent()) ? MODE_LINE : MODE_DISCONNECT);
    }
}

/*
 * accept HTTPS connection
 */
int login(string str)
{
    StringBuffer input, output;
    string warning, status;

    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	session->accept(FALSE);
	({ input, output, warning, status }) = session->receiveMessage(str);
	if (output) {
	    ::sendMessage(output, TRUE);
	}
	if (!status) {
	    connected = TRUE;
	    set_mode(MODE_LINE);
	}
	if (input) {
	    ::receive_message(input);
	}
	return (status && status != "connecting") ?
		MODE_DISCONNECT : MODE_NOCHANGE;
    }
}

/*
 * process login message
 */
static int receiveFirstMessage(string str)
{
    int code;

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
	    message("<HTML>\n<HEAD><TITLE>" + HTTP_INTERNAL_ERROR +
		    " Internal Error</TITLE></HEAD>\n<BODY><H1>" +
		    HTTP_INTERNAL_ERROR +
		    " Internal Error</H1></BODY>\n</HTML>\n");
	    /* fall through */
	default:
	    return MODE_DISCONNECT;
	}
    }

    startHeaders();
    return MODE_LINE;
}

/*
 * receive a message
 */
int receive_message(string str)
{
    StringBuffer input, output;
    string warning, status;

    if (previous_program() == LIB_CONN) {
	({ input, output, warning, status }) = session->receiveMessage(str);
	if (output) {
	    ::sendMessage(output, TRUE);
	}
	if (!status && !connected) {
	    connected = TRUE;
	    set_mode(MODE_LINE);
	}
	if (input) {
	    ::receive_message(input);
	}
	return (status && status != "connecting") ?
		MODE_DISCONNECT : MODE_NOCHANGE;
    }
}

/*
 * terminate connection
 */
void logout(int quit)
{
    StringBuffer output;

    if (previous_program() == LIB_CONN) {
	::logout(quit);
	output = session->close();
	if (output) {
	    ::sendMessage(output, TRUE);
	}
	destruct_object(this_object());
    }
}

/*
 * send an encrypted message
 */
static void sendMessage(StringBuffer str)
{
    ::sendMessage(session->sendMessage(str));
}

/*
 * send remainder of message
 */
int message_done()
{
    if (previous_program() == LIB_CONN) {
	return ::message_done();
    }
}

/*
 * receive a request
 */
static int receiveMessage(string str)
{
    int code;

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
        case HTTP_INTERNAL_ERROR:
            message(httpInternalError()->transport());
            return MODE_DISCONNECT;

        case HTTP_BAD_REQUEST:
            return MODE_DISCONNECT;

        default:
            return MODE_NOCHANGE;
        }
    }

    startHeaders();
    return MODE_LINE;
}

/*
 * send a response
 */
void sendResponse(HttpResponse response)
{
    if (previous_object() == server) {
	::sendResponse(response);
	sendMessage(new StringBuffer(response->transport()));
    }
}
