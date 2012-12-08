# include "~/open/include/http.h"


/*
 * NAME:	parse_header()
 * DESCRIPTION:	parse HTTP headers
 */
mixed *parse_header(string header)
{
    mixed *result;

    result = parse_string("\
whitespace = /[ \t]+|\n[ \t]+/						\
name = /[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+:/			\
token = /[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+/			\
string = /\"([^\x00-\x08\x0a-\x1f\x7f\"\\\\]|\n[ \t]+|\\\\[^\x00-\x08\x0a-\x1f\x7f])*\"/								\
comment = /\\(([^\x00-\x08\x0a-\x1f\x7f]|\n[ \t]+)*/			\
type = /[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+\\/[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+/							\
token_slash = /[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+\\//	\
slash_token = /\\/[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+/	\
assign = /[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+=[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+/							\
token_assign = /[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+=/		\
assign_token = /=[^\x00-\x20\x7f-\xff()<>@,;:\\\\\"/[\\]?={}]+/		\
funny = /[^\x00-\x20\x7f-\xff(),;\"]*[<>@:\\\\/[\\]?={}][^\x00-\x20\x7f-\xff(),;\"]*/									\
junk = /./								\
"+"\
Headers:								\
Headers:	Headers Header '\n'					\
"+"\
Header:		name List				? header	\
"+"\
List:		Items							\
List:		List ',' Items 				? list		\
"+"\
Items:		ItemsP StartItems MidItems EndItems	? items		\
"+"\
ItemsP:									\
ItemsP:		ItemsP ItemP						\
"+"\
ItemP:		StartItemP						\
ItemP:		StartItems MidItems MidItemP				\
ItemP:		StartItems MidItems EndItems EndItemP			\
"+"\
StartItems:								\
StartItems:	StartItems StartItem					\
"+"\
StartItemP:	StartItem Params			? param		\
"+"\
StartItem:	'/'					? token		\
StartItem:	'='					? token		\
StartItem:	slash_token				? token		\
StartItem:	assign_token				? token		\
"+"\
MidItems:								\
MidItems:	MidItems MidItem					\
"+"\
MidItemP:	MidItem Params				? param		\
"+"\
MidItem:	Item							\
MidItem:	token_slash String			? strange1	\
MidItem:	'/' String				? strange1	\
MidItem:	String slash_token			? strange2	\
MidItem:	String assign_token			? strange2	\
MidItem:	String '/'				? strange2	\
MidItem:	String '='				? strange2	\
"+"\
Item:		token					? token		\
Item:		name					? token		\
Item:		funny					? token		\
Item:		String					? stritem	\
Item:		comment					? comment	\
Item:		type					? type		\
Item:		token slash_token			? type		\
Item:		token_slash token			? type		\
Item:		token '/' token				? type		\
Item:		Assignment						\
"+"\
String:		string					? str		\
"+"\
Assignment:	assign					? assign	\
Assignment:	token assign_token			? assign	\
Assignment:	token_assign token			? assign	\
Assignment:	token_assign String			? assignstr	\
Assignment:	token '=' token				? assign	\
Assignment:	token '=' String			? assignstr	\
"+"\
EndItemP:	EndItem Params				? param		\
"+"\
EndItems:								\
EndItems:	EndItems EndItem					\
"+"\
EndItem:	'/'					? token		\
EndItem:	'='					? token		\
EndItem:	token_slash				? token		\
EndItem:	token_assign				? token		\
"+"\
Params:		Param							\
Params:		Params Param						\
"+"\
Param:		';' Item						\
",
			  header);
    return (result) ? result - ({ "\n" }) : nil;
}

/*
 * NAME:	header()
 * DESCRIPTION:	({ name }) or ({ name, value })
 */
static mixed *header(mixed *header)
{
    string str;

    str = header[0];
    header[0] = str[.. strlen(str) - 2];
    return ({ header });
}

/*
 * NAME:	list()
 * DESCRIPTION:	({ HTTPHDR_LIST, items, items, ... })
 */
static mixed *list(mixed *list)
{
    list -= ({ "," });
    if (sizeof(list) > 1) {
	if (list[0][0] == HTTPHDR_LIST) {
	    return ({ list[0] + list[1 ..] });
	} else {
	    return ({ ({ HTTPHDR_LIST }) + list });
	}
    } else {
	return list;
    }
}

/*
 * NAME:	items()
 * DESCRIPTION:	({ HTTPHDR_ITEMS, item, item, ... })
 */
static mixed *items(mixed *items)
{
    if (sizeof(items) > 1) {
	return ({ ({ HTTPHDR_ITEMS }) + items });
    } else {
	return items;
    }
}

/*
 * NAME:	token()
 * DESCRIPTION:	({ HTTPHDR_TOKEN, token })
 */
static mixed *token(string *token)
{
    return ({ ({ HTTPHDR_TOKEN, token[0] }) });
}

/*
 * NAME:	str()
 * DESCRIPTION:	remove quotes and backslashes from string
 */
static mixed *str(string *str)
{
    string result, head, tail;

    result = "";
    head = str[0];
    while (sscanf(head, "%s\\%s", head, tail) != 0) {
	result += head + tail[0 .. 0];
	head = tail[1 ..];
    }
    result += head;
    str[0] = result[1 .. strlen(result) - 2];
    return str;
}

/*
 * NAME:	stritem()
 * DESCRIPTION:	({ HTTPHDR_STRING, string })
 */
static mixed *stritem(string *stritem)
{
    return ({ ({ HTTPHDR_STRING }) + stritem });
}

/*
 * NAME:	type()
 * DESCRIPTION:	({ HTTPHDR_TYPE, type, subtype })
 */
static mixed *type(mixed *type)
{
    string main, sub;

    sscanf(implode(type, ""), "%s/%s", main, sub);
    return ({ ({ HTTPHDR_TYPE, main, sub }) });
}

/*
 * NAME:	assign()
 * DESCRIPTION:	({ HTTPHDR_ASSIGN, name, value })
 */
static mixed *assign(mixed *assign)
{
    string name, value;

    sscanf(implode(assign, ""), "%s=%s", name, value);
    return ({ ({ HTTPHDR_ASSIGN, name, value }) });
}

/*
 * NAME:	assignstr()
 * DESCRIPTION:	({ HTTPHDR_ASSIGNSTR, name, str })
 */
static mixed *assignstr(mixed *assignstr)
{
    string name, str;

    sscanf(implode(assignstr, ""), "%s=%s", name, str);
    return ({ ({ HTTPHDR_ASSIGNSTR, name, str }) });
}

/*
 * NAME:	param()
 * DESCRIPTION:	attach parameters to item
 */
static mixed *param(mixed *param)
{
    int i;

    i = (param[1] == ";") ? 1 : 2;
    return param[.. i - 2] + ({ param[i - 1] + (param[i + 1 ..] - ({ ";" })) });
}

/*
 * NAME:	strange1()
 * DESCRIPTION:	token_xxx string
 */
static mixed *strange1(mixed *strange1)
{
    return ({ ({ HTTPHDR_TOKEN, strange1[0] }),
	      ({ HTTPHDR_STRING, strange1[1] }) });
}

/*
 * NAME:	strange2()
 * DESCRIPTION:	string xxx_token, string xxx
 */
static mixed *strange2(mixed *strange2)
{
    return ({ ({ HTTPHDR_STRING, strange2[0] }),
	      ({ HTTPHDR_TOKEN, strange2[1] }) });
}

/*
 * NAME:	comment()
 * DESCRIPTION:	({ HTTPHDR_COMMENT, comment })
 */
static mixed *comment(string *comment)
{
    return ({ ({ HTTPHDR_COMMENT }) + comment });
}
