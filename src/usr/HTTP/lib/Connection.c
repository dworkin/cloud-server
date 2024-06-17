# include "HttpRequest.h"
# include "HttpField.h"
# include "HttpResponse.h"
# include "HttpConnection.h"
# include <Iterator.h>
# include <String.h>
# include <kfun.h>


/*
 * received HTTP request line
 */
static int receiveRequestLine(HttpRequest request)
{
    if (!request->path()) {
	/* invalid request */
	return HTTP_BAD_REQUEST;
    }

    return 0;
}

/*
 * received HTTP request header
 */
static int receiveRequestHeader(HttpRequest request, HttpField header)
{
    switch (header->lcName()) {
    case "host":
	request->setHost(header->value());
	break;
    }

    return 0;
}

/*
 * received HTTP request headers
 */
static int receiveRequestHeaders(HttpRequest request, HttpFields headers)
{
    Iterator i;
    HttpField header;
    int code;

    for (i = headers->iterator(); (header=i->next()); ) {
	code = receiveRequestHeader(request, header);
	if (code != 0) {
	    return code;
	}
    }

    request->setHeaders(headers);
    return 0;
}

/*
 * received HTTP response status line
 */
static void receiveStatusLine(HttpResponse response)
{
}

/*
 * received HTTP response header
 */
static void receiveResponseHeader(HttpResponse response, HttpField header)
{
}

/*
 * received HTTP response headers
 */
static int receiveResponseHeaders(HttpResponse response, HttpFields headers)
{
    Iterator i;
    HttpField header;

    for (i = headers->iterator(); (header=i->next()); ) {
	receiveResponseHeader(response, header);
    }

    response->setHeaders(headers);
}

/*
 * attempt to receive a WebSocket frame, return nil if the frame is not yet
 * complete
 */
static mixed *receiveWsFrame(string str)
{
    int len, masked, dataLen, opcode;
    string mask;

    len = strlen(str);
    if (len < 2) {
	return nil;	/* incomplete */
    }

    masked = (str[1] & 0x80) >> 5;
    dataLen = str[1] & 0x7f;
    if (dataLen >= 126) {
	if (dataLen == 126) {
	    /* two-byte length */
	    if (len < 4 + masked) {
		return nil;	/* incomplete */
	    }
	    dataLen = (str[2] << 8) | str[3];
	    len = 4;
	} else if (len < 10 + masked) {
	    return nil;	/* incomplete */
	} else {
	    /* eight-byte length */
	    dataLen = (str[2] << 56) | (str[3] << 48) | (str[4] << 40) |
		      (str[5] << 32) | (str[6] << 24) | (str[7] << 16) |
		      (str[8] << 8) | str[9];
	    len = 10;
	}
    } else if (len < 2 + masked) {
	return nil;	/* incomplete */
    } else {
	len = 2;	/* one-byte length */
    }

    /* check opcode */
    opcode = str[0] & WEBSOCK_OPCODE;
    if (opcode > WEBSOCK_BINARY &&
	(opcode < WEBSOCK_CLOSE || opcode > WEBSOCK_PONG)) {
	error("Invalid opcode in WebSocket frame");
    }

    if (masked) {
	/* set mask */
	mask = str[len .. len + 3];
	len += 4;
    }

    return ({ opcode, str[0] & WEBSOCK_FLAGS, dataLen, mask, str[len ..] });
}

/*
 * mask WebSocket chunk
 */
static StringBuffer maskWsChunk(StringBuffer buffer, string mask)
{
    StringBuffer masked;
    string remainder, str;
    int offset, len;

    masked = new StringBuffer;

    for (offset = 0; (str=buffer->chunk()); offset = strlen(remainder)) {
	/*
	 * mask a chunk
	 */
	if (offset != 0) {
	    if (offset + strlen(str) < 4) {
		remainder += str;
		continue;
	    }

	    masked->append(asn_xor(remainder + str[.. 3 - offset], mask));
	    offset = 4 - offset;
	}

# ifdef KF_MASK_XOR
	len = (strlen(str) & ~3) + offset;
	if (len > strlen(str)) {
	    len -= 4;
	}
	if (offset < len) {
	    masked->append(mask_xor(mask, str[offset .. len - 1]));
	    offset = len;
	}
# else
	/* mask all 4-byte pieces */
	for (len = strlen(str) - 4; offset <= len; offset += 4) {
	    masked->append(asn_xor(str[offset .. offset + 3], mask));
	}
# endif

	remainder = str[offset ..];
    }

    /* mask remainder */
    if (offset != 0) {
	masked->append(asn_xor(remainder, mask[.. offset - 1]));
    }

    return masked;
}

/*
 * prepare to send HTTP request
 */
static void sendRequest(HttpRequest request)
{
}

/*
 * prepare to send HTTP response
 */
static void sendResponse(HttpResponse response)
{
}

/*
 * send WebSocket frame
 */
static string sendWsFrame(int opcode, int flags, int length, string mask)
{
    string frame;

    if (opcode < WEBSOCK_CONTINUATION ||
	(opcode > WEBSOCK_BINARY &&
	 (opcode < WEBSOCK_CLOSE || opcode > WEBSOCK_PONG))) {
	error("Invalid WebSocket opcode");
    }
    if ((flags & WEBSOCK_FLAGS) != flags) {
	error("Invalid WebSocket flags");
    }

    frame = "..";
    frame[0] = flags | opcode;
    frame[1] = (!!mask) << 7;

    if (length < 126) {
	/* one-byte length */
	frame[1] |= length;
    } else if (length <= 0xffff) {
	/* two-byte length */
	frame[1] |= 126;
	frame += "..";
	frame[2] = length >> 8;
	frame[3] = length;
    } else {
	/* eight-byte length */
	frame[1] |= 127;
	frame += "........";
	frame[2] = length >> 56;
	frame[3] = length >> 48;
	frame[4] = length >> 40;
	frame[5] = length >> 32;
	frame[6] = length >> 24;
	frame[7] = length >> 16;
	frame[8] = length >> 8;
	frame[9] = length;
    }

    if (mask) {
	frame += mask;
    }

    return frame;
}
