# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpConnection.h"

inherit Http1TlsServer;
inherit "~System/lib/user";


int reqCert;		/* request client certificate */
string *hosts;		/* server hostnames */
private int received;	/* received at least one request */

/*
 * initialize connection object
 */
static void create(object server, string certificate, string key,
		   varargs int reqCert, string *hosts, string requestPath,
		   string fieldsPath, string tlsServerSessionPath)
{
    ::reqCert = reqCert;
    ::hosts = hosts;
    if (!requestPath) {
	requestPath = OBJECT_PATH(RemoteHttpRequest);
    }
    if (!fieldsPath) {
	fieldsPath = OBJECT_PATH(RemoteHttpFields);
    }
    if (!tlsServerSessionPath) {
	tlsServerSessionPath = OBJECT_PATH(TlsServerSession);
    }
    ::create(server, certificate, key, requestPath, fieldsPath,
	     tlsServerSessionPath);
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
	::login("HTTPS from " + address() + ", " +
		((code != 0) ? code + " " : "") + request->method() + " " +
		host + request->path() + "\n");
    } else {
	::login("HTTPS from " + address() +
		((code != 0) ? ", " + code : "") + "\n");
    }

    return code;
}

/*
 * accept HTTPS connection
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	return call_limited("tlsAccept", str, reqCert,
			    ((hosts) ? hosts : ({ }))...);
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
 * terminate connection
 */
void logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	call_limited("tlsClose", quit);
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
 * reprocess pending input
 */
void restart_input()
{
    if (previous_program() == BINARY_CONN) {
	call_limited("restartInput");
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
