#include <NKlib.h>

inherit Array;
inherit Terminal;
inherit NK_KFUN;

#define INDEX_SAMPLE 0
#define INDEX_SUM 1
#define INDEX_MEAN 2
#define INDEX_MEDIAN 3
#define INDEX_MIN 4
#define INDEX_MAX 5
#define INDEX_VAR 6
#define INDEX_STDV 7
#define INDEX_COUNT 7

mixed *startTime;
mixed *stopTime;
float min;
float max;
float sum;
float mean;
float median;
float standardDeviation;
float variance;

void sort(void) {
    if (size() < 2) {
        return;
    }
    ::sort();
}

float getMax(void) {
    return max;
}

float getMean(void) {
    return mean;
}

float getMedian(void) {
    return median;
}

float getMin(void) {
    return min;
}

float getStandardDeviation(void) {
    return standardDeviation;
}

float getSum(void) {
    return sum;
}

float getVariance(void) {
    return variance;
}

string toString(void) {
    string *str;
    int pad, sz;
    int t1, t2;
    float m1, m2;

    pad = 9;
    sz = size();
    ({ t1, m1 }) = startTime;
    ({ t2, m2 }) = stopTime;
    m1 = (float) t1 + m1;
    m2 = (float) t2 + m2;

    str = allocate(INDEX_COUNT + 1);
    str[INDEX_SAMPLE] = bold(lalign("Sample:", pad)) + " " + sz;
    str[INDEX_SUM] = bold(lalign("Sum:", pad)) + " " + sum;
    str[INDEX_MEAN] = bold(lalign("Mean:", pad)) + " " + mean;
    str[INDEX_MEDIAN] = bold(lalign("Median:", pad)) + " " + median;
    str[INDEX_MIN] = bold(lalign("Min:", pad)) + " " + min;
    str[INDEX_MAX] = bold(lalign("Max:", pad)) + " " + max;
    str[INDEX_VAR] = bold(lalign("Var:", pad)) + " " + variance;
    str[INDEX_STDV] = bold(lalign("Std Dev:", pad)) + " " + standardDeviation;

    return "\nData stats (computed in " + (m2 - m1) + " seconds):\n" +
           (new Array(str))->reduce(new ArrayToListReducer(), 0, INDEX_COUNT, 1);
}

void setStartTime(void) {
    startTime = millitime();
}

void setStopTime(void) {
    stopTime = millitime();
}

void findTheSumAndMean(void) {
    Iterator iterator;
    int i;

    if (!size()) {
        sum = 0.;
        mean = 0.;
        return;
    }

    if (size() == 1) {
        sum = get(0);
        mean = sum;
        return;
    }

    sum = 0.;
    iterator = new IntIterator(0, size() - 1);
    while (!iterator->end()) {
        sum += get(iterator->next());
    }
    mean = sum / (float) (iterator->current() + 1);
}

void findTheMinAndMaxAndMedian(void) {
    if (!size()) {
        min = 0.;
        max = 0.;
        median = 0.;
        return;
    }

    min = get(0);
    max = get(size() - 1);
    median = new Median()->evaluate(array);
}

void findTheVarianceAndStandardDeviation(void) {
    variance = new Variance()->evaluate(array, mean);
    standardDeviation = new StandardDeviation()->evaluate(variance);
}

float *covarianceAndCorrelationWith(Statistics that) {
    float covariance, correlation;

    covariance = new Covariance()->evaluate(this_object(), that);
    correlation = new Correlation()->evaluate(covariance, this_object(), that);

    return ({ covariance, correlation });
}

int isSubSet(Statistics x) {
    return (new Statistics((this_object() & x)->toArray()))->size() == size();
}

Statistics union(Statistics x) {
    return new Statistics(toArray() | x->toArray());
}

Statistics intersection(Statistics x) {
    return new Statistics(toArray() & x->toArray());
}

Statistics symmetricDifference(Statistics x) {
    return new Statistics(toArray() ^ x->toArray());
}

Statistics difference(Statistics x) {
    return intersection(symmetricDifference(x));
}

static Statistics operator| (Statistics x) {
    return this_object()->union(x);
}

static Statistics operator& (Statistics x) {
    return this_object()->intersection(x);
}

static Statistics operator^ (Statistics x) {
    return this_object()->symmetricDifference(x);
}

static Statistics operator- (Statistics x) {
    return this_object()->difference(x);
}

void save(string fileName) {
    save_object(fileName);
}

int restore(string fileName) {
    return restore_object(fileName);
}

static void create(float *data) {
    ::create(data);
}