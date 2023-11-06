# include <String.h>
# include "Extension.h"

inherit EarlyData;


/*
 * initialize EarlyData from a blob
 */
static void create(String blob, int offset, int end)
{
    if (offset != end) {
	error("DECODE_ERROR");
    }
    ::create();
}
