# include <kernel/kernel.h>
# include <kernel/user.h>
# include "~HTTP/api/include/http.h"

inherit "~System/lib/user";

# define SYS_INITD	"/usr/System/initd"

object userd;		/* user daemon */
string httphost;
string ftphost, ftphost2;
mapping urlmap;
string errormessage;	/* message returned in case of a login error */

static void create(void)
{
    userd = find_object(USERD);
    SYS_INITD->set_connection_manager("binary", 0, this_object());
    httphost = "localhost:8080";
    ftphost = "ftphost:8080";
    ftphost2 = "ftphost2:8080";
    urlmap = ([ "" : "~/obj/http" ]);
    errormessage = "<HTML>\n" +
		   "<HEAD><TITLE>400 Bad Request</TITLE></HEAD>\n" +
		   "<BODY><H1>400 Bad Request</H1></BODY>\n" +
		   "</HTML>\n";
}

string *query_host(void) { return ({ httphost, ftphost, ftphost2 }); }

static object request(string str)
{
    mixed *request;
    object obj;
    string *dirs;
    int i, sz;

    request = HTTPRequest->parse_request(str);
    if (request) {
	dirs = map_indices(urlmap);
	str = nil;
	for (i = 0, sz = sizeof(dirs); i < sz; i++) {
	    if (sscanf(request[HTTPREQ_PATH], dirs[i] + "/%*s") != 0) {
		str = urlmap[dirs[i]];
	    }
	}
	if (str != nil) {
	    obj = clone_object(str);
	    obj->request(request);
	    return obj;
	}
    }
}

/*
 * NAME:	select()
 * DESCRIPTION:	select user object based on loginname
 */
object select(string str)
{
    if (previous_object() == userd) {
	object obj;

	obj = request(str);
	return (obj) ? obj : this_object();
    }
}

/*
 * NAME:	query_timeout()
 * DESCRIPTION:	return login timeout
 */
int query_timeout(object obj)
{
    return 30;
}

/*
 * NAME:	query_banner()
 * DESCRIPTION:	return login banner
 */
string query_banner(object obj)
{
    return nil;
}

/*
 * NAME:	set_errormessage()
 * DESCRIPTION:	set the errormessage shown in case of a login error
 */
void set_errormessage(string str)
{
    if (SYSTEM()) {
	errormessage = str;
    }
}

/*
 * NAME:	login()
 * DESCRIPTION:	display an errormessage and disconnect
 */
int login(string str)
{
    previous_object()->message(errormessage);
    return MODE_DISCONNECT;
}
