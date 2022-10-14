/*
 * parse a comma-separated token list
 */
mixed tokenlist(string str)
{
    return parse_string("\
whitespace = /[ \t]+/							\
token = /[-!#$%&'*+.^_`|~A-Za-z0-9]+/					\
junk = /./								\
\
List: ListValue								\
List: List Comma ListValue						\
\
Comma: ','						? null		\
\
ListValue:								\
ListValue: token",	str);
}

/*
 * ','
 */
static mixed *null(mixed parsed)
{
    return ({ });
}
