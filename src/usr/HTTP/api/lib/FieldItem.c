# include <type.h>

/*
 * Base component of all fields and field items.
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
 * transport an entire item, field or fields
 */
string transport();
