# include "HttpRequest.h"
# include "HttpHeader.h"
# include "HttpResponse.h"


private HttpRequest request;	/* HTTP request */

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

    ::request = request;
    return 0;
}

/*
 * verify HTTP headers
 */
static int httpHeaders(HttpHeaders headers)
{
    request->setHeaders(headers);
    return 0;
}
