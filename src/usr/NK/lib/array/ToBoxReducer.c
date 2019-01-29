#include <NKlib.h>

inherit Function;
inherit Terminal;
inherit NK_KFUN;

private LongestString longestString;
private int mt;
private int mr;
private int mb;
private int ml;
private int pt;
private int pr;
private int pb;
private int pl;
private int boxType;
private string tl;
private string tr;
private string bl;
private string br;
private string h;
private string v;

private string _evaluate(mixed *items, int ls) {
    string output, marginTop, marginBottom, paddingTop, paddingBottom, line;
    int ll, i, sz;

    marginTop = "";
    for (i = 0; i < mt; i++) {
        marginTop += "\n";
    }

    marginBottom = "";
    for (i = 0; i < mb; i++) {
        marginBottom += "\n";
    }

    paddingTop = spaces(ml) + tl + repeat(h, ls + pl + pr) + tr + spaces(mr) + "\n";
    for (i = 0; i < pt; i++) {
        paddingTop += spaces(ml) + v + spaces(pl + pr + ls) + v + spaces(mr) + "\n";
    }

    paddingBottom = "";
    for (i = 0; i < pb; i++) {
        paddingBottom += spaces(ml) + v + spaces(pl + pr + ls) + v + spaces(mr) + "\n";
    }
    paddingBottom += spaces(ml) + bl + repeat(h, ls + pl + pr) + br + spaces(mr);

    sz = sizeof(items);
    output = "";
    for (i = 0; i < sz; i++) {
        line = items[i];
        ll = strlen(line);
        output += spaces(ml) + v + spaces(pl) + line + spaces(pr + ls - ll) + v + spaces(mr) + "\n";
    }

    return marginTop + paddingTop + output + paddingBottom + marginBottom;
}

string evaluate(mixed *items) {
    int ls;

    switch (boxType) {
        case BOX_STRING_SINGLE:
            tl = upLpipe(TRUE);
            tr = upLpipe();
            bl = lpipe();
            br = lpipe(TRUE);
            h = hpipe();
            v = vpipe();
            break;
        case BOX_STRING_DOUBLE:
            tl = upLpipeDouble(TRUE);
            tr = upLpipeDouble();
            bl = lpipeDouble();
            br = lpipeDouble(TRUE);
            h = hpipeDouble();
            v = vpipeDouble();
            break;
        case BOX_STRING_DOUBLE_HORIZONTAL:
            tl = upLpipeDouble(TRUE);
            tr = upLpipeDouble();
            bl = lpipeDouble();
            br = lpipeDouble(TRUE);
            h = hpipeDouble();
            v = vpipe();
            break;
        case BOX_STRING_ROUND:
            tl = upLpipeRound(TRUE);
            tr = upLpipeRound();
            bl = lpipeRound();
            br = lpipeRound(TRUE);
            h = hpipe();
            v = vpipe();
            break;
        default:
            tl = "+";
            tr = "+";
            bl = "+";
            br = "+";
            h = "-";
            v = "|";
            break;
    }

    return _evaluate(items, longestString->evaluate(items));
}

static void create(int *margins, int *padding, varargs int boxType) {
    longestString = new LongestString();
    ({ mt, mr, mb, ml }) = margins;
    ({ pt, pr, pb, pl }) = padding;
    ::boxType = boxType;
    ::create();
}
