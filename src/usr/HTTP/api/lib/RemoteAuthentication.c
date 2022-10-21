# include "HttpField.h"

inherit HttpAuthentication;


# define HTTP_AUTHORIZE		"/usr/HTTP/sys/authorize"

/*
 * create authentication item
 */
static void create(string blob)
{
    mixed *parsed;

    parsed = HTTP_AUTHORIZE->authorize(blob);
    if (!parsed) {
	error("Bad request");
    }
    ::create(parsed...);
}
