# include <Iterator.h>
# include <KVstore.h>
# include <type.h>

inherit Iterable;
private inherit "/lib/util/random";


# define INDEX		0
# define KEY		1
# define VALUE		2
# define CHANGES	3

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
 * KVstore[index] = value
 */
static void operator[]= (string index, mixed value)
{
    set(index, value);
}

/*
 * KVstore[index]
 */
static mixed operator[] (string index)
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
    if (!ref) {
	return nil;		/* empty root */
    }

    for (stack = ({ ref }); !ref[KEY]; stack = ({ ref }) + stack) {
	ref = ref[VALUE]->refIndex(accessKey, -1, 0);
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

    for (stack = ({ ref }); !ref[KEY]; stack = ({ ref }) + stack) {
	ref = ref[VALUE]->refKey(accessKey, key);
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
    mixed value;

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

	value = ref[VALUE];
	if (value != nil) {
	    /*
	     * not out of range
	     */
	    for (;;) {
		stack = ({ ref }) + stack;
		if (typeof(value) != T_OBJECT ||
		    sscanf(object_name(value), "%*s#-1") != 0) {
		    /* leaf */
		    return stack;
		}
		ref = value->refIndex(accessKey, -1, 0);
		value = ref[VALUE];
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
    mixed value;

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

	value = ref[VALUE];
	if (value != nil) {
	    /*
	     * not out of range
	     */
	    for (;;) {
		stack = ({ ref }) + stack;
		if (typeof(value) != T_OBJECT ||
		    sscanf(object_name(value), "%*s#-1") != 0) {
		    /* leaf */
		    return stack;
		}
		ref = value->refLast(accessKey);
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
	if (stack && (stack[0][KEY] != from || stack[0][VALUE] == nil)) {
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
    key = stack[0][KEY];
    value = stack[0][VALUE];
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
    key = stack[0][KEY];

    return (reverse) ? (key < last) : (last && key > last);
}
