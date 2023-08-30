# include <String.h>
# include "Extension.h"

inherit ServerName;


/*
 * initialize ServerName from a blob
 */
static void create(String blob, int offset, int end)
{
    string *names, name;

    if (len2Offset(blob, offset) != end) {
	error("DECODE_ERROR");
    }

    offset += 2;
    names = ({ });
    while (offset < end) {
	if (blob[offset] != '\0') {
	    error("DECODE_ERROR");
	}
	({ name, offset }) = len2Restore(blob, offset + 1);
	names += ({ name });
    }
    if (offset != end) {
	error("DECODE_ERROR");
    }

    ::create(names);
}
