# include <type.h>

private inherit hex "/lib/util/hex";


/*
 * encode string as JSON
 */
private string strEncode(string str)
{
    return "\"" +
	   implode(parse_string("\
escaped = /[\x00-\x1f\"\\\\]+/						\
normal = /[^\x00-\x1f\"\\\\]+/						\
\
String: String Escaped							\
String: String normal							\
String:									\
\
Escaped: escaped					? escaped",
				str), "") +
	   "\"";
}

/*
 * encode escaped character sequence
 */
static mixed *escaped(mixed *parsed)
{
    string str, *result;
    int len, i;

    str = parsed[0];
    len = strlen(str);
    result = allocate(len);
    for (i = 0; i < len; i++) {
	switch (str[i]) {
	case '\b':
	    result[i] = "\\b";
	    break;

	case '\f':
	    result[i] = "\\f";
	    break;

	case '\n':
	    result[i] = "\\n";
	    break;

	case '\r':
	    result[i] = "\\r";
	    break;

	case '\t':
	    result[i] = "\\t";
	    break;

	case '"':
	    result[i] = "\\\"";
	    break;

	case '\\':
	    result[i] = "\\\\";
	    break;

	default:
	    result[i] = "\\u00" + hex::encode(str[i], 2);
	    break;
	}
    }

    return result;
}


/*
 * encode an LPC value as JSON
 */
string encode(mixed value)
{
    string str, *result;
    int i, sz;
    mixed *indices, *values;

    switch (typeof(value)) {
    case T_NIL:
	return "null";

    case T_INT:
	return (string) value;

    case T_FLOAT:
	str = (string) value;
	if (sscanf(str, "%*s.") == 0 && sscanf(str, "%*se") == 0) {
	    if (value >= 0.0) {
		value += .05;
		str = (string) floor(value);
	    } else {
		value -= .05;
		str = (string) ceil(value);
	    }
	    str += "." + floor(fmod(fabs(value) * 10.0, 10.0));
	}
	return str;

    case T_STRING:
	return strEncode(value);

    case T_ARRAY:
	sz = sizeof(value);
	result = allocate(sz);
	for (i = 0; i < sz; i++) {
	    result[i] = encode(value[i]);
	}
	return "[" + implode(result, ",") + "]";

    case T_MAPPING:
	sz = map_sizeof(value);
	indices = map_indices(value);
	values = map_values(value);
	result = allocate(sz);
	for (i = 0; i < sz; i++) {
	    if (typeof(indices[i]) != T_STRING) {
		error("Invalid member name");
	    }
	    result[i] = strEncode(indices[i]) + ":" + encode(values[i]);
	}
	return "{" + implode(result, ",") + "}";

    default:
	error("Non-JSON value");
    }
}
