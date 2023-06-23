# include "Record.h"
# include "Extension.h"

inherit Data;


private string version;
private string random;
private string sessionId;
private string cipherSuite;
private int compressionMethod;
private Extension *extensions;

static void create(string random, string sessionId, string cipherSuite,
		   int compressionMethod, Extension *extensions,
		   varargs string version)
{
    ::create(HANDSHAKE_SERVER_HELLO);
    ::version = (version) ? version : VERSION_TLS_12;
    ::random = random;
    ::sessionId = sessionId;
    ::cipherSuite = cipherSuite;
    ::compressionMethod = compressionMethod;
    ::extensions = extensions;
}

string transport()
{
    string str;

    str = ".";
    str[0] = compressionMethod;

    return version +
	   random +
	   len1Save(sessionId) +
	   cipherSuite +
	   str +
	   extSave(extensions);
}


string version()		{ return version; }
string random()			{ return random; }
string sessionId()		{ return sessionId; }
string cipherSuite()		{ return cipherSuite; }
int compressionMethod()		{ return compressionMethod; }
Extension *extensions()		{ return extensions; }
