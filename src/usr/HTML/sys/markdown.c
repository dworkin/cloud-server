# include "markdown.h"

/*
 * stripped-down markdown parser
 */
mixed *markdown(string str)
{
    return parse_string("\
nl = /\n/								\
nl_tab = /\n(\t|    )/							\
nl_tab_tab = /\n(\t\t|        )/					\
spaces_nl = /   *\n/							\
spaces_nl_tab = /   *\n(\t|    )/					\
spaces_nl_tab_tab = /   *\n(\t\t|        )/				\
code = /\n```\n(([^\n]?[^\n]?|[^`\n][^\n]*|`[^`\n][^\n]*|``[^`\n][^\n]*)\n)*```\n/									\
header = /#|##|###|####|#####|######/					\
number = /[0-9]+\\./							\
text = /[^-#0-9\\\\ \t\n]([^\\\\ \t\n]+|\\\\.)*/			\
text = /--+([^\\\\ \t\n]+|\\\\.)*/					\
text = /#######([^\\\\ \t\n]+|\\\\.)*/					\
text = /[0-9]+/								\
text = /[0-9]+[^*0-9.\\\\ \t\n]([^\\\\ \t\n]+|\\\\.)+/			\
text = /\\\\.([^\\\\ \t\n]+|\\\\.)*/					\
spaces = /[ \t]+/							" +
"\
Markdown: OptParagraphs OptNewlines					\
\
OptNewlines:								\
OptNewlines: Newlines					? null		\
\
Newlines: nl								\
Newlines: Newlines nl							\
\
Snl: nl									\
Snl: spaces_nl								\
\
OptParagraphs:								\
OptParagraphs: OptNewlines Paragraphs					\
\
Paragraphs: Para							\
Paragraphs: Paragraphs Snl nl OptNewlines Para		? paragraphs	\
Paragraphs: CodeParagraphs						\
Paragraphs: CodeParagraphs OptNewlines Para				\
\
CodeParagraphs: Code							\
CodeParagraphs: Paragraphs OptNewlines Code				\
\
Para: LineList						? paragraph1	\
Para: Para nl LineList					? paragraph2	\
Para: Para spaces_nl LineList				? paragraph3	\
\
Code: code						? code		\
\
LineList: Line								\
LineList: UList								\
LineList: NList								\
\
UList: UList nl UListItem				? addList	\
UList: UList spaces_nl UListItem			? addList	\
UList: UListItem					? ulist		\
\
NList: NList nl NListItem				? addList	\
NList: NList spaces_nl NListItem			? addList	\
NList: NListItem					? nlist		\
\
UListItem: OptSpaces '-' Paragraphs2					\
NListItem: OptSpaces number Paragraphs2					\
\
Paragraphs2: Para2							\
Paragraphs2: Paragraphs2 Snl OptNewlines nl_tab Para2	? paragraphs	\
\
Para2: LineList2					? paragraph1	\
Para2: Para2 nl_tab LineList2				? paragraph2	\
Para2: Para2 spaces_nl_tab LineList2			? paragraph3	\
\
LineList2: Line								\
LineList2: UList2							\
LineList2: NList2							\
\
UList2: UList2 nl_tab UListItem2			? addList	\
UList2: UList2 spaces_nl_tab UListItem2			? addList	\
UList2: UListItem2					? ulist		\
\
NList2: NList2 nl_tab NListItem2			? addList	\
NList2: NList2 spaces_nl_tab NListItem2			? addList	\
NList2: NListItem2					? nlist		\
\
UListItem2: OptSpaces '-' Paragraphs3					\
NListItem2: OptSpaces number Paragraphs3				\
\
Paragraphs3: Para3							\
Paragraphs3: Paragraphs3 Snl OptNewlines nl_tab_tab Para3 ? paragraphs	\
\
Para3: LineList3					? paragraph1	\
Para3: Para3 nl_tab_tab LineList3			? paragraph2	\
Para3: Para3 spaces_nl_tab_tab LineList3		? paragraph3	\
\
LineList3: Line								\
LineList3: OptSpaces '-' OptWords			? line		\
LineList3: OptSpaces number OptWords			? line		" +
"\
OptSpaces:								\
OptSpaces: spaces					? null		\
\
Line: OptSpaces header OptSpaces OptWords		? header	\
Line: OptSpaces LimitedWords				? line		\
Line: Spaces						? line		\
\
OptWords:								\
OptWords: Words								\
\
Words: Word								\
Words: Words WordSpace							\
\
Word: text								\
Word: header								\
Word: '-'								\
Word: number								\
\
LimitedWords: text OptWordsSpace					\
\
OptWordsSpace:								\
OptWordsSpace: WordsSpace						\
\
WordsSpace: WordSpace							\
WordsSpace: WordsSpace WordSpace					\
\
WordSpace: Word								\
WordSpace: Spaces							\
\
Spaces: spaces						? spaces",
			"\n" + str + "\n");
}

/*
 * nothing
 */
static mixed *null(mixed parsed)
{
    return ({ });
}

/*
 * ({ MarkdownParagraph, MarkdownParagraph, ... })
 */
static mixed *paragraphs(mixed *parsed)
{
    int size;

    size = sizeof(parsed);
    return parsed[.. size - 4] + ({ parsed[size - 1] });
}

/*
 * paragraph of one line or list
 */
static mixed *paragraph1(mixed *parsed)
{
    return ({ new MarkdownParagraph(parsed[0]) });
}

/*
 * append line or list to paragraph
 */
static mixed *paragraph2(mixed *parsed)
{
    parsed[0]->add(parsed[2]);
    return ({ parsed[0] });
}

/*
 * append line or list to paragraph, with break
 */
static mixed *paragraph3(mixed *parsed)
{
    parsed[0]->addWithBreak(parsed[2]);
    return ({ parsed[0] });
}

/*
 * fenced code block
 */
static mixed *code(mixed *parsed)
{
    int len;

    len = strlen(parsed[0]);
    return ({ new MarkdownCode(parsed[0][5 .. len - 6], TRUE) });
}

/*
 * add item to list
 */
static mixed *addList(mixed *parsed)
{
    parsed[0]->add(parsed[3 ..]);
    return ({ parsed[0] });
}

/*
 * unnumbered list
 */
static mixed *ulist(mixed *parsed)
{
    return ({ new MarkdownList(LIST_UNNUMBERED, parsed[1 ..]) });
}

/*
 * numbered list
 */
static mixed *nlist(mixed *parsed)
{
    return ({ new MarkdownList(LIST_NUMBERED, parsed[1 ..]) });
}

/*
 * line with header
 */
static mixed *header(mixed *parsed)
{
    return ({ new MarkdownLine(implode(parsed[1 ..], ""), strlen(parsed[0])) });
}

/*
 * ordinary line
 */
static mixed *line(mixed *parsed)
{
    return ({ new MarkdownLine(implode(parsed, "")) });
}

/*
 * minimize whitespace
 */
mixed *spaces(mixed *parsed)
{
    return ({ " " });
}
