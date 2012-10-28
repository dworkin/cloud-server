/*
 * This code will only work for times up till Sun Feb  7 06:28:15 2106 GMT.
 * It will have to be revised if the start of the POSIX time epoch, now 1970,
 * is changed.
 */

/*
 * NAME:	timecmp()
 * DESCRIPTION:	compare two times as unsigned ints
 */
static int timecmp(int time1, int time2)
{
    if ((time1 ^ time2) >= 0) {
	/* equal sign */
	return (time1 < time2) ? -1 : (time1 == time2) ? 0 : 1;
    } else if (time1 < 0) {
	/* time1 < 0, time2 >= 0 */
	return 1;
    } else {
	/* time1 >= 0, time2 < 0 */
	return -1;
    }
}

# define SECSPERMIN	60
# define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
# define SECSPERDAY	(SECSPERMIN * MINSPERHOUR * HOURSPERDAY)
# define MINSPERHOUR	60
# define HOURSPERDAY	24
# define DAYSPERWEEK	7
# define DAYSPERYEAR	365
# define ISLEAP(y)	(((y) % 4 == 0 && (y) != 2100) || (y) == 2000)
# define LEAPCOUNT(y)	((y) / 4 - (y) / 100 + (y) / 400)
# define JAN1		0
# define FEB1		(JAN1 + 31)
# define MAR1		(FEB1 + 29)
# define APR1		(MAR1 + 31)
# define MAY1		(APR1 + 30)
# define JUN1		(MAY1 + 31)
# define JUL1		(JUN1 + 30)
# define AUG1		(JUL1 + 31)
# define SEP1		(AUG1 + 31)
# define OCT1		(SEP1 + 30)
# define NOV1		(OCT1 + 31)
# define DEC1		(NOV1 + 30)
# define LEAPDAY	(MAR1 - 1)
# define EPOCH_YEAR	1970
# define EPOCH_WEEKDAY	4	/* Thursday */

/*
 * NAME:	gmctime()
 * DESCRIPTION:	GMT ctime()
 */
static string gmctime(int time)
{
    int year, days, hours, minutes, seconds, leap;
    string month, weekday;

    days = 0;
    while (time < 0) {
	days += 24850;			/* 3550 weeks */
	time -= 24850 * SECSPERDAY;
    }
    days += time / SECSPERDAY;
    time %= SECSPERDAY;

    hours = time / SECSPERHOUR;
    time %= SECSPERHOUR;

    minutes = time / SECSPERMIN;
    seconds = time % SECSPERMIN;

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

/*
 * NAME:	gm2time()
 * DESCRIPTION:	convert a GMT ctime() string to POSIX time; perform only
 *		superficial checks to see if the input is correct
 */
static int gm2time(string gmtime)
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

    return SECSPERDAY * day + SECSPERHOUR * hour + SECSPERMIN * minute + second;
}
