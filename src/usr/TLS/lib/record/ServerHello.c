# include "Record.h"
# include "Extension.h"

inherit Data;


private string version;			/* legacy version */
private string random;			/* 32 secure random bytes */
private string sessionId;		/* legacy session ID */
private string cipherSuite;		/* selected cipherSuite */
private int compressionMethod;		/* 0 */
private Extension *extensions;		/* extensions */

/*
 * initialize ServerHello
 */
static void create(string random, string sessionId, string cipherSuite,
		   int compressionMethod, Extension *extensions,
		   varargs string version)
{
    ::create(HANDSHAKE_SERVER_HELLO);
    ::version = (version) ? version : TLS_VERSION_12;
    ::random = random;
    ::sessionId = sessionId;
    ::cipherSuite = cipherSuite;
    ::compressionMethod = compressionMethod;
    ::extensions = extensions;
}

/*
 * export as a blob
 */
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
