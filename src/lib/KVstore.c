# include <BTree.h>
# include <KVstore.h>

inherit BTree;


/*
 * initialize as a persistent BTree
 */
static object rootNode(string nodePath, string accessKey, int maxSize)
{
    return clone_object((nodePath) ? nodePath : KVNODE, accessKey, maxSize);
}

/*
 * get the value associated with a key
 */
mixed get(string key)
{
    return ::get(key);
}

/*
 * set the value associated with a key, nil for deletion
 */
int set(string key, mixed value)
{
    return ::set(key, value);
}

/*
 * set the value associated with a key, which must not yet exist
 */
int add(string key, mixed value)
{
    return ::add(key, value);
}

/*
 * set the value associated with a key, which must already exist
 */
int change(string key, mixed value)
{
    return ::change(key, value);
}
