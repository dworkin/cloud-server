# include <kernel/user.h>
# include <String.h>
# include "HttpRequest.h"
# include "HttpResponse.h"

inherit "~/lib/Connection1";


/* implemented at the top layer */
static void connect(string host, int port);

private object client;		/* assocated client object */
private string responsePath;	/* HttpResponse object path */
private string headersPath;	/* HttpFields object path */
private HttpResponse response;	/* HTTP response */

/*
 * initialize connection object
 */
static void create(object client, string host, int port, string responsePath,
		   string headersPath)
{
    ::client = client;
    ::responsePath = responsePath;
    ::headersPath = headersPath;
    ::create(client, headersPath);
    connect(host, port);
}

/*
 * default client inactivity timeout
 */
static int inactivityTimeout()
{
    return 120;
}

/*
 * connection established (client)
 */
static void connected()
{
    client->connected();
}

/*
 * connection failed (client)
 */
static void connectFailed(int errorcode)
{
    client->connectFailed(errorcode);
}

/*
 * receive response headers
 */
static void receiveHeaders(string str)
{
    try {
	receiveResponseHeaders(response, new_object(headersPath, str));
    } catch (...) {
	disconnect();
    }
    receiveResponse(response);
}

/*
 * finished handling a response
 */
static void _doneResponse(object prev)
{
    if (prev == client) {
	if (persistent()) {
	    idle();
	} else {
	    disconnect();
	}
    }
}

/*
 * flow: finished handling a response
 */
void doneResponse()
{
    call_out("_doneResponse", 0, previous_object());
}

/*
 * send a request
 */
static void _sendRequest(HttpRequest request, object prev)
{
    if (prev == client) {
	::sendRequest(request);
	sendMessage(new StringBuffer(request->transport()), FALSE,
		    request->headerValue("Transfer-Encoding") ||
		    request->headerValue("Content-Length"));
    }
}

/*
 * flow: send a request
 */
void sendRequest(HttpRequest request)
{
    call_out("_sendRequest", 0, request, previous_object());
}

/*
 * receive a response
 */
static int receiveMessage(string str)
{
    try {
	response = new_object(responsePath, str);
        receiveStatusLine(response);
    } catch (...) {
        return MODE_DISCONNECT;
    }

    startHeaders();
    return MODE_LINE;
}
