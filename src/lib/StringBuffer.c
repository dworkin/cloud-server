# include <String.h>
# include <type.h>
# include <status.h>

# define INITIAL_SIZE	8


private int bufMax, strMax;	/* limits */

/* ring buffer */
private mixed *ringBuffer;	/* ringBuffer for chunks */
private int ringStart;		/* first chunk index */
private int ringEnd;		/* last chunk index */
private int length;		/* total length of data in buffer */

/* chunk buffer */
private int chunkType;		/* type of the last chunk */
private int chunkFilled;	/* full elements in (last) chunks array */
private int chunkBuffered;	/* elements used in (last) chunks array */
private int chunkSize;		/* size of unfull (last) chunks array */
private int chunkIndex;		/* index in (first) chunks array */

/*
 * gather string fragments
 */
private int consolidateString(string *chunks)
{
    int consolidated;

    consolidated = FALSE;
    if (chunkFilled < chunkBuffered - 1) {
	string str;
	int offset;

	--chunkBuffered;
	if (chunkSize > strMax) {
	    /* split last string */
	    str = chunks[chunkBuffered];
	    offset = strlen(str) - (chunkSize - strMax);
	    chunks[chunkBuffered] = str[.. offset - 1];
	    str = str[offset ..];
	}

	/* consolidate string */
	chunks[chunkFilled] = implode(chunks[chunkFilled .. chunkBuffered], "");

	if (chunkSize >= strMax) {
	    chunks[++chunkFilled] = str;
	    chunkSize -= strMax;
	}
	while (chunkBuffered > chunkFilled) {
	    chunks[chunkBuffered] = nil;
	    --chunkBuffered;
	}
	if (chunkSize != 0) {
	    chunkBuffered++;
	}

	consolidated = TRUE;
    }

    if (chunkBuffered < sizeof(chunks)) {
	chunks[chunkBuffered] = nil;
    }
    return consolidated;
}

/*
 * gather character array fragments
 */
private int consolidateChars(int **chunks)
{
    int consolidated;

    consolidated = FALSE;
    if (chunkFilled < chunkBuffered - 1) {
	int size, i, j, k, *chars, *chunk, *fragment;

	--chunkBuffered;
	size = chunkSize;
	if (size > bufMax) {
	    /* split last character array */
	    chars = chunks[chunkBuffered];
	    i = sizeof(chars) - (size - bufMax);
	    chunks[chunkBuffered] = chars[.. i - 1];
	    chars = chars[i ..];
	    size -= bufMax;
	}

	/* consolidate characters */
	chunk = allocate_int(size);
	k = 0;
	for (i = chunkFilled; i <= chunkBuffered; i++) {
	    fragment = chunks[i];
	    for (j = 0, size = sizeof(fragment); j < size; j++) {
		chunk[k++] = fragment[j];
	    }
	}
	chunks[chunkFilled] = chunk;

	if (chunkSize >= bufMax) {
	    chunks[++chunkFilled] = chars;
	    chunkSize -= bufMax;
	}
	while (chunkBuffered > chunkFilled) {
	    chunks[chunkBuffered] = nil;
	    --chunkBuffered;
	}
	if (chunkSize != 0) {
	    chunkBuffered++;
	}

	consolidated = TRUE;
    }

    if (chunkBuffered < sizeof(chunks)) {
	chunks[chunkBuffered] = nil;
    }
    return consolidated;
}

/*
 * prepare to append data of a given type
 */
private void prepareType(int type)
{
    if (chunkType != type) {
	if (chunkType != T_NIL) {
	    int size;

	    if (chunkType == T_STRING) {
		consolidateString(ringBuffer[ringEnd]);
	    } else {
		consolidateChars(ringBuffer[ringEnd]);
	    }

	    size = sizeof(ringBuffer);
	    if ((ringEnd + 1) % size == ringStart) {
		/*
		 * grow ring
		 */
		if (size == bufMax) {
		    error("Buffer full");
		}
		if (size > bufMax - size) {
		    size = bufMax - size;
		}
		ringBuffer = ringBuffer[.. ringStart - 1] + allocate(size) +
			     ringBuffer[ringStart ..];
		ringStart += size;
	    }
	}
	ringEnd = (ringEnd + 1) % sizeof(ringBuffer);
	chunkType = type;
	chunkFilled = chunkBuffered = chunkSize = 0;
    }
}

/*
 * append a string to the StringBuffer
 */
private void appendString(string str)
{
    string *chunks;
    int size;

    prepareType(T_STRING);

    chunks = ringBuffer[ringEnd];
    if (!chunks) {
	chunks = allocate(INITIAL_SIZE);
    }

    size = sizeof(chunks);
    if (chunkBuffered == size) {
	if (size < bufMax) {
	    if (size > bufMax - size) {
		size = bufMax - size;
	    }
	    chunks += allocate(size);
	} else if (!consolidateString(chunks)) {
	    error("Buffer full");
	}
    }

    chunks[chunkBuffered++] = str;
    chunkSize += strlen(str);
    if (chunkSize >= strMax) {
	consolidateString(chunks);
    }

    ringBuffer[ringEnd] = chunks;
    length += strlen(str);
}

/*
 * append an array of characters to the StringBuffer
 */
private void appendChars(int *chars)
{
    mixed **chunks;
    int size;

    prepareType(T_ARRAY);

    chunks = ringBuffer[ringEnd];
    if (!chunks) {
	chunks = allocate(INITIAL_SIZE);
    }

    size = sizeof(chunks);
    if (chunkBuffered == size) {
	if (size < bufMax) {
	    if (size > bufMax - size) {
		size = bufMax - size;
	    }
	    chunks += allocate(size);
	} else if (!consolidateChars(chunks)) {
	    error("Buffer full");
	}
    }

    chunks[chunkBuffered++] = chars;
    chunkSize += sizeof(chars);
    if (chunkSize >= bufMax) {
	consolidateChars(chunks);
    }

    ringBuffer[ringEnd] = chunks;
    length += sizeof(chars);
}

/*
 * append to the StringBuffer
 */
void append(mixed str)
{
    StringBuffer buffer;
    mixed chunk;

    switch (typeof(str)) {
    case T_STRING:
	appendString(str);
	break;

    case T_ARRAY:
	appendChars(str);
	break;

    case T_OBJECT:
	buffer = str;
	if (buffer == this_object()) {
	    error("StringBuffer appended to itself");
	}

	/*
	 * append a different StringBuffer
	 */
	for (;;) {
	    chunk = buffer->chunk();
	    if (!chunk) {
		break;
	    }

	    if (typeof(chunk) == T_STRING) {
		appendString(chunk);
	    } else {
		appendChars(chunk);
	    }
	}
	break;

    default:
	error("Invalid value appended to StringBuffer");
    }
}

/*
 * return a chunk of the maximum appropriate length
 */
mixed chunk()
{
    mixed chunks, chunk;

    do {
	if (chunkType == T_NIL) {
	    return nil;
	}

	chunks = ringBuffer[ringStart];
	if (ringStart == ringEnd) {
	    if (chunkType == T_STRING) {
		consolidateString(chunks);
	    } else {
		consolidateChars(chunks);
	    }
	}

	chunk = chunks[chunkIndex];
	chunks[chunkIndex++] = nil;
	if (!chunk || chunkIndex == sizeof(chunks)) {
	    if (ringStart == ringEnd) {
		chunkType = T_NIL;
	    }
	    ringStart = (ringStart + 1) % sizeof(ringBuffer);
	    chunkIndex = 0;
	}
    } while (!chunk);

    length -= (typeof(chunk) == T_STRING) ? strlen(chunk) : sizeof(chunk);
    return chunk;
}

/*
 * return the length of the String in the buffer
 */
int length()
{
    return length;
}

/*
 * create StringBuffer
 */
static void create(varargs mixed str)
{
    bufMax = status(ST_ARRAYSIZE);
    strMax = status(ST_STRSIZE);
    ringBuffer = allocate(INITIAL_SIZE);
    ringStart = 0;
    ringEnd = -1;
    length = 0;
    chunkType = T_NIL;
    chunkIndex = 0;
    if (str) {
	append(str);
    }
}
