#include <NKlib.h>

inherit Test;

private Function func;
private Statistics a;
private Statistics bSorted;
private Statistics b;
private Statistics c;
private Statistics d;
private Statistics empty;
private Statistics one;

private void findTheSumAndMeanShouldSetExpectedValues(void) {
    a->findTheSumAndMean();
    b->findTheSumAndMean();
    empty->findTheSumAndMean();
    one->findTheSumAndMean();

    expectEqual(TEST_LINE, 21., a->getSum());
    expectEqual(TEST_LINE, 21., b->getSum());
    expectEqual(TEST_LINE, 42., one->getSum());
    expectEqual(TEST_LINE, 0., empty->getSum());
    expectEqual(TEST_LINE, 3.5, a->getMean());
    expectEqual(TEST_LINE, 3.5, b->getMean());
    expectEqual(TEST_LINE, 42., one->getMean());
    expectEqual(TEST_LINE, 0., empty->getMean());
}

private void sortShouldSortTheData(void) {
    bSorted->sort();

    expectEqual(TEST_LINE, 1., bSorted->get(0));
    expectEqual(TEST_LINE, 2., bSorted->get(1));
    expectEqual(TEST_LINE, 3., bSorted->get(2));
    expectEqual(TEST_LINE, 4., bSorted->get(3));
    expectEqual(TEST_LINE, 5., bSorted->get(4));
    expectEqual(TEST_LINE, 6., bSorted->get(5));
}

private void findTheMinAndMaxAndMedianShouldFindExpectedValues(void) {
    a->findTheMinAndMaxAndMedian();
    b->findTheMinAndMaxAndMedian();
    bSorted->findTheMinAndMaxAndMedian();
    empty->findTheMinAndMaxAndMedian();

    expectEqual(TEST_LINE, "Median: the sample size must be greater than unity.",
            catch(one->findTheMinAndMaxAndMedian()));

    expectEqual(TEST_LINE, 1., a->getMin());
    expectEqual(TEST_LINE, 6., a->getMax());
    expectEqual(TEST_LINE, 3.5, a->getMedian());

    expectEqual(TEST_LINE, 6., b->getMin());
    expectEqual(TEST_LINE, 1., b->getMax());
    expectEqual(TEST_LINE, 3.5, b->getMedian());

    expectEqual(TEST_LINE, 1., bSorted->getMin());
    expectEqual(TEST_LINE, 6., bSorted->getMax());
    expectEqual(TEST_LINE, 3.5, bSorted->getMedian());

    expectEqual(TEST_LINE, 42., one->getMin());
    expectEqual(TEST_LINE, 42., one->getMax());

    expectEqual(TEST_LINE, 0., empty->getMin());
    expectEqual(TEST_LINE, 0., empty->getMax());
    expectEqual(TEST_LINE, 0., empty->getMedian());
}

private void findTheVarianceAndStandardDeviationShouldFindExpectedValues(void) {
    a->findTheVarianceAndStandardDeviation();
    b->findTheVarianceAndStandardDeviation();

    expectEqual(TEST_LINE, "Variance: sample size must be larger than unity.",
            catch(one->findTheVarianceAndStandardDeviation()));
    expectEqual(TEST_LINE, "Variance: sample size must be larger than unity.",
            catch(empty->findTheVarianceAndStandardDeviation()));

    expectEqual(TEST_LINE, 3.5, a->getVariance());
    expectEqual(TEST_LINE, 1.87082869, a->getStandardDeviation());

    expectEqual(TEST_LINE, 3.5, b->getVariance());
    expectEqual(TEST_LINE, 1.87082869, b->getStandardDeviation());

    expectEqual(TEST_LINE, 0., one->getVariance());
    expectEqual(TEST_LINE, 0., one->getStandardDeviation());

    expectEqual(TEST_LINE, 0., empty->getVariance());
    expectEqual(TEST_LINE, 0., empty->getStandardDeviation());
}

private void correlationShouldReturnExpectedCorrelation(void) {
    func = new Correlation();

    expectEqual(TEST_LINE, -1., func->evaluate(-3.5, a, b));
    expectEqual(TEST_LINE, "Correlation: zero standard deviation: 0, 1.87082869",
        catch(func->evaluate(-3.5, one, b)));
    expectEqual(TEST_LINE, "Correlation: zero standard deviation: 1.87082869, 0",
        catch(func->evaluate(-3.5, a, one)));
    expectEqual(TEST_LINE, "Correlation: zero standard deviation: 0, 1.87082869",
        catch(func->evaluate(-3.5, empty, b)));
    expectEqual(TEST_LINE, "Correlation: zero standard deviation: 1.87082869, 0",
        catch(func->evaluate(-3.5, a, empty)));
}

private void covarianceShouldReturnExpectedCovariance(void) {
    func = new Covariance();

    expectEqual(TEST_LINE, -3.5, func->evaluate(a, b));
    expectEqual(TEST_LINE, "Covariance: the sample sizes do not match.", catch(func->evaluate(one, b)));
    expectEqual(TEST_LINE, "Covariance: the sample sizes do not match.", catch(func->evaluate(empty, b)));
    expectEqual(TEST_LINE, "Covariance: the sample size must be greater than unity.",
            catch(func->evaluate(one, one)));
    expectEqual(TEST_LINE, "Covariance: the sample size must be greater than unity.",
            catch(func->evaluate(empty, empty)));
}

private void exponentialPDFShouldReturnExpectedValue(void) {
    func = new ExponentialPDF(20.);

    expectEqual(TEST_LINE, 0.0452418709, func->evaluate(2.));
    expectEqual(TEST_LINE, "ExponentialPDF: division by zero.", catch(new ExponentialPDF(0.)));
    expectEqual(TEST_LINE, "ExponentialPDF: division by zero.", catch(func->apply(0.)));
}

private void medianShouldReturnExpectedMedian(void) {
    func = new Median();

    expectEqual(TEST_LINE, 3.5, func->evaluate(a->toArray()));
    expectEqual(TEST_LINE, 3.5, func->evaluate(b->toArray()));
    expectEqual(TEST_LINE, "Median: the sample size must be greater than unity.",
            catch(func->evaluate(one->toArray())));
    expectEqual(TEST_LINE, "Median: the sample size must be greater than unity.",
            catch(func->evaluate(empty->toArray())));
}

private void randomShouldReturnStringOfExpectedLength(void) {
    expectEqual(TEST_LINE, 42, strlen(new Random()->randomString(42)));
    expectEqual(TEST_LINE, "Random: cannot generate a random string with size less than unity.",
            catch(new Random()->randomString(0)));
    expectEqual(TEST_LINE, "Random: cannot generate a random string with size less than unity.",
            catch(new Random()->randomString(-1)));
}

private void standardDeviationShouldReturnExpectedStandardDeviation(void) {
    func = new StandardDeviation();

    expectEqual(TEST_LINE, 2., func->evaluate(4.));
    expectEqual(TEST_LINE, 0., func->evaluate(0.));
    expectEqual(TEST_LINE, "StandardDeviation: cannot evaluate sqrt of negative number -4", catch(func->evaluate(-4.)));
}

private void varianceShouldReturnExpectedVariance(void) {
    func = new Variance();

    expectEqual(TEST_LINE, 3.5, func->evaluate(a->toArray(), a->getMean()));
    expectEqual(TEST_LINE, 3.5, func->evaluate(b->toArray(), b->getMean()));
    expectEqual(TEST_LINE, "Variance: sample size must be larger than unity.",
            catch(func->evaluate(one->toArray(), one->getMean())));
    expectEqual(TEST_LINE, "Variance: sample size must be larger than unity.",
            catch(func->evaluate(empty->toArray(), empty->getMean())));
}

private void covarianceAndCorrelationWithShouldReturnExpectedValues(void) {
    float cov, cor;

    ({ cov, cor }) = a->covarianceAndCorrelationWith(b);
    expectEqual(TEST_LINE, -3.5, cov);
    expectEqual(TEST_LINE, -1., cor);

    ({ cov, cor }) = b->covarianceAndCorrelationWith(a);
    expectEqual(TEST_LINE, -3.5, cov);
    expectEqual(TEST_LINE, -1., cor);

    expectEqual(TEST_LINE, "Covariance: the sample sizes do not match.",
            catch(one->covarianceAndCorrelationWith(empty)));
    expectEqual(TEST_LINE, "Covariance: the sample sizes do not match.",
            catch(empty->covarianceAndCorrelationWith(one)));
    expectEqual(TEST_LINE, "Covariance: the sample size must be greater than unity.",
            catch(one->covarianceAndCorrelationWith(one)));
    expectEqual(TEST_LINE, "Covariance: the sample size must be greater than unity.",
            catch(empty->covarianceAndCorrelationWith(empty)));
}

private void isSubSetShouldIndicateAsExpected(void) {
    expectTrue(TEST_LINE, new Statistics(({ 2., 3. }))->isSubSet(new Statistics(({ 1., 2., 3., 4. }))));
    expectTrue(TEST_LINE, one->isSubSet(one));
    expectTrue(TEST_LINE, empty->isSubSet(empty));
    expectTrue(TEST_LINE, empty->isSubSet(one));
    expectFalse(TEST_LINE, new Statistics(({ 1., 2., 3., 4. }))->isSubSet(new Statistics(({ 2., 3. }))));
    expectFalse(TEST_LINE, one->isSubSet(empty));
}

private void unionShouldCreateUnionOfSets(void) {
    Statistics x;

    x = empty->union(one);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = empty | one;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->union(empty);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one | empty;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->union(one);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one | one;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = empty->union(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty | empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = c->union(d);

    expectEqual(TEST_LINE, 8, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 5., x->get(2));
    expectEqual(TEST_LINE, 6., x->get(3));
    expectEqual(TEST_LINE, 8., x->get(4));
    expectEqual(TEST_LINE, 1., x->get(5));
    expectEqual(TEST_LINE, 3., x->get(6));
    expectEqual(TEST_LINE, 7., x->get(7));

    x = c | d;

    expectEqual(TEST_LINE, 8, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 5., x->get(2));
    expectEqual(TEST_LINE, 6., x->get(3));
    expectEqual(TEST_LINE, 8., x->get(4));
    expectEqual(TEST_LINE, 1., x->get(5));
    expectEqual(TEST_LINE, 3., x->get(6));
    expectEqual(TEST_LINE, 7., x->get(7));

    x = d->union(c);

    expectEqual(TEST_LINE, 8, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 5., x->get(2));
    expectEqual(TEST_LINE, 7., x->get(3));
    expectEqual(TEST_LINE, 8., x->get(4));
    expectEqual(TEST_LINE, 2., x->get(5));
    expectEqual(TEST_LINE, 4., x->get(6));
    expectEqual(TEST_LINE, 6., x->get(7));

    x = d | c;

    expectEqual(TEST_LINE, 8, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 5., x->get(2));
    expectEqual(TEST_LINE, 7., x->get(3));
    expectEqual(TEST_LINE, 8., x->get(4));
    expectEqual(TEST_LINE, 2., x->get(5));
    expectEqual(TEST_LINE, 4., x->get(6));
    expectEqual(TEST_LINE, 6., x->get(7));
}

private void intersectionShouldCreateIntersectionOfSets(void) {
    Statistics x;

    x = empty->intersection(one);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty & one;

    expectEqual(TEST_LINE, 0, x->size());

    x = one->intersection(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = one & empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = one->intersection(one);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one & one;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = empty->intersection(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty & empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = c->intersection(d);

    expectEqual(TEST_LINE, 2, x->size());
    expectEqual(TEST_LINE, 5., x->get(0));
    expectEqual(TEST_LINE, 8., x->get(1));

    x = c & d;

    expectEqual(TEST_LINE, 2, x->size());
    expectEqual(TEST_LINE, 5., x->get(0));
    expectEqual(TEST_LINE, 8., x->get(1));

    x = d->intersection(c);

    expectEqual(TEST_LINE, 2, x->size());
    expectEqual(TEST_LINE, 5., x->get(0));
    expectEqual(TEST_LINE, 8., x->get(1));

    x = c & d;

    expectEqual(TEST_LINE, 2, x->size());
    expectEqual(TEST_LINE, 5., x->get(0));
    expectEqual(TEST_LINE, 8., x->get(1));
}

private void symmetricDifferenceShouldCreateSymDifOfSets(void) {
    Statistics x;

    x = empty->symmetricDifference(one);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = empty ^ one;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->symmetricDifference(empty);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one ^ empty;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->symmetricDifference(one);

    expectEqual(TEST_LINE, 0, x->size());

    x = one ^ one;

    expectEqual(TEST_LINE, 0, x->size());

    x = empty->symmetricDifference(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty ^ empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = c->symmetricDifference(d);

    expectEqual(TEST_LINE, 6, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 6., x->get(2));
    expectEqual(TEST_LINE, 1., x->get(3));
    expectEqual(TEST_LINE, 3., x->get(4));
    expectEqual(TEST_LINE, 7., x->get(5));

    x = c ^ d;

    expectEqual(TEST_LINE, 6, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 6., x->get(2));
    expectEqual(TEST_LINE, 1., x->get(3));
    expectEqual(TEST_LINE, 3., x->get(4));
    expectEqual(TEST_LINE, 7., x->get(5));

    x = d->symmetricDifference(c);

    expectEqual(TEST_LINE, 6, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 7., x->get(2));
    expectEqual(TEST_LINE, 2., x->get(3));
    expectEqual(TEST_LINE, 4., x->get(4));
    expectEqual(TEST_LINE, 6., x->get(5));

    x = d ^ c;

    expectEqual(TEST_LINE, 6, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 7., x->get(2));
    expectEqual(TEST_LINE, 2., x->get(3));
    expectEqual(TEST_LINE, 4., x->get(4));
    expectEqual(TEST_LINE, 6., x->get(5));
}

private void differenceShouldCreateDifferenceOfSets(void) {
    Statistics x;

    x = empty->difference(one);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty - one;

    expectEqual(TEST_LINE, 0, x->size());

    x = one->difference(empty);

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one - empty;

    expectEqual(TEST_LINE, 1, x->size());
    expectEqual(TEST_LINE, 42., x->get(0));

    x = one->difference(one);

    expectEqual(TEST_LINE, 0, x->size());

    x = one - one;

    expectEqual(TEST_LINE, 0, x->size());

    x = empty->difference(empty);

    expectEqual(TEST_LINE, 0, x->size());

    x = empty - empty;

    expectEqual(TEST_LINE, 0, x->size());

    x = c->difference(d);

    expectEqual(TEST_LINE, 3, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 6., x->get(2));

    x = c - d;

    expectEqual(TEST_LINE, 3, x->size());
    expectEqual(TEST_LINE, 2., x->get(0));
    expectEqual(TEST_LINE, 4., x->get(1));
    expectEqual(TEST_LINE, 6., x->get(2));

    x = d->difference(c);

    expectEqual(TEST_LINE, 3, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 7., x->get(2));

    x = d - c;

    expectEqual(TEST_LINE, 3, x->size());
    expectEqual(TEST_LINE, 1., x->get(0));
    expectEqual(TEST_LINE, 3., x->get(1));
    expectEqual(TEST_LINE, 7., x->get(2));
}

private void dataVisitorShouldIndicateExpectedResults(void) {
    DataVisitor visitor;
    mapping results;

    visitor = new DataVisitor();
    expectEqual(TEST_LINE, -1, visitor->evaluate(-1.));
    expectEqual(TEST_LINE, 0, visitor->evaluate(0.));
    expectEqual(TEST_LINE, 1, visitor->evaluate(1.));

    visitor->apply(2.);
    expectEqual(TEST_LINE, -1, visitor->evaluate(1.));
    expectEqual(TEST_LINE, 0, visitor->evaluate(2.));
    expectEqual(TEST_LINE, 1, visitor->evaluate(3.));

    results = visitor->getVisitResults();
    expectEqual(TEST_LINE, 2, results["<"]);
    expectEqual(TEST_LINE, 2, results["="]);
    expectEqual(TEST_LINE, 2, results[">"]);
}

private void geometricMeanShouldComputeCorrectValue(void) {
    GeometricMean gm;
    float actual;

    gm = new GeometricMean();

    actual = gm->evaluate(d->toArray());

    expectEqual(TEST_LINE, 3.84464157, actual);
    expectEqual(TEST_LINE, "GeometricMean: size of sample must be at least unity.",
            catch(gm->evaluate(({ }))));
    expectEqual(TEST_LINE, "GeometricMean: sample of positive different from zero numbers only.",
            catch(gm->evaluate(({ 42., -42. }))));
}

static void runBeforeTests(void) {
    a = new Statistics(({ 1., 2., 3., 4., 5., 6. }));
    b = new Statistics(({ 6., 5., 4., 3., 2., 1. }));
    bSorted = new Statistics(({ 6., 5., 4., 3., 2., 1. }));
    c = new Statistics(({ 2., 4., 5., 6., 8. }));
    d = new Statistics(({ 1., 3., 5., 7., 8. }));
    empty = new Statistics(({ }));
    one = new Statistics(({ 42. }));
}

void runTests(void) {
    findTheSumAndMeanShouldSetExpectedValues();
    sortShouldSortTheData();
    findTheMinAndMaxAndMedianShouldFindExpectedValues();
    findTheVarianceAndStandardDeviationShouldFindExpectedValues();
    correlationShouldReturnExpectedCorrelation();
    covarianceShouldReturnExpectedCovariance();
    exponentialPDFShouldReturnExpectedValue();
    medianShouldReturnExpectedMedian();
    randomShouldReturnStringOfExpectedLength();
    standardDeviationShouldReturnExpectedStandardDeviation();
    varianceShouldReturnExpectedVariance();
    covarianceAndCorrelationWithShouldReturnExpectedValues();
    isSubSetShouldIndicateAsExpected();
    unionShouldCreateUnionOfSets();
    intersectionShouldCreateIntersectionOfSets();
    symmetricDifferenceShouldCreateSymDifOfSets();
    differenceShouldCreateDifferenceOfSets();
    dataVisitorShouldIndicateExpectedResults();
    geometricMeanShouldComputeCorrectValue();
}