# include <kernel/access.h>
# include <kernel/kernel.h>
# include <kernel/user.h>
# include <objectd.h>
# include <upgraded.h>
# include <messages.h>
# include <status.h>
# include <type.h>

inherit auto	"~/lib/auto";
inherit wiztool LIB_WIZTOOL;
inherit body	"/lib/base/body";

private inherit	"/lib/util/string";

# define TELNETD	"/usr/System/sys/telnetd"
# define PLAYERD	"/usr/Player/sys/userd"


object user;		/* associated user object */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize object
 */
static create(int clone)
{
    if (clone) {
	wiztool::create(200);
	body::create();
	user = PLAYERD->find_user(query_owner());
    }
}

/*
 * NAME:	message()
 * DESCRIPTION:	pass on a message to the user
 */
static void message(string str)
{
    user->message(MSG_FORMATTED, str);
}

/*
 * NAME:	destruct_object()
 * DESCRIPTION:	destruct_object wrapper (almost literally the same as in
 *		/kernel/lib/wiztool.c, except for the last line)
 */
static int destruct_object(mixed obj)
{
    string path, owner, oowner;
    object driver;

    owner = query_owner();
    switch (typeof(obj)) {
    case T_STRING:
	driver = find_object(DRIVER);
	path = obj = driver->normalize_path(obj, query_directory(), owner);
	if (sscanf(path, "%*s/lib/") != 0) {
	    oowner = driver->creator(path);
	} else {
	    obj = find_object(path);
	    if (!obj) {
		return FALSE;
	    }
	    oowner = obj->query_owner();
	}
	break;

    case T_OBJECT:
	path = object_name(obj);
	oowner = obj->query_owner();
	break;
    }

    if (path && owner != oowner &&
	((sscanf(path, "/kernel/%*s") != 0 && sscanf(path, "%*s/lib/") == 0) ||
	 !access(owner, path, WRITE_ACCESS))) {
	message(path + ": Permission denied.\n");
	return -1;
    }
    return auto::destruct_object(obj);
}

/*
 * NAME:	new_object()
 * DESCRIPTION:	new_object wrapper
 */
static object new_object(string path)
{
    string owner;

    owner = query_owner();
    path = DRIVER->normalize_path(path, query_directory(), owner);
    if (sscanf(path, "/kernel/%*s") != 0 || !access(owner, path, READ_ACCESS)) {
	message(path + ": Permission denied.\n");
	return nil;
    }
    return auto::new_object(path);
}


/*
 * NAME:	cmd_issues()
 * DESCRIPTION:	list all object and include issues of a certain file
 */
static void cmd_issues(object user, string cmd, string str)
{
    mixed *files, *issues;
    object objectd;
    int i, j, sz, len;
    string file;

    if (!str) {
	message("Usage: " + cmd + " <file> [<file> ...]\n");
	return;
    }
    files = expand(str, -1, TRUE)[0]; /* May not exist, full filenames */
    objectd = find_object(OBJECTD);

    for (i = 0, sz = sizeof(files); i < sz; i++) {
	file = files[i];
	str = "No issues.";
	len = strlen(file);
	if (len >= 2 && file[len - 2 ..] == ".c") {
	    issues = objectd->query_issues(file[.. len - 3]);
	    j = sizeof(issues);
	    if (j != 0) {
		do {
		    --j;
		    issues[j] = (string) issues[j];
		} while (j != 0);
		str = implode(issues, ", ");
	    }
	}
	message(indent_string(file + ": ", str));
    }
}

/*
 * NAME:	cmd_upgrade()
 * DESCRIPTION:	upgrade an object and all that depends on it
 */
static void cmd_upgrade(object user, string cmd, string str)
{
    string *names;
    int i, sz, len;
    mapping failed;

    if (!str) {
	message("Usage: " + cmd + " <file> [<file> ...]\n");
	return;
    }

    names = expand(str, 1, TRUE)[0];
    for (i = 0, sz = sizeof(names); i < sz; i++) {
	str = names[i];
	len = strlen(str);
	if (len < 2 || str[len - 2 ..] != ".c") {
	    message(str + ": No such source file.\n");
	    names[i] = nil;
	} else {
	    str = str[.. len - 3];
	    names[i] = str;
	    if (!status(str)) {
		message(str + ": No such object.\n");
		names[i] = nil;
	    }
	}
    }

    names -= ({ nil });
    if (sizeof(names) != 0) {
	failed = ([ ]);
	str = UPGRADED->upgrade(query_owner(), names, failed);
	if (str) {
	    message(str);
	} else {
	    names -= ({ nil });
	    if (sizeof(names) != 0) {
		message("Objects successfully upgraded:\n" +
			break_string("<" + implode(names, ">, <") + ">", 0, 2));
	    }

	    i = map_sizeof(failed);
	    if (i != 0) {
		string **values;

		str = "";
		values = map_values(failed);
		do {
		    str += ".c, " + implode(values[--i], ".c, ");
		} while (i != 0);
		message("Errors occured compiling:\n" +
			break_string(str[4 ..] + ".c", 0, 2));
	    }
	}
    }
}


/*
 * NAME:	do_patch()
 * DESCRIPTION:	patch an object with this owner's resource limits
 */
void do_patch(object obj)
{
    if (previous_program() == UPGRADED) {
	call_limited("call_patch", obj);
    }
}

/*
 * NAME:	call_patch()
 * DESCRIPTION:	patch an object
 */
static void call_patch(object obj)
{
    obj->_F_patch();
}


/*
 * NAME:	list_access()
 * DESCRIPTION:	return an access listing in string form
 */
private string list_access(mapping access)
{
    string str, *files;
    int i, *values;

    files = map_indices(access);
    values = map_values(access);
    for (i = sizeof(files); --i >= 0; ) {
	switch (values[i]) {
	case READ_ACCESS:
	    files[i] += " [read-only]";
	    break;

	case FULL_ACCESS:
	    files[i] += " [full]";
	    break;
	}
    }

    return " " + implode(files, "\n ") + "\n";
}

/*
 * NAME:	access()
 * DESCRIPTION:	list special access
 */
static void cmd_access(object user, string cmd, string str)
{
    mapping access, *values;
    mixed *files;
    string *users;
    int i, sz;

    if (!str) {
	str = query_owner();
    }

    if (str == "global") {
	str = implode(query_global_access(), "\n /usr/");
	if (strlen(str) != 0) {
	    message("Global read access:\n /usr/" + str + "\n");
	}
    } else if (sizeof(query_users() & ({ str })) != 0) {
	access = query_user_access(str);
	str = capitalize(str);
	switch (map_sizeof(access)) {
	case 0:
	    message(str + " has no special access.\n");
	    break;

	case 1:
	    message(str + " has access to:" + list_access(access));
	    break;

	default:
	    message(str + " has access to:\n" + list_access(access));
	    break;
	}
    } else {
	if (sscanf(str, "%*s ") != 0 || (files=expand(str, 0, TRUE))[4] != 1) {
	    message("Usage: " + cmd + " <user> | global | <directory>\n");
	    return;
	}
	str = files[0][0];
	access = query_file_access(str);
	users = map_indices(access);
	if (sizeof(users) != 0) {
	    values = map_values(access);
	    for (i = 0, sz = sizeof(users); i < sz; i++) {
		access = values[i];
		message(capitalize(users[i]) +
			((map_sizeof(access) == 1) ?
			  " has access to:" : " has access to:\n") +
			list_access(values[i]));
	    }
	} else {
	    message("No special access to " + str + ".\n");
	}
    }
}

/*
 * NAME:	cmd_grant()
 * DESCRIPTION:	grant access
 */
static void cmd_grant(object user, string cmd, string str)
{
    string who, dir;
    mixed type, *files;

    if (!str ||
	(sscanf(str, "%s %s %s", who, dir, type) != 3 &&
	 sscanf(str, "%s %s", who, dir) != 2) ||
	(who == "global" && type) ||
	((dir == "access") ? type : (files=expand(dir, 0, TRUE))[4] != 1)) {
	message(
	    "Usage: " + cmd + " <user> access\n" +
	    "       " + cmd + " <user> <directory> [read | write | full]\n" +
	    "       " + cmd + " global <directory>\n");
	return;
    }

    str = (dir == "access") ? dir : files[0][0];
    switch (type) {
    case "read":
	type = READ_ACCESS;
	break;

    case nil:
    case "write":
	type = WRITE_ACCESS;
	break;

    case "full":
	type = FULL_ACCESS;
	break;

    default:
	message(
	    "Usage: " + cmd + " <user> access\n" +
	    "       " + cmd + " <user> <directory> [read | write | full]\n" +
	    "       " + cmd + " global <directory>\n");
	return;
    }

    if (who == "global") {
	/*
	 * global access
	 */
	if (sscanf(str, "/usr/%s", str) == 0 || sscanf(str, "%*s/") != 0) {
	    message("Global read access is for directories under /usr only.\n");
	} else if (sizeof(query_global_access() & ({ str })) != 0) {
	    message("That global access already exists.\n");
	} else {
	    set_global_access(str, TRUE);
	}
    } else if (who == "admin") {
	message("Admin has implicit full access.\n");
    } else if (dir == "access") {
	/*
	 * file access
	 */
	if (!(user=PLAYERD->find_user(who))) {
	    message("No such user.\n");
	} else if (sizeof(query_users() & ({ who })) != 0) {
	    message(capitalize(who) + " already has file access.\n");
	} else if (!access(query_owner(), "/", FULL_ACCESS)) {
	    message("Insufficient access granting privileges.\n");
	} else {
	    ::add_user(who);
	    ::add_owner(who);
	    wiztool::make_dir("/usr/" + who);
	    TELNETD->add_wiztool(user);
	}
    } else {
	/*
	 * special access
	 */
	if (sizeof(query_users() & ({ who })) == 0) {
	    message(capitalize(who) + " has no file access.\n");
	} else if (access(who, str + "/*", type)) {
	    message(capitalize(who) + " already has that access.\n");
	} else {
	    set_access(who, str, type);
	}
    }
}

/*
 * NAME:	cmd_ungrant()
 * DESCRIPTION:	remove access
 */
static void cmd_ungrant(object user, string cmd, string str)
{
    string who, dir;
    mixed *files;

    if (!str || sscanf(str, "%s %s", who, dir) != 2 ||
	(dir != "access" &&
	 (sscanf(dir, "%*s ") != 0 || (files=expand(dir, 0, TRUE))[4] != 1))) {
	message("Usage: " + cmd + " <user> access\n" +
		"       " + cmd + " <user> <directory>\n" +
		"       " + cmd + " global <directory>\n");
	return;
    }

    str = (dir == "access") ? dir : files[0][0];

    if (who == "global") {
	/*
	 * global access
	 */
	if (sscanf(str, "/usr/%s", str) == 0 || sscanf(str, "%*s/") != 0) {
	    message("Global read access is for directories under /usr only.\n");
	} else if (sizeof(query_global_access() & ({ str })) == 0) {
	    message("That global access does not exist.\n");
	} else {
	    set_global_access(str, FALSE);
	}
    } else if (who == "admin") {
	message("Access for Admin cannot be removed.\n");
    } else if (dir == "access") {
	/*
	 * file access
	 */
	if (!(user=PLAYERD->find_user(who))) {
	    message("No such user.\n");
	} else if (sizeof(query_users() & ({ who })) == 0) {
	    message(capitalize(who) + " has no file access.\n");
	} else {
	    remove_user(who);
	    TELNETD->remove_wiztool(user);
	}
    } else {
	/*
	 * special access
	 */
	if (sizeof(query_users() & ({ who })) == 0) {
	    message(capitalize(who) + " has no file access.\n");
	} else if (!query_user_access(who)[str]) {
	    message(capitalize(who) + " has no such access.\n");
	} else {
	    set_access(who, str, 0);
	}
    }
}

# define Container	"/lib/base/container"

static void cmd_goto(object user, string cmd, string str)
{
    object room, body;

    if (!str) {
	message("Usage: " + cmd + " <destination>\n");
	return;
    }

    if (str[0] != '/' && str[0] != '~') {
	str = query_directory() + "/" + str;
    }
    room = find_object(str);
    if (!(room && room <- Container)) {
	message("No such room.\n");
    } else {
	body = user->query_body();
	str = capitalize(body->query_name());
	if (body->query_env()) {
	    body->query_env()->message(MSG_UNFORMATTED,
				       str + " steps forward and disappears.\n",
				       body);
	}
	body->move(room);
	room->message(MSG_UNFORMATTED, str + " suddenly appears.\n", body);
	user->query_interface()->look(room, body);
    }
}

/*
 * NAME:	cmd_people()
 * DESCRIPTION:	show people online
 */
static void cmd_people(object user, string cmd, string str)
{
    int i, sz, time, secs, mins, hours, days;
    object *users, conn;
    string *hosts, *names, *idle, *environment;

    if (str) {
	message("Usage: " + cmd + "\n");
	return;
    }

    users = users();
    sz = sizeof(users);
    hosts = allocate(sz);
    names = allocate(sz);
    idle = allocate(sz);
    environment = allocate(sz);

    if (access(query_owner(), "/", WRITE_ACCESS)) {
	for (i = 0; i < sz; i++) {
	    conn = users[i]->query_conn();
	    while (conn <- LIB_USER) {
		conn = conn->query_conn();
	    }
	    hosts[i] = wiztool::query_ip_name(conn);
	}
    }

    time = time();
    for (i = 0; i < sz; i++) {
	user = users[i];
	names[i] = ((user->query_wiztool()) ? "*" : " ") + user->query_name();

	secs = time - user->query_last_activity();
	if (secs >= 60) {
	   days = secs / 86400;	secs %= 86400;
	   hours = secs / 3600;	secs %= 3600;
	   mins = secs / 60;	secs %= 60;

	   if (days != 0) {
	      str = days + "d";
	      if (hours != 0) {
		 str += " " + hours + "h";
	      }
	   } else if (hours != 0) {
	      str = hours + "h";
	      if (mins != 0) {
		 str += " " + mins + "m";
	      }
	   } else if (mins != 0) {
	      str = mins + "m";
	      if (secs != 0) {
		 str += " " + secs + "s";
	      }
	   } else {
	      str = secs + "s";
	   }
	   idle[i] = "[" + str + "]";
	}

	user = user->query_body();
	if (user) {
	    user = user->query_env();
	    if (user) {
		environment[i] = object_name(user);
	    }
	}
    }

    message(tabulate(hosts, names, idle, environment));
}

/*
 * NAME:	cmd_hotboot()
 * DESCRIPTION:	hotboot system
 */
static void cmd_hotboot(object user, string cmd, string str)
{
    if (str) {
	message("Usage: " + cmd + "\n");
	return;
    }

    if (!access(query_owner(), "/", FULL_ACCESS)) {
	message("Permission denied.\n");
    } else {
	dump_state(TRUE);
	shutdown(TRUE);
    }
}

/*
 * NAME:	cmd_wiz()
 * DESCRIPTION:	talk/emote on a (primitive) wizline
 */
static void cmd_wiz(object user, string cmd, string str)
{
    int i, sz;
    string name;
    object *users, usr;
    
    if (!str) {
	message("Usage: " + cmd + " <text>\n");
	return;
    }

    name = capitalize(user->query_name());
    switch (cmd) {
    case "@wiz":
	str = indent_string(name + " :: ", str);
	break;

    case "@wiz:":
	str = indent_string(":: " + name + " ", str);
	break;
    }

    users = users();
    for (i = 0, sz = sizeof(users); i < sz; i++) {
	usr = users[i];
	if (usr->query_wiztool()) {
	    usr->message(MSG_FORMATTED, str);
	}
    }
}


/*
 * NAME:	command()
 * DESCRIPTION:	process user input
 */
static int command(string str, mixed *args)
{
    string arg;

    if (query_editor(this_object())) {
	if (strlen(str) != 0 && str[0] == '!') {
	    str = str[1 ..];
	} else {
	    str = editor(str);
	    if (str) {
		message(str);
	    }
	    return TRUE;
	}
    }

    if (str == "") {
	return TRUE;
    }

    sscanf(str, "%s %s", str, arg);

    switch (str) {
    case "@code":
    case "@history":
    case "@clear":
    case "@clone":
    case "@destruct":
    case "@new":

    case "@compile":
    case "@issues":
    case "@upgrade":

    case "@cd":
    case "@pwd":
    case "@ls":
    case "@cp":
    case "@mv":
    case "@rm":
    case "@mkdir":
    case "@rmdir":
    case "@ed":

    case "@access":
    case "@grant":
    case "@ungrant":
    case "@quota":
    case "@rsrc":

    case "@goto":
    case "@people":
    case "@status":
    case "@swapout":
    case "@snapshot":
    case "@reboot":
    case "@hotboot":
	call_other(this_object(), "cmd_" + str[1 ..], user, str, arg);
	break;

    case "@halt":
	call_other(this_object(), "cmd_shutdown", user, str, arg);
	break;

    case "@wiz":
    case "@wiz:":
	call_other(this_object(), "cmd_wiz", user, str, arg);
	break;

    default:
	message("No command: " + str + "\n");
	break;
    }

    return TRUE;
}
