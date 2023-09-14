# include "Record.h"
# include "Extension.h"

inherit Data;


private string version;			/* legacy version */
private string random;			/* 32 secure random bytes */
private string sessionId;		/* legacy session ID */
private string *cipherSuites;		/* ordered list of cipher suites */
private string compressionMethods;	/* "\0" */
private Extension *extensions;		/* extensions */

/*
 * initialize ClientHello
 */
static void create(string random, string sessionId, string *cipherSuites,
		   string compressionMethods, Extension *extensions,
		   varargs string version)
{
    ::create(HANDSHAKE_CLIENT_HELLO);
    ::version = (version) ? version : TLS_VERSION_12;
    ::random = random;
    ::sessionId = sessionId;
    ::cipherSuites = cipherSuites;
    ::compressionMethods = compressionMethods;
    ::extensions = extensions;
}

/*
 * export as a blob
 */
string transport()
{
    return version +
	   random +
	   len1Save(sessionId) +
	   len2Save(implode(cipherSuites, "")) +
	   len1Save(compressionMethods) +
	   extSave(extensions);
}


string version()		{ return version; }
string random()			{ return random; }
string sessionId()		{ return sessionId; }
string *cipherSuites()		{ return cipherSuites; }
string compressionMethods()	{ return compressionMethods; }
Extension *extensions()		{ return extensions; }
