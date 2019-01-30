#include <NKlib.h>

private string colorString(mixed a, mixed b, string str) {
    return TERM_ESC + a + "m" + str + TERM_ESC + b + "m";
}

string bell(void) {
    return "\a";
}

string clear(void) {
    return TERM_ESC + "2J";
}

string bold(string str) {
    return colorString(1, 22, str);
}

string dim(string str) {
    return colorString(2, 22, str);
}

string inverse(string str) {
    return colorString(7, 27, str);
}

string italic(string str) {
    return colorString(3, 23, str);
}

string strikeThrough(string str) {
    return colorString(9, 29, str);
}

string underline(string str) {
    return colorString(4, 24, str);
}

string color256(int a, string str) {
    string aStr;

    if (a < 0 || a > 255) {
        error("Terminal: Invalid color.");
    }

    aStr = (string) a;

    if (a < 10) {
        aStr = "00" + a;
    } else if (a < 100) {
        aStr = "0" + a;
    }

    return "\x1b[38;5;" + aStr + "m" + str + "\x1b[0m";
}

string black(string str, varargs int useBold) {
    return colorString(useBold ? 90 : 30, 39, str);
}

string blue(string str, varargs int useBold) {
    return colorString(useBold ? 94 : 34, 39, str);
}

string cyan(string str, varargs int useBold) {
    return colorString(useBold ? 96 : 36, 39, str);
}

string green(string str, varargs int useBold) {
    return colorString(useBold ? 92 : 32, 39, str);
}

string magenta(string str, varargs int useBold) {
    return colorString(useBold ? 95 : 35, 39, str);
}

string red(string str, varargs int useBold) {
    return colorString(useBold ? 91 : 31, 39, str);
}

string yellow(string str, varargs int useBold) {
    return colorString(useBold ? 93 : 33, 39, str);
}

string white(string str, varargs int useBold) {
    return colorString(useBold ? 97 : 37, 39, str);
}

string gray(string str) {
    return colorString(90, 39, str);
}

string grey(string str) {
    return gray(str);
}

private string move(int amount, string code) {
    return TERM_ESC + amount + code;
}

string moveBack(int x) {
    return move(x, "D");
}

string moveDown(int y) {
    return move(y, "B");
}

string moveForward(int x) {
    return move(x, "C");
}

string moveUp(int y) {
    return move(y, "A");
}

string arrowDown(void) {
    return "↓";
}

string arrowLeft(void) {
    return "←";
}

string arrowRight(void) {
    return "→";
}

string arrowUp(void) {
    return "↑";
}

string check(void) {
    return "✔";
}

string cross(void) {
    return "✘";
}

string eks(void) {
    return "✖";
}

string info(void) {
    return "ℹ";
}

string play(void) {
    return "▶";
}

string hpipe(void) {
    return "─";
}

string hpipeDouble(void) {
    return "═";
}

string htpipe(void) {
    return "├";
}

string upLpipe(varargs int reversed) {
    return reversed ? "┌" : "┐";
}

string upLpipeRound(varargs int reversed) {
    return reversed ? "╭" : "╮";
}

string upLpipeDouble(varargs int reversed) {
    return reversed ? "╔" : "╗";
}

string lpipe(varargs int reversed) {
    return reversed ? "┘" : "└";
}

string lpipeRound(varargs int reversed) {
    return reversed ? "╯" : "╰";
}

string lpipeDouble(varargs int reversed) {
    return reversed ? "╝" : "╚";
}

string tpipe(void) {
    return "┬";
}

string vpipe(void) {
    return "│";
}

string vpipeDouble(void) {
    return "║";
}

string pointer(varargs int useLarge) {
    return useLarge ? "❯" : "›";
}

string star(void) {
    return "★";
}

string fourCorners(void) {
    return "⛶";
}

string shrug(void) {
    return "¯\\_(ツ)_/¯";
}
