/*
 * parse a comma-separated list
 */
mixed *list(string str)
{
    return parse_string("\
ws = /[ \t]+/								\
token = /[-!#$%&'*+.^_`|~A-Za-z0-9]+/					\
delimiters = /[/:;<=>?@[\\\\\\]{}]+/					\
string = /\"([^\x00-\x08\x0a-\x1f\x7f\"\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\"/										\
comment = /\\(([^\x00-\x08\x0a-\x1f\x7f()\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\\)/									\
junk = /./								\
\
List: ListValue Ows							\
List: List Comma ListValue Ows						\
\
Comma: ','						? null		\
\
ListValue: Ows								\
ListValue: Ows Value					? value		\
\
Value: Item								\
Value: Value OwsSeparator Item						\
\
Item: token								\
Item: delimiters							\
Item: string								\
Item: comment								\
\
Ows:									\
Ows: ws							? null		\
\
OwsSeparator:								\
OwsSeparator: ws							\
",
			str);
}

/*
 * whitespace or ','
 */
static mixed *null(mixed parsed)
{
    return ({ });
}

/*
 * list value
 */
static mixed *value(mixed *parsed)
{
    return ({ implode(parsed, "") });
}

