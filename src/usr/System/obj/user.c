# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/user.h>
# include <status.h>
# include <type.h>

inherit auto	"~/lib/auto";
inherit user	LIB_USER;
inherit wiztool	LIB_WIZTOOL;

private inherit	"/lib/util/string";


# define USER			"/usr/System/obj/user"
# define USERSERVER		"/usr/System/sys/userd"
# define OBJECTSERVER		"/usr/System/sys/objectd"
# define UPGRADESERVER		"/usr/System/sys/upgraded"

# define STATE_NORMAL		0
# define STATE_LOGIN		1
# define STATE_OLDPASSWD	2
# define STATE_NEWPASSWD1	3
# define STATE_NEWPASSWD2	4
# define STATE_PASTING		5

static string name;		/* user name */
static string Name;		/* capitalized user name */
static mapping state;		/* state for a connection object */
string password;		/* user password */
static string newpasswd;	/* new password */
static string paste_buffer;	/* buffer holding text being pasted */
static int nconn;		/* # of connections */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize object
 */
static create()
{
    wiztool::create(200);
    state = ([ ]);
}

/*
 * NAME:	find_object()
 * DESCRIPTION:	find_object wrapper
 */
static object find_object(string path)
{
    return auto::find_object(path);
}

/*
 * NAME:	clone_object()
 * DESCRIPTION:	clone_object wrapper
 */
static object clone_object(string path, mixed args...)
{
    string owner;

    owner = query_owner();
    path = DRIVER->normalize_path(path, query_directory(), owner);
    if (sscanf(path, "/kernel/%*s") != 0 || !access(owner, path, READ_ACCESS)) {
	message(path + ": Permission denied.\n");
	return nil;
    }
    return auto::clone_object(path, args...);
}

/*
 * NAME:	new_object()
 * DESCRIPTION:	new_object wrapper
 */
static object new_object(string path, mixed args...)
{
    string owner;

    owner = query_owner();
    path = DRIVER->normalize_path(path, query_directory(), owner);
    if (sscanf(path, "/kernel/%*s") != 0 || !access(owner, path, READ_ACCESS)) {
	message(path + ": Permission denied.\n");
	return nil;
    }
    return auto::new_object(path, args...);
}

/*
 * NAME:	compile_object()
 * DESCRIPTION:	compile an object
 */
static object compile_object(string path, string source...)
{
    if (path && sscanf(path, "%*s/@@@/") != 0) {
	message(path + ": Cannot compile leaf object.\n");
	return nil;
    }
    return wiztool::compile_object(path, source...);
}

/*
 * NAME:	ident()
 * DESCRIPTION:	find user
 */
static object ident(string str)
{
    return USERSERVER->find_user(str);
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
    objectd = find_object(OBJECTSERVER);

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
 * DESCRIPTION:	upgrade a source file and all that depends on it
 */
static void cmd_upgrade(object user, string cmd, string str)
{
    string *sources;
    int atom, i, sz, len;
    object patcher;

    if (!str || str == "-a") {
	message("Usage: " + cmd + " [-a|-p] <file> [<file> ...]\n");
	return;
    }
    atom = sscanf(str, "-a %s", str);
    if (!atom) {
	atom = sscanf(str, "-p %s", str);
	if (atom) {
	    patcher = this_object();
	}
    }
    sources = expand(str, 1, TRUE)[0];
    if (sizeof(sources) != 0) {
	mixed result;

	result = UPGRADESERVER->upgrade(query_owner(), sources, atom, patcher);
	if (typeof(result) == T_STRING) {
	    message(result + "\n");
	} else {
	    if (!atom || sizeof(result) == 0) {
		sources -= ({ nil });
		if (sizeof(sources) != 0) {
		    message("Successfully upgraded:\n" +
			    break_string(implode(sources, ", "), 0, 2));
		}
	    }

	    if (sizeof(result) != 0) {
		message("Errors occured compiling:\n" +
			break_string(implode(result, ".c, ") + ".c", 0, 2));
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
    if (previous_program() == UPGRADESERVER) {
	call_limited("call_patch", obj);
    }
}

/*
 * NAME:	call_patch()
 * DESCRIPTION:	patch an object
 */
static void call_patch(object obj)
{
    call_other(obj, "???");
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
	if (sscanf(who, "%*s/") != 0) {
	    message("Invalid user name.\n");
	} else if (sizeof(query_users() & ({ who })) != 0) {
	    message(capitalize(who) + " already has file access.\n");
	} else if (!access(query_owner(), "/", FULL_ACCESS)) {
	    message("Insufficient access granting privileges.\n");
	} else {
	    ::add_user(who);
	    ::add_owner(who);
	    wiztool::make_dir("/usr/" + who);
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
	if (!(user=USERSERVER->find_user(who))) {
	    message("No such user.\n");
	} else if (sizeof(query_users() & ({ who })) == 0) {
	    message(capitalize(who) + " has no file access.\n");
	} else {
	    remove_user(who);
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

/*
 * NAME:	cmd_snapshot()
 * DESCRIPTION:	create a snapshot
 */
static void cmd_snapshot(object user, string cmd, string str)
{
    int incr;

    if (str) {
	if (str != "-f") {
	    message("Usage: " + cmd + " [-f]\n");
	    return;
	}
	incr = FALSE;
    } else {
	incr = TRUE;
    }

    dump_state(incr);
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
 * NAME:	command()
 * DESCRIPTION:	process user input
 */
static int command(string str)
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
    case "code":
    case "history":
    case "clear":
    case "clone":
    case "destruct":
    case "new":

    case "compile":
    case "issues":
    case "upgrade":

    case "cd":
    case "pwd":
    case "ls":
    case "cp":
    case "mv":
    case "rm":
    case "mkdir":
    case "rmdir":
    case "ed":

    case "access":
    case "grant":
    case "ungrant":
    case "quota":
    case "rsrc":

    case "people":
    case "status":
    case "swapout":
    case "snapshot":
    case "reboot":
    case "hotboot":
	call_other(this_object(), "cmd_" + str, this_object(), str, arg);
	break;

    case "halt":
	call_other(this_object(), "cmd_shutdown", this_object(), str, arg);
	break;

    default:
	message("No command: " + str + "\n");
	break;
    }

    return TRUE;
}


/*
 * NAME:	tell_audience()
 * DESCRIPTION:	send message to listening users
 */
private void tell_audience(string str)
{
    object *users, user;
    int i;

    users = users();
    for (i = sizeof(users); --i >= 0; ) {
	user = users[i];
	if (user != this_object() &&
	    sscanf(object_name(user), USER + "#%*d") != 0) {
	    user->message(str);
	}
    }
}

/*
 * NAME:	login()
 * DESCRIPTION:	login a new user
 */
int login(string str)
{
    if (previous_program() == LIB_CONN) {
	if (nconn == 0) {
	    ::login(str);
	}
	nconn++;
	if (strlen(str) == 0 || sscanf(str, "%*s ") != 0 ||
	    sscanf(str, "%*s/") != 0) {
	    return MODE_DISCONNECT;
	}
	Name = name = str;
	if (Name[0] >= 'a' && Name[0] <= 'z') {
	    Name[0] -= 'a' - 'A';
	}

	if (password) {
	    /* check password */
	    previous_object()->message("Password:");
	    state[previous_object()] = STATE_LOGIN;
	} else {
	    /* no password; login immediately */
	    connection(previous_object());
	    tell_audience(Name + " logs in.\n");
	    if (str != "admin" && sizeof(query_users() & ({ str })) == 0) {
		message("> ");
		state[previous_object()] = STATE_NORMAL;
		return MODE_ECHO;
	    }
	    message("Pick a new password:");
	    state[previous_object()] = STATE_NEWPASSWD1;
	}
	return MODE_NOECHO;
    }
}

/*
 * NAME:	logout()
 * DESCRIPTION:	logout user
 */
void logout(int quit)
{
    if (previous_program() == LIB_CONN && --nconn == 0) {
	if (query_conn()) {
	    if (quit) {
		tell_audience(Name + " logs out.\n");
	    } else {
		tell_audience(Name + " disconnected.\n");
	    }
	}
	::logout(name);
    }
}

/*
 * NAME:	query_name()
 * DESCRIPTION:	return this user's name
 */
string query_name()
{
    return name;
}

/*
 * NAME:	receive_message()
 * DESCRIPTION:	process a message from the user
 */
int receive_message(string str)
{
    if (previous_program() == LIB_CONN) {
	string cmd;
	object user, *users;
	int i, sz;

	switch (state[previous_object()]) {
	case STATE_NORMAL:
	    cmd = str;
	    if (strlen(str) != 0 && str[0] == '!') {
		cmd = cmd[1 ..];
	    }

	    if (!query_editor(this_object()) || cmd != str) {
		/* check standard commands */
		if (strlen(cmd) != 0) {
		    switch (cmd[0]) {
		    case '\'':
			if (strlen(cmd) > 1) {
			    cmd[0] = ' ';
			    str = cmd;
			}
			cmd = "say";
			break;

		    case ':':
			if (strlen(cmd) > 1) {
			    cmd[0] = ' ';
			    str = cmd;
			}
			cmd = "emote";
			break;

		    default:
			sscanf(cmd, "%s ", cmd);
			break;
		    }
		}

		switch (cmd) {
		case "say":
		    if (sscanf(str, "%*s %s", str) == 0) {
			message("Usage: say <text>\n");
		    } else {
			tell_audience(Name + " says: " + str + "\n");
		    }
		    str = nil;
		    break;

		case "emote":
		    if (sscanf(str, "%*s %s", str) == 0) {
			message("Usage: emote <text>\n");
		    } else {
			tell_audience(Name + " " + str + "\n");
		    }
		    str = nil;
		    break;

		case "tell":
		    if (sscanf(str, "%*s %s %s", cmd, str) != 3 ||
			!(user=find_user(cmd))) {
			message("Usage: tell <user> <text>\n");
		    } else {
			user->message(Name + " tells you: " + str + "\n");
		    }
		    str = nil;
		    break;

		case "users":
		    users = users();
		    str = "Logged on:";
		    for (i = 0, sz = sizeof(users); i < sz; i++) {
			cmd = users[i]->query_name();
			if (cmd) {
			    str += " " + cmd;
			}
		    }
		    message(str + "\n");
		    str = nil;
		    break;

		case "password":
		    if (password) {
			message("Old password:");
			state[previous_object()] = STATE_OLDPASSWD;
		    } else {
			message("New password:");
			state[previous_object()] = STATE_NEWPASSWD1;
		    }
		    return MODE_NOECHO;

		case "paste":
		    ::message("End your pasting with a single period.\n\"\b");
		    state[previous_object()] = STATE_PASTING;
		    paste_buffer = "";
		    return MODE_ECHO;

		case "quit":
		    return MODE_DISCONNECT;
		}
	    }

	    if (str) {
		call_limited("command", str);
	    }
	    break;

	case STATE_LOGIN:
	    if (hash_string("crypt", str, password) != password) {
		previous_object()->message("\nBad password.\n");
		return MODE_DISCONNECT;
	    }
	    connection(previous_object());
	    message("\n");
	    tell_audience(Name + " logs in.\n");
	    break;

	case STATE_OLDPASSWD:
	    if (hash_string("crypt", str, password) != password) {
		message("\nBad password.\n");
		break;
	    }
	    message("\nNew password:");
	    state[previous_object()] = STATE_NEWPASSWD1;
	    return MODE_NOECHO;

	case STATE_NEWPASSWD1:
	    newpasswd = str;
	    message("\nRetype new password:");
	    state[previous_object()] = STATE_NEWPASSWD2;
	    return MODE_NOECHO;

	case STATE_NEWPASSWD2:
	    if (newpasswd == str) {
		password = hash_string("crypt", str);
		message("\nPassword changed.\n");
	    } else {
		message("\nMismatch; password not changed.\n");
	    }
	    newpasswd = nil;
	    break;

	case STATE_PASTING:
	    if (str != ".") {
		paste_buffer += str + "\n";
		::message("\"\b");
		return MODE_ECHO;
	    }
	    if (strlen(paste_buffer)) {
		tell_audience("--- Pasted segment from " + Name +
			      " ---\n" + paste_buffer +
			      "--- Pasted segment ends ---\n");
		state[previous_object()] = STATE_NORMAL;
		paste_buffer = nil;
	    }
	    break;
	}

	str = query_editor(this_object());
	if (str) {
	    message((str == "insert") ? "*\b" : ":");
	} else {
	    message("> ");
	}
	state[previous_object()] = STATE_NORMAL;
	return MODE_ECHO;
    }
}
