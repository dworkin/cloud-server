# include <kernel/user.h>
# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include "HttpConnection.h"

inherit Http1Client;
inherit "~System/lib/user";


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
	setMode(MODE_BLOCK);
	flow();
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
int flow_receive_message(string str, int mode)
{
    if (previous_program() == LIB_CONN) {
	call_out("receiveBytes", 0, str);
    }
    return TRUE;
}

/*
 * connection terminated
 */
static void _logout(int quit)
{
    call_limited("close", quit);
    destruct_object(this_object());
}

/*
 * connection terminated
 */
void flow_logout(int quit)
{
    if (previous_program() == LIB_CONN) {
	call_out("_logout", 0, quit);
    }
}

/*
 * output remainder of message
 */
void flow_message_done()
{
    if (previous_program() == LIB_CONN) {
	call_limited("messageDone");
    }
}
