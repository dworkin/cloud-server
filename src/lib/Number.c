#include <NKlib.h>

private mixed number;

int toInt(void) {
    return (int) number;
}

float toFloat(void) {
    return (float) number;
}

string toString(void) {
    return (string) number;
}

Rational toRational(void) {
    return new Rational(toFloat());
}

static void create(mixed number) {
    ::number = number;
}
