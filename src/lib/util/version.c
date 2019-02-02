#include <config.h>
#include <status.h>
#include <version.h>

#define NK_SERVER_NAME ("NK " + SERVER_NAME)
#define NK_SERVER_VERSION "0.1.0"

static string version(void) {
    return NK_SERVER_NAME + " v" + NK_SERVER_VERSION + ", " + status()[ST_VERSION];
}
