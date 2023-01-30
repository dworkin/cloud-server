# define BLOCKQUOTE	"::blockquote"

/*
 * start blockquote level management
 */
static void startLevel()
{
    tls_set(BLOCKQUOTE, 0);
}

/*
 * change the blockquote level
 */
static string setLevel(int level)
{
    string str;
    int blockquote;

    str = "";
    blockquote = tls_get(BLOCKQUOTE);
    if (blockquote != level) {
	if (blockquote < level) {
	    do {
		str += "<BLOCKQUOTE>";
	    } while (++blockquote < level);
	} else {
	    do {
		str += "</BLOCKQUOTE>";
	    } while (--blockquote > level);
	}
	tls_set(BLOCKQUOTE, blockquote);
    }

    return str;
}
