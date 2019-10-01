/*
 * encode part of a URL
 */
string encode(string str)
{
    return implode(parse_string("\
unescaped = /[-_.~\\/?&=A-Za-z0-9]+/					\
escaped = /[^-_.~\\/?&=A-Za-z0-9]/					\
\
str:									\
str: str part								\
\
part: unescaped								\
part: escaped						? escaped	\
",
				str),
		   "");
}

/*
 * escape reserved characters
 */
static string *escaped(string *parsed)
{
    int c;
    string str;

    c = parsed[0][0];
    str = "%..";
    str[1] = "01234567890ABCDEF"[c >> 4];
    str[2] = "01234567890ABCDEF"[c & 0x0f];

    return ({ str });
}
