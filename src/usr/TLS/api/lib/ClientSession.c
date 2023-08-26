# include <String.h>
# include "Record.h"
# include "Extension.h"
# include "tls.h"
# include <type.h>

inherit "~/lib/tls";
inherit "~/lib/ffdhe";


# define STATE_INITIAL		0	/* initial state */
# define STATE_WAIT_SH		1	/* expecting ServerHello */
# define STATE_WAIT_EE		2	/* expecting encrypted Extensions */
# define STATE_WAIT_CERT_CR	3	/* expecting Cert/CertRequest */
# define STATE_WAIT_CERT	4	/* expecting Cert */
# define STATE_WAIT_CV		5	/* expecting CertVerify */
# define STATE_WAIT_FINISHED	6	/* expecting Finished */
# define STATE_CONNECTED	7	/* connection established */

private int state;			/* client state */
private string prime, pubKey, privKey;	/* FFDHE parameters */
private string *messages;		/* transcript hash messages */
private int compatible;			/* middlebox compatible? */
private string cipherSuite;		/* cipher algorithm suite */
private string serverSecret;		/* server secret */
private string clientSecret;		/* client secret */
private string masterSecret;		/* master secret */

/*
 * initialize TLS client session
 */
static void create(varargs string certificate, string key)
{
    ::create();
    prime = ffdhe3072p();
    ({ pubKey, privKey }) = keyPair(prime);
    messages = ({ });
}

/*
 * transcribe a handshake message
 */
private void transcribe(mixed str)
{
    string chunk;

    if (typeof(str) == T_STRING) {
	messages += ({ str });
    } else {
	while ((chunk=str->chunk())) {
	    messages += ({ chunk });
	}
    }
}

/*
 * prepare to send a ClientHello message
 */
private Handshake sendClientHello(string *hosts)
{
    string random, sessionId, *cipherSuites;
    Extension *extensions;

    random = secure_random(32);
    if (compatible) {
	sessionId = hash_string("SHA256", asn_xor(random, random_string(32)));
    } else {
	sessionId = "";
    }
    cipherSuites = ({
	TLS_AES_128_GCM_SHA256,
	TLS_AES_256_GCM_SHA384,
	TLS_CHACHA20_POLY1305_SHA256,
	TLS_AES_128_CCM_SHA256,
	TLS_AES_128_CCM_8_SHA256
    });
    extensions = ({
	new Extension(EXT_SUPPORTED_VERSIONS,
		      new SupportedVersions(({ TLS_VERSION_13 }))),
	new Extension(EXT_SUPPORTED_GROUPS, new SupportedGroups(({
	    TLS_FFDHE3072,
	    TLS_FFDHE4096,
	    TLS_FFDHE6144,
	    TLS_FFDHE8192,
	    TLS_FFDHE2048
	}))),
	new Extension(EXT_SIGNATURE_ALGORITHMS, new SignatureAlgorithms(({
	    TLS_RSA_PSS_RSAE_SHA256,
	    TLS_RSA_PSS_RSAE_SHA384,
	    TLS_RSA_PSS_RSAE_SHA512,
	    TLS_RSA_PSS_PSS_SHA256,
	    TLS_RSA_PSS_PSS_SHA384,
	    TLS_RSA_PSS_PSS_SHA512
	}))),
	new Extension(EXT_KEY_SHARE, new KeyShareClient(({
	    ({ TLS_FFDHE3072, pubKey })
	})))
    });
    if (sizeof(hosts) != 0) {
	extensions += ({
	    new Extension(EXT_SERVER_NAME, new ServerName(hosts))
	});
    }

    return new Handshake(new ClientHello(random, sessionId, cipherSuites, "\0",
					 extensions));
}

/*
 * prepare to send a Finished message
 */
private Handshake sendFinished()
{
    return new Handshake(new Finished(verifyData(clientSecret, messages,
						 cipherSuite)));
}

/*
 * process a ServerHello message
 */
private void receiveServerHello(ServerHello serverHello, StringBuffer output)
{
    string secret, key, IV;
    Extension *extensions;
    int i;

    if (!alignedRecord()) {
	error("Unaligned ServerHello");
    }
    cipherSuite = serverHello->cipherSuite();
    extensions = serverHello->extensions();
    for (i = sizeof(extensions); --i >= 0; ) {
	if (extensions[i]->type() == EXT_KEY_SHARE) {
	    secret = sharedSecret(extensions[i]->data()->keyShare()[1],
				  privKey,
				  prime);
	    privKey = nil;
	    ({
		serverSecret,
		clientSecret,
		masterSecret
	    }) = handshakeSecrets(secret, messages, cipherSuite);
	    ({ key, IV }) = keyIV(serverSecret, cipherSuite);
	    setReceiveKey(key, IV, cipherSuite);
	    ({ key, IV }) = keyIV(clientSecret, cipherSuite);
	    setSendKey(key, IV, cipherSuite);
	}
    }
    if (compatible) {
	sendChangeCipherSpec(output);
    }
}

/*
 * process an Extensions message
 */
private void receiveExtensions(Extensions extensions, StringBuffer output)
{
}

/*
 * process a CertificateRequest message
 */
private void receiveCertificateRequest(CertificateRequest request,
				       StringBuffer output)
{
}

/*
 * process a Certificates message
 */
private void receiveCertificates(Certificates certificates, StringBuffer output)
{
}

/*
 * process a CertificateVerify message
 */
private void receiveCertificateVerify(CertificateVerify verify,
				      StringBuffer output)
{
}

/*
 * process a Finished message
 */
private void receiveFinished(Finished verify, StringBuffer output)
{
    string key, IV;

    sendData(output, sendFinished());

    ({
	serverSecret,
	clientSecret
    }) = applicationSecrets(masterSecret, messages, cipherSuite);
    masterSecret = nil;
    ({ key, IV }) = keyIV(serverSecret, cipherSuite);
    setReceiveKey(key, IV, cipherSuite);
    ({ key, IV }) = keyIV(clientSecret, cipherSuite);
    setSendKey(key, IV, cipherSuite);
}

/*
 * process a NewSessionTicket message
 */
private void receiveNewSessionTicket(NewSessionTicket ticket,
				     StringBuffer output)
{
}

/*
 * process a KeyUpdate message
 */
private void receiveKeyUpdate(KeyUpdate keyUpdate, StringBuffer output)
{
}


/*
 * establish a client TLS connection
 */
StringBuffer connect(int compatible, string hosts...)
{
    Handshake clientHello;
    string str;

    if (state != STATE_INITIAL) {
	error("Already connected");
    }
    ::compatible = compatible;
    clientHello = sendClientHello(hosts);
    str = clientHello->transport();
    transcribe(str);
    state = STATE_WAIT_SH;
    str = new Record(clientHello->type(), str, TLS_VERSION_10)->transport();
    return new StringBuffer(str);
}

/*
 * receive an encrypted message
 */
mixed *receiveMessage(string str)
{
    string alert;
    StringBuffer input, output;
    Record *records, record;
    Data *dataList, data;
    int rsize, i, dsize, j;

    if (state != STATE_CONNECTED) {
	alert = "connecting";
    }
    input = new StringBuffer;
    output = new StringBuffer;
    records = ::receiveMessage(str);
    for (rsize = sizeof(records), i = 0; i < rsize; i++) {
	dataList = ::receiveRecord(records[i]);
	for (dsize = sizeof(dataList), j = 0; j < dsize; j++) {
	    data = dataList[j];
	    switch (state) {
	    case STATE_WAIT_SH:
		if (data->type() != RECORD_HANDSHAKE) {
		    error("Handshake expected");
		}
		transcribe(data->transport());
		data = data->message();
		if (data->type() != HANDSHAKE_SERVER_HELLO) {
		    error("ServerHello expected");
		}
		receiveServerHello(data, output);
		state = STATE_WAIT_EE;
		break;

	    case STATE_WAIT_EE:
		if (data->type() == RECORD_CHANGE_CIPHER_SPEC &&
		    !data->decrypted() && strlen(str=data->payload()) == 1 &&
		    str[0] == '\1') {
		    break;
		}

		if (data->type() != RECORD_HANDSHAKE) {
		    error("Handshake expected");
		}
		transcribe(data->transport());
		data = data->message();
		if (data->type() != HANDSHAKE_EXTENSIONS) {
		    error("EncryptedExtensions expected " + data->type());
		}
		receiveExtensions(data, output);
		state = STATE_WAIT_CERT_CR;
		break;

	    case STATE_WAIT_CERT_CR:
		if (data->type() != RECORD_HANDSHAKE) {
		    error("Handshake expected");
		}
		transcribe(data->transport());
		data = data->message();
		switch (data->type()) {
		case HANDSHAKE_CERTIFICATE_REQUEST:
		    receiveCertificateRequest(data, output);
		    state = STATE_WAIT_CERT;
		    break;

		case HANDSHAKE_CERTIFICATES:
		    receiveCertificates(data, output);
		    state = STATE_WAIT_CV;
		    break;

		default:
		    error("CertificateRequest expected " + data->type());
		}
		break;

	    case STATE_WAIT_CERT:
		if (data->type() != RECORD_HANDSHAKE) {
		    error("Handshake expected");
		}
		transcribe(data->transport());
		data = data->message();
		if (data->type() != HANDSHAKE_CERTIFICATES) {
		    error("Certificates expected " + data->type());
		}
		receiveCertificates(data, output);
		state = STATE_WAIT_CV;
		break;

	    case STATE_WAIT_CV:
		if (data->type() != RECORD_HANDSHAKE) {
		    error("Handshake expected");
		}
		transcribe(data->transport());
		data = data->message();
		if (data->type() != HANDSHAKE_CERTIFICATE_VERIFY) {
		    error("CertificateVerify expected " + data->type());
		}
		receiveCertificateVerify(data, output);
		state = STATE_WAIT_FINISHED;
		break;

	    case STATE_WAIT_FINISHED:
		if (data->type() != RECORD_HANDSHAKE) {
		    error("Handshake expected");
		}
		str = verifyData(serverSecret, messages, cipherSuite);
		transcribe(data->transport());
		data = data->message();
		if (data->type() != HANDSHAKE_FINISHED) {
		    error("Finished expected " + data->type());
		}
		if (str != data->hash()) {
		    error("Verify finished hash failed");
		}
		receiveFinished(data, output);
		state = STATE_CONNECTED;
		alert = nil;
		break;

	    case STATE_CONNECTED:
		switch (data->type()) {
		case RECORD_ALERT:
		    if (data->level() == ALERT_WARNING &&
			data->description() == ALERT_CLOSE_NOTIFY) {
			alert = "EOF";
		    }
		    break;

		case RECORD_HANDSHAKE:
		    data = data->message();
		    switch (data->type()) {
		    case HANDSHAKE_NEW_SESSION_TICKET:
			receiveNewSessionTicket(data, output);
			break;

		    case HANDSHAKE_KEY_UPDATE:
			receiveKeyUpdate(data, output);
			break;

		    default:
			/* no post_handshake_auth */
			error("Unpexpected post-handshake handshake " + data->type());
			break;
		    }
		    break;

		case RECORD_APPLICATION_DATA:
		    input->append(data->payload());
		    break;
		}
		break;
	    }
	}
    }
    return ({
	(input->length()) ? input : nil,
	(output->length()) ? output : nil,
	alert
    });
}

/*
 * encrypt and send a message
 */
StringBuffer sendMessage(StringBuffer str)
{
    StringBuffer output;
    string chunk;

    if (state != STATE_CONNECTED) {
	error("Connection not established");
    }

    if (str->length() > 16384) {
	str = new StringBuffer(str, 16384);
    }
    output = new StringBuffer;
    while ((chunk=str->chunk())) {
	::sendMessage(output, chunk);
    }
    return output;
}
