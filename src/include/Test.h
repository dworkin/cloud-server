#ifndef _TEST_H_
#define _TEST_H_

#include <NKlib.h>

#define TEST_LINE testLine(call_trace())

#define TEST_RUNNER "/usr/Test/sys/TestRunner"

#define TEST_OBJ_PATH "/usr/Test/obj"
#define TEST_SPEC_PATH "/*Test.c"

#define TEST_LIB "/lib/Test"
#define Test object TEST_LIB

#define MOCK_RANDOM_LIB "/usr/Test/lib/MockRandom"
#define MockRandom object MOCK_RANDOM_LIB

#define TEST_FUNCTION "/usr/Test/lib/TestFunction"
#define TestFunction object TEST_FUNCTION

#endif
