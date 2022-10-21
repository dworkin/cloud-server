# include <Time.h>
# include <type.h>
# include "HttpField.h"

inherit HttpFields;

private inherit "/lib/util/ascii";


# define HTTP_FIELDS		"/usr/HTTP/sys/fields"
# define HTTP_AUTHENTICATE	"/usr/HTTP/sys/authenticate"
# define HTTP_PRODUCTS		"/usr/HTTP/sys/products"
# define HTTP_TYPEPARAM		"/usr/HTTP/sys/typeparam"
# define HTTP_LIST		"/usr/HTTP/sys/list"
# define HTTP_TOKENLIST		"/usr/HTTP/sys/tokenlist"
# define HTTP_TOKENPARAMLIST	"/usr/HTTP/sys/tokenparamlist"

/*
 * add unknown field value as a list
 */
static void addUnknownField(string name, string value)
{
    if (value) {
	addFieldList(name, HTTP_LIST->list(value));
    } else {
	addFieldList(name, ({ }));
    }
}

/*
 * populate fields from blob
 */
static void create(string blob)
{
    int i, sz;
    string **list, name, value;

    ::create();
    list = HTTP_FIELDS->fields(blob);
    if (!list) {
	error("Bad field");
    }
    for (sz = sizeof(list), i = 0; i < sz; i++) {
	({ name, value }) = list[i];
	switch (lower_case(name)) {
	case "authorization":
	    addField(name, new RemoteHttpAuthentication(value));
	    break;

	case "content-length":
	    addField(name, (int) value);
	    break;

	case "content-type":
	    addField(name, HTTP_TYPEPARAM->typeparam(value)...);
	    break;

	case "from":
	case "host":
	case "referer":
	    if (sscanf(value, "%*s ") != 0) {
		error("Bad field");
	    }
	    addField(name, value);
	    break;

	case "server":
	case "user-agent":
	    addField(name, HTTP_PRODUCTS->products(value));
	    break;

	case "if-modified-since":
	    addField(name, new RemoteHttpTime(value));
	    break;

	case "connection":
	    if (value) {
		addFieldList(name, HTTP_TOKENLIST->tokenlist(value));
	    } else {
		addFieldList(name, ({ }));
	    }
	    break;

	case "te":
	case "transfer-encoding":
	    if (value) {
		addFieldList(name,
			     HTTP_TOKENPARAMLIST->tokenparamlist(value)...);
	    } else {
		addFieldList(name, ({ }), ({ }));
	    }
	    break;

	case "www-authenticate":
	    if (value) {
		addFieldList(name, HTTP_AUTHENTICATE->authenticate(value));
	    } else {
		addFieldList(name, ({ }));
	    }
	    break;

	default:
	    addUnknownField(name, value);
	    break;
	}
    }
}
