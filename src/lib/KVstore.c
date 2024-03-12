# include <Iterator.h>
# include <KVstore.h>
# include <type.h>

inherit Iterable;
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


/*
 * first item
 */
private mixed **stackFirst()
{
    mixed *ref, **stack;

    ref = root->refIndex(accessKey, -1, 0);
    if (!ref) {
	return nil;		/* empty root */
    }

    for (stack = ({ ref }); !ref[1]; stack = ({ ref }) + stack) {
	ref = ref[2]->refIndex(accessKey, -1, 0);
    }

    return stack;
}

/*
 * find item by key
 */
private mixed **stackKey(string key)
{
    mixed *ref, **stack;

    ref = root->refKey(accessKey, key);
    if (!ref) {
	return nil;		/* empty root */
    }

    for (stack = ({ ref }); !ref[1]; stack = ({ ref }) + stack) {
	ref = ref[2]->refKey(accessKey, key);
    }

    return stack;
}

/*
 * next item
 */
private mixed **stackNext(mixed **stack)
{
    string key;
    mixed *ref;
    object node;

    key = stack[0][1];

    do {
	ref = stack[0];
	stack = stack[1 ..];

	if (!(node=(sizeof(stack) != 0) ? stack[0][2] : root) ||
	    !(ref=node->refIndex(accessKey, ref[3], ref[0] + 1))) {
	    /*
	     * reference outdated: fall back to search by key
	     */
	    stack = stackKey(key);
	    if (stack && stack[0][1] == key) {
		continue;
	    }
	    return stack;
	}

	if (ref[2] != nil) {
	    /*
	     * not out of range
	     */
	    for (;;) {
		stack = ({ ref }) + stack;
		if (typeof(ref[2]) != T_OBJECT ||
		    sscanf(object_name(ref[2]), "%*s#-1") != 0) {
		    /* leaf */
		    return stack;
		}
		ref = ref[2]->refIndex(accessKey, -1, 0);
	    }
	}
    } while (sizeof(stack) != 0);
}

/*
 * previous item
 */
private mixed **stackPrev(mixed **stack)
{
    string key;
    mixed *ref;
    object node;

    key = stack[0][1];

    do {
	ref = stack[0];
	stack = stack[1 ..];

	if (!(node=(sizeof(stack) != 0) ? stack[0][2] : root) ||
	    !(ref=node->refIndex(accessKey, ref[3], ref[0] - 1))) {
	    /*
	     * reference outdated: fall back to search by key
	     */
	    stack = stackKey(key);
	    if (stack) {
		continue;
	    }
	    return nil;
	}

	if (ref[2] != nil) {
	    /*
	     * not out of range
	     */
	    for (;;) {
		stack = ({ ref }) + stack;
		if (typeof(ref[2]) != T_OBJECT ||
		    sscanf(object_name(ref[2]), "%*s#-1") != 0) {
		    /* leaf */
		    return stack;
		}
		ref = ref[2]->refLast(accessKey);
	    }
	}
    } while (sizeof(stack) != 0);
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
	if (stack && (stack[0][1] != from || stack[0][2] == nil)) {
	    stack = stackPrev(stack);
	}
	return ({ stack, to, TRUE });
    }

    return ({ (from) ? stackKey(from) : stackFirst(), to, FALSE });
}

/*
 * return the next state for an iterator
 */
mixed *iteratorNext(mixed state)
{
    mixed **stack, value;
    string key, last;
    int reverse;

    ({ stack, last, reverse }) = state;
    state[0] = nil;
    if (!stack) {
	return ({ state, nil });
    }
    key = stack[0][1];
    value = stack[0][2];
    if (value == nil) {
	return ({ state, nil });
    }

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

    return ({ ({ stack, last, reverse }), ({ key, value }) });
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
