# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpConnection.h"

inherit client Http1Client;
inherit buffered "~/lib/BufferedConnection1";


private TlsClientSession session;	/* TLS session */
private int connected;			/* TLS connection established */

/*
 * initialize connection object
 */
static void create(object client, string address, int port, string responsePath,
		   string fieldsPath, string tlsClientSessionPath)
{
    client::create(client, address, port, responsePath, fieldsPath);
    buffered::create(client, fieldsPath);
    session = new_object(tlsClientSessionPath);
}

/*
 * establish connection
 */
static void tlsConnect(varargs string host)
{
    ::sendMessage(session->connect(TRUE, host), TRUE);
}

/*
 * receive a message
 */
static int receive_message(string str)
{
    StringBuffer input, output;
    string warning, status;

    ({ input, output, warning, status }) = session->receiveMessage(str);
    if (output) {
	::sendMessage(output, TRUE);
    }
    if (!status && !connected) {
	connected = TRUE;
	set_mode(MODE_BLOCK);
	call_limited("connected");
    }
    if (input) {
	::receive_message(input);
    }
    return (status && status != "connecting") ? MODE_DISCONNECT : MODE_NOCHANGE;
}

/*
 * connection terminated
 */
static void logout(int quit)
{
    StringBuffer output;

    ::logout(quit);
    output = session->close();
    if (output) {
	::sendMessage(output, TRUE);
    }
}

/*
 * send an encrypted message
 */
static void sendMessage(StringBuffer str)
{
    ::sendMessage(session->sendMessage(str));
}
