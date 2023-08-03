# include "Record.h"

inherit Data;


private Data data;	/* extension contained herein */

/*
 * initialize Extension
 */
static void create(int type, Data data)
{
    ::create(type);
    ::data = data;
}

/*
 * export as a blob
 */
string transport()
{
    int type;
    string str;

    type = type();
    str = "..";
    str[0] = type >> 8;
    str[1] = type;
    return str + len2Save(data->transport());
}


Data data()	{ return data; }
