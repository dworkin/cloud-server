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
    if (offset == end) {
	error("DECODE_ERROR");
    }
    do {
	if (blob[offset] != '\0' || hostName) {
	    error("ILLEGAL_PARAMETER");
	}
	({ hostName, offset }) = len2Restore(blob, offset + 1);
    } while (offset < end);
    if (offset != end) {
	error("DECODE_ERROR");
    }

    ::create(hostName);
}
