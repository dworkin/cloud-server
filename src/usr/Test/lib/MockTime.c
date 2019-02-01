#include <Test.h>

inherit Time;
inherit MOCK_MILLITIME_LIB;

static void create(varargs mixed time, float mtime) {
    if (!time && !mtime) {
        ({ time, mtime }) = millitime();
    }

    ::create(time, mtime);
}
