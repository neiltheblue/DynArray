#include "tree.h"

#include <setjmp.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <zcmocka.h>

#define COUNT 10
#define BUFFER 100
#define FILENAME "mm.dat"

hashTree *pHT = NULL;
hashTree *pMMHT = NULL;
hashTree *pOther = NULL;
const int buffer = BUFFER;
const int count = COUNT;
char keys[COUNT][BUFFER];
char values[COUNT][BUFFER];
keyEntry kEntry[COUNT];

void makeKeyValues(int count, char keys[][100], char values[][100],
                   keyEntry kEntry[]) {
  for (int i = 0; i < count; i++) {
    snprintf(keys[i], buffer, "Key %d", i);
    snprintf(values[i], buffer, "Values %d", i);
    kEntry[i] = (keyEntry){.key = keys[i], .length = strlen(keys[i])};
  }
}

hashEntry *getIndexNodeHT(size_t nodeIndex) {
  return getDA(pHT->da, nodeIndex);
}

void printTree(hashTree *pHT) { drawTree(pHT, NULL); }

void test_hash(void **state) {
  char *input = "this is my test key that needs to be longer than 32 chars";
  int len = strlen(input);

  uint32_t h = hash(input, len, 0);
  assert_int_equal(h, 1770453680);

  h = hash(input, len, 1);
  assert_int_equal(h, 4058936515);
}

void test_addFirstLevelHT(void **state) {

  setHT(pHT, &kEntry[0], values[0]);
  assert_int_equal(pHT->da->size, 1);
  assert_int_equal(pHT->root, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->value, values[0]);

  setHT(pHT, &kEntry[0], values[1]);
  assert_int_equal(pHT->da->size, 1);
  assert_int_equal(pHT->root, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->value, values[1]);

  assert_int_equal(maxDepthHT(pHT, 0), 1);
}

void test_addSecondLevelHT(void **state) {

  setHT(pHT, &kEntry[3], values[3]);
  assert_int_equal(pHT->da->size, 1);
  assert_int_equal(pHT->root, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->value, values[3]);

  setHT(pHT, &kEntry[1], values[1]);
  assert_int_equal(pHT->da->size, 2);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 1))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 1))->value, values[1]);

  setHT(pHT, &kEntry[7], values[7]);
  assert_int_equal(pHT->da->size, 3);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 2))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 2))->value, values[7]);

  setHT(pHT, &kEntry[8], values[8]);
  assert_int_equal(pHT->da->size, 4);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 3))->parent, 2);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 3))->value, values[8]);

  assert_int_equal(maxDepthHT(pHT, 0), 3);
}

void test_addThirdLevelHT(void **state) {

  setHT(pHT, &kEntry[3], values[3]);
  assert_int_equal(pHT->da->size, 1);
  assert_int_equal(pHT->root, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 0))->value, values[3]);

  setHT(pHT, &kEntry[1], values[1]);
  assert_int_equal(pHT->da->size, 2);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 1))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 1))->value, values[1]);

  setHT(pHT, &kEntry[7], values[7]);
  assert_int_equal(pHT->da->size, 3);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 2))->parent, 0);
  assert_int_equal(((hashEntry *)getDA(pHT->da, 2))->value, values[7]);

  size_t idx = 0;
  printf("Index %lu\n", idx);
  setHT(pHT, &kEntry[idx], values[idx]);
  assert_int_equal(pHT->da->size, 4);
}

void test_setAllHT(void **state) {

  for (int i = 0; i < count; i++) {
    setHT(pOther, &kEntry[i], values[i]);
  }

  assert_int_equal(pHT->da->size, 0);
  assert_int_equal(pOther->da->size, count);

  setAllHT(pHT, pOther);

  assert_int_equal(pHT->da->size, count);
  assert_int_equal(pOther->da->size, count);

  for (int i = 0; i < count; i++) {
    assert_true(hasEntryHT(pHT, getHT(pOther, &kEntry[i])->kEntry));
  }

  setAllHT(pHT, pOther);

  assert_int_equal(pHT->da->size, count);
  assert_int_equal(pOther->da->size, count);

  for (int i = 0; i < count - 1; i++) {
    assert_true(hasEntryHT(pHT, getHT(pOther, &kEntry[i])->kEntry));
  }
}

void test_drawNode(void **state) {

  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }
  printf("depth: %d\n", maxDepthHT(pHT, 0));
  drawNode(pHT, 0, NULL);
}

void test_getNode(void **state) {

  for (int i = 0; i < count - 1; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }

  size_t idx;
  hashEntry *found;

  idx = 0;
  found = getHT(pHT, &kEntry[idx]);
  assert_non_null(found);
  assert_int_equal(strcmp(found->kEntry->key, keys[idx]), 0);

  idx = 1;
  found = getHT(pHT, &kEntry[idx]);
  assert_non_null(found);
  assert_int_equal(strcmp(found->kEntry->key, keys[idx]), 0);

  idx = 2;
  found = getHT(pHT, &kEntry[idx]);
  assert_non_null(found);
  assert_int_equal(strcmp(found->kEntry->key, keys[idx]), 0);

  idx = 3;
  found = getHT(pHT, &kEntry[idx]);
  assert_non_null(found);
  assert_int_equal(strcmp(found->kEntry->key, keys[idx]), 0);

  idx = 4;
  found = getHT(pHT, &kEntry[idx]);
  assert_non_null(found);
  assert_int_equal(strcmp(found->kEntry->key, keys[idx]), 0);

  idx = 10;
  keyEntry none = (keyEntry){.key = "x", .length = strlen("x")};
  found = getHT(pHT, &none);
  assert_null(found);
}

bool visit_test(const hashEntry *entry, const size_t entryIndex, void *ref) {
  int *counter = (int *)ref;
  (*counter)++;
  return true;
}

void test_vist(void **state) {

  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }

  int counter = 0;
  visitNodesHT(pHT, visit_test, &counter);
  assert_int_equal(counter, count);
}

bool checkBalance(const hashEntry *entry, const size_t entryIndex, void *ref) {

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

bool checkParent(const hashEntry *entry, const size_t entryIndex, void *ref) {

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

  for (int i = 0; i < 1; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }
  balanceHT(pHT);
  assert_int_equal(maxDepthHT(pHT, pHT->root), 1);
}

void test_balanceLeft(void **state) {

  for (int i = 0; i < 7; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }
  balanceHT(pHT);
  visitNodesHT(pHT, checkBalance, NULL);
}

void test_balanceRight(void **state) {

  int idx;
  idx = 0;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 5;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 4;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 6;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 1;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 7;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 3;
  setHT(pHT, &kEntry[idx], values[idx]);

  balanceHT(pHT);
  assert_int_equal(maxDepthHT(pHT, pHT->root), 4);
  visitNodesHT(pHT, checkBalance, NULL);
}

void test_balanceRootRight(void **state) {

  int idx;
  idx = 0;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 4;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 5;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 6;
  setHT(pHT, &kEntry[idx], values[idx]);

  balanceHT(pHT);
  assert_int_equal(pHT->root, 1);
  assert_int_equal(maxDepthHT(pHT, pHT->root), 3);
  visitNodesHT(pHT, checkBalance, NULL);
}

void test_balanceRootLeft(void **state) {

  int idx;
  idx = 6;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 5;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 0;
  setHT(pHT, &kEntry[idx], values[idx]);
  idx = 4;
  setHT(pHT, &kEntry[idx], values[idx]);

  balanceHT(pHT);
  assert_int_equal(pHT->root, 1);
  assert_int_equal(maxDepthHT(pHT, pHT->root), 3);
  visitNodesHT(pHT, checkBalance, NULL);
}

void test_delete(void **state) {

  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }
  visitNodesHT(pHT, checkParent, NULL);
  assert_int_equal(pHT->da->size, count);

  // delete none
  keyEntry none = (keyEntry){.key = "x", .length = strlen("x")};
  deleteHT(pHT, &none);
  visitNodesHT(pHT, checkParent, NULL);
  assert_int_equal(pHT->da->size, count);

  // delete leaf
  int idx;
  idx = 7;
  deleteHT(pHT, &kEntry[idx]);
  visitNodesHT(pHT, checkParent, NULL);
  assert_true(getHT(pHT, &kEntry[idx]) == NULL);
  assert_int_equal(pHT->da->size, count - 1);

  // delete branch
  idx = 4;
  deleteHT(pHT, &kEntry[idx]);
  visitNodesHT(pHT, checkParent, NULL);
  assert_true(getHT(pHT, &kEntry[idx]) == NULL);
  assert_true(getHT(pHT, &kEntry[6]) != NULL);
  assert_true(getHT(pHT, &kEntry[8]) != NULL);
  assert_true(getHT(pHT, &kEntry[5]) != NULL);
  assert_int_equal(pHT->da->size, count - 2);

  // delete root
  idx = 0;
  deleteHT(pHT, &kEntry[idx]);
  visitNodesHT(pHT, checkParent, NULL);
  assert_true(getHT(pHT, &kEntry[idx]) == NULL);
  assert_true(getHT(pHT, &kEntry[1]) != NULL);
  assert_true(getHT(pHT, &kEntry[3]) != NULL);
  assert_true(getHT(pHT, &kEntry[2]) != NULL);
  assert_true(getHT(pHT, &kEntry[9]) != NULL);
  assert_true(getHT(pHT, &kEntry[6]) != NULL);
  assert_true(getHT(pHT, &kEntry[8]) != NULL);
  assert_true(getHT(pHT, &kEntry[5]) != NULL);
  assert_int_equal(pHT->da->size, count - 3);

  // delete rest
  int rest[] = {1, 3, 2, 9, 6, 8, 5};
  for (int i = 0; i < 7; i++) {
    idx = rest[i];
    deleteHT(pHT, &kEntry[idx]);
    visitNodesHT(pHT, checkParent, NULL);
    assert_true(getHT(pHT, &kEntry[idx]) == NULL);
    for (int j = i + 1; j < 7; j++) {
      assert_true(getHT(pHT, &kEntry[rest[j]]) != NULL);
    }
    assert_int_equal(pHT->da->size, count - 4 - i);
  }

  assert_int_equal(pHT->da->size, 0);

  // re-add
  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }
  visitNodesHT(pHT, checkParent, NULL);
  assert_int_equal(pHT->da->size, count);
}

void deletedCallback(const hashTree *pHT, const keyEntry *kEntry, void *value,
                     void *ref) {
  assert_int_equal(strcmp((char *)(kEntry->key), (char *)ref), 0);
}

void test_deleteCallback(void **state) {

  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }
  visitNodesHT(pHT, checkParent, NULL);
  assert_int_equal(pHT->da->size, count);

  for (int i = 0; i < count; i++) {
    hashEntry *he = getDA(pHT->da, 0);
    char kstr[100];
    strcpy(kstr, he->kEntry->key);
    deleteCallbackHT(pHT, he->kEntry, deletedCallback, kstr);
  }
}

void test_hasEntry(void **state) {

  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }
  keyEntry noEntry = (keyEntry){.key = "x", .length = strlen("x")};

  assert_false(hasEntryHT(pHT, &noEntry));

  for (int i = 0; i < count; i++) {
    assert_true(hasEntryHT(pHT, &kEntry[i]));
  }

  setHT(pOther, &kEntry[2], values[2]);
  setHT(pOther, &kEntry[5], values[5]);
  setHT(pOther, &kEntry[8], values[8]);

  assert_true(hasAllHT(pHT, pOther));

  setHT(pOther, &noEntry, "x");

  assert_false(hasAllHT(pHT, pOther));
}

void test_clear(void **state) {

  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }

  assert_int_equal(pHT->da->size, count);
  assert_int_equal(pHT->root, 0);

  clearHT(pHT);

  assert_int_equal(pHT->da->size, 0);
  assert_int_equal(pHT->root, -1);

  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }

  assert_int_equal(pHT->da->size, count);
  assert_int_equal(pHT->root, 0);
}

void test_copyTree(void **state) {

  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
  }

  assert_int_equal(pHT->da->size, count);
  assert_int_equal(pHT->root, 0);

  pOther = copyHT(pHT);
  clearHT(pHT);

  assert_int_equal(pHT->da->size, 0);
  assert_int_equal(pHT->root, -1);

  assert_int_equal(pOther->da->size, count);
  assert_int_equal(pOther->root, 0);

  for (int i = 0; i < count; i++) {
    assert_true(hasEntryHT(pOther, &kEntry[i]));
    assert_false(hasEntryHT(pHT, &kEntry[i]));
  }
}

void test_retainAll(void **state) {

  for (int i = 0; i < count; i++) {
    setHT(pHT, &kEntry[i], values[i]);
    if (i % 2 == 0) {
      setHT(pOther, &kEntry[i], values[i]);
    }
  }

  for (int i = 0; i < count; i++) {
    assert_true(hasEntryHT(pHT, &kEntry[i]));
    if (i % 2 == 0) {
      assert_true(hasEntryHT(pOther, &kEntry[i]));
    }
  }

  retainAllHT(pHT, pOther);

  for (int i = 0; i < count; i++) {
    if (i % 2 == 0) {
      assert_true(hasEntryHT(pOther, &kEntry[i]));
      assert_true(hasEntryHT(pHT, &kEntry[i]));
    } else {
      assert_false(hasEntryHT(pOther, &kEntry[i]));
      assert_false(hasEntryHT(pHT, &kEntry[i]));
    }
  }
}

void test_mmap(void **state) {

  hashTreeParams params = (hashTreeParams){.filename = FILENAME};
  pMMHT = createHT(compareString, &params);

  for (int i = 0; i < count; i++) {
    setHT(pMMHT, &kEntry[i], values[i]);
  }

  balanceHT(pMMHT);  
  freeHT(pMMHT);

  pMMHT = loadHT(FILENAME, compareString);
  
  assert_int_equal(pMMHT->root, 4);

  for (int i = 0; i < count; i++) {
    assert_true(hasEntryHT(pMMHT, &kEntry[i]));
  }
}

int setupHT(void **state) {

  pHT = createHT(compareString, NULL);
  pOther = createHT(compareString, NULL);
  pMMHT = NULL;
  makeKeyValues(count, keys, values, kEntry);

  return 0;
}

int teardownHT(void **state) {
  if (pHT) {
    freeHT(pHT);
    pHT = NULL;
  }

  if (pOther) {
    freeHT(pOther);
    pOther = NULL;
  }

  if (pMMHT) {
    freeHT(pMMHT);
    pMMHT = NULL;
  }

  remove(FILENAME);

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
      cmocka_unit_test_setup_teardown(test_delete, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_clear, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_deleteCallback, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_hasEntry, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_setAllHT, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_copyTree, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_retainAll, setupHT, teardownHT),
      cmocka_unit_test_setup_teardown(test_mmap, setupHT, teardownHT),

  };

  int count_fail_tests = cmocka_run_group_tests(tests, NULL, NULL);

  return count_fail_tests;
}