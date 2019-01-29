#include <NKlib.h>

inherit Test;

private Random random;
private Dice dice;
private DiceRoller diceRoller;
private Statistics diceDataGenerator;
private Polynomial diceProbability;

private void diceRollShouldRollExpectedValues(void) {
    expectEqual(TEST_LINE, 2, dice->roll());
    expectEqual(TEST_LINE, 4, dice->roll());
    expectEqual(TEST_LINE, 3, dice->roll());
    expectEqual(TEST_LINE, 5, dice->roll());
    expectEqual(TEST_LINE, 1, dice->roll());
    expectEqual(TEST_LINE, 1, dice->roll());

    random->reset();

    expectEqual(TEST_LINE, 2, dice->roll());
    expectEqual(TEST_LINE, 4, dice->roll());
    expectEqual(TEST_LINE, 3, dice->roll());
    expectEqual(TEST_LINE, 5, dice->roll());
    expectEqual(TEST_LINE, 1, dice->roll());
    expectEqual(TEST_LINE, 1, dice->roll());
}

private void diceRollerShouldRollDiceAndNotSortThem(void) {
    float *actual;

    random->reset();
    actual = diceRoller->roll(FALSE);

    expectEqual(TEST_LINE, 3, sizeof(actual));
    expectEqual(TEST_LINE, 2., actual[0]);
    expectEqual(TEST_LINE, 4., actual[1]);
    expectEqual(TEST_LINE, 3., actual[2]);
}

private void diceRollerShouldRollDiceAndSortThem(void) {
    float *actual;

    random->reset();
    actual = diceRoller->roll(TRUE);

    expectEqual(TEST_LINE, 3, sizeof(actual));
    expectEqual(TEST_LINE, 2., actual[0]);
    expectEqual(TEST_LINE, 3., actual[1]);
    expectEqual(TEST_LINE, 4., actual[2]);
}

private void diceRollerShouldRollAndSum(void) {
    float actual;

    random->reset();
    actual = diceRoller->rollAndSum();

    expectEqual(TEST_LINE, 9., actual);
}

private void diceProbabilitiesFor1D4(void) {
    diceProbability = new DiceProbability(1, 4);

    expectEqual(TEST_LINE, "x + x^2 + x^3 + x^4", diceProbability->toString());
}

private void diceProbabilitiesFor2D4(void) {
    diceProbability = new DiceProbability(2, 4);

    expectEqual(TEST_LINE, "x^2 + 2x^3 + 3x^4 + 4x^5 + 3x^6 + 2x^7 + x^8", diceProbability->toString());
}

private void diceProbabilitiesFor1D6(void) {
    diceProbability = new DiceProbability(1, 6);

    expectEqual(TEST_LINE, "x + x^2 + x^3 + x^4 + x^5 + x^6", diceProbability->toString());
}

private void diceProbabilitiesFor2D6(void) {
    diceProbability = new DiceProbability(2, 6);

    expectEqual(TEST_LINE, "x^2 + 2x^3 + 3x^4 + 4x^5 + 5x^6 + 6x^7 + 5x^8 + 4x^9 + 3x^10 + 2x^11 + x^12", diceProbability->toString());
}

private void diceProbabilitiesFor3D6(void) {
    diceProbability = new DiceProbability(3, 6);

    expectEqual(TEST_LINE, "x^3 + 3x^4 + 6x^5 + 10x^6 + 15x^7 + 21x^8 + 25x^9 + 27x^10 + 27x^11 + 25x^12 + 21x^13 + 15x^14 + 10x^15 + 6x^16 + 3x^17 + x^18", diceProbability->toString());
}

private void diceDataGeneratorShouldGenerateData(void) {
    random->reset();

    diceDataGenerator->rollTheDice(3, 6, 0);

    expectEqual(TEST_LINE, 3, diceDataGenerator->size());
    expectEqual(TEST_LINE, 9., diceDataGenerator->get(0));
    expectEqual(TEST_LINE, 7., diceDataGenerator->get(1));
    expectEqual(TEST_LINE, 3., diceDataGenerator->get(2));
}

void runBeforeTests(void) {
    random = new MockRandom(nil, ({ 2, 4, 3, 5, 1 }));
    dice = new Dice(6, random);
    diceRoller = new DiceRoller(3, 6, 0, dice);
    diceDataGenerator = new DiceDataGenerator(3, diceRoller);
}

void runTests(void) {
    diceRollShouldRollExpectedValues();
    diceRollerShouldRollDiceAndNotSortThem();
    diceRollerShouldRollDiceAndSortThem();
    diceRollerShouldRollAndSum();
    diceDataGeneratorShouldGenerateData();
    diceProbabilitiesFor1D4();
    diceProbabilitiesFor2D4();
    diceProbabilitiesFor1D6();
    diceProbabilitiesFor2D6();
    diceProbabilitiesFor3D6();
}