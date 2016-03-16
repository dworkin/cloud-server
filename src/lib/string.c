# include <string.h>
# include <status.h>
# include <type.h>

# define UTF8DECODE	"/sys/utf8decode"

# define MAX_LENGTH	16777216
# define INITIAL_SIZE	16


private mixed *bytes;	/* ({ ({ "text", " " }), 5, "23", 8 }) */
private mixed *chars;	/* ({ '1', 6, ({ '4', '5' }) }), 10 */

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
private mixed *append(mixed buffer, mixed str, int index, int bufMax,
		      int strMax)
{
    int len;
    mixed chunk;

    if (strLength(str) == 0) {
	return ({ buffer, index });	/* no change */
    }

    if (!buffer) {
	return ({ str, 0 });		/* start new buffer */
    }

    if (typeof(buffer[0]) == T_INT) {
	/*
	 * buffer is a string
	 */
	len = strLength(buffer);
	if (len < strMax) {
	    if (len + strLength(str) <= strMax) {
		/*
		 * simple append to string
		 */
		return ({ buffer + str, 0 });
	    } else {
		/*
		 * buffer becomes array of strings
		 */
		len = strMax - len;
		buffer = ({ buffer + str[.. len - 1] });
		str = str[len ..];
	    }
	} else {
	    /*
	     * buffer becomes array of strings
	     */
	    buffer = ({ buffer });
	}
	index = 0;
    } else {
	/*
	 * buffer is an array of strings
	 */
	chunk = buffer[index];
	len = strLength(chunk);
	if (len < strMax) {
	    if (len + strLength(str) <= strMax) {
		/*
		 * simple append to last string in buffer
		 */
		buffer[index] = chunk + str;
		return ({ buffer, index });
	    } else {
		len = strMax - len;
		buffer[index] = chunk + str[.. len - 1];
		str = str[len ..];
	    }
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
 * NAME:	appendAsChars()
 * DESCRIPTION:	append a string as an array of chars
 */
private int appendAsChars(string str, int index, int charOffset, int bufMax)
{
    int i, *buf;

    while (strlen(str) > bufMax) {
	buf = allocate_int(bufMax);
	for (i = bufMax; --i >= 0; ) {
	    buf[i] = str[i];
	}
	({ chars[index], charOffset }) =
	    append(chars[index], buf, charOffset, bufMax, bufMax);
	chars[index + 1] += bufMax;
	str = str[bufMax ..];
    }

    buf = allocate_int(strlen(str));
    for (i = strlen(str); --i >= 0; ) {
	buf[i] = str[i];
    }
    ({ chars[index], charOffset }) =
	append(chars[index], buf, charOffset, bufMax, bufMax);
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
		      appendAsChars(str, index, charOffset, bufMax) });
	}

	/*
	 * previous bytes were followed by chars
	 */
	/* trim buffers */
	if (typeof(bytes[index]) != T_STRING) {
	    bytes[index] = bytes[index][.. byteOffset];
	}
	if (chars[index] && typeof(chars[index][0]) != T_INT) {
	    chars[index] = chars[index][.. charOffset];
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
	append(bytes[index], str, byteOffset, bufMax, strMax);
    chars[index + 1] = bytes[index + 1] = len + strlen(str);

    if (chars[index + 1] > MAX_LENGTH) {
	error("String too long");
    }

    return ({ index, byteOffset, charOffset });
}

/*
 * NAME:	appendChars()
 * DESCRIPTION:	append sequence to bytes and chars
 */
private int *appendChars(mixed *data, int index, int byteOffset, int charOffset,
			int bufMax, int strMax)
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
		append(chars[index], str, charOffset, bufMax, bufMax);
	    chars[index + 1] += sizeof(str);

	    if (chars[index + 1] > MAX_LENGTH) {
		error("String too long");
	    }
	}

    }
    return ({ index, byteOffset, charOffset });
}

/*
 * NAME:	buildBytes()
 * DESCRIPTION:	build a byte string
 */
private int *buildBytes(string *strs, int bufMax, int strMax)
{
    int index, byteOffset, charOffset, i, sz;

    index = byteOffset = charOffset = 0;
    for (i = 0, sz = sizeof(strs); i < sz; i++) {
	({ index, byteOffset, charOffset }) =
	    appendBytes(strs[i], index, byteOffset, charOffset, bufMax, strMax);
    }
    return ({ index, byteOffset });
}

/*
 * NAME:	buildUTF8()
 * DESCRIPTION:	build a string from UTF8
 */
private int *buildUTF8(string *data, int bufMax, int strMax)
{
    object decoder;
    string str, left, remainder;
    int index, byteOffset, charOffset, i, sz;
    mixed *buf;

    decoder = find_object(UTF8DECODE);
    remainder = "";
    index = byteOffset = charOffset = 0;
    for (i = 0, sz = sizeof(data); i < sz; i++) {
	str = data[i];
	if (strlen(remainder) + strlen(str) > strMax) {
	    left = str[strMax - strlen(remainder) ..];
	    str = remainder + str[.. strMax - strlen(remainder) - 1];
	} else {
	    left = "";
	}
	({ buf, remainder }) = decoder->decode(remainder + str);
	if (!buf) {
	    error("Invalid UTF8 sequence");
	}
	remainder += left;
	({ index, byteOffset, charOffset }) =
	    appendChars(buf, index, byteOffset, charOffset, bufMax, strMax);
    }

    if (strlen(remainder) != 0) {
	({ buf, remainder }) = decoder->decode(remainder);
	if (!buf || remainder != "") {
	    error("Invalid UTF8 sequence");
	}
	({ index, byteOffset, charOffset }) =
	    appendChars(buf, index, byteOffset, charOffset, bufMax, strMax);
    }

    return ({ index, byteOffset, charOffset });
}

/*
 * NAME:	create()
 * DESCRIPTION:	initialize a string
 */
static void create(mixed data, varargs string utf8)
{
    int bufMax, strMax, index, byteOffset, charOffset;

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
	    data = UTF8DECODE->decode(data);
	    if (!data || data[1] != "") {
		error("Invalid UTF8 sequence");
	    }
	    bytes = allocate(INITIAL_SIZE);
	    chars = allocate(INITIAL_SIZE);
	    chars[1] = bytes[1] = 0;
	    ({ index, byteOffset, charOffset }) =
		appendChars(data[0], 0, 0, 0, bufMax, strMax);
	}
	break;

    case T_ARRAY:
	bytes = allocate(INITIAL_SIZE);
	chars = allocate(INITIAL_SIZE);
	chars[1] = bytes[1] = 0;
	if (!utf8) {
	    ({ index, byteOffset, charOffset }) =
		buildBytes(data, bufMax, strMax);
	} else {
	    ({ index, byteOffset, charOffset }) =
		buildUTF8(data, bufMax, strMax);
	}
	break;
    }

    if (chars[1] == 0) {
	bytes[0] = "";
    }

    /* trim buffers */
    if (byteOffset > 0) {
	bytes[index] = bytes[index][.. byteOffset];
    }
    if (charOffset > 0) {
	chars[index] = chars[index][.. charOffset];
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
 * NAME:	index()
 * DESCRIPTION:	return an index descriptor for the string
 *		({ bufIndex, buffer, offset, index })
 *		offset == nil: buffer is a string or *int
 *		offset != nil: buffer is an array of strings or *int
 */
private mixed *index(int index, varargs int low)
{
    int high, mid, start;
    mixed *buffer;

    /* binary search for the buffer index */
    high = sizeof(bytes);
    while (low < high) {
	mid = ((low + high) >> 1) & ~1;
	if (chars[mid + 1] <= index) {
	    /* search higher */
	    low = mid + 2;
	} else if (mid == 0) {
	    /* found at the beginning */
	    start = 0;
	    break;
	}
	start = chars[mid - 1];
	if (start <= index) {
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
	start = bytes[mid + 1];
	buffer = chars;
    }
    index -= start;

    if (typeof(buffer[mid][0]) == T_INT) {
	/* one string or *int */
	return ({ mid, buffer, nil, index });
    } else {
	/* array of strings or *int */
	high = sizeof(buffer[mid][0]);
	return ({ mid, buffer, index / high, index % high });
    }
}

/*
 * NAME:	[]
 * DESCRIPTION:	index a string
 */
static int operator[] (int index)
{
    int bufIndex;
    mixed *buffer, offset;

    if (index < 0 || index >= length()) {
	error("String index out of range");
    }

    ({ bufIndex, buffer, offset, index }) = index(index);
    if (offset == nil) {
	return buffer[bufIndex][index];
    } else {
	return buffer[bufIndex][offset][index];
    }
}

/*
 * NAME:	[]=
 * DESCRIPTION:	assign to string index not permitted
 */
static int operator[]= (int index, int value)
{
    error("Strings are immutable");
}
