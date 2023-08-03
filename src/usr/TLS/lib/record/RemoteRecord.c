# include "Record.h"

inherit Record;


/*
 * initialize Record from a blob
 */
static void create(string blob)
{
    int type;
    string version, fragment;

    type = blob[0];
    version = blob[1 .. 2];
    fragment = len2Restore(blob, 3)[0];
    ::create(type, fragment, version);
}
