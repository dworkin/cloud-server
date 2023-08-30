# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"

inherit "~/lib/BufferedConnection1";


object client;			/* assocated client object */
string responsePath;		/* HttpResponse object path */
string headersPath;		/* HttpFields object path */
string host;			/* remote host */
TlsClientSession session;	/* TLS session */
HttpResponse response;		/* HTTP response */

/*
 * initialize connection object
 */
static void create(object client, string host, int port,
		   varargs string responsePath, string fieldsPath)
{
    ::client = client;
    ::responsePath = (responsePath) ?
		      responsePath : OBJECT_PATH(RemoteHttpResponse);
    headersPath = (fieldsPath) ?
		   fieldsPath : OBJECT_PATH(RemoteHttpFields);
    ::host = host;
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
 * connection established
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	session = new TlsClientSession;
	::sendMessage(session->connect(TRUE, host), TRUE);
    }
    return MODE_NOCHANGE;
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
 * receive a message
 */
int receive_message(string str)
{
    StringBuffer input, output;
    string alert;
    int mode;

    if (previous_program() == LIB_CONN) {
	({ input, output, alert }) = session->receiveMessage(str);
	if (output) {
	    ::sendMessage(output, TRUE);
	}
	if (!alert && host) {
	    host = nil;
	    set_mode(MODE_BLOCK);
	    call_limited("connected");
	}
	if (input) {
	    ::receive_message(input);
	}
	return (alert == "EOF") ? MODE_DISCONNECT : MODE_NOCHANGE;
    }
}

/*
 * connection terminated
 */
void logout(int quit)
{
    StringBuffer output;

    if (previous_program() == LIB_CONN) {
	::logout(quit);
	output = client->close();
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
 * output remainder of message
 */
int message_done()
{
    if (previous_program() == LIB_CONN) {
	return ::message_done();
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
