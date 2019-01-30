#include <NKlib.h>

inherit Function;
inherit NK_KFUN;

string evaluate(mixed value) {
    switch (typeof(value)) {
        case T_STRING:
        case T_INT:
        case T_FLOAT:
            return (string) value;
    }

    if (T_OBJECT == typeof(value) && function_object("toString", value)) {
        return value->toString();
    }

    return dump_value(value, ([]));
}
