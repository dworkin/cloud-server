# include <String.h>
# include "Extension.h"

inherit SupportedGroups;


/*
 * initialize SupportedGroups from a blob
 */
static void create(String blob, int offset, int end)
{
    string groups;

    ({ groups, offset }) = len2Restore(blob, offset);
    if (offset != end) {
	error("DECODE_ERROR");
    }
    ::create(EXPLODE2->explode2(groups));
}
