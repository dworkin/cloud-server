# include <limits.h>


# define CHUNK_SIZE	65535

int length;		/* total length of strings in buffer */
string *buffer;		/* string buffer */

/*
 * NAME:	init()
 * DESCRIPTION:	initialize strchunk
 */
static void init(varargs string str)
{
    if (str) {
	length = strlen(str);
	buffer = ({ str });
    }
}

/*
 * NAME:	append()
 * DESCRIPTION:	append string to buffer
 */
void append(string str)
{
    int len, i, clen;
    string chunk;

    len = strlen(str);
    if (!buffer) {
	length = len;
	buffer = ({ str });
	return;
    }

    if (len == 0) {
	return;
    }
    length += len;
    i = sizeof(buffer) - 1;
    chunk = buffer[i];
    clen = strlen(chunk);
    if (clen < CHUNK_SIZE) {
	if (clen + len <= CHUNK_SIZE) {
	    buffer[i] = chunk + str;
	    return;
	}
	clen = CHUNK_SIZE - clen;
	buffer[i] = chunk + str[.. clen - 1];
	str = str[clen ..];
    }
    buffer += ({ str });
}

/*
 * NAME:	chunk()
 * DESCRIPTION:	remove the first chunk from the buffer
 */
string chunk()
{
    if (buffer) {
	string str;

	str = buffer[0];
	if (length <= CHUNK_SIZE) {
	    buffer = nil;
	} else {
	    length -= CHUNK_SIZE;
	    buffer = buffer[1 ..];
	}
	return str;
    } else {
	return nil;
    }
}

/*
 * NAME:	length()
 * DESCRIPTION:	return total length of strings in buffer
 */
int length()
{
    return (buffer) ? length : -1;
}
