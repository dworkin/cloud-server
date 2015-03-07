inherit "~/lib/room";

static void create()
{
    ::create();
    set_short_desc("Entrance");
    set_long_desc("A small, cramped room, roughly hewn from rock with little " +
		  "eye for detail.\n");
    add_neighbour("workroom",
		  "east", 0,
		  1, 1,
		  "A small room is visible through a doorway to the west.",
		  "to the west");
}

int move_body(object body, string direction)
{
    if (!body->query_soul()->query_wiztool()) {
	body->message(0, "You are stopped by the field.\n");
	return 0;
    }

    return ::move_body(body, direction);
}
