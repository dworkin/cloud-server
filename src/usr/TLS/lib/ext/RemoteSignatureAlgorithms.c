# include <String.h>
# include "Extension.h"

inherit SignatureAlgorithms;


/*
 * initialize SignatureAlgorithms from a blob
 */
static void create(String blob, int offset, int end)
{
    string algorithms;

    ({ algorithms, offset }) = len2Restore(blob, offset);
    if (offset != end) {
	error("DECODE_ERROR");
    }
    ::create(EXPLODE2->explode2(algorithms));
}
