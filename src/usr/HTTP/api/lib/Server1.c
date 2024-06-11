# include <kernel/user.h>
# include <String.h>
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include <config.h>
# include <version.h>
# include <status.h>

inherit "~/lib/Connection1";


private object server;		/* associated server object */
private string requestPath;	/* HttpRequest object path */
private string headersPath;	/* HttpFields object path */
private HttpRequest request;	/* HTTP request */


/*
 * initialize connection object
 */
static void create(object server, string requestPath, string headersPath)
{
    ::server = server;
    ::requestPath = requestPath;
    ::headersPath = headersPath;
    ::create(server, headersPath);
    call_out("terminate", 300);
}

/*
 * internal error message
 */
static string htmlInternalError()
{
    return "<HTML>\n<HEAD><TITLE>" + HTTP_INTERNAL_ERROR +
	   " Internal Error</TITLE></HEAD>\n<BODY><H1>" +
	   HTTP_INTERNAL_ERROR + " Internal Error</H1></BODY>\n</HTML>\n";
}

/*
 * bad request message
 */
static string htmlBadRequest()
{
    return "<HTML>\n<HEAD><TITLE>" + HTTP_BAD_REQUEST +
	   " Bad Request</TITLE></HEAD>\n<BODY><H1>" +
	   HTTP_BAD_REQUEST + " Bad Request</H1></BODY>\n</HTML>\n";
}

/*
 * send internal error response
 */
static void sendInternalError()
{
    HttpResponse response;
    HttpFields headers;
    string str;
    StringBuffer message;

    response = new HttpResponse(1.1, HTTP_INTERNAL_ERROR, "Internal Error");
    headers = new HttpFields();
    headers->add(new HttpField("Date", new HttpTime));
    headers->add(new HttpField("Server", ({
	new HttpProduct(SERVER_NAME, SERVER_VERSION),
	new HttpProduct(explode(status(ST_VERSION), " ")...)
    })));
    headers->add(new HttpField("Connection", ({ "close" })));

    str = htmlInternalError();
    headers->add(new HttpField("Content-Type", "text/html;charset=utf-8"));
    headers->add(new HttpField("Content-Length", strlen(str)));
    response->setHeaders(headers);

    message = new StringBuffer(response->transport());
    message->append(str);
    sendMessage(message);
}

/*
 * send internal error response
 */
static void sendBadRequest()
{
    HttpResponse response;
    HttpFields headers;
    string str;
    StringBuffer message;

    response = new HttpResponse(1.1, HTTP_BAD_REQUEST, "Bad Request");
    headers = new HttpFields();
    headers->add(new HttpField("Date", new HttpTime));
    headers->add(new HttpField("Server", ({
	new HttpProduct(SERVER_NAME, SERVER_VERSION),
	new HttpProduct(explode(status(ST_VERSION), " ")...)
    })));
    headers->add(new HttpField("Connection", ({ "close" })));

    str = htmlBadRequest();
    headers->add(new HttpField("Content-Type", "text/html;charset=utf-8"));
    headers->add(new HttpField("Content-Length", strlen(str)));
    response->setHeaders(headers);

    message = new StringBuffer(response->transport());
    message->append(str);
    sendMessage(message);
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
    setMode(MODE_BLOCK);
    try {
	code = server->receiveRequest(code, request);
    } catch (...) {
	code = HTTP_INTERNAL_ERROR;
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
	sendBadRequest();
    }

    switch (receiveRequest(code, request)) {
    case HTTP_INTERNAL_ERROR:
	sendInternalError();
	/* fall through */
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
	setMode((persistent()) ? MODE_LINE : MODE_DISCONNECT);
    }
}

/*
 * process login message
 */
static int receiveFirstLine(string str)
{
    int code;

    try {
	code = receiveRequestLine(str);
    } catch (...) {
	code = HTTP_BAD_REQUEST;
	sendMessage(new StringBuffer(htmlBadRequest()));
    }

    if (code != 0 || request->version() < 1.0) {
	/*
	 * call receiveRequest() early, on error output a raw HTML message
	 * and disconnect immediately
	 */
	switch (receiveRequest(code, request)) {
	case HTTP_INTERNAL_ERROR:
	    sendMessage(new StringBuffer(htmlInternalError()));
	    /* fall through */
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
 * receive a request
 */
static int receiveMessage(string str)
{
    int code;

    if (strlen(str) == 0) {
	return MODE_NOCHANGE;
    }

    try {
	code = receiveRequestLine(str);
	if (request->version() < 1.0) {
	    error("Invalid request version");
	}
    } catch (...) {
	code = HTTP_BAD_REQUEST;
	sendBadRequest();
    }

    if (code != 0) {
	switch (receiveRequest(code, request)) {
	case HTTP_INTERNAL_ERROR:
	    sendInternalError();
	    /* fall through */
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
	sendMessage(new StringBuffer(response->transport()), FALSE,
		    response->headerValue("Transfer-Encoding") ||
		    response->headerValue("Content-Length"));
    }
}
