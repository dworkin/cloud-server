# include "Record.h"
# include "Extension.h"

inherit Data;


private string context;		/* Certificate Request identifier */
private mixed **certificates;	/* list of ({ certificate, extensions }) */

/*
 * initialize Certificates
 */
static void create(string context, mixed **certificates)
{
    ::create(HANDSHAKE_CERTIFICATES);
    ::context = context;
    ::certificates = certificates;
}

/*
 * export as a blob
 */
string transport()
{
    int sz, i, j;
    string *certs;

    sz = sizeof(certificates);
    certs = allocate(2 * sz);
    for (i = 0, j = 0; i < sz; i++) {
	certs[j++] = len3Save(certificates[i][0]);
	certs[j++] = extSave(certificates[i][1]);
    }

    return len1Save(context) +
	   len3Save(implode(certs, ""));
}


string context()		{ return context; }
mixed **certificates()		{ return certificates; }
