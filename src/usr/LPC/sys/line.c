# include "compiler.h"


object stringParser;    /* string parser */

/*
 * initialize LPC parser
 */
static void create()
{
    stringParser = find_object(LPC_STRING_PARSER);
}

/*
 * #line 100
 */
int line(string str)
{
    return parse_string("\
ident = /[a-zA-Z_][a-zA-Z_0-9]*/					\
string = /\"[^\"\n]*\"/							\
decimal = /[1-9][0-9]*/							\
octal = /0[0-7]*/							\
hexadecimal = /0[xX][a-fA-F0-9]+/					\
whitespace = /([ \t\v\f\r\n]|\\/\\*([^*]*\\*+[^/*])*[^*]*\\*+\\/)+/	" +
"\
Line: '#' 'line' Number							\
Number: decimal						? decimal	\
Number: octal						? octal		\
Number: hexadecimal					? hexadecimal	",
			str)[2];
}

/*
 * decimal number
 */
static mixed *decimal(mixed *parsed)
{
    return ({ (int) parsed[0] });
}

/*
 * octal number
 */
static mixed *octal(mixed *parsed)
{
    return ({ stringParser->octalInt(parsed[0]) });
}

/*
 * hexadecimal number
 */
static mixed *hexadecimal(mixed *parsed)
{
    return ({ stringParser->hexadecimalInt(parsed[0]) });
}
