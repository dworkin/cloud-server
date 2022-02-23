/*
 * decode part of an URL
 */
string decode(string str)
{
    string *result;

    result = parse_string("\
unescaped = /[^%]+/							\
escaped = /%[0-9a-fA-F][0-9a-fA-F][^%]*/				\
junk = /./								\
\
Str:									\
Str: unescaped								\
Str: unescaped Encoded							\
Str: Encoded								\
\
Encoded: Escaped					? escaped	\
\
Escaped: escaped							\
Escaped: Escaped escaped", str);

    return (result) ? implode(result, "") : nil;
}

# define HEX	("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09......." +    \
		 "\x0A\x0B\x0C\x0D\x0E\x0F.........................." + \
		 "\x0a\x0b\x0c\x0d\x0e\x0f")

/*
 * decode escape sequences
 */
static string *escaped(string *parsed)
{
    string char;
    int i;

    char = ".";
    for (i = sizeof(parsed); --i >= 0; ) {
	char[0] = (HEX[parsed[i][1] - '0'] << 4) + HEX[parsed[i][2] - '0'];
	parsed[i] = char + parsed[i][3 ..];
    }

    return parsed;
}
