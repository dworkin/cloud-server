# include <kfun.h>
# include "asn1.h"
# include "x509.h"
# include "Record.h"
# include "Extension.h"
# include "tls.h"


static void create()
{
# ifdef KF_SECURE_RANDOM
    compile_object("sys/explode2");
    compile_object(OBJECT_PATH(Asn1Der));
    compile_object(OBJECT_PATH(X509Certificate));
    compile_object(OBJECT_PATH(X509Key));
    compile_object(OBJECT_PATH(TlsClientSession));
    compile_object(OBJECT_PATH(TlsServerSession));
    compile_object(OBJECT_PATH(RemoteRecord));
    compile_object(OBJECT_PATH(RemoteAlert));
    compile_object(OBJECT_PATH(RemoteHandshake));
    compile_object(OBJECT_PATH(RemoteClientHello));
    compile_object(OBJECT_PATH(RemoteServerHello));
    compile_object(OBJECT_PATH(RemoteNewSessionTicket));
    compile_object(OBJECT_PATH(RemoteExtensions));
    compile_object(OBJECT_PATH(RemoteCertificates));
    compile_object(OBJECT_PATH(RemoteCertificateRequest));
    compile_object(OBJECT_PATH(RemoteCertificateVerify));
    compile_object(OBJECT_PATH(RemoteFinished));
    compile_object(OBJECT_PATH(RemoteKeyUpdate));
    compile_object(OBJECT_PATH(RemoteExtension));
    compile_object(OBJECT_PATH(RemoteServerName));
    compile_object(OBJECT_PATH(RemoteSupportedGroups));
    compile_object(OBJECT_PATH(RemoteSignatureAlgorithms));
    compile_object(OBJECT_PATH(RemoteEarlyData));
    compile_object(OBJECT_PATH(RemoteSupportedVersions));
    compile_object(OBJECT_PATH(RemoteVersion));
    compile_object(OBJECT_PATH(RemoteCookie));
    compile_object(OBJECT_PATH(RemoteKeyShareClient));
    compile_object(OBJECT_PATH(RemoteKeyShareServer));
    compile_object(OBJECT_PATH(RemoteKeyShareRetry));
    compile_object(OBJECT_PATH(UnknownExtension));
# endif
}
