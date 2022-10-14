# include "HttpConnection.h"
# include "HttpRequest.h"
# include "HttpResponse.h"
# include "HttpField.h"
# include <String.h>
# include <kfun.h>

inherit HttpConnection;
private inherit hex "/lib/util/hex";

# define HTTP_TOKENPARAM	"/usr/HTTP/sys/tokenparam"


private int persistent;		/* is connection persistent? */

/*
 * received request line
 */
static int receiveRequestLine(HttpRequest request)
{
    int code;

    code = ::receiveRequestLine(request);
    if (code) {
	return code;
    }

    persistent = (request->version() >= 1.1);

    return (request->version() >= 2.0) ? HTTP_NOT_IMPLEMENTED : 0;
}

/*
 * received HTTP request header
 */
static int receiveRequestHeader(HttpRequest request, HttpField header)
{
    switch (header->lcName()) {
    case "connection":
	if (header->listContains("close")) {
	    persistent = FALSE;
	}
	break;
    }

    return ::receiveRequestHeader(request, header);
}

/*
 * wrap data in a chunk
 */
static StringBuffer chunked(StringBuffer buffer, varargs string *params,
			    HttpFields trailer)
{
    StringBuffer chunk;
    int length;

    chunk = new StringBuffer;

    length = (buffer) ? buffer->length() : 0;
    chunk->append(hex::encode(length));
    if (params && sizeof(params) != 0) {
	chunk->append(";" + implode(params, ";"));
    }
    chunk->append("\r\n");

    if (length != 0) {
	chunk->append(buffer);
    } else if (trailer) {
	chunk->append(trailer->transport());
    }

    chunk->append("\r\n");
    return chunk;
}

/*
 * parse a chunk line
 */
static mixed *chunkLine(string input)
{
    string token, *params;

    ({ token, params }) = HTTP_TOKENPARAM->tokenparam(input);
    return ({ hex::decode(token), params });
}

# ifdef KF_GUNZIP
/*
 * gunzip input from the connection
 */
static StringBuffer gunzip(StringBuffer buffer)
{
    StringBuffer outbuf;
    string chunk, *decompressed;
    int i, size;

    outbuf = new StringBuffer;

    if (buffer) {
	while (chunk=buffer->chunk()) {
	    decompressed = ::gunzip(chunk);
	    for (i = 0, size = sizeof(decompressed); i < size; i++) {
		outbuf->append(decompressed[i]);
	    }
	}
    } else {
	decompressed = ::gunzip();
	for (i = 0, size = sizeof(decompressed); i < size; i++) {
	    outbuf->append(decompressed[i]);
	}
    }

    return outbuf;
}
# endif

# ifdef KF_INFLATE
/*
 * inflate input from the connection
 */
static StringBuffer inflate(StringBuffer buffer)
{
    StringBuffer outbuf;
    string chunk, *decompressed;
    int i, size;

    outbuf = new StringBuffer;

    if (buffer) {
	while (chunk=buffer->chunk()) {
	    decompressed = ::inflate(chunk);
	    for (i = 0, size = sizeof(decompressed); i < size; i++) {
		outbuf->append(decompressed[i]);
	    }
	}
    } else {
	decompressed = ::inflate();
	for (i = 0, size = sizeof(decompressed); i < size; i++) {
	    outbuf->append(decompressed[i]);
	}
    }

    return outbuf;
}
# endif


static int persistent()	{ return persistent; }
