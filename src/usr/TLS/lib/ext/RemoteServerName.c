# include <String.h>
# include "Extension.h"

inherit ServerName;


/*
 * initialize ServerName from a blob
 */
static void create(String blob, int offset, int end)
{
    string hostName;

    if (len2Offset(blob, offset) != end) {
	error("DECODE_ERROR");
    }

    offset += 2;
    while (offset < end) {
	if (blob[offset] == '\0') {
	    if (hostName) {
		error("ILLEGAL_PARAMETER");
	    }
	    ({ hostName, offset }) = len2Restore(blob, offset + 1);
	} else {
	    offset = len2Offset(blob, offset + 1);
	}
    }
    if (offset != end) {
	error("DECODE_ERROR");
    }

    ::create(hostName);
}
