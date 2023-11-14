# include <kernel/user.h>
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpConnection.h"

inherit Http1Server;


/*
 * initialize connection object
 */
static void create(object server, varargs string requestPath, string fieldsPath)
{
    if (!requestPath) {
	requestPath = OBJECT_PATH(RemoteHttpRequest);
    }
    if (!fieldsPath) {
	fieldsPath = OBJECT_PATH(RemoteHttpFields);
    }
    ::create(server, requestPath, fieldsPath);
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
	::login("HTTP from " + address() + ", " + code + " " +
		request->method() + " " + host + request->path() + "\n");
    } else {
	::login("HTTP from " + address() + ", " + code + "\n");
    }

    return code;
}

/*
 * accept HTTP connection
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	::connection(previous_object());
	return receiveFirstLine(str);
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
