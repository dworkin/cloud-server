# include <type.h>

/*
 * Base component of all headers and header items.
 */

/*
 * transport part of an item
 */
static string transportValue(mixed item)
{
    string *names;
    mixed *values;
    int i, sz;

    switch (typeof(item)) {
    case T_NIL:
	return "";

    case T_INT:
    case T_FLOAT:
	return (string) item;

    case T_STRING:
	return item;

    case T_OBJECT:
	return item->transport();

    case T_ARRAY:
	values = allocate(sz = sizeof(item));
	for (i = 0; i < sz; i++) {
	    values[i] = transportValue(item[i]);
	}
	return implode(values, ", ");

    default:
	error("Unhandled value in transport");
    }
}

/*
 * transport an entire item, header or headers
 */
string transport();
