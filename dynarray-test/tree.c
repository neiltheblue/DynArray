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

  addHT(pHT, keys[8], strlen(keys[8]), values[8]);
  assert_int_equal(pHT->da->size, 4);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 3))->parent, 2);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 3))->value, values[8]);

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

  for (int i = 0; i < count - 1; i++) {
    addHT(pHT, keys[i], strlen(keys[i]), values[i]);
  }

  size_t idx;
  hashEntry *found;

  idx = 0;
  found = getHT(pHT, keys[idx], strlen(keys[idx]));
  assert_non_null(found);
  assert_int_equal(strcmp(found->key, keys[idx]), 0);

  idx = 1;
  found = getHT(pHT, keys[idx], strlen(keys[idx]));
  assert_non_null(found);
  assert_int_equal(strcmp(found->key, keys[idx]), 0);

  idx = 2;
  found = getHT(pHT, keys[idx], strlen(keys[idx]));
  assert_non_null(found);
  assert_int_equal(strcmp(found->key, keys[idx]), 0);

  idx = 3;
  found = getHT(pHT, keys[idx], strlen(keys[idx]));
  assert_non_null(found);
  assert_int_equal(strcmp(found->key, keys[idx]), 0);

  idx = 4;
  found = getHT(pHT, keys[idx], strlen(keys[idx]));
  assert_non_null(found);
  assert_int_equal(strcmp(found->key, keys[idx]), 0);

  idx = 10;
  found = getHT(pHT, keys[idx], strlen(keys[idx]));
  assert_null(found);
}

bool visit_test(hashEntry *entry, size_t entryIndex, void *ref) {
  int *counter = (int *)ref;
  (*counter)++;
  return true;
}

void test_vist(void **state) {
  int count = 10;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  for (int i = 0; i < count; i++) {
    addHT(pHT, keys[i], strlen(keys[i]), values[i]);
  }

  int counter = 0;
  visitNodesHT(pHT, visit_test, &counter);
  assert_int_equal(counter, count);
}

bool checkBalance(hashEntry *entry, size_t entryIndex, void *ref) {

  int left = maxDepthHT(pHT, entry->left);
  int right = maxDepthHT(pHT, entry->right);
  int diff = abs(left - right);
  assert_true(diff < 2);

  if (entry->left != -1) {
    assert_true(((hashEntry *)getDA(pHT->da, entry->left))->hash < entry->hash);
    assert_int_equal(((hashEntry *)getDA(pHT->da, entry->left))->parent,
                     entryIndex);
  }
  if (entry->right != -1) {
    assert_true(((hashEntry *)getDA(pHT->da, entry->right))->hash >
                entry->hash);
    assert_int_equal(((hashEntry *)getDA(pHT->da, entry->right))->parent,
                     entryIndex);
  }

  return true;
}

void test_balanceNone(void **state) {
  int count = 10;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  for (int i = 0; i < 1; i++) {
    addHT(pHT, keys[i], strlen(keys[i]), values[i]);
  }
  balanceHT(pHT);
  assert_int_equal(maxDepthHT(pHT, pHT->root), 1);
}

void test_balanceLeft(void **state) {
  int count = 10;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  for (int i = 0; i < 7; i++) {
    addHT(pHT, keys[i], strlen(keys[i]), values[i]);
  }
  balanceHT(pHT);
  visitNodesHT(pHT, checkBalance, NULL);
}

void test_balanceRight(void **state) {
  int count = 10;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  int idx;
  idx = 0;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 5;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 4;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 6;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 1;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 7;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 3;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);

  balanceHT(pHT);
  assert_int_equal(maxDepthHT(pHT, pHT->root), 4);
  visitNodesHT(pHT, checkBalance, NULL);
}

void test_balanceRootRight(void **state) {
  int count = 10;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  int idx;
  idx = 0;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 4;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 5;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 6;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);

  balanceHT(pHT);
  assert_int_equal(pHT->root, 1);
  assert_int_equal(maxDepthHT(pHT, pHT->root), 3);
  visitNodesHT(pHT, checkBalance, NULL);
}

void test_balanceRootLeft(void **state) {
  int count = 10;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  int idx;
  idx = 6;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 5;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 0;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);
  idx = 4;
  addHT(pHT, keys[idx], strlen(keys[idx]), values[idx]);

  balanceHT(pHT);
  assert_int_equal(pHT->root, 1);
  assert_int_equal(maxDepthHT(pHT, pHT->root), 3);
  visitNodesHT(pHT, checkBalance, NULL);
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
      cmocka_unit_test_setup_teardown(test_balanceNone, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_balanceLeft, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_balanceRight, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_balanceRootRight, setupHT,
                                      teardownHT),
      cmocka_unit_test_setup_teardown(test_balanceRootLeft, setupHT,
                                      teardownHT),
      cmocka_unit_test_setup_teardown(test_vist, setupHT, teardownHT),

  };

  int count_fail_tests = cmocka_run_group_tests(tests, setupHT, teardownHT);

  return count_fail_tests;
}