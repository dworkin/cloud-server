# include "Record.h"
# include "Extension.h"

inherit Data;


private string version;
private string random;
private string sessionId;
private string cipherSuites;
private string compressionMethods;
private Extension *extensions;

static void create(string random, string sessionId, string cipherSuites,
		   string compressionMethods, Extension *extensions,
		   varargs string version)
{
    ::create(HANDSHAKE_CLIENT_HELLO);
    ::version = (version) ? version : VERSION_TLS_12;
    ::random = random;
    ::sessionId = sessionId;
    ::cipherSuites = cipherSuites;
    ::compressionMethods = compressionMethods;
    ::extensions = extensions;
}

string transport()
{
    return version +
	   random +
	   len1Save(sessionId) +
	   len2Save(cipherSuites) +
	   len1Save(compressionMethods) +
	   extSave(extensions);
}


string version()		{ return version; }
string random()			{ return random; }
string sessionId()		{ return sessionId; }
string cipherSuites()		{ return cipherSuites; }
string compressionMethods()	{ return compressionMethods; }
Extension *extensions()		{ return extensions; }
