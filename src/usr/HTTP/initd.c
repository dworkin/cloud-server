# include "HttpRequest.h"
# include "HttpResponse.h"
# include "HttpField.h"
# include "HttpConnection.h"


void create()
{
    compile_object("sys/requestline");
    compile_object("sys/fields");
    compile_object("sys/urlencode");
    compile_object("sys/urldecode");
    compile_object("sys/authenticate");
    compile_object("sys/authorize");
    compile_object("sys/date");
    compile_object("sys/products");
    compile_object("sys/tokenparam");
    compile_object("sys/typeparam");
    compile_object("sys/list");
    compile_object("sys/tokenparamlist");
    compile_object(HTTP1_SERVER);
    compile_object(HTTP1_CLIENT);
    compile_object(OBJECT_PATH(RemoteHttpRequest));
    compile_object(OBJECT_PATH(RemoteHttpResponse));
    compile_object(OBJECT_PATH(RemoteHttpAuthentication));
    compile_object(OBJECT_PATH(HttpProduct));
    compile_object(OBJECT_PATH(RemoteHttpTime));
    compile_object(OBJECT_PATH(HttpField));
    compile_object(OBJECT_PATH(RemoteHttpFields));
}
