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
    buffered::create(MODE_LINE);
    session = new_object(tlsServerSessionPath, certificate, key);
}

/*
 * accept HTTPS connection
 */
static void tlsAccept(string str, varargs int reqCert, string hosts...)
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
    }
    if (input) {
	receiveBytes(input);
    }

    if (status && status != "connecting") {
	disconnect();
    }
}

/*
 * process login message
 */
static void receiveFirstMessage(string str)
{
    setMode(::receiveFirstLine(str));
}

/*
 * receive a message
 */
static void tlsReceive(string str)
{
    StringBuffer input, output;
    string warning, status;

    ({ input, output, warning, status }) = session->receiveMessage(str);
    if (output) {
	::sendMessage(output, TRUE);
    }
    if (!status && !connected) {
	connected = TRUE;
    }
    if (input) {
	receiveBytes(input);
    } else {
	startInput();
    }

    if (status && status != "connecting") {
	disconnect();
    }
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
