# include <config.h>

# define DRIVER		"/kernel/sys/driver"
# define AUTO		"/kernel/lib/auto"

/*
 * privilege levels
 */
# define KERNEL()	sscanf(previous_program(), "/kernel/%*s")
# define SYSTEM()	sscanf(previous_program(), USR_DIR + "/System/%*s")

/*
 * kernel TLS
 */
# define TLS_LIMIT	0	/* resource limits */
# define TLS_ARGUMENT	1	/* argument, result or error */
# define TLS_INHERIT	2	/* inherited objects */
# define TLS_USER	3	/* current user */
# define TLS_RESOURCE	4	/* resources incremented */
# define TLS_PUT_ATOMIC	5	/* send message through atomic barrier */
# define TLS_GET_ATOMIC	6	/* retrieve message through atomic barrier */
