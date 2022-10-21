/*
 * type/subtype params
 */
string *typeparam(string str)
{
    return parse_string("\
ws = /[ \t]+/								\
token = /[-!#$%&'*+.^_`|~A-Za-z0-9]+/					\
string = /\"([^\x00-\x08\x0a-\x1f\x7f\"\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\"/										\
junk = /./								\
\
Type: token '/' token Params				? type		\
\
Params:									\
Params: Params Param							\
\
Param: Ows ';' Ows token Ows '=' Ows Value		? param		\
\
Ows:									\
Ows: ws							? null		\
\
Value: token								\
Value: string", str);
}

/*
 * type/subtype params
 */
static mixed *type(mixed *parsed)
{
    return ({ parsed[0] + "/" + parsed[2], parsed[3 ..] });
}

/*
 * param=value
 */
static mixed *param(mixed *parsed)
{
    return ({ parsed[1] + "=" + parsed[3] });
}

/*
 * nothing
 */
static mixed null(mixed parsed)
{
    return ({ });
}
