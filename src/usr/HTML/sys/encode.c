/*
 * make a string HTML-ready
 */
string encode(string str)
{
    return implode(parse_string("\
token = /[^&\"'<>]+/							\
\
Text: 									\
Text: Text token							\
Text: Text Char								\
\
Char: '&'						? amp		\
Char: '\"'						? quot		\
Char: '\\''						? apos		\
Char: '<'						? lt		\
Char: '>'						? gt", str), "");
}

/*
 * &
 */
static mixed *amp(mixed parsed)
{
    return ({ "&amp;" });
}

/*
 * "
 */
static mixed *quot(mixed parsed)
{
    return ({ "&quot;" });
}

/*
 * '
 */
static mixed *apos(mixed parsed)
{
    return ({ "&apos;" });
}

/*
 * <
 */
static mixed *lt(mixed parsed)
{
    return ({ "&lt;" });
}

/*
 * >
 */
static mixed *gt(mixed parsed)
{
    return ({ "&gt;" });
}
