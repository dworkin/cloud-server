# include "Record.h"
# include "Extension.h"

inherit Data;


private string context;			/* request identifier */
private Extensions *extensions;		/* required: SignatureAlgorithms */

/*
 * initialize CertificateRequest
 */
static void create(string context, Extension *extensions)
{
    ::create(HANDSHAKE_CERTIFICATE_REQUEST);
    ::context = context;
    ::extensions = extensions;
}

/*
 * export as a blob
 */
string transport()
{
    return len1Save(context) +
	   extSave(extensions);
}


string context()		{ return context; }
Extension *extensions()		{ return extensions; }
