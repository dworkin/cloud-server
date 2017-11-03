# include "Time.h"

inherit Time;


/*
 * initialize from a GMT ctime
 */
static void create(string gmtime)
{
    string month;
    int day, hour, minute, second, year;

    if (sscanf(gmtime, "%*s %s %d %d:%d:%d %d", month, day, hour, minute,
	       second, year) != 7) {
	error("String not in date format");
    }

    /* count days since start of epoch */
    switch (month) {
    case "Jan":
	day += JAN1 - 1; break;
    case "Feb":
	day += FEB1 - 1; break;
    case "Mar":
	day += MAR1 - 1; break;
    case "Apr":
	day += APR1 - 1; break;
    case "May":
	day += MAY1 - 1; break;
    case "Jun":
	day += JUN1 - 1; break;
    case "Jul":
	day += JUL1 - 1; break;
    case "Aug":
	day += AUG1 - 1; break;
    case "Sep":
	day += SEP1 - 1; break;
    case "Oct":
	day += OCT1 - 1; break;
    case "Nov":
	day += NOV1 - 1; break;
    case "Dec":
	day += DEC1 - 1; break;
    default:
	error("String not in date format");
    }
    if (!ISLEAP(year) && day > LEAPDAY) {
	--day;
    }
    day += (year - EPOCH_YEAR) * DAYSPERYEAR +
	   LEAPCOUNT(year - 1) - LEAPCOUNT(EPOCH_YEAR - 1);

    ::create(SECSPERDAY * day + SECSPERHOUR * hour + SECSPERMIN * minute +
	     second);
}
