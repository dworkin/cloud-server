# include <type.h>

/*
 * Base component of all fields and field items.
 */

/*
 * transport part of an item
 */
static string transportValue(mixed item, varargs mixed *params)
{
    string str;
    mixed *values;
    int i, sz;

    switch (typeof(item)) {
    case T_NIL:
	str = "";
	break;

    case T_INT:
    case T_FLOAT:
	str = (string) item;
	break;

    case T_STRING:
	str = item;
	break;

    case T_OBJECT:
	str = item->transport();
	break;

    case T_ARRAY:
	values = allocate(sz = sizeof(item));
	for (i = 0; i < sz; i++) {
	    values[i] = transportValue(item[i]);
	    if (params && params[i] && sizeof(params[i]) != 0) {
		values[i] += ";" + implode(params[i], ";");
	    }
	}
	return implode(values, ", ");

    default:
	error("Unhandled value in transport");
    }

    if (params && sizeof(params) != 0) {
	str += ";" + implode(params, ";");
    }
    return str;
}

/*
 * transport an entire item, field or fields
 */
string transport();
