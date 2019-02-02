#include <Universe.h>

int cmd_look(object user, string cmd, string arg) {
    user->println("You look around.");
    return 1;
}
