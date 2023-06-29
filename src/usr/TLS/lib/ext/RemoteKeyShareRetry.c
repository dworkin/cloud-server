# include <String.h>
# include "Extension.h"

inherit KeyShareRetry;


static void create(String blob, int offset, int end)
{
    if (offset != end - 2) {
	error("Decode error");
    }
    ::create(substring(blob, offset, offset + 1));
}
