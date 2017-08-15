/*
 * parse an LPC string with escaped content
 */
string parse(string str)
{
    return implode(parse_string("\
unescaped = /[^\\\\]+/							\
octal = /\\\\[0-7][0-7]?[0-7]?/						\
hexadecimal = /\\\\[xX][0-9a-fA-F][0-9a-fA-F]?/				\
escaped = /\\\\./							\
									\
String:									\
String: String Characters						\
Characters: unescaped							\
Characters: octal					? octal		\
Characters: hexadecimal					? hexadecimal	\
Characters: escaped					? escaped",
				str), "");
}

/*
 * convert octal string to integer
 */
int octalInt(string str)
{
    int result, i, sz;

    result = 0;
    for (i = 1, sz = strlen(str); i < sz; i++) {
	result = (result << 3) + str[i] - '0';
    }
    return result;
}

/*
 * convert hexadecimal string to integer
 */
int hexadecimalInt(string str)
{
    string map;
    int result, i, sz;

    map = "................................" +
	  "................\0\1\2\3\4\5\6\7\x8\x9......" +
	  ".\xa\xb\xc\xd\xe\xf........................." +
	  ".\xa\xb\xc\xd\xe\xf";
    result = 0;
    for (i = 2, sz = strlen(str); i < sz; i++) {
	result = (result << 4) + map[str[i]];
    }
    return result;
}

/*
 * convert escaped octal to character
 */
static mixed *octal(mixed *parsed)
{
    string str;

    str = " ";
    str[0] = octalInt(parsed[0]);
    return ({ str });
}

/*
 * convert escaped hexadecimal to character
 */
static mixed *hexadecimal(mixed *parsed)
{
    string str;

    str = " ";
    str[0] = hexadecimalInt(parsed[0]);
    return ({ str });
}

/*
 * convert escape sequence to character
 */
static mixed *escaped(mixed *parsed)
{
    string str;

    str = parsed[0][1 ..];
    switch (str[0]) {
    case '\n':
	str = "";
	break;

    case 'a':
	str = "\a";
	break;

    case 'b':
	str = "\b";
	break;

    case 't':
	str = "\t";
	break;

    case 'n':
	str = "\n";
	break;

    case 'v':
	str = "\v";
	break;

    case 'f':
	str = "\f";
	break;

    case 'r':
	str = "\r";
	break;
    }
    return ({ str });
}
