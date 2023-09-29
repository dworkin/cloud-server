# include <String.h>
# include "Record.h"

inherit Certificates;


/*
 * initialize Certificates from a blob
 */
static void create(String blob, int offset, int end)
{
    string context;
    String certificate;
    mixed **certificates;
    int next;

    ({ context, offset }) = len1Restore(blob, offset);
    certificates = ({ });
    if (len3Offset(blob, offset) != end) {
	error("DECODE_ERROR");
    }
    offset += 3;
    while (offset < end) {
	({ certificate, offset }) = len3Restore(blob, offset);
	next = len2Offset(blob, offset);
	certificates += ({
	    ({ certificate, extRestore(blob, offset, next) })
	});
	offset = next;
    }
    if (offset != end) {
	error("DECODE_ERROR");
    }
    ::create(context, certificates);
}
