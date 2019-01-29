#include <NKlib.h>

inherit Function;

private int arithmeticOperation;

mixed evaluate(varargs mixed args...) {
    mixed nextValue, currentValue;

    ({ nextValue, currentValue }) = args;

    switch (arithmeticOperation) {
        case ARRAY_ARITHMETIC_ADD:
            return currentValue + nextValue;
        case ARRAY_ARITHMETIC_SUBTRACT:
            return currentValue - nextValue;
        case ARRAY_ARITHMETIC_MULTIPLY:
            return currentValue * nextValue;
        case ARRAY_ARITHMETIC_DIVIDE:
            return currentValue / nextValue;
        default:
            error("ArithmeticReducer: uUnknown arithmetic operation: " + arithmeticOperation);
    }
}

static void create(int arithmeticOperation) {
    if (arithmeticOperation < ARRAY_ARITHMETIC_ADD || arithmeticOperation > ARRAY_ARITHMETIC_DIVIDE) {
        error("ArithmeticReducer: invalid arithmetic operation: " + arithmeticOperation);
    }

    ::arithmeticOperation = arithmeticOperation;
}
