#include "tree.h"

#include <setjmp.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <zcmocka.h>

hashTree *pHT;
int buffer = 100;

void makeKeyValues(int count, char keys[][100], char values[][100]) {
  for (int i = 0; i < count; i++) {
    snprintf(keys[i], buffer, "Key %d", i);
    snprintf(values[i], buffer, "Values %d", i);
  }
}

void test_hash(void **state) {
  char *input = "this is my test key that needs to be longer than 32 chars";
  int len = strlen(input);

  uint32_t h = hash(input, len, 0);
  assert_int_equal(h, 3685744406);

  h = hash(input, len, 1);
  assert_int_equal(h, 3849301065);
}

void test_addFirstLevelHT(void **state) {
  int count = 1;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  addHT(pHT, keys[0], strlen(keys[0]), values[0]);
  assert_int_equal(pHT->da->size, 1);
  assert_int_equal(pHT->root, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->value, values[0]);

  addHT(pHT, keys[0], strlen(keys[0]), values[1]);
  assert_int_equal(pHT->da->size, 1);
  assert_int_equal(pHT->root, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->value, values[1]);

  assert_int_equal(maxDepthHT(pHT, 0), 1);
}

void test_addSecondLevelHT(void **state) {
  int count = 10;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  addHT(pHT, keys[3], strlen(keys[3]), values[3]);
  assert_int_equal(pHT->da->size, 1);
  assert_int_equal(pHT->root, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->value, values[3]);

  addHT(pHT, keys[1], strlen(keys[1]), values[1]);
  assert_int_equal(pHT->da->size, 2);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 1))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 1))->value, values[1]);

  addHT(pHT, keys[7], strlen(keys[7]), values[7]);
  assert_int_equal(pHT->da->size, 3);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 2))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 2))->value, values[7]);

  assert_int_equal(maxDepthHT(pHT, 0), 3);
}

void test_addThirdLevelHT(void **state) {
  int count = 10;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  addHT(pHT, keys[3], strlen(keys[3]), values[3]);
  assert_int_equal(pHT->da->size, 1);
  assert_int_equal(pHT->root, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->value, values[3]);

  addHT(pHT, keys[1], strlen(keys[1]), values[1]);
  assert_int_equal(pHT->da->size, 2);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 1))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 1))->value, values[1]);

  addHT(pHT, keys[7], strlen(keys[7]), values[7]);
  assert_int_equal(pHT->da->size, 3);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 2))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 2))->value, values[7]);

  size_t idx = 0;
  printf("Index %lu\n", idx);
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  assert_int_equal(pHT->da->size, 4);
}

void test_drawNode(void **state) {
  int count = 5;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  for (int i = 0; i < count; i++) {
    addHT(pHT, keys[i], strlen(keys[i]), values[i]);
  }
  printf("depth: %d\n", maxDepthHT(pHT, 0));
  drawNode(pHT, 0, NULL);
}

void test_getNode(void **state) {
  int count = 10;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  for (int i = 0; i < count; i++) {
    addHT(pHT, keys[i], strlen(keys[i]), values[i]);
  }
  drawNode(pHT, 0, NULL);
//  getHT(key[0]);
}

int setupHT(void **state) {

  pHT = NULL;
  return 0;
}

int teardownHT(void **state) {
  if (pHT) {
    freeHT(pHT);
    pHT = NULL;
  }

  return 0;
}

int test_tree(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test_setup_teardown(test_hash, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_addFirstLevelHT, setupHT,
                                      teardownHT),
      cmocka_unit_test_setup_teardown(test_addSecondLevelHT, setupHT,
                                      teardownHT),
      cmocka_unit_test_setup_teardown(test_addThirdLevelHT, setupHT,
                                      teardownHT),
      cmocka_unit_test_setup_teardown(test_drawNode, setupHT, teardownHT),
	  cmocka_unit_test_setup_teardown(test_getNode, setupHT, teardownHT),	  

  };

  int count_fail_tests = cmocka_run_group_tests(tests, setupHT, teardownHT);

  return count_fail_tests;
}