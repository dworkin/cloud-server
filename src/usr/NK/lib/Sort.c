private mixed *merge(mixed *array, int l, int m, int r) {
    int i, j, k;
    int n1, n2;
    mixed *left, *right;

    n1 = m - l + 1;
    n2 = r - m;
    left = allocate(n1);
    right = allocate(n2);

    for (i = 0; i < n1; i++) {
        left[i] = array[l + i];
    }

    for (j = 0; j < n2; j++) {
        right[j] = array[m + 1 + j];
    }

    i = 0;
    j = 0;
    k = l;

    while (i < n1 && j < n2) {
        if (left[i] <= right[j]) {
            array[k] = left[i];
            i++;
        } else {
            array[k] = right[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        array[k] = left[i];
        i++;
        k++;
    }

    while (j < n2) {
        array[k] = right[j];
        j++;
        k++;
    }

    return array;
}

private mixed *mergeSort(mixed *array, int l, int r) {
    int m;

    if (l < r) {
        m = l + (r - l) / 2;
        array = mergeSort(array, l, m);
        array = mergeSort(array, m + 1, r);
        array = merge(array, l, m, r);
    }

    return array;
}

mixed *sort(mixed *array, varargs int l, int r) {
    if (!l) {
        l = 0;
    }
    if (!r) {
        r = sizeof(array) - 1;
    }
    return mergeSort(array, l, r);
}
