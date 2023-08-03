# include <String.h>
# include "Record.h"
# include "Extension.h"

inherit ClientHello;


/*
 * initialize ClientHello from a blob
 */
static void create(String blob, int offset, int end)
{
    string version, random, sessionId, cipherSuites, compressionMethods;

    version = substring(blob, offset, offset + 1);
    offset += 2;
    random = substring(blob, offset, offset + 31);
    offset += 32;
    ({ sessionId, offset }) = len1Restore(blob, offset);
    ({ cipherSuites, offset }) = len2Restore(blob, offset);
    ({ compressionMethods, offset }) = len1Restore(blob, offset);

    ::create(random, sessionId, EXPLODE2->explode2(cipherSuites),
	     compressionMethods,
	     (offset != end) ? extRestore(blob, offset, end, TRUE) : ({ }),
	     version);
}
