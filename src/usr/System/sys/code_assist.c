/*
 * Proxy for having sane communication between Visual Studio Code and DGD.
 * 
 *
 * DO NOT MODIFY this file unless you have disabled automatic installation
 * in the Visual Studio Code extension. Your changes WILL be overwritten
 * if it is enabled. The setting is callde "codeAssistProxyInstall".
 * 
 * 
 * Notes:
 * - All calls must be indirectly executed by a user, so that message() works.
 * - Only programs in /usr/admin/ can use any functionality in here.
 */
#include <limits.h>
#include <status.h>
#include <std.h>
#include <type.h>
#include <kernel/user.h>
#include <kernel/kernel.h>

#ifndef USR_DIR
# define USR_DIR "/usr"
#endif

#define PROXY_VERSION 11
#define OWNER "admin"
#define ADMIN()	sscanf(previous_program(), USR_DIR + "/" + OWNER + "/%*s")

private int result_id;


static void create()
{
	result_id = 0;
}


int version()
{
	if(ADMIN() || SYSTEM()) {
		return PROXY_VERSION;
	}
	return 0;
}

void calledOutFunction(object user) {
	user->message("ping\n");
}

void callOutSomeFunction(object user) {
	call_out("calledOutFunction", 1, user);
}

/*
 * Copy of dump_value() from /kernel/lib/wiztool.c with minor tweaks.
 */
private string encode_json(mixed value, mapping seen)
{
	string str;
	int i, sz;
	mixed *indices, *values;

	switch (typeof(value)) {
		case T_FLOAT:
			str = (string) value;
			if (sscanf(str, "%*s.") == 0 && sscanf(str, "%*se") == 0) {
				str += ".0";
			}
			return str;

		case T_INT:
			return (string) value;

		case T_STRING:
			str = value;
			if (sscanf(str, "%*s\\") != 0) {
				str = implode(explode("\\" + str + "\\", "\\"), "\\\\");
			}
			if (sscanf(str, "%*s\"") != 0) {
				str = implode(explode("\"" + str + "\"", "\""), "\\\"");
			}
			if (sscanf(str, "%*s\n") != 0) {
				str = implode(explode("\n" + str + "\n", "\n"), "\\n");
			}
			if (sscanf(str, "%*s\t") != 0) {
				str = implode(explode("\t" + str + "\t", "\t"), "\\t");
			}
			return "\"" + str + "\"";

		case T_OBJECT:
			str = "";
			catch(
				str = value->to_string()
			);
			return encode_json("<" + object_name(value) + (str == "" || str == nil ? "" : ": " +  str + " ") + ">", seen);

		case T_ARRAY:
			if (seen[value]) {
				return "" + (seen[value] - 1);
			}

			seen[value] = map_sizeof(seen) + 1;
			sz = sizeof(value);
			if (sz == 0) {
				return "[ ]";
			}

			str = "[ ";
			for (i = 0, --sz; i < sz; i++) {
				str += encode_json(value[i], seen) + ", ";
			}
			return str + encode_json(value[i], seen) + " ]";

		case T_MAPPING:
			if (seen[value]) {
				return "" + (seen[value] - 1);
			}

			seen[value] = map_sizeof(seen) + 1;
			sz = map_sizeof(value);
			if (sz == 0) {
				return "{ }";
			}

			str = "{ ";
			indices = map_indices(value);
			values = map_values(value);
			for (i = 0, --sz; i < sz; i++) {
				str += encode_json("" + indices[i], seen) + ":" +
				encode_json(values[i], seen) + ", ";
			}
			return str + encode_json("" + indices[i], seen) + ":" +
					encode_json(values[i], seen) + " }";

		case T_NIL:
			return "null";
	}
}


/*
 * Output a result as JSON to this_user().
 */
private string result(mixed value, string error)
{
	this_user()->message(encode_json(
		([
			"id" : (result_id++),
			"success" : (error ? 0 : 1),
			"error" : error,
			"result" : value
		]),
		([ ])
	) + "\n");

	/* Make /usr/admin/_code.c always return this, VSCode will use it to detect end of a "message" and discard it */
	return "##ignore##";
}

/*
 * NAME:	parse_code()
 * DESCRIPTION:	parse the argument of code, replacing $num by the proper
 *		historic reference
 */
static mixed *parse_code(string str)
{
	mixed *argv;
	int argc, len, i, c;
	string result, head, tail, tmp;

	argv = ({ nil });
	argc = 0;
	result = "";

	result += str;
	len = strlen(result);
	if (len != 0 && result[len - 1] != ';' && result[len - 1] != '}') {
		result = "return " + result + ";";
	}
	argv[0] = result;
	return argv;
}


/*
 * Copy of cmd_code() from /kernel/lib/wiztool.c with minor tweaks.
 */
string code(string str)
{
	mixed *parsed, result;
	object obj, user;
	string name;
	string res;

	if(!ADMIN()) {
		return result(nil, "Not allowed");
	}

	user = this_user();

	if (!str) {
		return result(nil, "Usage: code <LPC-code>");
	}

	parsed = parse_code(str);

	name = USR_DIR + "/System/_code";
	obj = find_object(name);
	if (obj) {
		destruct_object(obj);
	}

	if (!parsed) {
		return result(nil, "failed to parse _code");;
	}

	str = "# include <float.h>\n# include <limits.h>\n" +
		"# include <status.h>\n# include <trace.h>\n" +
		"# include <type.h>\n" +
		"mixed exec(object user, mixed argv...) {\n" +
		"    mixed a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;\n\n" +
		"    " + parsed[0] + "\n}\n";
	res = catch(
		obj = compile_object(name, str),
		result = obj->exec(user, parsed[1 ..]...)
	);

	if (obj) {
		destruct_object(obj);
	}

	if (res) {
		DRIVER->message("_code failed: " + str + "\n");
		return result(nil, "Error: " + res + ". Code:\n" + str);
	} else {
		return result(result, nil);
	}
}

mixed uninstall()
{
	int res;

	if(!ADMIN()) {
		return result(nil, "Not allowed");
	}

	res = rename_file(
		object_name(this_object()) + ".c", 
		USR_DIR + "/" + this_user()->query_name() + "/code_assist." + time() + ".bak"
	);
	destruct_object(this_object());
	return res;
}
