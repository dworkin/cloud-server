# include <String.h>
# include <Iterator.h>
# include <status.h>
# include <type.h>

inherit Iterable;
private inherit unicode "util/unicode";


# define UTF8DECODE	"/sys/utf8decode"
# define UTF8ENCODE	"/sys/utf8encode"

# define MAX_LENGTH	16777216
# define INITIAL_SIZE	16


private mixed *bytes;	/* ({ ({ "text", " " }), 5, "23", 8 }) */
private mixed *chars;	/* ({ '1', 6, ({ '4', '5' }), 10 }) */

/*
 * return the length of a string or array of integers
 */
private int strLength(mixed str)
{
    return (typeof(str) == T_STRING) ? strlen(str) : sizeof(str);
}

/*
 * append a string (which can be an array of integers) to a buffer, which is
 * either a string or an array of strings
 */
private mixed *append(mixed *buffer, int index, mixed str, int bufMax,
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
 * append a string as an array of chars
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
 * add string to bytes
 */
private int *appendBytes(string str, int index, int byteOffset, int charOffset,
			 int bufMax, int strMax)
{
    int len, size;

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
	    bytes += allocate(size);
	    chars += allocate(size);
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
 * append sequence to bytes and chars
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
 * append a UTF8-encoded string
 */
private mixed *appendUTF8(object decoder, string remainder, string str,
			  int index, int byteOffset, int charOffset, int bufMax,
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
    ({ buf, remainder }) = decoder->decode(remainder + str);
    if (!buf) {
	error("Invalid UTF8 sequence");
    }
    remainder += left;
    if (sizeof(buf) != 0) {
	({ index, byteOffset, charOffset }) =
	    appendSequence(buf, index, byteOffset, charOffset, bufMax, strMax);

	if (strlen(remainder) != 0) {
	    ({ buf, remainder }) = decoder->decode(remainder);
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
 * process wide characters
 */
private mixed *processChars(int *characters)
{
    mixed *input, *output;
    int i, sz, *segments, numSegments, c;
    string char;

    /*
     * scan for byte and wide character segments
     */
    input = characters[..];
    sz = sizeof(input);
    segments = allocate_int(sz);
    numSegments = 0;
    char = " ";
    for (i = 0; i < sz; ) {
	c = input[i];
	if (c < 0 || c > 0x10ffff) {
	    error("Invalid character for String");
	}
	if (c <= 0xff) {
	    /* bytes */
	    do {
		char[0] = c;
		input[i] = char;
		if (++i >= sz) {
		    break;
		}
		c = input[i];
	    } while (c >= 0 && c <= 0xff);
	    segments[numSegments++] = i;
	} else {
	    /* wide characters */
	    do {
		if (++i >= sz) {
		    break;
		}
		c = input[i];
	    } while (c > 0xff && c <= 0x10ffff);
	    segments[numSegments++] = -i;
	}
    }

    /*
     * build output array of strings and wide character sequences
     */
    output = allocate(numSegments);
    for (i = sz = 0; i < numSegments; i++) {
	if (segments[i] > 0) {
	    /* bytes */
	    output[i] = implode(input[sz .. segments[i] - 1], "");
	    sz = segments[i];
	} else {
	    /* wide characters */
	    output[i] = input[sz .. -segments[i] - 1];
	    sz = -segments[i];
	}
    }
    return output;
}

/*
 * initialize a string
 */
static void create(mixed data, varargs string utf8)
{
    int bufMax, strMax, index, byteOffset, charOffset;
    string remainder;
    object decoder;
    StringBuffer buffer;

    if (utf8 && utf8 != "UTF8") {
	error("Bad encoding");
    }
    bufMax = status(ST_ARRAYSIZE);
    strMax = status(ST_STRSIZE);

    index = byteOffset = charOffset = 0;
    switch (typeof(data)) {
    case T_INT:
    case T_FLOAT:
	data = (string) data;
	/* fall through */
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

    case T_OBJECT:
	buffer = data;
	bytes = allocate(INITIAL_SIZE);
	chars = allocate(INITIAL_SIZE);
	chars[1] = bytes[1] = 0;
	if (!utf8) {
	    for (;;) {
		data = buffer->chunk();
		if (!data) {
		    break;
		}
		if (typeof(data) == T_STRING) {
		    ({ index, byteOffset, charOffset }) =
			appendBytes(data, index, byteOffset, charOffset, bufMax,
				    strMax);
		} else {
		    ({ index, byteOffset, charOffset }) =
			appendSequence(processChars(data), index, byteOffset,
				       charOffset, bufMax, strMax);
		}
	    }
	} else {
	    decoder = find_object(UTF8DECODE);
	    remainder = "";
	    for (;;) {
		data = buffer->chunk();
		if (!data) {
		    break;
		}
		({ remainder, index, byteOffset, charOffset }) =
		    appendUTF8(decoder, remainder, data, index, byteOffset,
			       charOffset, bufMax, strMax);
	    }
	    if (remainder != "") {
		error("Incomplete UTF8 sequence");
	    }
	}
	break;

    default:
	error("Invalid initialization value for String");
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
    if (sizeof(bytes) > ++index) {
	bytes = bytes[.. index];
	chars = chars[.. index];
    }
}

/*
 * return the length of the string
 */
int length()
{
    return chars[sizeof(chars) - 1];
}

/*
 * return TRUE if the string contains bytes only
 */
int isBytes()
{
    return (chars[0] == nil);
}

/*
 * index a string
 */
private mixed *index(int index)
{
    int low, high, mid, offset;
    mixed *buffer;

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
	return ({ buffer, mid, -1, index });
    } else {
	/* array of strings or *int */
	offset = strLength(buffer[mid][0]);
	return ({ buffer, mid, index / offset, index % offset });
    }
}

/*
 * start stream
 */
private mixed *streamStart()
{
    mixed *buf;

    buf = (bytes[0]) ? bytes : chars;
    return ({ buf, 0, (typeof(buf[0][0]) == T_INT) ? -1 : 0 });
}

/*
 * return chunk from stream
 */
private mixed *streamChunk(mixed *buffer, int bufIndex, int offset)
{
    mixed chunk;

    if (!buffer) {
	/* no more chunks */
	return ({ nil, 0, 0, nil });
    }

    if (offset < 0) {
	chunk = buffer[bufIndex];
    } else {
	chunk = buffer[bufIndex][offset];
	if (++offset < sizeof(buffer[bufIndex])) {
	    return ({ buffer, bufIndex, offset, chunk });
	}
    }

    /* switch buffers */
    if (buffer == bytes) {
	buffer = chars;
	if (!buffer[bufIndex]) {
	    /* no trailing chars */
	    return ({ nil, 0, 0, chunk });
	}
	offset = (typeof(buffer[bufIndex][0]) == T_INT) ? -1 : 0;
    } else {
	buffer = bytes;
	bufIndex += 2;
	if (bufIndex >= sizeof(buffer)) {
	    /* out of range */
	    return ({ nil, 0, 0, chunk });
	}
    }

    return ({ buffer, bufIndex, offset, chunk });
}

/*
 * index a string
 */
static int operator[] (int index)
{
    mixed *buffer;
    int bufIndex, offset;

    if (index < 0 || index >= length()) {
	error("String index out of range");
    }

    ({ buffer, bufIndex, offset, index }) = index(index);
    return (offset < 0) ?
	    buffer[bufIndex][index] : buffer[bufIndex][offset][index];
}

/*
 * assignment to string index not permitted
 */
static void operator[]= (int index, int value)
{
    error("Strings are immutable");
}

/*
 * encode a string as UTF8
 */
private void encodeUTF8String(StringBuffer buffer, string chunk, object encoder,
			      int max)
{
    while (strlen(chunk) > max) {
	buffer->append(encoder->encode(chunk[.. max - 1]));
	chunk = chunk[max ..];
    }
    buffer->append(encoder->encode(chunk));
}

/*
 * encode characters as UTF8
 */
private void encodeUTF8Chars(StringBuffer buffer, int *chunk)
{
    string str;
    int i, j, sz, c;

    for (i = j = 0, sz = sizeof(chunk); i < sz; i++) {
	c = chunk[i];
	switch (c) {
	case 0x000100 .. 0x0007ff:
	    str = "  ";
	    str[0] = 0xc0 + (c >> 6);
	    str[1] = 0x80 + (c & 0x3f);
	    buffer->append(str);
	    break;

	case 0x000800 .. 0x00ffff:
	    str = "   ";
	    str[0] = 0xe0 + (c >> 12);
	    str[1] = 0x80 + ((c >> 6) & 0x3f);
	    str[2] = 0x80 + (c & 0x3f);
	    buffer->append(str);
	    break;

	case 0x010000 .. 0x10ffff:
	    str = "    ";
	    str[0] = 0xf0 + (c >> 18);
	    str[1] = 0x80 + ((c >> 12) & 0x3f);
	    str[2] = 0x80 + ((c >> 6) & 0x3f);
	    str[3] = 0x80 + (c & 0x3f);
	    buffer->append(str);
	    break;
	}
    }
}

/*
 * create a StringBuffer for this String
 */
StringBuffer buffer(varargs string utf8)
{
    StringBuffer buffer;
    object encoder;
    int max, bufIndex, offset, index;
    mixed *buf, chunk;

    buffer = new StringBuffer;
    if (!utf8) {
	if (bytes[0] != "") {
	    ({ buf, bufIndex, offset }) = streamStart();

	    for (;;) {
		({ buf, bufIndex, offset, chunk }) =
		    streamChunk(buf, bufIndex, offset);
		if (!chunk) {
		    return buffer;
		}
		buffer->append(chunk);
	    }
	}
    } else {
	if (utf8 != "UTF8") {
	    error("Bad encoding");
	}

	if (bytes[0] != "") {
	    encoder = find_object(UTF8ENCODE);
	    max = status(ST_STRSIZE) >> 1;
	    ({ buf, bufIndex, offset }) = streamStart();

	    for (;;) {
		({ buf, bufIndex, offset, chunk }) =
		    streamChunk(buf, bufIndex, offset);
		switch (typeof(chunk)) {
		case T_NIL:
		    return buffer;

		case T_STRING:
		    encodeUTF8String(buffer, chunk, encoder, max);
		    break;

		case T_ARRAY:
		    encodeUTF8Chars(buffer, chunk);
		    break;
		}
	    }
	}
    }

    return buffer;
}

/*
 * return a StringBuffer for a subrange
 */
StringBuffer bufferRange(varargs mixed from, mixed to)
{
    int length, bufIndex, offset, index, len;
    StringBuffer buffer;
    mixed *buf, chunk;

    /*
     * check subrange
     */
    length = length();
    if (from == nil) {
	from = 0;
    } else if (typeof(from) != T_INT || from < 0 || from >= length) {
	error("Invalid String subrange");
    }
    if (to == nil) {
	to = length - 1;
    } else if (typeof(to) != T_INT || to < from - 1 || to >= length) {
	error("Invalid String subrange");
    }
    if (from == 0 && to == length - 1) {
	return buffer();
    }

    /* determine index and size */
    length = to - from + 1;
    ({ buf, bufIndex, offset, index }) = index(from);

    /*
     * get first chunk
     */
    ({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
    len = strLength(chunk);
    if (index + length < len) {
	chunk = chunk[index .. index + length - 1];
	length = 0;
    } else if (index > 0) {
	chunk = chunk[index ..];
	length -= len - index;
    } else {
	length -= len;
    }
    buffer = new StringBuffer;
    buffer->append(chunk);

    /*
     * get further chunks
     */
    while (length > 0) {
	({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
	len = strLength(chunk);
	if (length < len) {
	    chunk = chunk[.. length - 1];
	}
	length -= len;
	buffer->append(chunk);
    }

    return buffer;
}

/*
 * convert to UTF8-encoded LPC string
 */
string utf8()
{
    StringBuffer buffer;
    string str;

    buffer = buffer("UTF8");
    str = buffer->chunk();
    if (buffer->chunk()) {
	error("String too long");
    }
    return str;
}

/*
 * String subrange
 */
static String operator[..] (mixed from, mixed to)
{
    return new String(bufferRange(from, to));
}

/*
 * append something to a String
 */
static String operator+ (mixed str)
{
    StringBuffer buffer;

    switch (typeof(str)) {
    case T_INT:
    case T_FLOAT:
	str = (string) str;
	/* fall through */
    case T_STRING:
	break;

    case T_OBJECT:
	str = ((String) str)->buffer();
	break;

    default:
	error("Bad value added to String");
    }

    buffer = buffer();
    buffer->append(str);
    return new String(buffer);
}


/*
 * reset an iterator
 */
mixed iteratorStart(mixed from, mixed to)
{
    StringBuffer buffer;
    mixed chunk;

    buffer = bufferRange(from, to);
    chunk = buffer->chunk();
    return ({ buffer, chunk, 0 });
}

/*
 * return the next state for an iterator
 */
mixed *iteratorNext(mixed state)
{
    StringBuffer buffer;
    mixed chunk;
    int index, char;

    ({ buffer, chunk, index }) = state;
    if (!chunk) {
	/* reached the end */
	return ({ ({ nil, nil, 0 }), nil });
    }

    char = chunk[index];
    if (++index == strLength(chunk)) {
	chunk = buffer->chunk();
	index = 0;
    }
    return ({ ({ buffer, chunk, index }), char });
}

/*
 * check if an iterator is at its end
 */
int iteratorEnd(mixed state)
{
    return (!state[1]);
}


/*
 * compare two character arrays
 */
private int compareChars(int *chunk, int *chars, int len)
{
    int i;

    for (i = 0; i < len; i++) {
	if (chunk[i] != chars[i]) {
	    return chunk[i] - chars[i];
	}
    }
    return 0;
}

/*
 * compare two chunks
 */
private int compareChunk(mixed chunk, mixed str)
{
    int len, slen, comp;

    switch (typeof(chunk)) {
    case T_NIL:
	switch (typeof(str)) {
	case T_NIL:
	    /* equal */
	    return 0;

	case T_STRING:
	    /* equal if strlen is 0 */
	    return -strlen(str);
	    break;

	case T_ARRAY:
	    /* "" < array */
	    return -1;
	}

    case T_STRING:
	switch (typeof(str)) {
	case T_STRING:
	    slen = strlen(str);
	    if (slen != 0) {
		break;
	    }
	    /* fall through */
	case T_NIL:
	    /* string > "" */
	    return 1;

	case T_ARRAY:
	    /* string < array */
	    return -1;
	}

	len = strlen(chunk);
	if (len <= slen) {
	    if (len == slen) {
		if (chunk == str) {
		    return 0;
		}
	    } else {
		str = str[.. len - 1];
		if (chunk == str) {
		    return -1;
		}
	    }
	} else {
	    chunk = chunk[.. slen - 1];
	    if (chunk == str) {
		return 1;
	    }
	}
	return (chunk < str) ? -1 : 1;

    case T_ARRAY:
	switch (typeof(str)) {
	case T_NIL:
	case T_STRING:
	    /* array > string */
	    return 1;

	case T_ARRAY:
	    slen = sizeof(str);
	    break;
	}

	len = sizeof(chunk);
	if (len <= slen) {
	    comp = compareChars(chunk, str, len);
	    if (len < slen && comp == 0) {
		return -1;
	    }
	} else {
	    comp = compareChars(chunk, str, slen);
	    if (comp == 0) {
		return 1;
	    }
	}
	return comp;
    }
}

/*
 * compare with other string
 */
int compare(mixed str)
{
    int bufIndex, offset, comp;
    mixed *buf, chunk;

    switch (typeof(str)) {
    case T_STRING:
	/* compare to one chunk */
	if (length() == 0) {
	    return -strlen(str);
	}
	({ buf, bufIndex, offset }) = streamStart();
	({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
	comp = compareChunk(chunk, str);
	if (comp != 0) {
	    return comp;
	}
	if (!buf) {
	    return 0;
	}
	({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
	return !!chunk;

    case T_OBJECT:
	/* stream both Strings, compare chunk by chunk */
	str = ((String) str)->buffer();
	if (length() == 0) {
	    return -str->length();
	}
	({ buf, bufIndex, offset }) = streamStart();
	do {
	    ({ buf, bufIndex, offset, chunk }) =
		streamChunk(buf, bufIndex, offset);
	    comp = compareChunk(chunk, str->chunk());
	    if (comp != 0) {
		return comp;
	    }
	} while (buf);
	return 0;

    default:
	error("Can only compare to String or string");
    }
}

/*
 * this == str
 */
int equals(mixed str)
{
    return (compare(str) == 0);
}

/*
 * this < str
 */
static int operator< (mixed str)
{
    return (compare(str) < 0);
}

/*
 * this <= str
 */
static int operator<= (mixed str)
{
    return (compare(str) <= 0);
}

/*
 * this > str
 */
static int operator> (mixed str)
{
    return (compare(str) > 0);
}

/*
 * this >= str
 */
static int operator>= (mixed str)
{
    return (compare(str) >= 0);
}


/*
 * return String with first character converted to title case
 */
String capitalize()
{
    StringBuffer buffer;
    mixed *buf, chunk;
    int bufIndex, offset;

    if (length() == 0) {
	return new String("");
    }

    /* create StringBuffer and modify first char */
    buffer = new StringBuffer();
    ({ buf, bufIndex, offset }) = streamStart();
    ({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
    if (typeof(chunk) == T_ARRAY) {
	chunk = chunk[..];
    }
    chunk[0] = unicode::toTitleCase(chunk[0]);
    buffer->append(chunk);

    /* copy remaining chunks */
    for (;;) {
	({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
	if (!chunk) {
	    return new String(buffer);
	}
	buffer->append(chunk);
    }
}

/*
 * return String converted to upper case
 */
String toUpperCase()
{
    StringBuffer buffer;
    mixed *buf, chunk;
    int bufIndex, offset, i;

    /* create StringBuffer and modify chars */
    buffer = new StringBuffer();
    ({ buf, bufIndex, offset }) = streamStart();

    for (;;) {
	({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
	switch (typeof(chunk)) {
	case T_NIL:
	    return new String(buffer);

	case T_STRING:
	    i = strlen(chunk);
	    break;

	case T_ARRAY:
	    chunk = chunk[..];
	    i = sizeof(chunk);
	    break;
	}

	while (--i >= 0) {
	    chunk[i] = unicode::toUpperCase(chunk[i]);
	}
	buffer->append(chunk);
    }
}

/*
 * return String converted to lower case
 */
String toLowerCase()
{
    StringBuffer buffer;
    mixed *buf, chunk;
    int bufIndex, offset, i;

    /* create StringBuffer and modify chars */
    buffer = new StringBuffer();
    ({ buf, bufIndex, offset }) = streamStart();

    for (;;) {
	({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
	switch (typeof(chunk)) {
	case T_NIL:
	    return new String(buffer);

	case T_STRING:
	    i = strlen(chunk);
	    break;

	case T_ARRAY:
	    chunk = chunk[..];
	    i = sizeof(chunk);
	    break;
	}

	while (--i >= 0) {
	    chunk[i] = unicode::toLowerCase(chunk[i]);
	}
	buffer->append(chunk);
    }
}

/*
 * fold the case of a chunk
 */
private mixed foldChunk(mixed chunk, int copy)
{
    int i;

    switch (typeof(chunk)) {
    case T_NIL:
	return chunk;

    case T_STRING:
	i = strlen(chunk);
	break;

    case T_ARRAY:
	if (copy) {
	    chunk = chunk[..];
	}
	i = sizeof(chunk);
	break;
    }

    while (--i >= 0) {
	chunk[i] = unicode::foldCase(chunk[i]);
    }

    return chunk;
}

/*
 * compare with other string, ignoring case
 */
int compareIgnoreCase(mixed str)
{
    int bufIndex, offset, comp;
    mixed *buf, chunk;

    switch (typeof(str)) {
    case T_STRING:
	/* compare to one chunk */
	if (length() == 0) {
	    return -strlen(str);
	}
	({ buf, bufIndex, offset }) = streamStart();
	({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
	comp = compareChunk(foldChunk(chunk, TRUE), foldChunk(str, FALSE));
	if (comp != 0) {
	    return comp;
	}
	if (!buf) {
	    return 0;
	}
	({ buf, bufIndex, offset, chunk }) = streamChunk(buf, bufIndex, offset);
	return !!chunk;

    case T_OBJECT:
	/* stream both Strings, compare chunk by chunk */
	str = ((String) str)->buffer();
	if (length() == 0) {
	    return -str->length();
	}
	({ buf, bufIndex, offset }) = streamStart();
	do {
	    ({ buf, bufIndex, offset, chunk }) =
		streamChunk(buf, bufIndex, offset);
	    comp = compareChunk(foldChunk(chunk, TRUE),
				foldChunk(str->chunk(), FALSE));
	    if (comp != 0) {
		return comp;
	    }
	} while (buf);
	return 0;

    default:
	error("Can only compare to String or string");
    }
}

/*
 * this == str, ignoring case
 */
int equalsIgnoreCase(mixed str)
{
    return (compareIgnoreCase(str) == 0);
}
