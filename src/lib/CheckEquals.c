#include <NKlib.h>

private int objectHasEquals(mixed arg) {
    return T_OBJECT == typeof(arg) && function_object("equals", arg);
}

int checkEquals(mixed left, mixed right) {
    return objectHasEquals(left) ? left->equals(right) : left == right;
}
