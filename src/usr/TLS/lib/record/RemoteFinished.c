# include <String.h>
# include "Record.h"

inherit Finished;


static void create(String blob, int offset, int end)
{
    ::create(substring(blob, offset, end - 1));
}
