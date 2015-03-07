# define MOVABLE	"/usr/Player/lib/base/movable"


private mapping inventory;	/* object -> position */

/*
 * NAME:	create()
 * DESCRIPTION:	initialize
 */
static void create()
{
    inventory = ([ ]);
}

/*
 * NAME:	query_inv()
 * DESCRIPTION:	return inventory of this object
 */
object *query_inv()
{
    return map_indices(inventory);
}

/*
 * NAME:	query_inv_posn()
 * DESCRIPTION:	return position of an object in inventory
 */
mixed query_inv_posn(object obj)
{
    mixed position;

    position = inventory[obj];
    return (position == obj) ? nil : position;
}

/*
 * NAME:	_F_movein()
 * DESCRIPTION:	move object into this object (low level function)
 */
nomask void _F_movein(object obj, mixed position)
{
    if (previous_program() == MOVABLE) {
	inventory[obj] = position;
    }
}

/*
 * NAME:	_F_moveout()
 * DESCRIPTION:	move object out of this object (low level function)
 */
nomask void _F_moveout(object obj)
{
    if (previous_program() == MOVABLE) {
	inventory[obj] = nil;
    }
}
