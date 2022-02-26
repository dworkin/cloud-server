# include <Time.h>

/*
 * parse date formats
 */
Time date(string str)
{
    string *parsed;

    parsed = parse_string("\
whitespace = / /							\
word = /[A-Za-z]+/							\
number = /[0-9]+/							\
time = /[0-2][0-9]:[0-5][0-9]:[0-5][0-9]/				\
junk = /./								\
\
Date: word ',' number word number time 'GMT'		? rfc5322	\
Date: word ',' number '-' word '-' number time 'GMT'	? rfc850	\
Date: word word number time number",			/* asctime */
			  str);

    if (parsed) {
	catch {
	    return new GMTime(implode(parsed, " "));
	}
    }
    return nil;
}

/*
 * Sun, 06 Nov 1994 08:49:37 GMT
 */
static mixed *rfc5322(mixed *parsed)
{
    return ({ parsed[0], parsed[3], parsed[2], parsed[5], parsed[4] });
}

/*
 * Sunday, 06-Nov-94 08:49:37 GMT
 */
static mixed *rfc850(mixed *parsed)
{
    int year;

    year = (int) parsed[6];
    year += (year < 70) ? 2000 : 1900;	/* hopefully obsolete in 2070 */
    return ({ parsed[0], parsed[4], parsed[2], parsed[7], (string) year });
}
