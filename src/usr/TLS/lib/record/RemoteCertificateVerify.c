# include <String.h>
# include "Record.h"
# include "Extension.h"

inherit CertificateVerify;


/*
 * initialize CertificateVerify from a blob
 */
static void create(String blob, int offset, int end)
{
    string algorithm, signature;

    algorithm = substring(blob, offset, offset + 1);
    ({ signature, offset }) = len2Restore(blob, offset + 2);
    if (offset != end) {
	error("Decode error");
    }
    ::create(algorithm, signature);
}
