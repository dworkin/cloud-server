# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include <Iterator.h>

inherit "~System/lib/user";


/*
 * received HTTP request line
 */
static int receiveRequestLine(HttpRequest request)
{
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
 * received HTTP response status line
 */
static void receiveStatusLine(HttpResponse response)
{
}

/*
 * received HTTP response header
 */
static void receiveResponseHeader(HttpResponse response, HttpField header)
{
}

/*
 * received HTTP response headers
 */
static int receiveResponseHeaders(HttpResponse response, HttpFields headers)
{
    Iterator i;
    HttpField header;

    for (i = headers->iterator(); (header=i->next()); ) {
	receiveResponseHeader(response, header);
    }

    response->setHeaders(headers);
}

/*
 * prepare to send HTTP request
 */
static void sendRequest(HttpRequest request)
{
}

/*
 * prepare to send HTTP response
 */
static void sendResponse(HttpResponse response)
{
}
