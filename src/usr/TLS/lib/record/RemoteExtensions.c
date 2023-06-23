# include <String.h>
# include "Record.h"
# include "Extension.h"

inherit Extensions;


static void create(String blob, int offset, int end)
{
    ::create(extRestore(blob, offset, end));
}
