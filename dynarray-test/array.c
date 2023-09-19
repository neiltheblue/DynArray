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
  pDALng = createDA(sizeof(long), NULL, NULL);

  assert_int_equal(pDALng->capacity, 10);
  assert_float_equal(pDALng->growth, 1.5, 0.0);
}

void test_new_params(void **state) {
  dynArrayParams params =
      (dynArrayParams){.capacity = 20, .growth = 2.5, .size = 5};
  pDALng = createDA(sizeof(long), NULL, &params);

  assert_int_equal(pDALng->capacity, 20);
  assert_float_equal(pDALng->growth, 2.5, 0.0);
  assert_int_equal(pDALng->size, 5);
}

void test_new_params_limits(void **state) {
  dynArrayParams params =
      (dynArrayParams){.capacity = 20, .growth = 0.5, .size = 25};
  pDALng = createDA(sizeof(long), NULL, &params);

  assert_int_equal(pDALng->capacity, 25);
  assert_float_equal(pDALng->growth, 1.5, 0.0);
  assert_int_equal(pDALng->size, pDALng->capacity);
}

void test_addDA(void **state) {
  pDALng = createDA(sizeof(long), NULL, NULL);

  size_t i, max = 18;
  for (i = 0; i < max; i++) {
    assert_int_equal(*(size_t *)(addDA(pDALng, &i)), i);
  }

  assert_int_equal(pDALng->capacity, 22);
  assert_int_equal(pDALng->size, max);
}

void test_addAllDA(void **state) {
  pDALng = createDA(sizeof(long), NULL, NULL);

  assert_int_equal(addAllDA(pDALng, (long[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, 10),
                   true);

  long i;
  long *read;
  for (i = 0; i < 10; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, i);
  }

  assert_int_equal(
      addAllDA(pDALng, (long[]){10, 11, 12, 13, 14, 15, 16, 17, 18, 19}, 10),
      true);
  for (i = 0; i < 20; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, i);
  }

  for (i = 20; i < 30; i++) {
    assert_int_equal(*(long *)addDA(pDALng, &i), i);
  }
  assert_int_equal(pDALng->size, 30);
}

void test_getDA(void **state) {
  pDALng = createDA(sizeof(long), NULL, NULL);

  size_t max = pDALng->capacity;
  long i;
  for (i = 0; i < max; i++) {
    addDA(pDALng, &i);
  }

  long *read;
  for (i = 0; i < max; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, i);
  }

  read = getDA(pDALng, 100);
  assert_int_equal(read, NULL);
}

void test_setDA(void **state) {
  size_t i, max = 10;
  long value;
  long *read;
  dynArrayParams params = (dynArrayParams){.size = 10};
  pDALng = createDA(sizeof(long), NULL, &params);

  for (i = 0; i < max; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, 0);
    value = i * 10;
    assert_int_equal(setDA(pDALng, i, &value), true);
  }

  assert_int_equal(setDA(pDALng, 100, &value), false);

  for (i = 0; i < max; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, i * 10);
  }
}

void test_floatType(void **state) {
  size_t i, max = 10;
  float value;
  float *read;
  pDAFlt = createDA(sizeof(float), NULL, NULL);

  for (i = 0; i < max; i++) {
    value = i * 0.5;
    addDA(pDAFlt, &value);
  }

  for (i = 0; i < max; i++) {
    read = getDA(pDAFlt, i);
    assert_float_equal(*read, i * 0.5, 0.01);
  }
}

void test_doubleType(void **state) {
  size_t i, max = 10;
  double value;
  double *read;
  pDADbl = createDA(sizeof(double), NULL, NULL);

  for (i = 0; i < max; i++) {
    value = i * 0.5;
    addDA(pDADbl, &value);
  }

  for (i = 0; i < max; i++) {
    read = getDA(pDADbl, i);
    assert_float_equal(*read, i * 0.5, 0.01);
  }
}

void test_quickSort(void **state) {
  pDALng = createDA(sizeof(long), compareDAlong, NULL);

  long arr[] = {8, 7, 6, 1, 0, 9, 2, 6, 0};
  long sorted[] = {0, 0, 1, 2, 6, 6, 7, 8, 9};
  addAllDA(pDALng, arr, 9);

  sortDA(pDALng, compareDAlong);

  long *read;
  for (int i = 0; i < 9; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, sorted[i]);
  }
}

void test_memRelease(void **state) {
  pDALng = createDA(sizeof(long), NULL, NULL);

  long value;
  long *read;
  for (int i = 0; i < 100; i++) {
    value = i;
    addDA(pDALng, &value);
  }
  assert_true(pDALng->capacity > pDALng->size);
  reduceMemDA(pDALng);
  assert_int_equal(pDALng->capacity, pDALng->size);
  for (int i = 0; i < 100; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, i);
  }
}

void test_reverse(void **state) {
  pDALng = createDA(sizeof(long), NULL, NULL);

  long arr[] = {0, 0, 1, 2, 6, 6, 7, 8, 9};
  addAllDA(pDALng, arr, 9);

  long *read;
  for (int i = 0; i < 9; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, arr[i]);
  }

  reverseDA(pDALng);

  for (int i = 0; i < 9; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, arr[8 - i]);
  }

  pDAFlt = createDA(sizeof(float), NULL, NULL);
  float arrEven[] = {0.0, 0.0, 1.0, 2.0, 6.0, 6.0, 7.0, 8.0};
  addAllDA(pDAFlt, arrEven, 8);
  float *fread;

  for (int i = 0; i < 8; i++) {
    fread = getDA(pDAFlt, i);
    assert_float_equal(*fread, arrEven[i], 0.0);
  }

  reverseDA(pDAFlt);

  for (int i = 0; i < 8; i++) {
    fread = getDA(pDAFlt, i);
    assert_float_equal(*fread, arrEven[7 - i], 0.0);
  }
}

void test_subDA(void **state) {
  pDALng = createDA(sizeof(long), NULL, NULL);
  long value = 666;
  long *read;

  addAllDA(pDALng, (long[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, 10);
  sortDA(pDALng, compareDAlong);

  assert_int_equal(subDA(pDALng, 1, 0), NULL);
  assert_int_equal(subDA(pDALng, 3, 100), NULL);

  dynArray *sub = subDA(pDALng, 2, 7);
  assert_int_equal(sub->parent, pDALng);
  assert_int_equal((long *)addDA(sub, &value), NULL);

  reverseDA(sub);
  setDA(sub, 5, &value);

  read = getDA(pDALng, 7);
  assert_int_equal(*read, 666);

  read = getDA(pDALng, 2);
  assert_int_equal(*read, 7);

  freeDA(sub);
}

void test_copy(void **state) {
  pDALng = createDA(sizeof(long), compareDAlong, NULL);
  dynArray *copy;
  long *read;
  long arr[] = {0, 0, 1, 2, 6, 6, 7, 8, 9};

  addAllDA(pDALng, arr, 9);
  copy = copyDA(pDALng);
  for (int i = 0; i < 9; i++) {
    read = getDA(pDALng, i);
    assert_int_equal(*read, arr[i]);
    read = getDA(copy, i);
    assert_int_equal(*read, arr[i]);
  }
  assert_int_equal(copy->size, pDALng->size);
  assert_int_equal(copy->growth, pDALng->growth);

  freeDA(copy);
}

void test_binSearch(void **state) {
  pDALng = createDA(sizeof(long), compareDAlong, NULL);
  long value;
  assert_int_equal(searchDA(pDALng, &value, NULL), -1);

  value = 10;
  addDA(pDALng, &value);
  value = 9;
  assert_int_equal(searchDA(pDALng, &value, compareDAlong), -1);

  value = 10;
  assert_int_equal(searchDA(pDALng, &value, NULL), 0);

  value = 11;
  addDA(pDALng, &value);
  value = 9;
  assert_int_equal(searchDA(pDALng, &value, NULL), -1);

  value = 10;
  assert_int_equal(searchDA(pDALng, &value, NULL), 0);

  value = 11;
  assert_int_equal(searchDA(pDALng, &value, NULL), 1);

  long arr[] = {0, 0, 1, 2, 6, 6, 7, 8, 9};
  addAllDA(pDALng, arr, 9);

  value = 7;
  assert_int_equal(searchDA(pDALng, &value, compareDAlong), 6);

  value = 6;
  assert_int_equal(searchDA(pDALng, &value, compareDAlong), 5);

  value = 0;
  assert_int_equal(searchDA(pDALng, &value, compareDAlong), 1);

  value = 11;
  assert_int_equal(searchDA(pDALng, &value, NULL), 10);

  value = 12;
  assert_int_equal(searchDA(pDALng, &value, NULL), -1);
}

void test_appendDA(void **state) {
  pDALng = createDA(sizeof(long), NULL, NULL);
  dynArray *other = createDA(sizeof(long), NULL, NULL);
  long *v1, *v2;

  addAllDA(pDALng, (long[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, 10);

  appendDA(other, pDALng);
  assert_int_equal(other->size, pDALng->size);
  for (int i = 0; i < pDALng->size; i++) {
    v1 = getDA(other, i);
    v2 = getDA(pDALng, i);
    assert_int_equal(*v1, *v2);
  }

  appendDA(other, pDALng);
  assert_int_equal(other->size, pDALng->size * 2);
  for (int i = 0; i < pDALng->size; i++) {
    v1 = getDA(other, i);
    v2 = getDA(pDALng, i);
    assert_int_equal(*v1, *v2);

    v1 = getDA(other, i + pDALng->size);
    assert_int_equal(*v1, *v2);
  }

  freeDA(other);
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
        pDALng = createDA(sizeof(long), NULL, &params);
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
      cmocka_unit_test_setup_teardown(test_new, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_new_params, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_new_params_limits, setupDA,
                                      teardownDA),
      cmocka_unit_test_setup_teardown(test_addDA, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_addAllDA, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_getDA, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_setDA, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_floatType, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_doubleType, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_quickSort, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_memRelease, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_reverse, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_copy, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_binSearch, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_subDA, setupDA, teardownDA),
      cmocka_unit_test_setup_teardown(test_appendDA, setupDA, teardownDA),
#ifdef PERF
      cmocka_unit_test_setup_teardown(test_growing, setupDA, teardownDA),
#endif // PERF
  };

  int count_fail_tests = cmocka_run_group_tests(tests, NULL, NULL);

  return count_fail_tests;
}