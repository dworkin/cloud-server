# include <kernel/user.h>
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
	flow();
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
int flow_receive_message(string str, int mode)
{
    if (previous_program() == LIB_CONN) {
	call_out("tlsReceive", 0, str);
    }
    return TRUE;
}

/*
 * connection terminated
 */
static void _logout(int quit)
{
    tlsClose(quit);
    destruct_object(this_object());
}

/*
 * connection terminated
 */
void flow_logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	call_out("_logout", 0, quit);
    }
}

/*
 * output remainder of message
 */
void flow_message_done()
{
    if (previous_program() == LIB_CONN) {
	call_out("messageDone", 0);
    }
}
