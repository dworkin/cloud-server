# include <String.h>
# include <StringStream.h>
# include <type.h>

# define UTF8ENCODE	"/sys/utf8encode"


/* buffers */
private mixed *bytes, *str1Bytes, *str2Bytes;
private mixed *chars, *str1Chars, *str2Chars;

/* initial offset */
private mixed *initBuffer;
private int initBufIndex, initChunkOffset, initChunkIndex, initLength;

/* offset */
private mixed *buffer;
private int bufIndex, chunkOffset, chunkIndex, length;

/* current state */
private int state;
private object encoder;

/*
 * NAME:	zeroIndex()
 * DESCRIPTION:	start at offset 0
 */
private void zeroIndex()
{
    if (bytes[0]) {
	initBuffer = bytes;
    } else {
	initBuffer = chars;
    }
    initBufIndex = 0;
    initChunkOffset = (initBuffer[0] && typeof(initBuffer[0][0]) != T_INT) ?
		       0 : -1;
    initChunkIndex = 0;
}

/*
 * NAME:	index()
 * DESCRIPTION:	set initial index for string
 */
private void setIndex(int index)
{
    int low, high, mid, start;
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
	initBuffer = bytes;
    } else {
	/* chars */
	start = bytes[mid + 1];
	initBuffer = chars;
    }
    index -= start;

    initBufIndex = mid;
    if (typeof(initBuffer[mid][0]) == T_INT) {
	/* one string or *int */
	initChunkOffset = -1;
	initChunkIndex = index;
    } else {
	/* array of strings or *int */
	high = sizeof(initBuffer[mid][0]);
	initChunkOffset = index / high;
	initChunkIndex = index % high;
    }
}

/*
 * NAME:	reset()
 * DESCRIPTION:	reset the current state
 */
void reset()
{
    switch (state) {
    case S_STREAM2:
	state = S_STREAM1;
	/* fall through */
    case S_STREAM1:
	bytes = str1Bytes;
	chars = str1Chars;
	/* fall through */
    case S_STREAM:
    case S_UTF8:
	buffer = initBuffer;
	bufIndex = initBufIndex;
	chunkOffset = initChunkOffset;
	chunkIndex = initChunkIndex;
	length = initLength;
	break;
    }
}

/*
 * NAME:	create()
 * DESCRIPTION:	initialize StringStream
 */
static void create(String str, varargs int operation, mixed arg1, mixed arg2)
{
    ({ bytes, chars }) = str->exportData();
    switch (operation) {
    case SSO_STREAM:
	/* stream String */
	zeroIndex();
	initLength = str->length();
	state = S_STREAM;
	reset();
	break;

    case SSO_RANGE:
	/* stream subrange of String */
	length = str->length();
	if (arg1 == nil) {
	    arg1 = 0;
	} else if (typeof(arg1) != T_INT || arg1 < 0 || arg1 >= length) {
	    error("Invalid String subrange");
	}
	if (arg2 == nil) {
	    arg2 = length - 1;
	} else if (typeof(arg2) != T_INT || arg2 < arg1 - 1 || arg2 >= length) {
	    error("Invalid String subrange");
	}

	setIndex(arg1);
	initLength = arg2 - arg1 + 1;
	state = S_STREAM;
	reset();
	break;

    case SSO_ADD:
	/* stream the concatenation of two Strings */
	switch (typeof(arg1)) {
	case T_INT:
	case T_FLOAT:
	    arg1 = (string) arg1;
	    /* fall through */
	case T_STRING:
	    str2Bytes = ({ arg1, strlen(arg1) });
	    str2Chars = ({ nil, strlen(arg1) });
	    initLength = str->length() + strlen(arg1);
	    break;

	case T_OBJECT:
	    if (arg1 <- String) {
		({ str2Bytes, str2Chars }) = arg1->exportData();
		initLength = str->length() + arg1->length();
		break;
	    }
	    /* fall through */
	default:
	    error("Bad argument for String addition");
	}

	str1Bytes = bytes;
	str1Chars = chars;
	zeroIndex();
	state = S_STREAM1;
	reset();
	break;

    case SSO_UTF8:
	/* UTF8-encode String */
	encoder = find_object(UTF8ENCODE);
	zeroIndex();
	initLength = str->length();
	state = S_UTF8;
	reset();
	break;
    }
}

/*
 * NAME:	chunk()
 * DESCRIPTION:	get another chunk of input
 */
mixed chunk()
{
    mixed buf;
    int len;

    if (length == 0) {
	return nil;	/* no chunks left */
    }

    /* get chunk */
    buf = buffer[bufIndex];
    if (chunkOffset >= 0) {
	buf = buf[chunkOffset];
    }
    len = (typeof(buf) == T_STRING) ? strlen(buf) : sizeof(buf);
    if (len - chunkIndex >= length) {
	/* no chunks left */
	buf = buf[chunkIndex .. chunkIndex + length - 1];
	length = 0;
	return buf;
    }
    buf = buf[chunkIndex ..];
    length -= len - chunkIndex;

    /* prepare for next chunk */
    if (chunkOffset >= 0 && ++chunkOffset < sizeof(buffer[bufIndex])) {
	chunkIndex = 0;
    } else {
	/* change buffers */
	if (buffer == bytes && chars[bufIndex]) {
	    /* from bytes to chars */
	    buffer = chars;
	} else {
	    /* from chars to bytes */
	    buffer = bytes;
	    bufIndex += 2;
	    if (bufIndex == sizeof(buffer)) {
		/* switch to added string */
		state = S_STREAM2;
		bytes = str2Bytes;
		chars = str2Chars;
		if (bytes[0]) {
		    buffer = bytes;
		} else {
		    buffer = chars;
		}
		bufIndex = 0;
	    }
	}
	chunkOffset = (typeof(buffer[bufIndex][0]) != T_INT) ? 0 : -1;
	chunkIndex = 0;
    }

    return buf;
}

mixed bufferedChunk()
{
    mixed chunk;

    chunk = chunk();
    return chunk;
}
