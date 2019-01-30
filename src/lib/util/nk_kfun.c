#include <NKlib.h>

inherit "/lib/util/string";

#define PAD "                                                                              "

string spaces(int x) {
    if (x < 1) {
        return "";
    }

    return PAD[0 .. x - 1];
}

string repeat(string x, int y) {
    Iterator iterator;
    string str;

    str = "";
    if (y < 1) {
        return str;
    }

    if (y == 1) {
        return x;
    }

    iterator = new IntIterator(1, y);
    while (!iterator->end()) {
        str += x;
        iterator->next();
    }

    return str;
}

string lalign(mixed str, int pad) {
    int x;
    string s;

    s = (string)str;
    x = pad - strlen(s);
    if (x > 0) {
        s = s + PAD[0..x-1];
    }
    return s;
}

string centre(mixed str, int pad) {
    int x, l;
    string s;

    s = (string)str;
    l = strlen(s);
    x = (pad - l) / 2;
    if (x > -1) {
        s = PAD[0..x] + s;
    }
    x = l - x;
    if (x > -1) {
        s = s + PAD[0..x];
    }
    return s;
}

string ralign(mixed str, int pad) {
    int x;
    string s;

    s = (string)str;
    x = pad - strlen(s);
    if (x > 0) {
        s = PAD[0..x-1] + s;
    }
    return s;
}

static string dump_value(mixed value, mapping seen)
{
    string str;
    int i, sz;
    mixed *indices, *values;

    switch (typeof(value)) {
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

        case T_INT:
            return (string) value;

        case T_STRING:
            str = value;
        if (sscanf(str, "%*s\\") != 0) {
            str = implode(explode("\\" + str + "\\", "\\"), "\\\\");
        }
        if (sscanf(str, "%*s\"") != 0) {
            str = implode(explode("\"" + str + "\"", "\""), "\\\"");
        }
        if (sscanf(str, "%*s\n") != 0) {
            str = implode(explode("\n" + str + "\n", "\n"), "\\n");
        }
        if (sscanf(str, "%*s\t") != 0) {
            str = implode(explode("\t" + str + "\t", "\t"), "\\t");
        }
        return "\"" + str + "\"";

        case T_OBJECT:
            return "<" + object_name(value) + ">";

        case T_ARRAY:
            if (seen[value]) {
                return "#" + (seen[value] - 1);
            }

        seen[value] = map_sizeof(seen) + 1;
        sz = sizeof(value);
        if (sz == 0) {
            return "({ })";
        }

        str = "({ ";
        for (i = 0, --sz; i < sz; i++) {
            str += dump_value(value[i], seen) + ", ";
        }
        return str + dump_value(value[i], seen) + " })";

        case T_MAPPING:
            if (seen[value]) {
                return "@" + (seen[value] - 1);
            }

        seen[value] = map_sizeof(seen) + 1;
        sz = map_sizeof(value);
        if (sz == 0) {
            return "([ ])";
        }

        str = "([ ";
        indices = map_indices(value);
        values = map_values(value);
        for (i = 0, --sz; i < sz; i++) {
            str += dump_value(indices[i], seen) + ":" +
                   dump_value(values[i], seen) + ", ";
        }
        return str + dump_value(indices[i], seen) + ":" +
               dump_value(values[i], seen) + " ])";

        case T_NIL:
            return "nil";
    }
}

mixed min(mixed a, mixed b) {
    return ((a) < (b) ? (a) : (b));
}

mixed max(mixed a, mixed b) {
    return ((a) > (b) ? (a) : (b));
}

int gcd(int a, int b) {
    a = (a > 0) ? a : -a;
    b = (b > 0) ? b : -b;

    while (a != b) {
        if (a > b) {
            a -= b;
        } else {
            b -= a;
        }
    }

    return a;
}

float pi(void) {
    return atan(1.) * 4.;
}
