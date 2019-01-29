#include <NKlib.h>

inherit Random;

private mixed *randomValues;
private int randomValuesIndex;
private mixed forceValue;

void reset(void) {
    randomValuesIndex = 0;
}

private mixed _random(void) {
    if (T_NIL == typeof(randomValues)) {
        return forceValue;
    }

    if (randomValuesIndex < sizeof(randomValues)) {
        randomValuesIndex++;
    }

    return randomValues[randomValuesIndex - 1];
}

float randomFloat(varargs float f) {
    return _random();
}

int randomInt(void) {
    return _random();
}

string randomString(int length) {
    return _random();
}

static void create(varargs mixed forceValue, mixed *randomValues) {
    ::forceValue = forceValue;
    ::randomValues = randomValues;
    ::randomValuesIndex = 0;
    ::create();
}
