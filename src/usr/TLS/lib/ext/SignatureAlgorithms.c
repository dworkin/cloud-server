# include "Record.h"

inherit Data;


private string algorithms;

static void create(string algorithms)
{
    ::algorithms = algorithms;
}

string transport()
{
    return len2Save(algorithms);
}


string algorithms()	{ return algorithms; }
