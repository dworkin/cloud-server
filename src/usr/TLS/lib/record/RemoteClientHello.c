# include <String.h>
# include "Record.h"
# include "Extension.h"

inherit ClientHello;


static void create(String blob, int offset, int end)
{
    string version, random, sessionId, cipherSuite, compressionMethods;

    version = substring(blob, offset, offset + 1);
    offset += 2;
    random = substring(blob, offset, offset + 31);
    offset += 32;
    ({ sessionId, offset }) = len1Restore(blob, offset);
    ({ cipherSuite, offset }) = len2Restore(blob, offset);
    ({ compressionMethods, offset }) = len1Restore(blob, offset);

    ::create(random, sessionId, cipherSuite, compressionMethods,
	     (offset != end) ? extRestore(blob, offset, end, TRUE) : ({ }),
	     version);
}
