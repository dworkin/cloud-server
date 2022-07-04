# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include <Time.h>

inherit "~System/lib/user";


/*
 * received HTTP request line
 */
static int receiveRequestLine(HttpRequest request)
{
    switch (request->method()) {
    case "GET":
    case "HEAD":
    case "POST":
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
 * received HTTP request headers
 */
static int receiveRequestHeaders(HttpRequest request, HttpFields headers)
{
    request->setHeaders(headers);
    return 0;
}

/*
 * create bad request response
 */
static HttpResponse httpBadRequest(string responsePath)
{
    HttpResponse response;
    HttpFields headers;

    response = new_object(responsePath, 1.1, HTTP_BAD_REQUEST, "Bad Request");
    headers = new HttpFields();
    headers->add(new HttpField("Date", new HttpTime));
    headers->add(new HttpField("Connection", ({ "close" })));
    response->setHeaders(headers);

    return response;
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
