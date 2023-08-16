#include "array.h"

#include <setjmp.h>
#include <stdint.h>
#include <time.h>
#include <zcmocka.h>

DEFINE_COMPARE_TYPE(long)
DEFINE_COMPARE_TYPE(float)

dynArray *pDALng;
dynArray *pDAFlt;
dynArray *pDADbl;

void test_new(void **state) {
  pDALng = createDA(sizeof(long), NULL);

  assert_int_equal(pDALng->capacity, 10);
  assert_float_equal(pDALng->growth, 1.5, 0.0);
}

void test_new_params(void **state) {
  dynArrayParams params =
      (dynArrayParams){.capacity = 20, .growth = 2.5, .size = 5};
  pDALng = createDA(sizeof(long), &params);

  assert_int_equal(pDALng->capacity, 20);
  assert_float_equal(pDALng->growth, 2.5, 0.0);
  assert_int_equal(pDALng->size, 5);
}

void test_new_params_limits(void **state) {
  dynArrayParams params =
      (dynArrayParams){.capacity = 20, .growth = 0.5, .size = 25};
  pDALng = createDA(sizeof(long), &params);

  assert_int_equal(pDALng->capacity, 25);
  assert_float_equal(pDALng->growth, 1.5, 0.0);
  assert_int_equal(pDALng->size, pDALng->capacity);
}

void test_addDA(void **state) {
  pDALng = createDA(sizeof(long), NULL);

  size_t i, max = 18;
  for (i = 0; i < max; i++) {
    assert_int_equal(addDA(pDALng, &i), true);
  }

  assert_int_equal(pDALng->capacity, 22);
  assert_int_equal(pDALng->size, max);

  pDALng->parent = pDALng;
  assert_int_equal(addDA(pDALng, &i), false);
}

void test_addAllDA(void **state) {
  pDALng = createDA(sizeof(long), NULL);

  assert_int_equal(addAllDA(pDALng, (long[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, 10),
                   true);

  int i;
  long value;
  for (i = 0; i < 10; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, i);
  }

  assert_int_equal(
      addAllDA(pDALng, (long[]){10, 11, 12, 13, 14, 15, 16, 17, 18, 19}, 10),
      true);
  for (i = 0; i < 20; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, i);
  }

  for (i = 20; i < 30; i++) {
    assert_int_equal(addDA(pDALng, &i), true);
  }
  assert_int_equal(pDALng->size, 30);

  pDALng->parent = pDALng;
  assert_int_equal(
      addAllDA(pDALng, (long[]){10, 11, 12, 13, 14, 15, 16, 17, 18, 19}, 10),
      false);
}

void test_getDA(void **state) {
  pDALng = createDA(sizeof(long), NULL);

  size_t max = pDALng->capacity;
  long i;
  for (i = 0; i < max; i++) {
    addDA(pDALng, &i);
  }

  long value = 0;
  for (i = 0; i < max; i++) {
    assert_int_equal(getDA(pDALng, i, &value), true);
    assert_int_equal(value, i);
  }

  assert_int_equal(getDA(pDALng, 100, &value), false);
}

void test_setDA(void **state) {
  size_t i, max = 10;
  long value;
  dynArrayParams params = (dynArrayParams){.size = 10};
  pDALng = createDA(sizeof(long), &params);

  for (i = 0; i < max; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, 0);
    value = i * 10;
    assert_int_equal(setDA(pDALng, i, &value), true);
  }

  assert_int_equal(setDA(pDALng, 100, &value), false);

  for (i = 0; i < max; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, i * 10);
  }
}

void test_floatType(void **state) {
  size_t i, max = 10;
  float value;
  pDAFlt = createDA(sizeof(float), NULL);

  for (i = 0; i < max; i++) {
    value = i * 0.5;
    addDA(pDAFlt, &value);
  }

  for (i = 0; i < max; i++) {
    getDA(pDAFlt, i, &value);
    assert_float_equal(value, i * 0.5, 0.01);
  }
}

void test_doubleType(void **state) {
  size_t i, max = 10;
  double value;
  pDADbl = createDA(sizeof(double), NULL);

  for (i = 0; i < max; i++) {
    value = i * 0.5;
    addDA(pDADbl, &value);
  }

  for (i = 0; i < max; i++) {
    getDA(pDADbl, i, &value);
    assert_float_equal(value, i * 0.5, 0.01);
  }
}

void test_quickSort(void **state) {
  pDALng = createDA(sizeof(long), NULL);

  long arr[] = {8, 7, 6, 1, 0, 9, 2, 6, 0};
  long sorted[] = {0, 0, 1, 2, 6, 6, 7, 8, 9};
  addAllDA(pDALng, arr, 9);

  sortDA(pDALng, compareDAlong);

  long value;
  for (int i = 0; i < 9; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, sorted[i]);
  }
}

void test_memRelease(void **state) {
  pDALng = createDA(sizeof(long), NULL);

  long value;
  for (int i = 0; i < 100; i++) {
    value = i;
    addDA(pDALng, &value);
  }
  assert_true(pDALng->capacity > pDALng->size);
  reduceMemDA(pDALng);
  assert_int_equal(pDALng->capacity, pDALng->size);
  for (int i = 0; i < 100; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, i);
  }
}

void test_reverse(void **state) {
  pDALng = createDA(sizeof(long), NULL);

  long arr[] = {0, 0, 1, 2, 6, 6, 7, 8, 9};
  addAllDA(pDALng, arr, 9);

  long value;
  for (int i = 0; i < 9; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, arr[i]);
  }

  reverseDA(pDALng);

  for (int i = 0; i < 9; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, arr[8 - i]);
  }

  pDAFlt = createDA(sizeof(float), NULL);
  float arrEven[] = {0.0, 0.0, 1.0, 2.0, 6.0, 6.0, 7.0, 8.0};
  addAllDA(pDAFlt, arrEven, 8);
  float fValue;

  for (int i = 0; i < 8; i++) {
    getDA(pDAFlt, i, &fValue);
    assert_float_equal(fValue, arrEven[i], 0.0);
  }

  reverseDA(pDAFlt);

  for (int i = 0; i < 8; i++) {
    getDA(pDAFlt, i, &fValue);
    assert_float_equal(fValue, arrEven[7 - i], 0.0);
  }
}

void test_subDA(void **state) {
  pDALng = createDA(sizeof(long), NULL);
  long value = 666;

  addAllDA(pDALng, (long[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, 10);
  sortDA(pDALng, compareDAlong);

  assert_int_equal(subDA(pDALng, 1, 0), NULL);
  assert_int_equal(subDA(pDALng, 3, 100), NULL);

  dynArray *sub = subDA(pDALng, 2, 7);
  assert_int_equal(sub->parent, pDALng);
  assert_int_equal(addDA(sub, &value), false);

  reverseDA(sub);
  setDA(sub, 5, &value);

  getDA(pDALng, 7, &value);
  assert_int_equal(value, 666);

  getDA(pDALng, 2, &value);
  assert_int_equal(value, 7);

  freeDA(sub);
}

void test_copy(void **state) {
  pDALng = createDA(sizeof(long), NULL);
  dynArray *copy;
  long value;
  long arr[] = {0, 0, 1, 2, 6, 6, 7, 8, 9};

  addAllDA(pDALng, arr, 9);
  copy = copyDA(pDALng);
  for (int i = 0; i < 9; i++) {
    getDA(pDALng, i, &value);
    assert_int_equal(value, arr[i]);
    getDA(copy, i, &value);
    assert_int_equal(value, arr[i]);
  }
  assert_int_equal(copy->size, pDALng->size);
  assert_int_equal(copy->growth, pDALng->growth);

  freeDA(copy);
}

void test_binSearch(void **state) {
  pDALng = createDA(sizeof(long), NULL);
  long value;
  size_t index;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), false);

  value = 10;
  addDA(pDALng, &value);
  value = 9;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), false);

  value = 10;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), true);
  assert_int_equal(index, 0);

  value = 11;
  addDA(pDALng, &value);
  value = 9;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), false);

  value = 10;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), true);
  assert_int_equal(index, 0);

  value = 11;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), true);
  assert_int_equal(index, 1);

  long arr[] = {0, 0, 1, 2, 6, 6, 7, 8, 9};
  addAllDA(pDALng, arr, 9);

  value = 7;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), true);
  getDA(pDALng, index, &value);
  assert_int_equal(value, 7);

  value = 6;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), true);
  getDA(pDALng, index, &value);
  assert_int_equal(value, 6);

  value = 0;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), true);
  getDA(pDALng, index, &value);
  assert_int_equal(value, 0);

  value = 11;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), true);
  getDA(pDALng, index, &value);
  assert_int_equal(value, 11);

  value = 12;
  assert_int_equal(searchDA(pDALng, compareDAlong, &value, &index), false);
}

void test_appendDA(void **state) {
  pDALng = createDA(sizeof(long), NULL);
  dynArray *other = createDA(sizeof(long), NULL);
  long v1, v2;

  addAllDA(pDALng, (long[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, 10);

  appendDA(other, pDALng);
  assert_int_equal(other->size, pDALng->size);
  for (int i = 0; i < pDALng->size; i++) {
    getDA(other, i, &v1);
    getDA(pDALng, i, &v2);
    assert_int_equal(v1, v2);
  }

  appendDA(other, pDALng);
  assert_int_equal(other->size, pDALng->size * 2);
  for (int i = 0; i < pDALng->size; i++) {
    getDA(other, i, &v1);
    getDA(pDALng, i, &v2);
    assert_int_equal(v1, v2);

    getDA(other, i + pDALng->size, &v1);
    assert_int_equal(v1, v2);
  }
}

void test_growing(void **state) {
  size_t i, max = 10, capacity, range;
  clock_t start_t, end_t;
  double total_t;
  float growth;
  long value;

  for (growth = 1.5; growth <= 2.0; growth += 0.5) {
    for (capacity = 100; capacity <= 100000; capacity *= 100) {
      printf("Capacity:%lu\tGrowth:%f\n", capacity, growth);
      for (range = 100000; range <= 10000000; range *= 10) {
        max = range;
        dynArrayParams params =
            (dynArrayParams){.capacity = capacity, .growth = growth};
        pDALng = createDA(sizeof(long), &params);
        start_t = clock();
        for (i = 0; i < max; i++) {
          value = i;
          addDA(pDALng, &value);
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

int setupDA(void **state) {
  pDALng = NULL;
  pDAFlt = NULL;
  pDADbl = NULL;
  return 0;
}

int teardownDA(void **state) {
  freeDA(pDALng);
  pDALng = NULL;
  freeDA(pDAFlt);
  pDAFlt = NULL;
  freeDA(pDADbl);
  pDADbl = NULL;
  return 0;
}

int test_array(void) {
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
      cmocka_unit_test(test_memRelease),
      cmocka_unit_test(test_reverse),
      cmocka_unit_test(test_copy),
      cmocka_unit_test(test_binSearch),
      cmocka_unit_test(test_subDA),
      cmocka_unit_test(test_appendDA),
#ifdef PERF
      cmocka_unit_test(test_growing),
#endif // PERF
  };

  int count_fail_tests = cmocka_run_group_tests(tests, setupDA, teardownDA);

  return count_fail_tests;
}