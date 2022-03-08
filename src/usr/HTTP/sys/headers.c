/*
 * ({
 *   ({ name1, field1 }),
 *   ({ name2, field2 }),
 *         ...
 * })
 */
string **headers(string str)
{
    return parse_string("\
ws = /[ \t]+/								\
obs_fold = /[ \t]*(\n[ \t]+)+/						\
token = /[-!#$%&'*+.^_`|~A-Za-z0-9]+/					\
delimiter = /[,/;<=>?@[\\\\\\]{}]/					\
string = /\"([^\x00-\x08\x0a-\x1f\x7f\"\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\"/										\
comment = /\\(([^\x00-\x08\x0a-\x1f\x7f()\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\\)/									\
junk = /./								\
\
Headers:								\
Headers: Headers Header							\
\
Header: token ':' Ows Field Ows '\n'			? header	\
Header: token ':' Ows '\n'				? emptyHeader	\
\
Field: FieldItem							\
Field: Field OwsSeparator FieldItem					\
\
FieldItem: token							\
FieldItem: delimiter							\
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
 * return header line
 */
static string **header(mixed *parsed)
{
    return ({ ({ parsed[0], implode(parsed[2 .. sizeof(parsed) - 2], "") }) });
}

/*
 * return empty header
 */
static string **emptyHeader(mixed *parsed)
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
