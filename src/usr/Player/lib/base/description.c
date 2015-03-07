# include <type.h>


private string short_desc;	/* short description */
private string long_desc;	/* long description */
private string view_desc;	/* view description */
private mapping details;	/* descriptions of details */

/*
 * NAME:	create()
 * DESCRIPTION:	create description state
 */
static void create() { }

/*
 * NAME:	set_short_desc()
 * DESCRIPTION:	set the short description
 */
void set_short_desc(string str)
{
    short_desc = str;
}

/*
 * NAME:	set_long_desc()
 * DESCRIPTION:	set the long description
 */
void set_long_desc(string str)
{
    long_desc = str;
}

/*
 * NAME:	set_view_desc()
 * DESCRIPTION:	set the view description
 */
void set_view_desc(string str)
{
    view_desc = str;
}

/*
 * NAME:	add_detail()
 * DESCRIPTION:	set the description of a detail in/on this object
 */
void add_detail(mixed detail, string desc, varargs int plural, int scope,
		string adj...)
{
    string *strs;
    int i;

    if (!details) {
	details = ([ ]);
    }
    switch (typeof(detail)) {
    case T_STRING:
	details[detail] = ({ desc, plural, scope }) + adj;
	break;

    case T_ARRAY:
	strs = detail;
	detail = ({ desc, plural, scope }) + adj;
	for (i = sizeof(strs); --i >= 0; ) {
	    details[strs[i]] = detail;
	}
	details[nil] = nil;
	break;

    default:
	error("Bad detail");
    }
}

/*
 * NAME:	remove_detail()
 * DESCRIPTION:	remove a detail
 */
void remove_detail(string detail)
{
    if (details) {
	details[detail] = nil;
    }
}

/*
 * NAME:	query_short_desc()
 * DESCRIPTION:	return the short description
 */
string query_short_desc()
{
    return short_desc;
}

/*
 * NAME:	query_long_desc()
 * DESCRIPTION:	return the long description
 */
string query_long_desc()
{
    return long_desc;
}

/*
 * NAME:	query_view_desc()
 * DESCRIPTION:	return the view description
 */
string query_view_desc()
{
    return view_desc;
}

/*
 * NAME:	query_detail_desc()
 * DESCRIPTION:	return a detail description
 */
string query_detail_desc(string detail)
{
    mixed *data;

    if (!details) {
	return nil;
    }
    data = details[detail];
    return (data) ? data[0] : nil;
}

/*
 * NAME:	query_detail()
 * DESCRIPTION:	return detail data
 */
mixed *query_detail(string detail)
{
    mixed *data;

    if (!details) {
	return nil;
    }
    data = details[detail];
    return (data) ? data[..] : nil;
}

/*
 * NAME:	query_details()
 * DESCRIPTION:	return an array of detail names
 */
string *query_details()
{
    return (details) ? map_indices(details) : ({ });
}
