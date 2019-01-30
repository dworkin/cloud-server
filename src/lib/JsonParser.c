#include <type.h>

#define JSON_PARSER_GRAMMAR "whitespace=/[\n ]*/ " + \
        "colon=/:/ " + \
        "comma=/,/ " + \
        "string=/\"([^\\\\\"\n]*(\\\\.)*)*\"/ " + \
        "float = /[0-9]+\\.[0-9]+([eE][-+]?[0-9]+)?/ " + \
        "float = /\\.[0-9]+([eE][-+]?[0-9]+)?/ " + \
        "float = /[0-9]+[eE][-+]?[0-9]+/ " + \
        "integer=/[-+]?[0-9]+/ " + \
        "special=/(true|false|null)/ " + \
        "S : json_value " + \
        "jsonobject : '{' jsonobject_pairs '}' ? handle_jsonobject " + \
        "jsonobject_pairs : jsonobject_pair " + \
        "jsonobject_pairs : jsonobject_pairs comma jsonobject_pair " + \
        "jsonobject_pair : json_key colon json_value ? make_key_value_pair " + \
        "jsonarray : '[' jsonarray_elements ']' ? handle_jsonarray " + \
        "jsonarray : '[]' ? handle_empty_jsonarray " + \
        "jsonarray_elements : jsonobject " + \
        "jsonarray_elements : json_value " + \
        "jsonarray_elements : jsonarray_elements comma json_value " + \
        "json_key : string ? handle_string " + \
        "json_value: jsonobject " + \
        "json_value: jsonarray " + \
        "json_value: string ? handle_string " + \
        "json_value: number " + \
        "json_value: special ? handle_string " + \
        "number: integer ? handle_integer " + \
        "number: float ? handle_float "

mapping *make_key_value_pair(mixed *kv) {
    return ({ ([kv[0] : kv[2]]) });
}

mixed *handle_empty_jsonarray(mixed *json_array) {
    return ({ ({}) });
}

mixed *handle_jsonarray(mixed *json_array) {
    int i;
    int j;
    int sz;
    mixed *json_results;

    sz = sizeof(json_array);
    json_results = ({});
    for (i = 1, j = 0; i < sz; i += 2, j++) {
        json_results += ({ json_array[i] });
    }
    return ({ json_results });
}

mapping *handle_jsonobject(mixed *json_obj) {
    int i;
    int sz;
    mapping json_results;

    json_results = ([]);
    for (i = 0, sz = sizeof(json_obj); i < sz; i++) {
        if (typeof(json_obj[i]) == T_MAPPING) {
            json_results += json_obj[i];
        }
    }
    return ({ json_results });
}

string *handle_string(mixed *value) {
    return ({ implode(explode(value[0], "\""), "") });
}

int *handle_integer(mixed *value) {
    int i;
    return sscanf(value[0], "%d", i) == 1 ? ({ i }) : ({});
}

float *handle_float(mixed *value) {
    float f;
    return sscanf(implode(value, ""), "%f", f) == 1 ? ({ f }) : ({});
}

mapping parse(string json) {
    mixed *parsed;
    mapping map;
    parsed = parse_string(JSON_PARSER_GRAMMAR, json);
    map = ([]);
    map["json"] = parsed[0];
    return map;
}
