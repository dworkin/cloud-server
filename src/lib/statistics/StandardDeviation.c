#include <NKlib.h>

inherit Function;

float evaluate(float variance) {
    if (variance < 0.) {
        error("StandardDeviation: cannot evaluate sqrt of negative number " + variance);
    }
    return sqrt(variance);
}
