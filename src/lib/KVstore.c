# include <KVstore.h>
# include <type.h>

private inherit "/lib/util/random";


private string accessKey;	/* KVstore access key */
private object root;		/* KVstore root object */

/*
 * create KVstore, including root object
 */
atomic static void create(int maxSize)
{
    accessKey = random_string(32);
    root = clone_object(KVNODE, accessKey, maxSize);
}

/*
 * get the value associated with a key
 */
mixed get(string key)
{
    if (!key) {
	error("Invalid key");
    }
    return root->get(accessKey, key);
}

/*
 * set the value associated with a key, nil for deletion
 */
atomic void set(string key, mixed value)
{
    if (!key) {
	error("Invalid key");
    }
    if (typeof(value) == T_OBJECT && sscanf(object_name(value), "%*s#-1") == 0)
    {
	error("Invalid value");
    }
    root->set(accessKey, key, value, nil, nil, nil, nil);
}

/*
 * remove a KVstore, deleting all nodes
 */
atomic void remove()
{
    root->remove(accessKey);
}
