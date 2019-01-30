#include <NKlib.h>

inherit ArrayToListReducer;
inherit NK_KFUN;

#define LIST_GAP " "

private int order;
private Stringify stringify;

private string fmtVal(mixed first, mixed second) {
    return stringify->evaluate(first) +
        (order > 0 ? (order++) + ")" : pointer()) +
        LIST_GAP +
        stringify->evaluate(second) + "\n";
}

static string ifNotNextValue(mixed currentValue, varargs int isLast) {
    return fmtVal("", currentValue);
}

static string ifNextValue(mixed nextValue, mixed currentValue, varargs int isLast) {
    return fmtVal(currentValue, nextValue);
}

void apply(int order) {
    ::order = order;
}

static void create(varargs int order) {
    ::order = order;
    stringify = new Stringify();
}