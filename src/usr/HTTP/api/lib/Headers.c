# include <Time.h>
# include <type.h>
# include "HttpHeader.h"

inherit HttpHeaderItem;

private inherit "/lib/util/ascii";


private HttpHeader *headers;	/* headers in listed order */
private mapping map;		/* headers mapping */

/*
 * initialize headers object
 */
static void create()
{
    headers = ({ });
    map = ([ ]);
}

/*
 * add a new unique header
 */
static void addHeader(string name, mixed value)
{
    string lcName;
    HttpHeader header;

    lcName = lower_case(name);
    if (map[lcName]) {
	error("Header already exists");
    }
    header = new HttpHeader(name, value);
    headers += ({ header });
    map[lcName] = header;
}

/*
 * add a new list-header
 */
static void addHeaderList(string name, mixed *value)
{
    string lcName;
    HttpHeader header;

    lcName = lower_case(name);
    header = map[lcName];
    if (header) {
	header->add(value);
    } else {
	header = new HttpHeader(name, value);
	headers += ({ header });
	map[lcName] = header;
    }
}

/*
 * add a header
 */
void add(HttpHeader header)
{
    string name, lcName;
    mixed value;

    name = header->name();
    value = header->value();
    lcName = lower_case(name);
    if (map[lcName]) {
	if (typeof(value) == T_ARRAY) {
	    addHeaderList(name, value);
	} else {
	    error("Header already exists");
	}
    } else {
	headers += ({ header });
	map[lcName] = header;
    }
}

/*
 * get header by name
 */
HttpHeader get(string name)
{
    return map[lower_case(name)];
}

/*
 * transport headers
 */
string transport()
{
    int sz, i;
    string *results;

    sz = sizeof(headers);
    results = allocate(sz);
    for (i = 0; i < sz; i++) {
	results[i] = headers[i]->transport();
    }

    return implode(results, "\r\n") + "\r\n\r\n";
}
