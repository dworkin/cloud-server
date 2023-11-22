# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include "HttpConnection.h"

inherit Http1TlsClient;


/*
 * initialize connection object
 */
static void create(object client, string host, int port,
		   varargs string responsePath, string fieldsPath,
		   string tlsClientSessionPath)
{
    if (!responsePath) {
	responsePath = OBJECT_PATH(RemoteHttpResponse);
    }
    if (!fieldsPath) {
	fieldsPath = OBJECT_PATH(RemoteHttpFields);
    }
    if (!tlsClientSessionPath) {
	tlsClientSessionPath = OBJECT_PATH(TlsClientSession);
    }
    ::create(client, host, port, responsePath, fieldsPath,
	     tlsClientSessionPath);
}

/*
 * connection established
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	tlsConnect();
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
	return ::receive_message(str);
    }
}

/*
 * connection terminated
 */
void logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	::logout(quit);
	destruct_object(this_object());
    }
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
