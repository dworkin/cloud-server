# define Body		"/lib/base/body"


private object *bodies;	/* body objects for this soul */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize soul
 */
static void create()
{
    if (!bodies) {
	bodies = ({ });
    }
}

/*
 * NAME:	add_body()
 * DESCRIPTION:	add a new body controlled by this soul
 */
static atomic void add_body(object Body body)
{
    if (body) {
	body->_F_set_soul(this_object());
	bodies += ({ body });
    }
}

/*
 * NAME:	remove_body()
 * DESCRIPTION:	stop controlling a body
 */
static atomic void remove_body(object Body body)
{
    if (sizeof(bodies & ({ body })) == 0) {
	error("bad body");
    }
    body->_F_remove_soul(this_object());
    bodies -= ({ body });
}

/*
 * NAME:	query_bodies()
 * DESCRIPTION:	return list of bodies
 */
static object *query_bodies()
{
    return bodies[..];
}

/*
 * NAME:	command()
 * DESCRIPTION:	let body handle command
 */
static mixed command(object Body body, string command, mixed args...)
{
    return body->_F_command(command, args);
}
