# include <String.h>
# include "Record.h"
# include "Extension.h"

inherit Certificates;


static void create(String blob, int offset, int end)
{
    string context, certificate;
    mixed **certificates;
    int extEnd;

    ({ context, offset }) = len1Restore(blob, offset);
    certificates = ({ });
    if (len3Offset(blob, offset) != end) {
	error("Decode error");
    }
    offset += 3;
    while (offset < end) {
	({ certificate, offset }) = len3Restore(blob, offset);
	extEnd = len2Offset(blob, offset);
	certificates += ({
	    ({ certificate, extRestore(blob, offset, extEnd) })
	});
	offset = extEnd;
    }
    if (offset != end) {
	error("Decode error");
    }
    ::create(context, certificates);
}
