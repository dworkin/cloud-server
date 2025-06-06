# include <kernel/user.h>
# include "~TLS/tls.h"
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpConnection.h"

inherit Http1TlsServer;
inherit "~System/lib/user";


int reqCert;	/* request client certificate */
string *hosts;	/* server hostnames */
int received;	/* received at least one request */

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
static void receiveRequest(int code, HttpRequest request)
{
    string host;

    received = TRUE;
    ::receiveRequest(code, request);

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
}

/*
 * accept HTTPS connection
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	flow();
	call_limited("tlsAccept", str, reqCert, ((hosts) ? hosts : ({ }))...);
    }
    return MODE_NOCHANGE;
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
 * terminate connection
 */
static void _logout(int quit)
{
    tlsClose(quit);
    destruct_object(this_object());
}

/*
 * terminate connection
 */
void flow_logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	call_out("_logout", 0, quit);
    }
}

/*
 * send remainder of message
 */
void flow_message_done()
{
    if (previous_program() == LIB_CONN) {
	call_out("messageDone", 0);
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
