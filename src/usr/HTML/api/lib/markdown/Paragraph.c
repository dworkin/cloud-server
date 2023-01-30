# include "markdown.h"

inherit "blockquote";


private object *elements;	/* elements of the paragraph */

/*
 * create a paragraph of one element
 */
static void create(object element)
{
    ::elements = ({ element });
}

/*
 * add an element to the paragraph
 */
void add(object element)
{
    elements += ({ element });
}

/*
 * add an element to the paragraph, with a line break
 */
void addWithBreak(object element)
{
    elements += ({ nil, element });
}

/*
 * emit as HTML
 */
string html()
{
    int sz, i;
    string str, *strs;

    startLevel();
    sz = sizeof(elements);
    strs = allocate(sz);
    for (i = 0; i < sz; i++) {
	strs[i] = (elements[i]) ? elements[i]->html() : "<BR>\n";
    }

    return "<P STYLE=\"clear: left\">" + implode(strs, " ") + setLevel(0) +
	   "\n";
}


object *elements()	{ return elements; }
