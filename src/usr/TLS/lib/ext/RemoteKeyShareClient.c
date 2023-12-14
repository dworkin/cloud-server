# include <String.h>
# include "Extension.h"

inherit KeyShareClient;


/*
 * initialize KeyShare for ClientHello from a blob
 */
static void create(String blob, int offset, int end)
{
    string **keyShare, group, key;

    if (len2Offset(blob, offset) != end) {
	error("DECODE_ERROR");
    }

    offset += 2;
    keyShare = ({ });
    while (offset < end) {
	group = substring(blob, offset, offset + 1);
	({ key, offset }) = len2Restore(blob, offset + 2);
	keyShare += ({
	    ({ group, key })
	});
    }
    if (offset != end) {
	error("DECODE_ERROR");
    }

    ::create(keyShare);
}
