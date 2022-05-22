# include "HttpRequest.h"
# include "HttpHeader.h"
# include "HttpResponse.h"
# include <Time.h>


/*
 * verify HTTP request
 */
static int httpRequest(HttpRequest request)
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
 * verify HTTP headers
 */
static int httpHeaders(HttpRequest request, HttpHeaders headers)
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
    HttpHeaders headers;

    response = new_object(responsePath, 1.1, HTTP_BAD_REQUEST, "Bad Request");
    headers = new HttpHeaders();
    headers->add(new HttpHeader("Date", new HttpTime));
    headers->add(new HttpHeader("Connection", ({ "close" })));
    response->setHeaders(headers);

    return response;
}

/*
 * create internal error response
 */
static HttpResponse httpInternalError(string responsePath)
{
    HttpResponse response;
    HttpHeaders headers;

    response = new_object(responsePath, 1.1, HTTP_INTERNAL_ERROR,
			  "Internal Error");
    headers = new HttpHeaders();
    headers->add(new HttpHeader("Date", new HttpTime));
    headers->add(new HttpHeader("Connection", ({ "close" })));
    response->setHeaders(headers);

    return response;
}

/*
 * verify HTTP response
 */
static void httpResponse(HttpResponse response)
{
}
