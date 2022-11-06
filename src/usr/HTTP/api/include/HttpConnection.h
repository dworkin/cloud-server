# define HttpConnection		object "/usr/HTTP/lib/Connection"
# define Http1Connection	object "/usr/HTTP/lib/Connection1"

# define HTTP1_SERVER		"/usr/HTTP/api/obj/server1"
# define HTTP1_CLIENT		"/usr/HTTP/api/obj/client1"

# define WEBSOCK_OPCODE		0x0f	/* opcode */
# define WEBSOCK_CONTINUATION	0x00
# define WEBSOCK_TEXT		0x01
# define WEBSOCK_BINARY		0x02
# define WEBSOCK_CLOSE		0x08
# define WEBSOCK_PING		0x09
# define WEBSOCK_PONG		0x0a

# define WEBSOCK_FLAGS		0xf0	/* flags */
# define WEBSOCK_FIN		0x80
# define WEBSOCK_RSV1		0x40
# define WEBSOCK_RSV2		0x20
# define WEBSOCK_RSV3		0x10
