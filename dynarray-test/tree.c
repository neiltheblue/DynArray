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

void test_addHT(void **state) {
  int count = 1;
  char keys[count][buffer];
  char values[count][buffer];
  pHT = createHT(compareString, NULL);
  makeKeyValues(count, keys, values);

  for (int i = 0; i < count; i++) {
    printf("%d) Key: %s Value: %s\n", i, keys[i], values[i]);
    addHT(pHT, keys[i], strlen(keys[i]), values[i]);
  }
  
  assert_int_equal(pHT->da->size, 1);
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
      cmocka_unit_test_setup_teardown(test_addHT, setupHT, teardownHT),
  };

  int count_fail_tests = cmocka_run_group_tests(tests, setupHT, teardownHT);

  return count_fail_tests;
}