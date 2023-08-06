# include <String.h>
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

StringBuffer transport();	/* override inherited prototype */

/*
 * export as a blob
 */
StringBuffer transport()
{
    int sz, i;
    StringBuffer buffer;

    buffer = new StringBuffer(len1Save(context));
    sz = sizeof(certificates);
    for (i = 0; i < sz; i++) {
	buffer->append(len3Save(certificates[i][0]->buffer()));
	buffer->append(extSave(certificates[i][1]));
    }

    return buffer;
}


string context()		{ return context; }
mixed **certificates()		{ return certificates; }
