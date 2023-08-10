#include "main.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <zcmocka.h>

DEFINE_DYNARRAY_TYPE(long, dynArrayLong, dynArray)
DEFINE_DYNARRAY_TYPE(long, dynArrayLong, dynArrayTen, .size = 10)
DEFINE_DYNARRAY_TYPE(long, dynArrayLong, dynArrayBig, .capacity = 20, .growth = 2.5, .size = 5)
DEFINE_DYNARRAY_TYPE(long, dynArrayLong, dynArrayOversize, .capacity = 20, .growth = 0.5, .size = 25)

dynArrayLong* pDA;

void test_new(void** state)
{
    pDA = dynArray();

    assert_int_equal(pDA->capacity, 10);
    assert_float_equal(pDA->growth, 1.5, 0.0);
}


void test_new_params(void** state)
{
    pDA = dynArrayBig();

    assert_int_equal(pDA->capacity, 20);
    assert_float_equal(pDA->growth, 2.5, 0.0);
    assert_int_equal(pDA->size, 5);
}

void test_new_params_limits(void** state)
{
    pDA = dynArrayOversize();

    assert_int_equal(pDA->capacity, 25);
    assert_float_equal(pDA->growth, 1.5, 0.0);
    assert_int_equal(pDA->size, pDA->capacity);
}

void test_addDA(void** state)
{
    pDA = dynArray();

    size_t i, max = 18;
    for(i = 0; i < max; i++) {
        addDAdynArray(pDA, i);
    }

    assert_int_equal(pDA->capacity, 22);
    assert_int_equal(pDA->size, max);
}

void test_getDA(void** state)
{
    pDA = dynArray();

    size_t max = pDA->capacity;
    long i;
    for(i = 0; i < max; i++) {
        addDAdynArray(pDA, i);
    }

    long value = 0;
    for(i = 0; i < max; i++) {
        getDAdynArray(pDA, i, &value);
        assert_int_equal(value, i);
    }
}


void test_setDA(void** state)
{
    size_t i, max = 10;
    long value;
    pDA = dynArrayTen();

    for(i = 0; i < max; i++) {
        getDA(pDA, i, &value);
        assert_int_equal(value, 0);
        value = i * 10;
        setDAdynArray(pDA, i, value);
    }

    for(i = 0; i < max; i++) {
        getDAdynArrayTen(pDA, i, &value);
        assert_int_equal(value, i * 10);
    }
}


/* These functions will be used to initialize
   and clean resources up after each test run */
int setup(void** state)
{
    pDA = NULL;
    return 0;
}

int teardown(void** state)
{
    freeDA(pDA);
    return 0;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_new),
        cmocka_unit_test(test_new_params),
        cmocka_unit_test(test_new_params_limits),
        cmocka_unit_test(test_addDA),
        cmocka_unit_test(test_getDA),
        cmocka_unit_test(test_setDA),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */

    int count_fail_tests = cmocka_run_group_tests(tests, setup, teardown);

    return count_fail_tests;
}