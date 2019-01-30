#include <NKlib.h>
#include "nik.h"

inherit Terminal;
inherit NK_KFUN;

string getPrompt(object user) {
    return magenta(user->query_name() + "> ", TRUE);
}

static void done(object user) {
    user->message("\nDone.\n" + this_object()->getPrompt(user));
}

void cmd_cls(object user, string cmd, string arg) {
    if (previous_object() != user) {
        return;
    }

    user->message(clear());
}

void cmd_test(object user, string cmd, string arg) {
    if (previous_object() != user) {
        return;
    }

    "/usr/Test/sys/TestRunner"->runTests(user, arg);
}
