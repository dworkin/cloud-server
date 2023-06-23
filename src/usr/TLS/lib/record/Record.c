# include "Record.h"

inherit Data;


private string version;
private string fragment;

/*
 * create record
 */
static void create(int type, string fragment, varargs string version)
{
    ::create(type);
    ::fragment = fragment;
    ::version = (version) ? version : VERSION_TLS_12;
}

/*
 * encrypt payload
 */
void protect(string algorithm, string key, string iv, string aad, int pad)
{
    string padding;

    padding = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    while (strlen(padding) < pad) {
	padding += padding;
    }
    padding = padding[.. pad];
    padding[0] = type();
    ::create(RECORD_APPLICATION_DATA);
    fragment = encrypt(algorithm, key, iv, aad, fragment + padding);
}

string transport()
{
    return typeHeader() + version + len2Save(fragment);
}


string version()	{ return version; }
string fragment()	{ return fragment; }
