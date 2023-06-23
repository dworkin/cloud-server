# include "Record.h"

inherit Record;


static void create(string blob)
{
    int type;
    string version, fragment;

    type = blob[0];
    version = blob[1 .. 2];
    fragment = len2Restore(blob, 3)[0];
    ::create(type, fragment, version);
}

/*
 * decrypt payload
 */
void unprotect(string algorithm, string iv, string aad)
{
    /* decrypt */
    /* strip padding */
    /* update payload */
}
