# include "markdown.h"
# include <type.h>


# define LINE	"/usr/HTML/sys/line"

private int header;		/* header level */
private string id;		/* header ID */
private mixed *elements;	/* elements of the line */

/*
 * create a line
 */
static void create(string line, varargs int header)
{
    string str, name, remainder;

    ::header = header;
    if (header && sscanf(line, "%s {#%s}%s", str, name, remainder) == 3 &&
	strlen(remainder) == 0) {
	line = str;
	id = name;
    }
    elements = LINE->line(line);
    if (!elements) {
	error("Failed to parse markdown line");
    }
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
		   strs[i]->html() :
		   encoder->encode(strs[i]);
    }

    html = implode(strs, "");
    if (header != 0) {
	html = "<H" + header + ((id) ? " ID=\"" + id + "\">" : ">") + html +
	       "</H" + header + ">";
    }
    return html + "\n";
}


int header()		{ return header; }
string id()		{ return id; }
mixed *elements()	{ return elements; }
