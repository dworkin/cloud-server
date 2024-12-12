# include <kernel/user.h>
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpConnection.h"

inherit Http1Server;
inherit "~System/lib/user";


int received;	/* received at least one request */

/*
 * initialize connection object
 */
static void create(object server, varargs string requestPath, string fieldsPath)
{
    if (!requestPath) {
	requestPath = OBJECT_PATH(RemoteHttpRequest);
    }
    if (!fieldsPath) {
	fieldsPath = OBJECT_PATH(RemoteHttpFields);
    }
    ::create(server, requestPath, fieldsPath);
}

/*
 * receive a request
 */
static void receiveRequest(int code, HttpRequest request)
{
    string host;

    received = TRUE;
    ::receiveRequest(code, request);

    if (request) {
	host = request->host();
	if (!host) {
	    host = "";
	}
	::login("HTTP from " + address() + ", " +
		((code != 0) ? code + " " : "") + request->method() + " " +
		host + request->path() + "\n");
    } else {
	::login("HTTP from " + address() +
		((code != 0) ? ", " + code : "") + "\n");
    }
}

/*
 * accept HTTP connection
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	flow();
	return call_limited("receiveFirstLine", str);
    }
}

/*
 * receive a message
 */
int flow_receive_message(string str, int mode)
{
    if (previous_program() == LIB_CONN) {
	call_out("receiveBytes", 0, str);
    }
    return TRUE;
}

/*
 * terminate connection
 */
static void _logout(int quit)
{
    call_limited("close", quit);
    destruct_object(this_object());
}

/*
 * terminate connection
 */
void flow_logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	call_out("_logout", 0, quit);
    }
}

/*
 * send remainder of message
 */
void flow_message_done()
{
    if (previous_program() == LIB_CONN) {
	call_out("messageDone", 0);
    }
}

/*
 * time out if no request was received in time
 */
int timeout()
{
    if (previous_program() == LIB_CONN) {
	return !received;
    }
}
