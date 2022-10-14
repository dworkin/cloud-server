/*
 * parse a token-parameter
 */
mixed tokenparam(string str)
{
    return parse_string("\
whitespace = /[ \t]+/							\
token = /[-!#$%&'*+.^_`|~A-Za-z0-9]+/					\
string = /\"([^\x00-\x08\x0a-\x1f\x7f\"\\\\]|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\"/										\
junk = /./								\
\
TokenParam: token ParamList						\
\
ParamList:						? empty		\
ParamList: ParamList ';' token '=' Param		? paramList	\
\
Param: token								\
Param: string",	str);
}

/*
 * empty parameter list
 */
static mixed *empty(mixed parsed)
{
    return ({ ({ }) });
}

/*
 * parameter list
 */
static mixed *paramList(mixed *parsed)
{
    return ({ parsed[0] + ({ parsed[2] + "=" + parsed[4] }) });
}
