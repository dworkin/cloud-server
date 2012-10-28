/*
 * NAME:	prev_obj()
 * DESCRIPTION:	return previous object in clone list
 */
static object prev_obj(object obj)
{
    string oname;

    oname = object_name(obj);
    if (sscanf(oname, "/usr/" + query_owner() + "/%*s") != 0 &&
	sscanf(oname, "%*s/obj/") != 0) {
	return obj->_Q_prev_clone();
    }
}

/*
 * NAME:	next_obj()
 * DESCRIPTION:	return next object in clone list
 */
static object next_obj(object obj)
{
    string oname;

    oname = object_name(obj);
    if (sscanf(oname, "/usr/" + query_owner() + "/%*s") != 0 &&
	sscanf(oname, "%*s/obj/") != 0) {
	return obj->_Q_next_clone();
    }
}
