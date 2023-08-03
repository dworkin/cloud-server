# include "Record.h"

inherit Data;


private int level;		/* ALERT_WARNING, ALERT_FATAL */
private int description;	/* description of the alert condition */

/*
 * initialize alert
 */
static void create(int level, int description)
{
    ::create(RECORD_ALERT);
    ::level = level;
    ::description = description;
}

/*
 * export as a blob
 */
string transport()
{
    string str;

    str = "..";
    str[0] = level;
    str[1] = description;

    return str;
}


int level()		{ return level; }
int description()	{ return description; }
