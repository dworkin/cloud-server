# include <limits.h>


# define Body	object "/usr/Player/api/lib/body"

/*
 * NAME:	pluralize()
 * DESCRIPTION:	pluralize a string
 */
static string pluralize(string str)
{
    return str + "s";	/* XXX */
}

/*
 * NAME:	article()
 * DESCRIPTION:	return the appropriate indefinite article for a string
 */
static string article(string str)
{
    switch (str[0]) {
    case 'a':
    case 'A':
    case 'e':
    case 'E':
    case 'i':
    case 'I':
    case 'o':
    case 'O':
    case 'u':
    case 'U':	/* XXX */
	return "an";

    default:
	return "a";
    }
}

/*
 * NAME:	addart()
 * DESCRIPTION:	add indefinite article to a string
 */
static string addart(string str)
{
    return article(str) + " " + str;
}

/*
 * NAME:	numtostr()
 * DESCRIPTION:	express a number in words
 */
static string numtostr(int num)
{
    int rem;
    string str;

    switch (num) {
    case 0:	return "zero";
    case 1:	return "one";
    case 2:	return "two";
    case 3:	return "three";
    case 4:	return "four";
    case 5:	return "five";
    case 6:	return "six";
    case 7:	return "seven";
    case 8:	return "eight";
    case 9:	return "nine";
    case 10:	return "ten";
    case 11:	return "eleven";
    case 12:	return "twelve";
    case 13:	return "thirteen";
    case 14:	return "fourteen";
    case 15:	return "fifteen";
    case 16:	return "sixteen";
    case 17:	return "seventeen";
    case 18:	return "eighteen";
    case 19:	return "nineteen";
    case 20:	return "twenty";
    case 30:	return "thirty";
    case 40:	return "forty";
    case 50:	return "fifty";
    case 60:	return "sixty";
    case 70:	return "seventy";
    case 80:	return "eighty";
    case 90:	return "ninety";

    default:
	switch (num) {
	case 21 .. 99:
	    rem = num % 10;
	    return numtostr(num - rem) + "-" + numtostr(rem);

	case 100 .. 999:
	    rem = num % 100;
	    str = numtostr((num - rem) / 100) + " hundred";
	    return (rem == 0) ? str : str + " and " + numtostr(rem);

	case 1000 .. 999999:
	    rem = num % 1000;
	    str = numtostr((num - rem) / 1000) + " thousand";
	    return (rem == 0) ? str : str + ((rem < 100) ? " and " : " ") +
					    numtostr(rem);

	case 1000000 .. 999999999:
	    rem = num % 1000000;
	    str = numtostr((num - rem) / 1000000) + " million";
	    return (rem == 0) ? str : str + ((rem < 100) ? " and " : " ") +
					    numtostr(rem);

	case 1000000000 .. INT_MAX:
	    rem = num % 1000000000;
	    str = numtostr((num - rem) / 1000000000) + " billion";
	    return (rem == 0) ? str : str + ((rem < 100) ? " and " : " ") +
					    numtostr(rem);
	}
    }
}

/*
 * NAME:	describe_object()
 * DESCRIPTION:	return a string describing a single object
 */
static string describe_object(object obj)
{
    string str;

    str = obj->query_short_desc();
    if (!str) {
	return nil;
    }

    return (obj->query_unique()) ?
	    (obj <- Body) ? str : "the " + str : addart(str);
}

/*
 * NAME:	describe_objects()
 * DESCRIPTION:	return either a string describing a single object, or an array
 *		describing multiple objects with similar descriptions merged
 */
static mixed describe_objects(object *objs)
{
    mapping singular, plural, unique, multiple;
    int i, multi, *values;
    string str, plstr, *strs;
    object obj;

    singular = ([ ]);	/* str -> count */
    plural = ([ ]);	/* obj -> str */
    unique = ([ ]);	/* str -> 1 (unique) or 2 (unique body) */
    multiple = ([ ]);	/* str -> plstr */
    multi = FALSE;

    /* collect short descriptions */
    for (i = sizeof(objs); --i >= 0; ) {
	obj = objs[i];
	str = obj->query_short_desc();
	if (str) {
	    if (obj->query_plural()) {
		/* plural object, always described separately */
		multi = TRUE;
		plural[obj] = str;
	    } else {
		if (!singular[str]) {
		    singular[str] = 1;
		} else if (++singular[str] == 2) {
		    /* more than one of this description */
		    multi = TRUE;
		    plstr = obj->query_plural_short_desc();
		    multiple[str] = (plstr) ? plstr : pluralize(str);
		}
		if (obj->query_unique()) {
		    if (obj <- Body) {
			unique[str] = 2;	/* unique body */
		    } else {
			unique[str] = 1;	/* unique object */
		    }
		}
	    }
	}
    }

    /* add articles & counts */
    strs = map_indices(singular);
    values = map_values(singular);
    for (i = sizeof(strs); --i >= 0; ) {
	str = strs[i];
	if (values[i] == 1) {
	    switch (unique[str]) {
	    case 1:
		/* unique */
		strs[i] = "the " + str;
		break;

	    case 2:
		/* no change */
		break;

	    default:
		/* not unique */
		strs[i] = addart(str);
		break;
	    }
	} else {
	    strs[i] = numtostr(values[i]) + " " + multiple[str];
	}
    }

    /* return result */
    return (sizeof(strs) == 1 && !multi) ? strs[0] : strs;
}

/*
 * NAME:	summarize()
 * DESCRIPTION:	return a summary of a list of strings
 */
static string summarize(string *strs)
{
    int i;

    switch (i = sizeof(strs)) {
    case 0:
	return nil;

    case 1:
	return strs[0];

    default:
	return implode(strs[.. i - 2], ", ") + " and " + strs[i - 1];
    }
}
