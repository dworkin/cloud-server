# include <kernel/kernel.h>
# include <config.h>
# include <version.h>
# include <status.h>
# include <type.h>
# include <String.h>
# include <Time.h>
# include "~HTTP/HttpRequest.h"
# include "~HTTP/HttpField.h"
# include "~HTTP/HttpResponse.h"

private inherit	"/lib/util/ascii";


private float version;		/* request version */

/*
 * process HTTP request
 */
static int http_request(HttpRequest request)
{
    int content_length;
    mixed value;

    content_length = -1;
    version = request->version();

    try {
	value = request->headerValue("Content-Length");
	if (value != nil) {
	    content_length = value;
	}
    } catch (...) {
	return HTTP_BAD_REQUEST;
    }

    if (request->method() == "POST" && content_length < 0) {
	return HTTP_BAD_REQUEST;
    }
    return 0;
}

/*
 * construct a response to a request
 */
private HttpResponse http_response(int code, string str, varargs string type,
				   int length, int modtime, int expires)
{
    string message;
    HttpResponse response;
    HttpFields headers;
    HttpAuthentication authentication;
    Time time;

    switch (code) {
    case HTTP_OK:
	message = "OK";
	break;

    case HTTP_CREATED:
	message = "Created";
	break;

    case HTTP_ACCEPTED:
	message = "Accepted";
	break;

    case HTTP_NO_CONTENT:
	message = "No Content";
	break;

    case HTTP_MULTIPLE_CHOICES:
	message = "Multiple Choices";
	break;

    case HTTP_MOVED_PERMANENTLY:
	message = "Moved Permanently";
	break;

    case HTTP_FOUND:
	message = "Moved Temporarily";
	break;

    case HTTP_NOT_MODIFIED:
	message = "Not Modified";
	break;

    case HTTP_BAD_REQUEST:
	message = "Bad Request";
	break;

    case HTTP_UNAUTHORIZED:
	message = "Unauthorized";
	break;

    case HTTP_FORBIDDEN:
	message = "Forbidden";
	break;

    case HTTP_NOT_FOUND:
	message = "Not Found";
	break;

    case HTTP_INTERNAL_ERROR:
	message = "Internal Server Error";
	break;

    case HTTP_NOT_IMPLEMENTED:
	message = "Not Implemented";
	break;

    case HTTP_BAD_GATEWAY:
	message = "Bad Gateway";
	break;

    case HTTP_SERVICE_UNAVAILABLE:
	message = "Service Unavailable";
	break;

    default:
	error("Bad response code");
    }

    if (version < 1.0) {
	return nil;
    }


    response = new HttpResponse(1.0, code, message);
    headers = new HttpFields();
    headers->add(new HttpField("Date", new HttpTime()));
    headers->add(new HttpField("Server", ({
	new HttpProduct(SERVER_NAME, SERVER_VERSION),
	new HttpProduct(explode(status(ST_VERSION), " ")...)
    })));

    switch (code) {
    case HTTP_MOVED_PERMANENTLY:
    case HTTP_FOUND:
	headers->add(new HttpField("Location", str));
	break;

    case HTTP_UNAUTHORIZED:
	authentication = new HttpAuthentication("Basic",
						({ "realm=\"" + str + "\"" }));
	headers->add(new HttpField("WWW-Authenticate", authentication));
	break;
    }

    if (type) {
	headers->add(new HttpField("Content-Type", type));
	if (length >= 0) {
	    headers->add(new HttpField("Content-Length", length));
	}
	if (modtime != 0) {
	    time = new Time(modtime);
	    headers->add(new HttpField("Last-Modified", new HttpTime(time)));
	}
	if (expires != 0) {
	    time = new Time(expires);
	    headers->add(new HttpField("Expires", new HttpTime(time)));
	}
    }

    response->setHeaders(headers);
    return response;
}


static mixed *response(int code, string str, varargs string type, int length,
		       int modtime, string body)
{
    HttpResponse response;

    if (!body) {
	body = "";
    }
    response = http_response(code, str, type, length, modtime);
    if (response) {
	body = response->transport() + body;
    }
    return ({ code, new StringBuffer(body) });
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
