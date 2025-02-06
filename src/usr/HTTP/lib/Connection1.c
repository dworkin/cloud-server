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
static void flow_message(object message);
static void flow_mode(int mode, varargs int length);
static void disconnect();

static int inactivityTimeout();
static void receiveHeaders(string str);
static int receiveMessage(string str);

private object relay;		/* object to relay to */
private string trailersPath;	/* HttpFields object path */
private string frame;		/* HTTP 1.x headers/trailers or WsFrame */
private int inchunk;		/* expecting chunk? */
private string transform;	/* compression or masking */
private StringBuffer inbuf;	/* entity included in request/response */
private int length;		/* length of entity to receive */
private int active;		/* activity time */
private StringBuffer outbuf;	/* output buffer */
private int idle;		/* idle mode? */
private int quiet;		/* quiet output? */
private int persistent;		/* is connection persistent? */
private int webSocket;		/* WebSocket enabled? */

/*
 * initialize HTTP/1.x connection
 */
static void create(object relay, string trailersPath)
{
    int timeout;

    ::relay = relay;
    ::trailersPath = trailersPath;
    active = time();
    timeout = inactivityTimeout();
    if (timeout != 0) {
	call_out("inactive", timeout);
    }
}

/*
 * set the receive mode
 */
static void setMode(int mode, varargs int length)
{
    flow_mode(mode, length);
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
 * received HTTP request
 */
static void receiveRequest(int code, HttpRequest request)
{
    relay->receiveRequest(code, request);
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
 * received HTTP response
 */
static void receiveResponse(HttpResponse response)
{
    relay->receiveResponse(response);
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
 * be idle, but don't block input
 */
static void idle()
{
    idle = TRUE;
    setMode(MODE_RAW, 1);
}

/*
 * prepare to receive an entity
 */
static void _expectEntity(int length, object prev)
{
    if (prev == relay && length != 0) {
	inbuf = new StringBuffer;
	setMode(MODE_RAW, ::length = length);
    }
}

/*
 * flow: prepare to receive an entity
 */
void expectEntity(int length)
{
    call_out("_expectEntity", 0, length, previous_object());
}

/*
 * expect chunked data
 */
static void _expectChunk(string compression, object prev)
{
    if (prev == relay) {
	inchunk = TRUE;
	transform = compression;
	setMode(MODE_LINE);
    }
}

/*
 * flow: expect chunked data
 */
void expectChunk(varargs string compression)
{
    call_out("_expectChunk", 0, compression, previous_object());
}

/*
 * receive the first line of a chunk
 */
static void receiveChunkLine(string str)
{
    string token, *params;

    ({ token, params }) = HTTP_TOKENPARAM->tokenparam(str);
    length = hex::decode(token);
    relay->receiveChunkLine(length, params);

    if (length != 0) {
	inbuf = new StringBuffer;
	setMode(MODE_RAW, length);
    } else {
	frame = "";
	setMode(MODE_LINE);
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

    if (inchunk && (result=::receiveWsFrame(str))) {
	({ opcode, flags, len, transform, str }) = result;
	relay->receiveWsFrame(opcode, flags, len);

	if (len > strlen(str)) {
	    /* incomplete */
	    inbuf = new StringBuffer(str);
	    setMode(MODE_RAW, ::length = len - strlen(str));
	    return nil;
	}

	/* process a WebSocket chunk */
	receiveWsChunk(new StringBuffer(str[.. len - 1]));
	str = str[len ..];
    } else {
	setMode(MODE_RAW);
    }

    return str;
}

/*
 * expect a WebSocket frame
 */
static void _expectWsFrame(object prev)
{
    if (prev == relay) {
	string str;

	inchunk = TRUE;

	if (!webSocket) {
	    webSocket = TRUE;
	    frame = "";
	} else if (frame) {
	    str = frame;
	    frame = nil;

	    frame = receiveWsFrame(str);
	    return;
	}

	setMode(MODE_RAW);
    }
}

/*
 * flow: expect a WebSocket frame
 */
void expectWsFrame()
{
    call_out("_expectWsFrame", 0, previous_object());
}

/*
 * receive (part of) message
 */
static void receiveBytes(string str)
{
    StringBuffer chunk;

    active = time();
    if (idle) {
	relay->receiveError("HTTP protocol error");
	disconnect();
    } else if (webSocket) {
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
		} else {
		    setMode(MODE_UNBLOCK);
		}
	    }
	} catch (err) {
	    relay->receiveError(err);
	    disconnect();
	}
    } else if (frame) {
	/*
	 * headers or trailers
	 */
	if (strlen(str) != 0) {
	    frame += str + "\n";
	    setMode(MODE_LINE);
	} else {
	    str = frame;
	    frame = nil;

	    if (!inchunk) {
		/*
		 * headers
		 */
		receiveHeaders(str);
	    } else {
		/*
		 * chunk trailers
		 */
		try {
		    receiveChunk(nil, (strlen(str) != 0) ?
				       new_object(trailersPath, str) : nil);
		} catch (err) {
		    relay->receiveError(err);
		    disconnect();
		}
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
		disconnect();
	    } else {
		chunk = inbuf;
		inbuf = nil;

		try {
		    receiveChunk(chunk);
		} catch (err) {
		    relay->receiveError(err);
		    disconnect();
		}
	    }
	} else {
	    inbuf->append(str);
	    length -= strlen(str);
	    if (length == 0) {
		if (inchunk) {
		    setMode(MODE_LINE);		/* followed by \r\n */
		} else {
		    chunk = inbuf;
		    inbuf = nil;

		    relay->receiveEntity(chunk);
		}
	    } else {
		setMode(MODE_UNBLOCK);
	    }
	}
    } else if (inchunk) {
	/*
	 * chunk line
	 */
	try {
	    receiveChunkLine(str);
	} catch (err) {
	    relay->receiveError(err);
	    disconnect();
	}
    } else {
	/*
	 * request/response
	 */
	setMode(receiveMessage(str));
    }
}


/*
 * send (a chunk of) a message
 */
static void messageChunk()
{
    if (!outbuf || outbuf->length() == 0) {
	outbuf = nil;
	if (!quiet) {
	    relay->doneChunk();
	}
    } else {
	flow_message(outbuf);
	outbuf = new StringBuffer;
    }
}

/*
 * output remainder of message
 */
static void messageDone()
{
    messageChunk();
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
static void _sendChunk(StringBuffer chunk, string *params, object prev)
{
    if (prev == relay && chunk->length() != 0) {
	if (params) {
	    chunk = chunked(chunk, params);
	}
	sendMessage(chunk);
    }
}

/*
 * flow: send a chunk of data
 */
void sendChunk(StringBuffer chunk, varargs string *params)
{
    call_out("_sendChunk", 0, chunk, params, previous_object());
}

/*
 * send a length 0 chunk
 */
static void _endChunk(string *params, HttpFields trailers, object prev)
{
    if (prev == relay) {
	sendMessage(chunked(nil, params, trailers));
    }
}

/*
 * flow: send a length 0 chunk
 */
void endChunk(varargs string *params, HttpFields trailers)
{
    call_out("_endChunk", 0, params, trailers, previous_object());
}

/*
 * send a WebSocket chunk
 */
static void _sendWsChunk(int opcode, int flags, int mask, StringBuffer chunk,
			 object prev)
{
    if (prev == relay) {
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
 * flow: send a WebSocket chunk
 */
void sendWsChunk(int opcode, int flags, varargs int mask, StringBuffer chunk)
{
    call_out("_sendWsChunk", 0, opcode, flags, mask, chunk, previous_object());
}

/*
 * break the connection
 */
static void _terminate(object prev)
{
    if (prev == relay) {
	disconnect();
    }
}

/*
 * flow: break the connection
 */
void terminate()
{
    call_out("_terminate", 0, previous_object());
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
    idle = FALSE;
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

/*
 * see if the timeout for inactivity was reached
 */
static void inactive()
{
    int inactive, timeout;

    inactive = time() - active;
    timeout = inactivityTimeout();
    if (inactive >= timeout) {
	disconnect();
    } else {
	call_out("inactive", timeout - inactive);
    }
}


int persistent()	{ return persistent; }
int webSocket()		{ return webSocket; }
