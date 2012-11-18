inherit base	"/lib/base/container";
inherit obj	"/lib/object";


static void create()
{
    base::create();
    obj::create();
}

/*
 * NAME:	enter_inv()
 * DESCRIPTION:	object is about to enter inventory
 */
void enter_inv(object obj, object from)
{
}

/*
 * NAME:	leave_inv()
 * DESCRIPTION:	object has left inventory
 */
void leave_inv(object obj, object to)
{
}
