# include "Record.h"
# include "Extension.h"

inherit Data;


private string context;
private Extensions *extensions;

static void create(string context, Extension *extensions)
{
    ::create(HANDSHAKE_CERTIFICATE_REQUEST);
    ::context = context;
    ::extensions = extensions;
}

string transport()
{
    return len1Save(context) +
	   extSave(extensions);
}


string context()		{ return context; }
Extension *extensions()		{ return extensions; }
