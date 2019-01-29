#include <NKlib.h>

inherit Test;

private Function f;
private Integrator i;

private void integrateExponentialProbabilityDensityFunction(void) {
    f = new ExponentialPDF(20.);
    i = new SimpsonsRuleIntegrator(f);

    expectEqual(TEST_LINE, 0.03992997, i->integrate(4., 5.));
}

private void integratePolynomial(void) {
    f = new Polynomial(({ 0., 6., 12. }));
    i = new SimpsonsRuleIntegrator(f);

    expectEqual(TEST_LINE, 271., i->integrate(4., 5.));
}

void runTests(void) {
    integrateExponentialProbabilityDensityFunction();
    integratePolynomial();
}
