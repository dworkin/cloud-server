# include "Record.h"

inherit Data;


private int level;
private int description;

static void create(int level, int description)
{
    ::create(RECORD_ALERT);
    ::level = level;
    ::description = description;
}

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
