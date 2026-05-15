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
