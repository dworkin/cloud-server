#include <NKlib.h>

inherit Sort;

private Dice dice;
private int times;
private int keep;

private float *keptRolls(int shouldSortRolls, float *rolls) {
    if (!shouldSortRolls) {
        return sort(rolls)[sizeof(rolls) - keep ..];
    }
    return rolls[sizeof(rolls) - keep ..];
}

void setDice(Dice dice) {
    ::dice = dice;
}

float *roll(varargs int shouldSortRolls) {
    Iterator iterator;
    float *rolls;

    rolls = allocate_float(times);
    iterator = new IntIterator(0, times - 1);
    while (!iterator->end()) {
        rolls[iterator->next()] = (float) dice->roll();
    }

    if (shouldSortRolls) {
        rolls = sort(rolls);
    }

    return keep < times ? keptRolls(shouldSortRolls, rolls) : rolls;
}

float rollAndSum(void) {
    float *rolls;
    float sum;
    Iterator iterator;

    sum = 0.;
    rolls = roll();
    iterator = new IntIterator(0, sizeof(rolls) - 1);
    while (!iterator->end()) {
        sum += rolls[iterator->next()];
    }

    return sum;
}

static void create(int times, int faces, varargs int keep, Dice dice) {
    setDice(dice ? dice : new Dice(faces));
    ::times = times;
    if (!keep) {
        keep = times;
    }
    if (keep > times) {
        error("DiceRoller: the number of dice kept must not exceed the number of dice rolled.");
    }
    ::keep = keep;
}
