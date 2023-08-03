# include "Record.h"
# include "Extension.h"

inherit Data;


private Extensions *extensions;		/* list of extensions */

/*
 * initialize Extensions
 */
static void create(Extension *extensions)
{
    ::create(HANDSHAKE_EXTENSIONS);
    ::extensions = extensions;
}

/*
 * export as a blob
 */
string transport()
{
    return extSave(extensions);
}


Extension *extensions()		{ return extensions; }
