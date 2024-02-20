# include <String.h>
# include "Record.h"
# include "Extension.h"
# include "tls.h"

inherit "~/lib/tls";


# define STATE_INITIAL		0	/* initial state */
# define STATE_WAIT_SH		1	/* expecting ServerHello */
# define STATE_WAIT_SH2		2	/* expecting 2nd ServerHello */
# define STATE_WAIT_EE		3	/* expecting encrypted Extensions */
# define STATE_WAIT_CERT_CR	4	/* expecting Cert/CertRequest */
# define STATE_WAIT_CERT	5	/* expecting Cert */
# define STATE_WAIT_CV		6	/* expecting CertVerify */
# define STATE_WAIT_FINISHED	7	/* expecting Finished */
# define STATE_CONNECTED	8	/* connection established */

private int state;			/* client state */
private int inClosed, outClosed;	/* input/output closed */
private string warning;			/* last warning */
private string group;			/* keyshare group */
private string pubKey, privKey, param;	/* group parameters */
private string host;			/* remote hostname */
private int compatible;			/* middlebox compatible? */
private string sessionId;		/* backward compatible session ID */
private string cookie;			/* cookie from HelloRetryRequest */
private string *clientStack;		/* client certificate stack */
private string clientKey;		/* client certificate key */
private string serverCertificate;	/* server certificate */
private string cipherSuite;		/* cipher algorithm suite */
private string serverSecret;		/* server secret */
private string clientSecret;		/* client secret */
private string masterSecret;		/* master secret */
private int sendBytes;			/* encrypted bytes sent */

/*
 * initialize TLS client session
 */
static void create(varargs string certificate, string key)
{
    ::create();
    group = supportedGroups()[0];
    if (certificate) {
	({ clientStack, clientKey }) = certKey(certificate, key);
    }
}

/*
 * generate a KeyShare based on group
 */
static string *keyShare()
{
    ({ pubKey, privKey, param }) = keyGen(group);
    return ({ group, pubKey });
}

/*
 * prepare to send a ClientHello message (RFC 8446 section 4.1.2)
 */
static Handshake sendClientHello()
{
    string random;
    Extension *extensions;

    random = secure_random(32);
    sessionId = (compatible) ? asn_xor(random, "\xff") : "";
    extensions = ({	/* RFC 8446 section 4.1.1 */
	new Extension(EXT_SUPPORTED_VERSIONS,
		      new SupportedVersions(({ TLS_VERSION_13 }))),
	new Extension(EXT_SUPPORTED_GROUPS,
		      new SupportedGroups(supportedGroups())),
	new Extension(EXT_SIGNATURE_ALGORITHMS,
		      new SignatureAlgorithms(signatureAlgorithms())),
	new Extension(EXT_KEY_SHARE, new KeyShareClient(({ keyShare() })))
    });
    if (host) {
	extensions += ({
	    /* RFC 6066 section 3 */
	    new Extension(EXT_SERVER_NAME, new ServerName(host))
	});
    }
    if (cookie) {
	/* RFC 8446 section 4.2.2 */
	extensions += ({ new Extension(EXT_COOKIE, new Cookie(cookie)) });
    }

    return new Handshake(new ClientHello(random, sessionId, cipherSuites(),
					 "\0", extensions));
}

/*
 * prepare to send an empty list of certificates (RFC 8446 section 4.4.2)
 */
static Handshake sendCertificates()
{
    return new Handshake(new Certificates("", ({ })));
}

/*
 * prepare to send a Finished message (RFC 8446 section 4.4.4)
 */
static Handshake sendFinished()
{
    return new Handshake(new Finished(verifyData(clientSecret, cipherSuite)));
}

/*
 * process a ServerHello message (RFC 8446 section 4.1.3)
 */
static void receiveServerHello(ServerHello serverHello, StringBuffer output)
{
    string str, *keyShare, secret, key, IV;
    Extension *extensions;
    int version, i;

    alignedRecord();

    if (serverHello->version() != TLS_VERSION_12 ||
	serverHello->sessionId() != sessionId) {
	error("ILLEGAL_PARAMETER");
    }

    str = serverHello->cipherSuite();
    if (sizeof(cipherSuites() & ({ str })) == 0 ||
	serverHello->compressionMethod() != 0) {
	error("ILLEGAL_PARAMETER");
    }
    if (!cipherSuite) {
	cipherSuite = str;
    } else if (str != cipherSuite) {
	error("ILLEGAL_PARAMETER");	/* RFC 8446 section 4.1.4 */
    }

    extensions = serverHello->extensions();
    for (i = sizeof(extensions); --i >= 0; ) {
	switch (extensions[i]->type()) {
	case EXT_SUPPORTED_VERSIONS:
	    if (version || extensions[i]->data()->version() != TLS_VERSION_13) {
		error("ILLEGAL_PARAMETER");
	    }
	    version = TRUE;
	    break;

	case EXT_KEY_SHARE:
	    keyShare = extensions[i]->data()->keyShare();
	    if (keyShare[0] != group || !privKey) {
		error("ILLEGAL_PARAMETER");
	    }
	    secret = sharedSecret(group, keyShare[1], privKey, param);
	    group = pubKey = privKey = param = nil;
	    ({
		serverSecret,
		clientSecret,
		masterSecret
	    }) = handshakeSecrets(secret, cipherSuite);
	    ({ key, IV }) = keyIV(serverSecret, cipherSuite);
	    setReceiveKey(key, IV, cipherSuite);
	    ({ key, IV }) = keyIV(clientSecret, cipherSuite);
	    setSendKey(key, IV, cipherSuite);
	    break;

	default:
	    error("UNSUPPORTED_EXTENSION");
	}
    }
    if (!version || privKey) {
	error("MISSING_EXTENSION");
    }
}

/*
 * process a HelloRetryRequest message (RFC 8446 section 4.1.4)
 */
static void receiveHelloRetryRequest(ServerHello helloRetry,
				     StringBuffer output)
{
    string *keyShare, str, secret, key, IV;
    Extension *extensions;
    int version, i;

    if (cipherSuite) {
	error("UNEXPECTED_MESSAGE");	/* 2nd HelloRetryRequest */
    }

    if (helloRetry->version() != TLS_VERSION_12 ||
	helloRetry->sessionId() != sessionId) {
	error("ILLEGAL_PARAMETER");
    }

    cipherSuite = helloRetry->cipherSuite();
    if (sizeof(cipherSuites() & ({ cipherSuite })) == 0 ||
	helloRetry->compressionMethod() != 0) {
	error("ILLEGAL_PARAMETER");
    }

    extensions = helloRetry->extensions();
    for (i = sizeof(extensions); --i >= 0; ) {
	switch (extensions[i]->type()) {
	case EXT_SUPPORTED_VERSIONS:
	    if (version || extensions[i]->data()->version() != TLS_VERSION_13) {
		error("ILLEGAL_PARAMETER");
	    }
	    version = TRUE;
	    break;

	case EXT_KEY_SHARE:
	    str = extensions[i]->data()->group();
	    if (sizeof(supportedGroups() & ({ str })) == 0 ||
		str == group || !privKey) {
		error("ILLEGAL_PARAMETER");
	    }
	    group = str;
	    privKey = nil;
	    break;

	case EXT_COOKIE:
	    cookie = extensions[i]->data()->cookie();
	    break;

	default:
	    error("UNSUPPORTED_EXTENSION");
	}
    }
    if (!version || privKey) {
	error("MISSING_EXTENSION");
    }

    if (compatible) {
	/* RFC 8446 section D.4 */
	compatible = FALSE;
	sendChangeCipherSpec(output);
    }
    sendTranscribeData(output, sendClientHello());
}

/*
 * process an Extensions message (RFC 8446 section 4.3.1)
 */
static void receiveExtensions(Extensions extensions, StringBuffer output)
{
    Extension *list;
    int sz, i;

    list = extensions->extensions();
    for (sz = sizeof(list), i = 0; i < sz; i++) {
	switch (list[i]->type()) {
	case EXT_SUPPORTED_GROUPS:
	case EXT_SERVER_NAME:		/* RFC 6066 section 3 */
	    break;

	default:
	    error("ILLEGAL_PARAMETER");
	}
    }
}

/*
 * process a CertificateRequest message (RFC 8446 section 4.3.2)
 */
static void receiveCertificateRequest(CertificateRequest request,
				      StringBuffer output)
{
    if (compatible) {
	/* RFC 8446 section D.4 */
	compatible = FALSE;
	sendChangeCipherSpec(output);
    }

    sendTranscribeData(output, sendCertificates());
}

/*
 * process a Certificates message (RFC 8446 section 4.4.2)
 */
static void receiveCertificates(Certificates certificates, StringBuffer output)
{
    mixed *certs;
    int sz, i;
    string *stack;

    if (strlen(certificates->context()) != 0) {
	error("ILLEGAL_PARAMETER");
    }

    certs = certificates->certificates();
    sz = sizeof(certs);
    if (sz == 0) {
	error("DECODE_ERROR");
    }
    stack = allocate(sz);
    for (i = 0; i < sz; i++) {
	if (certs[i][0]->length() > 0xffff) {
	    error("UNSUPPORTED_CERTIFICATE");	/* alas */
	}
	if (sizeof(certs[i][1]) != 0) {
	    error("UNSUPPORTED_EXTENSION");
	}
	stack[i] = certs[i][0]->buffer()->chunk();
    }
    checkCertificates("TLS server", stack);
    serverCertificate = stack[0];
}

/*
 * process a CertificateVerify message (RFC 8446 section 4.4.3)
 */
static void receiveCertificateVerify(CertificateVerify verify,
				     StringBuffer output)
{
    certificateVerify("server", serverCertificate, verify->signature(),
		      verify->algorithm(), cipherSuite);
}

/*
 * process a Finished message (RFC 8446 section 4.4.4)
 */
static void receiveFinished(Finished verify, StringBuffer output)
{
    string key, IV;

    alignedRecord();

    if (compatible) {
	/* RFC 8446 section D.4 */
	sendChangeCipherSpec(output);
    }

    sendData(output, sendFinished());

    ({
	serverSecret,
	clientSecret
    }) = applicationSecrets(masterSecret, cipherSuite);
    masterSecret = nil;
    ({ key, IV }) = keyIV(serverSecret, cipherSuite);
    setReceiveKey(key, IV, cipherSuite);
    ({ key, IV }) = keyIV(clientSecret, cipherSuite);
    setSendKey(key, IV, cipherSuite);
}

/*
 * process a NewSessionTicket message (RFC 8446 section 4.6.1)
 */
static void receiveNewSessionTicket(NewSessionTicket ticket,
				    StringBuffer output)
{
}

/*
 * process a KeyUpdate message (RFC 8446 section 4.6.3)
 */
static void receiveKeyUpdate(KeyUpdate keyUpdate, StringBuffer output)
{
    string key, IV;

    alignedRecord();

    serverSecret = updateSecret(serverSecret, cipherSuite);
    ({ key, IV }) = keyIV(serverSecret, cipherSuite);
    setReceiveKey(key, IV, cipherSuite);

    switch (keyUpdate->updateRequested()) {
    case FALSE:
	break;

    case TRUE:
	sendBytes = 0;
	sendData(output, new KeyUpdate(FALSE));
	clientSecret = updateSecret(clientSecret, cipherSuite);
	({ key, IV }) = keyIV(clientSecret, cipherSuite);
	setSendKey(key, IV, cipherSuite);
	break;

    default:
	error("ILLEGAL_PARAMETER");
    }
}


/*
 * establish a client TLS connection
 */
StringBuffer connect(int compatible, varargs string host)
{
    Handshake clientHello;
    string str;

    if (state != STATE_INITIAL) {
	error("Already connected");
    }
    ::compatible = compatible;
    ::host = host;
    clientHello = sendClientHello();
    str = clientHello->transport();
    transcribe(str);
    state = STATE_WAIT_SH;

    /* RFC 8446 section 5.1 */
    str = new Record(clientHello->type(), str, TLS_VERSION_10)->transport();
    return new StringBuffer(str);
}

/*
 * receive an encrypted message
 */
mixed *receiveMessage(string str)
{
    string status;
    StringBuffer input, output;
    Record *records, record;
    Data *dataList, data, message;
    int rsize, i, dsize, j;

    if (inClosed) {
	error("Connection closed");
    }
    if (state != STATE_CONNECTED) {
	status = "connecting";
    }
    input = new StringBuffer;
    output = new StringBuffer;
    try {
	records = ::receiveMessage(str);
	for (rsize = sizeof(records), i = 0; i < rsize; i++) {
	    dataList = ::receiveRecord(records[i]);
	    for (dsize = sizeof(dataList), j = 0; j < dsize; j++) {
		data = dataList[j];
		if (data->type() == RECORD_CHANGE_CIPHER_SPEC) {
		    if (state == STATE_CONNECTED) {
			error("UNEXPECTED_MESSAGE");
		    }
		    continue;
		}

		if (data->type() == RECORD_ALERT) {
		    /* RFC 8446 section 6 */
		    if (data->level() == ALERT_WARNING) {
			if (data->description() == ALERT_CLOSE_NOTIFY) {
			    status = "EOF";
			    inClosed = TRUE;
			} else {
			    warning = alertString(data->description());
			    continue;
			}
		    } else {
			status = "Remote " + alertString(data->description());
			inClosed = outClosed = TRUE;
		    }

		    return ({
			(input->length() != 0) ? input : nil,
			(output->length() != 0) ? output : nil,
			warning,
			status
		    });
		}

		switch (state) {
		case STATE_WAIT_SH:
		    if (data->type() != RECORD_HANDSHAKE) {
			error("UNEXPECTED_MESSAGE");
		    }
		    message = data->message();
		    if (message->type() != HANDSHAKE_SERVER_HELLO) {
			error("UNEXPECTED_MESSAGE");
		    }
		    if (message->random() != HELLO_RETRY_REQUEST) {
			transcribe(data->transport());
			receiveServerHello(message, output);
			state = STATE_WAIT_EE;
		    } else {
			reTranscribe(message->cipherSuite());
			transcribe(data->transport());
			receiveHelloRetryRequest(message, output);
			state = STATE_WAIT_SH2;
		    }
		    continue;

		case STATE_WAIT_SH2:
		    if (data->type() != RECORD_HANDSHAKE) {
			error("UNEXPECTED_MESSAGE");
		    }
		    message = data->message();
		    if (message->type() != HANDSHAKE_SERVER_HELLO) {
			error("UNEXPECTED_MESSAGE");
		    }
		    if (message->random() == HELLO_RETRY_REQUEST) {
			error("UNEXPECTED_MESSAGE");
		    }
		    transcribe(data->transport());
		    receiveServerHello(message, output);
		    state = STATE_WAIT_EE;
		    continue;

		case STATE_WAIT_EE:
		    if (data->type() != RECORD_HANDSHAKE) {
			error("UNEXPECTED_MESSAGE");
		    }
		    message = data->message();
		    if (message->type() != HANDSHAKE_EXTENSIONS) {
			error("UNEXPECTED_MESSAGE");
		    }
		    receiveExtensions(message, output);
		    state = STATE_WAIT_CERT_CR;
		    break;

		case STATE_WAIT_CERT_CR:
		    if (data->type() != RECORD_HANDSHAKE) {
			error("UNEXPECTED_MESSAGE");
		    }
		    message = data->message();
		    switch (message->type()) {
		    case HANDSHAKE_CERTIFICATE_REQUEST:
			receiveCertificateRequest(message, output);
			state = STATE_WAIT_CERT;
			break;

		    case HANDSHAKE_CERTIFICATES:
			receiveCertificates(message, output);
			state = STATE_WAIT_CV;
			break;

		    default:
			error("UNEXPECTED_MESSAGE");
		    }
		    break;

		case STATE_WAIT_CERT:
		    if (data->type() != RECORD_HANDSHAKE) {
			error("UNEXPECTED_MESSAGE");
		    }
		    message = data->message();
		    if (message->type() != HANDSHAKE_CERTIFICATES) {
			error("UNEXPECTED_MESSAGE");
		    }
		    receiveCertificates(message, output);
		    state = STATE_WAIT_CV;
		    break;

		case STATE_WAIT_CV:
		    if (data->type() != RECORD_HANDSHAKE) {
			error("UNEXPECTED_MESSAGE");
		    }
		    message = data->message();
		    if (message->type() != HANDSHAKE_CERTIFICATE_VERIFY) {
			error("UNEXPECTED_MESSAGE");
		    }
		    receiveCertificateVerify(message, output);
		    state = STATE_WAIT_FINISHED;
		    break;

		case STATE_WAIT_FINISHED:
		    if (data->type() != RECORD_HANDSHAKE) {
			error("UNEXPECTED_MESSAGE");
		    }
		    str = verifyData(serverSecret, cipherSuite);
		    message = data->message();
		    if (message->type() != HANDSHAKE_FINISHED) {
			error("UNEXPECTED_MESSAGE");
		    }
		    if (str != message->hash()) {
			error("DECRYPT_ERROR");
		    }
		    transcribe(data->transport());
		    receiveFinished(message, output);
		    status = nil;
		    state = STATE_CONNECTED;
		    continue;

		case STATE_CONNECTED:
		    switch (data->type()) {
		    case RECORD_HANDSHAKE:
			message = data->message();
			switch (message->type()) {
			case HANDSHAKE_NEW_SESSION_TICKET:
			    receiveNewSessionTicket(message, output);
			    break;

			case HANDSHAKE_KEY_UPDATE:
			    receiveKeyUpdate(message, output);
			    break;

			default:
			    /* no post_handshake_auth */
			    error("UNEXPECTED_MESSAGE");
			    break;
			}
			break;

		    case RECORD_APPLICATION_DATA:
			input->append(data->payload());
			break;

		    default:
			error("UNEXPECTED_MESSAGE");
		    }
		    continue;
		}

		transcribe(data->transport());
	    }
	}
    } catch (err) {
	int desc;

	desc = alertDescription(err);
	sendData(output, new Alert(ALERT_FATAL, desc));
	status = (desc == ALERT_INTERNAL_ERROR) ? "INTERNAL_ERROR" : err;
	inClosed = outClosed = TRUE;
    }

    return ({
	(input->length() != 0) ? input : nil,
	(output->length() != 0) ? output : nil,
	warning,
	status
    });
}

/*
 * encrypt and send a message
 */
StringBuffer sendMessage(StringBuffer str)
{
    StringBuffer output;
    string chunk, key, IV;

    if (outClosed) {
	error("Connection closed");
    }
    if (state != STATE_CONNECTED) {
	error("Connection not established");
    }

    if (str->length() > 16384) {
	str = new StringBuffer(str, 16384);	/* RFC 8446 section 5.1 */
    }
    output = new StringBuffer;
    while ((chunk=str->chunk())) {
	if (sendBytes >= SEND_LIMIT) {
	    /* RFC 8446 section 4.6.3 */
	    sendBytes = 0;
	    sendData(output, new KeyUpdate(FALSE));
	    clientSecret = updateSecret(clientSecret, cipherSuite);
	    ({ key, IV }) = keyIV(clientSecret, cipherSuite);
	    setSendKey(key, IV, cipherSuite);
	}
	sendBytes += ::sendMessage(output, chunk);
    }
    return output;
}

/*
 * close connection
 */
StringBuffer close()
{
    StringBuffer output;

    if (state == STATE_INITIAL) {
	error("Unconnected");
    }

    if (!outClosed) {
	output = new StringBuffer;
	if (state != STATE_CONNECTED) {
	    sendData(output, new Alert(ALERT_WARNING, ALERT_USER_CANCELED));
	}
	sendData(output, new Alert(ALERT_WARNING, ALERT_CLOSE_NOTIFY));
	outClosed = TRUE;
    }

    return output;
}
