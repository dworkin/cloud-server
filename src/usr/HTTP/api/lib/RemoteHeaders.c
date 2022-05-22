# include <Time.h>
# include <type.h>
# include "HttpHeader.h"

inherit HttpHeaders;

private inherit "/lib/util/ascii";


# define HTTP_HEADERS		"/usr/HTTP/sys/headers"
# define HTTP_LIST		"/usr/HTTP/sys/list"

/*
 * add unknown header value as a list
 */
static void addUnknownHeader(string name, string value)
{
    if (value) {
	addHeaderList(name, HTTP_LIST->list(value));
    } else {
	addHeaderList(name, ({ value }));
    }
}

/*
 * populate headers from blob
 */
static void create(string blob)
{
    int i, sz;
    string **list, name, value;

    ::create();
    list = HTTP_HEADERS->headers(blob);
    if (!list) {
	error("Bad request");
    }
    for (sz = sizeof(list), i = 0; i < sz; i++) {
	({ name, value }) = list[i];
	switch (lower_case(name)) {
	case "authorization":
	    addHeader(name, new RemoteHttpAuthentication(value));
	    break;

	case "content-length":
	    addHeader(name, (int) value);
	    break;

	case "content-type":
	case "from":
	case "host":
	case "referer":
	case "user-agent":
	    addHeader(name, value);
	    break;

	case "if-modified-since":
	    addHeader(name, new RemoteHttpTime(value));
	    break;

	default:
	    addUnknownHeader(name, value);
	    break;
	}
    }
}
