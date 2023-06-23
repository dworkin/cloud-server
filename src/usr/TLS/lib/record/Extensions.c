# include "Record.h"
# include "Extension.h"

inherit Data;


private Extensions *extensions;

static void create(Extension *extensions)
{
    ::create(HANDSHAKE_EXTENSIONS);
    ::extensions = extensions;
}

string transport()
{
    return extSave(extensions);
}


Extension *extensions()		{ return extensions; }
