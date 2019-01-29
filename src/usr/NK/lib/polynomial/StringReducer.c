#include <NKlib.h>

inherit Function;

private int firstDisplay;
private string parameter;

mixed evaluate(float flt, int i) {
    string c, f;

    c = "";

    if (i == 1) {
        c += parameter;
    } else {
        c += parameter + "^" + i;
    }


    f = (flt == 1. ? "" : "" + flt);

    if (strlen(f) && f[0] == '0') {
        return "";
    }

    return (firstDisplay++ == TRUE ? "" : " + ") + f + c;
}

static void create(string parameter) {
    firstDisplay = TRUE;
    ::parameter = parameter;
}
