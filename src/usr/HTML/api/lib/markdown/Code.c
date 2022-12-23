# include "markdown.h"

private string code;	/* code phrase or block */
private int block;	/* phrase or block? */

/*
 * initialize code
 */
static void create(string code, varargs int block)
{
    ::code = code;
    ::block = block;
}

/*
 * emit html
 */
string html()
{
    string str;

    str = HTML_ENCODE->encode(code);
    return (block) ? "<PRE>" + str + "</PRE>" : "<CODE>" + str + "</CODE>";
}


string code()	{ return code; }
