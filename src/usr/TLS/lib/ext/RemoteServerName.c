# include <String.h>
# include "Extension.h"

inherit ServerName;


static void create(String blob, int offset, int end)
{
    string *names, name;

    if (len2Offset(blob, offset) != end) {
	error("Decode error");
    }

    offset += 2;
    names = ({ });
    while (offset < end) {
	if (blob[offset] != '\0') {
	    error("Decode error");
	}
	({ name, offset }) = len2Restore(blob, offset + 1);
	names += ({ name });
    }
    if (offset != end) {
	error("Decode error");
    }

    ::create(names);
}
