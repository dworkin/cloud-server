# include <String.h>
# include "Record.h"
# include <status.h>

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

mixed transport();	/* override inherited prototype */

/*
 * export as a blob
 */
mixed transport()
{
    StringBuffer buffer;
    mixed str;

    buffer = new StringBuffer(message->typeHeader());
    str = message->transport();
    buffer->append(len3(str));
    buffer->append(str);
    return (buffer->length() <= status(ST_STRSIZE)) ? buffer->chunk() : buffer;
}


Data message()	{ return message; }
