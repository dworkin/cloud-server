# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpConnection.h"

inherit server Http1Server;
inherit buffered "~/lib/BufferedConnection1";


TlsServerSession session;	/* TLS session */
int connected;			/* TLS connection established */


/*
 * initialize connection object
 */
static void create(object server, string certificate, string key,
		   varargs string requestPath, string fieldsPath,
		   string tlsServerSessionPath)
{
    if (!requestPath) {
	requestPath = OBJECT_PATH(RemoteHttpRequest);
    }
    if (!fieldsPath) {
	fieldsPath = OBJECT_PATH(RemoteHttpFields);
    }
    if (!tlsServerSessionPath) {
	tlsServerSessionPath = OBJECT_PATH(TlsServerSession);
    }
    server::create(server, requestPath, fieldsPath);
    buffered::create(server, fieldsPath);
    session = new_object(tlsServerSessionPath, certificate, key);
}

/*
 * receive a request
 */
static int receiveRequest(int code, HttpRequest request)
{
    string host;

    code = ::receiveRequest(code, request);

    if (request) {
	host = request->host();
	if (!host) {
	    host = "";
	}
	::login("HTTPS from " + address() + ", " + code + " " +
		request->method() + " " + host + request->path() + "\n");
    } else {
	::login("HTTPS from " + address() + ", " + code + "\n");
    }

    return code;
}

/*
 * accept HTTPS connection
 */
int login(string str)
{
    StringBuffer input, output;
    string warning, status;

    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	session->accept(FALSE);
	({ input, output, warning, status }) = session->receiveMessage(str);
	if (output) {
	    ::sendMessage(output, TRUE);
	}
	if (!status) {
	    connected = TRUE;
	    set_mode(MODE_LINE);
	}
	if (input) {
	    ::receive_message(input);
	}
	return (status && status != "connecting") ?
		MODE_DISCONNECT : MODE_NOCHANGE;
    }
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
int receive_message(string str)
{
    StringBuffer input, output;
    string warning, status;

    if (previous_program() == LIB_CONN) {
	({ input, output, warning, status }) = session->receiveMessage(str);
	if (output) {
	    ::sendMessage(output, TRUE);
	}
	if (!status && !connected) {
	    connected = TRUE;
	    set_mode(MODE_LINE);
	}
	if (input) {
	    ::receive_message(input);
	}
	return (status && status != "connecting") ?
		MODE_DISCONNECT : MODE_NOCHANGE;
    }
}

/*
 * terminate connection
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
 * send remainder of message
 */
int message_done()
{
    if (previous_program() == LIB_CONN) {
	return ::message_done();
    }
}
