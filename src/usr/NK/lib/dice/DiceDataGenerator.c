#include <NKlib.h>

inherit Statistics;

private int sampleSize;
private DiceRoller diceRoller;

void setDiceRoller(DiceRoller diceRoller) {
    ::diceRoller = diceRoller;
}

void rollTheDice(int times, int faces, int keep) {
    Function reducer;
    Iterator iterator;

    iterator = new IntIterator(0, sampleSize - 1);
    if (T_NIL == typeof(diceRoller)) {
        diceRoller = new DiceRoller(times, faces, keep);
    }

    reducer = new ArrayArithmeticReducer(ARRAY_ARITHMETIC_ADD);
    while (!iterator->end()) {
        array[iterator->next()] = new Array(diceRoller->roll())->reduce(reducer);
    }
}

static void create(int sampleSize, varargs DiceRoller diceRoller) {
    ::diceRoller = diceRoller;
    ::sampleSize = sampleSize;
    ::create(allocate_float(::sampleSize));
}
