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
#include <NKlib.h>

inherit jsonEncoder JSON_ENCODER_LIB;

#ifndef USR_DIR
# define USR_DIR "/usr"
#endif

#define PROXY_VERSION 11
#define OWNER "admin"
#define ADMIN()    sscanf(previous_program(), USR_DIR + "/" + OWNER + "/%*s")

private int result_id;


static void create(void) {
    result_id = 0;
}


int version(void) {
    if(ADMIN() || SYSTEM()) {
        return PROXY_VERSION;
    }
    return 0;
}

/*
 * Output a result as JSON to this_user().
 */
private string result(mixed value, string error) {
    this_user()->message(jsonEncoder::encode(
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
 * NAME:    parse_code()
 * DESCRIPTION:    parse the argument of code, replacing $num by the proper
 *        historic reference
 */
static mixed *parse_code(string str) {
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
string code(string str) {
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

mixed uninstall(void) {
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
