# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include "HttpConnection.h"

inherit Http1TlsClient;
inherit "~System/lib/user";


string host;		/* server hostname */

/*
 * initialize connection object
 */
static void create(object client, string address, int port,
		   varargs string host, string responsePath, string fieldsPath,
		   string tlsClientSessionPath)
{
    ::host = host;
    if (!responsePath) {
	responsePath = OBJECT_PATH(RemoteHttpResponse);
    }
    if (!fieldsPath) {
	fieldsPath = OBJECT_PATH(RemoteHttpFields);
    }
    if (!tlsClientSessionPath) {
	tlsClientSessionPath = OBJECT_PATH(TlsClientSession);
    }
    ::create(client, address, port, responsePath, fieldsPath,
	     tlsClientSessionPath);
}

/*
 * connection established
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	call_limited("tlsConnect", host);
    }
    return MODE_NOCHANGE;
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
    if (previous_program() == LIB_CONN) {
	return call_limited("tlsReceive", str);
    }
}

/*
 * connection terminated
 */
void logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	call_limited("tlsClose", quit);
	destruct_object(this_object());
    }
}

/*
 * output remainder of message
 */
int message_done()
{
    if (previous_program() == LIB_CONN) {
	return call_limited("messageDone");
    }
}

/*
 * reprocess pending input
 */
void restart_input()
{
    if (previous_program() == BINARY_CONN) {
	call_limited("restartInput");
    }
}
