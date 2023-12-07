# include <String.h>
# include "Extension.h"
# include "tls.h"
# include <type.h>


private int type;	/* type of data */

/*
 * initialize TLS data
 */
static void create(int type)
{
    ::type = type;
}

/*
 * subrange of string or String
 */
static string substring(mixed str, int from, int to)
{
    return (typeof(str) == T_STRING) ?
	    str[from .. to] :
	    str->bufferRange(from, to)->chunk();
}

/*
 * offset after length-1-encoded string
 */
static int len1Offset(mixed str, int offset)
{
    return offset + 1 + str[offset];
}

/*
 * offset after length-2-encoded string
 */
static int len2Offset(mixed str, int offset)
{
    return offset + 2 + ((str[offset] << 8) | str[offset + 1]);
}

/*
 * offset after length-3-encoded string
 */
static int len3Offset(String str, int offset)
{
    return offset + 3 +
	   ((str[offset] << 16) | (str[offset + 1] << 8) | str[offset + 2]);
}

/*
 * ({ length-1-encoded string, offset })
 */
static mixed *len1Restore(mixed str, int offset)
{
    int end;

    end = len1Offset(str, offset);
    return ({
	(typeof(str) == T_STRING) ?
	 str[offset + 1 .. end - 1] :
	 str->bufferRange(offset + 1, end - 1)->chunk(),
	end
    });
}

/*
 * ({ length-2-encoded string, end })
 */
static mixed *len2Restore(mixed str, int offset)
{
    int end;

    end = len2Offset(str, offset);
    return ({
	(typeof(str) == T_STRING) ?
	 str[offset + 2 .. end - 1] :
	 str->bufferRange(offset + 2, end - 1)->chunk(),
	end
    });
}

/*
 * ({ length-3-encoded string, end })
 */
static mixed *len3Restore(mixed str, int offset)
{
    int end;

    end = len3Offset(str, offset);
    return ({ str[offset + 3 .. end - 1], end });
}

/*
 * 4-byte big-endian integer
 */
static int int4Restore(String str, int offset)
{
    return (str[offset] << 24) | (str[offset + 1] << 16) |
	   (str[offset + 2] << 8) | str[offset + 3];
}

/*
 * list of client/server extensions
 */
static Extension *extRestore(String str, int offset, int end,
			     varargs int client)
{
    Extension *extensions;
    int next;

    if (end != len2Offset(str, offset)) {
	error("DECODE_ERROR");
    }
    offset += 2;
    extensions = ({ });
    while (offset < end) {
	next = len2Offset(str, offset + 2);
	extensions += ({
	    new RemoteExtension(str, offset, next, client)
	});
	offset = next;
    }
    if (offset != end) {
	error("DECODE_ERROR");
    }

    return extensions;
}

/*
 * restore KeyShare
 */
static mixed *keyShareRestore(string group, string key)
{
    switch (group) {
    case TLS_SECP256R1:
	if (key[0] != '\4' || strlen(key) != 1 + 32 + 32) {
	    error("DECODE_ERROR");
	}
	return ({ group, ({ key[1 .. 32], key[33 .. 64] }) });

    case TLS_SECP384R1:
	if (key[0] != '\4' || strlen(key) != 1 + 48 + 48) {
	    error("DECODE_ERROR");
	}
	return ({ group, ({ key[1 .. 48], key[49 .. 96] }) });

    case TLS_SECP521R1:
	if (key[0] != '\4' || strlen(key) != 1 + 66 + 66) {
	    error("DECODE_ERROR");
	}
	return ({ group, ({ key[1 .. 66], key[67 .. 132] }) });

    default:
	return ({ group, key });
    }
}

/*
 * save as length-1-encoded string
 */
static string len1Save(string str)
{
    int length;
    string len;

    length = strlen(str);
    len = ".";
    len[0] = length;

    return len + str;
}

/*
 * save as length-2-encoded string
 */
static string len2Save(string str)
{
    int length;
    string len;

    length = strlen(str);
    len = "..";
    len[0] = length >> 8;
    len[1] = length;

    return len + str;
}

/*
 * save as 3 byte length
 */
static string len3(mixed str)
{
    int length;
    string len;

    length = (typeof(str) == T_STRING) ? strlen(str) : str->length();
    len = "...";
    len[0] = length >> 16;
    len[1] = length >> 8;
    len[2] = length;

    return len;
}

/*
 * save as 2-byte bigendian integer
 */
static string int2Save(int i)
{
    string str;

    str = "..";
    str[0] = i >> 8;
    str[1] = i;

    return str;
}

/*
 * save as 4-byte bigendian integer
 */
static string int4Save(int i)
{
    string str;

    str = "....";
    str[0] = i >> 24;
    str[1] = i >> 16;
    str[2] = i >> 8;
    str[3] = i;

    return str;
}

/*
 * save list of extensions
 */
static string extSave(Extension *extensions)
{
    string *ext;
    int sz, i;

    ext = allocate(sz = sizeof(extensions));
    for (i = 0; i < sz; i++) {
	ext[i] = extensions[i]->transport();
    }
    return len2Save(implode(ext, ""));
}

/*
 * save KeyShare
 */
static string keyShareSave(mixed key)
{
    return (typeof(key) == T_ARRAY) ? "\4" + key[0] + key[1] : key;
}

/*
 * save type
 */
string typeHeader()
{
    string str;

    str = ".";
    str[0] = type;
    return str;
}

string transport();


int type()	{ return type; }
