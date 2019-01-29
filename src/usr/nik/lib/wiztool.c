#include <NKlib.h>

inherit Terminal;
inherit NK_KFUN;

string getPrompt(object user) {
    if (previous_object() != user) {
        return "> ";
    }
    return magenta(user->query_name() + "> ", TRUE);
}

void cmd_cls(object user, string cmd, string arg) {
    if (previous_object() != user) {
        return;
    }

    user->message(clear());
}

void cmd_echo(object user, string cmd, string arg) {
    if (previous_object() != user) {
        return;
    }

    if (!arg) {
        user->message("Usage: echo <string to echo>\n");
        return;
    }
    user->message("[" + arg + "]\n");
}

void cmd_test(object user, string cmd, string arg) {
    if (previous_object() != user) {
        return;
    }

    "/usr/NK/sys/TestRunner"->runTests(user, arg);
}
