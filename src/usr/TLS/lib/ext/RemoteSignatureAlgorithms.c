# include <String.h>
# include "Extension.h"

inherit SignatureAlgorithms;


static void create(String blob, int offset, int end)
{
    string algorithms;

    ({ algorithms, offset }) = len2Restore(blob, offset);
    if (offset != end) {
	error("Decode error");
    }
    ::create(algorithms);
}
