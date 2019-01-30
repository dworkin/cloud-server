#include <NKlib.h>

inherit Function;
inherit Terminal;

#define LIST_GAP " "

static string ifNotNextValue(mixed currentValue, varargs int isLast) {
    return (isLast == FALSE ? htpipe() : lpipe()) + hpipe() + LIST_GAP + currentValue + "\n";
}

static string ifNextValue(mixed nextValue, mixed currentValue, varargs int isLast) {
    return currentValue +
        (isLast == FALSE ? htpipe() : lpipe()) +
        hpipe() + LIST_GAP + (string) nextValue + "\n";
}

string evaluate(varargs mixed args...) {
    mixed nextValue, currentValue;
    int isLast;

    nextValue = args[0];
    currentValue = args[1];
    isLast = sizeof(args) == 3 && args[2];

    if (!nextValue) {
        return ifNotNextValue(currentValue, isLast);
    }

    return ifNextValue(nextValue, currentValue, isLast);
}
