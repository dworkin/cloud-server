# include <limits.h>


# define CHUNK_SIZE	65535

int length;		/* total length of strings in chunks */
string *chunks;		/* string chunks */

/*
 * NAME:	init()
 * DESCRIPTION:	initialize strchunk
 */
static void init(varargs string str)
{
    if (str) {
	length = strlen(str);
	chunks = ({ str });
    }
}

/*
 * NAME:	append()
 * DESCRIPTION:	append string to chunks
 */
void append(string str)
{
    int len, i, clen;
    string chunk;

    len = strlen(str);
    if (!chunks) {
	length = len;
	chunks = ({ str });
	return;
    }

    if (len == 0) {
	return;
    }
    length += len;
    i = sizeof(chunks) - 1;
    chunk = chunks[i];
    clen = strlen(chunk);
    if (clen < CHUNK_SIZE) {
	if (clen + len <= CHUNK_SIZE) {
	    chunks[i] = chunk + str;
	    return;
	}
	clen = CHUNK_SIZE - clen;
	chunks[i] = chunk + str[.. clen - 1];
	str = str[clen ..];
    }
    chunks += ({ str });
}

/*
 * NAME:	chunk()
 * DESCRIPTION:	remove the first of the chunks
 */
string chunk()
{
    if (chunks) {
	string str;

	str = chunks[0];
	if (length <= CHUNK_SIZE) {
	    chunks = nil;
	} else {
	    length -= CHUNK_SIZE;
	    chunks = chunks[1 ..];
	}
	return str;
    } else {
	return nil;
    }
}

/*
 * NAME:	length()
 * DESCRIPTION:	return total length of strings in chunks
 */
int length()
{
    return (chunks) ? length : -1;
}
