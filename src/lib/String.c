# include <String.h>
# include <status.h>
# include <type.h>

# define UTF8DECODE	"/sys/utf8decode"

# define MAX_LENGTH	16777216
# define INITIAL_SIZE	16


private mixed *bytes;	/* ({ ({ "text", " " }), 5, "23", 8 }) */
private mixed *chars;	/* ({ '1', 6, ({ '4', '5' }), 10 }) */

/*
 * NAME:	strLength()
 * DESCRIPTION:	return the length of a string or array of integers
 */
private int strLength(mixed str)
{
    return (typeof(str) == T_STRING) ? strlen(str) : sizeof(str);
}

/*
 * NAME:	append()
 * DESCRIPTION:	append a string (which can be an array of integers) to a
 *		buffer, which is either a string or an array of strings
 */
private mixed *append(mixed buffer, int index, mixed str, int bufMax,
		      int strMax)
{
    int len;
    mixed chunk;

    if (strLength(str) == 0) {
	return ({ buffer, index });	/* no change */
    }

    if (!buffer) {
	return ({ ({ str }), 0 });	/* start new buffer */
    }

    chunk = buffer[index];
    len = strLength(chunk);
    if (len < strMax) {
	if (len + strLength(str) <= strMax) {
	    /*
	     * append to last string in buffer
	     */
	    buffer[index] = chunk + str;
	    return ({ buffer, index });
	} else {
	    len = strMax - len;
	    buffer[index] = chunk + str[.. len - 1];
	    str = str[len ..];
	}
    }

    /*
     * add new string as buffer element
     */
    len = sizeof(buffer);
    if (++index == len) {
	if (len == bufMax) {
	    /* cannot grow any further */
	    error("String too large");
	}
	if (len > bufMax - len) {
	    len = bufMax - len;
	}
	buffer += allocate(len);
    }
    buffer[index] = str;

    return ({ buffer, index });
}

/*
 * NAME:	appendChars()
 * DESCRIPTION:	append a string as an array of chars
 */
private int appendChars(string str, int index, int charOffset, int bufMax)
{
    int i, *buf;

    while (strlen(str) > bufMax) {
	buf = allocate_int(bufMax);
	for (i = bufMax; --i >= 0; ) {
	    buf[i] = str[i];
	}
	({ chars[index], charOffset }) =
	    append(chars[index], charOffset, buf, bufMax, bufMax);
	chars[index + 1] += bufMax;
	str = str[bufMax ..];
    }

    buf = allocate_int(strlen(str));
    for (i = strlen(str); --i >= 0; ) {
	buf[i] = str[i];
    }
    ({ chars[index], charOffset }) =
	append(chars[index], charOffset, buf, bufMax, bufMax);
    chars[index + 1] += sizeof(buf);

    if (chars[index + 1] > MAX_LENGTH) {
	error("String too long");
    }

    return charOffset;
}

/*
 * NAME:	appendBytes()
 * DESCRIPTION:	add string to bytes
 */
private int *appendBytes(string str, int index, int byteOffset, int charOffset,
			 int bufMax, int strMax)
{
    int len, size;
    mixed *extend;

    len = chars[index + 1];
    if (bytes[index + 1] < len) {
	if (index + 2 == bufMax) {
	    /*
	     * out of space for bytes -- append as chars
	     */
	    return ({ index, byteOffset,
		      appendChars(str, index, charOffset, bufMax) });
	}

	/*
	 * previous bytes were followed by chars
	 */
	/* trim buffers */
	if (typeof(bytes[index]) == T_ARRAY) {
	    if (byteOffset == 0) {
		bytes[index] = bytes[index][0];
	    } else if (sizeof(bytes[index]) > byteOffset + 1) {
		bytes[index] = bytes[index][.. byteOffset];
	    }
	}
	if (typeof(chars[index]) == T_ARRAY) {
	    if (charOffset == 0) {
		chars[index] = chars[index][0];
	    } else if (sizeof(chars[index]) > charOffset + 1) {
		chars[index] = chars[index][.. charOffset];
	    }
	}

	index += 2;
	size = sizeof(bytes);
	if (index > size) {
	    /*
	     * extend buffers
	     */
	    if (size > bufMax - size) {
		size = bufMax - size;
	    }
	    extend = allocate(size);
	    bytes += extend;
	    chars += extend;
	}

	charOffset = byteOffset = 0;
    }

    ({ bytes[index], byteOffset }) =
	append(bytes[index], byteOffset, str, bufMax, strMax);
    chars[index + 1] = bytes[index + 1] = len + strlen(str);

    if (chars[index + 1] > MAX_LENGTH) {
	error("String too long");
    }

    return ({ index, byteOffset, charOffset });
}

/*
 * NAME:	appendSequence()
 * DESCRIPTION:	append sequence to bytes and chars
 */
private int *appendSequence(mixed *data, int index, int byteOffset,
			    int charOffset, int bufMax, int strMax)
{
    int i, sz;
    mixed str;

    for (i = 0, sz = sizeof(data); i < sz; i++) {
	str = data[i];
	if (typeof(str) == T_STRING) {
	    ({ index, byteOffset, charOffset }) =
		appendBytes(str, index, byteOffset, charOffset, bufMax, strMax);
	} else {
	    ({ chars[index], charOffset }) =
		append(chars[index], charOffset, str, bufMax, bufMax);
	    chars[index + 1] += sizeof(str);

	    if (chars[index + 1] > MAX_LENGTH) {
		error("String too long");
	    }
	}

    }
    return ({ index, byteOffset, charOffset });
}

/*
 * NAME:	appendUTF8()
 * DESCRIPTION:	append a UTF8-encoded string
 */
private mixed *appendUTF8(string remainder, string str, int index,
			  int byteOffset, int charOffset, int bufMax,
			  int strMax)
{
    string left;
    mixed *buf;

    if (strlen(remainder) + strlen(str) > strMax) {
	left = str[strMax - strlen(remainder) ..];
	str = remainder + str[.. strMax - strlen(remainder) - 1];
    } else {
	left = "";
    }
    ({ buf, remainder }) = UTF8DECODE->decode(remainder + str);
    if (!buf) {
	error("Invalid UTF8 sequence");
    }
    remainder += left;
    if (sizeof(buf) != 0) {
	({ index, byteOffset, charOffset }) =
	    appendSequence(buf, index, byteOffset, charOffset, bufMax, strMax);

	if (strlen(remainder) != 0) {
	    ({ buf, remainder }) = UTF8DECODE->decode(remainder);
	    if (!buf) {
		error("Invalid UTF8 sequence");
	    }
	    if (sizeof(buf) != 0) {
		({ index, byteOffset, charOffset }) =
		    appendSequence(buf, index, byteOffset, charOffset, bufMax,
				   strMax);
	    }
	}
    }

    return ({ remainder, index, byteOffset, charOffset });
}

/*
 * NAME:	create()
 * DESCRIPTION:	initialize a string
 */
static void create(mixed data, varargs string utf8)
{
    int bufMax, strMax, index, byteOffset, charOffset, i, sz;
    string remainder;

    if (utf8 && utf8 != "UTF8") {
	error("Bad encoding");
    }
    bufMax = status(ST_ARRAYSIZE);
    strMax = status(ST_STRSIZE);

    index = byteOffset = charOffset = 0;
    switch (typeof(data)) {
    case T_STRING:
	if (!utf8) {
	    bytes = ({ data, strlen(data) });
	    chars = ({ nil, bytes[1] });
	} else {
	    ({ data, remainder }) = UTF8DECODE->decode(data);
	    if (!data || remainder != "") {
		error("Invalid UTF8 sequence");
	    }
	    bytes = allocate(INITIAL_SIZE);
	    chars = allocate(INITIAL_SIZE);
	    chars[1] = bytes[1] = 0;
	    ({ index, byteOffset, charOffset }) =
		appendSequence(data, 0, 0, 0, bufMax, strMax);
	}
	break;

    case T_ARRAY:
	bytes = allocate(INITIAL_SIZE);
	chars = allocate(INITIAL_SIZE);
	chars[1] = bytes[1] = 0;
	if (!utf8) {
	    for (i = 0, sz = sizeof(data); i < sz; i++) {
		({ index, byteOffset, charOffset }) =
		    appendBytes(data[i], index, byteOffset, charOffset, bufMax,
				strMax);
	    }
	} else {
	    remainder = "";
	    for (i = 0, sz = sizeof(data); i < sz; i++) {
		({ remainder, index, byteOffset, charOffset }) =
		    appendUTF8(remainder, data[i], index, byteOffset,
			       charOffset, bufMax, strMax);
	    }
	    if (remainder != "") {
		error("Incomplete UTF8 sequence");
	    }
	}
	break;
    }

    if (chars[1] == 0) {
	bytes[0] = "";
    }

    /* trim buffers */
    if (typeof(bytes[index]) == T_ARRAY) {
	if (byteOffset == 0) {
	    bytes[index] = bytes[index][0];
	} else if (sizeof(bytes[index]) > byteOffset + 1) {
	    bytes[index] = bytes[index][.. byteOffset];
	}
    }
    if (typeof(chars[index]) == T_ARRAY) {
	if (charOffset == 0) {
	    chars[index] = chars[index][0];
	} else if (sizeof(chars[index]) > charOffset + 1) {
	    chars[index] = chars[index][.. charOffset];
	}
    }
    if (sizeof(bytes) > index + 1) {
	bytes = bytes[.. index + 1];
	chars = chars[.. index + 1];
    }
}

/*
 * NAME:	length()
 * DESCRIPTION:	return the length of the string
 */
int length()
{
    return chars[sizeof(chars) - 1];
}

/*
 * NAME:	isBytes()
 * DESCRIPTION:	return TRUE if the string contains bytes only
 */
int isBytes()
{
    return (chars[0] == nil);
}

/*
 * NAME:	[]
 * DESCRIPTION:	index a string
 */
static int operator[] (int index)
{
    int low, high, mid, offset;
    mixed *buffer;

    if (index < 0 || index >= length()) {
	error("String index out of range");
    }

    /* binary search for the buffer index */
    low = 0;
    high = sizeof(bytes);
    while (low < high) {
	mid = ((low + high) >> 1) & ~1;
	if (chars[mid + 1] <= index) {
	    /* search higher */
	    low = mid + 2;
	} else if (mid == 0) {
	    /* found at the beginning */
	    offset = 0;
	    break;
	}
	offset = chars[mid - 1];
	if (offset <= index) {
	    /* found at this offset */
	    break;
	}

	/* search lower */
	high = mid;
    }

    if (bytes[mid + 1] > index) {
	/* bytes */
	buffer = bytes;
    } else {
	/* chars */
	offset = bytes[mid + 1];
	buffer = chars;
    }
    index -= offset;

    if (typeof(buffer[mid][0]) == T_INT) {
	/* one string or *int */
	return buffer[mid][index];
    } else {
	/* array of strings or *int */
	offset = sizeof(buffer[mid][0]);
	return buffer[mid][index / offset][index % offset];
    }
}

/*
 * NAME:	[]=
 * DESCRIPTION:	assign to string index not permitted
 */
static void operator[]= (int index, int value)
{
    error("Strings are immutable");
}
