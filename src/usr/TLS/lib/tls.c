# include <String.h>
# include "Record.h"
# include "tls.h"

inherit "crypto";
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
 * return TRUE if the last message ends on a record boundary
 */
static int alignedRecord()
{
    return (!receiveBuffer);
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
