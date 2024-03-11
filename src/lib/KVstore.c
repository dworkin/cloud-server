# include <Iterator.h>
# include <KVstore.h>
# include <type.h>

inherit Iterable;
private inherit "/lib/util/random";


private string accessKey;	/* KVstore access key */
private object root;		/* KVstore root object */
private int changes;		/* change counter */

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
    changes++;
}

/*
 * remove a KVstore, deleting all nodes
 */
atomic void remove()
{
    root->remove(accessKey);
}


/*
 * first item
 */
private mixed **stackFirst()
{
    mixed *ref, **stack;

    ref = root->nextIndex(accessKey, 0);
    if (ref[2] == nil) {
	return nil;
    }

    for (stack = ({ ref }); !ref[1]; stack = ({ ref }) + stack) {
	ref = ref[2]->nextIndex(accessKey, 0);
    }

    return stack;
}

/*
 * find item by key
 */
private mixed **stackKey(string key)
{
    mixed *ref, **stack;

    ref = root->nextKey(accessKey, key);
    if (ref[2] == nil) {
	return nil;
    }

    for (stack = ({ ref }); !ref[1]; stack = ({ ref }) + stack) {
	ref = ref[2]->nextKey(accessKey, key);
    }

    return stack;
}

/*
 * next item
 */
private mixed **stackNext(mixed **stack)
{
    mixed *ref;
    object node;

    while (sizeof(stack) != 0) {
	ref = stack[0];
	stack = stack[1 ..];

	node = (sizeof(stack) != 0) ? stack[0][2] : root;
	ref = node->nextIndex(accessKey, ref[0] + 1);
	if (ref[2]) {
	    for (;;) {
		stack = ({ ref }) + stack;
		if (typeof(ref[2]) != T_OBJECT ||
		    sscanf(object_name(ref[2]), "%*s#-1") != 0) {
		    /* leaf */
		    return stack;
		}
		ref = ref[2]->nextIndex(accessKey, 0);
	    }
	}
    }
}

/*
 * previous item
 */
private mixed **stackPrev(mixed **stack)
{
    mixed *ref;
    object node;

    while (sizeof(stack) != 0) {
	ref = stack[0];
	stack = stack[1 ..];

	if (ref[0] != 0) {
	    node = (sizeof(stack) != 0) ? stack[0][2] : root;
	    ref = node->nextIndex(accessKey, ref[0] - 1);
	    for (;;) {
		stack = ({ ref }) + stack;
		if (typeof(ref[2]) != T_OBJECT ||
		    sscanf(object_name(ref[2]), "%*s#-1") != 0) {
		    /* leaf */
		    return stack;
		}
		ref = ref[2]->last(accessKey);
	    }
	}
    }
}

/*
 * reset an iterator
 */
mixed iteratorStart(mixed from, mixed to)
{
    mixed **stack;

    if (from && to && from > to) {
	/*
	 * backwards
	 */
	stack = stackKey(from);
	if (stack && stack[0][1] != from) {
	    stack = stackPrev(stack);
	}
	return ({ stack, to, changes, TRUE });
    }

    return ({ (from) ? stackKey(from) : stackFirst(), to, changes, FALSE });
}

/*
 * return the next state for an iterator
 */
mixed *iteratorNext(mixed state)
{
    mixed **stack, value;
    string key, last;
    int count, reverse;

    ({ stack, last, count, reverse }) = state;
    state[0] = nil;
    if (!stack) {
	return ({ state, nil });
    }
    if (count != changes) {
	/*
	 * reconstruct stack from key
	 */
	key = stack[0][1];
	stack = stackKey(key);
	if (stack && reverse && key != stack[0][1]) {
	    stack = stackPrev(stack);
	}
	if (!stack) {
	    return ({ state, nil });
	}
    }
    key = stack[0][1];
    value = stack[0][2];

    if (!reverse) {
	if (last && key > last) {
	    return ({ state, nil });
	}
	stack = stackNext(stack);
    } else {
	if (key < last) {
	    return ({ state, nil });
	}
	stack = stackPrev(stack);
    }

    return ({ ({ stack, last, changes, reverse }), ({ key, value }) });
}

/*
 * check if an iterator is at its end
 */
int iteratorEnd(mixed state)
{
    mixed **stack;
    string last, key;
    int count, reverse;

    ({ stack, last, count, reverse }) = state;
    if (!stack) {
	return TRUE;
    }
    key = stack[0][1];

    return (reverse) ? (key < last) : (last && key > last);
}
