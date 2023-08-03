# include "Record.h"

inherit Data;


private string *algorithms;	/* signature algorithms */

/*
 * initialize SignatureAlgorithms
 */
static void create(string *algorithms)
{
    ::algorithms = algorithms;
}

/*
 * export as a blob
 */
string transport()
{
    return len2Save(implode(algorithms, ""));
}


string *algorithms()	{ return algorithms; }
