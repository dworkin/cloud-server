# include <String.h>
# include "Record.h"

inherit KeyUpdate;


static void create(String blob, int offset, int end)
{
    if (offset + 1 != end) {
	error("Decode error");
    }
    ::create(blob[offset]);
}
