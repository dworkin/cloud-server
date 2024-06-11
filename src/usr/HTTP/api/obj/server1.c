# include <kernel/user.h>
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpConnection.h"

inherit Http1Server;
inherit "~System/lib/user";


private int received;	/* received at least one request */

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
static int receiveRequest(int code, HttpRequest request)
{
    string host;

    received = TRUE;
    code = ::receiveRequest(code, request);

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

    return code;
}

/*
 * accept HTTP connection
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	return call_limited("receiveFirstLine", str);
    }
}

/*
 * receive a message
 */
int receive_message(string str)
{
    if (previous_program() == LIB_CONN) {
	return call_limited("receiveBytes", str);
    }
}

/*
 * terminate connection
 */
void logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	call_limited("close", quit);
	destruct_object(this_object());
    }
}

/*
 * send remainder of message
 */
int message_done()
{
    if (previous_program() == LIB_CONN) {
	return call_limited("messageDone");
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
