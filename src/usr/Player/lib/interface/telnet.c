# include <kernel/user.h>
# include <messages.h>
# include <cmdparser.h>
# include <type.h>

inherit interface "~System/lib/interface";
inherit user	  "~/lib/api/user";
inherit parser	  "/lib/cmdparser";

private inherit "/lib/util/string";
private inherit "/lib/util/language";

# define STATE_NORMAL		0
# define STATE_LOGIN		1
# define STATE_OLDPASSWD	2
# define STATE_NEWPASSWD1	3
# define STATE_NEWPASSWD2	4
# define STATE_PASTING		5

# define BaseBody		"/lib/base/body"


private string name;		/* user name */
private string Name;		/* capitalized user name */
private mapping state;		/* state for a connection object */
private string passwd;		/* encrypted password */
private string newpasswd;	/* new password */
private string paste_buffer;	/* buffer holding text being pasted */
private mapping aliases;	/* command aliases */
private int suspended;		/* suspended for long-duration command */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize interface
 */
static void create()
{
    interface::create();
    user::create();
    parser::create();
    state = ([ ]);
    aliases = ([
	"n"  : "go north",
	"ne" : "go northeast",
	"e"  : "go east",
	"se" : "go southeast",
	"s"  : "go south",
	"sw" : "go southwest",
	"w"  : "go west",
	"u"  : "go up",
	"d"  : "go down"
    ]);
}


/*
 * NAME:	login()
 * DESCRIPTION:	login a new user
 */
static int login(string str)
{
    object connection;

    connection = previous_object();
    name = lower_case(str);
    Name = capitalize(name);
    if (passwd) {
	/* check password */
	connection->message("Password:");
	state[connection] = STATE_LOGIN;
    } else {
	/* no password; login immediately */
	::login(connection, name);
	::message("Pick a new password:");
	state[connection] = STATE_NEWPASSWD1;
    }
    return MODE_NOECHO;
}

/*
 * NAME:	logout()
 * DESCRIPTION:	logout user
 */
static void logout(int quit)
{
    if (previous_object() == query_conn()) {
	object body;

	newpasswd = nil;
	body = query_body();
	if (quit) {
	    body->query_env()->message(MSG_FORWARD, Name + " logs out.", body);
	} else {
	    body->query_env()->message(MSG_FORWARD, Name + " disconnected.", body);
	}
	::logout();
    }
}

/*
 * NAME:	message()
 * DESCRIPTION:	handle a message
 */
static void message(int type, string str)
{
    if (type & MSG_FORMATTED) {
	::message(str);
    } else {
	::message(break_string(capitalize(str)));
    }
}


/*
 * NAME:	cmd_look()
 * DESCRIPTION:	describe a room to the user
 */
static int cmd_look(object body, mixed *cmd)
{
    object room, obj, *neighbours, *inv, *bodies;
    string view, str;
    int i, j, sz;
    mixed desc;

    /*
     * room description
     */
    room = body->query_act_env();
    view = room->query_long_desc();

    /*
     * object views, neighbour background descriptions, and environment
     * background description
     */
    /* inventory background descriptions */
    inv = body->query_act_objects();
    for (i = 0, sz = sizeof(inv); i < sz; i++) {
	obj = inv[i];
	str = obj->query_view_desc();
	if (str) {
	    view += " " + str;
	}
    }
    /* neighbour room background descriptions */
    neighbours = room->query_neighbours();
    for (i = 0, sz = sizeof(neighbours); i < sz; i++) {
	if (room->query_neighbour_visibility(neighbours[i]) != 0) {
	    str = neighbours[i]->query_neighbour_background_desc(room);
	    if (str) {
		view += " " + str;
	    }
	} else {
	    neighbours[i] = nil;
	}
    }
    neighbours -= ({ nil });
    /* environment background description */
    obj = room->query_env();
    if (obj) {
	str = obj->query_background_desc();
	if (str) {
	    view += " " + str;
	}
    }
    message(MSG_UNFORMATTED, view);

    /*
     * object inventory: separate into bodies and ordinary objects
     */
    inv -= ({ body });
    sz = sizeof(inv);
    bodies = allocate(sz);
    for (i = j = 0; i < sz; i++) {
	obj = inv[i];
	if (obj <- BaseBody) {
	    if (!obj->query_soul() || obj->query_soul()->query_conn()) {
		bodies[j++] = obj;
	    }
	    inv[i] = nil;
	}
    }
    inv -= ({ nil });
    bodies = bodies[.. j - 1];

    /* object inventory */
    desc = describe_objects(inv);
    if (typeof(desc) == T_STRING) {
	message(MSG_UNFORMATTED, "There is " + desc + " here.");
    } else {
	desc = summarize(desc);
	if (desc) {
	    message(MSG_UNFORMATTED, "There are " + desc + " here.");
	}
    }

    /* XXX add included and environment room bodies */
    /* body inventory */
    desc = describe_objects(bodies);
    if (typeof(desc) == T_STRING) {
	message(MSG_UNFORMATTED, desc + " is here.");
    } else {
	desc = summarize(desc);
	if (desc) {
	    message(MSG_UNFORMATTED, desc + " are here.");
	}
    }

    /* neighbour room body inventory */
    view = "";
    for (i = 0, sz = sizeof(neighbours); i < sz; i++) {
	inv = neighbours[i]->query_room(room, body)->query_inv();
	do {
	    str = neighbours[i]->query_neighbour_context_desc(room);
	    room = room->query_original();
	} while (room && !str);
	for (j = sizeof(inv); --j >= 0; ) {
	    obj = inv[j];
	    if (!(obj <- BaseBody) ||
		(obj->query_soul() && !obj->query_soul()->query_conn())) {
		inv[j] = nil;
	    }
	}
	desc = describe_objects(inv - ({ nil }));
	if (typeof(desc) == T_STRING) {
	    view += " " + desc + " stands " + str + ".";
	} else {
	    desc = summarize(desc);
	    if (desc) {
		view += " " + desc + " stand " + str + ".";
	    }
	}
    }
    if (strlen(view) != 0) {
	message(MSG_UNFORMATTED, view);
    }

    return TRUE;
}

/*
 * NAME:	cmd_examine()
 * DESCRIPTION:	look at something
 */
static int cmd_examine(object body, mixed *cmd)
{
    mixed obj;

    obj = match_objects(cmd[2], body->query_inv() + body->query_act_objects());
    if (obj) {
	message(MSG_UNFORMATTED, obj->query_long_desc());
    } else {
	obj = match_detail(cmd[2], ({ body, body->query_env() }));
	if (!obj) {
	    message(MSG_UNFORMATTED, "No " + cmd[2][0][THING_NOUN] + " here.");
	} else {
	    message(MSG_UNFORMATTED, obj[0]->query_detail_desc(obj[1]));
	}
    }

    return TRUE;
}

/*
 * NAME:	cmd_inventory()
 * DESCRIPTION:	describe the inventory
 */
static int cmd_inventory(object body, mixed *cmd)
{
    mixed desc;

    desc = describe_objects(body->query_inv());
    if (typeof(desc) != T_STRING) {
	desc = summarize(desc);
    }
    if (desc) {
	message(MSG_UNFORMATTED, "You are carrying " + desc + ".");
    } else {
	message(MSG_UNFORMATTED, "You are not carrying anything.");
    }

    return TRUE;
}

/*
 * NAME:	cmd_say()
 * DESCRIPTION:	say something
 */
static int cmd_say(object body, mixed *cmd)
{
    string str;

    str = cmd[2];
    if (strlen(str) == 0) {
	message(MSG_FORMATTED, "Say what?\n");
    } else {
	if (str[0] == '"' && sscanf(str, "\"%*s\"") == 0) {
	    /* poor Zellski */
	    str = str[1 ..];
	}
	command(body, "say", str);
    }

    return TRUE;
}

/*
 * NAME:	cmd_go()
 * DESCRIPTION:	go somewhere
 */
static int cmd_go(object body, mixed *cmd)
{
    if (command(body, "go", cmd[2][0][THING_NOUN])) {
	cmd_look(body, ({ }));
    }
    return TRUE;
}

static int cmd_get(object body, mixed *cmd)
{
    mixed obj;
    string str;

    obj = match_objects(cmd[2], ({ body }) + body->query_act_objects() +
				body->query_inv());
    if (obj) {
	command(body, "get", obj);
    } else {
	obj = match_detail(cmd[2], ({ body, body->query_act_env() }));
	if (!obj) {
	    message(MSG_UNFORMATTED, "No " + cmd[2][0][THING_NOUN] + " here.");
	} else {
	    message(MSG_FORMATTED, "You can't take that.\n");
	}
    }

    return TRUE;
}

static int cmd_drop(object body, mixed *cmd)
{
    mixed obj;
    string str;

    obj = match_objects(cmd[2], ({ body }) + body->query_inv());
    if (obj) {
	command(body, "drop", obj);
    } else {
	message(MSG_FORMATTED, "You are not carrying that.\n");
    }

    return TRUE;
}

static int cmd_pull(object body, mixed *cmd)
{
    mixed obj;

    obj = match_objects(cmd[2], ({ body }) + body->query_inv() +
				body->query_act_objects());
    if (obj) {
	command(body, "pull", obj);
    } else {
	obj = match_detail(cmd[2], ({ body, body->query_act_env() }));
	if (!obj) {
	    message(MSG_UNFORMATTED, "No " + cmd[2][0][THING_NOUN] + " here.");
	} else {
	    message(MSG_FORMATTED, "You can't pull that.\n");
	}
    }

    return TRUE;
}

/*
 * NAME:	is_word_char()
 * DESCRIPTION:	
 */
private int is_word_char(int i)
{
    switch(i) {
    case 'a' .. 'z':
    case 'A' .. 'Z':
    case '0' .. '9':
	return TRUE;
    }
    return FALSE;
}

/*
 * NAME:	try_an_emotion()
 * DESCRIPTION:	handle output for a potential emotion
 *		XXX Fix when parse_string() is done
 */
static int cmd_feeling(object body, string myverb, string youverb, string str)
{
    mixed      *targets;
    object     *users;
    mapping	names;
    string	end;
    string	line;
    int		i;
    object	target;
    int		start;

    if (!str) {
	body->query_env()->message(MSG_FORWARD, Name + " " + youverb + ".",
			     body);
	body->query_env()->direct_message(MSG_UNFORMATTED, "You " + myverb + ".",
				    body);
	return TRUE;
    }
    if (str != "" && is_word_char(str[strlen(str)-1])) {
	end = ".";
    } else {
	end = "";
    }

    users = body->query_env()->query_inv();
    names = ([ ]);
    for (i = sizeof(users); --i >= 0; ) {
	if (users[i] <- BaseBody) {
	    names[users[i]->query_name()] = users[i];
	}
    }
    targets = ({ });
    start = -1;
    i = 0;

    do {
	while (i < strlen(str) && !is_word_char(str[i])) {
	    i ++;
	}
	start = i;
	while (i < strlen(str) && is_word_char(str[i])) {
	    i ++;
	}
	target = names[lower_case(str[start .. i-1])];
	if (target) {
	    targets += ({ ({ target, start, i }) });
	    if (str[start] >= 'a' && str[start] <= 'z') {
		str[start] += ('A' - 'a');
	    }
	}
    } while (i < strlen(str));

    for (i = 0; i < sizeof(targets); i ++) {
	body->query_env()->direct_message(MSG_UNFORMATTED,
				    Name + " " + youverb + " " +
				    str[.. targets[i][1]-1] + "you" +
				    str[targets[i][2] ..] + end,
				    targets[i][0]);
	targets[i] = targets[i][0];
    }

    body->query_env()->message(MSG_FORWARD, Name + " " + youverb + " " + str + end,
			 body, targets...);
    body->query_env()->direct_message(MSG_UNFORMATTED,
				"You " + myverb + " " + str + end, body);

    return TRUE;
}

/*
 * NAME:	cmd_emote()
 * DESCRIPTION:	emote something
 */
static int cmd_emote(object body, mixed *cmd)
{
    string str;

    str = cmd[2];
    if (strlen(str) == 0) {
	message(MSG_FORMATTED, "Usage: emote <text>\n");
    } else {
	body->query_env()->message(MSG_UNFORMATTED,
				   body->query_actor_desc() + " " + str);
    }

    return TRUE;
}


/*
 * NAME:	receive_message()
 * DESCRIPTION:	process a message from the user
 */
static int receive_message(string str)
{
    object connection;
    string cmd;
    object body, wiztool, who, *users;
    int i, sz;

    connection = previous_object();
    body = query_body();
    wiztool = query_wiztool();

    switch (state[connection]) {
    case STATE_NORMAL:
	cmd = str;
	if (strlen(str) != 0 && str[0] == '!') {
	    cmd = cmd[1 ..];
	}

	if (!wiztool || !query_editor(wiztool) || cmd != str) {
	    if (aliases) {
		string verb, arg, subst;

		if (sscanf(cmd, "%s %s", verb, arg) != 0) {
		    arg = " " + arg;
		} else {
		    verb = cmd;
		    arg = "";
		}
		subst = aliases[verb];
		if (subst) {
		    cmd = str = subst + arg;
		}
	    }

	    /* check standard commands */
	    if (strlen(cmd) == 0) {
		break;
	    }
	    if (cmd[0] != '@') {
		mixed *parsed;

		parsed = parse(cmd);
		if (parsed) {
		    parsed = parsed[0];
		    if (call_other(this_object(), "cmd_" + parsed[1], body,
				   parsed)) {
			break;
		    }
		} else {
		    string verbs;

		    str = nil;
		    sscanf(cmd, "%s %s", cmd, str);
		    if ((verbs="/sys/sould"->query_verb(cmd))) {
			cmd_feeling(body, cmd, verbs, str);
			break;
		    }
		}
		::message("What?\n");
		break;
	    }

	    sscanf(cmd, "%s ", cmd);
	    switch(cmd) {
	    case "@paste":
	       ::message("End your pasting with a single period.\n\"\b");
	       state[connection] = STATE_PASTING;
	       paste_buffer = "";
	       return MODE_ECHO;

	    case "@tell":
		if (sscanf(str, "%*s %s %s", cmd, str) != 3) {
		    ::message("Usage: tell <user> <text>\n");
		} else if (!(who=user::find_user(cmd))) {
		    ::message("No such user.\n");
		} else if (!who->query_conn()) {
		    ::message("That user is not logged on.\n");
		} else {
		    who->message(MSG_FORMATTED,
				 indent_string(Name + " tells you: ", str));
		    ::message(indent_string("You tell " +
						 capitalize(cmd) + ": ",
						 str));
		}
		str = nil;
		break;

	    case "@users":
		users = users();
		str = "Logged on:";
		for (i = 0, sz = sizeof(users); i < sz; i++) {
		    cmd = users[i]->query_name();
		    if (cmd) {
			str += " " + cmd;
		    }
		}
		::message(str + "\n");
		str = nil;
		break;

	    case "@alias":
		if (!aliases) {
		    aliases = ([ ]);
		}
		if (sscanf(str, "%*s %s", cmd) != 0) {
		    if (sscanf(cmd, "%s %s", cmd, str) != 0) {
			if (cmd == "@alias") {
			    ::message("You can't alias @alias.\n");
			} else {
			    aliases[cmd] = str;
			}
		    } else {
			aliases[cmd] = nil;
			::message("Alias cleared.\n");
		    }
		} else {
		    string *verbs, *substs;

		    verbs = map_indices(aliases);
		    substs = map_values(aliases);
		    for (i = 0, sz = sizeof(verbs); i < sz; i++) {
			::message(verbs[i] + ":\t" + substs[i] + "\n");
		    }
		}
		str = nil;
		break;

	    case "@password":
		if (passwd) {
		    ::message("Old password:");
		    state[connection] = STATE_OLDPASSWD;
		} else {
		    ::message("New password:");
		    state[connection] = STATE_NEWPASSWD1;
		}
		return MODE_NOECHO;

	    case "@quit":
		return MODE_DISCONNECT;
	    }
	}

	if (str) {
	    if (wiztool) {
		command(wiztool, str);
	    } else if (strlen(str) != 0) {
		::message("No command: " + str + "\n");
	    }
	}
	break;

    case STATE_LOGIN:
	if (crypt(str, passwd) != passwd) {
	    connection->message("\nBad password.\n");
	    return MODE_DISCONNECT;
	}
	::login(connection, name);
	::message("\n\n");
	if (!body) {
	    add_body(body = clone_object("/obj/body/human"));
	    set_current_body(body);
	    body->set_name(name);
	    body->move(find_object("/usr/Pattern/room/entrance"));
	}
	body->query_env()->message(MSG_FORWARD, Name + " logs in.", body);
	cmd_look(body, ({ }));
	break;

    case STATE_OLDPASSWD:
	if (crypt(str, passwd) != passwd) {
	    ::message("\nBad password.\n");
	    break;
	}
	::message("\nNew password:");
	state[connection] = STATE_NEWPASSWD1;
	return MODE_NOECHO;

    case STATE_NEWPASSWD1:
	newpasswd = str;
	::message("\nRetype new password:");
	state[connection] = STATE_NEWPASSWD2;
	return MODE_NOECHO;

    case STATE_NEWPASSWD2:
	if (newpasswd == str) {
	    passwd = crypt(str);
	    ::message("\nPassword changed.\n");
	} else {
	    ::message("\nMismatch; password not changed.\n");
	}
	newpasswd = nil;
	if (!body) {
	    add_body(body = clone_object("/obj/body/human"));
	    set_current_body(body);
	    body->set_name(name);
	    body->move(find_object("/usr/Pattern/room/entrance"));

	    body->query_env()->message(MSG_FORWARD, Name + " logs in.", body);
	    ::message("\n");
	    cmd_look(body, ({ }));
	}
	break;

    case STATE_PASTING:
       if (str != ".") {
	  paste_buffer += str + "\n";
	  ::message("\"\b");
	  return MODE_ECHO;
       }
       if (strlen(paste_buffer)) {
	   body->query_env()->message(MSG_FORMATTED,
				      "--- Pasted segment from " + Name +
				      " ---\n" + paste_buffer +
				      "--- Pasted segment ends ---\n",
				      body);
	   state[connection] = STATE_NORMAL;
	   paste_buffer = nil;
       }
       break;
    }

    if (!suspended) {
	str = (wiztool) ? query_editor(wiztool) : nil;
	if (str) {
	    ::message((str == "insert") ? "*\b" : ":");
	} else {
	    ::message("> ");
	}
    }
    state[connection] = STATE_NORMAL;
    return MODE_ECHO;
}

/*
 * NAME:	suspend()
 * DESCRIPTION:	suspend processing of input
 */
static void suspend()
{
    suspended = TRUE;
}

/*
 * NAME:	release()
 * DESCRIPTION:	release input suspension
 */
static void release()
{
    object wiztool;
    string str;

    suspended = FALSE;
    wiztool = query_wiztool();
    str = (wiztool) ? query_editor(wiztool) : nil;
    if (str) {
	::message((str == "insert") ? "*\b" : ":");
    } else {
	::message("> ");
    }
}
