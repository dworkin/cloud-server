/*
 * HMAC (RFC 2104)
 */
static string HMAC(string key, string message, string hash)
{
    int blocksize;
    string ixor, oxor;

    switch (hash) {
    case "MD5":
    case "SHA1":
    case "SHA224":
    case "SHA256":
	blocksize = 64;
	ixor = "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36";
	oxor = "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c";
	break;

    case "SHA384":
    case "SHA512":
	blocksize = 128;
	ixor = "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36" +
	       "\x36\x36\x36\x36\x36\x36\x36\x36";
	oxor = "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c" +
	       "\x5c\x5c\x5c\x5c\x5c\x5c\x5c\x5c";
	break;

    default:
	error("Unknown hash function");
    }

    if (strlen(key) > blocksize) {
	key = hash_string(hash, key);
    }
    if (strlen(key) < blocksize) {
	key = (key + "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" +
		     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0")[.. blocksize - 1];
    }

    return hash_string(hash, asn_xor(key, oxor),
		       hash_string(hash, asn_xor(key, ixor), message));
}

/*
 * HKDF-Extract (RFC 5869)
 */
static string HKDF_Extract(string ikm, string hash, varargs string salt)
{
    if (!salt || strlen(salt) == 0) {
	/*
	 * will be padded by HMAC (blocksize >= hashlen)
	 */
	salt = "\0";
    }

    return HMAC(salt, ikm, hash);
}

/*
 * HKDF-Expand (RFC 5869)
 */
static string HKDF_Expand(string prk, string info, int length, string hash)
{
    string t, okm, s;
    int i;

    t = okm = "";
    s = ".";
    i = 0;
    while (strlen(okm) < length) {
	s[0] = ++i;
	t = HMAC(prk, t + info + s, hash);
	okm += t;
    }
    return okm[.. length - 1];
}

/*
 * HKDF (RFC 5869)
 */
static string HKDF(string ikm, string info, int length, string hash,
		   varargs string salt)
{
    return HKDF_Expand(HKDF_Extract(ikm, hash, salt), info, length, hash);
}
