# include "HttpHeader.h"

void create()
{
    compile_object("sys/requestline");
    compile_object("sys/headers");
    compile_object("sys/urlencode");
    compile_object("sys/urldecode");
    compile_object("sys/date");
    compile_object(OBJECT_PATH(RemoteHttpAuthentication));
    compile_object(OBJECT_PATH(RemoteHttpTime));
    compile_object(OBJECT_PATH(HttpHeader));
    compile_object(OBJECT_PATH(RemoteHttpHeaders));
}
