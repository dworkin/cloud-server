#include <Test.h>

inherit Test;
inherit NK_KFUN;
inherit MOCK_MILLITIME_LIB;

private int time1;
private int time2;
private Time time;

private void timeFromNoArg(void) {
    time = new MockTime();

    expectEqual(TEST_LINE, 50000, time->time());
    expectEqual(TEST_LINE, 0.5, time->mtime());
    expectEqual(TEST_LINE, 50000500., time->millis());
}

private void timeFromTimeMtime(void) {
    time = new Time(42, .42);

    expectEqual(TEST_LINE, 42, time->time());
    expectEqual(TEST_LINE, 0.42, time->mtime());
    expectEqual(TEST_LINE, 42420., time->millis());
}

private void timeFromInvalid(void) {
    expectEqual(TEST_LINE, "Time: time must not be negative", catch(new Time(-1, 1.)));
    expectEqual(TEST_LINE, "Time: mtime must not be negative", catch(new Time(1, -1.)));
    expectEqual(TEST_LINE, "Bad argument 2 (int) for function create", catch(new Time(1, 1)));
    expectEqual(TEST_LINE, "Value is not an int", catch(new Time("string", 1.)));
}

private void timeComparisons(void) {
    expectTrue(TEST_LINE, new Time(time1)->equals(new Time(time1)));
    expectFalse(TEST_LINE, new Time(time1)->equals(new Time(time2)));

    expectTrue(TEST_LINE, new Time(time2)->equals(new Time(time2)));
    expectFalse(TEST_LINE, new Time(time2)->equals(new Time(time1)));

    expectFalse(TEST_LINE, new Time(time1) > new Time(time2));
    expectTrue(TEST_LINE, new Time(time2) > new Time(time1));
    expectFalse(TEST_LINE, new Time(time1) > new Time(time1));
    expectFalse(TEST_LINE, new Time(time2) > new Time(time2));

    expectTrue(TEST_LINE, new Time(time1) < new Time(time2));
    expectFalse(TEST_LINE, new Time(time2) < new Time(time1));
    expectFalse(TEST_LINE, new Time(time1) < new Time(time1));
    expectFalse(TEST_LINE, new Time(time2) < new Time(time2));

    expectFalse(TEST_LINE, new Time(time1) >= new Time(time2));
    expectTrue(TEST_LINE, new Time(time2) >= new Time(time1));
    expectTrue(TEST_LINE, new Time(time1) >= new Time(time1));
    expectTrue(TEST_LINE, new Time(time1 + 1) >= new Time(time1));
    expectFalse(TEST_LINE, new Time(time1) >= new Time(time1 + 1));

    expectTrue(TEST_LINE, new Time(time1) <= new Time(time2));
    expectFalse(TEST_LINE, new Time(time2) <= new Time(time1));
    expectTrue(TEST_LINE, new Time(time1) <= new Time(time1));
    expectFalse(TEST_LINE, new Time(time1 + 1) <= new Time(time1));
    expectTrue(TEST_LINE, new Time(time2) <= new Time(time2 + 1));
}

private void timeGmtime(void) {
    expectEqual(TEST_LINE, "Thu Jan 31 07:37:04 2019", new Time(time1)->gmctime());
    expectEqual(TEST_LINE, "Fri Feb  1 00:25:24 2019", new Time(time2)->gmctime());
}

private void timeAsDuration(void) {
    mixed *actual;

    actual = (new Time(time2) - new Time(time1))->asDuration();

    expectEqual(TEST_LINE, 5, sizeof(actual));
    expectEqual(TEST_LINE, 20, actual[0]);
    expectEqual(TEST_LINE, 48, actual[1]);
    expectEqual(TEST_LINE, 16, actual[2]);
    expectEqual(TEST_LINE, 0.700231481, actual[3]);
    expectEqual(TEST_LINE, "16:48:20", actual[4]);

    actual = new Time(0)->asDuration();
    expectEqual(TEST_LINE, 3, sscanf(actual[4], "%s:%s:%s"));

    actual = new Time(9)->asDuration();
    expectEqual(TEST_LINE, "00:00:09", actual[4]);

    actual = new Time(10)->asDuration();
    expectEqual(TEST_LINE, "00:00:10", actual[4]);

    actual = new Time(11)->asDuration();
    expectEqual(TEST_LINE, "00:00:11", actual[4]);

    actual = new Time(59)->asDuration();
    expectEqual(TEST_LINE, "00:00:59", actual[4]);

    actual = new Time(60)->asDuration();
    expectEqual(TEST_LINE, "00:01:00", actual[4]);

    actual = new Time(61)->asDuration();
    expectEqual(TEST_LINE, "00:01:01", actual[4]);
}

void runBeforeTests(void) {
    time1 = 1548920224;
    time2 = 1548980724;
}

void runTests(void) {
    timeFromNoArg();
    timeFromTimeMtime();
    timeFromInvalid();
    timeComparisons();
    timeGmtime();
    timeAsDuration();
}
