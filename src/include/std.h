# include <AUTO>

# define __PathOfobject		/**/
# define __PATHOFOBJECT(o)	__PathOf##o
# define OBJECT_PATH(o)		__PATHOFOBJECT(o)

# define TRUE			1
# define FALSE			0
