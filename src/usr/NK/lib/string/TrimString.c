#include <NKlib.h>

inherit Function;

string evaluate(string str) {
    string *parsed;

    parsed = parse_string("whitespace = /[\n\b\r\t ]+/ " +
                          "word = /[^\n\b\r\t ]+/ S: word S: S word", str);
    parsed -= ({ "" });

    return implode(parsed, " ");
}
