#include <NKlib.h>

int randomInt(void) {
    return random(0);
}

float randomFloat(varargs float f) {
    if (!f) {
        f = 1.;
    }

    return f * ((float)random(0) / (float)INT_MAX);
}

string randomString(int length) {
    int x;
    string str;
    string *results;
    Iterator iterator;

    if (length < 1) {
        error("Random: cannot generate a random string with size less than unity.");
    }

    str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    iterator = new IntIterator(0, length - 1);
    results = allocate(length);

    while (!iterator->end()) {
        x = random(strlen(str));
        results[iterator->next()] = str[x..x];
    }

    return implode(results, "");
}
