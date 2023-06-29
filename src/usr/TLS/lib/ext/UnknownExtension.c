# include <String.h>
# include "Record.h"

inherit Data;


private string data;

static void create(String blob, int offset, int end)
{
    data = substring(blob, offset, end - 1);
}

string transport()
{
    return data;
}


string data()	{ return data; }
