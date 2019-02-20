# include <config.h>
# include <version.h>
# include <status.h>
# include <type.h>
# include <Time.h>
# include "http.h"

private inherit "/lib/util/string";

/*
 * HTTP protocol class
 */


private object httpreq;		/* http request parser */
private object httphdr;		/* http header parser */

private float version;		/* request HTTP version */
private string method;		/* request method */
private string scheme;		/* RequestURI scheme */
private string host;		/* RequestURI host */
private string path;		/* RequestURI path */
private string *parameters;	/* request parameters */

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
static void create(void)
{
    httpreq = find_object(HTTPRequest);
    httphdr = find_object(HTTPHeader);
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
    parameters = nil;

    authorization = nil;
    from = nil;
    if_modified_since = 0;
    no_cache = FALSE;
    user_agent = nil;
    content_length = -1;
    content_type = nil;

    if (typeof(request) == T_STRING) {
	request = httpreq->parse_request(request);
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
    parameters = request[HTTPREQ_PARAMS];

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
static string  *query_parameters()		{ return parameters[..]; }


# define BASE64	("...........................................\x3e..." +   \
		 "\x3f\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d...=..." +  \
		 "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c" + \
		 "\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19" + \
		 "......" +						  \
		 "\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26" + \
		 "\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33" + \
		 "...................................................." + \
		 "...................................................." + \
		 ".............................")

/*
 * NAME:	base64_decode()
 * DESCRIPTION:	decode a base64 string
 */
private string base64_decode(string str)
{
    string result, bits;
    int i, len, b1, b2, b3, b4;

    result = "";
    bits = "...";
    for (i = 0, len = strlen(str); i < len; i += 4) {
	b1 = BASE64[str[i]];
	b2 = BASE64[str[i + 1]];
	b3 = BASE64[str[i + 2]];
	b4 = BASE64[str[i + 3]];
	bits[0] = (b1 << 2) | (b2 >> 4);
	bits[1] = (b2 << 4) | (b3 >> 2);
	bits[2] = (b3 << 6) | b4;
	result += bits;
    }

    if (b3 == '=') {
	return result[.. strlen(result) - 3];
    } else if (b4 == '=') {
	return result[.. strlen(result) - 2];
    }
    return result;
}

/*
 * NAME:	time2date()
 * DESCRIPTION:	convert POSIX time to date string
 */
private string time2date(int time)
{
    string str, weekday, month;
    int day, year;

    str = new Time(time)->gmctime();
    sscanf(str, "%s %s %d %s %d", weekday, month, day, str, year);
    return weekday + ", " + ((day >= 10) ? (string) day : "0" + day) + " " +
	   month + " " + year + " " + str + " GMT";
}

/*
 * NAME:	date2time()
 * DESCRIPTION:	convert a date to POSIX time
 */
private int date2time(mixed *date)
{
    string str, month;
    int i, year, day, size;

    if (date[0] == HTTPHDR_ITEMS) {
	/* asctime-date */
	if (sizeof(date) != 6) {
	    return 0;
	}
	str = "";
	for (i = 1; i < 6; i++) {
	    if (date[i][0] != HTTPHDR_TOKEN) {
		return 0;
	    }
	    str += date[i][1] + " ";
	}
	catch {
	    return new GMTime(str)->time();
	} : {
	    return 0;
	}
    }

    if (date[0] != HTTPHDR_LIST || sizeof(date) != 3 ||
	date[1][0] != HTTPHDR_TOKEN) {
	return 0;
    }
    size = (strlen(date[1][1]) == 3) ? 6 : 4;
    date = date[2];
    if (date[0] != HTTPHDR_ITEMS || sizeof(date) != size) {
	return 0;
    }
    str = "";
    for (i = 1; i < size; i++) {
	if (date[i][0] != HTTPHDR_TOKEN) {
	    return 0;
	}
	str += date[i][1] + " ";
    }

    if (size == 6) {
	/* rfc1123-date */
	if (sscanf(str, "%d %s %d %s GMT", day, month, year, str) != 4) {
	    return 0;
	}
    } else {
	/* rfc850-date */
	if (sscanf(str, "%d-%s-%d %s GMT", day, month, year, str) != 4) {
	    return 0;
	}
	year += (year < 70) ? 2000 : 1900;	/* hopefully obsolete in 2070 */
    }

    catch {
	return new GMTime("Someday " + month + " " + day + " " + str + " " +
			  year)->time();
    } : {
	return 0;
    }
}

/*
 * NAME:	headers()
 * DESCRIPTION:	parse HTTP/1.x headers
 */
static int http_headers(string str)
{
    mixed **headers, *header, *value;
    int i, j, sz;

    headers = httphdr->parse_header(str);
    if (!headers) {
	return HTTP_BAD_REQUEST;
    }

    for (i = 0, sz = sizeof(headers); i < sz; i++) {
	header = headers[i];
	value = (sizeof(header) > 1) ? header[1] : nil;
	switch (lower_case(header[0])) {
	case "authorization":
	    if (value && value[0] == HTTPHDR_ITEMS && sizeof(value) == 3 &&
		value[1][0] == HTTPHDR_TOKEN && value[2][0] == HTTPHDR_TOKEN &&
		lower_case(value[1][1]) == "basic") {
		str = value[2][1];
		if ((strlen(str) & 3) == 0) {
		    authorization = base64_decode(str);
		}
	    }
	    break;

	case "content-length":
	    if (value && value[0] == HTTPHDR_TOKEN) {
		sscanf(value[1], "%d", content_length);
		if (content_length < 0) {
		    return HTTP_BAD_REQUEST;
		}
	    }
	    break;

	case "content-type":
	    if (value && value[0] == HTTPHDR_TYPE) {
		content_type = lower_case(value[1] + "/" + value[2]);
	    }
	    break;

	case "from":
	    if (value && value[0] == HTTPHDR_TOKEN) {
		from = value[1];
	    }
	    break;

	case "host":
	    if (value && value[0] == HTTPHDR_TOKEN && !host) {
		host = value[1];
	    }
	    break;

	case "if-modified-since":
	    if (value) {
		if_modified_since = date2time(value);
	    }
	    break;

	case "pragma":
	    if (value) {
		if (value[0] == HTTPHDR_TOKEN) {
		    if (lower_case(value[1]) == "no-cache") {
			no_cache = TRUE;
		    }
		} else if (value[0] == HTTPHDR_ITEMS) {
		    for (j = sizeof(value); --j != 0; ) {
			if (value[j][0] == HTTPHDR_TOKEN &&
			    lower_case(value[j][1]) == "no-cache") {
			    no_cache = TRUE;
			    break;
			}
		    }
		}
	    }
	    break;

	case "referer":
	    if (value && value[0] == HTTPHDR_TOKEN) {
		referer = value[1];
	    }
	    break;

	case "user-agent":
	    if (value) {
		if (value[0] == HTTPHDR_TYPE) {
		    user_agent = value[1] + "/" + value[2];
		} else if (value[0] == HTTPHDR_ITEMS) {
		    str = "";
		    for (j = 1; j < sizeof(value); j++) {
			if (value[j][0] == HTTPHDR_TYPE) {
			    str += " " + value[j][1] + "/" + value[j][2];
			}
		    }
		    if (strlen(str) != 0) {
			user_agent = str[1 ..];
		    }
		}
	    }
	    break;
	}
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
