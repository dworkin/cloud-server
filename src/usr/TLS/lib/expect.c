# include "asn1.h"
# include "x509.h"

private inherit asn "/lib/util/asn";


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
    Asn1 *list;

    list = expect(node, ASN1_SEQUENCE);
    if (sizeof(list) > 1 && expect(list[1], ASN1_NULL) != "") {
	error("BAD_CERTIFICATE");
    }
    return expect(list[0], ASN1_OBJECT_IDENTIFIER);
}

/*
 * RFC 4055 section 3
 *
 * expect a RSASSA-PSS parameter node
 */
static string expectParams(Asn1 params)
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
