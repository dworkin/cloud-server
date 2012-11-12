# include <http.h>

string httphost;
string ftphost;
mapping urlmap;

static void create()
{
    httphost = "localhost";
    ftphost = "ftphost";
    urlmap = ([ "" : HTTP_HANDLER ]);
}

string *query_host() { return ({ httphost, ftphost }); }

object request(string str)
{
    if (previous_program() == "/usr/System/sys/binaryd") {
	mixed *request;
	object obj;
	string *dirs;
	int i, sz;

	request = HTTP_REQUEST->parse_request(str);
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
}
