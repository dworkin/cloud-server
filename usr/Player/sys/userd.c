# include <kernel/kernel.h>

private inherit "/lib/util/string";


object telnetd;		/* telnet daemon */
object binaryd;		/* binary daemon */
mapping users;		/* 2-step mapping for user objects */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize this object
 */
static void create()
{
    telnetd = find_object("/usr/System/sys/telnetd");
    binaryd = find_object("/usr/System/sys/binaryd");
    users = ([ ]);
}

/*
 * NAME:	find_user()
 * DESCRIPTION:	find a user object
 */
object find_user(string name)
{
    mapping map;

    name = lower_case(name);
    map = users[name[0 .. 1]];
    return (map) ? map[name] : nil;
}

/*
 * NAME:	get_user()
 * DESCRIPTION:	return the proper user object, creating a new one if required
 */
object get_user(string name)
{
    object obj;
    mapping map;

    obj = previous_object();
    if (obj == telnetd || obj == binaryd) {
	if (!parse_string("word = /[A-Za-z][A-Za-z]+/ junk = /./ name: word",
			  name)) {
	    /* bad name */
	    return nil;
	}

	name = lower_case(name);
	map = users[name[0 .. 1]];
	if (map) {
	    obj = map[name];
	    if (!obj) {
		obj = map[name] = clone_object("~/obj/user");
	    }
	} else {
	    obj = clone_object("~/obj/user");
	    users[name[0 .. 1]] = ([ name : obj ]);
	}
	return obj;
    }
}
