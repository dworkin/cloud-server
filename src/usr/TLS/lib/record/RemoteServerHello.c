# include <String.h>
# include "Record.h"
# include "Extension.h"

inherit ServerHello;


/*
 * initialize ServerHello from a blob
 */
static void create(String blob, int offset, int end)
{
    string version, random, sessionId, cipherSuite;
    int compressionMethod;

    version = substring(blob, offset, offset + 1);
    offset += 2;
    random = substring(blob, offset, offset + 31);
    offset += 32;
    ({ sessionId, offset }) = len1Restore(blob, offset);
    cipherSuite = substring(blob, offset, offset + 1);
    offset += 2;
    compressionMethod = blob[offset++];

    ::create(random, sessionId, cipherSuite, compressionMethod,
	     extRestore(blob, offset, end), version);
}
