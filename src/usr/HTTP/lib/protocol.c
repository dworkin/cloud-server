# include <config.h>
# include <version.h>
# include <status.h>
# include <type.h>
# include <Time.h>
# include "http.h"
# include "HttpHeader.h"

private inherit		"/lib/util/ascii";
private inherit base64	"/lib/util/base64";

/*
 * HTTP protocol class
 */


# define HTTP_REQUESTLINE	"/usr/HTTP/sys/requestline"

private object httpreq;		/* http request parser */

private float version;		/* request HTTP version */
private string method;		/* request method */
private string scheme;		/* RequestURI scheme */
private string host;		/* RequestURI host */
private string path;		/* RequestURI path */

private string authorization;	/* request authorization */
private string from;		/* requestor */
private int if_modified_since;	/* if-modified-since time */
private int no_cache;		/* no-cache flag */
private string referer;		/* referring page */
private string user_agent;	/* user agent description */
private int content_length;	/* entity length */
private string content_type;	/* entity type */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize HTTP protocol object
 */
static void create()
{
    httpreq = find_object(HTTP_REQUESTLINE);
}

/*
 * NAME:	request()
 * DESCRIPTION:	parse and/or interpret HTTP request
 */
static int http_request(mixed request)
{
    version = 0.0;
    method = nil;
    scheme = nil;
    host = nil;
    path = nil;

    authorization = nil;
    from = nil;
    if_modified_since = 0;
    no_cache = FALSE;
    user_agent = nil;
    content_length = -1;
    content_type = nil;

    if (typeof(request) == T_STRING) {
	request = httpreq->request(request);
    }
    if (!request) {
	/* cannot parse request */
	return HTTP_BAD_REQUEST;
    }

    version = request[HTTPREQ_VERSION];
    method = request[HTTPREQ_METHOD];
    scheme = request[HTTPREQ_SCHEME];
    host = request[HTTPREQ_HOST];
    path = request[HTTPREQ_PATH];

    if (version >= 2.0) {
	/* don't understand this */
	return HTTP_NOT_IMPLEMENTED;
    }

    switch (method) {
    case "GET":
    case "HEAD":
    case "POST":
	break;

    default:
	/* method not implemented */
	return HTTP_NOT_IMPLEMENTED;
    }

    if (!path) {
	/* invalid request */
	return HTTP_BAD_REQUEST;
    }

    return 0;
}

static float	query_version()			{ return version; }
static string	query_method()			{ return method; }
static string	query_scheme()			{ return scheme; }
static string	query_host()			{ return host; }
static string	query_path()			{ return path; }


/*
 * NAME:	time2date()
 * DESCRIPTION:	convert POSIX time to date string
 */
private string time2date(int time)
{
    return new HttpTime(new Time(time))->transport();
}

/*
 * NAME:	headers()
 * DESCRIPTION:	parse HTTP/1.x headers
 */
static int http_headers(string str)
{
    try {
	HttpHeaders headers;
	HttpHeader header;

	headers = new RemoteHttpHeaders(str);
	header = headers->get("authorization");
	if (header && lower_case(header->value()->scheme()) == "basic")
	    authorization = header->value()->authentication();
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

    if (method == "POST" && content_length < 0) {
	return HTTP_BAD_REQUEST;
    }
    return 0;
}

static string	query_authorization()		{ return authorization; }
static string	query_from()			{ return from; }
static int	query_if_modified_since()	{ return if_modified_since; }
static int	query_no_cache()		{ return no_cache; }
static string	query_referer()			{ return referer; }
static string	query_user_agent()		{ return user_agent; }
static int	query_content_length()		{ return content_length; }
static string	query_content_type()		{ return content_type; }


/*
 * NAME:	http_response()
 * DESCRIPTION:	construct a response to a request
 */
static string
http_response(int code, string str,
	      varargs string type, int length, int modtime, int expires)
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
	      "\r\nServer: " +
	      implode(explode(status(ST_VERSION), " "), "/") + " " +
	      SERVER_NAME + "/" + SERVER_VERSION + "\r\n";

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
