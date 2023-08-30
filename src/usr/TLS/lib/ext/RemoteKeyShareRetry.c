# include <String.h>
# include "Extension.h"

inherit KeyShareRetry;


/*
 * initialize KeyShare for HelloRetryRequest from a blob
 */
static void create(String blob, int offset, int end)
{
    if (offset != end - 2) {
	error("DECODE_ERROR");
    }
    ::create(substring(blob, offset, offset + 1));
}
