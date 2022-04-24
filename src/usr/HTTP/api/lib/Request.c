# include "HttpHeader.h"


private float version;		/* HTTP protocol version */
private string method;		/* request method */
private string scheme;		/* request scheme, including :// */
private string host;		/* request host */
private string path;		/* request path */
private HttpHeaders headers;	/* request headers */

/*
 * create request
 */
static void create(float version, string method, string scheme, string host,
		   string path)
{
    ::version = version;
    ::method = method;
    ::scheme = scheme;
    ::host = host;
    ::path = path;
}

/*
 * set headers of request
 */
void setHeaders(HttpHeaders headers)
{
    ::headers = headers;
}

/*
 * transport this request
 */
string transport()
{
    string str;

    str = " ";
    if (scheme) {
	str += scheme;
    }
    if (host) {
	str += host;
    }
    if (path) {
	str += path;
    }
    if (str == " ") {
	str = " *";
    }
    if (version < 1.0) {
	return method + str;
    } else {
	return method + str + " HTTP/" + version + headers->transport();
    }
}

float version()		{ return version; }
string method()		{ return method; }
string scheme()		{ return scheme; }
string host()		{ return host; }
string path()		{ return path; }
HttpHeaders headers()	{ return headers; }
