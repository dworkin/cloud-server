# include "Record.h"
# include "Extension.h"

inherit Data;


private string context;
private mixed **certificates;

static void create(string context, mixed **certificates)
{
    ::create(HANDSHAKE_CERTIFICATES);
    ::context = context;
    ::certificates = certificates;
}

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
