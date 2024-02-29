/*
 * encode a string, given a base64 translation map
 */
private string e64(string str, string map)
{
    string *result, bits;
    int len, blen, i, j;

    len = strlen(str);
    result = allocate((len + 2) / 3);
    bits = "====";
    blen = len % 3;
    len -= blen;
    for (i = j = 0; i < len; j++, i += 3) {
	bits[0] = map[str[i] >> 2];
	bits[1] = map[((str[i] << 4) | (str[i + 1] >> 4)) & 0x3f];
	bits[2] = map[((str[i + 1] << 2) | (str[i + 2] >> 6)) & 0x3f];
	bits[3] = map[str[i + 2] & 0x3f];
	result[j] = bits;
    }

    switch (blen) {
    case 1:
	bits = "====";
	bits[0] = map[str[i] >> 2];
	bits[1] = map[(str[i] << 4) & 0x3f];
	result[j] = bits;
	break;

    case 2:
	bits = "====";
	bits[0] = map[str[i] >> 2];
	bits[1] = map[((str[i] << 4) | (str[i + 1] >> 4)) & 0x3f];
	bits[2] = map[(str[i + 1] << 2) & 0x3f];
	result[j] = bits;
	break;
    }
    return implode(result, "");
}

/*
 * decode a string, given a base64 translation map
 */
private string d64(string str, string map)
{
    string *result, bits;
    int len, i, j, b1, b2, b3, b4;

    len = strlen(str);
    result = allocate((len + 3) / 4);
    bits = "...";
    for (i = j = 0; i < len; j++, i += 4) {
	b1 = map[str[i]];
	b2 = map[str[i + 1]];
	b3 = map[str[i + 2]];
	b4 = map[str[i + 3]];
	bits[0] = (b1 << 2) | (b2 >> 4);
	bits[1] = (b2 << 4) | (b3 >> 2);
	bits[2] = (b3 << 6) | b4;
	result[j] = bits;
    }

    if (b3 == 0x40) {
	result[j - 1] = bits[0 .. 0];
    } else if (b4 == 0x40) {
	result[j - 1] = bits[0 .. 1];
    }
    return implode(result, "");
}

/*
 * encode as base64 string
 */
static string encode(string str)
{
    return e64(str, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" +
		    "0123456789+/");
}

/*
 * decode a base64 string
 */
static string decode(string str)
{
    return d64(str, "...........................................\x3e..." +
		    "\x3f\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d...\x40..." +
		    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c" +
		    "\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19" +
		    "......" +
		    "\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26" +
		    "\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33" +
		    "...................................................." +
		    "...................................................." +
		    ".............................");
}

/*
 * encode as URL-safe base64 string
 */
static string urlEncode(string str)
{
    int len;

    str = e64(str, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" +
		   "0123456789-_");
    len = strlen(str);
    if (len > 0) {
	if (str[len - 2] == '=') {
	    str = str[.. len - 3];
	} else if (str[len - 1] == '=') {
	    str = str[.. len - 2];
	}
    }

    return str;
}

/*
 * decode an URL-safe base64 string
 */
static string urlDecode(string str)
{
    switch (strlen(str) % 4) {
    case 2:
	str += "==";
	break;

    case 3:
	str += "=";
	break;
    }

    return d64(str, ".............................................\x3e." +
		    ".\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d...\x40..." +
		    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c" +
		    "\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19" +
		    "....\x3f." +
		    "\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26" +
		    "\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33" +
		    "...................................................." +
		    "...................................................." +
		    ".............................");
}
