# include <kernel/kernel.h>
# include <config.h>
# include <version.h>
# include <status.h>
# include <type.h>
# include <String.h>
# include <Time.h>
# include "~HTTP/api/include/HttpRequest.h"
# include "~HTTP/api/include/HttpField.h"
# include "~HTTP/api/include/HttpResponse.h"

private inherit	"/lib/util/ascii";
private inherit base64 "/lib/util/base64";


private float version;		/* request version */
private string host;		/* request host */
private string authorization;	/* request authorization */
private string from;		/* requestor */
private int if_modified_since;	/* if-modified-since time */
private int no_cache;		/* no-cache flag */
private string referer;		/* referring page */
private string user_agent;	/* user agent description */
private int content_length;	/* entity length */
private string content_type;	/* entity type */

/*
 * process HTTP request
 */
static int http_request(HttpRequest request)
{
    HttpFields headers;
    HttpField header;

    authorization = nil;
    from = nil;
    host = nil;
    if_modified_since = 0;
    no_cache = FALSE;
    user_agent = nil;
    content_length = -1;
    content_type = nil;

    version = request->version();
    host = request->host();

    headers = request->headers();
    if (headers) try {
	headers = request->headers();
	header = headers->get("authorization");
	if (header && lower_case(header->value()->scheme()) == "basic")
	    authorization = base64::decode(header->value()->authentication());
	header = headers->get("content-length");
	if (header) content_length = header->value();
	header = headers->get("content-type");
	if (header) content_type = header->value();
	header = headers->get("from");
	if (header) from = header->value();
	header = headers->get("host");
	if (header) host = header->value();
	header = headers->get("if-modified-since");
	if (header) if_modified_since = header->value()->time()->time();
	header = headers->get("pragma");
	if (header && typeof(header->value()) == T_STRING &&
	    lower_case(header->value()) == "no-cache")
	    no_cache = TRUE;
	header = headers->get("referer");
	if (header) referer = header->value();
	header = headers->get("user-agent");
	if (header) user_agent = header->value();
    } catch (...) {
	return HTTP_BAD_REQUEST;
    }

    if (request->method() == "POST" && content_length < 0) {
	return HTTP_BAD_REQUEST;
    }
    return 0;
}

/*
 * convert POSIX time to date string
 */
private string time2date(int time)
{
    return new HttpTime(new Time(time))->transport();
}

static string	query_authorization()		{ return authorization; }
static string	query_from()			{ return from; }
static string	query_host()			{ return host; }
static int	query_if_modified_since()	{ return if_modified_since; }
static int	query_no_cache()		{ return no_cache; }
static string	query_referer()			{ return referer; }
static string	query_user_agent()		{ return user_agent; }
static int	query_content_length()		{ return content_length; }
static string	query_content_type()		{ return content_type; }


/*
 * construct a response to a request
 */
private string http_response(int code, string str, varargs string type,
			     int length, int modtime, int expires)
{
    string message;

    switch (code) {
    case HTTP_OK:
	message = HTTP_OK + " OK";
	break;

    case HTTP_CREATED:
	message = HTTP_CREATED + " Created";
	break;

    case HTTP_ACCEPTED:
	message = HTTP_ACCEPTED + " Accepted";
	break;

    case HTTP_NO_CONTENT:
	message = HTTP_NO_CONTENT + " No Content";
	break;

    case HTTP_MULTIPLE_CHOICES:
	message = HTTP_MULTIPLE_CHOICES + " Multiple Choices";
	break;

    case HTTP_MOVED_PERMANENTLY:
	message = HTTP_MOVED_PERMANENTLY + " Moved Permanently";
	break;

    case HTTP_MOVED_TEMPORARILY:
	message = HTTP_MOVED_TEMPORARILY + " Moved Temporarily";
	break;

    case HTTP_NOT_MODIFIED:
	message = HTTP_NOT_MODIFIED + " Not Modified";
	break;

    case HTTP_BAD_REQUEST:
	message = HTTP_BAD_REQUEST + " Bad Request";
	break;

    case HTTP_UNAUTHORIZED:
	message = HTTP_UNAUTHORIZED + " Unauthorized";
	break;

    case HTTP_FORBIDDEN:
	message = HTTP_FORBIDDEN + " Forbidden";
	break;

    case HTTP_NOT_FOUND:
	message = HTTP_NOT_FOUND + " Not Found";
	break;

    case HTTP_INTERNAL_ERROR:
	message = HTTP_INTERNAL_ERROR + " Internal Server Error";
	break;

    case HTTP_NOT_IMPLEMENTED:
	message = HTTP_NOT_IMPLEMENTED + " Not Implemented";
	break;

    case HTTP_BAD_GATEWAY:
	message = HTTP_BAD_GATEWAY + " Bad Gateway";
	break;

    case HTTP_UNAVAILABLE:
	message = HTTP_UNAVAILABLE + " Service Unavailable";
	break;

    default:
	error("Bad response code");
    }

    if (version < 1.0) {
	return "";
    }


    message = "HTTP/1.0 " + message + "\r\nDate: " + time2date(time()) +
	      "\r\nServer: " + SERVER_NAME + "/" + SERVER_VERSION + " " +
	      implode(explode(status(ST_VERSION), " "), "/") + "\r\n";

    switch (code) {
    case HTTP_MOVED_PERMANENTLY:
    case HTTP_MOVED_TEMPORARILY:
	message += "Location: " + str + "\r\n";
	break;

    case HTTP_UNAUTHORIZED:
	message += "WWW-Authenticate: Basic realm=\"" + str + "\"\r\n";
	break;
    }

    if (type) {
	message += "Content-Type: " + type + "\r\n";
	if (length >= 0) {
	    message += "Content-Length: " + length + "\r\n";
	}
	if (modtime != 0) {
	    message += "Last-Modified: " + time2date(modtime) + "\r\n";
	}
	if (expires != 0) {
	    message += "Expires: " + time2date(expires) + "\r\n";
	}
    }

    return message + "\r\n";
}


static mixed *response(int code, string str, varargs string type, int length,
		       int modtime, string body)
{
    if (!body) {
	body = "";
    }
    return ({
	code,
	new StringBuffer(http_response(code, str, type, length, modtime) +
			 body)
    });
}

static mixed *bad_request()
{
    string str;

    str = "<HTML><HEAD><TITLE>Bad Request</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_BAD_REQUEST + " Bad Request</H1>" +
	  "This server could not understand your query." +
	  "</BODY></HTML>";
    return response(HTTP_BAD_REQUEST, nil, "text/html", strlen(str), 0, str);
}

static mixed *not_implemented()
{
    string str;

    str = "<HTML><HEAD><TITLE>Not Implemented</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_NOT_IMPLEMENTED + " Not Implemented</H1>" +
	  "Unimplemented server function." +
	  "</BODY></HTML>";
    return response(HTTP_NOT_IMPLEMENTED, nil, "text/html", strlen(str), 0,
		    str);
}

static mixed *not_found()
{
    string str;

    str = "<HTML><HEAD><TITLE>Not Found</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_NOT_FOUND + " Not Found</H1>" +
	  "This server could not find the path you specified." +
	  "</BODY></HTML>";
    return response(HTTP_NOT_FOUND, nil, "text/html", strlen(str), 0, str);
}

static mixed *unauthorized(string realm)
{
    string str;

    str = "<HTML><HEAD><TITLE>Unauthorized</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_UNAUTHORIZED + " Unauthorized</H1>" +
	  "You must supply a user name and password to use this method." +
	  "</BODY></HTML>";
    return response(HTTP_UNAUTHORIZED, realm, "text/html", strlen(str), 0, str);
}


int receiveRequest(int code, HttpRequest request)
{
    StringBuffer reply;

    ({ code, reply }) = this_object()->http_message(code, request, nil);
    previous_object()->sendChunk(reply);
    return code;
}

int doneChunk()
{
    previous_object()->disconnect();
}
