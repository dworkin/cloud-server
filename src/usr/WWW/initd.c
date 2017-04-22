# define SYS_INITD	"/usr/System/initd"

void create()
{
    compile_object("obj/http");
    SYS_INITD->set_connection_manager("binary", 0,
				      compile_object("sys/server"));
}
