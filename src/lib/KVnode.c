# include <BTree.h>

inherit BTnode;


static string nodePath();

/*
 * create a new KVnode
 */
static object newNode(string accessKey, int maxSize, string *keys,
		      mixed *values)
{
    return clone_object(nodePath(), accessKey, maxSize, keys, values);
}

/*
 * remove the current node
 */
static void delNode()
{
    destruct_object(this_object());
}
