# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpConnection.h"

inherit server Http1Server;
inherit buffered "~/lib/BufferedConnection1";


private TlsServerSession session;	/* TLS session */
private int connected;			/* TLS connection established */
private StringBuffer outbuf;		/* output buffer */


/*
 * initialize connection object
 */
static void create(object server, string certificate, string key,
		   string requestPath, string fieldsPath,
		   string tlsServerSessionPath)
{
    server::create(server, requestPath, fieldsPath);
    buffered::create(server, fieldsPath);
    session = new_object(tlsServerSessionPath, certificate, key);
}

/*
 * accept HTTPS connection
 */
static int tlsAccept(string str, varargs int reqCert, string hosts...)
{
    StringBuffer input, output;
    string warning, status;

    session->accept(reqCert, ((hosts) ? hosts : ({ }))...);
    ({ input, output, warning, status }) = session->receiveMessage(str);
    if (output) {
	::sendMessage(output, TRUE);
    }
    if (!status) {
	connected = TRUE;
	set_mode(MODE_LINE);
    }
    if (input) {
	receiveBytes(input);
    }
    return (status && status != "connecting") ? MODE_DISCONNECT : MODE_NOCHANGE;
}

/*
 * process login message
 */
static int receiveFirstMessage(string str)
{
    return ::receiveFirstLine(str);
}

/*
 * receive a message
 */
static int tlsReceive(string str)
{
    StringBuffer input, output;
    string warning, status;

    ({ input, output, warning, status }) = session->receiveMessage(str);
    if (output) {
	::sendMessage(output, TRUE);
    }
    if (!status && !connected) {
	connected = TRUE;
	set_mode(MODE_LINE);
    }
    if (input) {
	receiveBytes(input);
    }
    return (status && status != "connecting") ? MODE_DISCONNECT : MODE_NOCHANGE;
}

/*
 * terminate connection
 */
static void tlsClose(int quit)
{
    StringBuffer output;

    close(quit);
    output = session->close();
    if (output) {
	::sendMessage(output, TRUE);
    }
}

/*
 * send an encrypted message
 */
static void sendMessage(StringBuffer str, varargs int quiet, int hold)
{
    if (outbuf) {
	outbuf->append(str);
	str = outbuf;
	outbuf = nil;
    }

    if (hold) {
	outbuf = str;
    } else {
	::sendMessage(session->sendMessage(str), quiet);
    }
}


string host()	{ return session->host(); }
