# include <type.h>


# define Container	"/lib/base/container"

private object environment;	/* environment of this object */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize state of this object
 */
static void create() { }

/*
 * NAME:	move()
 * DESCRIPTION:	move this object to a new destination, which may be nil.
 *		The "position" can be any integer or floating-point value.
 */
atomic void move(object Container dest, varargs mixed position)
{
    switch (typeof(position)) {
    case T_INT:
    case T_FLOAT:
    case T_STRING:
	break;

    default:
	if (position == nil) {
	    position = this_object();	/* placeholder value */
	} else {
	    error("bad position");
	}
    }

    /* actually move the object */
    if (environment) {
	environment->_F_moveout(this_object());
    }
    if (dest) {
	dest->_F_movein(this_object(), position);
    }
    environment = dest;
}

/*
 * NAME:	query_env()
 * DESCRIPTION:	return the environment of this object
 */
object query_env()
{
    return environment;
}
