#include <NKlib.h>

private Random random;
private int faces;

void setRandom(Random random) {
    ::random = random;
}

int roll(void) {
    if (random) {
        return random->randomInt();
    }

    return random(0) % faces + 1;
}

static void create(int faces, varargs Random random) {
    ::faces = faces;
    ::random = random;
}
