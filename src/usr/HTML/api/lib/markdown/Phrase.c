# include "markdown.h"
# include <type.h>


private int style;		/* style of phrase */
private mixed *elements;	/* elements in phrase */

/*
 * create phrase
 */
static void create(int style, mixed *elements)
{
    ::style = style;
    ::elements = elements;
}

/*
 * emit as HTML
 */
string html()
{
    object encoder;
    mixed *strs;
    int sz, i;
    string html;

    encoder = find_object(HTML_ENCODE);
    strs = elements[..];
    for (sz = sizeof(strs), i = 0; i < sz; i++) {
	strs[i] = (typeof(strs[i]) == T_OBJECT) ?
		   strs[i]->html() : encoder->encode(strs[i]);
    }

    html = implode(strs, "");
    switch (style) {
    case STYLE_BOLD:
	html = "<STRONG>" + html + "</STRONG>";
	break;

    case STYLE_ITALICS:
	html = "<EM>" + html + "</EM>";
	break;

    case STYLE_BOLD_ITALICS:
	html = "<STRONG><EM>" + html + "</EM></STRONG>";
	break;
    }

    return html;
}


int style()		{ return style; }
mixed *elements()	{ return elements; }
