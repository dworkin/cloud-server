# include <String.h>
# include "Extension.h"

inherit Cookie;


/*
 * initialize Cookie from a blob
 */
static void create(String blob, int offset, int end)
{
    string cookie;

    ({ cookie, offset }) = len2Restore(blob, offset);
    if (offset != end) {
	error("Decode error");
    }
    ::create(cookie);
}
