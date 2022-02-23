/*
 * encode part of an URL
 */
string encode(string str)
{
    return implode(parse_string("\
unreserved = /[-_.~A-Za-z0-9]+/						\
reserved = /[^-_.~A-Za-z0-9][-_.~A-Za-z0-9]*/				\
\
Str:									\
Str: unreserved								\
Str: unreserved Encode							\
Str: Encode								\
\
Encode: Reserved					? reserved	\
\
Reserved: reserved							\
Reserved: Reserved reserved", str), "");
}

/*
 * encode reserved characters
 */
static string *reserved(string *parsed)
{
    int i, c, d;

    for (i = sizeof(parsed); i != 0; ) {
	c = parsed[--i][0];
	d = c >> 4;
	c &= 0x0f;
	parsed[i] = "%" + "01234567890ABCDEF"[d .. d] +
		    "01234567890ABCDEF"[c .. c] + parsed[i][1 ..];
    }

    return parsed;
}
