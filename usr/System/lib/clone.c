/*
 * This object is inherited by all clones.  It provides the functions for
 * keeping a linked list of clones.
 */

# include <kernel/kernel.h>

inherit "auto";

# define SYSTEM_AUTO	"/usr/System/lib/auto"


private object prev, next;	/* previous and next in clone list */

/*
 * NAME:	_F_prev_clone()
 * DESCRIPTION:	set the previous clone in the linked list
 */
nomask void _F_prev_clone(object obj)
{
    if (previous_program() == SYSTEM_AUTO) {
	prev = obj;
    }
}

/*
 * NAME:	_F_next_clone()
 * DESCRIPTION:	set the next clone in the linked list
 */
nomask void _F_next_clone(object obj)
{
    if (previous_program() == SYSTEM_AUTO) {
	next = obj;
    }
}

/*
 * NAME:	_Q_prev_clone()
 * DESCRIPTION:	return the previous clone in the linked list
 */
nomask object _Q_prev_clone()
{
    if (SYSTEM()) {
	return prev;
    }
}

/*
 * NAME:	_Q_next_clone()
 * DESCRIPTION:	return the next clone in the linked list
 */
nomask object _Q_next_clone()
{
    if (SYSTEM()) {
	return next;
    }
}
