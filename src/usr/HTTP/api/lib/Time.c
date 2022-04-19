# include <Time.h>
# include "HttpHeader.h"

inherit HttpHeaderItem;


private Time time;	/* actual time */

/*
 * create time item
 */
static void create(Time time)
{
    ::time = time;
}

/*
 * transport time item
 */
string transport()
{
    string weekday, month, hms;
    int day, year;

    sscanf(time->gmctime(), "%s %s %d %s %d", weekday, month, day, hms, year);
    return weekday + ", " + ((day >= 10) ? (string) day : "0" + day) + " " +
		     month + " " + year + " " + hms + " GMT";
}

Time time()	{ return time; }
