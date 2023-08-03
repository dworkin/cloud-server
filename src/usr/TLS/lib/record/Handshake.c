# include "Record.h"

inherit Data;


private Data message;	/* handshake message */

/*
 * initialize Handshake
 */
static void create(Data message)
{
    ::create(RECORD_HANDSHAKE);
    ::message = message;
}

/*
 * export as a blob
 */
string transport()
{
    string str;

    return message->typeHeader() + len3Save(message->transport());
}


Data message()	{ return message; }
