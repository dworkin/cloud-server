# include <String.h>
# include "Record.h"
# include "Extension.h"

inherit Extension;


static void create(String blob, int offset, int end, varargs int client)
{
    int type;
    Data data;

    type = (blob[offset] << 8) | blob[offset + 1];
    offset += 4;
    switch (type) {
    case EXT_SERVER_NAME:
	data = new RemoteServerName(blob, offset, end);
	break;

    case EXT_SUPPORTED_GROUPS:
	data = new RemoteSupportedGroups(blob, offset, end);
	break;

    case EXT_SIGNATURE_ALGORITHMS:
	data = new RemoteSignatureAlgorithms(blob, offset, end);
	break;

    case EXT_SUPPORTED_VERSIONS:
	data = (client) ?
		new RemoteSupportedVersions(blob, offset, end) :
		new RemoteVersion(blob, offset, end);
	break;

    case EXT_COOKIE:
	data = new RemoteCookie(blob, offset, end);
	break;

    case EXT_SIGNATURE_ALGORITHMS_CERT:
	data = new RemoteSignatureAlgorithms(blob, offset, end);
	break;

    case EXT_KEY_SHARE:
	data = (client) ?
		new RemoteKeyShareClient(blob, offset, end) :
		(offset != end - 2) ?
		 new RemoteKeyShareServer(blob, offset, end) :
		 new RemoteKeyShareRetry(blob, offset, end);
	break;

    default:
	data = new UnknownExtension(blob, offset, end);
	break;
    }

    ::create(type, data);
}
