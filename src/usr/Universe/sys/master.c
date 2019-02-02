#include <Universe.h>

mapping avatars;

object addAvatar(string name) {
    avatars[name] = clone_object(AVATAR_OBJ);
    return avatars[name];
}

mixed removeAvatar(string name) {
    avatars[name] = nil;
    avatars[name];
}

static void create(void) {
    avatars = ([ ]);
}
