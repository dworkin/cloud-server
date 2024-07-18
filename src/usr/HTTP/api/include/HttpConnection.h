# define Http1Server		object "/usr/HTTP/api/lib/Server1"
# define Http1Client		object "/usr/HTTP/api/lib/Client1"
# define Http1TlsServer		object "/usr/HTTP/api/lib/TlsServer1"
# define Http1TlsClient		object "/usr/HTTP/api/lib/TlsClient1"

# define HTTP1_SERVER		"/usr/HTTP/api/obj/server1"
# define HTTP1_CLIENT		"/usr/HTTP/api/obj/client1"
# define HTTP1_TLS_SERVER	"/usr/HTTP/api/obj/tls_server1"
# define HTTP1_TLS_CLIENT	"/usr/HTTP/api/obj/tls_client1"

# define WEBSOCK_OPCODE		0x0f		/* opcode */
# define WEBSOCK_CONTINUATION	0x00
# define WEBSOCK_TEXT		0x01
# define WEBSOCK_BINARY		0x02
# define WEBSOCK_CLOSE		0x08
# define WEBSOCK_PING		0x09
# define WEBSOCK_PONG		0x0a

# define WEBSOCK_FLAGS		0xf0		/* flags */
# define WEBSOCK_FIN		0x80
# define WEBSOCK_RSV1		0x40
# define WEBSOCK_RSV2		0x20
# define WEBSOCK_RSV3		0x10

# define WEBSOCK_STATUS_1000	"\x03\xe8"	/* normal */
# define WEBSOCK_STATUS_1001	"\x03\xe9"	/* going away */
# define WEBSOCK_STATUS_1002	"\x03\xea"	/* protocol error */
# define WEBSOCK_STATUS_1003	"\x03\xeb"	/* invalid input */
# define WEBSOCK_STATUS_1005	"\x03\xed"	/* missing status code */
# define WEBSOCK_STATUS_1006	"\x03\xee"	/* closed abnormally */
# define WEBSOCK_STATUS_1007	"\x03\xef"	/* format error */
# define WEBSOCK_STATUS_1008	"\x03\xf0"	/* policy violation */
# define WEBSOCK_STATUS_1009	"\x03\xf1"	/* message too big */
# define WEBSOCK_STATUS_1010	"\x03\xf2"	/* missing extension */
# define WEBSOCK_STATUS_1011	"\x03\xf3"	/* general failure */
# define WEBSOCK_STATUS_1015	"\x03\xf7"	/* TLS handshake failure */
