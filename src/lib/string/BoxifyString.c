#include <NKlib.h>

inherit Function;

private int boxType;
private int *margins;
private int *padding;

string evaluate(string str) {
    return new Array(explode(str, "\n"))->box(new ToBoxReducer(margins, padding, boxType));
}

static void create(varargs int boxType, int *margins, int *padding) {
    if (!margins) {
        margins = ({ 0, 0, 0, 1 });
    }

    if (!padding) {
        padding = ({ 0, 1, 0, 1 });
    }

    ::boxType = boxType;
    ::margins = margins;
    ::padding = padding;
}
