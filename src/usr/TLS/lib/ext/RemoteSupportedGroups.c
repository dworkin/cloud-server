# include <String.h>
# include "Extension.h"

inherit SupportedGroups;


static void create(String blob, int offset, int end)
{
    string groups;

    ({ groups, offset }) = len2Restore(blob, offset);
    if (offset != end) {
	error("Decode error");
    }
    ::create(groups);
}
