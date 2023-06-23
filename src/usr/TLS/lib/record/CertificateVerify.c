# include "Record.h"

inherit Data;


private string algorithm;
private string signature;

static void create(string algorithm, string signature)
{
    ::create(HANDSHAKE_CERTIFICATE_VERIFY);
    ::algorithm = algorithm;
    ::signature = signature;
}

string transport()
{
    return algorithm +
	   len2Save(signature);
}


string algorithm()	{ return algorithm; }
string signature()	{ return signature; }
