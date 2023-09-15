# include "asn1.h"
# include "x509.h"
# include "tls.h"


private string type;		/* key type */
private string modulus;		/* RSA modulus */
private string privKey;		/* private key */
private string pubKey;		/* public key (RSA or ECDSA) */
private string prime1;		/* RSA prime1 */
private string prime2;		/* RSA prime2 */
private string exponent1;	/* RSA exponent1 */
private string exponent2;	/* RSA exponent2 */
private string coefficient;	/* RSA coefficient */

/*
 * expect a particular node
 */
static mixed expect(Asn1 node, int tag, varargs int class)
{
    if (node->class() != class || node->tag() != tag) {
	error("Bad key");
    }
    return node->contents();
}

/*
 * expect an algorithm node
 */
static string expectAlgorithm(Asn1 node)
{
    node = expect(node, ASN1_SEQUENCE)[0];
    return expect(node, ASN1_OBJECT_IDENTIFIER);
}

/*
 * obtain RSA key data
 */
static void rsaKey(string str)
{
    Asn1 *list;

    list = expect(new Asn1Der(str), ASN1_SEQUENCE);
    modulus = expect(list[1], ASN1_INTEGER);
    pubKey = expect(list[2], ASN1_INTEGER);
    privKey = expect(list[3], ASN1_INTEGER);
    prime1 = expect(list[4], ASN1_INTEGER);
    prime2 = expect(list[5], ASN1_INTEGER);
    exponent1 = expect(list[6], ASN1_INTEGER);
    exponent2 = expect(list[7], ASN1_INTEGER);
    coefficient = expect(list[8], ASN1_INTEGER);
}

/*
 * obtain EdDSA key data
 */
static void eddsaKey(string str)
{
    privKey = expect(new Asn1Der(str), ASN1_OCTET_STRING);
}

/*
 * initialize key
 */
static void create(string str)
{
    Asn1 *list;

    list = expect(new Asn1Der(str), ASN1_SEQUENCE);
    switch (expect(list[0], ASN1_INTEGER)) {
    case "\0":
	switch (expectAlgorithm(list[1])) {
	case OID_RSA_ENCRYPTION:
	    type = "RSA";
	    rsaKey(expect(list[2], ASN1_OCTET_STRING));
	    break;

	case OID_RSASSA_PSS:
	    type = "RSA-PSS";
	    rsaKey(expect(list[2], ASN1_OCTET_STRING));
	    break;

	case OID_ED25519:
	    type = TLS_ED25519;
	    eddsaKey(expect(list[2], ASN1_OCTET_STRING));
	    break;

	case OID_ED448:
	    type = TLS_ED448;
	    eddsaKey(expect(list[2], ASN1_OCTET_STRING));
	    break;

	default:
	    error("Bad key");
	}
	break;

    case "\1":
	privKey = expect(list[1], ASN1_OCTET_STRING);
	pubKey = expect(expect(list[3], 1, ASN1_CLASS_CONTEXTUAL)[0],
			ASN1_BIT_STRING);
	if (pubKey[0] != '\0') {
	    error("Bad key");
	}
	pubKey = pubKey[1 ..];
	switch (expect(expect(list[2], 0, ASN1_CLASS_CONTEXTUAL)[0],
		       ASN1_OBJECT_IDENTIFIER)) {
	case OID_SECP256R1:
	    type = TLS_ECDSA_SECP256R1_SHA256;
	    break;

	case OID_SECP384R1:
	    type = TLS_ECDSA_SECP384R1_SHA384;
	    break;

	case OID_SECP521R1:
	    type = TLS_ECDSA_SECP521R1_SHA512;
	    break;

	default:
	    error("Bad key");
	}
	break;

    default:
	error("Bad key");
    }
}


string type()			{ return type; }
string modulus()		{ return modulus; }
string privKey()		{ return privKey; }
string pubKey()			{ return pubKey; }
string prime1()			{ return prime1; }
string prime2()			{ return prime2; }
string exponent1()		{ return exponent1; }
string exponent2()		{ return exponent2; }
string coefficient()		{ return coefficient; }
