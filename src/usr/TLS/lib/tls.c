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
private string receiveCipher;		/* receive cipher */
private int receiveTaglen;		/* receive tag length */
private string sendKey;			/* send key */
private string sendIV;			/* send IV */
private int sendSequence;		/* send sequence number */
private string sendCipher;		/* send cipher */
private int sendTaglen;			/* send tag length */

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
 * Derive-Secret (RFC 8446)
 */
private string Derive_Secret(string secret, string label, string transcriptHash,
			     string hash)
{
    int length;

    switch (hash) {
    case "SHA256":
	length = 32;
	break;

    case "SHA384":
	length = 48;
	break;

    default:
	error("Unsupported hash function");
    }

    return HKDF_Expand_Label(secret, label, transcriptHash, length, hash);
}

/*
 * determine key schedule
 */
static string *keySchedule(string sharedSecret, string *messages,
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
    receiveCipher = info[0];
    receiveTaglen = info[2];
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
    sendCipher = info[0];
    sendTaglen = info[2];
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
	    error("No key");
	}
	if (!record->unprotect(receiveCipher,
			       receiveKey,
			       asn_xor(receiveIV,
				       asn::encode(receiveSequence++)),
			       receiveTaglen)) {
	    error("Unprotect failed");
	}
    }

    fragment = record->payload();
    switch (record->type()) {
    case RECORD_CHANGE_CIPHER_SPEC:
	if (record->decrypted()) {
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
	}
	if (receiveBuffer) {
	    /* previously received partial handshake */
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
	}
	if (strlen(fragment) != 1) {
	    /* no partial or changes */
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
	}
	return ({ record });

    case RECORD_ALERT:
	if (receiveKey && !record->decrypted()) {
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
	}
	if (receiveBuffer) {
	    /* previously received partial handshake */
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
	}
	if (strlen(fragment) != 2) {
	    /* no partial or coalesced alerts */
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
	}
	return ({ new RemoteAlert(fragment) });

    case RECORD_HANDSHAKE:
	if (receiveKey && !record->decrypted()) {
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
	}
	if (strlen(fragment) == 0) {
	    /* no empty handshakes */
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
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
	if (!record->decrypted()) {
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
	}
	if (receiveBuffer) {
	    /* previously received partial handshake */
	    error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
	}
	return ({ record });

    default:
	error("ALERT_UNEXPECTED_MESSAGE");	/* XXX what alert type? */
    }
}