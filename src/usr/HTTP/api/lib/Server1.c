# include <kernel/user.h>
# include <String.h>
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"

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
    set_mode(MODE_BLOCK);
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
 * process login message
 */
static int receiveFirstLine(string str)
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
