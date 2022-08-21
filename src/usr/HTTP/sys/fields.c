/*
 * ({
 *   ({ name1, items }),
 *   ({ name2, items }),
 *         ...
 * })
 */
string **fields(string str)
{
    return parse_string("\
ws = /[ \t]+/								\
obs_fold = /[ \t]*(\n[ \t]+)+/						\
token = /[-!#$%&'*+.^_`|~A-Za-z0-9]+/					\
delimiters = /[,/;<=>?@[\\\\\\]{}]+/					\
string = /\"([^\x00-\x08\x0a-\x1f\x7f\"\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\"/										\
comment = /\\(([^\x00-\x08\x0a-\x1f\x7f()\\\\]|\\(([^\x00-\x08\x0a-\x1f\x7f()\\\\]|\\(([^\x00-\x08\x0a-\x1f\x7f()\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\\)|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\\)|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\\)/	\
junk = /./								\
\
Fields:									\
Fields: Fields Field							\
\
Field: token ':' Ows FieldItems Ows '\n'		? field		\
Field: token ':' Ows '\n'				? emptyField	\
\
FieldItems: FieldItem							\
FieldItems: FieldItems OwsSeparator FieldItem				\
\
FieldItem: token							\
FieldItem: delimiters							\
FieldItem: ':'								\
FieldItem: string							\
FieldItem: comment							\
\
Ows:									\
Ows: ws							? null		\
Ows: obs_fold						? null		\
\
OwsSeparator:								\
OwsSeparator: ws					? whitespace	\
OwsSeparator: obs_fold					? whitespace	\
",
			str);
}

/*
 * return field line
 */
static string **field(mixed *parsed)
{
    return ({ ({ parsed[0], implode(parsed[2 .. sizeof(parsed) - 2], "") }) });
}

/*
 * return empty field
 */
static string **emptyField(mixed *parsed)
{
    return ({ ({ parsed[0], nil }) });
}

/*
 * return nothing
 */
static string *null(mixed parsed)
{
    return ({ });
}

/*
 * return canonical whitespace
 */
static string *whitespace(mixed parsed)
{
    return ({ " " });
}
