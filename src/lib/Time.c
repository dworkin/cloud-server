# include <Time.h>

private int time;
private float mtime;

/*
 * initialize Time
 */
static void create(int time, varargs float mtime)
{
    ::time = time;
    ::mtime = mtime;
}

int time()	{ return time; }
float mtime()	{ return mtime; }


/*
 * compare with another Time object
 */
private int compare(Time t)
{
    int time2;

    time2 = t->time();
    if ((time ^ time2) >= 0) {
	/* same sign */
	if (time == time2) {
	    return (int) ((mtime - t->mtime()) * 1000.0);
	} else {
	    return time - time2;
	}
    } else {
	/* unsigned comparison */
	return (time < 0) ? 1 : -1;
    }
}

/*
 * this == t
 */
int equals(Time t)
{
    return (compare(t) == 0);
}

/*
 * this <= t
 */
static int operator<= (Time t)
{
    return (compare(t) <= 0);
}

/*
 * this < t
 */
static int operator< (Time t)
{
    return (compare(t) < 0);
}

/*
 * this >= t
 */
static int operator>= (Time t)
{
    return (compare(t) >= 0);
}

/*
 * this > t
 */
static int operator> (Time t)
{
    return (compare(t) > 0);
}

/*
 * GMT ctime()
 */
string gmctime()
{
    int t, year, days, hours, minutes, seconds, leap;
    string month, weekday;

    days = 0;
    t = time;
    while (t < 0) {
	days += 24850;			/* 3550 weeks */
	t -= 24850 * SECSPERDAY;
    }
    days += t / SECSPERDAY;
    t %= SECSPERDAY;

    hours = t / SECSPERHOUR;
    t %= SECSPERHOUR;

    minutes = t / SECSPERMIN;
    seconds = t % SECSPERMIN;

    switch ((EPOCH_WEEKDAY + days) % DAYSPERWEEK) {
    case 0:
	weekday = "Sun"; break;
    case 1:
	weekday = "Mon"; break;
    case 2:
	weekday = "Tue"; break;
    case 3:
	weekday = "Wed"; break;
    case 4:
	weekday = "Thu"; break;
    case 5:
	weekday = "Fri"; break;
    case 6:
	weekday = "Sat"; break;
    }

    year = EPOCH_YEAR;
    while (days < 0 || days > DAYSPERYEAR + (leap=ISLEAP(year))) {
	int newyear;

	newyear = (days < 0) ? year - 1 : year + days / DAYSPERYEAR;
	days -= (newyear - year) * DAYSPERYEAR +
		LEAPCOUNT(newyear - 1) - LEAPCOUNT(year - 1);
	year = newyear;
    }

    if (!leap && days >= LEAPDAY) {
	days++;
    }
    switch (days) {
    case JAN1 .. FEB1 - 1:
	month = "Jan"; days -= JAN1 - 1; break;
    case FEB1 .. MAR1 - 1:
	month = "Feb"; days -= FEB1 - 1; break;
    case MAR1 .. APR1 - 1:
	month = "Mar"; days -= MAR1 - 1; break;
    case APR1 .. MAY1 - 1:
	month = "Apr"; days -= APR1 - 1; break;
    case MAY1 .. JUN1 - 1:
	month = "May"; days -= MAY1 - 1; break;
    case JUN1 .. JUL1 - 1:
	month = "Jun"; days -= JUN1 - 1; break;
    case JUL1 .. AUG1 - 1:
	month = "Jul"; days -= JUL1 - 1; break;
    case AUG1 .. SEP1 - 1:
	month = "Aug"; days -= AUG1 - 1; break;
    case SEP1 .. OCT1 - 1:
	month = "Sep"; days -= SEP1 - 1; break;
    case OCT1 .. NOV1 - 1:
	month = "Oct"; days -= OCT1 - 1; break;
    case NOV1 .. DEC1 - 1:
	month = "Nov"; days -= NOV1 - 1; break;
    default:
	month = "Dec"; days -= DEC1 - 1; break;
    }

    return weekday + " " + month + " " +
	   ((days >= 10) ? (string) days : " " + days) + " " +
	   ((hours >= 10) ? (string) hours : "0" + hours) + ":" +
	   ((minutes >= 10) ? (string) minutes : "0" + minutes) + ":" +
	   ((seconds >= 10) ? (string) seconds : "0" + seconds) + " " + year;
}
