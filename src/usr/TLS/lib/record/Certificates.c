# include <String.h>
# include "Record.h"

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
    StringBuffer certbuf, buffer;
    int sz, i;

    certbuf = new StringBuffer;
    sz = sizeof(certificates);
    for (i = 0; i < sz; i++) {
	buffer = certificates[i][0]->buffer();
	certbuf->append(len3(buffer));
	certbuf->append(buffer);
	certbuf->append(extSave(certificates[i][1]));
    }
    buffer = new StringBuffer(len1Save(context));
    buffer->append(len3(certbuf));
    buffer->append(certbuf);

    return buffer;
}


string context()		{ return context; }
mixed **certificates()		{ return certificates; }
