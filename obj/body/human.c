inherit "/lib/body/human";

static void create(int clone)
{
    if (clone) {
	::create();
    }
}
