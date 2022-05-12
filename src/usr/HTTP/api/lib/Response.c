# include "HttpHeader.h"


private float version;		/* HTTP version */
private int code;		/* response code */
private string comment;		/* response comment */
private HttpHeaders headers;	/* response headers */

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
void setHeaders(HttpHeaders headers)
{
    ::headers = headers;
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
	   comment + "\r\n" + headers->transport();
}


float version()		{ return version; }
int code()		{ return code; }
string comment()	{ return comment; }
HttpHeaders headers()	{ return headers; }
