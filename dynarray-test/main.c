#include "main.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>
#include <zcmocka.h>

DEFINE_DYNARRAY_TYPE(long, dynArrayLong, dynArray)

dynArrayLong *pDA;

void test_new(void **state) {
  pDA = dynArrayDefault();

  assert_int_equal(pDA->capacity, 10);
  assert_float_equal(pDA->growth, 1.5, 0.0);
}

void test_new_params(void **state) {
  pDA = dynArray((DynArrayParams){.capacity = 20, .growth = 2.5, .size = 5});

  assert_int_equal(pDA->capacity, 20);
  assert_float_equal(pDA->growth, 2.5, 0.0);
  assert_int_equal(pDA->size, 5);
}

void test_new_params_limits(void **state) {
  pDA = dynArray((DynArrayParams){.capacity = 20, .growth = 0.5, .size = 25});

  assert_int_equal(pDA->capacity, 25);
  assert_float_equal(pDA->growth, 1.5, 0.0);
  assert_int_equal(pDA->size, pDA->capacity);
}

void test_addDA(void **state) {
  pDA = dynArrayDefault();

  size_t i, max = 18;
  for (i = 0; i < max; i++) {
    addDAdynArray(pDA, i);
  }

  assert_int_equal(pDA->capacity, 22);
  assert_int_equal(pDA->size, max);
}

void test_getDA(void **state) {
  pDA = dynArrayDefault();

  size_t max = pDA->capacity;
  long i;
  for (i = 0; i < max; i++) {
    addDAdynArray(pDA, i);
  }

  long value = 0;
  for (i = 0; i < max; i++) {
    getDAdynArray(pDA, i, &value);
    assert_int_equal(value, i);
  }
}

void test_setDA(void **state) {
  size_t i, max = 10;
  long value;
  pDA = dynArray((DynArrayParams){.size = 10});

  for (i = 0; i < max; i++) {
    getDA(pDA, i, &value);
    assert_int_equal(value, 0);
    value = i * 10;
    setDAdynArray(pDA, i, value);
  }

  for (i = 0; i < max; i++) {
    getDAdynArray(pDA, i, &value);
    assert_int_equal(value, i * 10);
  }
}

void test_growing(void **state) {
  size_t i, max = 10, capacity, range;
  clock_t start_t, end_t;
  double total_t;
  float growth;

  for (growth = 1.5; growth <= 2.0; growth += 0.1) {
    for (capacity = 100; capacity <= 100000; capacity *= 100) {
      printf("Capacity:%lu\tGrowth:%f\n", capacity, growth);
      for (range = 100000; range <= 10000000; range *= 10) {
        max = range;
        pDA =
            dynArray((DynArrayParams){.capacity = capacity, .growth = growth});
        start_t = clock();
        for (i = 0; i < max; i++) {
          addDAdynArray(pDA, (long)i);
        }
        end_t = clock();
        total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        printf("Total:%lu\tCapacity: %lu\tCPU Time(sec): %f\n", max,
               pDA->capacity, total_t);
        freeDA(pDA);
        pDA = NULL;
      }
    }
  }
}

/* These functions will be used to initialize
   and clean resources up after each test run */
int setup(void **state) {
  pDA = NULL;
  return 0;
}

int teardown(void **state) {
  freeDA(pDA);
  pDA = NULL;
  return 0;
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_new),
      cmocka_unit_test(test_new_params),
      cmocka_unit_test(test_new_params_limits),
      cmocka_unit_test(test_addDA),
      cmocka_unit_test(test_getDA),
      cmocka_unit_test(test_setDA),
      cmocka_unit_test(test_growing),
  };

  /* If setup and teardown functions are not
     needed, then NULL may be passed instead */

  int count_fail_tests = cmocka_run_group_tests(tests, setup, teardown);

  return count_fail_tests;
}