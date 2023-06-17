# include "asn1.h"

inherit Asn1;


/*
 * parse ASN1
 */
private mixed *parse(string str, int offset)
{
    int c, flags, tag, length;

    c = str[offset++];
    flags = c;

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

    return ({ flags, tag, length, offset });
}

/*
 * parse ASN1 sequence
 */
private Asn1 *parseSequence(string str, int offset, int end)
{
    Asn1 *list;
    int flags, tag, length;
    mixed contents;

    list = ({ });
    while (offset < end) {
	({ flags, tag, length, offset }) = parse(str, offset);

	if (flags & 0x20) {
	    contents = parseSequence(str, offset, offset + length);
	} else {
	    contents = str[offset .. offset + length - 1];
	}
	offset += length;

	list += ({ new Asn1(tag, contents, flags >> 6) });
    }
    if (offset != end) {
	error("Bad length");
    }

    return list;
}

/*
 * parse flat ASN1 sequence
 */
private string *parseFlatSequence(string str, int offset, int end)
{
    string *list, tail;
    int dummy, length, start;

    list = ({ });
    while (offset < end) {
	({ dummy, dummy, length, start }) = parse(str, offset);
	list += ({ str[offset .. start + length - 1] });
	offset = start + length;
    }
    if (offset != end) {
	error("Bad length");
    }

    return list;
}

/*
 * create ASN1 from DER
 */
static void create(string str, varargs int flat)
{
    int flags, tag, length, offset;
    mixed contents;

    ({ flags, tag, length, offset }) = parse(str, 0);
    if (offset + length != strlen(str)) {
	error("Bad length");
    }

    if (flags & 0x20) {
	if (flat) {
	    contents = parseFlatSequence(str, offset, offset + length);
	} else {
	    contents = parseSequence(str, offset, offset + length);
	}
    } else {
	contents = str[offset ..];
    }

    ::create(tag, contents, flags >> 6);
}
