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
# define HTTP_TOKENPARAMLIST	"/usr/HTTP/sys/tokenparamlist"

/*
 * parse WWW-Authenticate
 */
static HttpAuthentication *parseAuthenticate(string str)
{
    return HTTP_AUTHENTICATE->authenticate(str);
}

/*
 * parse User-Agent, Server
 */
static HttpProduct *parseProducts(string str)
{
    return HTTP_PRODUCTS->products(str);
}

/*
 * parse Content-Type
 */
static mixed *parseTypeParam(string str)
{
    return HTTP_TYPEPARAM->typeparam(str);
}

/*
 * parse generic list
 */
static string *parseList(string str)
{
    return HTTP_LIST->list(str);
}

/*
 * parse TE, Transfer-Encoding
 */
static mixed *parseTokenParamList(string str)
{
    return HTTP_TOKENPARAMLIST->tokenparamlist(str);
}

/*
 * add unknown field value as a list
 */
static void addUnknownField(string name, string value)
{
    if (value) {
	addFieldList(name, parseList(value));
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
	    addField(name, parseTypeParam(value)...);
	    break;

	case "date":
	case "expires":
	case "if-modified-since":
	case "last-modified":
	    addField(name, new RemoteHttpTime(value));
	    break;

	case "from":
	case "host":
	case "referer":
	case "sec-websocket-key":
	    if (!value || sscanf(value, "%*s ") != 0) {
		error("Bad field");
	    }
	    addField(name, value);
	    break;

	case "server":
	case "user-agent":
	    addField(name, parseProducts(value));
	    break;

	case "te":
	case "transfer-encoding":
	    if (value) {
		addFieldList(name, parseTokenParamList(value)...);
	    } else {
		addFieldList(name, ({ }), ({ }));
	    }
	    break;

	case "www-authenticate":
	    if (value) {
		addFieldList(name, parseAuthenticate(value));
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
