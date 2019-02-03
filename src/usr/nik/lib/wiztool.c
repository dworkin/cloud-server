#include "nik.h"

inherit Terminal;
inherit NK_KFUN;

string getPrompt(object user) {
    return magenta(user->query_name() + "> ", TRUE);
}
