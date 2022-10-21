# include "HttpField.h"

/*
 * return credentials
 */
mixed *authorize(string str)
{
    return parse_string("\
ws = /[ \t]+/								\
token66 = /[-+._~A-za-z0-9]+/						\
extra = /[!#$%&'*^`|]+/							\
string = /\"([^\x00-\x08\x0a-\x1f\x7f\"\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\"/										\
junk = /./								\
\
Credentials: Token							\
Credentials: Token ws Token68				? credentials1	\
Credentials: Token ws Params				? credentials2	\
\
Ows:									\
Ows: ws							? null		\
\
Comma: ','						? null		\
\
Token: TokenParts					? token		\
\
TokenParts: TokenPart							\
TokenParts: TokenParts TokenPart					\
\
TokenPart: token66							\
TokenPart: extra							\
\
Token68: T68Parts T68Tail				? token		\
\
T68Parts: T68Part							\
T68Parts: T68Parts T68Part						\
\
T68Part: token66							\
T68Part: '/'								\
\
T68Tail:								\
T68Tail: T68Tail '='							\
\
Params: Param								\
Params: Params Ows Comma Ows Param					\
\
Param: Token Ows '=' Ows Value				? token		\
\
Value: Token								\
Value: string", str);
}

/*
 * scheme token68
 */
static mixed *credentials1(mixed *parsed)
{
    return ({ parsed[0], parsed[2] });
}

/*
 * scheme param1=value1, param2=value2
 */
static mixed *credentials2(mixed *parsed)
{
    return ({ parsed[0], parsed[2 ..] });
}

/*
 * nothing
 */
static mixed null(mixed parsed)
{
    return ({ });
}

/*
 * token
 * token68
 * param=value
 */
static mixed *token(mixed *parsed)
{
    return ({ implode(parsed, "") });
}
