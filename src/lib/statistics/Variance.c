#include <NKlib.h>

inherit Function;

float evaluate(mixed *data, float mean) {
    Iterator iterator;
    int i, x;
    float variance;

    x = sizeof(data);
    if (x < 2) {
        error("Variance: sample size must be larger than unity.");
    }

    x--;

    iterator = new IntIterator(0, x);
    variance = 0.;

    while (!iterator->end()) {
        i = iterator->next();
        variance += ((float) data[i] - mean) * ((float) data[i] - mean);
    }

    variance /= (float) x;

    return variance;
}
