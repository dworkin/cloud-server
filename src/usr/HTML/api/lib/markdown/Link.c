# include "markdown.h"
# include <type.h>


private string url;		/* URL linked to */
private mixed *elements;	/* elements of the description */
private string comment;		/* optional comment */
private int image;		/* is this an image link? */

/*
 * create the link
 */
void create(string url, mixed *elements, varargs string comment)
{
    ::url = url;
    ::elements = elements;
    ::comment = comment;
}

/*
 * mark this as an image link
 */
void setImage()
{
    image = TRUE;
}

/*
 * emit as HTML
 */
string html()
{
    object encoder;
    mixed *strs;
    int sz, i;
    string desc, html;

    encoder = find_object(HTML_ENCODE);
    strs = elements[..];
    for (sz = sizeof(strs), i = 0; i < sz; i++) {
	strs[i] = (typeof(strs[i]) == T_OBJECT) ?
		   strs[i]->html() : encoder->encode(strs[i]);
    }
    desc = implode(strs, "");

    if (image) {
	html = "<IMG STYLE=\"float: left\" SRC=\"" + url + "\" ALT=\"" + desc +
	       "\"";
	if (comment) {
	    html += " TITLE=\"" + comment + "\"";
	}
	html += "/>";
    } else {
	html = "<A HREF=\"" + url + "\"";
	if (comment) {
	    html += " TITLE=\"" + comment + "\"";
	}
	html += ">" + desc + "</A>";
    }

    return html;
}


string url()		{ return url; }
mixed *elements()	{ return elements; }
string comment()	{ return comment; }
int image()		{ return image; }
