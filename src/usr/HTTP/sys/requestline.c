/*
 * ({ version, method, path, scheme, host })
 */
mixed *request(string str)
{
    return parse_string("\
sp = /[ \t]+/								\
version = /HTTP\\/[0-9]+\\.[0-9]+/					\
method = /[^\x00-\x20\x7f-\xff:/?%]+/					\
scheme_host = /[A-Za-z][-+.A-Za-z0-9]*:(\\/\\/[^\x00-\x20\x7f-\xff/?%]*)?/ \
path = /[/?]([^\x00-\x20\x7f-\xff%]|%[0-9A-Fa-f][0-9A-Fa-f])*/		\
junk = /./								\
\
Request:	WS 'GET' sp RequestURI WS		? simpleQ	\
Request:	WS Method sp RequestURI sp version WS	? fullQ		\
\
WS:									\
WS:		sp					? null		\
\
Method:		'GET'							\
Method:		'*'							\
Method:		method							\
\
RequestURI:	SchemeHost Path				? schemeHostPath \
RequestURI:	Path					? nullScheme	\
RequestURI:	SchemeHost				? nullPath	\
RequestURI:	'*'					? star		\
\
SchemeHost:	scheme_host				? schemeHost	\
\
Path:		path					? path		\
",
			str);
}

# define HEX	("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09......." +	\
		 "\x0A\x0B\x0C\x0D\x0E\x0F.........................." +	\
		 "\x0a\x0b\x0c\x0d\x0e\x0f")

/*
 * decode escape sequences
 */
private string decode(string str)
{
    string result, char, head;
    int c1, c2;

    result = "";
    char = ".";
    while (sscanf(str, "%s%%%c%c%s", head, c1, c2, str) != 0) {
	char[0] = (HEX[c1 - '0'] << 4) + HEX[c2 - '0'];
	result += head + char;
    }
    return result + str;
}

/*
 * HTTP/0.9 query request
 */
static mixed *simpleQ(mixed *parsed)
{
    return ({ 0.9, "GET" }) + parsed[2 ..];
}

/*
 * HTTP/1.x query request
 */
static mixed *fullQ(mixed *parsed)
{
    int size;
    float version;

    size = sizeof(parsed);
    sscanf(parsed[size - 1], "HTTP/%f", version);
    if (version < 1.0) {
	version = 1.0;
    }
    return ({ version, parsed[0] }) + parsed[2 .. size - 3];
}

/*
 * return empty array
 */
static mixed *null(mixed parsed)
{
    return ({ });
}

/*
 * don't accept http:?path
 */
static mixed *schemeHostPath(mixed *parsed)
{
    return (parsed[1] || parsed[2][0] != '?') ?
	    ({ parsed[2], parsed[0], parsed[1] }) : nil;
}

/*
 * no scheme, no host
 */
static mixed *nullScheme(mixed *parsed)
{
    return (parsed[0][0] != '?') ? parsed + ({ nil, nil }) : nil;
}

/*
 * don't accept null path without host
 */
static mixed *nullPath(mixed *parsed)
{
    return (parsed[1]) ? ({ "" }) + parsed : nil;
}

/*
 * deal with "*" URI
 */
static mixed *star(mixed parsed)
{
    return ({ nil, nil, nil });
}

/*
 * scheme, and possibly a host
 */
static mixed *schemeHost(mixed *parsed)
{
    string scheme, host;

    scheme = parsed[0];
    if (sscanf(scheme, "%s://%s", scheme, host) == 0) {
	sscanf(scheme, "%s:%s", scheme, host);
    }
    if (strlen(host) == 0) {
	host = nil;
    }
    return ({ scheme, host });
}

/*
 * handle a path
 */
static mixed *path(string *parsed)
{
    return ({ decode(parsed[0]) });
}
