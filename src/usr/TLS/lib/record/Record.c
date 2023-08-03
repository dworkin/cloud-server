# include "Record.h"

inherit Data;


private string version;		/* legacy version */
private string payload;		/* payload (possibly encrypted) */
private int decrypted;		/* record was originally encrypted */

/*
 * create record
 */
static void create(int type, string payload, varargs string version)
{
    ::create(type);
    ::payload = payload;
    ::version = (version) ? version : TLS_VERSION_12;
}

/*
 * record header
 */
private string header(int length)
{
    return typeHeader() + version + int2Save(length);
}

/*
 * encrypt payload
 */
void protect(string cipher, string key, string iv, int taglen, varargs int pad)
{
    string padding;

    padding = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    while (strlen(padding) <= pad) {
	padding += padding;
    }
    padding = padding[.. pad];
    padding[0] = type();

    ::create(RECORD_APPLICATION_DATA);
    payload = encrypt(cipher, key, iv,
		      header(strlen(payload) + 1 + pad + taglen), taglen,
		      payload + padding);
}

/*
 * decrypt payload
 */
int unprotect(string cipher, string key, string iv, int taglen)
{
    string str;
    int len;

    str = decrypt(cipher, key, iv, header(strlen(payload)), taglen, payload);
    if (!str) {
	return FALSE;
    }

    for (len = strlen(str); str[--len] == '\0'; ) ;
    ::create(str[len]);
    payload = str[.. len - 1];

    decrypted = TRUE;
    return TRUE;
}

/*
 * export as a blob
 */
string transport()
{
    return header(strlen(payload)) + payload;
}


string version()	{ return version; }
string payload()	{ return payload; }
int decrypted()		{ return decrypted; }
