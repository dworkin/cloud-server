# include <kernel/kernel.h>
# include <Time.h>
# include "~HTTP/HttpRequest.h"
# include "~HTTP/HttpResponse.h"

inherit "~/lib/http";
private inherit "/lib/util/ascii";
private inherit base64 "/lib/util/base64";


# define CHUNK_SIZE	65535

string httphost;
string ftphost;
string ftphost2;

static void create()
{
    ::create();
    ({ httphost, ftphost, ftphost2 }) = "~WWW/sys/server"->query_host();
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

	case "pdf":
	    return "application/pdf";

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
	  "<TR><TH><IMG SRC=\"http://" + httphost + "/icons/blank.png\"</TH>" +
	  "<TH>Name</TH><TH>Last modified</TH><TH>Size</TH>" +
	  "<TH>Description</TH></TR><TR><TH COLSPAN=\"5\"><HR></TH></TR>";
    if (dir != "/") {
	parent = DRIVER->normalize_path(dir + "/..", "");
	if (parent != "/") {
	    parent += "/";
	}
	str += "<TR><TD VALIGN=\"top\">" +
	       "<IMG SRC=\"http://" + httphost + "/icons/back.png\"</TD>" +
	       "<TD><A HREF=\"" + parent +
	       "\">Parent Directory</A></TD><TD>&nbsp;</TD>" +
	       "<TD ALIGN=\"right\">-</TD></TR>";
    } else {
	dir = "";
    }
    for (i = 0; i < sizeof(list[0]); i++) {
	str += "<TR><TD VALIGN=\"top\">";
	if (list[1][i] == -2) {
	    str += "<IMG SRC=\"http://" + httphost + "/icons/folder.png\"";
	} else if (sscanf(list[3][i], "application/%*s") != 0) {
	    str += "<IMG SRC=\"http://" + httphost + "/icons/compressed.png\"";
	} else {
	    str += "<IMG SRC=\"http://" + httphost + "/icons/text.png\"";
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

mixed *http_message(int code, HttpRequest request, object entity)
{
    mixed *info, value;
    string scheme, host, file;
    int code2;
    object mesg, header;

    if (request) {
	code2 = http_request(request);
    }
    if (code != 0) {
	return response(code, nil);
    }
    if (code2 != 0) {
	return response(code2, nil);
    }

    scheme = request->scheme();
    if (scheme && lower_case(scheme) != "http://") {
	return not_found();
    }
    host = request->host();
    if (!host) {
	host = httphost;
    } else {
	host = lower_case(host);
    }
    file = request->path();
    if (!file) {
	return bad_request();
    }
    sscanf(file, "%s?", file);
    file = DRIVER->normalize_path(file, "");

    if (host == httphost) {
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

	switch (request->method()) {
	case "GET":
	    value = request->headerValue("If-Modified-Since");
	    if (value && value->time() >= new Time(info[1])) {
		return response(HTTP_NOT_MODIFIED, nil);
	    }
	    mesg = response(HTTP_OK, nil, info[2], info[0], info[1])[1];
	    http_read_file(mesg, file);
	    return ({ HTTP_OK, mesg });

	case "HEAD":
	    return response(HTTP_OK, nil, info[2], info[0], info[1]);

	case "POST":
	    value = request->headerValue("Authorization");
	    if (value && lower_case(value->scheme()) == "basic" &&
		base64::decode(value->authorization()) != "foobar:gnu") {
		return unauthorized("posting");
	    }
	    mesg = response(HTTP_CREATED, nil, info[2], info[0], info[1])[1];
	    http_read_file(mesg, file);
	    return ({ HTTP_CREATED, mesg });

	default:
	    return not_implemented();
	}
    } else if (host == ftphost) {
	file = "/usr/WWW/ftp" + file;

	info = http_file_info(file);
	if (!info) {
	    return not_found();
	}

	switch (request->method()) {
	case "GET":
	    value = request->headerValue("If-Modified-Since");
	    if (value && value->time() >= new Time(info[1])) {
		return response(HTTP_NOT_MODIFIED, nil);
	    }
	    if (info[0] == -2) {
		file = ftp_dir(file);
		info[0] = strlen(file);
		mesg = response(HTTP_OK, nil, "text/html", info[0], info[1])[1];
		mesg->append(file);
	    } else {
		mesg = response(HTTP_OK, nil, info[2], info[0], info[1])[1];
		http_read_file(mesg, file);
	    }
	    return ({ HTTP_OK, mesg });

	case "HEAD":
	    if (info[0] == -2) {
		file = ftp_dir(file);
		info[0] = strlen(file);
		mesg = response(HTTP_OK, nil, "text/html", info[0], info[1])[1];
	    } else {
		mesg = response(HTTP_OK, nil, info[2], info[0], info[1])[1];
	    }
	    return ({ HTTP_OK, mesg });

	default:
	    return not_implemented();
	}
    } else if (host == ftphost2) {
	return response(HTTP_MOVED_PERMANENTLY, "http://" + ftphost + file);
    } else {
	return not_found();
    }
}
