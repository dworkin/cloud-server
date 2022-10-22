# include <kernel/user.h>
# include <String.h>
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include "HttpConnection.h"

inherit Http1Connection;


object client;		/* assocated client object */
string responsePath;	/* HttpResponse object path */
string headersPath;	/* HttpFields object path */
HttpResponse response;	/* HTTP response */

/*
 * initialize connection object
 */
static void create(object client, string address, int port,
		   varargs string responsePath, string fieldsPath)
{
    ::client = client;
    ::responsePath = (responsePath) ?
		      responsePath : OBJECT_PATH(RemoteHttpResponse);
    headersPath = (fieldsPath) ?
		   fieldsPath : OBJECT_PATH(RemoteHttpFields);
    ::create(client, headersPath);
    connect(address, port);
}

/*
 * connection established (client)
 */
static void _connected()
{
    client->connected();
}

/*
 * connection established
 */
void connected()
{
    if (previous_program() == LIB_CONN) {
	call_limited("_connected");
    }
}

/*
 * connection failed (client)
 */
static void connectFailed(int errorcode)
{
    client->connectFailed(errorcode);
}

/*
 * connection failed
 */
void connect_failed(int errorcode)
{
    if (previous_program() == LIB_CONN) {
	call_limited("connectFailed", errorcode);
	destruct_object(this_object());
    }
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
 * initial connection mode
 */
int login(string str)
{
    return MODE_BLOCK;
}

/*
 * send a request
 */
void sendRequest(HttpRequest request)
{
    if (previous_object() == client) {
	::sendRequest(request);
	sendMessage(new StringBuffer(request->transport()));
    }
}

/*
 * receive a response
 */
static int receiveMessage(string str)
{
    try {
	response = new_object(responsePath, str);
        call_limited("receiveStatusLine", response);
    } catch (...) {
        return MODE_DISCONNECT;
    }

    startHeaders();
    return MODE_LINE;
}
