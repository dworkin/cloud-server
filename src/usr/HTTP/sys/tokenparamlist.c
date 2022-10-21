/*
 * parse a comma-separated token-parameter list, return separate token and
 * parameter lists
 */
mixed *tokenparamlist(string str)
{
    mixed *list;
    int sz, i;
    string *values, **params;

    list = parse_string("\
whitespace = /[ \t]+/							\
token = /[-!#$%&'*+.^_`|~A-Za-z0-9]+/					\
string = /\"([^\x00-\x08\x0a-\x1f\x7f\"\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\"/										\
junk = /./								\
\
List: ListValue								\
List: List Comma ListValue						\
\
Comma: ','						? null		\
\
ListValue:								\
ListValue: token Params					? token		\
\
Params:									\
Params: Params Param							\
\
Param: ';' token '=' Value				? param		\
\
Value: token								\
Value: string",	str);

    sz = sizeof(list);
    values = allocate(sz / 2);
    params = allocate(sz / 2);
    for (i = 0; i < sz; i += 2) {
	values[i / 2] = list[i];
	params[i / 2] = list[i + 1];
    }
    return ({ values, params });
}

/*
 * ','
 */
static mixed null(mixed parsed)
{
    return ({ });
}

/*
 * token params
 */
static mixed *token(mixed *parsed)
{
    return ({ parsed[0], parsed[1 ..] });
}

/*
 * param=value
 */
static mixed *param(mixed *parsed)
{
    return ({ parsed[1] + "=" + parsed[3] });
}
