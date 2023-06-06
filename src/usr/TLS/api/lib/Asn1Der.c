# include "asn1.h"

inherit Asn1;


/*
 * parse ASN1
 */
private mixed *parse(string str)
{
    int c, offset, flags, tag, length;

    c = str[0];
    flags = c;
    offset = 1;

    /*
     * tag
     */
    c &= 0x1f;
    if (c < 0x1f) {
	tag = c;
    } else {
	if ((str[offset] & 0x7f) == 0) {
	    error("Bad tag");
	}

	tag = 0;
	do {
	    if (tag > 0x00ffffff) {
		error("Tag too large");
	    }
	    c = str[offset++];
	    tag = (tag << 7) | (c & 0x7f);
	} while (c & 0x80);
    }

    /*
     * length
     */
    c = str[offset++];
    if (c <= 0x7f) {
	length = c;
    } else {
	c &= 0x7f;
	if (c == 0 || str[offset] == 0) {
	    error("Bad length");
	}
	if (c > 2) {
	    error("Length too large");
	}

	length = 0;
	do {
	    length = (length << 8) | str[offset++];
	} while (--c != 0);
    }

    return ({ flags, tag, length, str[offset ..] });
}

/*
 * parse ASN1 sequence
 */
private Asn1 *parseSequence(string str)
{
    Asn1 *list;
    int flags, tag, length;
    mixed contents;

    list = ({ });
    while (strlen(str) != 0) {
	({ flags, tag, length, str }) = parse(str);
	contents = str[.. length - 1];
	str = str[length ..];

	if (flags & 0x20) {
	    contents = parseSequence(contents);
	}

	list += ({ new Asn1(tag, contents, flags >> 6) });
    }

    return list;
}

/*
 * create ASN1 from DER
 */
static void create(string str)
{
    int flags, tag, length;
    mixed contents;

    ({ flags, tag, length, contents }) = parse(str);
    if (strlen(contents) != length) {
	error("Bad length");
    }

    if (flags & 0x20) {
	contents = parseSequence(contents);
    }

    ::create(tag, contents, flags >> 6);
}
