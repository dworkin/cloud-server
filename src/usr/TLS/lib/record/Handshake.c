# include "Record.h"

inherit Data;


private Data message;

static void create(Data message)
{
    ::create(RECORD_HANDSHAKE);
    ::message = message;
}

string transport()
{
    string str;

    return message->typeHeader() + len3Save(message->transport());
}


Data message()	{ return message; }
