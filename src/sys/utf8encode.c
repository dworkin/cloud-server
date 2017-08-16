/*
 * encode a binary string as UTF8
 */
string encode(string str)
{
    return implode(parse_string("\
lower = /[\x00-\x7f]+/							\
upper1char = /[\x80-\xbf]/						\
upper2char = /[\xc0-\xff]/						\
upper1str = /[\x80-\xbf]+/						\
upper2str = /[\xc0-\xff]+/						\
\
Binary:									\
Binary: Binary lower							\
Binary: Binary Upper							\
\
Upper: upper1char		? upper1char				\
Upper: upper2char		? upper2char				\
Upper: upper1str		? upper1str				\
Upper: upper2str		? upper2str", str), "");
}

/*
 * encode single \x80-\xbf character
 */
static mixed *upper1char(mixed *parsed)
{
    string str;

    str = "\xc2 ";
    str[1] = parsed[0][0];
    return ({ str });
}

/*
 * encode single \xc0-\xff character
 */
static mixed *upper2char(mixed *parsed)
{
    string str;

    str = "\xc3 ";
    str[1] = parsed[0][0] & ~0x40;
    return ({ str });
}

/*
 * encode \x80-\xbf sequence
 */
static mixed *upper1str(mixed *parsed)
{
    return ({ "\xc2" + implode(explode(parsed[0], ""), "\xc2") });
}

/*
 * encode \xc0-\xff sequence
 */
static mixed *upper2str(mixed *parsed)
{
    string *upper;
    int i;

    upper = explode(parsed[0], "");
    for (i = sizeof(upper); i > 0; ) {
	upper[--i][0] &= ~0x40;
    }
    return ({ "\xc3" + implode(upper, "\xc3") });
}
