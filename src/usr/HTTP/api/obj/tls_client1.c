# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include "HttpConnection.h"

inherit client Http1Client;
inherit buffered "~/lib/BufferedConnection1";


string host;			/* remote host */
TlsClientSession session;	/* TLS session */

/*
 * initialize connection object
 */
static void create(object client, string host, int port,
		   varargs string responsePath, string fieldsPath,
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
    client::create(client, host, port, responsePath, fieldsPath);
    buffered::create(client, fieldsPath);
    session = new_object(tlsClientSessionPath);
}

/*
 * connection established
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	::sendMessage(session->connect(TRUE, host), TRUE);
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
    StringBuffer input, output;
    string warning, status;

    if (previous_program() == LIB_CONN) {
	({ input, output, warning, status }) = session->receiveMessage(str);
	if (output) {
	    ::sendMessage(output, TRUE);
	}
	if (!status && host) {
	    host = nil;
	    set_mode(MODE_BLOCK);
	    call_limited("connected");
	}
	if (input) {
	    ::receive_message(input);
	}
	return (status && status != "connecting") ?
		MODE_DISCONNECT : MODE_NOCHANGE;
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
	output = session->close();
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
