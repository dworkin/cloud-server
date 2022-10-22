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
    HttpProduct lib, driver;
    HttpAuthentication authentication;
    Time time;

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
	return nil;
    }


    response = new HttpResponse(1.0, code, message);
    headers = new HttpFields();
    headers->add(new HttpField("Date", new HttpTime()));
    lib = new HttpProduct(SERVER_NAME, SERVER_VERSION);
    driver = new HttpProduct(explode(status(ST_VERSION), " ")...);
    headers->add(new HttpField("Server", ({ lib, driver })));

    switch (code) {
    case HTTP_MOVED_PERMANENTLY:
    case HTTP_MOVED_TEMPORARILY:
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
