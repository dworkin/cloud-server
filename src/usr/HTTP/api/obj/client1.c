# include <kernel/user.h>
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include "HttpConnection.h"

inherit Http1Client;


/*
 * initialize connection object
 */
static void create(object client, string host, int port,
		   varargs string responsePath, string fieldsPath)
{
    if (!responsePath) {
	responsePath = OBJECT_PATH(RemoteHttpResponse);
    }
    if (!fieldsPath) {
	fieldsPath = OBJECT_PATH(RemoteHttpFields);
    }
    ::create(client, host, port, responsePath, fieldsPath);
    connect(host, port);
}

/*
 * connection established
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	set_mode(MODE_BLOCK);
	call_limited("connected");
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
	return call_limited("receiveBytes", str);
    }
}

/*
 * connection terminated
 */
void logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	call_limited("close", quit);
	destruct_object(this_object());
    }
}

/*
 * output remainder of message
 */
int message_done()
{
    if (previous_program() == LIB_CONN) {
	return call_limited("messageDone");
    }
}
