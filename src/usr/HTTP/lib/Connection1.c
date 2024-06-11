# include <kernel/user.h>
# include <String.h>
# include "HttpRequest.h"
# include "HttpResponse.h"
# include "HttpField.h"
# include <kfun.h>

inherit "Connection";
private inherit hex "/lib/util/hex";

# define HTTP_TOKENPARAM	"/usr/HTTP/sys/tokenparam"


/* implemented at the top layer */
static int message(string str);
static void set_mode(int mode);
static void set_message_length(int length);
static void disconnect();

private object relay;		/* object to relay to */
private string trailersPath;	/* HttpFields object path */
private string frame;		/* HTTP 1.x headers/trailers or WsFrame */
private int inchunk;		/* expecting chunk? */
private string transform;	/* compression or masking */
private StringBuffer inbuf;	/* entity included in request/response */
private int length;		/* length of entity to receive */
private StringBuffer outbuf;	/* output buffer */
private int quiet;		/* quiet output? */
private int persistent;		/* is connection persistent? */
private int webSocket;		/* WebSocket enabled? */

/*
 * initialize HTTP/1.x connection
 */
static void create(object relay, string trailersPath)
{
    ::relay = relay;
    ::trailersPath = trailersPath;
}

/*
 * set the receive mode
 */
static void setMode(int mode)
{
    set_mode(mode);
}

/*
 * set the receive buffer size
 */
static void setMessageLength(int length)
{
    set_message_length(length);
}

/*
 * received request line
 */
static int receiveRequestLine(HttpRequest request)
{
    int code;

    code = ::receiveRequestLine(request);
    if (code != 0) {
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

    case "transfer-encoding":
	if (!header->listContains("chunked")) {
	    error("Transfer-Encoding without chunked");
	}
	break;
    }

    return ::receiveRequestHeader(request, header);
}

/*
 * received response status line
 */
static void receiveStatusLine(HttpResponse response)
{
    float version;

    version = response->version();
    if (floor(version) != 1.0) {
	error("Server version out of range");
    }

    if (version < 1.1) {
	persistent = FALSE;
    }
    ::receiveStatusLine(response);
}

/*
 * received HTTP response header
 */
static void receiveResponseHeader(HttpResponse response, HttpField header)
{
    switch (header->lcName()) {
    case "connection":
	if (header->listContains("close")) {
	    persistent = FALSE;
	}
	break;
    }
    ::receiveResponseHeader(response, header);
}

/*
 * begin receiving headers
 */
static void startHeaders()
{
    frame = "";
}

# ifdef KF_GUNZIP
/*
 * gunzip input from the connection
 */
private StringBuffer gunzip(StringBuffer input)
{
    StringBuffer buffer;
    string chunk, *decompressed;
    int i, size;

    buffer = new StringBuffer;

    if (input) {
	while (chunk=input->chunk()) {
	    decompressed = ::gunzip(chunk);
	    for (i = 0, size = sizeof(decompressed); i < size; i++) {
		buffer->append(decompressed[i]);
	    }
	}
    } else {
	decompressed = ::gunzip();
	for (i = 0, size = sizeof(decompressed); i < size; i++) {
	    buffer->append(decompressed[i]);
	}
    }

    return buffer;
}
# endif

# ifdef KF_INFLATE
/*
 * inflate input from the connection
 */
private StringBuffer inflate(StringBuffer input)
{
    StringBuffer buffer;
    string chunk, *decompressed;
    int i, size;

    buffer = new StringBuffer;

    if (input) {
	while (chunk=input->chunk()) {
	    decompressed = ::inflate(chunk);
	    for (i = 0, size = sizeof(decompressed); i < size; i++) {
		buffer->append(decompressed[i]);
	    }
	}
    } else {
	decompressed = ::inflate();
	for (i = 0, size = sizeof(decompressed); i < size; i++) {
	    buffer->append(decompressed[i]);
	}
    }

    return buffer;
}
# endif

/*
 * prepare to receive an entity
 */
void expectEntity(int length)
{
    if (previous_object() == relay && length != 0) {
	setMode(MODE_RAW);
	inbuf = new StringBuffer;
	setMessageLength(::length = length);
    }
}

/*
 * expect chunked data
 */
void expectChunk(varargs string compression)
{
    if (previous_object() == relay) {
	setMode(MODE_LINE);
	inchunk = TRUE;
	transform = compression;
    }
}

/*
 * receive the first line of a chunk
 */
static int receiveChunkLine(string str)
{
    string token, *params;

    ({ token, params }) = HTTP_TOKENPARAM->tokenparam(str);
    length = hex::decode(token);
    relay->receiveChunkLine(length, params);

    if (length != 0) {
	inbuf = new StringBuffer;
	setMessageLength(length);
	return MODE_RAW;
    } else {
	frame = "";
	return MODE_LINE;
    }
}

/*
 * receive a chunk
 */
static void receiveChunk(StringBuffer chunk, varargs HttpFields trailers)
{
    inchunk = FALSE;
    switch (transform) {
# ifdef KF_GUNZIP
    case "gzip":
	chunk = gunzip(chunk);
	break;
# endif

# ifdef KF_INFLATE
    case "deflate":
	chunk = inflate(chunk);
	break;
# endif
    }
    relay->receiveChunk(chunk, trailers);
}

/*
 * received a WebSocket chunk
 */
static void receiveWsChunk(StringBuffer chunk)
{
    if (transform) {
	chunk = maskWsChunk(chunk, transform);
    }
    setMode(MODE_BLOCK);
    inchunk = FALSE;
    relay->receiveWsChunk(chunk);
}

/*
 * receive a WebSocket frame
 */
static string receiveWsFrame(string str)
{
    mixed *result;
    int opcode, flags, len;

    while (inchunk && (result=::receiveWsFrame(str))) {
	({ opcode, flags, len, transform, str }) = result;
	relay->receiveWsFrame(opcode, flags, len);

	if (len > strlen(str)) {
	    /* incomplete */
	    setMessageLength(::length = len - strlen(str));
	    inbuf = new StringBuffer(str);
	    return nil;
	}

	/* process a WebSocket chunk */
	receiveWsChunk(new StringBuffer(str[.. len - 1]));
	str = str[len ..];
    }

    return str;
}

/*
 * expect a WebSocket frame
 */
void expectWsFrame()
{
    if (previous_object() == relay) {
	string str;

	setMode(MODE_RAW);
	inchunk = TRUE;

	if (!webSocket) {
	    webSocket = TRUE;
	    frame = "";
	} else if (frame) {
	    str = frame;
	    frame = nil;

	    frame = receiveWsFrame(str);
	}
    }
}

static int receiveHeaders(string str);
static int receiveMessage(string str);

/*
 * receive (part of) message
 */
static int receiveBytes(string str)
{
    StringBuffer chunk;

    if (webSocket) {
	try {
	    if (frame) {
		str = frame + str;
		frame = nil;

		frame = receiveWsFrame(str);
	    } else {
		inbuf->append(str);
		length -= strlen(str);
		if (length == 0) {
		    chunk = inbuf;
		    inbuf = nil;

		    receiveWsChunk(chunk);
		}
	    }
	    return MODE_NOCHANGE;
	} catch (err) {
	    relay->receiveError(err);
	    return MODE_DISCONNECT;
	}
    } else if (frame) {
	/*
	 * headers or trailers
	 */
	if (strlen(str) != 0) {
	    frame += str + "\n";
	    return MODE_NOCHANGE;
	}
	str = frame;
	frame = nil;

	setMode(MODE_BLOCK);
	if (!inchunk) {
	    /*
	     * headers
	     */
	    return receiveHeaders(str);
	} else {
	    /*
	     * chunk trailers
	     */
	    try {
		receiveChunk(nil, (strlen(str) != 0) ?
				   new_object(trailersPath, str) : nil);
		return MODE_NOCHANGE;
	    } catch (err) {
		relay->receiveError(err);
		return MODE_DISCONNECT;
	    }
	}
    } else if (inbuf) {
	/*
	 * entity/chunk
	 */
	if (length == 0) {
	    /*
	     * end of chunk
	     */
	    if (strlen(str) != 0) {
		/* \r\n expected */
		relay->receiveError("HTTP protocol error");
		return MODE_DISCONNECT;
	    }

	    chunk = inbuf;
	    inbuf = nil;

	    setMode(MODE_BLOCK);
	    try {
		receiveChunk(chunk);
	    } catch (err) {
		relay->receiveError(err);
		return MODE_DISCONNECT;
	    }
	} else {
	    inbuf->append(str);
	    length -= strlen(str);
	    if (length == 0) {
		if (inchunk) {
		    return MODE_LINE;	/* followed by \r\n */
		}

		chunk = inbuf;
		inbuf = nil;

		setMode(MODE_BLOCK);
		relay->receiveEntity(chunk);
	    }
	}

	return MODE_NOCHANGE;
    } else if (inchunk) {
	/*
	 * chunk line
	 */
	try {
	    return receiveChunkLine(str);
	} catch (err) {
	    relay->receiveError(err);
	    return MODE_DISCONNECT;
	}
    } else {
	/*
	 * request/response
	 */
	return receiveMessage(str);
    }
}


/*
 * send (a chunk of) a message
 */
static void messageChunk()
{
    string str;

    if (!outbuf || !(str=outbuf->chunk())) {
	outbuf = nil;
	if (!quiet) {
	    relay->doneChunk();
	}
    } else {
	while (message(str)) {
	    str = outbuf->chunk();
	    if (!str) {
		outbuf = nil;
		break;
	    }
	}
    }
}

/*
 * output remainder of message
 */
static int messageDone()
{
    messageChunk();
    return MODE_NOCHANGE;
}

/*
 * send a message
 */
static void sendMessage(StringBuffer chunk, varargs int quiet, int hold)
{
    if (outbuf) {
	outbuf->append(chunk);
	::quiet &= quiet;
    } else {
	outbuf = chunk;
	::quiet = quiet;
	messageChunk();
    }
}

/*
 * wrap data in a chunk
 */
private StringBuffer chunked(StringBuffer buffer, varargs string *params,
			     HttpFields trailers)
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
    } else if (trailers) {
	chunk->append(trailers->transport());
    }

    chunk->append("\r\n");
    return chunk;
}

/*
 * send a chunk of data
 */
void sendChunk(StringBuffer chunk, varargs string *params)
{
    if (previous_object() == relay && chunk->length() != 0) {
	if (params) {
	    chunk = chunked(chunk, params);
	}
	sendMessage(chunk);
    }
}

/*
 * send a length 0 chunk
 */
void endChunk(varargs string *params, HttpFields trailers)
{
    if (previous_object() == relay) {
	sendMessage(chunked(nil, params, trailers));
    }
}

/*
 * send a WebSocket chunk
 */
void sendWsChunk(int opcode, int flags, varargs int mask, StringBuffer chunk)
{
    if (previous_object() == relay) {
	StringBuffer buffer;
	string str;

	if (mask != 0) {
	    str = "....";
	    str[0] = mask >> 24;
	    str[1] = mask >> 16;
	    str[2] = mask >> 8;
	    str[3] = mask;
	}

	buffer = new StringBuffer(sendWsFrame(opcode, flags,
					      (chunk) ? chunk->length() : 0,
					      str));
	if (chunk) {
	    if (mask != 0) {
		chunk = maskWsChunk(chunk, str);
	    }
	    buffer->append(chunk);
	}
	sendMessage(buffer);
    }
}

/*
 * break the connection
 */
void terminate()
{
    if (previous_object() == this_object() || previous_object() == relay) {
	disconnect();
    }
}

/*
 * connection terminated
 */
static void close(int quit)
{
    relay->disconnected();
}

/*
 * prepare to send a HTTP request
 */
static void sendRequest(HttpRequest request)
{
    HttpField connection;

    if (request->version() != 1.1) {
	error("Unsupported HTTP version");
    }
    connection = request->headers()->get("connection");
    persistent = !(connection && connection->listContains("close"));
    ::sendRequest(request);
    setMode(MODE_LINE);
}

/*
 * prepare to send a HTTP response
 */
static void sendResponse(HttpResponse response)
{
    HttpField connection;

    if (response->version() != 1.1) {
	error("Unsupported HTTP version");
    }
    connection = response->headers()->get("connection");
    if (connection && connection->listContains("close")) {
	persistent = FALSE;
    }
    ::sendResponse(response);
}


int persistent()	{ return persistent; }
int webSocket()		{ return webSocket; }
