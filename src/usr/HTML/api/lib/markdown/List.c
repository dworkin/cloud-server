# include "markdown.h"


private int style;			/* list style */
private MarkdownParagraph **items;	/* items in list */

/*
 * create a list with one item
 */
static void create(int style, MarkdownParagraph *item)
{
    ::style = style;
    items = ({ item });
}

/*
 * add an item to a list
 */
void add(MarkdownParagraph *item)
{
    items += ({ item });
}

/*
 * emit as HTML
 */
string html()
{
    string *strs, *p, html;
    int sz, i, size, j;
    MarkdownParagraph *paragraphs;

    sz = sizeof(items);
    strs = allocate(sz);
    for (i = 0; i < sz; i++) {
	paragraphs = items[i];
	size = sizeof(paragraphs);
	p = allocate(size);
	for (j = 0; j < size; j++) {
	    p[j] = paragraphs[j]->html();
	}
	strs[i] = "<LI>" + implode(p, "") + "</LI>";
    }
    html = implode(strs, "");

    if (style == LIST_UNNUMBERED) {
	html = "<UL>" + html + "</UL>";
    } else {
	html = "<OL>" + html + "</OL>";
    }
    return html;
}


int style()			{ return style; }
MarkdownParagraph **items()	{ return items; }
