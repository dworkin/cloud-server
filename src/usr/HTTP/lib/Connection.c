# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include <Iterator.h>

inherit "~System/lib/user";
private inherit "/lib/util/ascii";


/*
 * received HTTP request line
 */
static int receiveRequestLine(HttpRequest request)
{
    switch (request->method()) {
    case "GET":
    case "HEAD":
    case "POST":
    case "PUT":
    case "DELETE":
    case "CONNECT":
    case "OPTIONS":
    case "TRACE":
	break;

    default:
	/* method not implemented */
	return HTTP_NOT_IMPLEMENTED;
    }

    if (!request->path()) {
	/* invalid request */
	return HTTP_BAD_REQUEST;
    }

    return 0;
}

/*
 * received HTTP request header
 */
static int receiveRequestHeader(HttpRequest request, HttpField header)
{
    switch (header->lcName()) {
    case "host":
	request->setHost(header->value());
	break;
    }

    return 0;
}

/*
 * received HTTP request headers
 */
static int receiveRequestHeaders(HttpRequest request, HttpFields headers)
{
    Iterator i;
    HttpField header;
    int code;

    for (i = headers->iterator(); (header=i->next()); ) {
	code = receiveRequestHeader(request, header);
	if (code != 0) {
	    return code;
	}
    }

    request->setHeaders(headers);
    return 0;
}

/*
 * create internal error response
 */
static HttpResponse httpInternalError(string responsePath)
{
    HttpResponse response;
    HttpFields headers;

    response = new_object(responsePath, 1.1, HTTP_INTERNAL_ERROR,
			  "Internal Error");
    headers = new HttpFields();
    headers->add(new HttpField("Date", new HttpTime));
    headers->add(new HttpField("Connection", ({ "close" })));
    response->setHeaders(headers);

    return response;
}

/*
 * prepare to send HTTP response
 */
static void sendResponse(HttpResponse response)
{
}
