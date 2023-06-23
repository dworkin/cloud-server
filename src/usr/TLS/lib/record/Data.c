# include <String.h>
# include "Extension.h"
# include <type.h>


private int type;

static void create(int type)
{
    ::type = type;
}

static string substring(mixed str, int offset, int end)
{
    return (typeof(str) == T_STRING) ?
	    str[offset .. end] :
	    str->bufferRange(offset, end)->chunk();
}

static int len1Offset(mixed str, int offset)
{
    return offset + 1 + str[offset];
}

static int len2Offset(mixed str, int offset)
{
    return offset + 2 + ((str[offset] << 8) | str[offset + 1]);
}

static int len3Offset(String str, int offset)
{
    return offset + 3 +
	   ((str[offset] << 16) | (str[offset + 1] << 8) | str[offset + 2]);
}

static mixed *len1Restore(mixed str, int offset)
{
    int end;

    end = len1Offset(str, offset);
    return ({
	(typeof(str) == T_STRING) ?
	 str[offset + 1 .. end - 1] :
	 str->bufferRange(offset + 1, end - 1)->chunk(),
	end
    });
}

static mixed *len2Restore(mixed str, int offset)
{
    int end;

    end = len2Offset(str, offset);
    return ({
	(typeof(str) == T_STRING) ?
	 str[offset + 2 .. end - 1] :
	 str->bufferRange(offset + 2, end - 1)->chunk(),
	end
    });
}

static mixed *len3Restore(mixed str, int offset)
{
    int end;

    end = len3Offset(str, offset);
    return ({
	(typeof(str) == T_STRING) ?
	 str[offset + 3 .. end - 1] :
	 str->bufferRange(offset + 3, end - 1)->chunk(),
	end
    });
}

static int int4Restore(String str, int offset)
{
    return (str[offset] << 24) | (str[offset + 1] << 16) |
	   (str[offset + 2] << 8) | str[offset + 3];
}

static Extension *extRestore(String str, int offset, int end)
{
    Extension *extensions;
    int extEnd;

    if (end != len2Offset(str, offset)) {
	error("Decode error");
    }
    offset += 2;
    extensions = ({ });
    while (offset < end) {
	extEnd = len2Offset(str, offset + 2);
	extensions += ({
	    new Extension((str[offset] << 8) | str[offset + 1],
			  substring(str, offset + 4, extEnd - 1))
	});
	offset = extEnd;
    }
    if (offset != end) {
	error("Decode error");
    }

    return extensions;
}

static string len1Save(string str)
{
    int length;
    string len;

    length = strlen(str);
    len = ".";
    len[0] = length;

    return len + str;
}

static string len2Save(string str)
{
    int length;
    string len;

    length = strlen(str);
    len = "..";
    len[0] = length >> 8;
    len[1] = length;

    return len + str;
}

static string len3Save(string str)
{
    int length;
    string len;

    length = strlen(str);
    len = "...";
    len[0] = length >> 16;
    len[1] = length >> 8;
    len[2] = length;

    return len + str;
}

static string int4Save(int i)
{
    string str;

    str = "....";
    str[0] = i >> 24;
    str[1] = i >> 16;
    str[2] = i >> 8;
    str[3] = i;

    return str;
}

static string extSave(Extension *extensions)
{
    string *ext;
    int sz, i;

    ext = allocate(sz = sizeof(extensions));
    for (i = 0; i < sz; i++) {
	ext[i] = extensions[i]->transport();
    }
    return len2Save(implode(ext, ""));
}

static string typeHeader()
{
    string str;

    str = ".";
    str[0] = type;
    return str;
}

string transport();


int type()	{ return type; }
