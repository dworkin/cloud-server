#include <Test.h>

inherit Function;
inherit NK_KFUN;

private int calledTimes;
private mixed *calledWith;

int getCalledTimes(void) {
    return calledTimes;
}

mixed *getCalledWith(int index) {
    if (index == -1) {
        return calledWith;
    }
    return calledWith[index];
}

mixed evaluate(varargs mixed args...) {
    calledWith += ({ args });
    calledTimes++;
    
    return calledTimes;
}

static void create(void) {
    ::calledTimes = 0 ;
    ::calledWith = ({ });
    ::create();
}