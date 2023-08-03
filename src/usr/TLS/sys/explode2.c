/*
 * return array of two-char strings
 */
string *explode2(string str)
{
    return parse_string("\
token = /../								\
\
Tokens: token								\
Tokens: Tokens token", str);
}
