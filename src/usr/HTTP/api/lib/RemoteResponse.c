# include "HttpResponse.h"

inherit HttpResponse;


/*
 * create response
 */
static void create(string blob)
{
    float version;
    int code;
    string comment;

    if (sscanf(blob, "HTTP/%f %d %s", version, code, comment) != 3) {
	error("Bad response");
    }
    ::create(version, code, comment);
}
