#include "main.h"

#include "dynarray.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <zcmocka.h>

DEFINE_DYNARRAY_TYPE(long, dynArrayLng)
DEFINE_DYNARRAY_TYPE(float, dynArrayFlt)
DEFINE_DYNARRAY_TYPE(double, dynArrayDbl)

dynArray *pDALng;
dynArray *pDAFlt;
dynArray *pDADbl;

void test_new(void **state) {
  pDALng = dynArrayLng(NULL);

  assert_int_equal(pDALng->capacity, 10);
  assert_float_equal(pDALng->growth, 1.5, 0.0);
}

void test_new_params(void **state) {
  dynArrayParams params =
      (dynArrayParams){.capacity = 20, .growth = 2.5, .size = 5};
  pDALng = dynArrayLng(&params);

  assert_int_equal(pDALng->capacity, 20);
  assert_float_equal(pDALng->growth, 2.5, 0.0);
  assert_int_equal(pDALng->size, 5);
}

void test_new_params_limits(void **state) {
  dynArrayParams params =
      (dynArrayParams){.capacity = 20, .growth = 0.5, .size = 25};
  pDALng = dynArrayLng(&params);

  assert_int_equal(pDALng->capacity, 25);
  assert_float_equal(pDALng->growth, 1.5, 0.0);
  assert_int_equal(pDALng->size, pDALng->capacity);
}

void test_addDA(void **state) {
  pDALng = dynArrayLng(NULL);

  size_t i, max = 18;
  for (i = 0; i < max; i++) {
    assert_int_equal(addDAdynArrayLng(pDALng, i), i);
  }

  assert_int_equal(pDALng->capacity, 22);
  assert_int_equal(pDALng->size, max);
}

void test_addAllDA(void **state) {
  pDALng = dynArrayLng(NULL);

  assert_int_equal(
      addAllDAdynArrayLng(pDALng, (long[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, 10),
      9);

  int i;
  long value;
  for (i = 0; i < 10; i++) {
    getDAdynArrayLng(pDALng, i, &value);
    assert_int_equal(value, i);
  }

  assert_int_equal(
      addAllDAdynArrayLng(pDALng,
                          (long[]){10, 11, 12, 13, 14, 15, 16, 17, 18, 19}, 10),
      19);
  for (i = 0; i < 20; i++) {
    getDAdynArrayLng(pDALng, i, &value);
    assert_int_equal(value, i);
  }

  for (i = 20; i < 30; i++) {
    assert_int_equal(addDAdynArrayLng(pDALng, i), i);
  }
  assert_int_equal(pDALng->size, 30);
}

void test_getDA(void **state) {
  pDALng = dynArrayLng(NULL);

  size_t max = pDALng->capacity;
  long i;
  for (i = 0; i < max; i++) {
    addDAdynArrayLng(pDALng, i);
  }

  long value = 0;
  for (i = 0; i < max; i++) {
    getDAdynArrayLng(pDALng, i, &value);
    assert_int_equal(value, i);
  }
}

void test_setDA(void **state) {
  size_t i, max = 10;
  long value;
  dynArrayParams params = (dynArrayParams){.size = 10};
  pDALng = dynArrayLng(&params);

  for (i = 0; i < max; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, 0);
    value = i * 10;
    assert_int_equal(setDAdynArrayLng(pDALng, i, value), i);
  }

  for (i = 0; i < max; i++) {
    getDAdynArrayLng(pDALng, i, &value);
    assert_int_equal(value, i * 10);
  }
}

void test_floatType(void **state) {
  size_t i, max = 10;
  float value;
  pDAFlt = dynArrayFlt(NULL);

  for (i = 0; i < max; i++) {
    value = i * 0.5;
    addDAdynArrayFlt(pDAFlt, value);
  }

  for (i = 0; i < max; i++) {
    getDAdynArrayFlt(pDAFlt, i, &value);
    assert_float_equal(value, i * 0.5, 0.01);
  }
}

void test_doubleType(void **state) {
  size_t i, max = 10;
  double value;
  pDADbl = dynArrayDbl(NULL);

  for (i = 0; i < max; i++) {
    value = i * 0.5;
    addDAdynArrayDbl(pDADbl, value);
  }

  for (i = 0; i < max; i++) {
    getDAdynArrayDbl(pDADbl, i, &value);
    assert_float_equal(value, i * 0.5, 0.01);
  }
}

void test_quickSort(void **state) {
  pDALng = dynArrayLng(NULL);

  long arr[] = {8, 7, 6, 1, 0, 9, 2, 6, 0};
  long sorted[] = {0, 0, 1, 2, 6, 6, 7, 8, 9};
  addAllDAdynArrayLng(pDALng, arr, 9);

  sortDAdynArrayLng(pDALng, dynArrayLngCompare);
  long value;
  for (int i = 0; i < 9; i++) {
    getDAdynArrayLng(pDALng, i, &value);
    assert_int_equal(value, sorted[i]);
  }
}

void test_growing(void **state) {
  size_t i, max = 10, capacity, range;
  clock_t start_t, end_t;
  double total_t;
  float growth;

  for (growth = 1.5; growth <= 2.0; growth += 0.5) {
    for (capacity = 100; capacity <= 100000; capacity *= 100) {
      printf("Capacity:%lu\tGrowth:%f\n", capacity, growth);
      for (range = 100000; range <= 10000000; range *= 10) {
        max = range;
        dynArrayParams params =
            (dynArrayParams){.capacity = capacity, .growth = growth};
        pDALng = dynArrayLng(&params);
        start_t = clock();
        for (i = 0; i < max; i++) {
          addDAdynArrayLng(pDALng, (long)i);
        }
        end_t = clock();
        total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;
        printf("Total:%lu\tCapacity: %lu\tCPU Time(sec): %f\n", max,
               pDALng->capacity, total_t);
        freeDA(pDALng);
        pDALng = NULL;
      }
    }
  }
}

/* These functions will be used to initialize
   and clean resources up after each test run */
int setup(void **state) {
  pDALng = NULL;
  pDAFlt = NULL;
  pDADbl = NULL;
  return 0;
}

int teardown(void **state) {
  freeDA(pDALng);
  pDALng = NULL;
  freeDA(pDAFlt);
  pDAFlt = NULL;
  freeDA(pDADbl);
  pDADbl = NULL;
  return 0;
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_new),
      cmocka_unit_test(test_new_params),
      cmocka_unit_test(test_new_params_limits),
      cmocka_unit_test(test_addDA),
      cmocka_unit_test(test_addAllDA),
      cmocka_unit_test(test_getDA),
      cmocka_unit_test(test_setDA),
      cmocka_unit_test(test_floatType),
      cmocka_unit_test(test_doubleType),
      cmocka_unit_test(test_quickSort),
#ifdef PERF
      cmocka_unit_test(test_growing),
#endif // PERF
  };

  /* If setup and teardown functions are not
     needed, then NULL may be passed instead */

  int count_fail_tests = cmocka_run_group_tests(tests, setup, teardown);

  return count_fail_tests;
}