# include <kernel/user.h>
# include <String.h>
# include "~TLS/tls.h"
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpConnection.h"

inherit Http1TlsServer;


int reqCert;		/* request client certificate */
string *hosts;		/* server hostnames */

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
    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	return tlsAccept(str, reqCert, ((hosts) ? hosts : ({ }))...);
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
 * terminate connection
 */
void logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	::logout(quit);
	destruct_object(this_object());
    }
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
