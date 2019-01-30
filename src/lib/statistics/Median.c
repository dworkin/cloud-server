#include <NKlib.h>

inherit Function;

float evaluate(float *data) {
    int x, sampleSize;

    sampleSize = sizeof(data);

    if (sampleSize < 2) {
        error("Median: the sample size must be greater than unity.");
    }

    x = sampleSize / 2;

    if (x > 0 && sampleSize % 2 == 0) {
        return (data[x] + data[x - 1]) * 0.5;
    }

    return data[x];
}
