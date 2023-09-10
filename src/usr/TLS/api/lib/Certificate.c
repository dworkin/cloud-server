# include "x509.h"
# include "asn1.h"
# include "tls.h"
# include <Time.h>

private inherit asn "/lib/util/asn";


private string flatCertificate;		/* unsigned certificate as a string */
private string signatureType;		/* type of signature */
private string signature;		/* signature on certificate */
private int version;			/* version (2 means 3) */
private string serial;			/* serial number */
private string signatureAlgorithm;	/* signature algorithm */
private string issuer;			/* issuer name */
private Time validFrom;			/* start of valid period */
private Time validTo;			/* end of valid period */
private string subject;			/* subject name */
private string publicKeyType;		/* public key type */
private string publicKey;		/* public key */
private string publicKeyParam;		/* public key parameter */

/*
 * expect a particular node
 */
static mixed expect(Asn1 node, int tag, varargs int class)
{
    if (node->class() != class || node->tag() != tag) {
	error("BAD_CERTIFICATE");
    }
    return node->contents();
}

/*
 * expect an algorithm node
 */
static string expectAlgorithm(Asn1 node)
{
    Asn1 params;

    ({ node, params }) = expect(node, ASN1_SEQUENCE);
    if (expect(params, ASN1_NULL) != "") {
	error("BAD_CERTIFICATE");
    }
    return expect(node, ASN1_OBJECT_IDENTIFIER);
}

/*
 * expect a string
 */
static string expectString(Asn1 node)
{
    if (node->class() != ASN1_CLASS_UNIVERSAL) {
	error("BAD_CERTIFICATE");
    }
    switch (node->tag()) {
    case ASN1_UTF8_STRING:
    case ASN1_PRINTABLE_STRING:
    case ASN1_TELETEX_STRING:
    case ASN1_IA5_STRING:
    case ASN1_UNIVERSAL_STRING:
    case ASN1_BMP_STRING:
	return node->contents();

    default:
	error("BAD_CERTIFICATE");
    }
}

/*
 * pick the common name from a sequence of identifiers
 */
static string expectName(Asn1 node)
{
    Asn1 *list, str;
    int sz, i;

    ({ node }) = expect(node, ASN1_SEQUENCE);
    list = expect(node, ASN1_SET);
    for (sz = sizeof(list), i = 0; i < sz; i++) {
	({ node, str }) = expect(list[i], ASN1_SEQUENCE);
	if (expect(node, ASN1_OBJECT_IDENTIFIER) == OID_COMMON_NAME) {
	    return expectString(str);
	}
    }
}

/*
 * convert GENERALIZED_TIME to GMT(ish)
 */
private string gmtime(string time)
{
    string month;

    switch (time[4 .. 5]) {
    case "01":	month = "Jan"; break;
    case "02":	month = "Feb"; break;
    case "03":	month = "Mar"; break;
    case "04":	month = "Apr"; break;
    case "05":	month = "May"; break;
    case "06":	month = "Jun"; break;
    case "07":	month = "Jul"; break;
    case "08":	month = "Aug"; break;
    case "09":	month = "Sep"; break;
    case "10":	month = "Oct"; break;
    case "11":	month = "Nov"; break;
    case "12":	month = "Dec"; break;
    default:
	error("BAD_CERTIFICATE");
    }

    return "Mon " + month + " " + time[6 .. 7] + " " + time[8 .. 9] + ":" +
	   time[10 .. 11] + ":" + time[12 .. 13] + " " + time[0 .. 3];
}

/*
 * expect a time
 */
static Time expectTime(Asn1 node)
{
    string time;

    switch (node->tag()) {
    case ASN1_UTC_TIME:
	time = node->contents();
	if (time[0 .. 1] >= "70") {
	    time = gmtime("19" + time);
	} else {
	    time = gmtime("20" + time);
	}
	return new GMTime(time);

    case ASN1_GENERALIZED_TIME:
	return new GMTime(gmtime(node->contents()));

    default:
	error("BAD_CERTIFICATE");
    }
}

/*
 * RFC 4055 section 3
 *
 * Verify RSASSA-PSS parameters, and return the hash function if specified.
 */
private string pssParams(Asn1 params)
{
    Asn1 *list, node;
    int sz, offset, n;
    string hash;

    if (params->tag() == ASN1_NULL) {
	if (params->contents() != "") {
	    error("BAD_CERTIFICATE");
	}
	return nil;	/* no parameters */
    }

    list = expect(params, ASN1_SEQUENCE);
    sz = sizeof(list);
    offset = 0;
    if (offset < sz && list[offset]->class() == ASN1_CLASS_CONTEXTUAL &&
	list[offset]->tag() == 0) {
	/*
	 * hash function
	 */
	({ node }) = list[offset++]->contents();
	switch (expectAlgorithm(node)) {
	case OID_SHA256:
	    hash = "SHA256";
	    break;

	case OID_SHA384:
	    hash = "SHA384";
	    break;

	case OID_SHA512:
	    hash = "SHA512";
	    break;

	default:
	    error("UNSUPPORTED_CERTIFICATE");
	}
    } else {
	/* default hash function is invalid */
	error("UNSUPPORTED_CERTIFICATE");
    }

    if (offset < sz && list[offset]->class() == ASN1_CLASS_CONTEXTUAL &&
	list[offset]->tag() == 1) {
	/*
	 * mask generator function (must be MGF1)
	 */
	({ node }) = list[offset++]->contents();
	({ node, params }) = expect(node, ASN1_SEQUENCE);
	if (expect(node, ASN1_OBJECT_IDENTIFIER) != OID_MGF1) {
	    error("UNSUPPORTED_CERTIFICATE");
	}

	/*
	 * hash function must be specified, because the default is invalid for
	 * TLS 1.3
	 */
	switch (expectAlgorithm(params)) {
	case OID_SHA256:
	    if (hash != "SHA256") {
		error("UNSUPPORTED_CERTIFICATE");
	    }
	    break;

	case OID_SHA384:
	    if (hash != "SHA384") {
		error("UNSUPPORTED_CERTIFICATE");
	    }
	    break;

	case OID_SHA512:
	    if (hash != "SHA512") {
		error("UNSUPPORTED_CERTIFICATE");
	    }
	    break;

	default:
	    error("UNSUPPORTED_CERTIFICATE");
	}
    } else {
	/* default hash function is invalid */
	error("UNSUPPORTED_CERTIFICATE");
    }

    if (offset < sz && list[offset]->class() == ASN1_CLASS_CONTEXTUAL &&
	list[offset]->tag() == 2) {
	/*
	 * minimum salt length, default is okay
	 */
	({ node }) = list[offset++]->contents();
	n = asn::decode(expect(node, ASN1_INTEGER));
	switch (hash) {
	case "SHA256":
	    if (n > 32) {
		error("UNSUPPORTED_CERTIFICATE");
	    }
	    break;

	case "SHA384":
	    if (n > 48) {
		error("UNSUPPORTED_CERTIFICATE");
	    }
	    break;

	case "SHA512":
	    if (n > 64) {
		error("UNSUPPORTED_CERTIFICATE");
	    }
	    break;
	}
    }

    if (offset < sz && list[offset]->class() == ASN1_CLASS_CONTEXTUAL &&
	list[offset]->tag() == 3) {
	/*
	 * trailer field, default is okay
	 */
	({ node }) = list[offset++]->contents();
	if (expect(node, ASN1_INTEGER) != "1") {
	    error("UNSUPPORTED_CERTIFICATE");
	}
    }

    if (offset < sz) {
	error("BAD_CERTIFICATE");
    }

    return hash;
}

/*
 * expect a public key algorithm with parameters
 */
static mixed *expectPublicKey(Asn1 node)
{
    Asn1 *list;
    string type, hash;

    list = expect(node, ASN1_SEQUENCE);
    type = expect(list[0], ASN1_OBJECT_IDENTIFIER);
    switch (type) {
    case OID_RSA_ENCRYPTION:
	if (sizeof(list) != 2 || expect(list[1], ASN1_NULL) != "") {
	    error("BAD_CERTIFICATE");
	}
	return ({ "RSA", nil });

    case OID_RSASSA_PSS:
	if (sizeof(list) != 2) {
	    error("BAD_CERTIFICATE");
	}
	return ({ "RSA-PSS", pssParams(list[1]) });

    case OID_EC_PUBLIC_KEY:
	if (sizeof(list) != 2) {
	    error("BAD_CERTIFICATE");
	}
	switch (expect(list[1], ASN1_OBJECT_IDENTIFIER)) {
	case OID_SECP256R1:
	    type = TLS_ECDSA_SECP256R1_SHA256;
	    hash = "SHA256";
	    break;

	case OID_SECP384R1:
	    type = TLS_ECDSA_SECP384R1_SHA384;
	    hash = "SHA384";
	    break;

	case OID_SECP521R1:
	    type = TLS_ECDSA_SECP521R1_SHA512;
	    hash = "SHA512";
	    break;

	default:
	    error("UNSUPPORTED_CERTIFICATE");
	}
	return ({ type, hash });

    case OID_ED25519:
	if (sizeof(list) != 1) {
	    error("BAD_CERTIFICATE");
	}
	return ({ TLS_ED25519, nil });

    case OID_ED448:
	if (sizeof(list) != 1) {
	    error("BAD_CERTIFICATE");
	}
	return ({ TLS_ED448, nil });

    default:
	error("UNSUPPORTED_CERTIFICATE");
    }
}

/*
 * initialize a certificate
 */
static void create(string cert)
{
    int offset;
    Asn1 *list, node, start, end, key;

    offset = 0;

    ({
	flatCertificate, signatureType, signature
    }) = expect(new Asn1Der(cert, TRUE), ASN1_SEQUENCE);

    list = new Asn1Der(flatCertificate)->contents();
    if (list[offset]->class() == ASN1_CLASS_CONTEXTUAL &&
	list[offset]->tag() == 0) {
	({ node }) = list[offset++]->contents();
	version = asn::decode(expect(node, ASN1_INTEGER));
	if (version < 0 || version > 2) {
	    error("UNSUPPORTED_CERTIFICATE");
	}
    }

    serial = expect(list[offset++], ASN1_INTEGER);

    signatureAlgorithm = expectAlgorithm(list[offset++]);

    issuer = expectName(list[offset++]);

    ({ start, end }) = expect(list[offset++], ASN1_SEQUENCE);
    validFrom = expectTime(start);
    validTo = expectTime(end);

    subject = expectName(list[offset++]);

    ({ node, key }) = expect(list[offset++], ASN1_SEQUENCE);
    ({ publicKeyType, publicKeyParam }) = expectPublicKey(node);
    publicKey = expect(key, ASN1_BIT_STRING);

    if (list[offset]->class() == ASN1_CLASS_CONTEXTUAL &&
	list[offset]->tag() == 1) {
	offset++;
    }
    if (list[offset]->class() == ASN1_CLASS_CONTEXTUAL &&
	list[offset]->tag() == 2) {
	offset++;
    }
    if (list[offset]->class() == ASN1_CLASS_CONTEXTUAL &&
	list[offset]->tag() == 3) {
	offset++;
    }
    if (offset != sizeof(list)) {
	error("BAD_CERTIFICATE");
    }

    signatureType = expectAlgorithm(new Asn1Der(signatureType));
    signature = expect(new Asn1Der(signature), ASN1_BIT_STRING);
}


string flatCertificate()	{ return flatCertificate; }
int version()			{ return version; }
string serial()			{ return serial; }
string signatureAlgorithm()	{ return signatureAlgorithm; }
string issuer()			{ return issuer; }
Time validFrom()		{ return validFrom; }
Time validTo()			{ return validTo; }
string subject()		{ return subject; }
string publicKeyType()		{ return publicKeyType; }
string publicKey()		{ return publicKey; }
string publicKeyParam()		{ return publicKeyParam; }
string signatureType()		{ return signatureType; }
string signature()		{ return signature; }
