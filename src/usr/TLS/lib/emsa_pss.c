private inherit asn "/lib/util/asn";


/*
 * EMSA-PSS, RFC 8017 section 9.1
 */

/*
 * mask generator function
 */
static string MGF1(string seed, int len, string hash)
{
    string T, C;
    int i;

    T = "";
    i = 0;
    do {
	C = "\0\0\0" + asn::encode(i++);
	T += hash_string(hash, seed, C[strlen(C) - 4 ..]);
    } while (strlen(T) < len);
    return T[.. len - 1];
}

/*
 * EMSA-PSS encode
 */
static string encode(string M, int bits, string hash)
{
    int emLen, hlen;
    string mHash, salt, H, maskedData;

    emLen = (bits + 7) / 8;
    mHash = hash_string(hash, M);
    hlen = strlen(mHash);
    if (emLen < hlen + hlen + 2) {
	error("Not enough bits");
    }

    salt = secure_random(hlen);
    H = hash_string(hash, "\0\0\0\0\0\0\0\0", mHash, salt);
    maskedData = asn_xor("\1" + salt, MGF1(H, emLen - hlen - 1, hash));
    maskedData[0] &= 0xff >> ((8 - (bits & 7)) & 7);

    return maskedData + H + "\xbc";
}

/*
 * EMSA-PSS verify
 */
static int verify(string M, string EM, int bits, string hash)
{
    int emLen, hlen, bitmask, i;
    string mHash, H, data;

    EM = asn::unsignedExtend(EM, emLen = (bits + 7) / 8);
    mHash = hash_string(hash, M);
    hlen = strlen(mHash);
    bitmask = 0xff >> ((8 - (bits & 7)) & 7);
    if (emLen < hlen + hlen + 2 || EM[emLen - 1] != '\xbc' ||
	(EM[0] & ~bitmask)) {
	return FALSE;
    }

    H = EM[emLen - hlen - 1 .. emLen - 2];
    data = asn_xor(EM[.. emLen - hlen - 2], MGF1(H, emLen - hlen - 1, hash));
    data[0] &= bitmask;
    for (i = 0; data[i] == '\0'; i++) ;
    if (data[i] != '\1' || i != strlen(data) - hlen - 1) {
	return FALSE;
    }

    return (H == hash_string(hash,
			     "\0\0\0\0\0\0\0\0", mHash, data[i + 1 ..]));
}
