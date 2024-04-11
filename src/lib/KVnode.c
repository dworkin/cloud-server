# include <KVstore.h>


/*
 * The KVstore is implemented as a B+ tree.
 */

# define FLAT		0	/* root node without children */
# define LEAF		1	/* leaf node */
# define INTERIOR	2	/* interior node */
# define ROOT		3	/* root node with children */

# define SET		0	/* set operation successful */
# define SPLIT		1	/* insert caused split */
# define BORDER_LEFT	2	/* delete caused left key change */
# define BORDER_RIGHT	3	/* delete caused right key change */
# define MERGE_LEFT	4	/* delete caused merge to left */
# define MERGE_RIGHT	5	/* delete caused merge to right */

private string accessKey;	/* access key */
private int type;		/* root, interior, leaf */
private int minSize, maxSize;	/* minimum and maximum size */
private string *keys;		/* keys */
private mixed *values;		/* values or nodes */
private int changes;		/* structural change counter */

static string nodePath();

/*
 * initialize node
 */
static void create(string accessKey, int maxSize,
		   varargs string *keys, mixed *values)
{
    ::accessKey = accessKey;
    ::maxSize = maxSize;
    minSize = maxSize / 2;
    if (keys) {
	type = (sizeof(keys) < sizeof(values)) ? INTERIOR : LEAF;
	::keys = keys;
	::values = values;
    } else {
	type = FLAT;
	::keys = ({ });
	::values = ({ });
    }
}

/*
 * steal K/V from the left side, if available
 */
mixed *stealLeft(string accessKey)
{
    if (accessKey == ::accessKey) {
	string key;
	mixed value;

	if (sizeof(keys) == minSize) {
	    return ({ nil, nil, nil });
	}

	key = keys[0];
	value = values[0];
	keys = keys[1 ..];
	values = values[1 ..];
	changes++;

	return ({ key, value });
    }
}

/*
 * steal K/V from the right side, if available
 */
mixed *stealRight(string accessKey)
{
    if (accessKey == ::accessKey) {
	int size;
	string key;
	mixed value;

	size = sizeof(keys);
	if (size == minSize) {
	    return ({ nil, nil });
	}

	key = keys[size - 1];
	keys = keys[.. size - 2];
	size = sizeof(values);
	value = values[size - 1];
	values = values[ .. size - 2];
	changes++;

	return ({ key, value });
    }
}

/*
 * add K/Vs on the left side
 */
void addLeft(string accessKey, string *keys, mixed *values)
{
    if (accessKey == ::accessKey) {
	::keys = keys + ::keys;
	::values = values + ::values;
	changes++;
    }
}

/*
 * add K/Vs on the right side
 */
void addRight(string accessKey, string *keys, mixed *values)
{
    if (accessKey == ::accessKey) {
	::keys += keys;
	::values += values;
	changes++;
    }
}

/*
 * delete this node and return contents
 */
mixed *delete(string accessKey)
{
    if (accessKey == ::accessKey) {
	destruct_object(this_object());
	return ({ keys, values });
    }
}

/*
 * search among the keys
 */
private int *search(string str)
{
    int low, high, mid;
    string key;

    low = 0;
    high = sizeof(keys);
    while (low < high) {
	mid = (low + high) / 2;
	key = keys[mid];
	if (str >= key) {
	    if (str == key) {
		return ({ mid, TRUE });
	    }
	    low = mid + 1;
	} else {
	    high = mid;
	}
    }

    return ({ low, FALSE });
}

/*
 * increase the size
 */
private mixed *grow(int index, object prev, string prevKey, string nextKey,
		    object next)
{
    string key;
    mixed value;

    if (prev) {
	({ key, value }) = prev->stealRight(accessKey);
	if (key) {
	    keys = ({ (prevKey) ? prevKey : key }) + keys;
	    values = ({ value }) + values;
	    return ({ BORDER_LEFT, key, nil });
	} else if (next) {
	    ({ key, value }) = next->stealLeft(accessKey);
	    if (key) {
		keys += ({ (nextKey) ? nextKey : key });
		values += ({ value });
		return ({ BORDER_RIGHT, key, nil });
	    }
	}

	/*
	 * merge with prev
	 */
	if (prevKey) {
	    keys = ({ prevKey }) + keys;
	}
	prev->addRight(accessKey, keys, values);
	destruct_object(this_object());
	return ({ MERGE_LEFT, nil, nil });
    } else {
	({ key, value }) = next->stealLeft(accessKey);
	if (key) {
	    keys += ({ (nextKey) ? nextKey : key });
	    values += ({ value });
	    return ({ BORDER_RIGHT, key, nil });
	}

	/*
	 * merge with next
	 */
	if (nextKey) {
	    keys += ({ nextKey });
	}
	next->addLeft(accessKey, keys, values);
	destruct_object(this_object());
	return ({ MERGE_RIGHT, nil, nil });
    }
}

/*
 * determine keyValue
 */
static mixed keyValue(string key, mixed value)
{
    return value;
}

/*
 * get V for K
 */
mixed get(string accessKey, string key)
{
    if (accessKey == ::accessKey) {
	int index, found;

	({ index, found }) = search(key);
	if (type <= LEAF) {
	    return (found) ? keyValue(key, values[index]) : nil;
	} else {
	    return values[index + found]->get(accessKey, key);
	}
    }
}

/*
 * set the V for a K, nil for deletion
 */
mixed *set(string accessKey, string key, mixed value, int exists,
	   object prev, string prevKey, string nextKey, object next)
{
    if (accessKey == ::accessKey) {
	int index, found, state;

	({ index, found }) = search(key);
	if (type <= LEAF) {
	    if (value != nil) {
		if (found) {
		    /*
		     * set value in FLAT/LEAF
		     */
		    if (exists < 0 && keyValue(key, values[index]) != nil) {
			error("Key present");
		    }
		    values[index] = value;
		} else if (exists > 0) {
		    error("Key not present");
		} else if (index > 0 &&
			   keyValue(keys[index - 1], values[index - 1]) == nil)
		{
		    /*
		     * replace preceding K/V
		     */
		    keys[index - 1] = key;
		    values[index - 1] = value;
		} else if (index < sizeof(keys) &&
			   keyValue(keys[index], values[index]) == nil) {
		    /*
		     * replace following K/V
		     */
		    keys[index] = key;
		    values[index] = value;
		} else {
		    /*
		     * insert key/value in FLAT/LEAF
		     */
		    keys = keys[.. index - 1] + ({ key }) + keys[index ..];
		    values = values[.. index - 1] + ({ value }) +
			     values[index ..];
		    changes++;
		    if (sizeof(keys) > maxSize) {
			if (keyValue(keys[maxSize], values[maxSize]) == nil) {
			    /*
			     * discard last K/V
			     */
			    keys = keys[.. maxSize - 1];
			    values = values[.. maxSize - 1];
			} else if (keyValue(keys[0], values[0]) == nil) {
			    /*
			     * discard first K/V
			     */
			    keys = keys[1 ..];
			    values = values[1 ..];
			} else {
			    /*
			     * split
			     */
			    index = minSize + (index < minSize);
			    key = keys[index];
			    value = clone_object(nodePath(), accessKey, maxSize,
						 keys[index ..],
						 values[index ..]);
			    keys = keys[.. index - 1];
			    values = values[.. index - 1];

			    if (type == LEAF) {
				return ({ SPLIT, key, value });
			    }

			    /*
			     * convert FLAT to ROOT
			     */
			    type = ROOT;
			    values = ({
				clone_object(nodePath(), accessKey, maxSize,
					     keys, values),
				value
			    });
			    keys = ({ key });
			}
		    }
		}
	    } else if (found) {
		/*
		 * delete value in FLAT/LEAF
		 */
		if (exists < 0) {
		    if (keyValue(key, values[index]) != nil) {
			error("Key present");
		    }
		} else if (exists > 0 && keyValue(key, values[index]) == nil) {
		    error("Key not present");
		}
		keys = keys[.. index - 1] + keys[index + 1 ..];
		values = values[.. index - 1] + values[index + 1 ..];
		changes++;
		if (type != FLAT && sizeof(keys) < minSize) {
		    return grow(index, prev, nil, nil, next);
		}
	    } else if (exists > 0) {
		error("Key not present");
	    }
	} else {
	    string lKey, rKey;
	    object lValue, rValue;

	    if (found) {
		index++;
	    }
	    if (index != 0) {
		lKey = keys[index - 1];
		lValue = values[index - 1];
	    }
	    if (index < sizeof(keys)) {
		rKey = keys[index];
		rValue = values[index + 1];
	    }
	    ({
		state,
		key,
		value
	    }) = values[index]->set(accessKey, key, value, exists, lValue, lKey,
				    rKey, rValue);
	    switch (state) {
	    case SPLIT:
		/*
		 * insertion caused split
		 */
		keys = keys[.. index - 1] + ({ key }) + keys[index ..];
		values = values[.. index] + ({ value }) + values[index + 1 ..];
		changes++;
		if (sizeof(keys) > maxSize) {
		    index = minSize;
		    key = keys[index];
		    value = clone_object(nodePath(), accessKey, maxSize,
					 keys[index + 1 ..],
					 values[index + 1 ..]);
		    keys = keys[.. index - 1];
		    values = values[.. index];

		    if (type == INTERIOR) {
			return ({ SPLIT, key, value });
		    }

		    /*
		     * add new ROOT level
		     */
		    values = ({
			clone_object(nodePath(), accessKey, maxSize, keys,
				     values),
			value
		    });
		    keys = ({ key });
		}
		break;

	    case BORDER_LEFT:
		/*
		 * deletion caused theft
		 */
		keys[index - 1] = key;
		changes++;
		break;

	    case BORDER_RIGHT:
		/*
		 * deletion caused theft
		 */
		keys[index] = key;
		changes++;
		break;

	    case MERGE_LEFT:
		/*
		 * deletion caused merge
		 */
		keys = keys[.. index - 2] + keys[index ..];
		values = values[.. index - 1] + values[index + 1 ..];
		changes++;
		if (type == INTERIOR) {
		    if (sizeof(keys) < minSize) {
			return grow(index, prev, prevKey, nextKey, next);
		    }
		} else if (sizeof(keys) == 0) {
		    ({ keys, values }) = values[0]->delete(accessKey);
		    if (sizeof(keys) == sizeof(values)) {
			type = FLAT;
		    }
		}
		break;

	    case MERGE_RIGHT:
		/*
		 * deletion caused merge
		 */
		keys = keys[.. index - 1] + keys[index + 1 ..];
		values = values[.. index - 1] + values[index + 1 ..];
		changes++;
		if (type == INTERIOR) {
		    if (sizeof(keys) < minSize) {
			return grow(index, prev, prevKey, nextKey, next);
		    }
		} else if (sizeof(keys) == 0) {
		    ({ keys, values }) = values[0]->delete(accessKey);
		    if (sizeof(keys) == sizeof(values)) {
			type = FLAT;
		    }
		}
		break;
	    }
	}
    }

    return ({ SET, nil, nil });
}

/*
 * remove entire tree
 */
void remove(string accessKey)
{
    if (accessKey == ::accessKey) {
	if (type >= INTERIOR) {
	    int n;

	    for (n = sizeof(values); --n >= 0; ) {
		values[n]->remove(accessKey);
	    }
	}
	destruct_object(this_object());
    }
}

/*
 * last K/V
 */
mixed *refLast(string accessKey)
{
    if (accessKey == ::accessKey) {
	int last;

	last = sizeof(values) - 1;
	if (last >= 0) {
	    if (type >= INTERIOR) {
		return ({ last, nil, values[last], changes });
	    } else {
		return ({
		    last,
		    keys[last],
		    keyValue(keys[last], values[last]),
		    changes
		});
	    }
	}
    }
}

/*
 * find by index
 */
mixed *refIndex(string accessKey, int changes, int index)
{
    if (accessKey == ::accessKey && (changes == ::changes || changes < 0)) {
	if (index >= 0 && index < sizeof(values)) {
	    if (type >= INTERIOR) {
		return ({ index, nil, values[index], ::changes });
	    } else {
		return ({
		    index,
		    keys[index],
		    keyValue(keys[index], values[index]),
		    ::changes
		});
	    }
	} else {
	    return ({ index, nil, nil, ::changes });	/* out of range */
	}
    }
}

/*
 * find by key
 */
mixed *refKey(string accessKey, string key)
{
    if (accessKey == ::accessKey) {
	int index, found;

	({ index, found }) = search(key);
	if (type >= INTERIOR) {
	    index += found;
	    return ({ index, nil, values[index], changes });
	} else if (index < sizeof(values)) {
	    return ({
		index,
		keys[index],
		keyValue(keys[index], values[index]),
		changes
	    });
	} else {
	    return ({ index, nil, nil, changes });	/* out of range */
	}
    }
}
