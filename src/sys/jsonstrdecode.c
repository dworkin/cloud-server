#include <String.h>

private inherit hex "/lib/util/hex";


object utf8Decode;	/* UTF8 decoder */

/*
 * initialize JSON string decoder
 */
static void create()
{
    utf8Decode = find_object("/sys/utf8decode");
}

/*
 * decode JSON string to String
 */
String decode(string str)
{
    mixed *result;

    result = parse_string("\
chars = /[^\"\\\\]+/							\
escaped = /(\\\\[^u])+/							\
unicode = /(\\\\u[0-9A-Za-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f])+/	\
invalid = /./								\
\
String: '\"' Characters '\"'				? str		\
String: '\"' '\"'					? emptyStr	\
\
Characters: Characters Character			? sequence	\
Characters: Character					? start		\
\
Character: chars					? utf8		\
Character: escaped					? escaped	\
Character: unicode					? unicode",
			  str);

    return (result) ? result[0] : nil;
}

/*
 * return intermediate result as String
 */
static mixed *str(mixed *parsed)
{
    return ({ new String(parsed[1]) });
}

/*
 * ""
 */
static mixed *emptyStr(mixed parsed)
{
    return ({ new String("") });
}

/*
 * append sequence to StringBuffer
 */
static mixed *sequence(mixed *parsed)
{
    StringBuffer buffer;
    int sz, i;

    buffer = parsed[0];
    for (sz = sizeof(parsed), i = 1; i < sz; i++) {
	buffer->append(parsed[i]);
    }
    return ({ buffer });
}

/*
 * create intermediate StringBuffer
 */
static mixed *start(mixed *parsed)
{
    StringBuffer buffer;
    int sz, i;

    buffer = new StringBuffer(parsed[0]);
    for (sz = sizeof(parsed), i = 1; i < sz; i++) {
	buffer->append(parsed[i]);
    }
    return ({ buffer });
}

/*
 * translate UTF-8 sequence
 */
static mixed *utf8(mixed *parsed)
{
    return utf8Decode->decode(parsed[0])[0];
}

/*
 * translate escape sequence
 */
static mixed *escaped(mixed *parsed)
{
    string str, *result;
    int sz, i, j;

    str = parsed[0];
    sz = strlen(str) / 2;
    result = allocate(sz);
    for (j = 1, i = 0; i < sz; j += 2, i++) {
	switch (str[j]) {
	case 'b':
	    result[i] = "\b";
	    break;

	case 'f':
	    result[i] = "\f";
	    break;

	case 'n':
	    result[i] = "\n";
	    break;

	case 'r':
	    result[i] = "\r";
	    break;

	case 't':
	    result[i] = "\t";
	    break;

	default:
	    result[i] = str[j .. j];
	    break;
	}
    }

    return ({ implode(result, "") });
}

/*
 * translate unicode escape sequence
 */
static mixed *unicode(mixed *parsed)
{
    mapping map;
    string str;
    int len, i, j, code, code2;

    map = ([ ]);
    str = parsed[0];
    for (len = strlen(str), i = 2; i < len; i += 6) {
	code = hex::decode(str[i.. i + 3]);
	if (code >= 0xd800 && code <= 0xdc00) {
	    /*
	     * high surrogate
	     */
	    i += 6;
	    if (i >= len) {
		return nil;
	    }
	    code2 = hex::decode(str[i .. i + 3]);
	    if (code2 < 0xdc00) {
		return nil;
	    }
	    code = 0x10000 + ((code - 0xd800) << 10) + code2 - 0xdc00;
	}
	map[j++] = code;
    }
    return ({ map_values(map) });
}
