# include <String.h>
# include "Extension.h"

inherit Version;


/*
 * initialize Version from a blob
 */
static create(String blob, int offset, int end)
{
    if (offset != end - 2) {
	error("Decode error");
    }
    ::create(substring(blob, offset, end - 1));
}
