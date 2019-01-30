#include <NKlib.h>

inherit Function;

private mapping visitResults;
private float target;

mapping getVisitResults(void) {
    return visitResults;
}

int evaluate(float f, varargs int i) {
    if (f < target) {
        visitResults["<"]++;
        return -1;
    }

    if (f == target) {
        visitResults["="]++;
        return 0;
    }

    visitResults[">"]++;
    return 1;
}

void apply(varargs float target) {
    ::target = target;
}

static void create(varargs float target) {
    apply(target);
    visitResults = ([ "<": 0, "=": 0, ">": 0 ]);
    ::create();
}