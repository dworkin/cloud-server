# include <messages.h>

inherit "~/lib/body";

private inherit "/lib/util/string";


static int act_say(string str)
{
    query_env()->message(MSG_FORMATTED,
			 indent_string(capitalize(query_actor_desc()) +
				       " says: ", str),
			 this_object());
    message(MSG_FORMATTED, indent_string("You say: ", str));

    return TRUE;
}

static int act_go(string str)
{
    switch (str) {
    case "north":
    case "northeast":
    case "east":
    case "southeast":
    case "south":
    case "southwest":
    case "west":
    case "northwest":
    case "up":
    case "down":
	if (query_env()->query_exit(str)) {
	    if (query_env()->move_body(this_object(), str)) {
		return TRUE;
	    }
	} else {
	    message(MSG_FORMATTED, "You can't go that way.\n");
	}
	break;
    }

    return FALSE;
}

static int act_get(object obj)
{
    string str;

    if (obj->can_get(this_object())) {
	if (obj->query_env() == this_object()) {
	    message(MSG_FORMATTED, "You already have that.\n");
	} else {
	    /* do_get */
	    obj->move(this_object());
	    str = obj->query_short_desc();
	    message(MSG_UNFORMATTED, "You take the " + str + ".");
	    query_env()->message(MSG_UNFORMATTED,
				 query_actor_desc() + " takes a " + str + ".",
				 this_object());
	}
    } else {
	message(MSG_FORMATTED, "You can't take that.\n");
    }

    return TRUE;
}

static int act_drop(object obj)
{
    string str;

    if (obj->can_drop(this_object())) {
	obj->move(query_env());
	str = obj->query_short_desc();
	message(MSG_UNFORMATTED, "You drop the " + str + ".");
	query_env()->message(MSG_UNFORMATTED,
			     query_actor_desc() + " drops a " + str + ".",
			     this_object());
    } else {
	message(MSG_FORMATTED, "You can't drop that.\n");
    }

    return TRUE;
}

static int act_pull(object obj)
{
    if (obj->can_pull(this_object())) {
	obj->do_pull(this_object());
    } else {
	message(MSG_FORMATTED, "You can't pull that.\n");
    }

    return TRUE;
}
