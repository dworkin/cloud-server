# include "x509.h"
# include "asn1.h"
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

/*
 * expect a particular node
 */
static mixed expect(Asn1 node, int tag, varargs int class)
{
    if (node->class() != class || node->tag() != tag) {
	error("Bad certificate");
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
	error("Bad certificate");
    }
    return expect(node, ASN1_OBJECT_IDENTIFIER);
}

/*
 * expect a string
 */
static string expectString(Asn1 node)
{
    if (node->class() != ASN1_CLASS_UNIVERSAL) {
	error("Bad certificate");
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
	error("Bad certificate");
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
	error("Bad certificate");
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
	error("Bad certificate");
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
	({ node }) = expect(list[offset++], 0, ASN1_CLASS_CONTEXTUAL);
	version = asn::decode(expect(node, ASN1_INTEGER));
	if (version < 0 || version > 2) {
	    error("Bad certificate");
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
    publicKeyType = expectAlgorithm(node);
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
	error("Bad certificate");
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
string signatureType()		{ return signatureType; }
string signature()		{ return signature; }
