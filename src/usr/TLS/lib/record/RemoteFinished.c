# include <String.h>
# include "Record.h"

inherit Finished;


/*
 * initialize Finished from a blob
 */
static void create(String blob, int offset, int end)
{
    ::create(substring(blob, offset, end - 1));
}
