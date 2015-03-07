static void create()
{
    compile_object("sys/userd");
    compile_object("sys/cmdparser");
    compile_object("sys/sould");
    compile_object("obj/user");
    compile_object("obj/body/human");
    compile_object("room/entrance");
    compile_object("room/workroom");
}
