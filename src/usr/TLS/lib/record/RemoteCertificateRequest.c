# include <String.h>
# include "Record.h"

inherit CertificateRequest;


/*
 * initialize CertificateRequest from a blob
 */
static void create(String blob, int offset, int end)
{
    string context;

    ({ context, offset }) = len1Restore(blob, offset);
    ::create(context, extRestore(blob, offset, end));
}
