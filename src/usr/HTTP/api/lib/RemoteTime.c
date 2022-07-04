# include <Time.h>
# include "HttpField.h"

inherit HttpTime;


# define HTTP_DATE	"/usr/HTTP/sys/date"

/*
 * create time item
 */
static void create(string blob)
{
    Time time;

    time = HTTP_DATE->date(blob);
    if (!time) {
	error("Bad request");
    }
    ::create(time);
}
