inherit "~/lib/room";

void create()
{
    ::create();
    set_short_desc("Dworkin's workroom");
    set_long_desc("A windowless room, bookshelves on all four walls, a huge rug " +
	     "resembling an Ardebil on the floor. There is a skull on the " +
	     "desk in front of you, and a globe stands beside the desk. To " +
	     "the right is a long, low table covered with books and papers. " +
	     "Every niche in the walls and open space on the shelves is " +
	     "filled with bizarre curios: bones, pottery, tablets, " +
	     "instruments of unknown function.");

    add_detail("window", "No window here.");
    add_detail("windows", "A long and exhaustive search confirms that there are no windows.", 1);
    add_detail("rug", "The rug depicts a familiar Pattern.");
    add_detail("ardebil", "What the hell is an Ardebil? You have no clue.");
    add_detail("skull", "It seems to be amusing itself.");
    add_detail(({ "desk", "table" }),
"It is covered with papers. Scribbled on the papers are titles such\n" +
"as \"Multiverse design\" and \"Draft LPC specification\".");
    add_detail("globe",
"That is not the world as you know it... however, a tiny patch encircled in " +
"red ink seems familiar.");
    add_detail("patch",
"You grab an instrument resembling a telescope from a shelf and focus on the "+
"globe. The magnification is enormous; you can make out a small village " +
"with tiny figures scurrying on the main street past the church, like " +
"upright toads in their bulky armour, staggering under heaps of treasure.",
	     0, 0, "tiny");
    add_detail("books",
"Rather than the wizardry books you expected, you find sentimental novels " +
"of popular romance, notably \"Wuthering Heights\".", 1);
    add_detail("bones",
"A collection of single bones, varying as to size, function and origin, each "+
"carefully preserved.", 1);
    add_detail(({ "shelves", "bookshelves" }), "Just bookshelves.", 1);
    add_detail("tablets",
"The stone, inscribed variety. None of them are in any language known to " +
"you; some have corrections written on them in purple ink.", 1);
    add_detail(({ "pots", "pottery" }),
"Most of the pots serve as sealed containers, with rows of inscriptions " +
"on them, all but the bottom ones struck through: \"wyvern clover\", " +
"\"spare parts\", \"defissurizer\".", 1);
    add_detail("instruments",
"Most of the instruments are for tuning, constructing or testing other " +
"instruments. By the time you have gone through the entire inventory, " +
"you are hopelessly confused as to which is which.", 1);
    add_detail("papers",
"You pick up a sheet, which appears to be page 219 of a tract on how to " +
"Fix Everything Once for All. This particular sheet lists catagories to " +
"be abolished -- or was it your list of favourite things in order of " +
"preference? Sobered and chilled, you return the sheet were you found it.", 1);

    add_neighbour("entrance",
		  "west", 0,
		  1, 1,
		  "A shimmering blue forcefield occupies the only doorway, to the east.",
		  "behind the forcefield");
}
