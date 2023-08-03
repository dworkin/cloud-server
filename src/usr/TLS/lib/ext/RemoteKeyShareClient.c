# include <String.h>
# include "Extension.h"

inherit KeyShareClient;


/*
 * initialize KeyShare for ClientHello from a blob
 */
static void create(String blob, int offset, int end)
{
    string **keyShare, group, keyExchange;

    if (len2Offset(blob, offset) != end) {
	error("Decode error");
    }

    offset += 2;
    keyShare = ({ });
    while (offset < end) {
	group = substring(blob, offset, offset + 1);
	({ keyExchange, offset }) = len2Restore(blob, offset + 2);
	keyShare += ({
	    ({ group, keyExchange })
	});
    }
    if (offset != end) {
	error("Decode error");
    }

    ::create(keyShare);
}
