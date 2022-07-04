# include <Time.h>
# include "HttpField.h"

inherit HttpFieldItem;


private Time time;	/* actual time */

/*
 * create time item
 */
static void create(varargs Time time)
{
    ::time = (time) ? time : new Time(millitime()...);
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
