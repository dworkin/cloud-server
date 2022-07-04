# include "HttpField.h"

inherit HttpAuthentication;

private inherit base64 "/lib/util/base64";


/*
 * create authentication item
 */
static void create(string blob)
{
    string scheme, authentication;

    if (sscanf(blob, "%s %s", scheme, authentication) == 0 ||
	(strlen(authentication) & 3) != 0) {
	error("Bad request");
    }
    ::create(scheme, base64::decode(authentication));
}
