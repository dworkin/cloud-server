# include <String.h>
# include "Extension.h"

inherit KeyShareServer;


/*
 * initialize KeyShare for ServerHello from a blob
 */
static void create(String blob, int offset, int end)
{
    string group, key;

    group = substring(blob, offset, offset + 1);
    ({ key, offset }) = len2Restore(blob, offset + 2);
    if (offset != end) {
	error("DECODE_ERROR");
    }

    ::create(({ group, key }));
}
