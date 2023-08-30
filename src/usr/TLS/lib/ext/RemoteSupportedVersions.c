# include <String.h>
# include "Extension.h"

inherit SupportedVersions;


/*
 * initialize SupportedVersions from a blob
 */
static create(String blob, int offset, int end)
{
    string versions;
    int next;

    ({ versions, next }) = len1Restore(blob, offset);
    if (next != end) {
	error("DECODE_ERROR");
    }
    ::create(EXPLODE2->explode2(versions));
}
