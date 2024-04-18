# include <kernel/user.h>
# include <String.h>
# include "HttpRequest.h"
# include "HttpResponse.h"

inherit "~/lib/Connection1";


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
static int receiveHeaders(string str)
{
    try {
	receiveResponseHeaders(response, new_object(headersPath, str));
    } catch (...) {
	return MODE_DISCONNECT;
    }
    client->receiveResponse(response);
    return MODE_NOCHANGE;
}

/*
 * finished handling a response
 */
void doneResponse()
{
    if (previous_object() == client) {
	if (!persistent()) {
	    set_mode(MODE_DISCONNECT);
	}
    }
}

/*
 * send a request
 */
void sendRequest(HttpRequest request)
{
    if (previous_object() == client) {
	::sendRequest(request);
	sendMessage(new StringBuffer(request->transport()), FALSE,
		    request->headerValue("Transfer-Encoding") ||
		    request->headerValue("Content-Length"));
    }
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
