/* GET http://foo.bar.com/index.html HTTP/1.0 */
# define HTTPREQ_VERSION	0	/* 1.0 */
# define HTTPREQ_METHOD		1	/* GET */
# define HTTPREQ_SCHEME		2	/* http */
# define HTTPREQ_HOST		3	/* foo.bar.com */
# define HTTPREQ_PATH		4	/* /index.html */

/* HTTP return codes */
# define HTTP_OK		200	/* successful */
# define HTTP_CREATED		201
# define HTTP_ACCEPTED		202
# define HTTP_NO_CONTENT	204
# define HTTP_MULTIPLE_CHOICES	300	/* redirection */
# define HTTP_MOVED_PERMANENTLY	301
# define HTTP_MOVED_TEMPORARILY	302
# define HTTP_NOT_MODIFIED	304
# define HTTP_BAD_REQUEST	400	/* client error */
# define HTTP_UNAUTHORIZED	401
# define HTTP_FORBIDDEN		403
# define HTTP_NOT_FOUND		404
# define HTTP_INTERNAL_ERROR	500	/* server error */
# define HTTP_NOT_IMPLEMENTED	501
# define HTTP_BAD_GATEWAY	502
# define HTTP_UNAVAILABLE	503
