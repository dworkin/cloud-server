# include <String.h>
# include "Record.h"

inherit Extensions;


/*
 * initialize Extensions from a blob
 */
static void create(String blob, int offset, int end)
{
    ::create(extRestore(blob, offset, end));
}
