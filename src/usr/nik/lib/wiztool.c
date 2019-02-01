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

private string idleTime(object user) {
    return (new Time() - new Time(user->queryIdle()))->asDuration()[4];
}

void cmd_who(object user, string cmd, string arg) {
    object *users;
    string *list;
    int sz;
    Iterator i;

    users = users() - ({ user });
    sz = sizeof(users);
    if (sz == 0) {
        user->message("One is the loneliest number.\n");
        return;
    }
    list = allocate(sz);
    i = new IntIterator(0, sz - 1);
    while (!i->end()) {
        list[i->next()] = users[i->current()]->query_name() + " " + idleTime(users[i->current()]);
    }
    arg = "Users logged in:\n" +
            new Array(list)->reduce(new ArrayToListReducer(), 0, sz - 1, 1);

    user->message(arg + "\n");
}
