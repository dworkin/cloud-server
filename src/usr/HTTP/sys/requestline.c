/*
 * ({ version, method, scheme, host, path })
 */
mixed *request(string str)
{
    return parse_string("\
version = /HTTP\\/[0-9]+\\.[0-9]+/					\
method = /[^\x00-\x20\x7f-\xff:/?%]+/					\
scheme_host = /[A-Za-z][-+.A-Za-z0-9]*:\\/\\/[^\x00-\x20\x7f-\xff/?%]*/	\
scheme_path = /[A-Za-z][-+.A-Za-z0-9]*:([^\x00-\x20\x7f-\xff/?%]|%[0-9A-Fa-f][0-9A-Fa-f])*/ \
path = /[/?]([^\x00-\x20\x7f-\xff%]|%[0-9A-Fa-f][0-9A-Fa-f])*/		\
junk = /./								\
\
Request:	'GET' ' ' RequestURI 			? simpleQ	\
Request:	Method ' ' RequestURI ' ' version	? fullQ		\
\
Method:		'GET'							\
Method:		'*'							\
Method:		method							\
\
RequestURI:	scheme_host path			? schemeHostPath \
RequestURI:	scheme_host				? schemeHost	\
RequestURI:	scheme_path				? schemePath	\
RequestURI:	scheme_path path			? schemePathPath \
RequestURI:	path					? path		\
RequestURI:	'*'					? star		\
",
			str);
}

/*
 * HTTP/0.9 request
 */
static mixed *simpleQ(mixed *parsed)
{
    return ({ 0.9, "GET" }) + parsed[2 ..];
}

/*
 * HTTP/1.x request
 */
static mixed *fullQ(mixed *parsed)
{
    float version;
    string str;

    sscanf(parsed[6], "HTTP/%f", version);
    if (version < 1.0) {
	version = 1.0;
    }
    if (parsed[0] == "CONNECT") {
	return (parsed[2] != nil && parsed[3] == nil &&
		sscanf(parsed[4], "%*d%s", str) == 2 && strlen(str) == 0) ?
		({ version, "CONNECT", nil, parsed[2] + parsed[4], nil }) :
		nil;
    }
    return ({ version, parsed[0] }) + parsed[2 .. 4];
}

/*
 * scheme://host/path
 */
static mixed *schemeHostPath(mixed *parsed)
{
    string scheme, host;

    sscanf(parsed[0], "%s://%s", scheme, host);
    if (strlen(host) == 0) {
	if (parsed[1][0] == '?') {
	    return nil;
	}
	host = nil;
    }
    return ({ scheme + "://", host, parsed[1] });
}

/*
 * scheme://host
 */
static mixed *schemeHost(mixed *parsed)
{
    string scheme, host;

    sscanf(parsed[0], "%s://%s", scheme, host);
    if (strlen(host) == 0) {
	host = nil;
    }
    return ({ scheme + "://", host, "/" });
}

/*
 * scheme:path
 */
static mixed *schemePath(mixed *parsed)
{
    string scheme, path;

    sscanf(parsed[0], "%s:%s", scheme, path);
    if (strlen(path) == 0) {
	return nil;
    }
    return ({ scheme + ":", nil, path });
}

/*
 * scheme:/path, scheme:path1/path2
 */
static mixed *schemePathPath(mixed *parsed)
{
    string scheme, path;

    sscanf(parsed[0], "%s:%s", scheme, path);
    if (strlen(path) == 0 && parsed[1][0] == '?') {
	return nil;
    }
    return ({ scheme + ":", nil, path + parsed[1] });
}

/*
 * /path, ?path
 */
static mixed *path(mixed *parsed)
{
    return (parsed[0][0] != '?') ? ({ nil, nil, parsed[0] }) : nil;
}

/*
 * *
 */
static mixed *star(mixed parsed)
{
    return ({ nil, nil, nil });
}
