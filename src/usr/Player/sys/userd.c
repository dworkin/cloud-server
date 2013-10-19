# include <kernel/kernel.h>
# include <kernel/user.h>

inherit "~System/lib/user";

private inherit "/lib/util/string";

# define Interface	"/usr/Player/lib/interface"

object userd;		/* user daemon */
mapping users;		/* 2-step mapping for user objects */
mapping logins;		/* logged-in users */
string banner;		/* login message */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize this object
 */
static void create()
{
    userd = find_object(USERD);
    users = ([ ]);
    logins = ([ ]);
    banner = "\n" +
	     "Welcome to the Cloud Server.\n" +
	     "\n" +
	     "After login, the following commands are available:\n" +
	     " - @users               see who is logged on\n" +
	     " - say, ', emote, :     communication\n" +
	     " - @quit                leave the mud\n" +
	     "\n" +
	     "login: ";
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
 * NAME:	user_login()
 * DESCRIPTION:	login a user
 */
void user_login(object obj)
{
    if (previous_program() == Interface) {
	logins[obj] = 1;
    }
}

/*
 * NAME:	user_logout()
 * DESCRIPTION:	logout a user
 */
void user_logout(object obj)
{
    if(previous_program() == Interface) {
	logins[obj] = nil;
    }
}

/*
 * NAME:	query_logins()
 * DESCRIPTION:	return array of logged-in users
 */
object *query_logins()
{
    return map_indices(logins);
}

/*
 * NAME:	get_user()
 * DESCRIPTION:	return the proper user object, creating a new one if required
 */
static object get_user(string name)
{
    object obj;
    mapping map;

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

/*
 * NAME:	select()
 * DESCRIPTION:	select user object based on loginname
 */
object select(string name)
{
    if (previous_object() == userd) {
	object obj;

	obj = get_user(name);
	return (obj) ? obj : this_object();
    }
}

/*
 * NAME:	query_timeout()
 * DESCRIPTION:	return the login timeout
 */
int query_timeout(object obj)
{
    return DEFAULT_TIMEOUT;
}

/*
 * NAME:	set_banner()
 * DESCRIPTION:	set the login banner
 */
void set_banner(string str)
{
    if (SYSTEM()) {
	banner = str;
    }
}

/*
 * NAME:	query_banner()
 * DESCRIPTION:	return the login banner
 */
string query_banner(object obj)
{
    return banner;
}

/*
 * NAME:	login()
 * DESCRIPTION:	display an errormessage and disconnect
 */
int login(string str)
{
    previous_object()->message("\"" + str + "\" is not a valid name.\n\n");
    return MODE_DISCONNECT;
}
