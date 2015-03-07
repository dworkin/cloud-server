# include <kernel/kernel.h>
# include <messages.h>
# include <type.h>

inherit container	"base/container";
inherit desc		"base/description";
inherit movable		"base/movable";


private string background_desc;	/* background description */
private mapping neighbours;	/* neighbour rooms */
private mapping forward;	/* neighbours to forward to */
private mapping exits;		/* exits to neighbours */
private mapping exitposns;	/* destination position */

# define N_VISIBILITY	0	/* visibility of things in neighbour room */
# define N_FORWARD	1	/* forwarding of messages to neighbour room */
# define N_BACKGROUND	2	/* background message for neighbour */
# define N_CONTEXT	3	/* context string for neighbour */


/*
 * NAME:	create()
 * DESCRIPTION:	initialize room
 */
static void create()
{
    if (!neighbours) {
	container::create();
	desc::create();
	movable::create();

	neighbours = ([ ]);
	forward = ([ ]);
	exits = ([ ]);
	exitposns = ([ ]);
    }
}

/*
 * NAME:	set_background_desc()
 * DESCRIPTION:	set the background string for contained rooms
 */
void set_background_desc(string str)
{
    background_desc = str;
}

/*
 * NAME:	query_background_desc()
 * DESCRIPTION:	return the background for contained rooms
 */
string query_background_desc()
{
    return background_desc;
}


/*
 * NAME:	add_neighbour()
 * DESCRIPTION:	add a neighbour room to this room
 *		    direction:
 *			nil: no exit
 *			other: normal exit
 *		    position:
 *			position in destination room (only if direction non-nil)
 *		    visibility:
 *			visibility of neighbour room from this room
 *		    forwarding:
 *			forwarding of messages to neighbour room
 *		    background:
 *			background desc for neighbour room
 *		    context:
 *			context string for neighbour room
 */
void add_neighbour(mixed room,
		   varargs string direction, mixed position,
			   int visibility, int forwarding,
			   string background, string context)
{
    mixed fwd;

    if (typeof(room) == T_STRING) {
	object driver;
	string oname;

	driver = find_object(DRIVER);
	oname = object_name(this_object());
	room = driver->normalize_path(room, oname + "/..",
				      driver->creator(oname));
	if (!neighbours[room]) {
	    object obj;

	    obj = find_object(room);
	    if (obj) {
		room = obj;
	    }
	}
    } else if (typeof(room) != T_OBJECT) {
	error("bad neighbour");
    }
    if (typeof(position) > T_STRING) {
	error("bad position");
    }

    if (direction) {
	exits[direction] = room;
	exitposns[direction] = position;
    }
    fwd = (forwarding) ? TRUE : nil;
    neighbours[room] = ({ visibility, fwd, background, context });
    forward[room] = fwd;
}

/*
 * NAME:	remove_neighbour()
 * DESCRIPTION:	remove a room as a neighbour, and exits leading to it
 */
void remove_neighbour(mixed room)
{
    if (typeof(room) == T_STRING) {
	object driver;
	string oname;

	driver = find_object(DRIVER);
	oname = object_name(this_object());
	room = driver->normalize_path(room, oname + "/..",
				      driver->creator(oname));
	if (!neighbours[room]) {
	    object obj;

	    obj = find_object(room);
	    if (obj) {
		room = obj;
	    }
	}
    } else if (typeof(room) != T_OBJECT) {
	error("bad neighbour");
    }

    if (neighbours[room]) {
	string *directions;
	mixed *rooms;
	int i;

	forward[room] = neighbours[room] = nil;

	/*
	 * remove exits to neighbour
	 */
	directions = map_indices(exits);
	rooms = map_values(exits);
	for (i = sizeof(rooms); --i >= 0; ) {
	    if (rooms[i] == room) {
		exits[directions[i]] = nil;
	    }
	}
    }
}

/*
 * NAME:	query_neighbours()
 * DESCRIPTION:	return an array with neighbour rooms
 */
object *query_neighbours()
{
    mixed *rooms;
    int i;
    string str;
    object obj;

    rooms = map_indices(neighbours);
    for (i = sizeof(rooms); --i >= 0; ) {
	if (typeof(rooms[i]) == T_STRING) {
	    str = rooms[i];
	    obj = find_object(str);
	    if (obj) {
		rooms[i] = obj;
		forward[obj] = (neighbours[obj] = neighbours[str])[N_FORWARD];
		forward[str] = neighbours[str] = nil;
	    } else {
		rooms[i] = nil;
	    }
	}
    }

    return (str) ? rooms - ({ nil }) : rooms;
}

/*
 * NAME:	neighbour()
 * DESCRIPTION:	deal with a neighbour room as a string or object
 */
private mixed neighbour(mixed room)
{
    if (neighbours[room]) {
	return room;
    } else if (typeof(room) == T_STRING) {
	object driver;
	string oname;

	driver = find_object(DRIVER);
	oname = object_name(this_object());
	room = driver->normalize_path(room, oname + "/..",
				      driver->creator(oname));
	if (neighbours[room]) {
	    return room;
	}
	room = find_object(room);
	if (neighbours[room]) {
	    return room;
	}
    } else if (typeof(room) == T_OBJECT) {
	string str;

	if (neighbours[str = object_name(room)]) {
	    forward[room] = (neighbours[room] = neighbours[str])[N_FORWARD];
	    forward[str] = neighbours[str] = nil;
	    return room;
	}
    }

    error("bad neighbour");
}

/*
 * NAME:	find_neighbour()
 * DESCRIPTION:	find a neighbour room object, and update internal datastructs
 */
private object find_neighbour(string str)
{
    object obj;

    obj = find_object(str);
    if (obj && !neighbours[obj]) {
	forward[obj] = (neighbours[obj] = neighbours[str])[N_FORWARD];
	forward[str] = neighbours[str] = nil;
    }
    return obj;
}


/*
 * NAME:	add_exit()
 * DESCRIPTION:	add an exit to a neighbour room, or to this room itself
 */
void add_exit(mixed room, string direction, varargs mixed position)
{
    if (room == this_object()) {
	exits[direction] = room;
    } else {
	exits[direction] = neighbour(room);
    }
    exitposns[direction] = position;
}

/*
 * NAME:	remove_exit()
 * DESCRIPTION:	remove an exit from the room
 */
void remove_exit(string direction)
{
    exits[direction] = nil;
    exitposns[direction] = nil;
}

/*
 * NAME:	query_exits()
 * DESCRIPTION:	return the room's exits
 */
string *query_exits()
{
    return map_indices(exits);
}

/*
 * NAME:	query_exit()
 * DESCRIPTION:	return an exit in a specific direction
 */
object query_exit(string direction)
{
    mixed room;

    room = exits[direction];
    if (typeof(room) == T_STRING) {
	room = find_neighbour(room);
	if (room) {
	    exits[direction] = room;
	}
    }
    return room;
}

/*
 * NAME:	query_exitposn()
 * DESCRIPTION:	return the position in the destination for an exit
 */
mixed query_exitposn(string direction)
{
    return exitposns[direction];
}


/*
 * NAME:	set_neighbour_visibility()
 * DESCRIPTION:	set the visibility of a neighbour room from this room
 */
void set_neighbour_visibility(mixed room, int visibility)
{
    neighbours[neighbour(room)][N_VISIBILITY] = visibility;
}

/*
 * NAME:	set_neighbour_forwarding()
 * DESCRIPTION:	set forwarding of messages to a neighbour room
 */
void set_neighbour_forwarding(mixed room, int forwarding)
{
    mixed forward;

    room = neighbour(room);
    forward = (forwarding) ? TRUE : nil;
    forward[room] = neighbours[room][N_FORWARD] = forward;
}

/*
 * NAME:	set_neighbour_background_desc()
 * DESCRIPTION:	set the background description of a neighbour room
 *		(overrides default)
 */
void set_neighbour_background_desc(mixed room, string background)
{
    neighbours[neighbour(room)][N_BACKGROUND] = background;
}

/*
 * NAME:	set_neighbour_context_desc()
 * DESCRIPTION:	set the context string of a neighbour room
 */
void set_neighbour_context_desc(mixed room, string context)
{
    neighbours[neighbour(room)][N_CONTEXT] = context;
}

/*
 * NAME:	query_visibility()
 * DESCRIPTION:	return the visibility of a neighbour from this room
 */
int query_neighbour_visibility(mixed room)
{
    return neighbours[neighbour(room)][N_VISIBILITY];
}

/*
 * NAME:	query_neighbour_forwarding()
 * DESCRIPTION:	return the message forwarding status to a neighbour room
 */
int query_neighbour_forwarding(mixed room)
{
    mixed forward;

    forward = forward[neighbour(room)];
    return (forward) ? TRUE : FALSE;
}

/*
 * NAME:	query_neighbour_background_desc()
 * DESCRIPTION:	return the background set for a neighbour room
 */
string query_neighbour_background_desc(mixed room)
{
    return neighbours[neighbour(room)][N_BACKGROUND];
}

/*
 * NAME:	query_neighbour_context_desc()
 * DESCRIPTION:	return the context set for a neighbour room
 */
string query_neighbour_context_desc(mixed room)
{
    return neighbours[neighbour(room)][N_CONTEXT];
}


/*
 * NAME:	message()
 * DESCRIPTION:	pass on a message to all objects in a room, except those
 *		that should be skipped
 */
void message(int type, string mesg, object skip...)
{
    mixed *inv, obj;
    int i, sz;

    if (type & MSG_FORWARD) {
	type &= ~MSG_FORWARD;

	/*
	 * forward to environment
	 */
	obj = query_env();
	if (obj) {
	    obj->message(type, mesg, this_object(), skip...);
	}

	/*
	 * forward to inventory
	 */
	inv = query_inv() - skip;
	for (i = 0, sz = sizeof(inv); i < sz; i++) {
	    if (inv[i]) {
		inv[i]->message(type, mesg, skip...);
	    }
	}

	if (map_sizeof(forward) != 0) {
	    /*
	     * forward to neighbours
	     */
	    inv = map_indices(forward) - skip;
	    for (i = 0, sz = sizeof(inv); i < sz; i++) {
		obj = inv[i];
		if (typeof(obj) == T_STRING) {
		    obj = find_neighbour(obj);
		}
		if (obj) {
		    obj = obj->query_room(this_object());
		    if (type & MSG_FORMATTED) {
			obj->message(type, mesg, skip...);
		    } else {
			obj->message(type,
				     neighbours[obj][N_CONTEXT] + ", " + mesg,
				     skip...);
		    }
		}
	    }
	}
    } else {
	/*
	 * forward to non-rooms in inventory
	 */
	inv = query_inv() - skip;
	for (i = 0, sz = sizeof(inv); i < sz; i++) {
	    if (inv[i]) {
		inv[i]->message(type, mesg, skip...);
	    }
	}
    }
}

/*
 * NAME:	direct_message()
 * DESCRIPTION:	pass on a message to a specific group of targets
 */
void direct_message(int type, string mesg, object target...)
{
    int i, sz;

    for (i = 0, sz = sizeof(target); i < sz; i++) {
	if (target[i]) {
	    target[i]->message(type, mesg);
	}
    }
}

/*
 * NAME:	query_room()
 * DESCRIPTION:	return the "entrance room" for this room; normally the
 *		room itself
 */
object query_room(object room, varargs object body)
{
    return this_object();
}

/*
 * NAME:	move_body()
 * DESCRIPTION:	move a body from this room to another; return 1 if successful
 */
int move_body(object body, string direction)
{
    mixed dest;
    string str;

    dest = exits[direction];
    if (dest && (dest=dest->query_room(this_object(), body))) {
	str = body->query_short_desc();
	message(MSG_UNFORMATTED, str + " leaves " + direction + ".", body);
	body->move(dest, exitposns[direction]);
	dest->message(MSG_UNFORMATTED, str + " arrives.", body);
	return 1;
    }
    return 0;
}
