# include "markdown.h"

/*
 * stripped-down markdown line parser
 */
mixed *line(string str)
{
    return parse_string("\
escaped = /\\\\./							\
string = /\"([^\"\\\\]|\\\\.)*\"/					\
code = /`[^`]+`/							\
code2 = /``[^`]+(`[^`]+)*``/						\
star1 = /\\*/								\
star2 = /\\*\\*/							\
star3 = /\\*\\*\\*/							\
space = / /								\
token = /([^\\\\\"`\\* [\\]()!]|```|\\*\\*\\*\\*+)+/			" +
"\
Line: OptWordsSpace							\
Line: Line Phrase OptWordsSpace						\
\
Phrase: star1 Line1 star1				? phrase1	\
Phrase: star2 Line2 star2				? phrase2	\
Phrase: star3 Line3 star3				? phrase3	\
Phrase: code						? code		\
Phrase: code2						? code2		\
Phrase: Link								\
Phrase: Image								\
\
Link: '[' Line LinkTail					? link		\
\
LinkTail: ']' '(' OptSpace token OptComment OptSpace ')'		\
\
Image: '!' Link						? image		\
\
OptComment:						? nocomment	\
OptComment: space string				? comment	" +
"\
Line1: OptWordsSpace							\
Line1: Line1 Phrase1 OptWordsSpace					\
\
Phrase1: star2 Line3 star2				? phrase2	\
Phrase1: star3 Line3 star3				? phrase3	\
Phrase1: code						? code		\
Phrase1: code2						? code2		\
Phrase1: Link1								\
Phrase1: Image1								\
\
Link1: '[' Line1 LinkTail				? link		\
\
Image1: '!' Link1					? image		" +
"\
Line2: OptWordsSpace							\
Line2: Line2 Phrase2 OptWordsSpace					\
\
Phrase2: star1 Line3 star1				? phrase1	\
Phrase2: star3 Line3 star3				? phrase3	\
Phrase2: code						? code		\
Phrase2: code2						? code2		\
Phrase2: Link2								\
Phrase2: Image2								\
\
Link2: '[' Line2 LinkTail				? link		\
\
Image2: '!' Link2					? image		" +
"\
Line3: OptWordsSpace							\
Line3: Line3 Phrase3 OptWordsSpace					\
\
Phrase3: code						? code		\
Phrase3: code2						? code2		\
Phrase3: Link3								\
Phrase3: Image3								\
\
Link3: '[' Line3 LinkTail				? link		\
\
Image3: '!' Link3					? image		\
\
OptWordsSpace:								\
OptWordsSpace: WordsSpace				? words		" +
"\
WordsSpace: WordSpace							\
WordsSpace: WordsSpace WordSpace					\
\
WordSpace: Word								\
WordSpace: space							\
\
Word: token								\
Word: escaped						? escaped	\
Word: string								\
Word: '('								\
Word: ')'								\
\
OptSpace:								\
OptSpace: space						? null",
			str);
}

/*
 * phrase in italics
 */
static mixed *phrase1(mixed *parsed)
{
    return ({
	new MarkdownPhrase(STYLE_ITALICS, parsed[1 .. sizeof(parsed) - 2])
    });
}

/*
 * phrase in bold
 */
static mixed *phrase2(mixed *parsed)
{
    return ({
	new MarkdownPhrase(STYLE_BOLD, parsed[1 .. sizeof(parsed) - 2])
    });
}

/*
 * phrase in bold italics
 */
static mixed *phrase3(mixed *parsed)
{
    return ({
	new MarkdownPhrase(STYLE_BOLD_ITALICS, parsed[1 .. sizeof(parsed) - 2])
    });
}

/*
 * HTTP link
 */
static mixed *link(mixed *parsed)
{
    int size;

    size = sizeof(parsed);
    return ({
	new MarkdownLink(parsed[size - 3], parsed[1 .. size - 6],
			 parsed[size - 2])
    });
}

/*
 * image link
 */
static mixed *image(mixed *parsed)
{
    parsed[1]->setImage();
    return ({ parsed[1] });
}

/*
 * link without comment
 */
static mixed *nocomment(mixed parsed)
{
    return ({ nil });
}

/*
 * link comment
 */
static mixed *comment(mixed *parsed)
{
    int len;

    len = strlen(parsed[1]);
    return ({ parsed[1][1 .. len - 2] });
}

/*
 * concatenate words
 */
static mixed *words(mixed *parsed)
{
    return ({ implode(parsed, "") });
}

/*
 * escaped character
 */
static mixed *escaped(mixed *parsed)
{
    return ({ parsed[0][1 .. 1] });
}

/*
 * code phrase
 */
static mixed *code(mixed *parsed)
{
    int len;

    len = strlen(parsed[0]);
    return ({ new MarkdownCode(parsed[0][1 .. len - 2]) });
}

/*
 * code phrase
 */
static mixed *code2(mixed *parsed)
{
    int len;

    len = strlen(parsed[0]);
    return ({ new MarkdownCode(parsed[0][2 .. len - 3]) });
}

/*
 * nothing
 */
static mixed *null(mixed parsed)
{
    return ({ });
}
