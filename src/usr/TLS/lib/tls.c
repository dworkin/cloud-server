# include <String.h>
# include "Record.h"
# include "x509.h"
# include "asn1.h"
# include "tls.h"

inherit "hkdf";
inherit "ffdhe";
inherit emsa_pss "emsa_pss";
private inherit asn "/lib/util/asn";


private string buffer;			/* record buffer */
private StringBuffer receiveBuffer;	/* partial handshake buffer */
private string receiveKey;		/* receive key */
private string receiveIV;		/* receive IV */
private int receiveSequence;		/* receive sequence number */
private string cipher;			/* cipher */
private int taglen;			/* tag length */
private string sendKey;			/* send key */
private string sendIV;			/* send IV */
private int sendSequence;		/* send sequence number */

/*
 * initialize TLS library
 */
static void create()
{
    buffer = "";
}

/*
 * return information about a cipher suite
 */
private mixed *cipherInfo(string algorithm)
{
    switch (algorithm) {
	/*
	 *	  cipher,		keylen,	taglen,	hash
	 */
    case TLS_AES_128_GCM_SHA256:
	return ({ "AES-128-GCM",	16,	16,	"SHA256" });

    case TLS_AES_256_GCM_SHA384:
	return ({ "AES-256-GCM",	32,	16,	"SHA384" });

    case TLS_CHACHA20_POLY1305_SHA256:
	return ({ "ChaCha20-Poly1305",	32,	16,	"SHA256" });

    case TLS_AES_128_CCM_SHA256:
	return ({ "AES-128-CCM",	16,	16,	"SHA256" });

    case TLS_AES_128_CCM_8_SHA256:
	return ({ "AES-128-CCM",	16,	 8,	"SHA256" });
    }
}

/*
 * HKDF-Expand-Label (RFC 8446)
 */
private string HKDF_Expand_Label(string secret, string label, string context,
				 int length, string hash)
{
    string str1, str2;

    str1 = "...tls13 ";
    str1[0] = length >> 8;
    str1[1] = length;
    str1[2] = strlen(label) + 6;
    str2 = ".";
    str2[0] = strlen(context);

    return HKDF_Expand(secret, str1 + label + str2 + context, length, hash);
}

/*
 * Derive-Secret (RFC 8446, but 3rd argument is not hashed)
 */
private string Derive_Secret(string secret, string label, string transcriptHash,
			     string hash)
{
    return HKDF_Expand_Label(secret, label, transcriptHash,
			     (hash == "SHA256") ? 32 : 48, hash);
}

/*
 * determine handshake secrets
 */
static string *handshakeSecrets(string sharedSecret, string *messages,
				string algorithm)
{
    string hash, zeroKey, earlySecret, emptyHash, transcriptHash, derivedSecret,
	   handshakeSecret, serverSecret, clientSecret, masterSecret;

    hash = cipherInfo(algorithm)[3];
    if (hash == "SHA256") {
	zeroKey = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    } else {
	zeroKey = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    }
    earlySecret = HKDF_Extract(zeroKey, hash);
    emptyHash = hash_string(hash, "");
    derivedSecret = Derive_Secret(earlySecret, "derived", emptyHash, hash);
    handshakeSecret = HKDF_Extract(sharedSecret, hash, derivedSecret);
    transcriptHash = hash_string(hash, messages...);
    serverSecret = Derive_Secret(handshakeSecret, "s hs traffic",
				 transcriptHash, hash);
    clientSecret = Derive_Secret(handshakeSecret, "c hs traffic",
				 transcriptHash, hash);
    derivedSecret = Derive_Secret(handshakeSecret, "derived", emptyHash, hash);
    masterSecret = HKDF_Extract(zeroKey, hash, derivedSecret);

    return ({ serverSecret, clientSecret, masterSecret });
}

/*
 * hash based on secret and transcript
 */
static string verifyData(string secret, string *messages, string algorithm)
{
    string hash;

    hash = cipherInfo(algorithm)[3];
    return HMAC(Derive_Secret(secret, "finished", "", hash),
		hash_string(hash, messages...), hash);
}

/*
 * check stack of certificates, translate OpenSSL error to TLS error
 */
static void checkCertificates(string origin, string *certificates)
{
    string tlsError;

    try {
	switch (verify_certificate(origin, certificates...)) {
	case nil:
	    return;

	case "certificate signature failure":
	case "format error in certificate's notBefore field":
	case "format error in certificate's notAfter field":
	case "CA signature digest algorithm too weak":
	    tlsError = "BAD_CERTIFICATE";
	    break;

	case "certificate revoked":
	    tlsError = "CERTIFICATE_REVOKED";
	    break;

	case "certificate has expired":
	case "certificate is not yet valid":
	    tlsError = "CERTIFICATE_EXPIRED";
	    break;

	case "self-signed certificate":
	case "unsuitable certificate purpose":
	case "certificate not trusted":
	case "certificate rejected":
	    tlsError = "UNSUPPORTED_CERTIFICATE";
	    break;

	case "unable to get issuer certificate":
	case "unable to get local issuer certificate":
	    tlsError = "UNKNOWN_CA";
	    break;

	default:
	    tlsError = "CERTIFICATE_UNKNOWN";
	    break;
	}
    } catch (err) {
	tlsError = (err == "Bad certificate" ||
		    err == "Bad intermediate certificate") ?
		    "BAD_CERTIFICATE" : "INTERNAL_ERROR";
    }

    error(tlsError);
}

/*
 * verify RSA signature
 */
static void rsaVerify(string signature, string message, string publicKey,
		      string hash)
{
    Asn1 node, node2;
    string modulus, decrypted;

    try {
	node = new Asn1Der(publicKey[1 ..]);
    } catch (...) {
	error("BAD_CERTIFICATE");
    }
    if (node->tag() != ASN1_SEQUENCE) {
	error("BAD_CERTIFICATE");
    }
    ({ node, node2 }) = node->contents();
    if (node->tag() != ASN1_INTEGER || node2->tag() != ASN1_INTEGER) {
	error("BAD_CERTIFICATE");
    }

    modulus = node->contents();
    publicKey = node2->contents();
    decrypted = asn_pow("\0" + signature, publicKey, modulus);
    if (decrypted[0] == '\0') {
	decrypted = decrypted[1 ..];
    }
    if (!emsa_pss::verify(hash_string(hash, message), decrypted,
			  asn::bits(modulus) - 1, hash)) {
	error("DECRYPT_ERROR");
    }
}

/*
 * verify a signature made with a certificate
 */
static void certificateVerify(string certificate, string signature,
			      string *messages, string scheme, string algorithm)
{
    string message, hash, publicKey;
    Certificate cert;

    message = "                                " +
	      "                                " +
	      "TLS 1.3, server CertificateVerify\0" +
	      hash_string(cipherInfo(algorithm)[3], messages...);

    cert = new Certificate(certificate);
    publicKey = cert->publicKey();
    if (publicKey[0] != '\0') {
	error("BAD_CERTIFICATE");
    }

    switch (cert->publicKeyType()) {
    case "RSA":
	switch (scheme) {
	case TLS_RSA_PSS_RSAE_SHA256:	hash = "SHA256"; break;
	case TLS_RSA_PSS_RSAE_SHA384:	hash = "SHA384"; break;
	case TLS_RSA_PSS_RSAE_SHA512:	hash = "SHA512"; break;
	default:
	    error("UNEXPECTED_MESSAGE");
	}

	rsaVerify(signature, message, publicKey, hash);
	break;

    case "RSA-PSS":
	switch (scheme) {
	case TLS_RSA_PSS_PSS_SHA256:	hash = "SHA256"; break;
	case TLS_RSA_PSS_PSS_SHA384:	hash = "SHA384"; break;
	case TLS_RSA_PSS_PSS_SHA512:	hash = "SHA512"; break;
	default:
	    error("UNEXPECTED_MESSAGE");
	}
	if (hash != cert->publicKeyParam() && cert->publicKeyParam()) {
	    error("UNEXPECTED_MESSAGE");
	}

	rsaVerify(signature, message, publicKey, hash);
	break;

    case TLS_ECDSA_SECP256R1_SHA256:
    case TLS_ECDSA_SECP384R1_SHA384:
    case TLS_ECDSA_SECP521R1_SHA512:
    case TLS_ED25519:
    case TLS_ED448:
    default:
	error("UNSUPPORTED_CERTIFICATE");
    }
}

/*
 * determine application secrets
 */
static string *applicationSecrets(string masterSecret, string *messages,
				  string algorithm)
{
    string hash, transcriptHash;

    hash = cipherInfo(algorithm)[3];
    transcriptHash = hash_string(hash, messages...);
    return ({
	Derive_Secret(masterSecret, "s ap traffic", transcriptHash, hash),
	Derive_Secret(masterSecret, "c ap traffic", transcriptHash, hash)
    });
}

/*
 * determine key and IV
 */
static string *keyIV(string secret, string algorithm)
{
    mixed *info;
    int keylen;
    string hash;

    info = cipherInfo(algorithm);
    keylen = info[1];
    hash = info[3];

    return ({
	HKDF_Expand_Label(secret, "key", "", keylen, hash),
	HKDF_Expand_Label(secret, "iv", "", 12, hash)
    });
}

/*
 * set the receive key/IV/taglen/cipher
 */
static void setReceiveKey(string key, string IV, string cipherSuite)
{
    mixed *info;

    receiveKey = key;
    receiveIV = IV;
    receiveSequence = 0;
    info = cipherInfo(cipherSuite);
    cipher = info[0];
    taglen = info[2];
}

/*
 * set the send key/IV/taglen/cipher
 */
static void setSendKey(string key, string IV, string cipherSuite)
{
    mixed *info;

    sendKey = key;
    sendIV = IV;
    sendSequence = 0;
    info = cipherInfo(cipherSuite);
    cipher = info[0];
    taglen = info[2];
}

/*
 * translate error into alert description
 */
static int alertDescription(string error)
{
    switch (error) {
    case "UNEXPECTED_MESSAGE":		return ALERT_UNEXPECTED_MESSAGE;
    case "BAD_RECORD_MAC":		return ALERT_BAD_RECORD_MAC;
    case "RECORD_OVERFLOW":		return ALERT_RECORD_OVERFLOW;
    case "HANDSHAKE_FAILURE":		return ALERT_HANDSHAKE_FAILURE;
    case "BAD_CERTIFICATE":		return ALERT_BAD_CERTIFICATE;
    case "UNSUPPORTED_CERTIFICATE":	return ALERT_UNSUPPORTED_CERTIFICATE;
    case "CERTIFICATE_REVOKED":		return ALERT_CERTIFICATE_REVOKED;
    case "CERTIFICATE_EXPIRED":		return ALERT_CERTIFICATE_EXPIRED;
    case "CERTIFICATE_UNKNOWN":		return ALERT_CERTIFICATE_UNKNOWN;
    case "ILLEGAL_PARAMETER":		return ALERT_ILLEGAL_PARAMETER;
    case "UNKNOWN_CA":			return ALERT_UNKNOWN_CA;
    case "ACCESS_DENIED":		return ALERT_ACCESS_DENIED;
    case "DECODE_ERROR":		return ALERT_DECODE_ERROR;
    case "DECRYPT_ERROR":		return ALERT_DECRYPT_ERROR;
    case "PROTOCOL_VERSION":		return ALERT_PROTOCOL_VERSION;
    case "INSUFFICIENT_SECURITY":	return ALERT_INSUFFICIENT_SECURITY;
    case "INAPPROPRIATE_FALLBACK":	return ALERT_INAPPROPRIATE_FALLBACK;
    case "MISSING_EXTENSION":		return ALERT_MISSING_EXTENSION;
    case "UNSUPPORTED_EXTENSION":	return ALERT_UNSUPPORTED_EXTENSION;
    case "UNRECOGNIZED_NAME":		return ALERT_UNRECOGNIZED_NAME;
    case "BAD_CERTIFICATE_STATUS_RESPONSE":
	return ALERT_BAD_CERTIFICATE_STATUS_RESPONSE;
    case "UNKNOWN_PSK_IDENTITY":	return ALERT_UNKNOWN_PSK_IDENTITY;
    case "CERTIFICATE_REQUIRED":	return ALERT_CERTIFICATE_REQUIRED;
    case "NO_APPLICATION_PROTOCOL":	return ALERT_NO_APPLICATION_PROTOCOL;
    default:				return ALERT_INTERNAL_ERROR;
    }
}

/*
 * receive a message
 */
static Record *receiveMessage(string str)
{
    Record *records;
    int buflen, len;

    records = ({ });

    while ((buflen=strlen(buffer)) + strlen(str) >= 5) {
	if (buflen < 5) {
	    buffer += str[.. 4 - len];
	    str = str[5 - len ..];
	    buflen = 5;
	}
	len = (buffer[3] << 8) | buffer[4];
	if (len + 5 > buflen + strlen(str)) {
	    buffer += str;
	    break;
	}

	if (len + 5 > buflen) {
	    buffer += str[.. len + 4 - buflen];
	    str = str[len + 5 - buflen ..];
	    buflen = len + 5;
	}
	records += ({ new RemoteRecord(buffer) });
	if (buflen > len + 5) {
	    buffer = buffer[len + 5 ..];
	} else {
	    buffer = str;
	    str = "";
	}
    }

    return records;
}

/*
 * check that the message is at the end of a record
 */
static void alignedRecord()
{
    if (receiveBuffer) {
	error("UNEXPECTED_MESSAGE");
    }
}

/*
 * offset after length-3-encoded string
 */
private int len3Offset(String str, int offset)
{
    return offset + 3 +
	   ((str[offset] << 16) | (str[offset + 1] << 8) | str[offset + 2]);
}

/*
 * from records to unencrypted messages
 */
static Data *receiveRecord(Record record)
{
    string fragment;
    Data *list;
    String str;
    int len, offset, end;

    if (record->type() == RECORD_APPLICATION_DATA) {
	if (!receiveKey) {
	    error("UNEXPECTED_MESSAGE");
	}
	if (!record->unprotect(cipher,
			       receiveKey,
			       asn_xor(receiveIV,
				       asn::encode(receiveSequence++)),
			       taglen)) {
	    error("BAD_RECORD_MAC");
	}
    }

    fragment = record->payload();
    switch (record->type()) {
    case RECORD_CHANGE_CIPHER_SPEC:
	if (record->decrypted() || receiveBuffer || strlen(fragment) != 1 ||
	    fragment[0] != '\1') {
	    error("UNEXPECTED_MESSAGE");
	}
	return ({ record });

    case RECORD_ALERT:
	if ((receiveKey && !record->decrypted()) || receiveBuffer ||
	    strlen(fragment) != 2) {
	    error("UNEXPECTED_MESSAGE");
	}
	return ({ new RemoteAlert(fragment) });

    case RECORD_HANDSHAKE:
	if ((receiveKey && !record->decrypted()) || strlen(fragment) == 0) {
	    error("UNEXPECTED_MESSAGE");
	}

	list = ({ });
	if (receiveBuffer) {
	    receiveBuffer->append(fragment);
	    if (receiveBuffer->length() < 4) {
		return list;
	    }
	    str = new String(receiveBuffer);
	} else {
	    if (strlen(fragment) < 4) {
		receiveBuffer = new StringBuffer(fragment);
		return list;
	    }
	    str = new String(fragment);
	}
	for (len = str->length(), offset = 0;
	     offset + 4 <= len && (end=len3Offset(str, offset + 1)) <= len;
	     offset = end) {
	    list += ({ new RemoteHandshake(str, offset, end) });
	}

	if (offset == len) {
	    receiveBuffer = nil;
	} else if (offset != 0) {
	    receiveBuffer = str->bufferRange(offset, len - 1);
	}
	return list;

    case RECORD_APPLICATION_DATA:
	if (receiveBuffer) {
	    error("UNEXPECTED_MESSAGE");
	}
	return ({ record });

    default:
	error("UNEXPECTED_MESSAGE");
    }
}

/*
 * send a record
 */
private void sendRecord(StringBuffer output, Record record)
{
    record->protect(cipher,
		    sendKey,
		    asn_xor(sendIV,
			    asn::encode(sendSequence++)),
		    taglen);
    output->append(record->transport());
}

/*
 * send data (handshake or alert)
 */
static void sendData(StringBuffer output, Data data)
{
    sendRecord(output, new Record(data->type(), data->transport()));
}

/*
 * send a message
 */
static void sendMessage(StringBuffer output, string str, varargs int type)
{
    if (type == 0) {
	type = RECORD_APPLICATION_DATA;
    }
    sendRecord(output, new Record(type, str));
}

/*
 * send fake ChangeCipherSpec (unprotected)
 */
static void sendChangeCipherSpec(StringBuffer output)
{
    output->append("\24\3\3\0\1\1");
}
