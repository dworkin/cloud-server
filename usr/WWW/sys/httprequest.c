/*
 * NAME:	parse_request()
 * DESCRIPTION:	Parse the given line, return a structure of the form
 *
 *		    ({ version, method, scheme, host, path, parameter list })
 *
 *		if it is a HTTP request, or nil otherwise.
 */
mixed *parse_request(string line)
{
    return parse_string("\
sp = /[ \t]+/								\
version = /HTTP\\/[0-9]+\\.[0-9]+/					\
token = /[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?&={}]+/		\
scheme_host = /[^\x00-\x20\x7f\"<>#/?&:]+:(\\/\\/[^\x00-\x20\x7f\"<>#/?&]*)?/ \
path = /\\/([^\x00-\x20\x7f\"<>#%?&]|%[0-9A-Fa-f][0-9A-Fa-f])*/		\
param1 = /\\?([^\x00-\x20\x7f\"<>#%?&]|%[0-9A-Fa-f][0-9A-Fa-f])*/	\
paramn = /&([^\x00-\x20\x7f\"<>#%?&]|%[0-9A-Fa-f][0-9A-Fa-f])*/		\
junk = /./								\
"+"\
Request:	WS 'GET' sp RequestURI WS		? simpleq	\
Request:	WS Method sp RequestURI sp version WS	? fullq		\
"+"\
WS:									\
WS:		sp					? null		\
"+"\
Method:		'GET'							\
Method:		'*'							\
Method:		token							\
"+"\
RequestURI:	SchemeHost Path OptParams				\
RequestURI:	'*'					? star		\
"+"\
SchemeHost:						? nullscheme	\
SchemeHost:	scheme_host				? schemehost	\
"+"\
Path:		path					? path		\
"+"\
OptParams:						? nullparams	\
OptParams:	param1 Params				? param1	\
"+"\
Params:									\
Params:		Params Param						\
Param:		paramn					? paramn	\
",
			line);
}

/*
 * NAME:	simpleq()
 * DESCRIPTION:	return HTTP/0.9 query request
 */
static mixed *simpleq(mixed *request)
{
    return ({ 0.9, "GET" }) + request[2 ..];
}

/*
 * NAME:	fullq()
 * DESCRIPTION:	return HTTP/1.x query request
 */
static mixed *fullq(mixed *request)
{
    int size;
    float version;

    size = sizeof(request);
    sscanf(request[size - 1], "HTTP/%f", version);
    if (version < 1.0) {
	version = 1.0;
    }
    return ({ version, request[0] }) + request[2 .. size - 3];
}

/*
 * NAME:	null()
 * DESCRIPTION:	return empty array
 */
static mixed *null(mixed stuff)
{
    return ({ });
}

/*
 * NAME:	star()
 * DESCRIPTION:	deal with "*" URI for HTTP/1.1 compatibility
 */
static mixed *star(mixed star)
{
    return ({ nil, nil, nil, ({ }) });
}

# define HEX	("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09......." +	\
		 "\x0A\x0B\x0C\x0D\x0E\x0F.........................." +	\
		 "\x0a\x0b\x0c\x0d\x0e\x0f")

/*
 * NAME:	escape()
 * DESCRIPTION:	handle an escape sequence
 */
private string escape(string str)
{
    string result, byte, head;
    int c1, c2;

    result = "";
    byte = ".";
    while (sscanf(str, "%s%%%c%c%s", head, c1, c2, str) != 0) {
	byte[0] = (HEX[c1 - '0'] << 4) + HEX[c2 - '0'];
	result += head + byte;
    }
    return result + str;
}

/*
 * NAME:	nullscheme()
 * DESCRIPTION:	no scheme, no host
 */
static mixed *nullscheme(mixed stuff)
{
    return ({ nil, nil });
}

/*
 * NAME:	schemehost()
 * DESCRIPTION:	scheme, and possibly a host
 */
static mixed *schemehost(mixed *schemehost)
{
    string scheme, host;

    scheme = schemehost[0];
    sscanf(scheme, "%s//%s", scheme, host);
    return ({ scheme[.. strlen(scheme) - 2], host });
}

/*
 * NAME:	path()
 * DESCRIPTION:	handle a path
 */
static string *path(string *path)
{
    return ({ escape(path[0]) });
}

/*
 * NAME:	nullparams()
 * DESCRIPTION:	no parameters
 */
static string **nullparams(mixed params)
{
    return ({ ({ }) });
}

/*
 * NAME:	param1()
 * DESCRIPTION:	deal with first parameter
 */
static string **param1(mixed *params)
{
    return ({ ({ escape(params[0][1 ..]) }) + params[1 ..] });
}

/*
 * NAME:	paramn()
 * DESCRIPTION:	deal with nth parameter
 */
static string *paramn(mixed *params)
{
    return ({ escape(params[0][1 ..]) });
}
