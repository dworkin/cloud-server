# include <String.h>


object strDecode;	/* JSON string decoder */

/*
 * initialize JSON decoder
 */
static void create()
{
    strDecode = find_object("/sys/jsonstrdecode");
}

/*
 * Simple JSON decoder to native LPC values.
 */
mixed decode(string str)
{
    mixed *result;

    result = parse_string("\
string = /\"([^\"\\\\]|\\\\.)*\"/					\
int = /-?[0-9]+/							\
float = /-?[0-9]+\\.[0-9]+([Ee][-+]?[0-9]+)?/				\
whitespace = /[ \t\n\r]+/						\
invalid = /./								\
\
Value: Object								\
Value: Array								\
Value: string						? decodeString	\
Value: int						? decodeInt	\
Value: float						? decodeFloat	\
Value: 'true'						? decodeTrue	\
Value: 'false'						? decodeFalse	\
Value: 'null'						? decodeNull	\
\
Object: '{' Members '}'					? objectMembers	\
Object: '{' '}'						? emptyObject	\
\
Members: Members ',' Member				? members	\
Members: Member						? member	\
\
Member: string ':' Value						\
\
Array: '[' Elements ']'					? arrayElements	\
Array: '[' ']'						? emptyArray	\
\
Elements: Elements ',' Value				? elements	\
Elements: Value						? element",
			  str);

    if (!result) {
	error("Invalid JSON");
    }
    return result[0];
}

/*
 * decode JSON string
 */
static mixed *decodeString(mixed *parsed)
{
    return ({ strDecode->decode(parsed[0])->utf8() });
}

/*
 * decode JSON number to int
 */
static mixed *decodeInt(mixed *parsed)
{
    return ({ (int) parsed[0] });
}

/*
 * decode JSON number to float
 */
static mixed *decodeFloat(mixed *parsed)
{
    return ({ (float) parsed[0] });
}

/*
 * decode JSON true to TRUE
 */
static mixed *decodeTrue(mixed parsed)
{
    return ({ TRUE });
}

/*
 * decode JSON false to FALSE
 */
static mixed *decodeFalse(mixed parsed)
{
    return ({ FALSE });
}

/*
 * decode JSON null to nil
 */
static mixed *decodeNull(mixed parsed)
{
    return ({ nil });
}

/*
 * JSON object with members
 */
static mixed *objectMembers(mixed *parsed)
{
    return ({ parsed[1] });
}

/*
 * empty JSON object
 */
static mixed *emptyObject(mixed *parsed)
{
    return ({ ([ ]) });
}

/*
 * add JSON object members to mapping
 */
static mixed *members(mixed *parsed)
{
    mapping members;
    string key;

    members = parsed[0];
    key = strDecode->decode(parsed[2])->utf8();
    if (members[key]) {
	error("Duplicate name in JSON object");
    }
    members[key] = parsed[4];

    return ({ members });
}

/*
 * create mapping for JSON object
 */
static mixed *member(mixed *parsed)
{
    return ({ ([ strDecode->decode(parsed[0])->utf8() : parsed[2] ]) });
}

/*
 * JSON array with elements
 */
static mixed *arrayElements(mixed *parsed)
{
    return ({ map_values(parsed[1]) });
}

/*
 * empty JSON array
 */
static mixed *emptyArray(mixed parsed)
{
    return ({ ({ }) });
}

/*
 * add elements to JSON array
 */
static mixed *elements(mixed *parsed)
{
    mapping elements;

    elements = parsed[0];
    elements[map_sizeof(elements)] = parsed[2];

    return ({ elements });
}

/*
 * create mapping for JSON array
 */
static mixed element(mixed *parsed)
{
    return ({ ([ 0 : parsed[0] ]) });
}
