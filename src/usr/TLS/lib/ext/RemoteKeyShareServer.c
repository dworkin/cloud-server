# include <String.h>
# include "Extension.h"

inherit KeyShareServer;


/*
 * initialize KeyShare for ServerHello from a blob
 */
static void create(String blob, int offset, int end)
{
    string group, keyExchange;

    group = substring(blob, offset, offset + 1);
    ({ keyExchange, offset }) = len2Restore(blob, offset + 2);
    if (offset != end) {
	error("Decode error");
    }

    ::create(({ group, keyExchange }));
}
