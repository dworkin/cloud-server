# include <type.h>

/*
 * ASN.1 node implementation.  The contents is either an integer, a string
 * or an array of ASN.1 nodes.
 *
 * Limits:
 *  - tag is at most 32 bytes
 *  - length is at most 2 bytes
 */


private int class;	/* universal=0 application=1 contextual=2 private=3 */
private int tag;	/* tag of ASN.1 node */
private mixed contents;	/* contents of node */

/*
 * create ASN.1 node
 */
static void create(int tag, mixed contents, varargs int class)
{
    if (tag < 0 || tag > 0x7fffffff) {
	error("Bad tag");
    }
    ::class = class;
    ::tag = tag;
    ::contents = contents;
}

/*
 * transport ASN.1 node
 */
static string transport()
{
    string ident, len, str;
    int sz, i;
    string *list;

    /* tag */
    switch (tag) {
    case 0 .. 0x1e:
	ident = ".";
	ident[0] = tag;
	break;

    case 0x1f .. 0x7f:
	ident = "\x1f.";
	ident[1] = tag;
	break;

    case 0x80 .. 0x3fff:
	ident = "\x1f..";
	ident[1] = 0x80 | (tag >> 7);
	ident[2] = tag & 0x7f;
	break;

    case 0x4000 .. 0x1fffff:
	ident = "\x1f...";
	ident[1] = 0x80 | (tag >> 14);
	ident[2] = 0x80 | (tag >> 7);
	ident[3] = tag & 0x7f;
	break;

    case 0x200000 .. 0x0fffffff:
	ident = "\x1f....";
	ident[1] = 0x80 | (tag >> 21);
	ident[2] = 0x80 | (tag >> 14);
	ident[3] = 0x80 | (tag >> 7);
	ident[4] = tag & 0x7f;
	break;

    default:
	ident = "\x1f.....";
	ident[1] = 0x80 | (tag >> 28);
	ident[2] = 0x80 | (tag >> 21);
	ident[3] = 0x80 | (tag >> 14);
	ident[4] = 0x80 | (tag >> 7);
	ident[5] = tag & 0x7f;
	break;
    }
    if (typeof(contents) == T_ARRAY) {
	ident[0] |= 0x20;
    }
    ident[0] |= class << 6;

    /* content */
    switch (typeof(contents)) {
    case T_NIL:
	str = "";
	break;

    case T_STRING:
	str = contents;
	break;

    case T_ARRAY:
	sz = sizeof(contents);
	list = allocate(sz);
	for (i = 0; i < sz; i++) {
	    list[i] = contents[i]->transport();
	}
	str = implode(list, "");
	break;

    default:
	str = contents->transport();
	break;
    }

    /* length */
    sz = strlen(str);
    switch (sz) {
    case 0 .. 0x7f:
	len = ".";
	len[0] = sz;
	break;

    case 0x80 .. 0xff:
	len = "\x81.";
	len[1] = sz;
	break;

    default:
	len = "\x82..";
	len[1] = sz >> 8;
	len[2] = sz;
	break;
    }

    return ident + len + str;
}


int tag()		{ return tag; }
mixed contents()	{ return contents; }
