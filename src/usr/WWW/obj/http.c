# include <kernel/kernel.h>
# include <http.h>

inherit HTTP_USER;
inherit "~/lib/http_protocol";

private inherit "/lib/util/string";
private inherit "/lib/util/time";


# define CHUNK_SIZE	65535

int response_code;

static void create(int clone)
{
    if (clone) {
	::create();
	call_out("disconnect", 300);
    }
}

static int query_code() { return response_code; }

private object
response(int code, string str,
	 varargs string type, int length, int modtime, string body)
{
    response_code = code;
    if (!body) {
	body = "";
    }
    return new_object("/data/strbuffer",
		      http_response(code, str, type, length, modtime) + body);
}

private object bad_request()
{
    string str;

    str = "<HTML><HEAD><TITLE>Bad Request</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_BAD_REQUEST + " Bad Request</H1>" +
	  "This server could not understand your query." +
	  "</BODY></HTML>";
    return response(HTTP_BAD_REQUEST, nil, "text/html", strlen(str), 0, str);
}

private object not_implemented()
{
    string str;

    str = "<HTML><HEAD><TITLE>Not Implemented</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_NOT_IMPLEMENTED + " Not Implemented</H1>" +
	  "Unimplemented server function." +
	  "</BODY></HTML>";
    return response(HTTP_NOT_IMPLEMENTED, nil, "text/html", strlen(str), 0,
		    str);
}

private object not_found()
{
    string str;

    str = "<HTML><HEAD><TITLE>Not Found</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_NOT_FOUND + " Not Found</H1>" +
	  "This server could not find the path you specified." +
	  "</BODY></HTML>";
    return response(HTTP_NOT_FOUND, nil, "text/html", strlen(str), 0, str);
}

private object unauthorized(string realm)
{
    string str;

    str = "<HTML><HEAD><TITLE>Unauthorized</TITLE></HEAD>" +
	  "<BODY><H1>" + HTTP_UNAUTHORIZED + " Unauthorized</H1>" +
	  "You must supply a user name and password to use this method." +
	  "</BODY></HTML>";
    return response(HTTP_UNAUTHORIZED, realm, "text/html", strlen(str), 0, str);
}

private string file_type(string file)
{
    string *exp;

    exp = explode(file, ".");
    if (sizeof(exp) > 1) {
	switch (lower_case(exp[sizeof(exp) - 1])) {
	case "html":
	    return "text/html";

	case "jpg":
	    return "image/jpeg";

	case "gif":
	    return "image/gif";

	case "png":
	    return "image/png";

	case "mov":
	    return "video/quicktime";

	case "zip":
	    return "application/zip";

	case "gz":
	case "tgz":
	    return "application/x-gzip";

	case "lha":
	    return "application/x-lha";

	case "zoo":
	    return "application/x-zoo";

	case "hqx":
	    return "application/x-hqx";
	}
    }

    return "text/plain";
}

private mixed *http_file_info(string file)
{
    mixed *info;

    info = file_info(file);
    if (info) {
	info[2] = file_type(file);
    }

    return info;
}

private mixed *http_get_dir(string dir)
{
    mixed **dirlist;
    int i;

    dirlist = get_dir(dir);
    for (i = 0; i < sizeof(dirlist[0]); i++) {
	dirlist[3][i] = file_type(dirlist[0][i]);
    }

    return dirlist;
}

private string display(float f)
{
    string str, frag;

    str = (string) f;
    if (sscanf(str, "%s.%s", str, frag) != 0 && strlen(str) == 1) {
	str += "." + frag[0 .. 0];
    }
    return str;
}

private string ftp_dir(string dir)
{
    mixed **list;
    string str, parent, month, time, year;
    int i, day;

    list = http_get_dir(dir + "/*");
    sscanf(dir, "/usr/WWW/ftp%s", dir);
    if (dir != "/" && dir[strlen(dir) - 1] == '/') {
	dir = dir[.. strlen(dir) - 2];
    }
    str = "<HTML><HEAD><TITLE>Index of " + dir + "</TITLE></HEAD>" +
	  "<BODY><H1>Index of " + dir + "</H1><TABLE>" +
	  "<TR><TH><IMG SRC=\"http://www.dworkin.nl/icons/blank.gif\"</TH>" +
	  "<TH>Name</TH><TH>Last modified</TH><TH>Size</TH>" +
	  "<TH>Description</TH></TR><TR><TH COLSPAN=\"5\"><HR></TH></TR>";
    if (dir != "/") {
	parent = DRIVER->normalize_path(dir + "/..", "");
	if (parent != "/") {
	    parent += "/";
	}
	str += "<TR><TD VALIGN=\"top\">" +
	       "<IMG SRC=\"http://www.dworkin.nl/icons/back.gif\"</TD>" +
	       "<TD><A HREF=\"" + parent +
	       "\">Parent Directory</A></TD><TD>&nbsp;</TD>" +
	       "<TD ALIGN=\"right\">-</TD></TR>";
    } else {
	dir = "";
    }
    for (i = 0; i < sizeof(list[0]); i++) {
	str += "<TR><TD VALIGN=\"top\">";
	if (list[1][i] == -2) {
	    str += "<IMG SRC=\"http://www.dworkin.nl/icons/folder.gif\"";
	} else if (sscanf(list[3][i], "application/%*s") != 0) {
	    str += "<IMG SRC=\"http://www.dworkin.nl/icons/compressed.gif\"";
	} else {
	    str += "<IMG SRC=\"http://www.dworkin.nl/icons/text.gif\"";
	}
	str += "</TD><TD><A HREF=\"" + dir + "/" + list[0][i];
	if (list[1][i] == -2) {
	    str += "/";
	}
	str += "\">" + list[0][i] + "</A></TD>";
	sscanf(ctime(list[2][i]), "%*s %s %d %s %s", month, day, time, year);
	str += "<TD ALIGN=\"right\">" + day + "-" + month + "-" + year + " " +
	       time + "</TD><TD ALIGN=\"right\">";
	switch (list[1][i]) {
	case -2:
	    str += "-";
	    break;

	case 0 .. 1023:
	    str += list[1][i];
	    break;

	case 1024 .. 1048575:
	    str += display((float) list[1][i] / 1024.0) + "K";
	    break;

	default:
	    str += display((float) list[1][i] / 1048576.0) + "M";
	    break;
	}
	str += "</TD></TR>";
    }
    str += "<TR><TH COLSPAN=\"5\"><HR></TH></TR></TABLE></BODY></HTML>";

    return str;
}

private object http_read_file(object mesg, string file)
{
    int offset, len;
    string str;

    offset = 0;

    for (;;) {
	str = read_file(file, offset, CHUNK_SIZE);
	len = strlen(str);
	if (len == 0) {
	    return mesg;
	}
	mesg->append(str);
	offset += len;
    }
}

static object http_message(int code, object entity)
{
    mixed *info;
    string scheme, host, file;
    int modified;
    object mesg;

    scheme = query_scheme();
    if (scheme && lower_case(scheme) != "http") {
	return not_found();
    }
    host = query_host();
    if (!host) {
	host = "www.dworkin.nl";
    } else {
	host = lower_case(host);
    }
    file = query_path();
    if (!file) {
	return bad_request();
    }
    file = DRIVER->normalize_path(file, "");

    switch (host) {
    case "www.dworkin.nl":
	file = "/usr/WWW/html" + file;

	info = http_file_info(file);
	if (!info) {
	    return not_found();
	}
	if (info[0] == -2) {
	    file += "/index.html";
	    info = http_file_info(file);
	    if (!info) {
		return not_found();
	    }
	}

	switch (query_method()) {
	case "GET":
	    modified = query_if_modified_since();
	    if (modified != 0 && timecmp(modified, info[1]) >= 0) {
		return response(HTTP_NOT_MODIFIED, nil);
	    }
	    mesg = response(HTTP_OK, nil, info[2], info[0], info[1]);
	    http_read_file(mesg, file);
	    return mesg;

	case "HEAD":
	    return response(HTTP_OK, nil, info[2], info[0], info[1]);

	case "POST":
	    if (query_authorization() != "foodibar:gnulifoo") {
		return unauthorized("posting");
	    }
	    mesg = response(HTTP_CREATED, nil, info[2], info[0], info[1]);
	    http_read_file(mesg, file);
	    return mesg;

	default:
	    return not_implemented();
	}

    case "ftp.dworkin.nl":
	file = "/usr/WWW/ftp" + file;

	info = http_file_info(file);
	if (!info) {
	    return not_found();
	}

	switch (query_method()) {
	case "GET":
	    modified = query_if_modified_since();
	    if (modified != 0 && timecmp(modified, info[1]) >= 0) {
		return response(HTTP_NOT_MODIFIED, nil);
	    }
	    if (info[0] == -2) {
		file = ftp_dir(file);
		info[0] = strlen(file);
		mesg = response(HTTP_OK, nil, "text/html", info[0], info[1]);
		mesg->append(file);
	    } else {
		mesg = response(HTTP_OK, nil, info[2], info[0], info[1]);
		http_read_file(mesg, file);
	    }
	    return mesg;

	case "HEAD":
	    if (info[0] == -2) {
		file = ftp_dir(file);
		info[0] = strlen(file);
		mesg = response(HTTP_OK, nil, "text/html", info[0], info[1]);
	    } else {
		mesg = response(HTTP_OK, nil, info[2], info[0], info[1]);
	    }
	    return mesg;

	default:
	    return not_implemented();
	}

    default:
	return not_found();
    }
}
