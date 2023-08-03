# include "Record.h"

inherit Alert;


/*
 * initialize Alert from a blob
 */
static void create(string blob)
{
    ::create(blob[0], blob[1]);
}
