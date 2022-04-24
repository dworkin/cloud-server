# include "HttpRequest.h"

inherit HttpRequest;


# define HTTP_REQUESTLINE	"/usr/HTTP/sys/requestline"

/*
 * create request
 */
static void create(string blob)
{
    ::create(HTTP_REQUESTLINE->request(blob)...);
}
