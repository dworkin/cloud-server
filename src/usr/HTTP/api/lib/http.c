# include <kernel/kernel.h>
# include <String.h>
# include "http.h"

inherit "~/lib/user";
inherit "~/lib/protocol";


int response_code;

static int query_code(void) { return response_code; }

static object response(int code, string str, varargs string type, int length,
		       int modtime, string body)
{
    response_code = code;
    if (!body) {
	body = "";
    }
    return new StringBuffer(http_response(code, str, type, length, modtime) +
			    body);
}

static object bad_request(void)
{
    string str;

    str = "<HTML><HEAD><TITLE>Bad Request</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_BAD_REQUEST + " Bad Request</H1>" +
	  "This server could not understand your query." +
	  "</BODY></HTML>";
    return response(HTTP_BAD_REQUEST, nil, "text/html", strlen(str), 0, str);
}

static object not_implemented(void)
{
    string str;

    str = "<HTML><HEAD><TITLE>Not Implemented</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_NOT_IMPLEMENTED + " Not Implemented</H1>" +
	  "Unimplemented server function." +
	  "</BODY></HTML>";
    return response(HTTP_NOT_IMPLEMENTED, nil, "text/html", strlen(str), 0,
		    str);
}

static object not_found(void)
{
    string str;

    str = "<HTML><HEAD><TITLE>Not Found</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_NOT_FOUND + " Not Found</H1>" +
	  "This server could not find the path you specified." +
	  "</BODY></HTML>";
    return response(HTTP_NOT_FOUND, nil, "text/html", strlen(str), 0, str);
}

static object unauthorized(string realm)
{
    string str;

    str = "<HTML><HEAD><TITLE>Unauthorized</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_UNAUTHORIZED + " Unauthorized</H1>" +
	  "You must supply a user name and password to use this method." +
	  "</BODY></HTML>";
    return response(HTTP_UNAUTHORIZED, realm, "text/html", strlen(str), 0, str);
}
