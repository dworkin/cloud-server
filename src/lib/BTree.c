# include <Iterator.h>
# include <BTree.h>
# include <type.h>

inherit Iterable;
private inherit "/lib/util/random";


# define CHANGE		3

/* ref = ({ index, key, value, changes }) */
# define INDEX		0
# define KEY		1
# define VALUE		2
# define CHANGES	3

private string accessKey;	/* BTree access key */
private object root;		/* BTree root object */

/*
 * create a root node for the BTree
 */
static object rootNode(string nodePath, string accessKey, int maxSize)
{
    return new_object((nodePath) ? nodePath : OBJECT_PATH(BTnode), accessKey,
		      maxSize);
}

/*
 * create BTree, including root object
 */
atomic static void create(int maxSize, varargs string accessKey,
			  string nodePath)
{
    ::accessKey = (accessKey) ? accessKey : random_string(32);
    root = rootNode(nodePath, ::accessKey, maxSize);
}

/*
 * get the value associated with a key
 */
mixed get(mixed key)
{
    if (key == nil) {
	error("Invalid key");
    }
    return root->get(accessKey, key);
}

/*
 * set the value associated with a key, nil for deletion
 */
atomic int set(mixed key, mixed value)
{
    if (key == nil) {
	error("Invalid key");
    }
    return root->set(accessKey, key, value, 0, nil, nil, nil, nil)[CHANGE];
}

/*
 * set the value associated with a key, which must not yet exist
 */
atomic int add(mixed key, mixed value)
{
    if (key == nil) {
	error("Invalid key");
    }
    return root->set(accessKey, key, value, -1, nil, nil, nil, nil)[CHANGE];
}

/*
 * set the value associated with a key, which must already exist
 */
atomic int change(mixed key, mixed value)
{
    if (key == nil) {
	error("Invalid key");
    }
    return root->set(accessKey, key, value, 1, nil, nil, nil, nil)[CHANGE];
}

/*
 * remove a BTree, including all nodes
 */
atomic void remove()
{
    root->remove(accessKey);
}

/*
 * BTree[index] = value
 */
static void operator[]= (mixed index, mixed value)
{
    set(index, value);
}

/*
 * BTree[index]
 */
static mixed operator[] (mixed index)
{
    return get(index);
}


/*
 * first item
 */
private mixed **stackFirst()
{
    mixed *ref, **stack;

    ref = root->refIndex(accessKey, -1, 0);
    if (ref[VALUE] == nil && ref[KEY] == nil) {
	return nil;		/* empty root */
    }

    for (stack = ({ ref }); ref[KEY] == nil; stack = ({ ref }) + stack) {
	ref = ref[VALUE]->refIndex(accessKey, -1, 0);
    }

    return stack;
}

/*
 * find item by key
 */
private mixed **stackKey(mixed key)
{
    mixed *ref, **stack;

    ref = root->refKey(accessKey, key);
    if (ref[VALUE] == nil && ref[KEY] == nil) {
	return nil;		/* empty root */
    }

    for (stack = ({ ref }); ref[KEY] == nil; stack = ({ ref }) + stack) {
	ref = ref[VALUE]->refKey(accessKey, key);
    }

    return stack;
}

/*
 * next item
 */
private mixed **stackNext(mixed **stack)
{
    mixed key, *ref;
    object node;

    key = stack[0][KEY];

    do {
	ref = stack[0];
	stack = stack[1 ..];

	if (!(node=(sizeof(stack) != 0) ? stack[0][VALUE] : root) ||
	    !(ref=node->refIndex(accessKey, ref[CHANGES], ref[INDEX] + 1))) {
	    /*
	     * reference outdated: fall back to search by key
	     */
	    stack = stackKey(key);
	    if (stack && stack[0][KEY] == key) {
		continue;
	    }
	    return stack;
	}

	if (ref[VALUE] != nil || ref[KEY] != nil) {
	    /*
	     * not out of range
	     */
	    for (;;) {
		stack = ({ ref }) + stack;
		if (ref[KEY] != nil) {
		    /* leaf */
		    return stack;
		}
		ref = ref[VALUE]->refIndex(accessKey, -1, 0);
	    }
	}
    } while (sizeof(stack) != 0);
}

/*
 * previous item
 */
private mixed **stackPrev(mixed **stack)
{
    mixed key, *ref;
    object node;

    key = stack[0][KEY];

    do {
	ref = stack[0];
	stack = stack[1 ..];

	if (!(node=(sizeof(stack) != 0) ? stack[0][VALUE] : root) ||
	    !(ref=node->refIndex(accessKey, ref[CHANGES], ref[INDEX] - 1))) {
	    /*
	     * reference outdated: fall back to search by key
	     */
	    stack = stackKey(key);
	    if (stack) {
		continue;
	    }
	    return nil;
	}

	if (ref[VALUE] != nil || ref[KEY] != nil) {
	    /*
	     * not out of range
	     */
	    for (;;) {
		stack = ({ ref }) + stack;
		if (ref[KEY] != nil) {
		    /* leaf */
		    return stack;
		}
		ref = ref[VALUE]->refLast(accessKey);
	    }
	}
    } while (sizeof(stack) != 0);
}

/*
 * reset an iterator
 */
mixed iteratorStart(mixed first, mixed last)
{
    mixed **stack;

    if (first != nil && last != nil && first > last) {
	/*
	 * backwards
	 */
	stack = stackKey(first);
	if (stack && stack[0][KEY] != first) {
	    stack = stackPrev(stack);
	}
	return ({ stack, last, TRUE });
    }

    return ({ (first != nil) ? stackKey(first) : stackFirst(), last, FALSE });
}

/*
 * return the next state for an iterator
 */
mixed *iteratorNext(mixed state)
{
    mixed **stack, last, key, value;
    int reverse;

    ({ stack, last, reverse }) = state;
    if (stack) {
	key = stack[0][KEY];
	value = stack[0][VALUE];
	if (key != nil) {
	    if (!reverse) {
		if (last == nil || key <= last) {
		    return ({
			({ stackNext(stack), last, reverse }),
			({ key, value })
		    });
		}
	    } else if (key >= last) {
		return ({
		    ({ stackPrev(stack), last, reverse }),
		    ({ key, value })
		});
	    }
	}
    }

    return ({ ({ nil, last, reverse }), ({ nil, nil }) });
}

/*
 * check whether an iterator is at its end
 */
int iteratorEnd(mixed state)
{
    mixed **stack, last, key;
    int reverse;

    ({ stack, last, reverse }) = state;
    if (!stack) {
	return TRUE;
    }
    key = stack[0][KEY];
    if (key == nil) {
	return TRUE;
    }

    return (reverse) ? (key < last) : (last != nil && key > last);
}
