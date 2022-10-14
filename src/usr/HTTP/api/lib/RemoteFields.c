# include <Time.h>
# include <type.h>
# include "HttpField.h"

inherit HttpFields;

private inherit "/lib/util/ascii";


# define HTTP_FIELDS		"/usr/HTTP/sys/fields"
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
	case "from":
	case "host":
	case "referer":
	case "user-agent":
	    addField(name, value);
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

	default:
	    addUnknownField(name, value);
	    break;
	}
    }
}
