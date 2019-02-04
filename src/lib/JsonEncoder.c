#include <type.h>

string encode(mixed value, mapping seen) {
    string str;
    int i, sz;
    mixed *indices, *values;

    switch (typeof(value)) {
        case T_FLOAT:
            str = (string) value;
            if (sscanf(str, "%*s.") == 0 && sscanf(str, "%*se") == 0) {
                str += ".0";
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
            str = "";
            catch(str = value->to_string());
            return encode("<" + object_name(value) + (str == "" || str == nil ? "" : ": " +  str + " ") + ">", seen);

        case T_ARRAY:
            if (seen[value]) {
                return "" + (seen[value] - 1);
            }

            seen[value] = map_sizeof(seen) + 1;
            sz = sizeof(value);
            if (sz == 0) {
                return "[ ]";
            }

            str = "[ ";
            for (i = 0, --sz; i < sz; i++) {
                str += encode(value[i], seen) + ", ";
            }
            return str + encode(value[i], seen) + " ]";

        case T_MAPPING:
            if (seen[value]) {
                return "" + (seen[value] - 1);
            }

            seen[value] = map_sizeof(seen) + 1;
            sz = map_sizeof(value);
            if (sz == 0) {
                return "{ }";
            }

            str = "{ ";
            indices = map_indices(value);
            values = map_values(value);
            for (i = 0, --sz; i < sz; i++) {
                str += encode("" + indices[i], seen) + ":" + encode(values[i], seen) + ", ";
            }
            return str + encode("" + indices[i], seen) + ":" + encode(values[i], seen) + " }";

        case T_NIL:
            return "null";
    }
}
