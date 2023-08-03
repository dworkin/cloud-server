# include <String.h>
# include "Record.h"

inherit Handshake;


/*
 * initialize Handshake from a blob
 */
static create(String blob, int offset, int end)
{
    int type;
    Data data;

    type = blob[offset];
    switch (type) {
    case HANDSHAKE_CLIENT_HELLO:
	data = new RemoteClientHello(blob, offset + 4, end);
	break;

    case HANDSHAKE_SERVER_HELLO:
	data = new RemoteServerHello(blob, offset + 4, end);
	break;

    case HANDSHAKE_NEW_SESSION_TICKET:
	data = new RemoteNewSessionTicket(blob, offset + 4, end);
	break;

    case HANDSHAKE_END_OF_EARLY_DATA:
	if (offset + 4 != end) {
	    error("Decode error");
	}
	break;

    case HANDSHAKE_EXTENSIONS:
	data = new RemoteExtensions(blob, offset + 4, end);
	break;

    case HANDSHAKE_CERTIFICATES:
	data = new RemoteCertificates(blob, offset + 4, end);
	break;

    case HANDSHAKE_CERTIFICATE_REQUEST:
	data = new RemoteCertificateRequest(blob, offset + 4, end);
	break;

    case HANDSHAKE_CERTIFICATE_VERIFY:
	data = new RemoteCertificateVerify(blob, offset + 4, end);
	break;

    case HANDSHAKE_FINISHED:
	data = new RemoteFinished(blob, offset + 4, end);
	break;

    case HANDSHAKE_KEY_UPDATE:
	data = new RemoteKeyUpdate(blob, offset + 4, end);
	break;

    default:
	error("Unimplemented");
    }
    ::create(data);
}
