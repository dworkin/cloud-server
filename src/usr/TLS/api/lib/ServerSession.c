# include <String.h>
# include "Record.h"
# include "Extension.h"
# include "tls.h"

inherit "~/lib/tls";
private inherit "/lib/util/ascii";


# define STATE_INITIAL		0	/* initial state */
# define STATE_WAIT_CH		1	/* expecting ClientHello */
# define STATE_WAIT_CERT	2	/* expecting Certificates */
# define STATE_WAIT_CV		3	/* expecting CertificateVerify */
# define STATE_WAIT_FINISHED	4	/* expecting Finished */
# define STATE_CONNECTED	5	/* connection established */

private int state;			/* client state */
private int inClosed, outClosed;	/* input/output closed */
private string warning;			/* last warning */
private string group;			/* keyshare group */
private string pubKey, privKey, param;	/* group parameters */
private string *hosts;			/* server hostnames */
private string host;			/* chosen host */
private int compatible;			/* middlebox compatible? */
private string sessionId;		/* backward compatible session ID */
private string *serverStack;		/* certificate stack */
private string serverKey;		/* certificate key */
private string clientCertificate;	/* client certificate */
private string *signatureAlgorithms;	/* client signature algorithms */
private string *certificateAlgorithms;	/* client certificate algorithms */
private string cipherSuite;		/* cipher algorithm suite */
private string serverSecret;		/* server secret */
private string clientSecret;		/* client secret */
private string masterSecret;		/* master secret */
private int sendBytes;			/* encrypted bytes sent */

/*
 * initialize TLS server session
 */
static void create(string certificate, string key)
{
    ::create();
    ({ serverStack, serverKey }) = certKey(certificate, key);
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
 * prepare to send a ServerHello message (RFC 8446 section 4.1.3)
 */
static Handshake sendServerHello()
{
    Extension *extensions;

    extensions = ({
	new Extension(EXT_SUPPORTED_VERSIONS, new Version(TLS_VERSION_13)),
	new Extension(EXT_KEY_SHARE, new KeyShareServer(keyShare()))
    });

    return new Handshake(new ServerHello(secure_random(32), sessionId,
					 cipherSuite, 0, extensions));
}

/*
 * prepare to send a HelloRetryRequest message (RFC 8446 section 4.1.4)
 */
static Handshake sendHelloRetryRequest()
{
    Extension *extensions;

    extensions = ({
	new Extension(EXT_SUPPORTED_VERSIONS, new Version(TLS_VERSION_13)),
	new Extension(EXT_KEY_SHARE, new KeyShareRetry(group))
    });

    return new Handshake(new ServerHello(HELLO_RETRY_REQUEST, sessionId,
					 cipherSuite, 0, extensions));
}

/*
 * prepare to send Extensions (RFC 8446 section 4.3.1)
 */
static Handshake sendExtensions(int named)
{
    Extension *extensions;

    extensions = ({
	new Extension(EXT_SUPPORTED_GROUPS,
		      new SupportedGroups(supportedGroups()))
    });
    if (named) {
	extensions += ({
	    /* RFC 6066 section 3 */
	    new Extension(EXT_SERVER_NAME, new ServerName(nil))
	});
    }

    return new Handshake(new Extensions(extensions));
}

/*
 * prepare to send Certificates (RFC 8446 section 4.4.2)
 */
static Handshake sendCertificates()
{
    mixed **certificates;
    int sz, i;

    sz = sizeof(serverStack);
    certificates = allocate(sz);
    for (i = 0; i < sz; i++) {
	certificates[i] = ({ new String(serverStack[i]), ({ }) });
    }

    return new Handshake(new Certificates("", certificates));
}

/*
 * prepare to send CertificateVerify (RFC 8446 section 4.4.3)
 */
static Handshake sendCertificateVerify()
{
    string scheme, signature;

    ({
	scheme,
	signature
    }) = certificateSign("server", serverKey, certificateAlgorithms,
			 cipherSuite);
    return new Handshake(new CertificateVerify(scheme, signature));
}

/*
 * prepare to send a CertificateRequest message (RFC 8446 section 4.3.2)
 */
static Handshake sendCertificateRequest()
{
}

/*
 * prepare to send a Finished message (RFC 8446 section 4.4.4)
 */
static Handshake sendFinished()
{
    return new Handshake(new Finished(verifyData(serverSecret, cipherSuite)));
}

/*
 * process a ClientHello message (RFC 8446 section 4.1.2)
 */
static int receiveClientHello(ClientHello clientHello, StringBuffer output)
{
    string *strs, *groups, **shares, key, IV, secret;
    int sz, i, version, nshares, j;
    Extension *extensions;

    alignedRecord();

    if (clientHello->version() != TLS_VERSION_12) {
	error("PROTOCOL_VERSION");
    }
    sessionId = clientHello->sessionId();
    if (strlen(sessionId) != 0) {
	compatible = TRUE;
    }
    strs = clientHello->cipherSuites() & cipherSuites();
    if (sizeof(strs) == 0) {
	error("HANDSHAKE_FAILURE");
    }
    cipherSuite = strs[0];
    if (clientHello->compressionMethods() != "\0") {
	error("ILLEGAL_PARAMETER");
    }
    extensions = clientHello->extensions();
    for (sz = sizeof(extensions), i = 0; i < sz; i++) {
	switch (extensions[i]->type()) {
	case EXT_SUPPORTED_VERSIONS:
	    if (version) {
		error("ILLEGAL_PARAMETER");
	    }
	    strs = extensions[i]->data()->versions() & ({ TLS_VERSION_13 });
	    if (sizeof(strs) == 0) {
		error("PROTOCOL_VERSION");
	    }
	    version = TRUE;
	    break;

	case EXT_SERVER_NAME:
	    if (host) {
		error("ILLEGAL_PARAMETER");
	    }
	    host = extensions[i]->data()->hostName();
	    if (hosts && sizeof(hosts & ({ lower_case(host) })) == 0) {
		error("UNRECOGNIZED_NAME");
	    }
	    break;

	case EXT_SUPPORTED_GROUPS:
	    if (groups) {
		error("ILLEGAL_PARAMETER");
	    }
	    groups = extensions[i]->data()->groups();
	    if (sizeof(supportedGroups() & groups) == 0) {
		error("HANDSHAKE_FAILURE");
	    }
	    break;

	case EXT_SIGNATURE_ALGORITHMS:
	    if (signatureAlgorithms) {
		error("ILLEGAL_PARAMETER");
	    }
	    signatureAlgorithms = extensions[i]->data()->algorithms();
	    break;

	case EXT_SIGNATURE_ALGORITHMS_CERT:
	    if (certificateAlgorithms) {
		error("ILLEGAL_PARAMETER");
	    }
	    certificateAlgorithms = extensions[i]->data()->algorithms();
	    break;

	case EXT_KEY_SHARE:
	    if (shares) {
		error("ILLEGAL_PARAMETER");
	    }
	    shares = extensions[i]->data()->keyShares();
	    for (nshares = sizeof(shares), j = 0; j < nshares; j++) {
		({ group, key }) = shares[j];
		if (sizeof(supportedGroups() & ({ group })) != 0) {
		    break;
		}
		key = nil;
	    }
	    break;

	case EXT_EARLY_DATA:
	    /*
	     * RFC 8446 section 4.6.1
	     * Let's just say that max_early_data_size is 0.
	     */
	    error("UNEXPECTED_MESSAGE");
	}
    }

    if (!version) {
	error("PROTOCOL_VERSION");		/* don't support TLS 1.2 */
    }
    if (hosts && !host) {
	error("MISSING_EXTENSION");		/* RFC 6066 section 3 */
    }

    if (!groups) {
	groups = supportedGroups();
    } else {
	if (key && sizeof(groups & ({ group })) == 0) {
	    error("ILLEGAL_PARAMETER");
	}

	groups = supportedGroups() & groups;	/* server's ordering */
	if (sizeof(groups) == 0) {
	    error("HANDSHAKE_FAILURE");
	}
    }

    if (!signatureAlgorithms) {
	error("MISSING_EXTENSION");		/* RFC 8446 section 4.2.3 */
    }
    if (!certificateAlgorithms) {
	certificateAlgorithms = signatureAlgorithms;
    }

    if (key) {
	sendTranscribeData(output, sendServerHello());
	if (compatible) {
	    sendChangeCipherSpec(output);	/* RFC 8446 section D.4 */
	}

	secret = sharedSecret(group, key, privKey, param);
	group = pubKey = privKey = param = nil;
	({
	    serverSecret,
	    clientSecret,
	    masterSecret
	}) = handshakeSecrets(secret, cipherSuite);
	({ key, IV }) = keyIV(serverSecret, cipherSuite);
	setSendKey(key, IV, cipherSuite);
	({ key, IV }) = keyIV(clientSecret, cipherSuite);
	setReceiveKey(key, IV, cipherSuite);

	sendTranscribeData(output, sendExtensions(hosts && host));
	sendTranscribeData(output, sendCertificates());
	sendTranscribeData(output, sendCertificateVerify());
	sendTranscribeData(output, sendFinished());

	return TRUE;
    } else {
	group = groups[0];
	signatureAlgorithms = nil;
	certificateAlgorithms = nil;
	reTranscribe(cipherSuite);
	sendTranscribeData(output, sendHelloRetryRequest());
	if (compatible) {
	    sendChangeCipherSpec(output);	/* RFC 8446 section D.4 */
	}
	return FALSE;
    }
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
    checkCertificates("TLS client", stack);
    clientCertificate = stack[0];
}

/*
 * process a CertificateVerify message (RFC 8446 section 4.4.3)
 */
static void receiveCertificateVerify(CertificateVerify verify,
				     StringBuffer output)
{
    certificateVerify("client", clientCertificate, verify->signature(),
		      verify->algorithm(), cipherSuite);
}

/*
 * process a Finished message (RFC 8446 section 4.4.4)
 */
static void receiveFinished(Finished verify, StringBuffer output)
{
    string key, IV;

    alignedRecord();

    ({
	serverSecret,
	clientSecret
    }) = applicationSecrets(masterSecret, cipherSuite);
    masterSecret = nil;
    ({ key, IV }) = keyIV(serverSecret, cipherSuite);
    setSendKey(key, IV, cipherSuite);
    ({ key, IV }) = keyIV(clientSecret, cipherSuite);
    setReceiveKey(key, IV, cipherSuite);
}

/*
 * process a KeyUpdate message (RFC 8446 section 4.6.3)
 */
static void receiveKeyUpdate(KeyUpdate keyUpdate, StringBuffer output)
{
    string key, IV;

    alignedRecord();

    clientSecret = updateSecret(clientSecret, cipherSuite);
    ({ key, IV }) = keyIV(clientSecret, cipherSuite);
    setReceiveKey(key, IV, cipherSuite);

    switch (keyUpdate->updateRequested()) {
    case FALSE:
	break;

    case TRUE:
	sendBytes = 0;
	sendData(output, new KeyUpdate(FALSE));
	serverSecret = updateSecret(serverSecret, cipherSuite);
	({ key, IV }) = keyIV(serverSecret, cipherSuite);
	setSendKey(key, IV, cipherSuite);
	break;

    default:
	error("ILLEGAL_PARAMETER");
    }
}


/*
 * accept a TLS connection
 */
void accept(int reqCert, string hosts...)
{
    if (state != STATE_INITIAL) {
	error("Already connected");
    }
    if (sizeof(hosts) != 0) {
	::hosts = hosts;
    }
    state = STATE_WAIT_CH;
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
		case STATE_WAIT_CH:
		    if (data->type() != RECORD_HANDSHAKE) {
			error("UNEXPECTED_MESSAGE");
		    }
		    message = data->message();
		    if (message->type() != HANDSHAKE_CLIENT_HELLO) {
			error("UNEXPECTED_MESSAGE");
		    }
		    transcribe(data->transport());
		    if (receiveClientHello(message, output)) {
			state = STATE_WAIT_FINISHED;
		    }
		    continue;

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
		    str = verifyData(clientSecret, cipherSuite);
		    message = data->message();
		    if (message->type() != HANDSHAKE_FINISHED) {
			error("UNEXPECTED_MESSAGE");
		    }
		    if (str != message->hash()) {
			error("DECRYPT_ERROR");
		    }
		    receiveFinished(message, output);
		    status = nil;
		    state = STATE_CONNECTED;
		    continue;

		case STATE_CONNECTED:
		    switch (data->type()) {
		    case RECORD_HANDSHAKE:
			message = data->message();
			if (message->type() != HANDSHAKE_KEY_UPDATE) {
			    error("UNEXPECTED_MESSAGE");
			}
			receiveKeyUpdate(message, output);
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
	    serverSecret = updateSecret(serverSecret, cipherSuite);
	    ({ key, IV }) = keyIV(serverSecret, cipherSuite);
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


string host()		{ return host; }
