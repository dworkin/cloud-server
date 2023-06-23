# include "Record.h"

inherit Data;


private string data;

static void create(int type, string data)
{
    ::create(type);
    ::data = data;
}

string transport()
{
    int type;
    string str;

    type = type();
    str = "..";
    str[0] = type >> 8;
    str[1] = type;
    return str + len2Save(data);
}


string data()	{ return data; }
