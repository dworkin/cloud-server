# include "Record.h"

inherit Data;


private string algorithm;	/* signature algorithm */
private string signature;	/* signature */

/*
 * initialize CertificateVerify
 */
static void create(string algorithm, string signature)
{
    ::create(HANDSHAKE_CERTIFICATE_VERIFY);
    ::algorithm = algorithm;
    ::signature = signature;
}

/*
 * export as a blob
 */
string transport()
{
    return algorithm +
	   len2Save(signature);
}


string algorithm()	{ return algorithm; }
string signature()	{ return signature; }
