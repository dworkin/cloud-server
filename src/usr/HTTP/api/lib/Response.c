# include "HttpField.h"


private float version;		/* HTTP version */
private int code;		/* response code */
private string comment;		/* response comment */
private HttpFields headers;	/* response headers */

/*
 * create response
 */
static void create(float version, int code, string comment)
{
    ::version = version;
    ::code = code;
    ::comment = comment;
}

/*
 * add headers
 */
void setHeaders(HttpFields headers)
{
    ::headers = headers;
}

/*
 * get header value from response
 */
mixed headerValue(string str)
{
    HttpField header;

    if (!headers) {
	return nil;
    }
    header = headers->get(str);
    if (!header) {
	return nil;
    }

    return header->value();
}


/*
 * transport this response
 */
string transport()
{
    string str;

    if (version < 1.0) {
	return "";
    }
    str = (string) (version * 10.0);
    return "HTTP/" + str[0 .. 0] + "." + str[1 .. 1] + " " + code + " " +
	   comment + "\r\n" + headers->transport() + "\r\n";
}


float version()		{ return version; }
int code()		{ return code; }
string comment()	{ return comment; }
HttpFields headers()	{ return headers; }
